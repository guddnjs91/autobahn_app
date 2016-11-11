#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "nvm0nvm.h"
#include "nvm0inode.h"
#include "nvm0monitor.h"

void *monitor_thread_func(void* data);
static void nvm_monitor();
static void printLFQueueGauge();
static void printThroughput();
static void coloring(double state);
static void reset();
static double TimeSpecToSeconds(struct timespec* ts);

void*
monitor_thread_func(
    void* data)
{
    while(sys_terminate == 0)
    {
        usleep(1 * 1000);
        nvm_monitor();
    }
    
    return NULL;
}

void
nvm_monitor()
{
    static int counter = 200;
    printLFQueueGauge();

    if(counter == 200)
    {
        printThroughput();
        counter = 0;
    }
    counter++;
}

void printLFQueueGauge()
{
    printf("    [free LFQueue]                  ");
    printf("    [dirty LFQueue]                 ");
    printf("    [sync LFQueue]                  ");
    printf("    [clean LFQueue]                 ");
    printf("\n");

    for(int i = MONITORING_AMOUNT; i >= 0; i--)
    {
        if(DEFAULT_NUM_FREE - 1 < i) {
                printf("                                    ");
        } else {
            printf("[%2d]", i);
            inode_free_lfqueue[i]->monitor();
            printf("  ");
        }
        if((int)NUM_FLUSH - 1 < i) {
                printf("                                    ");
        } else {
            printf("[%2d]", i);
            inode_dirty_lfqueue[i]->monitor();
            printf("  ");
        }
        if(DEFAULT_NUM_SYNCER - 1 < i) {
                printf("                                    ");
        } else {
            printf("[%2d]", i);
            inode_sync_lfqueue[i]->monitor();
            printf("  ");
        }
        if(DEFAULT_NUM_BALLOON - 1 < i) {
                printf("                                    ");
        } else {
            printf("[%2d]", i);
            inode_clean_lfqueue[i]->monitor();
            printf("  ");
        }
        printf("\n");
    }

    /* sorry for hard coding */
    uint64_t total_size = 0;
    double fullness = 0;

    for(int i = 0; i < DEFAULT_NUM_FREE; i++)
    {
        total_size += inode_free_lfqueue[i]->get_size();
    }
 
    fullness = (double)total_size / (double)nvm->max_inode_entry * 100;

    inode_dirty_lfqueue[0]->coloring(fullness);

    printf("    ");
    for (int i = 0; i < 20; i++) {
        if(fullness < 5 * i) {
            printf("-");
        } else {
            printf("|");
        }
    }
   
    printf(" %7.3lf %%", fullness);
    printf("\033[0m  ");

    total_size = 0;
    for(volume_idx_t i = 0; i < nvm->max_volume_entry; i++)
    {
        total_size += inode_dirty_lfqueue[i]->get_size();
    }
 
    fullness = (double)total_size / (double)nvm->max_inode_entry * 100;
 
    inode_dirty_lfqueue[0]->coloring(fullness);
 
    printf("    ");
    for (int i = 0; i < 20; i++) {
        if(fullness < 5 * i) {
            printf("-");
        } else {
            printf("|");
        }
    }

    printf(" %7.3lf %%", fullness);
    printf("\033[0m");
    printf("\033[0m  ");
 
    total_size = 0;
    for(int i = 0; i < DEFAULT_NUM_SYNCER; i++)
    {
        total_size += inode_sync_lfqueue[i]->get_size();
    }
 
    fullness = (double)total_size / (double)nvm->max_inode_entry * 100;
    inode_dirty_lfqueue[0]->coloring(fullness);
 
    printf("    ");
    for (int i = 0; i < 20; i++) {
        if(fullness < 5 * i) {
            printf("-");
        } else {
            printf("|");
        }
    }
   
    printf(" %7.3lf %%", fullness);
    printf("\033[0m  ");
 
    total_size = 0;
    for(int i = 0; i < DEFAULT_NUM_BALLOON; i++)
    {
        total_size += inode_clean_lfqueue[i]->get_size();
    }

    fullness = (double)total_size / (double)nvm->max_inode_entry * 100;

    inode_dirty_lfqueue[0]->coloring(fullness);

    printf("    ");
    for (int i = 0; i < 20; i++) {
        if(fullness < 5 * i) {
            printf("-");
        } else {
            printf("|");
        }
    }
   
    printf(" %7.3lf %%", fullness);
    printf("\033[0m  ");

    for(int i = 0; i < MONITORING_AMOUNT + 2; i++)
    {
        printf("\033[1A\r");
    }
}


void printThroughput()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double time_interval = 0;
    time_interval = TimeSpecToSeconds(&now) - TimeSpecToSeconds(&monitor.time_recorder);

    //GiB
    #define unitSize (1024 * 1024 * 1024LLU)
    double free = (double)monitor.free.load() * 16 * 1024 / unitSize / time_interval;
    double dirty = (double)monitor.dirty.load() * 16 * 1024 / unitSize / time_interval;
    double sync = (double)monitor.sync.load() * 16 * 1024 / unitSize / time_interval;
    double clean = (double)monitor.clean.load() * 16 * 1024 / unitSize / time_interval;

    for(int i = 0; i < MONITORING_AMOUNT + 3; i++)
    {
        printf("\n \r");
    }

    printf("    clean->free: ");
    coloring(free);
    printf("%.2lf GiB/s         ", free); 

    printf("\033[0m    free->dirty: ");
    coloring(dirty);
    printf("%.2lf GiB/s         ", dirty); 

    printf("\033[0m    dirty->sync: ");
    coloring(sync);
    printf("%.2lf GiB/s         ", sync); 

    printf("\033[0m    sync->clean: ");
    coloring(clean);
    printf("%.2lf GiB/s         ", clean); 

    printf("\033[0m");

    for(int i = 0; i < MONITORING_AMOUNT + 3; i++)
    {
        printf("\033[1A \r");
    }

    reset();
}

void reset()
{
    monitor.free = 0;
    monitor.dirty = 0;
    monitor.clean = 0;
    monitor.sync = 0;
    clock_gettime(CLOCK_MONOTONIC, &monitor.time_recorder);
}

void coloring(double state)
{
    //red
    if(state < 1)
    {
        printf("\033[31m");
    }

    //yellow
    else if(state < 2)
    {
        printf("\033[33m");
    }

    //green
    else if(state < 3)
    {
        printf("\033[32m");
    }

    //blue
    else
    {
        printf("\033[34m");
    }
}

static double TimeSpecToSeconds(struct timespec* ts)
{
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}
