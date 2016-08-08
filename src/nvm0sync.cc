#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "nvm0common.h"

//private function declarations
void nvm_sync();

/**
 * Sync thread function ... */
void*
sync_thread_func(
    void* data)
{
    printf("Sync thread running.....\n");

    while(sys_terminate == 0) {

        usleep( 10 * 1000 );

        if(inode_sync_lfqueue->get_size() >=
            MIN_SYNC_FREQUENCY + 
            inode_free_lfqueue->get_size() / nvm->max_inode_entry * (nvm->max_inode_entry - MIN_SYNC_FREQUENCY)
        ) {

            nvm_sync();
        }
    }
    
    printf("Sync thread termintated.....\n");

    return NULL;
}

/**
 * Sync ... */
void
nvm_sync(
    void)
{
    uint32_t n = inode_sync_lfqueue->get_size();

    sync();
    sync();

    for (uint32_t i = 0; i < n; i++) {
        inode_idx_t idx = inode_sync_lfqueue->dequeue();
        inode_entry* inode = &nvm->inode_table[idx];

        inode->state = INODE_STATE_CLEAN;

        // TODO: 
        // 1. search hash node with (inode->volum_entry->hash_table, inode->lbn)
        // 2. push_back into global clean list
        
        pthread_mutex_unlock(&inode->lock);
    }
}
