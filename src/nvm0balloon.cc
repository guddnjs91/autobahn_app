#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <stack>
#include "nvm0common.h"

//private function declarations
void nvm_balloon();
void balloon_wait();
void fill_free_inodes();

/**
Balloon thread function wakes up when free inode shortage.*/
void*
balloon_thread_func(
    void* data)
{
    printf("Balloon thread running.....\n");
    
    while(sys_terminate == 0) {
        nvm_balloon();
    }
    
    printf("Balloon thread terminated.....\n");
    
    return NULL;
}

/**
 * When necessary, balloon function takes inodes from inode_clean_list and fills up the inode_free_lfqueue.
 */
void
nvm_balloon()
{
    balloon_wait();

    if(sys_terminate == 1) {
        return;
    }

    fill_free_inodes();
}

/**
 * Wait for writer's signal or after 10 seconds.
 * Writers will signal if there is no more free inodes.
 */
void
balloon_wait()
{
    struct timeval now;
    struct timespec timeout;

    pthread_mutex_lock(&g_balloon_mutex);
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 10;
    timeout.tv_nsec = now.tv_usec * 1000;
    pthread_cond_timedwait(&g_balloon_cond, &g_balloon_mutex, &timeout);
    pthread_mutex_unlock(&g_balloon_mutex);
}

/**
 * 1. Looks through inode_clean_list,
 * 2. invalidate the node in hash_table,
 * 3. and fill up the inode_free_lfqueue.
 */
void
fill_free_inodes()
{
    pthread_rwlock_wrlock(&g_balloon_rwlock);

    //Traverse volume table that each entry has a hash table.
    while(!inode_clean_lfqueue->is_empty()) {

        inode_idx_t idx = inode_clean_lfqueue->dequeue();
        struct inode_entry* inode = &nvm->inode_table[idx];

        //concurrency problem may occur here!
        if (inode->state == INODE_STATE_DIRTY)
        {
           continue; 
        }

        logical_delete_hash_node(inode->volume->hash_table, inode->hash_node);

        inode->state = INODE_STATE_FREE;
        inode_free_lfqueue->enqueue(idx);
    }

    pthread_rwlock_unlock(&g_balloon_rwlock);
}
