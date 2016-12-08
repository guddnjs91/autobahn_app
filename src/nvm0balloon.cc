#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <stack>
#include "nvm0nvm.h"
#include "nvm0inode.h"
#include "nvm0hash.h"
#include "nvm0monitor.h"

//private function declarations
void balloon_wait(int index);
void fill_free_inodes(int index);
bool trylock_hash_node(struct hash_node *node);

/**
 * Balloon thread function wakes up when free inode shortage.
 */
void* balloon_thread_func(void* data)
{
    int index = *(int *) data;
    while (sys_terminate == 0) {
        fill_free_inodes(index);
    }
    return NULL;
}

/**
 * Wait for writer's signal or after 10 seconds.
 * Writers will signal if there is no more free inodes.
 */
void balloon_wait(int index)
{
    struct timeval now;
    struct timespec timeout;

    pthread_mutex_lock(&g_balloon_mutex[index]);
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 10;
    timeout.tv_nsec = now.tv_usec * 1000;
    pthread_cond_timedwait(&g_balloon_cond[index], &g_balloon_mutex[index], &timeout);
    pthread_mutex_unlock(&g_balloon_mutex[index]);
}

/**
 * 1. Looks through inode_clean_list,
 * 2. invalidate the node in hash_table,
 * 3. and fill up the inode_free_lfqueue.
 */
void fill_free_inodes(int index)
{
    while (!inode_clean_lfqueue[index]->is_empty()) {

        /* 1. get inode from clean queue */
        inode_idx_t idx = inode_clean_lfqueue[index]->dequeue();
        struct inode_entry* inode = &nvm->inode_table[idx];

        /* 2. check if really clean & lock */
        /* filter 1 */
        if (inode->state != INODE_STATE_CLEAN) {
            continue; 
        }
        /* get hash node */
        struct hash_node* hash_node = search_hash_node(inode->volume->hash_table, inode->lbn);
        /* filter 2 */
        if (!trylock_hash_node(hash_node)) {
            continue;
        }

        /* 3. invalidate the hash node */
        logical_delete_hash_node(inode->volume->hash_table, hash_node);

        /* 4. insert the inode into free queue */
        inode->state = INODE_STATE_FREE;
        inode_free_lfqueue->enqueue(idx);
        pthread_mutex_unlock(&hash_node->mutex);
        monitor.free++;
    }
}

inline bool trylock_hash_node(struct hash_node *hash_node)
{
    /* hash_node trylock */
    int error = pthread_mutex_trylock(&hash_node->mutex);
    if (error) { //trylock failed
        return false; /* writer will make it dirty anyways */
    }

    /* if the node became dirty just before the trylock */
    if (hash_node->inode->state != INODE_STATE_CLEAN) {
        pthread_mutex_unlock(&hash_node->mutex);
        return false;
    }
    return true;
}
