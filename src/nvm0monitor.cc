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

void*
monitor_thread_func(
    void* data)
{
//    printf("monitor thread running.....\n");

    while(sys_terminate == 0)
    {
        usleep(1 * 1000);
        nvm_monitor();
    }
    
//    printf("monitor thread termintated.....\n");

    return NULL;
}

void
nvm_monitor()
{
    static int counter = 0;
    printLFQueueGauge();
    counter++;

    if(counter == 1000)
    {
        printThroughput();
        counter = 0;
    }
}

void printLFQueueGauge()
{
    printf("\t\t[free LFQueue]");
    printf("\t\t\t\t[dirty LFQueue]");
    printf("\t\t\t\t[sync LFQueue]");
    printf("\t\t\t\t[clean LFQueue]");
    printf("\n");

    for(int i = 0; i < NUM_FLUSH_THR; i++)
    {
        printf("\t\t\t\t        dirty[%d]", i);
        inode_dirty_lfqueue[i]->monitor();
        printf("\n");
    }

    printf("\t");
    inode_free_lfqueue->monitor();

    printf("\t  total]");

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

    printf("\t\t");
    inode_sync_lfqueue->monitor();

    printf("\t\t");
    inode_clean_lfqueue->monitor();

    printf("\n");


    for(int i = 0; i < NUM_FLUSH_THR + 2; i++)
    {
        printf("\033[1A");
    }

    printf("\r");
}


void printThroughput()
{
    //GiB
    #define unitSize (1024 * 1024 * 1024LLU)
    double free = (double)monitor.free.load() * 16 * 1024 / unitSize;
    double dirty = (double)monitor.dirty.load() * 16 * 1024 / unitSize;
    double sync = (double)monitor.sync.load() * 16 * 1024 / unitSize;
    double clean = (double)monitor.clean.load() * 16 * 1024 / unitSize;

    for(int i = 0; i < NUM_FLUSH_THR + 2; i++)
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

    for(int i = 0; i < NUM_FLUSH_THR + 2; i++)
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

