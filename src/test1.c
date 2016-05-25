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

unsigned int MAX_BUF_SIZE = 512*1024*1024;

extern NVM_metadata* NVM;

void* thread_sync_func(void* data)
{
    while(1)
    {
        while(NVM->SYNC_INODE_LIST_HEAD == NULL)
        {
            // No data blocks to sync
            sched_yield();
        }

        nvm_sync();
    }

    return NULL;
}

void fill_buf(char *buf, size_t size)
{
	int i;
	for(i = 0; i < size - 1; i++)
	{
		if(rand()%10 == 0)
		{
			buf[i] = '\n';
			continue;
		}
		if(rand()%5 == 0)
			buf[i] = ' ';
		else
			buf[i] = rand() % 26 + 'A';
	}

        buf[size-1] = '\0';
}

void *thread_write_func(void *data)
{
	int tid = *((int *)data);
	char *buf = (char *)malloc(sizeof(char) * MAX_BUF_SIZE);

	/* Test case #1 : Write 512 Bytes at one time */
	fill_buf(buf, MAX_BUF_SIZE);

        nvm_atomic_write(tid, 0, buf, MAX_BUF_SIZE);	// change to nvm_write() later..

	return NULL;
}

int main(int argc, char * argv[])
{
	srand(time(NULL));
	
	int		i;
	int		shmid;
	void	*shm_addr = (void *)0;

//////////////////////////////////////////////////////////////////////////////////////////////

	/* This part is the INITIALIZE part.
	   Making Shared Memory, Allocating, Setting up */
	
	// Get Shared Memory space with key
	printf("\n");
	printf("Getting Shared Memory with key %d ..", SHM_KEY);
	shmid = shmget((key_t)SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT);
	if(shmid == -1)
	{
		perror("shmget failed : ");
		exit(0);
	}
	else
	{
		printf("[Succeeded]\n");
	}

	// Attach Shared Memory to the Process
	printf("Allocating Shared Memory with size %d Bytes ..", SHM_SIZE);
	shm_addr = shmat(shmid, (void *)0, 0);
	if(shm_addr == (void *)-1)
	{
		perror("shmat failed : ");
		exit(0);
	}
	else
	{
		printf("[Succeeded]\n");
		printf("Shared Memory space starts from [%p] to [%p]\n",
				(char *)shm_addr, (char *)shm_addr+SHM_SIZE);
	}
	
////////////////////////////////////////////////////////////////////////////////////////////

	/* Initialize start address of shared memory */
	init_nvm_address(shm_addr);

	print_nvm_info();

	pthread_t sync_thread;  // sync thread always runs 
	int status;

	pthread_create(&sync_thread, NULL, thread_sync_func, NULL);
        
        // Multi-writer
	pthread_t write_thread[1];
	int tid[1];
	for(i=0; i<1; i++)
		tid[i] = i + 1;

	for(i=0; i<1; i++)
		pthread_create(&write_thread[i], NULL, thread_write_func, (void *)&tid[i]);

	for(i=0; i<1; i++)
		pthread_join(write_thread[i], (void **)&status);
	
	print_nvm_info();

	pthread_join(sync_thread, (void**)&status);

	return 0;
}
