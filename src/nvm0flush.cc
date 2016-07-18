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

    printf("Flush thread terminated.....\n");

    return NULL;
}

/**
Flush one dirty data block from nvm to disk. */
void
nvm_flush(
    void)
{
    if(!inode_dirty_lfqueue->get_size())
    {
        return ;
    }

    /* Pick dirty inodes in dirty inode LFQ. */
    inode_idx_t idx = inode_dirty_lfqueue->dequeue();
    inode_entry* inode = &nvm->inode_table[idx];
    
    /* Flushing one data block is locked with inode lock. */
    pthread_mutex_lock(&inode->lock);

    /* Write one data block from nvm to disk 
    and make inode state CLEAN. */
    write(inode->volume->fd, nvm->datablock_table + nvm->block_size * idx, nvm->block_size);
    inode->state = INODE_STATE_CLEAN;

    /* Unlock inode lock. */
    pthread_mutex_unlock(&inode->lock);
}

///**
// * Reclaim thread function.
// * When threads are waken up, get the number of inodes which are
// * in synced_inode_lfqueue, and reclaim those inodes. */
//void*
//reclaim_thread_func(
//    void* data)
//{
//    /**
//     * Policy : Wakes up and get the number of inodes for reclamation, 
//     * reclaim them and sleeps again. */
//     while(1) {
//         pthread_mutex_lock(&reclaim_lock);
//         pthread_cond_wait(&reclaim_cond, &reclaim_lock); // wait
//         /* When wake up */
//         uint32_t size = INODE_SYNCED_LFQUEUE->get_size(); // get size
//         while(size) {
//             NVM_inode* inode = INODE_SYNCED_LFQUEUE->dequeue(); // get synced inode
//             inode->vte = delete_nvm_inode(inode->vte, inode); // delete from AVL tree
//             inode->state = INODE_STATE_FREE; // change state
//             INODE_FREE_LFQUEUE->enqueue(inode); // enqueue inode to free-inode-lfqueue.
//             size = size - 1;
//         }
//         pthread_mutex_unlock(&reclaim_lock);
//     }
//}
