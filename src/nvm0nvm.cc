#include <string>
#include <unistd.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <atomic>
#include "nvm0nvm.h"
#include "nvm0lfqueue.h"
#include "nvm0monitor.h"
#include "nvm0block.h"

//options

//Declarations of global variables
struct nvm_metadata* nvm;
struct monitor monitor;
int sys_terminate;

uint64_t kFlushLwm;

int sync_idx[DEFAULT_NUM_SYNCER];
int clean_idx[DEFAULT_NUM_BALLOON];

/* volume lfqueues */
lfqueue<volume_idx_t>* volume_free_lfqueue;
lfqueue<volume_idx_t>* volume_inuse_lfqueue;

/* inode lfqueues */
lfqueue<inode_idx_t>* inode_free_lfqueue;

lfqueue<inode_idx_t>* inode_dirty_lfqueue[MAX_VOLUME_ENTRY];
atomic<inode_idx_t>   inode_dirty_count;

lfqueue<inode_idx_t>* inode_sync_lfqueue[DEFAULT_NUM_SYNCER];
atomic<uint_fast64_t> sync_queue_idx;

lfqueue<inode_idx_t>* inode_clean_lfqueue[DEFAULT_NUM_BALLOON];
atomic<uint_fast64_t> clean_queue_idx;

/* lpthreads */
pthread_cond_t   g_balloon_cond[DEFAULT_NUM_BALLOON];
pthread_mutex_t  g_balloon_mutex[DEFAULT_NUM_BALLOON];

pthread_t flush_thread[DEFAULT_NUM_FLUSH];
pthread_t sync_thread[DEFAULT_NUM_SYNCER];
pthread_t balloon_thread[DEFAULT_NUM_BALLOON];
pthread_t monitor_thread;

//private function declaration
void recovery_start();
void print_nvm_info();
nvm_metadata* create_nvm_in_shm();
void remove_nvm_in_shm();

/**
 * This function is called ONCE when the NVM system is first created in a system.
 * - NVM data structures
 *     - volume_table
 *     - inode_table
 *     - block_table
 *
 *********************NOTE********************
 * - The space left at the end after the 16KiB alignment will not be utilized.
 *
 *  TODO: Update the table once the design is finalized!
 *  +---------------------------------------------------------------------------------------+
 *  |                                  NON-VOLATILE MEMORY                                  |
 *  +--------------------------+------------------+-----------------------+-----------------+
 *  |         METADATA         |   VOLUME TABLE   |      INODE TABLE      |   BLOCK TABLE   |
 *  +--------------------------+------------------+-----------------------+-----------------+
 *  | nvm_size                 | volume id        | logical block number  | (looks small,   |
 *  | max volume entry         | fd               | state                 |  but actually   |
 *  | max inode entry          | inode root       | its volume entry      |  takes up most  |
 *  | block size               |                  |                       |  of the nvm     |
 *  | volume table             |                  |                       |  space)         |
 *  | inode table              |                  |                       |                 |
 *  | data block table         |                  |                       |                 |
 *  +--------------------------+------------------+-----------------------+-----------------+
 */
void
nvm_structure_build()
{
    /* get the starting address of the non-volatile memory */
    nvm = create_nvm_in_shm();  /* temportarily gets from shm;
                                   later should be modified to possibly
                                   automatically detect nvm space & size */

    /* initialize and divide up the nvm structure */
    nvm->nvm_size           = NVM_SIZE;             /* this could be moved up if system can 
                                                       automatically detects the size of nvm. */
    nvm->max_volume_entry   = MAX_VOLUME_ENTRY;     /* recommended value = nvm_size / avg_file_size */
    nvm->max_ownerless_block= MAX_OWNERLESS_BLOCK;  /* recommended value = MAX_VOLUME_ENTRY */
    nvm->block_size         = BLOCK_SIZE;           /* this should be 16KiB (SSD alignment) */
    
    int unused_end_space    = ((uint64_t)nvm + nvm->nvm_size) % nvm->block_size;   /* yeah, it's complicated */
    uint32_t max_block      =  (  nvm->nvm_size                                         /*  free space  */
                                - sizeof(struct nvm_metadata)                           /* ----over---- */
                                - sizeof(struct volume_entry) * nvm->max_volume_entry   /*  entry size  */
                                - unused_end_space )                                    /* => possible  */
                             / ( sizeof(struct inode_entry) + nvm->block_size );        /*    number of */
                                                                                        /*    entries   */
    nvm->max_inode_entry    = max_block - nvm->max_ownerless_block;

    nvm->volume_table   = (volume_entry*) (nvm + 1);
    nvm->inode_table    = (inode_entry*)  (nvm->volume_table + nvm->max_volume_entry);
    nvm->block_table    = (block_entry*)  (  (uint64_t)(nvm->inode_table + max_block)
                                           - (uint64_t)(nvm->inode_table + max_block) % nvm->block_size
                                           + nvm->block_size );

    //Initialize all inode_entries to state_free
    for (inode_idx_t i = 0; i < nvm->max_inode_entry; i++) {
        nvm->inode_table[i].state = INODE_STATE_FREE;
        nvm->inode_table[i].block_index = i;
    }
    for (volume_idx_t i = 0; i < nvm->max_volume_entry; i++) {
        nvm->volume_table[i].block_index = i + nvm->max_inode_entry;
    }
}

