#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include "nvm0common.h"

//local function declarations
volume_entry* get_volume_entry(uint32_t);

/**
 * Atomically write data to nvm 
 - get vte
 - calculate how many inode need
 - get inode and memcpy data block
 - insert inode to sync-list */
void
nvm_write(
    uint32_t vid,   /* !<in: volume ID */
    off_t ofs,      /* !<in: ??? */ 
    void* ptr,      /* !<in: buffer */
    size_t len)     /* !<in: size of buffer to be written */
{
    volume_entry* volume;
    inode_entry* inode;
    uint32_t lbn_begin, lbn_end, lbn;

    // 1. get volume entry with vid
    volume = get_volume_entry(vid);
    
    // calculate which lbns to write with offset and length
    lbn_start   = ofs / BLOCK_SIZE;
    lbn_end     = (ofs + len) / BLOCK_SIZE;

    write_bytes = 0;
    offset      = ofs % BLOCK_SIZE;
    
    /* CONCURRENCY ALERT: THIS PART NEEDS TO BE HANDLED CAREFULLY */
    // 3 Critical Steps here dealing with inode
    // step 1: finding/getting inode
    // step 2: critical section (writing data to nvm data block)
    // step 3. pushing inode to the next stage (make it dirty)
    for(lbn = lbn_begin; lbn <= lbn_end; lbn++) {
        // STEP 1
        // get inode entry with lbn
        inode = get_inode_entry(volume, lbn);
        
        // STEP 2
        // calculate how many bytes to write
        write_bytes = (len > BLOCK_SIZE - offset) ? (BLOCK_SIZE - offset) : len;
        
        // write data to the NVM data block space
        unsigned int idx = get_nvm_inode_idx(inode);
        char* data_dst = NVM->DATA_START + BLOCK_SIZE * idx + offset; 
        memcpy(data_dst, ptr, write_bytes);
        inode->state = INODE_STATE_DIRTY; // state changed to DIRTY.
        
        // STEP 3
        // Insert written inode to dirty_inode_lfqueue.
        // Enqueud inode would be flushed by flush_thread at certain time.
        INODE_DIRTY_LFQUEUE->enqueue(inode);
        
        // Re-inintialize offset and len
        offset = 0;
        len -= write_bytes;
    }
}

/**
 * Find/create a corresponding volume_entry with given volume id.
 * @return volume entry for the vid */
volume_entry*
get_volume_entry(
    uint32_t vid)   /* !<in: key to find VT_entry */
{
    volume_entry* volume;
    uint32_t i;

    // 1. Search volume_entry in Volume Table
    //    If found, returns the entry
    //    If not found, continue to step 2
    for(i = 0; i < nvm->max_volume_entry; i++) {
        volume = &nvm->volume_table[i];
        /* fd = -1 means a volume is not assigned to the entry */
        if(volume->fd != -1 && volume->vid == vid) {
            return volume;
        }
    }
    
    // 2. Allocate a free volume_entry for the new volume
    i = volume_free_queue->dequeue();
    volume = &nvm->volume_table[i];
    volume->vid = vid;
    volume->fd = open( "VOL_" + to_string(vid) + ".txt", O_RDWR | O_CREAT, 0666);
    volume->root = NULL;
    return volume;
}

/**
 * Get NVM_inode object for lbn.
 * @return inode */
inode_entry*
get_inode_entry(
    volume_entry* volume,   /* !<in: volume that contains inodes we are looking for */
    uint32_t lbn)           /* !<in: find inode which has this lbn */
{
    inode_entry* inode;
    uint32_t i;

    // 1. Search inode_entry in inode tree
    if(inode = search_nvm_inode(vte->iroot, lbn)) {
        return inode;
    }

    // 2. Allocate a free inode_entry for the new inode
    i = inode_free_queue->dequeue();
    inode = &nvm->volume_table[i];
    inode->lbn = lbn;
    inode->volume = volume;
    inode->state = 
    vte->iroot = insert_nvm_inode(vte->iroot, inode);
    return inode;
}

/**
 * Allocate new VT_entry from free-list of vte.
 * @return inode */
NVM_inode*
alloc_nvm_inode(
    unsigned int lbn) /* !<in: lbn to new allocating inode */
{
    /**
     * Get the free inode from free_inode_lfqueue.
     * Multi-writers can ask for each free-inode and
     * lf-queue deque(consume) free-inode to each writers.
     * Thread asking for deque might be waiting for the queue if it's empty. */
    NVM_inode* inode = INODE_FREE_LFQUEUE->dequeue();

    /**
     * Setting up the acquired inode.
     * Give inode its lbn, and make its state as ALLOCATED */
    inode->lbn = lbn;
    inode->state = INODE_STATE_ALLOCATED;
    return inode;
}
