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

void start_recording_report()
{
    time_t timer;
    struct tm *t;

    timer = time(NULL);
    t = localtime(&timer);
    
    report_time_YMD = to_string(t->tm_year + 1900) + "-" + to_string(t->tm_mon + 1) + "-" + to_string(t->tm_mday);
    report_time_HMS = to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec);

    report_fd = open( ("/opt/nvm1/NVM/report/" + report_time_YMD).c_str() , O_RDWR | O_CREAT | O_APPEND, 0666 ); 
    write(report_fd, ("# Testing Start at : " + report_time_HMS + "\n").c_str(),
    strlen(("Testing Start at : " + report_time_HMS + "\n").c_str())); 
}

int main()
{
    system("clear");

    start_recording_report();

    //////////NVM durable write//////////
    write(report_fd, "\n## [nvm durable write test]\n", strlen("\n## [nvm durable write test]\n")); 
    printf("\n[nvm durable write test]\n");
    test_write_performance(test_nvm_durable_write);

    printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n\n");
    getchar();
    system("clear");


    //////////write//////////
    write(report_fd, "\n## [write test]\n", strlen("\n## [write test]\n")); 
    printf("\n[write test]\n");
    test_write_performance(test_write);

    printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n\n");
    getchar();
    system("clear");

    //////////durable write//////////
    write(report_fd, "\n## [durable write test]\n", strlen("\n## [durable write test]\n")); 
    printf("\n[durable write test]\n");
    test_write_performance(test_durable_write);

    return 0;
}
