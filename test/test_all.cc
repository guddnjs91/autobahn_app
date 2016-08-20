#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include "test.h"
#include "nvm0nvm.h"
using namespace std;

int report_fd;
FILE *report_fp;
string report_time_YMD;
string report_time_HMS;
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

    for(int i = 0; i < 5; i ++)
    {
    /* write (WRITE_BYTES1) bytes at a time */
    printf("[Writing %d bytes at a time totaling %llu bytes / %d flushers working]\n", WRITE_BYTES1, TOTAL_FILE_SIZE, NUM_FLUSH_THR);
    dprintf(report_fd, "## threads \t|\tinterval(sec)\n");

    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
        printf("#-------- %d threads APPEND TEST -------\n", nthread);
        dprintf(report_fd, "\t%3d", nthread);
        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES1, _WRITE_APPEND_);
//        printf("#-------------- RANDOM TEST ---------------------\n");
//        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES1, _WRITE_RANDOM_);
        remove_files(nthread);
    }
    }

    printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n");
    getchar();

    /* write (WRITE_BYTES2) bytes at a time */
    printf("[Writing %d bytes at a time totaling %llu bytes / %d flushers working]\n", WRITE_BYTES2, TOTAL_FILE_SIZE, NUM_FLUSH_THR);
    dprintf(report_fd, "### \t# of threads \tinterval(sec)\n");

    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
        printf("#-------- %d threads APPEND TEST -------\n", nthread);
        dprintf(report_fd, "\t%3d", nthread);
        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES2, _WRITE_APPEND_);
//        printf("#-------------- RANDOM TEST ---------------------\n");
//        (*test_func)(TOTAL_FILE_SIZE, nthread, WRITE_BYTES2, _WRITE_RANDOM_);
        remove_files(nthread);
    }
}

void start_recording_report()
{
    time_t timer;
    struct tm *t;

    timer = time(NULL);
    t = localtime(&timer);
    
    report_time_YMD = to_string(t->tm_year + 1900) + "-" + to_string(t->tm_mon + 1) + "-" + to_string(t->tm_mday);
    report_time_HMS = to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec);

    report_fd = open( ("./report/" + report_time_YMD).c_str() , O_RDWR | O_CREAT | O_APPEND, 0666 ); 
    dprintf(report_fd, "\n# Testing Start at : %s\n", report_time_HMS.c_str());
    dprintf(report_fd, "# %llu GiB file write, %d flushers running\n", TOTAL_FILE_SIZE / 1024 / 1024 / 1024, NUM_FLUSH_THR );
}

int main()
{
    system("clear");

    start_recording_report();

    //////////NVM durable write//////////
    dprintf(report_fd,"## [nvm durable write test]\n");
    printf("\n[nvm durable write test]\n");
    test_write_performance(test_nvm_durable_write);

    printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n\n");
    getchar();
    system("clear");

    //////////write//////////
    dprintf(report_fd,"## [write test]\n");
    printf("\n[write test]\n");
    test_write_performance(test_write);

    printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n\n");
    getchar();
    system("clear");

    //////////durable write//////////
    dprintf(report_fd,"## [durable write test]\n");
    printf("\n[durable write test]\n");
    test_write_performance(test_durable_write);

    return 0;
}
