#pragma once

#include <cstdint>
#include <sys/types.h>
#include <pthread.h>
#include <atomic>
#include "nvm0inode.h"
#include "nvm0volume.h"
#include "nvm0lfqueue.h"

//config (make changes here ONLY)
#define DEFAULT_NVM_SIZE        (4 * 1024 * 1024 * 1024LLU)
#define MAX_VOLUME_ENTRY        (128)
#define MAX_OWNERLESS_BLOCK     (MAX_VOLUME_ENTRY)
#define BLOCK_SIZE              (1 << 14)

#define DEFAULT_NUM_FLUSH       (16)
#define DEFAULT_NUM_SYNCER      (2)
#define DEFAULT_NUM_BALLOON     (8)

#define DEFAULT_FREE_MIN_COUNT  (100)       // if it's 100, then 100 left is enough, 99 is not enough.
#define DEFAULT_FLUSH_LWM       (1)         // this is not currently used. it is instead set to max_inode_entry * p
#define DEFAULT_SYNC_LWM        (1)

#define FLUSH_BATCH_SIZE        (1024)      // maximum batch size for writev is 1024

#define DEFAULT_SYNC_OPTION     (1)
#define DEFAULT_MONITOR_OPTION  (1)
#define MONITORING_AMOUNT       (16)

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

//#define TEMP_FIX
#define MULTIPLE_STORAGE_ON

/* NVM options */
extern uint64_t NVM_SIZE;
extern uint32_t NUM_FLUSH;
extern uint32_t SYNC_OPTION;
extern uint32_t MONITOR_OPTION;

extern uint64_t kFlushLwm;

/** Represents the metadata of NVM */
struct nvm_metadata {
    //system config value
    uint64_t    nvm_size;
    uint32_t    max_volume_entry;
    uint32_t    max_inode_entry;
    uint32_t    max_ownerless_block;
    uint32_t    block_size;

    //Base address of each table
    volume_entry*   volume_table;
    inode_entry*    inode_table;
    block_entry*    block_table;
};

//Global variables
extern struct nvm_metadata* nvm; //holds information about NVM

extern pthread_cond_t   g_balloon_cond[DEFAULT_NUM_BALLOON];     //balloon condition variable
extern pthread_mutex_t  g_balloon_mutex[DEFAULT_NUM_BALLOON];    //mutex for b_cond

extern pthread_t flush_thread[DEFAULT_NUM_FLUSH];
extern pthread_t sync_thread[DEFAULT_NUM_SYNCER];
extern pthread_t balloon_thread[DEFAULT_NUM_BALLOON];
extern pthread_t monitor_thread;

//Conditional variable for system termination
extern int sys_terminate; 

//=================== lfqueues ====================//
/* for volumes */
extern lfqueue<volume_idx_t>* volume_free_lfqueue;
extern lfqueue<volume_idx_t>* volume_inuse_lfqueue;

/* for free inodes */
extern lfqueue<inode_idx_t>* inode_free_lfqueue;

/* for dirty inodes */
extern lfqueue<inode_idx_t>* inode_dirty_lfqueue[MAX_VOLUME_ENTRY];
extern atomic<inode_idx_t>   inode_dirty_count;

/* for sync inodes */
extern lfqueue<inode_idx_t>* inode_sync_lfqueue[DEFAULT_NUM_SYNCER];
extern atomic<uint_fast64_t> sync_queue_idx;

/* for clean inodes */
extern lfqueue<inode_idx_t>* inode_clean_lfqueue[DEFAULT_NUM_BALLOON];
extern atomic<uint_fast64_t> clean_queue_idx;

/* for ownerless blocks */
extern lfqueue<inode_idx_t>* block_ownerless_lfqueue;

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
