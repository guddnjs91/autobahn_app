#pragma pack(1)
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include "config.h"
#include "nvm0common.h"

#define FILE_SIZE 80 * 1024 * 1024 * 1024
#define MAX_THREAD 64

/**
 * Fill in buffer with given size 
 write each element with random characters */
//void
//fill_buf(
//    char *buf,
//    size_t size)
//{
//    int i;
//    for(i = 0; i < (int)size - 1; i++) {
//        if(rand()%10 == 0) {
//            buf[i] = '\n';
//            continue;
//        }
//
//        if(rand()%5 == 0) {
//            buf[i] = ' ';
//        } else {
//            buf[i] = rand() % 26 + 'A';
//        }
//    }
//    
//    buf[size-1] = '\0';
//}
//
///**
// * Write thread fills in buffer and write it to nvm */
//void
//*thread_write_func(
//    void *data)
//{
//    int tid = *((int *)data);
//    char *buf = (char *)malloc(sizeof(char) * MAX_BUF_SIZE);
//
//    /* Test case #1 : Write 512 Bytes at one time */
//    fill_buf(buf, MAX_BUF_SIZE);
//    
//    nvm_atomic_write(tid, 0, buf, MAX_BUF_SIZE);    // change to nvm_write() later..
//
//    return NULL;
//}

void create_files(int n_thread)
{
}

delete_files(int n_thread)
{
}

void create_buffer()
{
}

void test_nvmwrite()
{
    int n_thread, i;

    for( n_thread = 1; n_thread < MAX_THREAD; n_thread*=2) {
        //create (n_thread) files that totals (FILE_SIZE)
        create_files(n_thread);

        //Construct the NVM structure
        nvm_structure_build();

        //start the nvm system
        nvm_system_init();

        //TEST START
        create_buffer();

        //TODO TIMER
        pthread_t write_thread[n_thread];
        int t_id[n_thread];
        for(i=0; i<n_thread; i++) {
            t_id[i] = i+1;
            pthread_create(&write_thread[i], NULL, thread_write_func, (void *)&tid[i]);
        }
                
        //TEST END
        //TODO TIMER
        for(i=0; i<n_thread; i++) {
            pthread_join(write_thread[i], NULL);
        }

        //TODO (FILE_SIZE) file delete
        delete_files(n_thread);

        //terminate nvm system
        nvm_system_close();
    }
}

int main()
{
    test_nvmwrite();
}
