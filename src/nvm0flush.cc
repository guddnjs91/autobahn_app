#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/uio.h>
#include "nvm0nvm.h"
#include "nvm0inode.h"
#include "nvm0monitor.h"

//private function declarations
void nvm_flush(int dirty_queue_idx);

/**
 * Flush thread function proactively runs and flushes dirty inodes. */
void*
flush_thread_func(
    void* data)
{
//    printf("Flush thread running.....\n");

    while(sys_terminate == 0) {
        
        usleep(10 * 1000);

        if(volume_inuse_lfqueue->get_size() == 0) {
            continue;
        }

        volume_idx_t idx = volume_inuse_lfqueue->dequeue();
        
        while(inode_dirty_lfqueue[idx]->get_size() > 1 && sys_terminate == 0) {
            nvm_flush(idx);
        }

        volume_inuse_lfqueue->enqueue(idx);

    }
    
//    printf("Flush thread termintated.....\n");

    return NULL;
}

/**
 * Flush a batch of dirty data block from NVM to a permenant storage. */
void
nvm_flush(int dirty_queue_idx)
{
#if testing
    uint64_t num_blocks, i;
    struct iovec* iov;
    inode_idx_t* indexes;

    //number of blocks to write
    num_blocks = inode_dirty_lfqueue[dirty_queue_idx]->get_size()-1;

    //1. take them out from dirty_queue
    //2. lock them
    indexes = (inode_idx_t*) malloc( num_blocks * sizeof(inode_idx_t));
    for(i = 0; i < num_blocks; i++) {

        indexes[i] = inode_dirty_lfqueue[dirty_queue_idx]->dequeue();

        inode_entry* inode = &nvm->inode_table[indexes[i]];
        pthread_mutex_lock(&inode->lock);
    }

    //1. look ahead
    //2. batch write (writev)
    i = 0;
    while( i < num_blocks ) {
        
        //look ahead
        int num_ahead = 0;
        uint32_t curr_lbn = (&nvm->inode_table[indexes[i]])->lbn;
        for(uint64_t j = i+1; j < num_blocks; j++) {

            uint32_t succ_lbn = (&nvm->inode_table[indexes[j]])->lbn;

            if(succ_lbn == curr_lbn+1) {
                num_ahead++;
                curr_lbn = succ_lbn;
            } else {
                break;
            }
        }

        //write
        iov = (struct iovec*) malloc( (1000) * sizeof(struct iovec));
        int num_left = num_ahead+1;
        for(int j = 0; j < (num_ahead+1 + (1000-1) ) / 1000; j++) {
            int num_write = (num_left >= 1000) ? 1000 : num_left;
            num_left -= num_write;

            for(int k = 0; k < num_write; k++) {
                iov[k].iov_base = nvm->datablock_table + nvm->block_size * indexes[i+k];
                iov[k].iov_len  = nvm->block_size;
            }
                
            inode_idx_t  start_idx   = indexes[i];
            inode_entry* start_inode = &nvm->inode_table[start_idx];
    
            lseek(start_inode->volume->fd, (off_t) nvm->block_size * start_inode->lbn, SEEK_SET);
            ssize_t bytes_written = writev(start_inode->volume->fd, iov, num_write);
            if(bytes_written != nvm->block_size * num_write) {
                printf("ERROR: writev failed to write requested number of bytes! bytes_written: %ld, bytes_requested: %d\n", bytes_written, nvm->block_size * num_write);
                exit(0);
            }

            for(int k = 0; k < num_write; k++) {
                inode_idx_t idx = indexes[i++];
                inode_entry* inode = &nvm->inode_table[idx];
    
                inode->state = INODE_STATE_SYNC;
                inode_sync_lfqueue->enqueue(idx);
                monitor.sync++;
            }
        }

//        iov = (struct iovec*) malloc( (num_ahead +1) * sizeof(struct iovec));
//        for(int j = 0; j < num_ahead+1; j++) {
//            iov[j].iov_base = nvm->datablock_table + nvm->block_size * indexes[i+j];
//            iov[j].iov_len  = nvm->block_size;
//        }
//            
//        inode_idx_t  start_idx   = indexes[i];
//        inode_entry* start_inode = &nvm->inode_table[start_idx];
//
//        lseek(start_inode->volume->fd, (off_t) nvm->block_size * start_inode->lbn, SEEK_SET);
//        writev(start_inode->volume->fd, iov, num_ahead+1);

        //state change
//        for(int j = 0; j < num_ahead+1; j++) {
//            inode_idx_t idx = indexes[i++];
//            inode_entry* inode = &nvm->inode_table[idx];
//
//            inode->state = INODE_STATE_SYNC;
//            inode_sync_lfqueue->enqueue(idx);
//            monitor.sync++;
//        }
        free(iov);
    }

    free(indexes);
#else
    inode_idx_t idx = inode_dirty_lfqueue[dirty_queue_idx]->dequeue();
    inode_entry* inode = &nvm->inode_table[idx];
    
    pthread_mutex_lock(&inode->lock);

    //Flush one data block
    lseek(inode->volume->fd, (off_t) nvm->block_size * inode->lbn, SEEK_SET);
    write(inode->volume->fd, nvm->datablock_table + nvm->block_size * idx, nvm->block_size);

    inode->state = INODE_STATE_SYNC;
    inode_sync_lfqueue->enqueue(idx);
    monitor.sync++;
#endif
}
