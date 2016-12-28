#include <cstdint>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include "nvm0nvm.h"
#include "nvm0hash.h"
#include "nvm0inode.h"
#include "nvm0volume.h"
#include "nvm0monitor.h"
#include <assert.h>
#include <errno.h>
//private function declarations


bool isValidNode(struct hash_node *node)
{
   return node && node->is_valid ? true : false; 
}

bool isFreeLFQueueEnough()
{
    //TODO: synchronize getting free_idx time
    return inode_free_lfqueue->get_size() >= DEFAULT_FREE_MIN_COUNT ? true : false;
}

void awakeBalloonThread()
{
    for(int i = 0; i < DEFAULT_NUM_BALLOON; i++) {
        pthread_mutex_lock(&g_balloon_mutex[i]);
        pthread_cond_signal(&g_balloon_cond[i]);
        pthread_mutex_unlock(&g_balloon_mutex[i]);
    }
}

inline inode_idx_t get_inode_entry_idx_from_hash(struct hash_node *hash_node) {
    return (inode_idx_t) ((char *)hash_node->inode - (char *)nvm->inode_table)/sizeof(inode_entry);
}

size_t writeDataToNvmBlock(inode_idx_t inode_index, uint32_t offset, const char *ptr, size_t count)
{
    inode_entry *inode = &nvm->inode_table[inode_index];
    block_idx_t ownerless_block_index = inode->volume->block_index;
    char *ownerless_block = nvm->block_table[ownerless_block_index].data;

    memcpy(ownerless_block + offset, ptr, count);

    inode->volume->block_index = inode->block_index;
    inode->block_index = ownerless_block_index;

    //TODO:need cache line write guarantee

    return count;
}

inode_idx_t getFreeInodeFromFreeLFQueue(struct volume_entry *ve, uint32_t lbn)
{
    inode_idx_t idx = inode_free_lfqueue->dequeue();
    struct inode_entry* inode = &nvm->inode_table[idx];

    inode->lbn = lbn;
    inode->volume = ve;
    /* inode->state is already free */

    return idx;
}

struct hash_node *get_hash_node_with_lock(
        volume_entry *ve,
        uint32_t lbn)
{
    while (1) {
        struct hash_node *hash_node = search_hash_node(ve->hash_table, lbn);
    
        /* invalid hash node */
        if (!isValidNode(hash_node)) {

            /* get a new inode */
            if (!isFreeLFQueueEnough()) {
                awakeBalloonThread();
                //TODO: do garbage collection in the mean time
                continue;
            }
            inode_idx_t i_idx = getFreeInodeFromFreeLFQueue(ve, lbn);
            inode_entry* new_inode = &nvm->inode_table[i_idx];

            /* if hash was null */
            if (!hash_node) {
                hash_node = new_hash_node(new_inode);
                insert_hash_node(ve->hash_table, hash_node);

            /* if hash was invalid */
            } else if (!hash_node->is_valid) {
                validate_hash_node(hash_node, new_inode);
            }
    
            pthread_mutex_lock(&hash_node->mutex);
            return hash_node;

        /* valid hash node */
        } else {
            pthread_mutex_lock(&hash_node->mutex);

            /* if balloon invalidated the hash node just before the mutex lock */
            if (!hash_node->is_valid) {
                pthread_mutex_unlock(&hash_node->mutex);
                continue;
            }

            return hash_node;
        }
    }
}

/**
 * Durably writes (count) bytes of data pointed by (buf) at (offset) in file described by (vid).
 * Details:
 * Uses non-volatile memory space to efficiently and durably write the data.
 * The data is not necessarily written to the permanent storage at the point of this function's return.
 * Instead, this function guarantees that the data is durably written to the non-volatile space.
 * The data stored in non-volatile memory is later flushed into permanent storage.
 *
 * 1. look
 * TODO: fill in
 * @return the actual bytes durably written.
 */
size_t nvm_durable_write(        
        uint32_t    vid,    /* !<in: volume ID, a unique file descriptor */
        off_t       offset, /* !<in: position of the file to write */ 
        const char  *buf,   /* !<in: buffer */
        size_t      count)  /* !<in: size of the buffer */
{
    /* exception handling */
    //vid
    //offset?
    //buf?
    //count <= 0
    //count % block_size != 0

    /* get volume entry using vid. */
    volume_idx_t    v_idx   = get_volume_entry_idx(vid);
    volume_entry    *ve     = get_volume_entry(v_idx);

    /* let's write! */
    uint32_t curr_lbn = offset / nvm->block_size;
    size_t bytes_written = 0;
    for (uint32_t i = 0; i < count / nvm->block_size; i++, curr_lbn++) {

        /* 1. get hash node and inode index with lbn */
        struct hash_node *hash_node = get_hash_node_with_lock(ve, curr_lbn);
        inode_idx_t idx = get_inode_entry_idx_from_hash(hash_node);

        /* 2. let's write to a block */
        pthread_mutex_lock(&hash_node->inode->lock);    /* critical section start */

        int old_state = hash_node->inode->state;
        hash_node->inode->state = INODE_STATE_DIRTY;

        bytes_written += writeDataToNvmBlock(idx, 0, buf + nvm->block_size * i, nvm->block_size);

        if (old_state != INODE_STATE_DIRTY) {
            inode_dirty_count++;
            inode_dirty_lfqueue[v_idx]->enqueue(idx);
            monitor.dirty++;
        }

        pthread_mutex_unlock(&hash_node->inode->lock);  /* critical section finish */

        pthread_mutex_unlock(&hash_node->mutex);
    }
    return bytes_written;
}
