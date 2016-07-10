/**
 * nvm0inode.h - header file for iNode */

#ifndef nvm0inode_h
#define nvm0inode_h

/* State of inode */
#define INODE_STATE_FREE        0
#define INODE_STATE_ALLOCATED   1
#define INODE_STATE_DIRTY       2
#define INODE_STATE_SYNCED      3

typedef uint32_t inode_idx_t;

/* Represent one inode object */
struct inode_entry {
    uint32_t                lbn;    // logical block number
    int                     state;  // state of block
    struct volume_entry*    volume; // volume id (implicit filename)
};

extern lfqueue<inode_idx_t>* inode_free_lfqueue;
extern lfqueue<inode_idx_t>* inode_inuse_lfqueue;
extern lfqueue<inode_idx_t>* inode_dirty_lfqueue;

#endif
