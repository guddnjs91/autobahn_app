#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include "test.h"
#include "nvm0nvm.h"
#include <getopt.h>

using namespace std;

/* Command argument options */
long long unsigned int total_file_size = DEFAULT_FILE_SIZE;
int num_thread      = DEFAULT_NUM_THREAD;
int num_flusher     = DEFAULT_NUM_FLUSH;
int verbose_flag    = MONITOR_OFF;
int sync_flag       = SYNC_ON;
int write_mode      = WRITE_MODE_APPEND;

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
int  handle_cmd_arguments(int, char**);
void usage(void);

void test_write_performance(void (*test_func)(long long unsigned int, int, size_t, int) )
{
    int nthread;

    /* write (WRITE_BYTES1) bytes at a time */
    printf("[Writing %d bytes at a time totaling %llu bytes / %d flushers working]\n", WRITE_BYTES1, total_file_size, num_flusher);
    dprintf(report_fd, "## threads \t|\tinterval(sec)\n");

    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
        printf("#-------- %d threads APPEND TEST -------\n", nthread);
        dprintf(report_fd, "\t%3d", nthread);
        (*test_func)(total_file_size, nthread, WRITE_BYTES1, WRITE_MODE_APPEND);
//        printf("#-------------- RANDOM TEST ---------------------\n");
//        (*test_func)(total_file_size, nthread, WRITE_BYTES1, WRITE_MODE_RANDOM);
        remove_files(nthread);
    }

    printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n");
    getchar();

    /* write (WRITE_BYTES2) bytes at a time */
    printf("[Writing %d bytes at a time totaling %llu bytes / %d flushers working]\n", WRITE_BYTES2, total_file_size, num_flusher);
    dprintf(report_fd, "### \t# of threads \tinterval(sec)\n");

    for(nthread = 1; nthread <= MAX_THREADS; nthread*=2) {
        printf("#-------- %d threads APPEND TEST -------\n", nthread);
        dprintf(report_fd, "\t%3d", nthread);
        (*test_func)(total_file_size, nthread, WRITE_BYTES2, WRITE_MODE_APPEND);
//        printf("#-------------- RANDOM TEST ---------------------\n");
//        (*test_func)(total_file_size, nthread, WRITE_BYTES2, WRITE_MODE_RANDOM);
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
    dprintf(report_fd, "# %llu GiB file write, %d flushers running\n", total_file_size / 1024 / 1024 / 1024, num_flusher );
}

int main(int argc, char** argv)
{
    system("clear");

    if(argc > 1) {

        if (handle_cmd_arguments(argc, argv) == -1) {
            usage();
            return -1;
        }

        start_recording_report();

        //////////NVM durable write//////////
        dprintf(report_fd, "\n## [nvm durable write test]\n"); 
        printf("\n[nvm durable write test]\n");
        printf("[Writing %d bytes at a time totaling %llu bytes / %d flushers working]\n", WRITE_BYTES1, total_file_size, num_flusher);
        test_nvm_durable_write(total_file_size, num_thread, WRITE_BYTES1, write_mode);
        remove_files(num_thread);

        printf("\n\n**PRESS ENTER TO CONTINUE TO NEXT TEST**\n");
        getchar();
        
        printf("[Writing %d bytes at a time totaling %llu bytes / %d flushers working]\n", WRITE_BYTES2, total_file_size, num_flusher);
        test_nvm_durable_write(total_file_size, num_thread, WRITE_BYTES2, write_mode);
    }
    else {
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
    }

    return 0;
}

int handle_cmd_arguments(int argc, char** argv)
{
    while(1) {
        static struct option long_options[] = 
        {
            /* monitoring option */
            {"verbose", no_argument, 0, 'v'},
            /* sync option */
            {"sync", required_argument, 0, 's'},
            /* mode option */
            {"mode", required_argument, 0, 'm'},
            /* file size option */
            {"total_size", required_argument, 0, 't'},
            /* number of writer thread option */
            {"num_thread", required_argument, 0, 'n'},
            /* number of flusher thread option */
            {"flusher", required_argument, 0, 'f'},
            {0, 0, 0, 0}
        };

        int c;
        string str;
        long long unsigned int mult = 1;
        int option_index = 0;
        c = getopt_long(argc, argv, "vs:m:t:f:n:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        switch(c) {
            case 0:
                printf("what is this case ??\n");
                if (long_options[option_index].flag != 0)
                    break;
            /* monitoring option. */
            case 'v':
                printf("verbose option : %d\n", verbose_flag);
                verbose_flag = 1;
                printf("verbose option : %d\n", verbose_flag);
                break;

            /* sync option. */
            case 's':
                str = optarg;
                if (str == "yes" || str == "Y" || str == "y") {
                    sync_flag = 1;
                } 
                else if (str == "no" || str == "N" || str == "n") {
                    sync_flag = 0;
                }
                printf("sync flag : %d\n", sync_flag);
                break;

            /* write mode : append or random. */
            case 'm':
                str = optarg;
                if(str == "random" || str == "R" || str == "r") {
                    write_mode = WRITE_MODE_RANDOM;
                }
                printf("write mode : %d\n", write_mode);
                break;

            /* total file size option */
            case 't':
                str = optarg;
                if (str.back() == 'G' || str.back() == 'g') { // GiB
                    mult = 1024 * 1024 * 1024;
                }
                else if (str.back() == 'M' || str.back() == 'm') { // MiB
                    mult = 1024 * 1024; 
                }
                else if (str.back() == 'K' || str.back() == 'k') { // KiB
                    mult = 1024; 
                }
                str.pop_back();
                total_file_size = (long long unsigned int)(stod(str) * mult);
                printf("total file size : %llu\n", total_file_size);
                break;

            /* number of write threads. */
            case 'n':
                num_thread = atoi(optarg);
                printf("thread number = %d\n", num_thread);
                break;

            /* number of flush threads. */
            case 'f':
                num_flusher = atoi(optarg);
                printf("flush thread number = %d\n", num_flusher);
                break;

            case '?':
                printf("Unknown option argument\n");
                return -1;
        }
    }

    return 0;
}

void usage(void)
{
    printf("Usage : ./bin/test/ [opts]\n");
    printf("Possible options \n");
    printf("1. total size : '-t' or '--total_size'\t\t");
    printf(" - You can define file size by typing '--total_size=20G' (Default : 20G)\n");
    printf("2. num thread : '-n' or '--num_thread'\t\t");
    printf(" - You can define thread number by typing '--num_thread=8' (Default : 1)\n");
    printf("3. flusher : '-f' or '--flusher'\t\t");
    printf(" - You can define flusher number by typing '--flusher=4' (Default : 8)\n");
    printf("4. sync option : '-s' or '--sync' \t\t");
    printf(" - You can rule out sync option by typing '--sync=no' (Default : yes)\n");
    printf("5. write mode option : '-m' or '--mode'\t\t");
    printf(" - You can test random write test by typing '--mode=random' (Default : append)\n");
    printf("6. verbose option : '-v' or '--verbose'\t\t");
    printf(" - Print out system progress status. (default : no monitoring)\n");
}
