#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/uio.h>
#include <atomic>
#include "nvm0nvm.h"
#include "nvm0inode.h"
#include "nvm0monitor.h"

using namespace std;

//private function declarations
void nvm_flush(volume_idx_t v_idx, inode_idx_t* i_idxs, struct iovec* iov);

/**
 * Flush thread function proactively runs and flushes dirty inodes. */
void*
flush_thread_func(
    void* data)
{
    //init
    inode_idx_t     *i_idxs = (inode_idx_t *)  malloc( FLUSH_BATCH_SIZE * sizeof(inode_idx_t));
    struct iovec    *iov    = (struct iovec *) malloc( FLUSH_BATCH_SIZE * sizeof(struct iovec));

    while (sys_terminate == 0) {
        
        //TODO: put more comments here
        if (inode_dirty_count < kFlushLwm) {
            usleep(10);
            continue;
        }
        if (unlikely(volume_inuse_lfqueue->get_size() == 0)) {
            pthread_yield();
            continue;
        }

        volume_idx_t v_idx = volume_inuse_lfqueue->dequeue();
        
        if (inode_dirty_lfqueue[v_idx]->get_size() > 1) {
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
    int i, indexToWrite, batch_count;
    uint32_t curr_lbn, succ_lbn;
    inode_entry *inode, *start_inode;
    ssize_t bytes_written;
    int sync_idx;

    int num_dirty_inodes = inode_dirty_lfqueue[v_idx]->get_size()-1;
    int write_size = (num_dirty_inodes >= FLUSH_BATCH_SIZE) ? FLUSH_BATCH_SIZE : num_dirty_inodes;

    volume_entry *ve = get_volume_entry(v_idx);

    //dequeue from dirty and lock
    for (i = 0; i < write_size; i++) {
        i_idxs[i] = inode_dirty_lfqueue[v_idx]->dequeue();
        inode = &nvm->inode_table[i_idxs[i]];

        ////inode lock/////////////////////////////////////////
        int error = pthread_mutex_trylock(&inode->lock);

        if (error) { //lock failed
            inode_dirty_lfqueue[v_idx]->enqueue(i_idxs[i--]);
            if (--num_dirty_inodes < FLUSH_BATCH_SIZE) {
                write_size--;
            }
        }
        ///////////////////////////////////////////////////////
    }

    indexToWrite = 0;
    while (indexToWrite < write_size) {

        //look ahead
        batch_count = 1;
        curr_lbn = (&nvm->inode_table[i_idxs[indexToWrite]])->lbn;

        for (i = indexToWrite+1; i < write_size; i++) {

            succ_lbn = (&nvm->inode_table[i_idxs[i]])->lbn;

            if (succ_lbn == curr_lbn + 1) {
                batch_count++;
                curr_lbn = succ_lbn;
            } else {
                break;
            }
        }

        //batch
        for (i = 0; i < batch_count; i++) {
            block_idx_t block_index = nvm->inode_table[i_idxs[indexToWrite+i]].block_index;
            iov[i].iov_base = nvm->block_table[block_index].data;
            //iov[i].iov_base = nvm->block_table[i_idxs[indexToWrite+i]].data;

            off_t file_size = get_filesize(ve->vid);
            uint32_t lbn = (&nvm->inode_table[i_idxs[indexToWrite+i]])->lbn;
            
            iov[i].iov_len  = nvm->block_size;

            /*if ((file_size - 1) / nvm->block_size == lbn) {
                iov[i].iov_len = ((file_size - 1) % nvm->block_size) + 1;
            } else {
                iov[i].iov_len  = nvm->block_size;
            }*/
        }

        //write
        start_inode = &nvm->inode_table[i_idxs[indexToWrite]];
        bytes_written = pwritev(start_inode->volume->fd, iov, batch_count, (off_t) nvm->block_size * start_inode->lbn);
        if(bytes_written != nvm->block_size * batch_count) {
            printf("ERROR: writev failed to write requested number of bytes! bytes_written: %ld, bytes_requested: %d\n", bytes_written, nvm->block_size * batch_count);
extern uint64_t kFlushLwm;
            exit(0);
        }

#ifdef TEMP_FIX
        // In this case, sync() is processed in flush thread.
        uint_fast64_t clean_idx;

        if(likely(SYNC_OPTION)) {
            sync();
        }

        for (i = 0; i < batch_count; i++) {
            inode_idx_t idx = i_idxs[indexToWrite+i];
            inode = &nvm->inode_table[idx];

            /* hands off dirty */
            inode_dirty_count--;

            /* hands on clean */
            inode->state = INODE_STATE_CLEAN;
            inode_clean_lfqueue[ clean_queue_idx++ % DEFAULT_NUM_BALLOON ]->enqueue(idx);
            monitor.clean++;
            pthread_mutex_unlock(&inode->lock);
        }
#else
        //enqueue into sync
        for(i = 0; i < batch_count; i++) {
            inode_idx_t i_idx = i_idxs[indexToWrite+i];
            inode = &nvm->inode_table[i_idx];

            /* hands off dirty */
            inode_dirty_count--;

            /* hands on sync */
            inode->state = INODE_STATE_SYNC;
            inode_sync_lfqueue[ sync_queue_idx++ % DEFAULT_NUM_SYNCER ]->enqueue(i_idx);
            monitor.sync++;
        }
#endif
        indexToWrite += batch_count;
    }
}
