#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "nvm0common.h"

//private function declarations
const char* get_filename(uint32_t vid);

/**
Write out len bytes data pointed by ptr to nvm structure.
After writing out to nvm, data blocks are enqueued to dirty LFQ.
@return the byte size of data written to nvm */
size_t
nvm_write(
    uint32_t vid,       /* !<in: volume ID */
    off_t    ofs,       /* !<in: volume offset */ 
    const char* ptr,    /* !<in: buffer */
    size_t   len)       /* !<in: size of buffer to be written */
{
    /* Get the volume entry index from the nvm volume table.
    The volume entry contains the hash structure, representing one file. */
    volume_idx_t v_idx = get_volume_entry_idx(vid);
    volume_entry* ve = &nvm->volume_table[v_idx];

    /* Calculate how many blocks needed for writing */
    uint32_t lbn_start   = ofs / nvm->block_size;
    uint32_t lbn_end     = (ofs + len) / nvm->block_size;
    if((ofs + len) % nvm->block_size == 0) {
        lbn_end--; // To avoid dummy data block write
    }
    uint32_t offset      = ofs % nvm->block_size;
    size_t written_bytes = 0;

    /* Each loop write one data block to nvm */
    for(uint32_t lbn = lbn_start; lbn <= lbn_end; lbn++) {
        
        pthread_rwlock_rdlock(&g_balloon_rwlock);

        hash_node* node_searched = search_hash_node(ve->hash_table, lbn);
        
        /* If there is no hash node with lbn found or is invalid,
        get free inode from free inode LFQ. */
        if(node_searchd == nullptr || node_searched->valid == HASH_NODE_INVALID) {
            /* If the ratio of free inode LFQ is below 10 %, 
            wake up balloon thread for reclaiming inodes */
            if(inode_free_lfqueue->get_size() < 1000) //0.1 * nvm->max_inode_entry)
            {
                pthread_rwlock_unlock(&g_balloon_rwlock);
                lbn--;
                pthread_mutex_lock(&g_balloon_mutex);
                pthread_cond_signal(&g_balloon_cond);
                pthread_mutex_unlock(&g_balloon_mutex);
                /* If hash table has too much invalid state nodes. 
                TODO: garbage collecting. */
                continue;
            }

            /* Get inode from inode_free_lfqueue. */
            inode_idx_t idx = alloc_inode_entry_idx(lbn);
            inode_entry* inode = &nvm->inode_table[idx];
            inode->volume = ve;

            if(node_searched == nullptr)
            {
                // TODO: Initialize hash node and insert to hash table
                // node_searched = init_hash_node(inode);
                // insert_hash_node(ve->tree, node);
            }
            else if(tnode->valid == TREE_INVALID)
            {
                // TODO : Reassign inode and make hash node valid, remove hash node from invalid list
                // tnode->inode = inode;
                // tnode->valid = TREE_VALID;
                // ve->tree->count_invalid--;
            }
        }

        /* Protect from flush_thread by inode lock */
        pthread_mutex_lock(&searhced_node->inode->lock);

        /* Change the state of inode to DIRTY */
        int old_state = searched_node->inode->state;
        searched_node->inode->state = INODE_STATE_DIRTY;

        /* Write out one data block to the NVM */
        uint32_t write_bytes = (len > nvm->block_size - offset) ? (nvm->block_size - offset) : len;
        inode_idx_t idx = (inode_idx_t)((char *)searched_node->inode - (char *)nvm->inode_table)/sizeof(inode_entry);
        char* data_dst = nvm->datablock_table + nvm->block_size * idx + offset; 
        memcpy(data_dst, ptr, write_bytes);
        //TODO:need cache line write guarantee


        //TODO: state clean? 
        //      remove from clean list
        //      enqueue dirty queue
        if(old_state != INODE_STATE_DIRTY) {
            /* Insert written inode to dirty_inode_lfqueue.
            Enqueued inode would be flushed by flush_thread at certain time. */
            inode_dirty_lfqueue->enqueue(idx);
        }


        /* Unlock inode lock */
        pthread_mutex_unlock(&searched_node->inode->lock);

        /* Re-inintialize offset and len*/
        offset = 0;
        len -= write_bytes;
        written_bytes += write_bytes;

        /* Unlock read-lock */
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
    volume_idx_t idx;

    idx = search_volume_entry_idx(vid);
    
    // 2. If search found ve, return it
    if(idx == nvm->max_volume_entry) {
        // 3. If not found, allocate new ve
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

    // Just linear search in this code
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
     * TODO: Initialize hash table 
     * the logical blocks of inodes for its volume(file) */
    nvm->volume_table[idx].fd = open(get_filename(vid), O_RDWR| O_CREAT, 0644);
   
    // nvm->volume_table[idx].tree = (struct tree_root*)malloc(sizeof(struct tree_root));
    // nvm->volume_table[idx].tree->root = nullptr;
    // nvm->volume_table[idx].tree->count_total = 0;
    // nvm->volume_table[idx].tree->count_invalid = 0;
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
    hash_node* hnode;
    inode_entry* inode;

    // 1. Search from inode hash 
    hnode  = search_hash_node(ve->hash_table, lbn);

    // 2. If search found inode, return it
    if(hnode != nullptr) {
        inode = hnode->inode;
        idx = inode - nvm->inode_table;
        return idx;
    } else {
        // 3. If not found, allocate new inode
        idx = alloc_inode_entry_idx(lbn);
        inode = &nvm->inode_table[idx];
        inode->volume = ve;
        hnode = init_hash_node(inode);
        insert_hash_node(ve->hash_table, hnode);
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

