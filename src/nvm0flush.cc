#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "nvm0common.h"

/**
Flush thread function proactively runs and flushes dirty inodes. */
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
Flush one dirty data block from nvm to disk. */
void
nvm_flush(
    void)
{
    // inode_dirty_lfqueue->monitor();
    inode_idx_t indexes[32];
    int i;

    for(i = 0; i < 32; i++) {
        
        /* Pick dirty inodes in dirty inode LFQ. */
        indexes[i] = inode_dirty_lfqueue->dequeue();
        inode_idx_t idx = indexes[i];
        inode_entry* inode = &nvm->inode_table[idx];
        
        /* Flushing one data block is locked with inode lock. */
        pthread_mutex_lock(&inode->lock);

        /* Write one data block from nvm to disk 
        and make inode state CLEAN. */
        lseek(inode->volume->fd, nvm->block_size * inode->lbn, SEEK_SET);
    //    printf("VOL_%u.txt : offset %u => ", inode->volume->vid, nvm->block_size * inode->lbn);
        write(inode->volume->fd, nvm->datablock_table + nvm->block_size * idx, nvm->block_size);
    }

    sync();
    sync();

    for (i = 0; i < 32; i++) {
        inode_entry* inode = &nvm->inode_table[indexes[i]];
        //printf("%u Bytes Data flushed from nvm->inode_table[%u] ", nvm->block_size, idx);
        inode->state = INODE_STATE_CLEAN;

        /* Unlock inode lock. */
        pthread_mutex_unlock(&inode->lock);
    }
}
