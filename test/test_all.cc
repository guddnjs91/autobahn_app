#include "nvm0common.h"
#include <stdlib.h>
#include <stdio.h>
#include "test.h"

long long unsigned int filesize;
int nthread;
size_t nbytes;
char* buffer;
double* durations;

void test_write(long long unsigned int, int, size_t, int);
void test_durable_write(long long unsigned int, int, size_t, int);
void test_nvm_durable_write(long long unsigned int, int, size_t, int);
void remove_files(int);

void test_write_performance(void (*test_func)(long long unsigned int, int, size_t, int) )
{
    int nthread;

    /* write (WRITE_BYTES1) bytes at a time */
    printf("[Writing %d bytes at a time totaling %llu bytes / %d flushers working]\n", WRITE_BYTES1, TOTAL_FILE_SIZE, NUM_FLUSH_THR);
    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
        printf("#-------- %d threads APPEND TEST -------\n", nthread);
        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES1, _WRITE_APPEND_);
//        printf("#-------------- RANDOM TEST ---------------------\n");
//        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES1, _WRITE_RANDOM_);
        remove_files(nthread);
    }

    printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n");
    getchar();

    /* write (WRITE_BYTES2) bytes at a time */
    printf("[Writing %d bytes at a time totaling %llu bytes / %d flushers working]\n", WRITE_BYTES2, TOTAL_FILE_SIZE, NUM_FLUSH_THR);
    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
        printf("#-------- %d threads APPEND TEST -------\n", nthread);
        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES2, _WRITE_APPEND_);
//        printf("#-------------- RANDOM TEST ---------------------\n");
//        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES2, _WRITE_RANDOM_);
        remove_files(nthread);
    }
}

int main()
{
    system("clear");

    //////////NVM durable write//////////
    printf("\n[nvm durable write test]\n");
    test_write_performance(test_nvm_durable_write);

    printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n\n");
    getchar();
    system("clear");


    //////////write//////////
    printf("\n[write test]\n");
    test_write_performance(test_write);

    printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n\n");
    getchar();
    system("clear");

    //////////durable write//////////
    printf("\n[durable write test]\n");
    test_write_performance(test_durable_write);

    return 0;
}