/**
 * This function is called everytime a NVM-system-embedded system is booted.
 * 1. Start the recovery process to recover data in NVM, in case of previous system failure.
 * 2. Starts NVM system
 *     - Create abstract data types to control NVM data structure.
 *         - volume_free_lfqueue
 *         - volume_inuse_lfqueue
 *         - inode_free_lfqueue
 *         - inode_dirty_lfqueue
 *         - inode_clean_lfqueue
 *     - Create necessary locks (mutex lock, rw lock) for concurrency control.
 *     - Create necessary threads to aid NVM system.
 *         - flush_thread
 *         - sync_thread
 *         - balloon_thread
 */
void
nvm_system_init()
{
    //Starts recovery
    printf("Starting Recovery Process...\n");
    recovery_start();
    printf("Recovery successful!\n\n");

    //Starts NVM system
    printf("Starting NVM system...\n");

    //volume data structure
    volume_free_lfqueue =   new lfqueue<volume_idx_t>(nvm->max_volume_entry);
    volume_inuse_lfqueue =  new lfqueue<volume_idx_t>(nvm->max_volume_entry);
    for(volume_idx_t i = 0; i < nvm->max_volume_entry; i++) {
        nvm->volume_table[i].vid = 0x0U - 1;
        volume_free_lfqueue->enqueue(i);
   }

    /* inode lfqueues */
    /* free lfqueue */
    inode_free_lfqueue = new lfqueue<inode_idx_t>(nvm->max_inode_entry);
    /* dirty lfqueue */
    for(volume_idx_t i = 0; i < nvm->max_volume_entry; i++) {
        inode_dirty_lfqueue[i] = new lfqueue<inode_idx_t>(nvm->max_inode_entry);
    }
    inode_dirty_count = 0;    
    /* sync lfqueue */
    for(int i = 0; i < DEFAULT_NUM_SYNCER; i++) {
        inode_sync_lfqueue[i]  = new lfqueue<inode_idx_t>(nvm->max_inode_entry);
    }
    /* clean lfqueue */
    for(int i = 0; i < DEFAULT_NUM_BALLOON; i++) {
        inode_clean_lfqueue[i]  = new lfqueue<inode_idx_t>(nvm->max_inode_entry);
    }

    /* fill free lfqueue */
    for(inode_idx_t i = 0; i < nvm->max_inode_entry; i++) {
        nvm->inode_table[i].state = INODE_STATE_FREE;
        nvm->inode_table[i].lock = PTHREAD_MUTEX_INITIALIZER;
        inode_free_lfqueue->enqueue(i);
    }

    //locks
    for(int i = 0; i < DEFAULT_NUM_BALLOON; i++) {
        pthread_cond_init(&g_balloon_cond[i], NULL);
        pthread_mutex_init(&g_balloon_mutex[i], NULL);
    }

    //create threads
    kFlushLwm = nvm->max_inode_entry * 0.5;
    sys_terminate = 0;
    for(uint32_t i = 0; i < NUM_FLUSH; i++) {
        pthread_create(&flush_thread[i], NULL, flush_thread_func, NULL);
    }
    printf("%d Flush thread created...\n", NUM_FLUSH);

    for(int i = 0; i < DEFAULT_NUM_SYNCER; i++) {
        sync_idx[i] = i;
        pthread_create(&sync_thread[i], NULL, sync_thread_func, (void *)&sync_idx[i]);
    }
    sync_queue_idx = 0;
    printf("%d Sync thread created...\n", DEFAULT_NUM_SYNCER);

    for(int i = 0; i < DEFAULT_NUM_BALLOON; i++) {
        clean_idx[i] = i;
        pthread_create(&balloon_thread[i], NULL, balloon_thread_func, (void *)&clean_idx[i]);
    }
    clean_queue_idx = 0;
    printf("Balloon thread created...\n");

    if(MONITOR_OPTION) {
        pthread_create(&monitor_thread, NULL, monitor_thread_func, NULL);
        printf("monitor thread created...\n");
    }

    printf("NVM system successfully started!\n\n");
}

