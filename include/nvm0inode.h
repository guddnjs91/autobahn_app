#pragma once

#include <cstdint>
#include <pthread.h>

/* State of inode */
#define INODE_STATE_FREE    0
#define INODE_STATE_DIRTY   1
#define INODE_STATE_SYNC    2
#define INODE_STATE_CLEAN   3

typedef uint32_t inode_idx_t;

/* Represent one inode object */
struct inode_entry {
    uint32_t                lbn;    // logical block number
    int                     state;  // state of block
    struct volume_entry*    volume; // volume id (implicit filename)
    pthread_mutex_t         lock;   // inode lock
};
