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

        struct hash_node* hash_node = search_hash_node(inode->volume->hash_table, inode->lbn);
        if(hash_node == nullptr){
            printf("fail!\n");
            printf("hash_table: %p, lbn : %d\n", inode->volume->hash_table, inode->lbn);
        }

        inode_clean_lfqueue->enqueue(idx);
         
        // push_back_list_node(inode_clean_list, hash_node);
         
        pthread_mutex_unlock(&inode->lock);
    }
}