/**
 * This function is called when a NVM-system-embedded system is shutting down.
 * Thus no user should be calling/running nvm_write at this point.
 * 1. Terminate threads
 *     - flush_thread
 *     - sync_thread
 *     - balloon_thread
 * 2. Close all the files(volumes) opened in NVM system
 * 3. Deallocate data structures
 *     - volume_free_lfqueue
 *     - volume_inuse_lfqueue
 *     - inode_free_lfqueue
 *     - inode_dirty_lfqueue
 *     - inode_sync_lfqueue
 *     - inode_clean_lfqueue
 */
void
nvm_system_close()
{
    //Terminate threads
    sys_terminate = 1;

    //balloon thread - wakes up balloon thread if sleeping
    for(int i = 0; i < DEFAULT_NUM_BALLOON; i++) {
        pthread_mutex_lock(&g_balloon_mutex[i]);
        pthread_cond_signal(&g_balloon_cond[i]);
        pthread_mutex_unlock(&g_balloon_mutex[i]); 
        pthread_join(balloon_thread[i], NULL);
    }

    //flush thread - notify lfqueue to avoid spinlock
    for(uint32_t i = 0; i < NUM_FLUSH; i++) {
        pthread_join(flush_thread[i], NULL);
    }

    //sync thread
    for(int i = 0; i < DEFAULT_NUM_SYNCER; i++) {
        pthread_join(sync_thread[i], NULL);
    }

    if(MONITOR_OPTION) {
        pthread_join(monitor_thread, NULL);
        for(int i = 0; i < MONITORING_AMOUNT + 4; i++)
        {
            printf("\n");
        }
    }

    printf("Closing NVM system...\n");

    //flushes the remained dirty blocks in NVM to a permanent storage
    printf("Flushing NVM system...\n");
    for(volume_idx_t i = 0; i < nvm->max_volume_entry; i++) {
        while(!inode_dirty_lfqueue[i]->is_empty()) {
            inode_idx_t idx = inode_dirty_lfqueue[i]->dequeue();
            inode_entry* inode = &nvm->inode_table[idx];

            lseek(inode->volume->fd, (off_t) nvm->block_size * inode->lbn, SEEK_SET);
            write(inode->volume->fd, nvm->block_table[inode->block_index].data, nvm->block_size);
        }
    }

    sync();
    sync();

    for(inode_idx_t idx = 0; idx < nvm->max_inode_entry; idx++) {
        inode_entry* inode = &nvm->inode_table[idx];
        if(inode->state == INODE_STATE_DIRTY || inode->state == INODE_STATE_SYNC) {
            nvm->inode_table[idx].state = INODE_STATE_CLEAN;
        }
    }

    //close files (volumes)
    printf("Closing volumes in NVM...\n");
    while(!volume_inuse_lfqueue->is_empty()) {
        volume_idx_t idx = volume_inuse_lfqueue->dequeue();
        volume_entry* volume = &nvm->volume_table[idx];
        close(volume->fd);
        free(volume->hash_table);
    }

    //Deallocates data structures
    delete volume_free_lfqueue;
    delete volume_inuse_lfqueue;

    delete inode_free_lfqueue;
    for(volume_idx_t i = 0; i < nvm->max_volume_entry; i++) {
        delete inode_dirty_lfqueue[i];
    }
    for(int i = 0; i < DEFAULT_NUM_SYNCER; i++) {
        delete inode_sync_lfqueue[i];
    }
    for(int i = 0; i < DEFAULT_NUM_BALLOON; i++) {
        delete inode_clean_lfqueue[i];
    }

    printf("NVM system successfully closed!\n\n");
}

