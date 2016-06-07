#include <stdio.h>
#include "nvm0common.h"

#define SHM_KEY 1234

NVM_metadata*
create_nvm_in_shm()
{
    int shm_id;
    char* shm_addr;

    printf("Creating NVM in DRAM (shared memory).\n");

    shm_id = shmget((key_t) SHM_KEY, NVM_SIZE, 0666 | IPC_CREAT);
    if(shmid == -1) {
        perror("shmget failed: ");
        exit(0);
    }
    shm_addr = shmat(shmid, (void *)0, 0);
    if(shm_addr == (char*) -1) {
        perror("shmat failed: ");
        exit(0);
    }

    printf("[Succeeded]\n");

    return (NVM_metadata*) shm_addr;
}

/**
 * Construct NVM data structure
 * - volume_table
 * - inode_table
 * - block_table
 *
 *  +---------------------------------------------------------------------------------------+
 *  |                                  NON-VOLATILE MEMORY                                  |
 *  +--------------------------+------------------+-----------------------+-----------------+
 *  |         METADATA         |   VOLUME TABLE   |      INODE TABLE      |   BLOCK TABLE   |
 *  +--------------------------+------------------+-----------------------+-----------------+
 *  | NVM_SIZE                 | Volume ID        | Logical Block Number  |                 |
 *  | Max Volume Table Entry   | fd               | INode State           |                 |
 *  | Max INode Table Entry    | INode Tree Root  | Volume Table Entry    |                 |
 *  | Block Size               |                  |                       |                 |
 *  | Volume Table             |                  |                       |                 |
 *  | INode Table              |                  |                       |                 |
 *  | Data Block Table         |                  |                       |                 |
 *  +--------------------------+------------------+-----------------------+-----------------+
 */
void
nvm_structure_build()
{
    int i;

    NVM = create_nvm_in_shm();

    NVM->NVM_SIZE           = NVM_SIZE;
    NVM->MAX_VT_ENTRY       = MAX_VT_ENTRY;
    NVM->MAX_INODE_ENTRY    = ( NVM_SIZE - sizeof(NVM_metadata) - sizeof(VT_entry)*MAX_VT_ENTRY ) /
                              ( sizeof(NVM_inode) + BLOCK_SIZE )
    NVM->BLOCK_SIZE         = BLOCK_SIZE;

    NVM->VOLUME_TABLE   = (char *) (NVM + 1);
    NVM->INODE_TABLE    = (char *) ((VT_entry*) NVM->VOLUME_TABLE_START + NVM->MAX_VT_ENTRY);
    NVM->DATA_TABLE     = (char *) ((NVM_inode*) NVM->INODE_START + MAX_NVM_INODE);

    for(i = 0; i < MAX_INODE_ENTRY; i++)
    {
        NVM->INODE_TABLE[i] = INODE_STATE_FREE;
    }

    print_nvm_info();
}

/**
 * Starts the NVM system
 * 1. Create abstract data types to control NVM data structure.
 *     - VTE_FREE_LFQUEUE
 *     - VTE_INUSE_LFQUEUE
 *     - INODE_FREE_LFQUEUE
 *     - INODE_DIRTY_LFQUEUE
 * 2. Create necessary threads for NVM system.
 *     - FLUSH_THREAD
 *     - BALLOON_THREAD
 */
void
nvm_system_init()
{
}

/**
 *  */
void
nvm_system_close()
{
}

/**
 * Print NVM address information */
void
print_nvm_info()
{
    printf("\nNVM information \n");
    
    printf("\n[RESERVED AREA]\n");
    printf("- start  : %p\n", NVM);
    printf("- size   : %ld\n", sizeof(NVM_metadata));

    printf("\n[VOLUME TABLE]\n");
    printf("- start  : %p\n", NVM->VOL_TABLE_START);
    printf("- end    : %p\n", NVM->VOL_TABLE_START + MAX_VT_ENTRY);
    printf("- vte size  : %ld Bytes\n", sizeof(VT_entry));
    printf("- #vte      : %d\n", MAX_VT_ENTRY);
    printf("- free list : %p\n", NVM->FREE_VTE_LIST_HEAD);
    printf("- #free vte : %d\n", get_free_vte_num());
//  data_dump((unsigned char*)NVM->VOL_TABLE_START, 1024);

    printf("\n[INODE]\n");
    printf("- start : %p\n", NVM->INODE_START);
    printf("- end   : %p\n", NVM->INODE_START + MAX_NVM_INODE);
    printf("- inode size  : %ld Bytes\n", sizeof(NVM_inode));
    printf("- #inodes     : %d\n", MAX_NVM_INODE);
    printf("- free list   : %p\n", NVM->FREE_INODE_LIST_HEAD);
    printf("- #free inode : %d\n", get_free_inode_num());
    printf("- sync list   : %p\n", NVM->SYNC_INODE_LIST_HEAD);
    printf("- #sync inode : %d\n", get_sync_inode_num());
//  data_dump((unsigned char*)NVM->INODE_START, sizeof(NVM_inode)*20);
    
    printf("\n[DATA]\n");
    printf("- start : %p\n", NVM->DATA_START);
    printf("- end   : %p\n", NVM->DATA_START + MAX_NVM_INODE * BLOCK_SIZE);
    printf("- block size : %d Bytes\n", BLOCK_SIZE);
    printf("- #blk in nvm: %d\n", MAX_NVM_INODE - get_free_inode_num());
    printf("\n\n");
//  data_dump((unsigned char*)NVM->DATA_START, BLOCK_SIZE * 33);
}
