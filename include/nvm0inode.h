/**
 * nvm0inode.h - header file for iNode */

#ifndef nvm0inode_h
#define nvm0inode_h

#include <pthread.h>

/* State of inode */
#define INODE_STATE_FREE        0
#define INODE_STATE_DIRTY       1
#define INODE_STATE_CLEAN       2

typedef uint32_t inode_idx_t;

/* Represent one inode object */
struct inode_entry {
    uint32_t                lbn;    // logical block number
    int                     state;  // state of block
    struct volume_entry*    volume; // volume id (implicit filename)
    pthread_mutex_t         lock;   // inode lock
};

extern lfqueue<inode_idx_t>* inode_free_lfqueue;
extern lfqueue<inode_idx_t>* inode_dirty_lfqueue;

#endif
