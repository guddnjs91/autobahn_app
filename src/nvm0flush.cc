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
    
    while(sys_terminate == 0)
    {
        /* Proactively call nvm_flush(). */
        nvm_flush();
    }

    return NULL;
}

/**
Flush one dirty data block from nvm to disk. */
void
nvm_flush(
    void)
{
    /* Pick dirty inodes in dirty inode LFQ. */
    inode_idx_t idx = inode_dirty_lfqueue->dequeue();
    inode_entry* inode = &nvm->inode_table[idx];
    
    /* Flushing one data block is locked with inode lock. */
    pthread_mutex_lock(&inode->lock);

    /* Write one data block from nvm to disk 
    and make inode state CLEAN. */
    write(inode->volume->fd, nvm->datablock_table + nvm->block_size * idx, nvm->block_size);
//    printf("%u Bytes Data flushed to VOL_%u.txt from nvm->inode_table[%u]\n", nvm->block_size, inode->volume->vid, idx);
    inode->state = INODE_STATE_CLEAN;

    /* Unlock inode lock. */
    pthread_mutex_unlock(&inode->lock);
}
