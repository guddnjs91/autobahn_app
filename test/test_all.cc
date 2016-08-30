#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <algorithm>
#include "test.h"
#include "nvm0nvm.h"

using namespace std;

/* Command argument options */
uint64_t NVM_SIZE               = DEFAULT_NVM_SIZE;
uint32_t NUM_FLUSH              = DEFAULT_NUM_FLUSH;
uint32_t SYNC_OPTION            = DEFAULT_SYNC_OPTION;
uint32_t MONITOR_OPTION         = DEFAULT_MONITOR_OPTION;
uint64_t TOTAL_FILE_SIZE        = DEFAULT_TOTAL_FILE_SIZE;
uint32_t TEST_FILE_RANGE_START  = DEFAULT_TEST_FILE_RANGE_START;
uint32_t TEST_FILE_RANGE_END    = DEFAULT_TEST_FILE_RANGE_END;
uint32_t WRITE_MODE             = DEFAULT_WRITE_MODE;
uint32_t TEST_CYCLE             = DEFAULT_TEST_CYCLE;
uint32_t NVM_WRITE              = DEFAULT_NVM_WRITE;
uint32_t BYTES_PER_WRITE        = DEFAULT_BYTES_PER_WRITE;
uint32_t REGRESSION_TEST        = DEFAULT_REGRESSION_TEST;
//string   DATAPATH               = DEFAULT_DATAPATH;

/////////////////////////////////////////////////////////////
int report_fd;
string report_time_YMD;
string report_time_HMS;
int nthread;
char* buffer;
bool run;
char option_buf[256];

void test_write(int n_thread);
void test_durable_write(int n_thread);
void test_nvm_durable_write(int n_thread);

void print_notice_board();
void get_test_option();
void set_test_option();
void option_checker(string option);
void start_testing();
void remove_files(int);
void start_recording_report();
void reset();

int main()
{
    do {
       print_notice_board();
       get_test_option();
       set_test_option();
    } while ( !run );

    start_testing();
}

void print_notice_board()
{
    system("clear");
    printf("\n\n");
    printf("\t==========================================================================================================\n");
    printf("\toptions               shortcut         now setting      example\n");
    printf("\tnvm-size                (ns)            %3lu GiB    \tnvm-size=4G, ns=512M\n",
            NVM_SIZE / 1024 / 1024 / 1024);
    printf("\ttotal-file-size         (tfs)           %3lu GiB    \ttotal-file-size=80G, tfs=512M\n",
            TOTAL_FILE_SIZE/ 1024 / 1024 / 1024);
    printf("\ttest-file-range         (tfr)           %3d-%-3d     \ttest-file-range=16, tfr=4-128\n",
            TEST_FILE_RANGE_START, TEST_FILE_RANGE_END);
    printf("\tbytes-per-write         (bpw)           %3u KiB    \tbytes-per-write=1M, bpw=16k  \n",
            BYTES_PER_WRITE / 1024 );
    printf("\tnum-flush               (nf)               %-3d      \tnum-flush=8, nf=16\n", NUM_FLUSH);
    printf("\tsync                    (s)                %-3s      \tsync=on, s=off\n", SYNC_OPTION ? "ON" : "OFF");
    printf("\tmonitor                 (m)                %-3s      \tmonitor=on, m=off\n",
            MONITOR_OPTION ? "ON" : "OFF");
    printf("\twrite-mode              (wm)             %s    \twrite-mode=append, write-mode=a, wm=r, wm=random\n",
            WRITE_MODE? "RANDOM" : "APPEND");
    printf("\tnvm-write               (nw)               %-3s      \tnvm-write=on, nw=off\n", NVM_WRITE ? "ON" : "OFF");
    printf("\ttest-cycle              (tc)               %-3d      \ttest-cycle=5, tc=10\n", TEST_CYCLE);
    printf("\tregression-test         (rt)               %-3s      \tregression-test=on rt=off\n",
            REGRESSION_TEST ? "ON" : "OFF");
    //printf("\tdatapath                (dp)             %s     \tdatapath=path, dp=path\n", DATAPATH);
    printf("\t\033[1mrun                     (r)                          \tstart testing\033[0m\n");
    printf("\t=========================================================================================================\n");
}
void get_test_option()
{
    printf("\n\toptions > ");
    fgets(option_buf, sizeof(option_buf), stdin);
}

void set_test_option()
{
    string option;

    for (int i = 0; ; i++) {
       if(option_buf[i] == ' ' || option_buf[i] == '\n') {
           option_checker(option);
           option.clear();

           if (option_buf[i] == '\n') {
               break;
           }
       } else {
           option += option_buf[i];
       }
    }
}

