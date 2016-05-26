#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include "nvm0common.h"

extern NVM_metadata* NVM;

/* Atomically write data to nvm 
   - get vte
   - calculate how many inode need
   - get inode and memcpy data block
   - insert inode to sync-list
*/
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
    
    for (unsigned int i = 0; i < lbn_end - lbn_start + 1; i++)
    {
        // get inode with its lbn
        NVM_inode* inode = get_nvm_inode(vte, lbn_start + i);
        
        // need lock?
        
        // calculate how many bytes to write
        write_bytes = (len > BLOCK_SIZE - offset) ? (BLOCK_SIZE - offset) : len;
        
        // write data to the NVM data block space
        unsigned int idx = get_nvm_inode_idx(inode);
        char* data_dst = NVM->DATA_START + BLOCK_SIZE * idx + offset; 
        memcpy(data_dst, ptr, write_bytes);
        inode->state = INODE_STATE_WRITTEN;
        
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
    unsigned int vid)
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
    VT_entry* vt_root,
    unsigned int vid)
{
    // Just linear search in this code
    // need to search from tree structure later
    int i;
    VT_entry* vte = NVM->VOL_TABLE_START;

    for(i = 0; i < MAX_VT_ENTRY; i++) {
        if(vte->vid == vid) {
            return vte;
            vte++;
    }
    return NULL;
}

/**
 * Allocate new VT_entry from free-list of vte
 * @return allocated volume entry */
VT_entry*
alloc_vt_entry(
    unsigned int vid)
{
    VT_entry* vte;

    // Check if free-list is empty
    if(NVM->FREE_VTE_LIST_HEAD != NVM->FREE_VTE_LIST_TAIL)
    {
        vte = __sync_lock_test_and_set(
                &NVM->FREE_VTE_LIST_HEAD, NVM->VOL_TABLE_START + NVM->FREE_VTE_LIST_HEAD->next);
    }
    else
    {
        // wait until vt_entry freed (later)
    }

    vte->vid = vid;
        vte->fd = open(get_filename(vid), O_WRONLY| O_CREAT, 0644);
    vte->next = MAX_VT_ENTRY;   // meaning next null
    vte->iroot = NULL;

    return vte;
}

/**
 * Get NVM_inode object for lbn.
 * @return inode */
NVM_inode*
get_nvm_inode(
    VT_entry* vte,
    unsigned int lbn)
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
    unsigned int lbn)
{
    NVM_inode* inode;

    // Check if free-list is empty
    if(NVM->FREE_INODE_LIST_HEAD != NVM->FREE_INODE_LIST_TAIL) {
        inode = __sync_lock_test_and_set(&NVM->FREE_INODE_LIST_HEAD,
                                         NVM->INODE_START +
                                         NVM->FREE_INODE_LIST_HEAD->f_next);
    } else {
        // wait until vt_entry freed (later)
    }

    inode->lbn = lbn;
    inode->f_next = MAX_NVM_INODE;  // meaning NULL
    inode->s_prev = NULL;           // meaning NULL
    inode->s_next = NULL;           // meaning NULL
    inode->state = INODE_STATE_ALLOCATED;
    return inode;
}

/**
 * Insert inode to sync-list for flushing. */
void
insert_sync_inode_list(
    NVM_inode* inode)
{
    if(NVM->SYNC_INODE_LIST_HEAD == NULL) {
        NVM->SYNC_INODE_LIST_HEAD = inode;
        NVM->SYNC_INODE_LIST_TAIL = inode;
    } else {
        inode->s_prev
            = __sync_lock_test_and_set(&NVM->SYNC_INODE_LIST_TAIL, inode);
        inode->s_prev->s_next = inode;
    }
}

const char*
get_filename(
    unsigned int vid)
{
    std::string filename = "VOL";
    std::stringstream ss;
    ss << vid;
    filename += ss.str();
    filename += ".txt";

    return filename.c_str();
}

