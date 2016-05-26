#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

#define BILLION 1000000000L
#define MAX_BUF_SIZE 8000000

int fd;
char* filename = "VOL0003.txt";
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void fill_buf(char *buf, int size)
{
	int i;
	for(i = 0; i < size; i++)
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
}

void *thread_func(void *data)
{
	int tid = *((int *)data);
	int i, j;
	char buf[MAX_BUF_SIZE];

	/* Test case #3 : Update 125 times at random position */
	for(j = 0; j < 125; j++)
	{
		fill_buf(buf, MAX_BUF_SIZE);
		off_t position = lseek(fd, rand()%(MAX_BUF_SIZE * 124), SEEK_CUR);
		write(fd, buf, strlen(buf));	// change to nvm_write() later..
		printf("Update 8MB occurs at %ld !\n", (long)position);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	long i;
	struct timespec s_time, e_time;
	uint64_t diff;

	srand(time(NULL));

	pthread_t thread[1];
	int tid[1];
	for(i=0; i<1; i++)
		tid[i] = i + 1;
	int status;

	clock_gettime(CLOCK_MONOTONIC, &s_time);
	fd = open(filename, O_WRONLY | O_CREAT, 0644);
	printf("fd : %d\n", fd);
	
	for(i=0; i<1; i++)
		pthread_create(&thread[i], NULL, thread_func, (void *)&tid[i]);

	for(i=0; i<1; i++)
		pthread_join(thread[i], (void **)&status);


	close(fd);
	clock_gettime(CLOCK_MONOTONIC, &e_time);
	diff = BILLION * (e_time.tv_sec - s_time.tv_sec) + (e_time.tv_nsec - s_time.tv_nsec);
	printf("Elapsed Time for Thread : %llu\n", (long long unsigned int)diff);

	return 0;
}

