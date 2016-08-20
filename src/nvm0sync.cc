#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "nvm0nvm.h"
#include "nvm0monitor.h"
#include "nvm0lfqueue.h"

//private function declarations
void nvm_sync();

/**
 * Sync thread function ... */
void*
sync_thread_func(
    void* data)
{
//    printf("Sync thread running.....\n");

    while(sys_terminate == 0) {

//        usleep( 10 * 1000 );

        if(inode_sync_lfqueue->get_size() >=
            MIN_SYNC_FREQUENCY + 
            inode_free_lfqueue->get_size() / nvm->max_inode_entry * (nvm->max_inode_entry - MIN_SYNC_FREQUENCY)
        ) {

            nvm_sync();
        }
    }
    
//    printf("Sync thread termintated.....\n");

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

    for (uint32_t i = 0; i < n; i++) {
        inode_idx_t idx = inode_sync_lfqueue->dequeue();
        inode_entry* inode = &nvm->inode_table[idx];

        inode->state = INODE_STATE_CLEAN;
        inode_clean_lfqueue->enqueue(idx);
        monitor.clean++;
        pthread_mutex_unlock(&inode->lock);
    }
}
