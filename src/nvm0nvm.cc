#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdlib.h>
#include "nvm0common.h"
#include <pthread.h>

#define SHM_KEY 1234
void *shm_addr;

// Declarations of global variables
struct nvm_metadata* nvm;
lfqueue<volume_idx_t>* volume_free_lfqueue;
lfqueue<volume_idx_t>* volume_inuse_lfqueue;
lfqueue<inode_idx_t>* inode_free_lfqueue;
lfqueue<inode_idx_t>* inode_dirty_lfqueue;

/* Global pthread variables for ballooning */
pthread_rwlock_t     g_balloon_rwlock;   // global balloon read/write lock
pthread_cond_t       g_balloon_cond;     // global balloon condition variable
pthread_mutex_t      g_balloon_mutex;    // mutex for b_cond

/* Global pthread flush thread and balloon thread. */
pthread_t flush_thread;
pthread_t balloon_thread;

/* System termination variable */
volatile int sys_terminate; // 1 : terminate yes

//private function declaration
void print_nvm_info();
nvm_metadata* create_nvm_in_shm();
void remove_nvm_in_shm();

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
 *  | Max Volume Entry         | fd               | INode State           |                 |
 *  | Max INode Entry          | INode Tree Root  | Volume Table Entry    |                 |
 *  | Block Size               |                  |                       |                 |
 *  | Volume Table             |                  |                       |                 |
 *  | INode Table              |                  |                       |                 |
 *  | Data Block Table         |                  |                       |                 |
 *  +--------------------------+------------------+-----------------------+-----------------+
 */
void
nvm_structure_build()
{
    uint32_t i;

    //Initialize nvm_metadata
    nvm = create_nvm_in_shm();

    nvm->nvm_size           = NVM_SIZE;
    nvm->max_volume_entry   = MAX_VOLUME_ENTRY;
    nvm->max_inode_entry    = ( NVM_SIZE - sizeof(struct nvm_metadata) - 
                                sizeof(struct volume_entry)*MAX_VOLUME_ENTRY )
                            / ( sizeof(struct inode_entry) + BLOCK_SIZE );
    nvm->block_size         = BLOCK_SIZE;

    nvm->volume_table       = (struct volume_entry*) (nvm + 1);
    nvm->inode_table        = (struct inode_entry*)  (nvm->volume_table + nvm->max_volume_entry);
    nvm->datablock_table    = (char *)               (nvm->inode_table + nvm->max_inode_entry);

    //Initialize all inode_entries to state_free
    for(i = 0; i < nvm->max_inode_entry; i++)
    {
        nvm->inode_table[i].state = INODE_STATE_FREE;
    }
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
    volume_free_lfqueue = new lfqueue<volume_idx_t>(nvm->max_volume_entry);
    volume_inuse_lfqueue = new lfqueue<volume_idx_t>(nvm->max_volume_entry);
    for(volume_idx_t i = 0; i < nvm->max_volume_entry; i++)
    {
        volume_free_lfqueue->enqueue(i);
    }

    inode_free_lfqueue = new lfqueue<inode_idx_t>(nvm->max_inode_entry);
    inode_dirty_lfqueue = new lfqueue<inode_idx_t>(nvm->max_inode_entry);
    for(inode_idx_t i = 0; i < nvm->max_inode_entry; i++)
    {
        inode_free_lfqueue->enqueue(i);
    }

    printf("Created system latch-free-queue ...\n");

    pthread_rwlock_init(&g_balloon_rwlock, NULL);
    pthread_cond_init(&g_balloon_cond, NULL);
    pthread_mutex_init(&g_balloon_mutex, NULL);

    //TODO: create  flush thread & balloom thread
    sys_terminate = 0;
    //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_create(&flush_thread, NULL, flush_thread_func, NULL);
    pthread_create(&balloon_thread, NULL, balloon_thread_func, NULL);

    printf("Created flush thread and balloon thread ...\n");
}

/**
 *  */
