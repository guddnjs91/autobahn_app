/******************************************************************//**
@file include/nvm0metadata.h
Config and Metadata are stored in this header file.
Config is the configuration before setting up and starting NVM System.
Meta Data stores the information that has to be persistent
throughout the NVM System use with given config settings.

Created 2016/06/07 Sang Rhee
***********************************************************************/

#ifndef nvm0metadata_h
#define nvm0metadata_h

#include <pthread.h>

//config (make changes here ONLY)
#define NVM_SIZE            1 * 1024 * 1024 * 1024LLU
#define MAX_VOLUME_ENTRY    1024
#define BLOCK_SIZE          16 * 1024

/** Represents the metadata of NVM */
struct nvm_metadata {
    //system config value
    uint64_t    nvm_size;
    uint32_t    max_volume_entry;
    uint32_t    max_inode_entry;
    uint32_t    block_size;

    // Base address of each table
    volume_entry*   volume_table;
    inode_entry*    inode_table;
    char*           datablock_table;
};

/* Global variable that can access every NVM area */
extern struct nvm_metadata* nvm;

/* Global pthread variables for ballooning */
extern pthread_rwlock_t     g_balloon_rwlock;   // global balloon read/write lock
extern pthread_cond_t       g_balloon_cond;     // global balloon condition variable
extern pthread_mutex_t      g_balloon_mutex;    // mutex for b_cond
extern pthread_cond_t       g_flush_cond;     
extern pthread_mutex_t      g_flush_mutex;    

/* Global pthread flush thread and balloon thread */
extern pthread_t flush_thread;
extern pthread_t balloon_thread;

/* System termination condition variables */
extern volatile int sys_terminate;

#endif
