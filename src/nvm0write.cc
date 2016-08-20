#include <cstdint>
#include <string.h>
#include <pthread.h>
#include "nvm0nvm.h"
#include "nvm0hash.h"
#include "nvm0inode.h"
#include "nvm0volume.h"
#include "nvm0lfqueue.h"
#include "nvm0monitor.h"

//private function declarations


bool isValidNode(struct hash_node *node)
{
   return node && node->is_valid ? true : false; 
}

bool isFreeLFQueueEnough()
{
    return inode_free_lfqueue->get_size() > 1000 ? true : false;
}

void awakeBalloonThread()
{
    pthread_mutex_lock(&g_balloon_mutex);
    pthread_cond_signal(&g_balloon_cond);
    pthread_mutex_unlock(&g_balloon_mutex);
}

inode_idx_t getFreeInodeFromFreeLFQueue(struct volume_entry* ve, uint32_t lbn)
{
    inode_idx_t idx = inode_free_lfqueue->dequeue();
    struct inode_entry* inode = &nvm->inode_table[idx];

    inode->lbn = lbn;
    inode->volume = ve;

    return idx;
}

uint32_t writeDataBlockToNVM(size_t len, uint32_t offset, const char*ptr, struct hash_node *hash_node)
{
    uint32_t write_bytes = (len > nvm->block_size - offset) ? (nvm->block_size - offset) : len;
    inode_idx_t idx = (inode_idx_t)((char *)hash_node->inode - (char *)nvm->inode_table)/sizeof(inode_entry);
    char* data_dst = nvm->datablock_table + nvm->block_size * idx + offset; 
    memcpy(data_dst, ptr, write_bytes);
    //TODO:need cache line write guarantee
    
    return write_bytes;
}

/**
 * Write out (len) bytes data pointed by ptr to nvm structure.
After writing out to nvm, data blocks are enqueued to dirty LFQ.
@return the byte size of data written to nvm */
size_t
nvm_durable_write(
    uint32_t vid,       /* !<in: volume ID */
    off_t    ofs,       /* !<in: volume offset */ 
    const char* ptr,    /* !<in: buffer */
    size_t   len )      /* !<in: size of buffer to be written */
{
    //declaration
    inode_idx_t i_idx;

    /* Get the volume entry index from the nvm volume table.
    The volume entry contains the hash structure, representing one file. */
    volume_idx_t v_idx = get_volume_entry_idx(vid);
    volume_entry* ve = &nvm->volume_table[v_idx];

    /* Calculate how many blocks needed for writing */
    uint32_t lbn_start = ofs / nvm->block_size;
    uint32_t lbn_end = (ofs + len) / nvm->block_size;
    if((ofs + len) % nvm->block_size == 0) {
        lbn_end--; // To avoid dummy data block write
    }
    uint32_t offset = ofs % nvm->block_size;
    size_t written_bytes = 0;

    /* Each loop write one data block to nvm */
    for(uint32_t lbn = lbn_start; lbn <= lbn_end; lbn++) {

        hash_node* node_searched = search_hash_node(ve->hash_table, lbn);

        if (!isValidNode(node_searched)) {

            if(!isFreeLFQueueEnough()) {
                awakeBalloonThread();

                //TODO: do garbage collection in the mean time

                lbn--;

                continue;
            }

            i_idx = getFreeInodeFromFreeLFQueue(ve, lbn);
            inode_entry* new_inode = &nvm->inode_table[i_idx];

            if(node_searched == nullptr) {

                node_searched = new_hash_node(new_inode);
                insert_hash_node(ve->hash_table, node_searched);

            } else if(!node_searched->is_valid) {

                validate_hash_node(node_searched, new_inode);

            }

        }

        pthread_mutex_lock(&node_searched->mutex);
        if(!node_searched->is_valid)
        {
            lbn--;
            continue;
        }

        pthread_mutex_lock(&node_searched->inode->lock);

        int old_state = node_searched->inode->state;
        node_searched->inode->state = INODE_STATE_DIRTY;

        int write_bytes = writeDataBlockToNVM(len, offset, ptr, node_searched);

        if(old_state != INODE_STATE_DIRTY) {
            inode_idx_t idx = (inode_idx_t)((char *)node_searched->inode - (char *)nvm->inode_table)/sizeof(inode_entry);
#if testing
            inode_dirty_lfqueue[vid % MAX_VOLUME_ENTRY]->enqueue(idx);
#else
            inode_dirty_lfqueue[vid % NUM_FLUSH_THR]->enqueue(idx);
#endif
            monitor.dirty++;
        }

        pthread_mutex_unlock(&node_searched->mutex);
        pthread_mutex_unlock(&node_searched->inode->lock);

        /* Re-inintialize offset and len*/
        offset = 0;
        len -= write_bytes;
        written_bytes += written_bytes;
    }

    return written_bytes;
}
