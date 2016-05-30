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
    
    //AVL tree fields
    struct _nvm_inode*  left;
    struct _nvm_inode*  right;
    int                 height;
    
} NVM_inode;

lfqueue<_nvm_inode*>*  INODE_FREE_LFQUEUE;
lfqueue<_nvm_inode*>*  INODE_DIRTY_LFQUEUE;
lfqueue<_nvm_inode*>*  INODE_SYNCED_LFQUEUE;

