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
Balloon function traverses tree and reclaims free inodes
in invalid tree nodes. */
void
nvm_balloon(
    void)
{
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);
    timeout.tv_sec = now.tv_sec + 10;
    timeout.tv_nsec = now.tv_usec * 1000;

    /* Wait signal from nvm_write() function. */
    pthread_mutex_lock(&g_balloon_mutex);
    pthread_cond_timedwait(&g_balloon_cond, &g_balloon_mutex, &timeout);
    pthread_mutex_unlock(&g_balloon_mutex);

    if(sys_terminate) {
        return;
    }

    /* Lock write-lock to be mutually exclusive to write threads. */
    pthread_rwlock_wrlock(&g_balloon_rwlock);

    /* Traverse volume table that each entry has a hash table. */

        //TODO:  4 node
        // 1. Traverse list and change state clean to invalid 
        // 2. Enqueue the node into Free LFQueue

    /* Unlock write-lock. */
    pthread_rwlock_unlock(&g_balloon_rwlock);
}
