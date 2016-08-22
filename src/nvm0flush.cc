#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/uio.h>
#include "nvm0nvm.h"
#include "nvm0inode.h"
#include "nvm0monitor.h"

//private function declarations
void nvm_flush(volume_idx_t v_idx, inode_idx_t* i_idxs, struct iovec* iov);

/**
 * Flush thread function proactively runs and flushes dirty inodes. */
void*
flush_thread_func(
    void* data)
{
    //init
    volume_idx_t    v_idx;
    inode_idx_t*    i_idxs = (inode_idx_t*)  malloc( FLUSH_BATCH_SIZE * sizeof(inode_idx_t));
    struct iovec*   iov     = (struct iovec*) malloc( FLUSH_BATCH_SIZE * sizeof(struct iovec));

    while (sys_terminate == 0) {
        
        if (unlikely(volume_inuse_lfqueue->get_size() == 0)) {
            pthread_yield();
            continue;
        }

        v_idx = volume_inuse_lfqueue->dequeue();
        
        while (inode_dirty_lfqueue[v_idx]->get_size() > 1 && sys_terminate == 0) {
            nvm_flush(v_idx, i_idxs, iov);
        }

        volume_inuse_lfqueue->enqueue(v_idx);

    }
    
    free(i_idxs);
    free(iov);

    return NULL;
}

/**
 * Flush a batch of dirty data block from NVM to a permenant storage. */
void
nvm_flush(volume_idx_t v_idx, inode_idx_t* i_idxs, struct iovec* iov)
{

#if testing

    int i, indexToWrite, batch_count;
    uint32_t curr_lbn, succ_lbn;
    inode_entry *inode, *start_inode;
    inode_idx_t i_idx;
    ssize_t bytes_written;

    int num_dirty_inodes = inode_dirty_lfqueue[v_idx]->get_size()-1;
    int write_size = (num_dirty_inodes >= FLUSH_BATCH_SIZE) ? FLUSH_BATCH_SIZE : num_dirty_inodes;

    //dequeue from dirty and lock
    for(i = 0; i < write_size; i++) {
        i_idxs[i] = inode_dirty_lfqueue[v_idx]->dequeue();
        inode = &nvm->inode_table[i_idxs[i]];
        pthread_mutex_lock(&inode->lock);
    }

    indexToWrite = 0;
    while (indexToWrite < write_size) {

        //look ahead
        batch_count = 1;
        curr_lbn = (&nvm->inode_table[i_idxs[indexToWrite]])->lbn;

        for(i = indexToWrite+1; i < write_size; i++) {

            succ_lbn = (&nvm->inode_table[i_idxs[i]])->lbn;

            if(succ_lbn == curr_lbn + 1) {
                batch_count++;
                curr_lbn = succ_lbn;
            } else {
                break;
            }
        }

        //batch
        for(i = 0; i < batch_count; i++) {
            iov[i].iov_base = nvm->datablock_table + nvm->block_size * i_idxs[indexToWrite+i];
            iov[i].iov_len  = nvm->block_size;
        }

        //write
        start_inode = &nvm->inode_table[i_idxs[indexToWrite]];
        bytes_written = pwritev(start_inode->volume->fd, iov, batch_count, (off_t) nvm->block_size * start_inode->lbn);
        if(bytes_written != nvm->block_size * batch_count) {
            printf("ERROR: writev failed to write requested number of bytes! bytes_written: %ld, bytes_requested: %d\n", bytes_written, nvm->block_size * batch_count);
            exit(0);
        }

        //enqueue into sync
        for(i = 0; i < batch_count; i++) {
            i_idx = i_idxs[indexToWrite+i];
            inode = &nvm->inode_table[i_idx];
    
            inode->state = INODE_STATE_SYNC;
            inode_sync_lfqueue->enqueue(i_idx);
            monitor.sync++;
        }

        indexToWrite += batch_count;
    }

#else

    inode_idx_t idx = inode_dirty_lfqueue[v_idx]->dequeue();
    inode_entry* inode = &nvm->inode_table[idx];
    
    pthread_mutex_lock(&inode->lock);

    pwrite(inode->volume->fd, nvm->datablock_table + nvm->block_size * idx, nvm->block_size, (off_t) nvm->block_size * inode->lbn);

    inode->state = INODE_STATE_SYNC;
    inode_sync_lfqueue->enqueue(idx);
    monitor.sync++;

#endif

}
