#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include "nvm0common.h"

void* monitor_thread_func(void* data);
void nvm_monitor();
void printLFQueueGauge();
void printThroughput();
void coloring(double state);
void reset();


void*
monitor_thread_func(
    void* data)
{
//    printf("monitor thread running.....\n");

    while(sys_terminate == 0)
    {
        usleep(1000 * 1000);
        nvm_monitor();
    }
    
//    printf("monitor thread termintated.....\n");

    return NULL;
}

void
nvm_monitor()
{
    printLFQueueGauge();
    printThroughput();
    printf("\n");
    reset();
}

void printLFQueueGauge()
{
    printf("\t[clean LFQueue]");
    printf("   \t[free LFQueue]");
    printf("   \t[dirty LFQueue]");
    printf("   \t[sync LFQueue]");
    printf("\n");
    
    printf("\t");
    inode_clean_lfqueue->monitor();

    printf("\t");
    inode_free_lfqueue->monitor();

    printf("\t");
    inode_dirty_lfqueue->monitor();

    printf("\t");
    inode_sync_lfqueue->monitor();
    printf("\n");
}


void printThroughput()
{
    //GiB
    #define unitSize (1024 * 1024 * 1024LLU)
    double free = (double)monitor.free.load() * 16 * 1024 / unitSize;
    double dirty = (double)monitor.dirty.load() * 16 * 1024 / unitSize;
    double sync = (double)monitor.sync.load() * 16 * 1024 / unitSize;
    double clean = (double)monitor.clean.load() * 16 * 1024 / unitSize;

    printf("\tfree: ");
    coloring(free);
    printf("%.2lf GiB/s ", free); 

    printf("\t\033[0mdirty: ");
    coloring(dirty);
    printf("%.2lf GiB/s ", dirty); 

    printf("\t\033[0msync: ");
    coloring(sync);
    printf("%.2lf GiB/s ", sync); 

    printf("\t\033[0mclean: ");
    coloring(clean);
    printf("%.2lf GiB/s ", clean); 

    printf("\033[0m");
    printf("\n");
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

