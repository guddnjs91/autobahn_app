#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "nvm0common.h"
#include "nvm0monitor.h"
//private function declarations
void nvm_flush(int);

/**
 * Flush thread function proactively runs and flushes dirty inodes. */
void*
flush_thread_func(
    void* data)
{
    printf("Flush thread running.....\n");

    int flush_idx = *((int *)data);
    while(sys_terminate == 0) {
        
        usleep(10 * 1000);
        
        while(inode_dirty_lfqueue[flush_idx]->get_size() > FLUSH_LWM && sys_terminate == 0) {
            nvm_flush(flush_idx);
        }
    }
    
    printf("Flush thread termintated.....\n");

    return NULL;
}

/**
 * Flush a batch of dirty data block from NVM to a permenant storage. */
void
nvm_flush(
    int flush_idx)
{
    // inode_dirty_lfqueue->monitor();
    inode_idx_t idx = inode_dirty_lfqueue[flush_idx]->dequeue();
    inode_entry* inode = &nvm->inode_table[idx];
    
    pthread_mutex_lock(&inode->lock);

    //Flush one data block
    lseek(inode->volume->fd, (off_t)nvm->block_size * inode->lbn, SEEK_SET);
    write(inode->volume->fd, nvm->datablock_table + nvm->block_size * idx, nvm->block_size);

    inode->state = INODE_STATE_SYNC;
    inode_sync_lfqueue->enqueue(idx);
    monitor.sync++;
}
