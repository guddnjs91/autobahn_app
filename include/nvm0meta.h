#ifndef _NVM_META_H_
#define _NVM_META_H_

/**
 * nvm0meta.h - Config and Metadata are stored in this header file.
 *   Config is the configuration before setting up and starting NVM System.
 *   Meta Data stores the information that has to be persistent
 *     throughout the NVM System use with given config settings. */

//config (make changes here ONLY)
#define NVM_SIZE        8 * 1024 * 1024 * 1024
#define MAX_VT_ENTRY    1024
#define BLOCK_SIZE      16 * 1024

/* Represent the metadata of NVM */
typedef struct _nvm_metadata {

    //system config value
    uint64_t    NVM_SIZE;
    uint64_t    MAX_VT_ENTRY;
    uint64_t    MAX_INODE_ENTRY;
    uint64_t    BLOCK_SIZE;

    // Base address of each table
    char*       VOLUME_TABLE_START;
    char*       INODE_TABLE_START;
    char*       DATA_TABLE_START;

} NVM_metadata;

/* Global variable that can access every NVM area */
extern NVM_metadata* NVM;

#endif
