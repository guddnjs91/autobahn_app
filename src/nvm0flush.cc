#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "nvm0nvm.h"
#include "nvm0lfqueue.h"
#include "nvm0monitor.h"

//private function declarations
void nvm_flush(int dirty_Q_idx);

/**
 * Flush thread function proactively runs and flushes dirty inodes. */
void*
flush_thread_func(
    void* data)
{
//    printf("Flush thread running.....\n");

    while(sys_terminate == 0) {
        
        usleep(10 * 1000);

#if testing
        volume_idx_t idx = flush_ready_idx_lfqueue->dequeue();
        
        while(inode_dirty_lfqueue[idx]->get_size() > FLUSH_LWM && sys_terminate == 0) {
            nvm_flush(idx % MAX_VOLUME_ENTRY);
        }

        flush_ready_idx_lfqueue->enqueue(idx % MAX_VOLUME_ENTRY);
#else
        int idx = *((int *)data);
        while(inode_dirty_lfqueue[idx]->get_size() > FLUSH_LWM && sys_terminate == 0) {
            nvm_flush(idx);
        }
#endif

    }
    
//    printf("Flush thread termintated.....\n");

    return NULL;
}

/**
 * Flush a batch of dirty data block from NVM to a permenant storage. */
void
nvm_flush(int dirty_Q_idx)
{
    inode_idx_t idx = inode_dirty_lfqueue[dirty_Q_idx]->dequeue();
    inode_entry* inode = &nvm->inode_table[idx];
    
    pthread_mutex_lock(&inode->lock);

    //Flush one data block
    lseek(inode->volume->fd, nvm->block_size * inode->lbn, SEEK_SET);
    write(inode->volume->fd, nvm->datablock_table + nvm->block_size * idx, nvm->block_size);

    inode->state = INODE_STATE_SYNC;
    inode_sync_lfqueue->enqueue(idx);
    monitor.sync++;
}
