#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <atomic>
#include "nvm0nvm.h"
#include "nvm0monitor.h"
#include "nvm0inode.h"

//private function declarations
void nvm_sync(int index);

/**
 * Sync thread function ... */
void*
sync_thread_func(
    void* data)
{
    int index = *(int *)data;

    while(sys_terminate == 0) {

        usleep( 10 * 1000 );

        uint64_t free_idx = free_enqueue_idx.load();

        if(inode_sync_lfqueue[index]->get_size() >= 1
//            MIN_SYNC_FREQUENCY + 
//            inode_free_lfqueue[free_idx % MAX_NUM_FREE]->get_size() / (nvm->max_inode_entry / MAX_NUM_FREE)  * (nvm->max_inode_entry / MAX_NUM_SYNCER - MIN_SYNC_FREQUENCY)
        ) {
            nvm_sync(index);
        }
    }
    
    return NULL;
}

/**
 * Sync ... */
void
nvm_sync(
    int index)
{
    uint_fast64_t clean_idx;
    uint32_t i;

    uint32_t n = inode_sync_lfqueue[index]->get_size();

    if(likely(SYNC_OPTION)) {
        sync();
    }

    for (i = 0; i < n; i++) {
        inode_idx_t idx = inode_sync_lfqueue[index]->dequeue();
        inode_entry* inode = &nvm->inode_table[idx];

        inode->state = INODE_STATE_CLEAN;
        clean_idx = clean_queue_idx.fetch_add(1);
        inode_clean_lfqueue[clean_idx%MAX_NUM_BALLOON]->enqueue(idx);
        monitor.clean++;
        pthread_mutex_unlock(&inode->lock);
    }
}
