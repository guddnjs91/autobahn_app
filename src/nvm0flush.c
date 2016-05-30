#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "nvm0common.h"
#include "nvm0lfq.h"

extern NVM_metadata* NVM;
extern pthread_mutex_t reclaim_lock; /* Lock and Condition Variable for signaling */
extern pthread_cond_t reclaim_cond;  /* between write_thread and reclain_thread.  */

/**
 * Flush thread function.
 * Flushing dirty inodes in NVM to disk storage proactively */
void*
flush_thread_func(
    void* data)
{
    while(1) {
        /**
         * Policy : if there are dirty inodes, flush it */
        while(dirty_inode_lfqueue.is_empty() == true) {
            sched_yield();
        }
        nvm_flush();
    }

    return NULL;
}

/**
 * Flush one data block from NVM to data */
void
nvm_flush(
    void)
{
    /**
     * Get the dirty inode which will be flushed soon. */
    NVM_inode* inode = dirty_inode_lfqueue.dequeue();

    /**
     * Get the index of inode which is also the index of data block,
     * write and flush that data block from NVM to storage. */
    unsigned int idx = get_nvm_inode_idx(inode);
    write(inode->vte->fd, NVM->DATA_START + BLOCK_SIZE * idx, BLOCK_SIZE);
    fsync(inode->vte->fd);
    inode->state = INODE_STATE_SYNCED;
    printf("sync %d Bytes\n", BLOCK_SIZE);

    /**
     * Once flushed inode should be reclaimed for re-use.
     * This inode temporarily enqueued to synced_inode_lfqueue. */
    INODE_SYNCED_LFQUEUE.enqueue(*inode);
}

/**
 * Reclaim thread function.
 * When threads are waken up, get the number of inodes which are
 * in synced_inode_lfqueue, and reclaim those inodes. */
void*
reclaim_thread_func(
    void* data)
{
    /**
     * Policy : Wakes up and get the number of inodes for reclamation, 
     * reclaim them and sleeps again. */
     while(1) {
         pthread_mutex_lock(&reclaim_lock);
         pthread_cond_wait(&reclaim_cond, &reclaim_lock); // wait
         /* When wake up */
         uint32_t size = INODE_SYNCED_LFQUEUE.get_size(); // get size
         while(size) {
             NVM_inode* inode = INODE_SYNCED_LFQUEUE.dequeue(); // get synced inode
             inode->vte = delete_nvm_inode(inode->vte, inode); // delete from AVL tree
             inode->state = INODE_STATE_FREE; // change state
             INODE_FREE_LFQUEUE.enqueue(*inode); // enqueue inode to free-inode-lfqueue.
             size = size - 1;
         }
         pthread_mutex_unlock(&reclaim_lock);
     }
}
