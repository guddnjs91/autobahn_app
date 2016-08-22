#pragma once

#include <cstdint>
#include <sys/types.h>
#include <pthread.h>
#include "nvm0inode.h"
#include "nvm0volume.h"
#include "nvm0lfqueue.h"

//config (make changes here ONLY)
#define NVM_SIZE            (4 * 1024 * 1024 * 1024LLU)
#define MAX_VOLUME_ENTRY    (1024)
#define BLOCK_SIZE          (16 * 1024)

#define NUM_FLUSH_THR       (16)
#define FLUSH_BATCH_SIZE    (1024)
#define FLUSH_LWM           (128) //not used anymore!

#define MIN_SYNC_FREQUENCY  (1<<14)

#define MONITORING          (1)
#define SYNC_OPTION         (1)
#define testing             (1)

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/** Represents the metadata of NVM */
struct nvm_metadata {
    //system config value
    uint64_t    nvm_size;
    uint32_t    max_volume_entry;
    uint32_t    max_inode_entry;
    uint32_t    block_size;

    //Base address of each table
    volume_entry*   volume_table;
    inode_entry*    inode_table;
    char*           datablock_table;
};

//Global variables
extern struct nvm_metadata* nvm; //holds information about NVM

extern pthread_rwlock_t g_balloon_rwlock;   //balloon read/write lock
extern pthread_cond_t   g_balloon_cond;     //balloon condition variable
extern pthread_mutex_t  g_balloon_mutex;    //mutex for b_cond

extern pthread_t flush_thread[NUM_FLUSH_THR];
extern pthread_t sync_thread;
extern pthread_t balloon_thread;
extern pthread_t monitor_thread;

//Conditional variable for system termination
extern int sys_terminate; 

//lfqueues
extern lfqueue<volume_idx_t>* volume_free_lfqueue;
extern lfqueue<volume_idx_t>* volume_inuse_lfqueue;

extern lfqueue<inode_idx_t>* inode_free_lfqueue;
extern lfqueue<inode_idx_t>* inode_dirty_lfqueue[MAX_VOLUME_ENTRY];
extern lfqueue<inode_idx_t>* inode_sync_lfqueue;
extern lfqueue<inode_idx_t>* inode_clean_lfqueue;

/* functions */

/* in file nvm0nvm.cc */
void nvm_structure_build();
void nvm_system_init();
void nvm_system_close();
void nvm_structure_destroy();
void print_nvm_info();

size_t nvm_durable_write(uint32_t vid, off_t ofs, const char* ptr, size_t len);

void* flush_thread_func(void* data);
void* balloon_thread_func(void* data);
void* sync_thread_func(void* data);
void* monitor_thread_func(void* data);
