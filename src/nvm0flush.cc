#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "nvm0common.h"

/**
 * Flush thread function proactively runs and flushes dirty inodes. */
void*
flush_thread_func(
    void* data)
{
    printf("Flush thread running.....\n");

    while(sys_terminate == 0) {

        pthread_mutex_lock(&g_flush_mutex);
        pthread_cond_wait(&g_flush_cond, &g_flush_mutex);
        pthread_mutex_unlock(&g_flush_mutex);

        while(!inode_dirty_lfqueue->isQuiteEmpty()) {
            nvm_flush();
        }
    }
    
    printf("Flush thread termintated.....\n");

    return NULL;
}

/**
 * Flush a batch of dirty data block from NVM to a permenant storage. */
void
nvm_flush(
    void)
{
    // inode_dirty_lfqueue->monitor();
    inode_idx_t indexes[FLUSH_BATCH_SIZE];
    int i, n_flushed = 0;

    for(i = 0; i < FLUSH_BATCH_SIZE; i++) {
        
        if(inode_dirty_lfqueue->is_empty()) {
            break;
        }

        indexes[n_flushed++] = inode_dirty_lfqueue->dequeue();
        inode_idx_t idx = indexes[i];
        inode_entry* inode = &nvm->inode_table[idx];
        
        pthread_mutex_lock(&inode->lock);

        //Flush one data block
        lseek(inode->volume->fd, nvm->block_size * inode->lbn, SEEK_SET);
        write(inode->volume->fd, nvm->datablock_table + nvm->block_size * idx, nvm->block_size);
    }

    sync();
    sync();

    for (i = 0; i < n_flushed; i++) {
        inode_entry* inode = &nvm->inode_table[indexes[i]];
        inode->state = INODE_STATE_CLEAN;

        pthread_mutex_unlock(&inode->lock);
    }
}