void option_checker(string option)
{
    if (option == "run" || option == "r") {
       run = true; 
    } else {
        size_t pos;
        string option_name;
        string option_value;
        
        pos = option.find("=");
        if(pos == string::npos)
            return;

        option_name = option.substr(0, pos);
        option_value = option.substr(pos + 1);
        transform(option_value.begin(), option_value.end(), option_value.begin(), ::toupper);

        if (option_name == "nvm_size" || option_name == "ns") {
            uint64_t mult;

            pos = option_value.find("G");
            if(pos == string::npos) {
                pos = option_value.find("M");
                if(pos == string::npos) {
                        return;
                } else {
                    mult = 1024 * 1024;
                }
            } else {
                mult = 1024 * 1024 * 1024LLU;
            }

            NVM_SIZE = stoi(option_value.substr(0,pos)) * mult;
        } else if (option_name == "sync" || option_name == "s") {
            if (option_value == "ON") {
                SYNC_OPTION = 1;
            } else if(option_value == "OFF") {
                SYNC_OPTION = 0;
            }
        } else if (option_name == "num-flush" || option_name == "nf") {
            NUM_FLUSH = stoi(option_value);
        } else if (option_name == "monitor" || option_name == "m") {
            if (option_value == "ON") {
                MONITOR_OPTION = 1;
            } else if(option_value == "OFF") {
                MONITOR_OPTION = 0;
            }
        } else if (option_name == "total-file-size" || option_name == "tfs") {
            uint64_t mult;

            pos = option_value.find("G");
            if(pos == string::npos) {
                pos = option_value.find("M");
                if(pos == string::npos) {
                        return;
                } else {
                    mult = 1024 * 1024;
                }
            } else {
                mult = 1024 * 1024 * 1024LLU;
            }

            TOTAL_FILE_SIZE = stoi(option_value.substr(0,pos)) * mult;
        } else if (option_name == "test-file-range" || option_name == "tfr") {
            pos = option_value.find("-");
            if (pos == string::npos) {
               TEST_FILE_RANGE_START = stoi(option_value); 
               TEST_FILE_RANGE_END   = stoi(option_value); 
            } else {
                TEST_FILE_RANGE_START = stoi(option_value.substr(0, pos));
                TEST_FILE_RANGE_END   = stoi(option_value.substr(pos + 1));
            }
        } else if (option_name == "write-mode" || option_name == "wm") {
            if(option_value == "APPEND" || option_value == "A") {
                WRITE_MODE = WRITE_MODE_APPEND;
            } else if(option_value == "RANDOM" || option_value == "R") {
                WRITE_MODE = WRITE_MODE_RANDOM;
            }
        } else if (option_name == "test-cycle" || option_name == "tc") {
            TEST_CYCLE = stoi(option_value);
        } else if (option_name == "nvm-write" || option_name == "nw") {
            if (option_value == "ON") {
                NVM_WRITE = 1;
            } else if(option_value == "OFF") {
                NVM_WRITE = 0;
            }
        } else if (option_name == "bytes-per-write" || option_name == "bpw") {
            uint64_t mult;

            pos = option_value.find("M");
            if(pos == string::npos) {
                pos = option_value.find("K");
                if(pos == string::npos) {
                    mult = 1; 
                } else {
                    mult = 1024;
                }
            } else {
                mult = 1024 * 1024 ;
            }

            BYTES_PER_WRITE = stoi(option_value.substr(0,pos)) * mult;
        } else if (option_name == "datapath" || option_name == "dp") {
            
        } else if (option_name == "regression-test" || option_name == "rt") {
            if (option_value == "ON") {
                REGRESSION_TEST = 1;
            } else if(option_value == "OFF") {
                REGRESSION_TEST = 0;
            }
        }
    }
}

void start_testing()
{
    system("clear");
    //start_recording_report();
    
    if(NVM_WRITE) {
        printf("nvm_write testing start > \n");
    } else {
        printf("system call write() testing start > \n");
    }

    for (uint32_t i = 0; i < TEST_CYCLE; i++) {
        //dprintf cycle (1/5) like this
        for (uint32_t n_thread = TEST_FILE_RANGE_START; n_thread <= TEST_FILE_RANGE_END; n_thread*= 2) {
            printf("%u files start to be written\n", n_thread);
            if(NVM_WRITE) {
                test_nvm_durable_write(n_thread);
            } else {
                test_write(n_thread);
            }
            remove_files(n_thread);
        }            
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
    //dprintf(report_fd, "\n# Testing Start at : %s\n", report_time_HMS.c_str());
    //dprintf(report_fd, "# %llu GiB file write, %d flushers running\n", TOTAL_FILE_SIZE / 1024 / 1024 / 1024, NUM_FLUSH );
    //dprintf(report_fd, "# FREE LFQ: %d SYNC LFQ: %d CLEAN LFQ: %d\n", MAX_NUM_FREE, MAX_NUM_SYNCER, MAX_NUM_BALLOON );
}

double TimeSpecToSeconds(struct timespec* ts)
{
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}
