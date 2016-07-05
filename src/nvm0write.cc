#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include "nvm0common.h"

extern struct nvm_metadata* nvm;
extern lfqueue<uint32_t>* volume_free_lfqueue;
extern lfqueue<uint32_t>* volume_inuse_lfqueue;
extern lfqueue<uint32_t>* inode_free_lfqueue;
extern lfqueue<uint32_t>* inode_dirty_lfqueue;

/**
 * Atomically write data to nvm 
 - get ve
 - calculate how many inode need
 - get inode and memcpy data block
 - insert inode to sync-list */
void
nvm_atomic_write(
    uint32_t vid, /* !<in: volume ID */
    off_t    ofs, /* !<in: volume offset */ 
    void*    ptr, /* !<in: buffer */
    size_t   len) /* !<in: size of buffer to be written */
{
    printf("NVM write %u Bytes to VOL%u.txt\n", len, vid);
    
    // get ve with vid
    volume_entry* ve = get_volume_entry(vid);
    
    // calculate total size for writing by offset and length
    uint32_t lbn_start   = ofs / BLOCK_SIZE;
    uint32_t lbn_end     = (ofs + len) / BLOCK_SIZE;
    uint32_t write_bytes = 0;
    uint32_t offset      = ofs % BLOCK_SIZE;
    
    for (uint32_t i = 0; i < lbn_end - lbn_start + 1; i++) {
        // get inode with its lbn
        inode_entry* inode = get_inode_entry(ve, lbn_start + i);
        
        // calculate how many bytes to write
        write_bytes = (len > BLOCK_SIZE - offset) ? (BLOCK_SIZE - offset) : len;
        
        // write data to the NVM data block space
        uint32_t idx = get_inode_entry_idx(inode);
        char* data_dst = NVM->DATA_START + BLOCK_SIZE * idx + offset; 
        memcpy(data_dst, ptr, write_bytes);
        inode->state = INODE_STATE_DIRTY; // state changed to DIRTY.
        
        printf("Data Written %d Bytes to %p\n", write_bytes, data_dst);
        
        // Insert written inode to dirty_inode_lfqueue.
        // Enqueud inode would be flushed by flush_thread at certain time.
        inode_dirty_lfqueue->enqueue(inode);
        
        // Re-inintialize offset and len
        offset = 0;
        len -= write_bytes;
    }
}

/**
 * Get volume_entry object which has vid.
 * @return a free volume entry */
volume_entry*
get_volume_entry(
    uint32_t vid) /* !<in: key to find volume_entry */
{
    volume_entry* ve;

    // 1. search from vt_tree
    ve = search_volume_entry(nvm->volume_table, vid);
    
    // 2. If search found ve, return it
    if(ve != NULL) {
        return ve;
    } else {
        // 3. If not found, allocate new ve
        ve = alloc_volume_entry(vid);
        // insert this ve to ve_tree structure (later)
    }
    return ve;
}

/**
 * Search volume_entry object from Volume Table.
 * @return found entry */
volume_entry*
search_volume_entry(
    volume_entry* vt_base, /* !<in: volume table base address */
    uint32_t      vid)     /* !<in: searching tree with vid */
{
    // Just linear search in this code
    int i;
    volume_entry* ve = nvm->volume_table;

    for(i = 0; i < nvm->max_volume_entry; i++) {
        if(ve->vid == vid) {
            return ve;
        }
        ve++;
    }
    return NULL;
}

/**
 * Allocate new volume_entry from volume_free_lfqueue
 * @return allocated volume entry */
volume_entry*
alloc_volume_entry(
    uint32_t vid) /* !<in: given vid to new allocated volume_entry */
{
    /**
     * Get the free ve from volume_free_lfqueue.
     * Multi-writers can ask for each free volume entry and
     * lfqueue dequeue(consume) free volume entry to each writers.
     * Thread asking for deque might be waiting for the queue if it's empty. */
	 volume_entry* ve = volume_free_lfqueue->dequeue();

    /**
     * Setting up acquired free volume entry for use now.
     * Give ve its vid, fd from opening the file.
     * Initialize root NULL which would contain
     * the logical blocks of inodes for its volume(file) */
    ve->vid = vid;
    ve->fd = open(get_filename(vid), O_WRONLY| O_CREAT, 0644);
    ve->root = NULL;

    return ve;
}

/**
 * Get inode_entry object for lbn.
 * @return inode */
inode_entry*
get_inode_entry(
    volume_entry* ve,    /* !<in: volume that contains inodes we are looking for */
    uint32_t lbn) /* !<in: find inode which has this lbn */
{
    inode_entry* inode;

    // 1. Search from inode tree
    inode = search_inode_entry(ve->root, lbn);

    // 2. If search found inode, return it
    if(inode != NULL) {
        return inode;
    } else {
        // 3. If not found, allocate new inode
        inode = alloc_inode_entry(lbn);
        inode->volume = ve;
        ve->root = insert_inode_entry(ve->root, inode);
    }
    return inode;
}

/**
 * Allocate new volume_entry from free-list of ve.
 * @return inode */
inode_entry*
alloc_inode_entry(
    uint32_t lbn) /* !<in: lbn to new allocating inode */
{
    /**
     * Get the free inode from free_inode_lfqueue.
     * Multi-writers can ask for each free-inode and
     * lf-queue deque(consume) free-inode to each writers.
     * Thread asking for deque might be waiting for the queue if it's empty. */
    inode_entry* inode = inode_free_lfqueue->dequeue();

    /**
     * Setting up the acquired inode.
     * Give inode its lbn, and make its state as ALLOCATED */
    inode->lbn = lbn;
    inode->state = INODE_STATE_ALLOCATED;
    return inode;
}

/**
 * Get filename with argument vid (vid maps filename 1 to 1)
 @return char* containing filename */
const char*
get_filename(
    uint32_t vid) /* !<in: vid representing its own filename */
{
    std::string filename = "VOL";
    std::stringstream ss;
    ss << vid;
    filename += ss.str();
    filename += ".txt";

    return filename.c_str();
}

