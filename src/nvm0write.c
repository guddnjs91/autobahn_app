#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include "nvm0common.h"

extern NVM_metadata* NVM;

/**
 * Atomically write data to nvm 
 - get vte
 - calculate how many inode need
 - get inode and memcpy data block
 - insert inode to sync-list */
void
nvm_atomic_write(
    unsigned int vid, /* !<in: volume ID */
    unsigned int ofs, /* !<in: ??? */ 
    void* ptr,        /* !<in: buffer */
    unsigned int len) /* !<in: size of buffer to be written */
{
    printf("NVM write %u Bytes to VOL%u.txt\n", len, vid);
    
    // get vte with vid
    VT_entry* vte = get_vt_entry(vid);
    
    // calculate total size for writing by offset and length
    unsigned int lbn_start   = ofs / BLOCK_SIZE;
    unsigned int lbn_end     = (ofs + len) / BLOCK_SIZE;
    unsigned int write_bytes = 0;
    unsigned int offset      = ofs % BLOCK_SIZE;
    
    for (unsigned int i = 0; i < lbn_end - lbn_start + 1; i++) {
        // get inode with its lbn
        NVM_inode* inode = get_nvm_inode(vte, lbn_start + i);
        
        // calculate how many bytes to write
        write_bytes = (len > BLOCK_SIZE - offset) ? (BLOCK_SIZE - offset) : len;
        
        // write data to the NVM data block space
        unsigned int idx = get_nvm_inode_idx(inode);
        char* data_dst = NVM->DATA_START + BLOCK_SIZE * idx + offset; 
        memcpy(data_dst, ptr, write_bytes);
        inode->state = INODE_STATE_WRITTEN; // state changed to WRITTEN.
        
        printf("Data Written %d Bytes to %p\n", write_bytes, data_dst);
        
        // innsert written inode to sync list
        insert_sync_inode_list(inode);
        
        // Re-inintialize offset and len
        offset = 0;
        len -= write_bytes;
    }
}

/**
 * Get VT_entry object which has vid.
 * @return a free volume entry */
VT_entry*
get_vt_entry(
    unsigned int vid) /* !<in: key to find VT_entry */
{
    VT_entry* vte;

    // 1. search from vt_tree
    vte = search_vt_entry(NVM->VOL_TABLE_START, vid);
    
    // 2. If search found vte, return it
    if(vte != NULL) {
        return vte;
    } else {
        // 3. If not found, allocate new vte
        vte = alloc_vt_entry(vid);
        // insert this vte to vte_tree structure (later)
    }
    return vte;
}

/**
 * Search VT_entry object from Volume Table.
 * @return found entry */
VT_entry*
search_vt_entry(
    VT_entry* vt_root, /* !<in: root of tree (later) */
    unsigned int vid)  /* !<in: searching tree with vid */
{
    // Just linear search in this code
    // need to search from tree structure later
    int i;
    VT_entry* vte = NVM->VOL_TABLE_START;

    for(i = 0; i < MAX_VT_ENTRY; i++) {
        if(vte->vid == vid) {
            return vte;
        }
        vte++;
    }
    return NULL;
}

/**
 * Allocate new VT_entry from free-list of vte
 * @return allocated volume entry */
VT_entry*
alloc_vt_entry(
    unsigned int vid) /* !<in: given vid to new allocated VT_entry */
{
    /**
     * Get the free vte from free_vte_lfqueue.
     * Multi-writers can ask for each free-vte and
     * lf-queue deque(consume) free-vte to each writers.
     * Thread asking for deque might be waiting for the queue if it's empty. */
    VT_entry* vte = free_vte_lfqueue.dequeue();

    /**
     * Setting up acquired free-vte for use now.
     * Give vte its vid, fd from opening the file.
     * Initialize iroot NULL which would contain
     * the logical blocks of inodes for its volume(file) */
    vte->vid = vid;
    vte->fd = open(get_filename(vid), O_WRONLY| O_CREAT, 0644);
    vte->iroot = NULL;

    return vte;
}

/**
 * Get NVM_inode object for lbn.
 * @return inode */
NVM_inode*
get_nvm_inode(
    VT_entry* vte,    /* !<in: volume that contains inodes we are looking for */
    unsigned int lbn) /* !<in: find inode which has this lbn */
{
    NVM_inode* inode;

    // 1. Search from inode tree
    inode = search_nvm_inode(vte->iroot, lbn);

    // 2. If search found inode, return it
    if(inode != NULL) {
        return inode;
    } else {
        // 3. If not found, allocate new inode
        inode = alloc_nvm_inode(lbn);
        inode->vte = vte;
        vte->iroot = insert_nvm_inode(vte->iroot, inode);
    }
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
    NVM_inode* inode = free_inode_lfqueue.dequeue();

    /**
     * Setting up the acquired inode.
     * Give inode its lbn, and make its state as ALLOCATED */
    inode->lbn = lbn;
    inode->state = INODE_STATE_ALLOCATED;
    return inode;
}

/**
 * Insert inode to sync-list for flushing. */
void
insert_sync_inode_list(
    NVM_inode* inode) /* !<in: inode which would be inserted to sync-inode-list */
{
    /**
     * Insert written inode to dirty_inode_lfqueue.
     * Enqueud inode would be flushed by flush_thread at certain time.
     * There is no waiting inode to be enqueued because 
     * maximum inodes of allocated inode is less then queue size. (Invariant) */
     dirty_inode_lfqueue.enqueue(inode);

     inode->state = INODE_STATE_SYNCED; // state changed to SYNCED.
}

/**
 * Get filename with argument vid (vid maps filename 1 to 1)
 @return char* containing filename */
const char*
get_filename(
    unsigned int vid) /* !<in: vid representing its own filename */
{
    std::string filename = "VOL";
    std::stringstream ss;
    ss << vid;
    filename += ss.str();
    filename += ".txt";

    return filename.c_str();
}