void
nvm_system_close()
{
    //TODO: terminate flush thread & balloon thread
    sys_terminate = 1;
    pthread_mutex_lock(&g_balloon_mutex);
    pthread_cond_signal(&g_balloon_cond);
    pthread_mutex_unlock(&g_balloon_mutex);
    
    /* For the busy-waiting flush thread. */
    if(inode_dirty_lfqueue->get_size() == 0)
    {
        //printf("flush thread must be canceled\n");
        //pthread_cancel(flush_thread);
        
        /* Select one CLEAN inode and add to dirty inode LFQ. (Not a good solution) */
        for(inode_idx_t idx = 0; idx < nvm->max_inode_entry; idx++)
        {
            inode_entry* inode = &nvm->inode_table[idx];
            if(inode->state == INODE_STATE_CLEAN)
            {
                inode_dirty_lfqueue->enqueue(idx);
                break;
            }
        }
    }
        
    pthread_join(flush_thread, NULL);
    pthread_join(balloon_thread, NULL);

    while(inode_dirty_lfqueue->get_size() != 0)
    {
        inode_idx_t idx = inode_dirty_lfqueue->dequeue();
        inode_entry* inode = &nvm->inode_table[idx];

//        printf("dirty inode LFQ cleaning\n");
    lseek(inode->volume->fd, nvm->block_size * inode->lbn, SEEK_SET);
//    printf("VOL_%u.txt : offset %u => ", inode->volume->vid, nvm->block_size * inode->lbn);
    write(inode->volume->fd, nvm->datablock_table + nvm->block_size * idx, nvm->block_size);
//    printf("%u Bytes Data flushed from nvm->inode_table[%u]\n", nvm->block_size, idx);
    inode->state = INODE_STATE_CLEAN;
    }

    while(volume_inuse_lfqueue->get_size() != 0)
    {
        volume_idx_t v = volume_inuse_lfqueue->dequeue();
        volume_entry* ve = &nvm->volume_table[v];

        close(ve->fd);
    }


    delete volume_free_lfqueue;
    delete volume_inuse_lfqueue;
    delete inode_free_lfqueue;
    delete inode_dirty_lfqueue;
}

void
nvm_structure_destroy()
{
    remove_nvm_in_shm();
}

/**
 * Print NVM address information */
void
print_nvm_info()
{
    printf("====================NVM Information====================\n");
    printf("NVM Size          : %llu\n", (long long unsigned int) nvm->nvm_size);
    printf("Metadata Size     : %d\n", (int) sizeof(struct nvm_metadata));
    printf("Volume Entry Size : %d\n", (int) sizeof(struct volume_entry));
    printf("Max Volume Entry  : %u\n", nvm->max_volume_entry);
    printf("Inode Entry Size  : %d\n", (int) sizeof(struct inode_entry));
    printf("Data Block Size   : %d\n", (int) nvm->block_size);
    printf("Max Inode Entry   : %u\n", nvm->max_inode_entry);
    printf("\n");
    printf("NVM Start Address : %p\n", nvm);
    printf("Volume Table Addr : %p\n", nvm->volume_table);
    printf("Volume Table Size : %u\n", (unsigned int) sizeof(struct volume_entry) * nvm->max_volume_entry);
    printf("Inode Table Addr  : %p\n", nvm->inode_table);
    printf("Inode Table Size  : %u\n", (unsigned int) sizeof(struct inode_entry) * nvm->max_inode_entry);
    printf("Data Block Table  : %p\n", nvm->datablock_table);
    printf("Block Table Size  : %llu\n", (long long unsigned int) nvm->block_size * 
                                         (long long unsigned int) nvm->max_inode_entry);
//    printf("Volume Table Addr : %llu\n", 
//            (long long unsigned int) nvm->volume_table - (long long unsigned int) nvm);
//    printf("Inode Table Addr  : %llu\n", 
//            (long long unsigned int) nvm->inode_table - (long long unsigned int) nvm);
//    printf("Data Block Table  : %llu\n", 
//            (long long unsigned int) nvm->datablock_table - (long long unsigned int) nvm);
    printf("====================NVM Information====================\n");
}

struct nvm_metadata*
create_nvm_in_shm()
{
    int shm_id;
    struct shmid_ds shm_info;

    printf("Creating NVM in DRAM (shared memory)...\n");

    shm_id = shmget((key_t) SHM_KEY, NVM_SIZE, 0666 | IPC_CREAT);
    if(shm_id == -1) {
        perror("shmget failed: ");
        exit(0);
    }
    shm_addr = shmat(shm_id, (void *)0, 0);
    if(shm_addr == (char*) -1) {
        perror("shmat failed: ");
        exit(0);
    }

    printf("[Succeeded]\n");

    return (struct nvm_metadata*) shm_addr;
}

void remove_nvm_in_shm()
{
    int     shm_id;
    struct  shmid_ds shm_info;

    printf("Removing NVM from DRAM (shared memory)...\n");

    // Make Shared Memory space.
    shm_id = shmget((key_t) SHM_KEY, NVM_SIZE, 0666 | IPC_CREAT);

    if(shm_id == -1) {
        perror("shmget failed : ");
        exit(0);
    }

    // Detach Shared Memory
    if(shmdt(shm_addr) == -1) {
        perror("shmdt failed : ");
        exit(0);
    }

    /* get rid of shared memory allocated space*/
    if(shmctl(shm_id, IPC_RMID, 0) < 0) {
        printf("shmctl error");
        exit(0);
    }

    printf("[Succeeded]\n");
}
