#ifndef _NVM_INODE_H_
#define _NVM_INODE_H_

/**
 * nvm0inode.h - header file for iNode */

/* State of inode */
#define INODE_STATE_FREE        0
#define INODE_STATE_ALLOCATED   1
#define INODE_STATE_DIRTY       2
#define INODE_STATE_SYNCED      3

/* Represent one inode object */
typedef struct _nvm_inode {
    
    unsigned int        lbn;      // logical block number
    int                 state;    // state of block
    struct _vt_entry*   vte;      // volume id (implicit filename)
    
} NVM_inode;

lfqueue<_nvm_inode*>*  INODE_FREE_LFQUEUE;
lfqueue<_nvm_inode*>*  INODE_DIRTY_LFQUEUE;
lfqueue<_nvm_inode*>*  INODE_SYNCED_LFQUEUE;

#endif
