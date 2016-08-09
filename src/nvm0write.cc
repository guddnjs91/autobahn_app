#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "nvm0common.h"

//private function declarations
const char* get_filename(uint32_t vid);

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

inode_entry *getFreeInodeFromFreeLFQueue(struct volume_entry* ve, uint32_t lbn)
{
    inode_idx_t inode_idx = alloc_inode_entry_idx(lbn);
    inode_entry* inode = &nvm->inode_table[inode_idx];
    inode->lbn = lbn;
    inode->volume = ve;

    return inode;
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
void changeInodeStateToDirty(struct volume_entry* ve, struct hash_node *hash_node)
{   
    int old_state = hash_node->inode->state;
    if (old_state == INODE_STATE_CLEAN) 
    {
        remove_list_node(ve->hash_table->invalid_list, hash_node);
    }

    hash_node->inode->state = INODE_STATE_DIRTY;

    inode_idx_t idx = (inode_idx_t)((char *)hash_node->inode - (char *)nvm->inode_table)/sizeof(inode_entry);
    if(old_state != INODE_STATE_DIRTY)
    {
        inode_dirty_lfqueue->enqueue(idx);
    }
}

/**
Write out len bytes data pointed by ptr to nvm structure.
After writing out to nvm, data blocks are enqueued to dirty LFQ.
@return the byte size of data written to nvm */
size_t
nvm_write(
    uint32_t vid,       /* !<in: volume ID */
    off_t    ofs,       /* !<in: volume offset */ 
    const char* ptr,    /* !<in: buffer */ size_t   len)       /* !<in: size of buffer to be written */ {
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

    std::cout << "lbn_end : " << lbn_end << std::endl;
    /* Each loop write one data block to nvm */
    for(uint32_t lbn = lbn_start; lbn <= lbn_end; lbn++) {

        pthread_rwlock_rdlock(&g_balloon_rwlock);

        hash_node* node_searched = search_hash_node(ve->hash_table, lbn);

        if (!isValidNode(node_searched)) 
        {
            if(!isFreeLFQueueEnough())
            {
                pthread_rwlock_unlock(&g_balloon_rwlock);
                awakeBalloonThread();

                /* If hash table has too much invalid state nodes. 
                TODO: garbage collecting. */
                lbn--;
                continue;
            }

            inode_entry* new_inode = getFreeInodeFromFreeLFQueue(ve, lbn); 

            if(node_searched == nullptr)
            {
                // TODO: Initialize hash node and insert to hash table
                node_searched = new_hash_node(new_inode);
                insert_hash_node(ve->hash_table, node_searched);
            }

            else if(node_searched->is_valid == false)
            {
                // TODO : Reassign inode and make hash node valid, remove hash node from invalid list
                node_searched->inode = new_inode;
                node_searched->is_valid = true;
                remove_list_node(ve->hash_table->invalid_list, node_searched);
            }

        }

        pthread_mutex_lock(&node_searched->inode->lock);

        changeInodeStateToDirty(ve, node_searched);

        int write_bytes = writeDataBlockToNVM(len, offset, ptr, node_searched);

        pthread_mutex_unlock(&node_searched->inode->lock);

        /* Re-inintialize offset and len*/
        offset = 0;
        len -= write_bytes;
        written_bytes += written_bytes;

        pthread_rwlock_unlock(&g_balloon_rwlock);
    }

    return written_bytes;
}

/**
 * Get volume_entry object which has vid.
 * @return a free volume entry */
volume_idx_t
get_volume_entry_idx(
    uint32_t vid) /* !<in: key to find volume_entry */
{
    volume_idx_t idx = search_volume_entry_idx(vid);
    
    if(idx == nvm->max_volume_entry) {
        idx = alloc_volume_entry_idx(vid);
    }
    
    return idx;
}

/**
 * Search volume_entry object from Volume Table.
 * @return found entry */
volume_idx_t
search_volume_entry_idx(
    uint32_t vid)     /* !<in: searching tree with vid */
{
    volume_idx_t idx;

    for(idx = 0; idx < nvm->max_volume_entry; idx++) 
    {
        if(nvm->volume_table[idx].vid == vid)
        {
            return idx;
        }
    }

    return idx;
}

/**
 * Allocate new volume_entry from volume_free_lfqueue
 * @return allocated volume entry */
volume_idx_t
alloc_volume_entry_idx(
    uint32_t vid) /* !<in: given vid to new allocated volume_entry */
{
    /**
     * Get the free ve from volume_free_lfqueue.
     * Multi-writers can ask for each free volume entry and
     * lfqueue dequeue(consume) free volume entry to each writers.
     * Thread asking for deque might be waiting for the queue if it's empty. */
     volume_idx_t idx = volume_free_lfqueue->dequeue();

    /**
     * Setting up acquired free volume entry for use now.
     * Give ve its vid, fd from opening the file.
     * Initialize hash table 
     * the logical blocks of inodes for its volume(file) */
    nvm->volume_table[idx].fd = open(get_filename(vid), O_RDWR| O_CREAT, 0644);
    nvm->volume_table[idx].hash_table = new_hash_table();
    nvm->volume_table[idx].vid = vid;

    return idx;
}

/**
 * Get inode_entry object for lbn.
 * @return inode */
inode_idx_t
get_inode_entry_idx(
    volume_entry* ve, /* !<in: volume that contains inodes we are looking for */
    uint32_t lbn)     /* !<in: find inode which has this lbn */
{
    inode_idx_t idx;
    hash_node* hash_node;
    inode_entry* inode;

    hash_node  = search_hash_node(ve->hash_table, lbn);

    if(hash_node != nullptr) {
        inode = hash_node->inode;
        idx = inode - nvm->inode_table;
        return idx;
    } else {
        idx = alloc_inode_entry_idx(lbn);
        inode = &nvm->inode_table[idx];
        inode->volume = ve;
        hash_node = new_hash_node(inode);
        insert_hash_node(ve->hash_table, hash_node);
    }

    return idx;
}

/**
 * Allocate new volume_entry from free-list of ve.
 * @return inode */
inode_idx_t
alloc_inode_entry_idx(
    uint32_t lbn) /* !<in: lbn to new allocating inode */
{
    /**
     * Get the free inode from free_inode_lfqueue.
     * Multi-writers can ask for each free-inode and
     * lf-queue deque(consume) free-inode to each writers.
     * Thread asking for deque might be waiting for the queue if it's empty. */
    inode_idx_t idx = inode_free_lfqueue->dequeue();

    /**
     * Setting up the acquired inode.
     * Give inode its lbn, and make its state as ALLOCATED */
    nvm->inode_table[idx].lbn = lbn;
    nvm->inode_table[idx].volume = nullptr;
    nvm->inode_table[idx].lock = PTHREAD_MUTEX_INITIALIZER;
    return idx;
}

/**
 * Get filename with argument vid (vid maps filename 1 to 1)
 @return const char* containing filename */
const char*
get_filename(
    uint32_t vid) /* !<in: vid representing its own filename */
{
    std::string filename = "VOL_";
    filename += std::to_string(vid);
    filename += ".txt";

    return filename.c_str();
}

