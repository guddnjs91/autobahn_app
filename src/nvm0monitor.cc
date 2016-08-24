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
    static int counter = 1000;
    printLFQueueGauge();

    if(counter == 1000)
    {
        printThroughput();
        counter = 0;
    }
    counter++;
}

void printLFQueueGauge()
{
    printf("\t\t[free LFQueue]");
    printf("\t\t\t\t[dirty LFQueue]");
    printf("\t\t\t\t[sync LFQueue]");
    printf("\t\t\t\t[clean LFQueue]");
    printf("\n");

    for(int i = 0; i < num_flusher - 1; i++)
    {
        printf("\t\t\t\t        dirty[%2d]", i);
        inode_dirty_lfqueue[i]->monitor();
        printf("\n");
    }

    printf("\t\t\t\t        dirty[%2d]", num_flusher - 1);
    inode_dirty_lfqueue[num_flusher-1]->monitor();
    printf("\tsync[%2d]", MAX_NUM_SYNCER - 2);
    inode_sync_lfqueue[MAX_NUM_SYNCER-2]->monitor();
    printf("\tballoon[%2d]", MAX_NUM_BALLOON - 2);
    inode_clean_lfqueue[MAX_NUM_BALLOON-2]->monitor();
    printf("\n");

    printf("\t");
    inode_free_lfqueue->monitor();

    printf("\t   total]");

    /* sorry for hard coding */
    uint64_t total_size = 0;
    for(volume_idx_t i = 0; i < nvm->max_volume_entry; i++)
    {
        total_size += inode_dirty_lfqueue[i]->get_size();
    }

    double fullness = (double)total_size / (double)nvm->max_inode_entry * 100;

    inode_dirty_lfqueue[0]->coloring(fullness);

    for (int i = 0; i < 20; i++) {
        if(fullness < 5 * i) {
            printf("-");
        } else {
            printf("|");
        }
    }
   
    printf(" %7.3lf %%", fullness);
    printf("\033[0m");

    printf("\tsync[%2d]", MAX_NUM_SYNCER-1);
    inode_sync_lfqueue[MAX_NUM_SYNCER-1]->monitor();

    printf("\tballoon[%2d]", MAX_NUM_BALLOON-1);
    inode_clean_lfqueue[MAX_NUM_BALLOON-1]->monitor();

    printf("\n");


    for(int i = 0; i < num_flusher + 2; i++)
    {
        printf("\033[1A");
    }

    printf("\r");
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

    for(int i = 0; i < num_flusher + 2; i++)
    {
        printf("\n \r");
    }

    printf("\tclean->free: ");
    coloring(free);
    printf("%.2lf GiB/s ", free); 

    printf("\t\t\033[0mfree->dirty: ");
    coloring(dirty);
    printf("%.2lf GiB/s ", dirty); 

    printf("\t\t\033[0mdirty->sync: ");
    coloring(sync);
    printf("%.2lf GiB/s ", sync); 

    printf("\t\t\033[0msync->clean: ");
    coloring(clean);
    printf("%.2lf GiB/s ", clean); 

    printf("\033[0m");

    for(int i = 0; i < num_flusher + 2; i++)
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
