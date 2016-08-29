#pragma once

#include <cstdint>
#include <sys/types.h>
#include <pthread.h>
#include <atomic>
#include "nvm0inode.h"
#include "nvm0volume.h"
#include "nvm0lfqueue.h"

//config (make changes here ONLY)
#define NVM_SIZE            (1LLU << 32)
#define MAX_VOLUME_ENTRY    (128)
#define BLOCK_SIZE          (1 << 14)

#define MAX_NUM_FREE        (2)

#define MAX_NUM_FLUSHER     (32)
#define FLUSH_BATCH_SIZE    (1024)

#define MAX_NUM_SYNCER      (2)
#define MIN_SYNC_FREQUENCY  (1<<13)

#define MAX_NUM_BALLOON     (8)

#define MONITORING_AMOUNT   (7) 

#define testing             (0)

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/* Command argument defined options */
#define DEFAULT_FILE_SIZE   (80 * 1024 * 1024 * 1024LLU)
#define DEFAULT_NUM_THREAD  (1)
#define DEFAULT_NUM_FLUSH   (16)
#define MONITOR_OFF         (0)
#define MONITOR_ON          (1)
#define SYNC_OFF            (0)
#define SYNC_ON             (1)

extern long long unsigned int total_file_size;
extern int num_thread;
extern int num_flusher;
extern int verbose_flag;
extern int sync_flag;
extern int write_mode;

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

extern pthread_cond_t   g_balloon_cond[MAX_NUM_BALLOON];     //balloon condition variable
extern pthread_mutex_t  g_balloon_mutex[MAX_NUM_BALLOON];    //mutex for b_cond

extern pthread_t flush_thread[MAX_NUM_FLUSHER];
extern pthread_t sync_thread[MAX_NUM_SYNCER];
extern pthread_t balloon_thread[MAX_NUM_BALLOON];
extern pthread_t monitor_thread;

//Conditional variable for system termination
extern int sys_terminate; 

//lfqueues
extern lfqueue<volume_idx_t>* volume_free_lfqueue;
extern lfqueue<volume_idx_t>* volume_inuse_lfqueue;

extern lfqueue<inode_idx_t>* inode_free_lfqueue[MAX_NUM_FREE];
extern atomic<uint_fast64_t> free_enqueue_idx;
extern atomic<uint_fast64_t> free_dequeue_idx;
extern lfqueue<inode_idx_t>* inode_dirty_lfqueue[MAX_VOLUME_ENTRY];
extern lfqueue<inode_idx_t>* inode_sync_lfqueue[MAX_NUM_SYNCER];
extern atomic<uint_fast64_t> sync_queue_idx;
extern lfqueue<inode_idx_t>* inode_clean_lfqueue[MAX_NUM_BALLOON];
extern atomic<uint_fast64_t> clean_queue_idx;

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
