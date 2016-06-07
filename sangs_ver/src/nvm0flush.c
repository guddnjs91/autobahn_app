#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "nvm0common.h"

void* flush_thread_func(void*);
void nvm_flush();
void* reclaim_thread_func(void*);

pthread_mutex_t reclaim_lock; /* Lock and Condition Variable for signaling */
pthread_cond_t reclaim_cond;  /* between write_thread and reclain_thread.  */

/**
 * Flush thread function.
 * Flushing dirty inodes in NVM to disk storage proactively */
void*
flush_thread_func(
    void* data)
{
    while(1) {
        /* Policy : if there are dirty inodes, flush it */
        while(inode_dirty_queue->is_empty() == true) {
            sched_yield();
        }
        nvm_flush();
    }

    return NULL;
}

/**
 * Flush one data block from NVM to data */
void
nvm_flush()
{
    inode_entry* inode;
    uint32_t index;

    /* Get the dirty inode which will be flushed soon. */
    index = inode_dirty_queue->dequeue();

    /* Get the index of inode which is also the index of data block,
     * write and flush that data block from NVM to storage. */
    inode = &inode_table[index];
    write(inode->volume->fd, nvm->datablock_table + nvm->block_size * index, nvm->block_size);
    fsync(inode->volume->fd);
    inode->state = INODE_STATE_SYNCED;

    /* Once flushed inode should be reclaimed for re-use.
     * This inode temporarily enqueued to synced_inode_lfqueue. */
    //INODE_SYNCED_LFQUEUE->enqueue(inode);
}

/**
 * Reclaim thread function.
 * When threads are waken up, get the number of inodes which are
 * in synced_inode_lfqueue, and reclaim those inodes. */
void*
balloon_thread_func(
    void* data)
{
    /* Policy : Wakes up and get the number of inodes for reclamation, 
     * reclaim them and sleeps again. */
//   while(1) {
//       pthread_mutex_lock(&reclaim_lock);
//       pthread_cond_wait(&reclaim_cond, &reclaim_lock); // wait
//       /* When wake up */
//       uint32_t size = INODE_SYNCED_LFQUEUE->get_size(); // get size
//       while(size) {
//           NVM_inode* inode = INODE_SYNCED_LFQUEUE->dequeue(); // get synced inode
//           inode->vte = delete_nvm_inode(inode->vte, inode); // delete from AVL tree
//           inode->state = INODE_STATE_FREE; // change state
//           INODE_FREE_LFQUEUE->enqueue(inode); // enqueue inode to free-inode-lfqueue.
//           size = size - 1;
//       }
//       pthread_mutex_unlock(&reclaim_lock);
//   }
}