/**
 * This function is called when you don't want to use NVM system anymore.
 * Frees NVM space from NVM system */
void
nvm_structure_destroy()
{
    remove_nvm_in_shm();
}

/**
 * Recovers the data in NVM and stores into permanent storage. 
 * 1. Looks through the whole inode table
 *    Finds dirty inode and sync inode
 *    Writes the corresponding data block to its destination
 * 2. sync all at the end (makes the data durable in a permanent storage)
 * 3. set all the dirty nodes to clean (since durably stored in permanent storage now) */
void
recovery_start()
{
    inode_entry* inode;
    inode_idx_t idx;
    uint32_t vid;
    int fd;

    //Write dirty blocks to permanent storage
    for(idx = 0; idx < nvm->max_inode_entry; idx++){
        inode = &nvm->inode_table[idx];
        if(inode->state == INODE_STATE_DIRTY || inode->state == INODE_STATE_SYNC){
            vid = inode->volume->vid;
            fd = open(("VOL_" + to_string(vid) + ".txt").c_str(), O_RDWR | O_CREAT, 0644);
            lseek(fd, nvm->block_size * inode->lbn, SEEK_SET);
            write(fd, nvm->block_table[inode->block_index].data, nvm->block_size);
            close(fd);
        }
    }

    //Sync all
    sync();
    sync();

    //Set all the DIRTY inodes to CLEAN
    for(idx = 0; idx < nvm->max_inode_entry; idx++) {
        inode = &nvm->inode_table[idx];
        if(inode->state == INODE_STATE_DIRTY || inode->state == INODE_STATE_SYNC) {
            nvm->inode_table[idx].state = INODE_STATE_CLEAN;
        }
    }
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
    printf("Data Block Table  : %p\n", nvm->block_table);
    printf("Block Table Size  : %llu\n", (long long unsigned int) nvm->block_size * 
                                         (long long unsigned int) nvm->max_inode_entry);
//    printf("Volume Table Addr : %llu\n", 
//            (long long unsigned int) nvm->volume_table - (long long unsigned int) nvm);
//    printf("Inode Table Addr  : %llu\n", 
//            (long long unsigned int) nvm->inode_table - (long long unsigned int) nvm);
//    printf("Data Block Table  : %llu\n", 
//            (long long unsigned int) nvm->block_table - (long long unsigned int) nvm);
    printf("====================NVM Information====================\n");
}

/*------------------------------------------------------
 *-----allocates shared memory to create a Fake NVM-----
 *------------------------------------------------------*/
#define SHM_KEY 1234
void *shm_addr;

/**
 * Creates a fake NVM space in RAM */
struct nvm_metadata*
create_nvm_in_shm()
{
    int shm_id;

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

    char *page_fault_maker = (char*)shm_addr;
    char page_fault_test;
#define PAGE_FAULT_UNIT_SIZE (1<<12)
    while(page_fault_maker < (char*)shm_addr + NVM_SIZE) {
        page_fault_test = *page_fault_maker;
        page_fault_maker += PAGE_FAULT_UNIT_SIZE;
    }

    return (struct nvm_metadata*) shm_addr;
}

/**
 * Deallocates the fake NVM space in RAM */
void
remove_nvm_in_shm()
{
    int shm_id;

    //printf("Removing NVM from DRAM (shared memory)...\n");

    //Make Shared Memory space.
    shm_id = shmget((key_t) SHM_KEY, NVM_SIZE, 0666 | IPC_CREAT);
    if(shm_id == -1) {
        perror("shmget failed : ");
        exit(0);
    }
    //Detach Shared Memory
    if(shmdt(shm_addr) == -1) {
        perror("shmdt failed : ");
        exit(0);
    }
    //Get rid of shared memory allocated space
    if(shmctl(shm_id, IPC_RMID, 0) < 0) {
        printf("shmctl error");
        exit(0);
    }

    //printf("[Succeeded]\n");
}
