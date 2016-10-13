#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <algorithm>
#include "test.h"
#include "nvm0nvm.h"

using namespace std;

/* command argument options */
uint64_t NVM_SIZE               = DEFAULT_NVM_SIZE;
uint32_t NUM_FLUSH              = DEFAULT_NUM_FLUSH;
uint32_t SYNC_OPTION            = DEFAULT_SYNC_OPTION;
uint32_t MONITOR_OPTION         = DEFAULT_MONITOR_OPTION;

uint64_t TOTAL_FILE_SIZE        = DEFAULT_TOTAL_FILE_SIZE;
uint32_t TEST_FILE_RANGE_START  = DEFAULT_TEST_FILE_RANGE_START;
uint32_t TEST_FILE_RANGE_END    = DEFAULT_TEST_FILE_RANGE_END;
uint32_t BYTES_PER_WRITE        = DEFAULT_BYTES_PER_WRITE;
uint32_t WRITE_MODE             = DEFAULT_WRITE_MODE;
uint32_t NVM_WRITE              = DEFAULT_NVM_WRITE;
uint32_t TEST_CYCLE             = DEFAULT_TEST_CYCLE;
//string   DATAPATH               = DEFAULT_DATAPATH;

bool reg_test;
bool run;
bool quit;

/* global variables */
uint32_t kNumThread;
double kTimeSec;
char* buffer;
int report_fd;

/* write test functions */
void test_write();
void test_nvm_durable_write();

/* private function declaration */
void print_notice_board();
void handle_user_input();

void start_test();
void start_regression_test();
void start_recording_report();

/**
 *  1. nvm-size
 *  2. num-flush
 *  3. sync
 *  4. monitor
 *  5. total-file-size
 *  6. test-file-range
 *  7. bytes-per-write
 *  8. write-mode
 *  9. nvm-write
 * 10. test-cycle
 * 11. regression-test
 * 12. run
 * 13. quit
 */
void print_notice_board()
{
    string mode;
    if (WRITE_MODE == WRITE_MODE_APPEND) {
        mode = "APPEND";
    } else if (WRITE_MODE == WRITE_MODE_RANDOM) {
        mode = "RANDOM";
    } else if (WRITE_MODE == WRITE_MODE_SKEWED) {
        mode = "SKEWED";
    }

    system("clear");
    printf("\n");
    printf("\t===============================================================================\n");
    printf("\toptions             shortcut   current setting        example\n");
    printf("\tnvm-size              (ns)         %3lu GiB\t  nvm-size=4G, ns 512M\n", NVM_SIZE / 1024 / 1024 / 1024);
    printf("\tnum-flush             (nf)           %3d  \t  num-flush=8, nf 16\n", NUM_FLUSH);
    printf("\tsync                  (s)             %-3s \t  sync=on, s off\n", SYNC_OPTION ? "ON" : "OFF");
    printf("\tmonitor               (m)             %-3s \t  monitor=on, m off\n", MONITOR_OPTION ? "ON" : "OFF");
    printf("\ttotal-file-size       (tfs)        %3lu GiB\t  total-file-size=80G, tfs 512M\n", TOTAL_FILE_SIZE/ 1024 / 1024 / 1024);
    printf("\ttest-file-range       (tfr)        %3d-%-3d\t  test-file-range=16, tfr 4-128\n", TEST_FILE_RANGE_START, TEST_FILE_RANGE_END);
    printf("\tbytes-per-write       (bpw)        %3u KiB\t  bytes-per-write=1M, bpw 16k  \n", BYTES_PER_WRITE / 1024 );
    printf("\twrite-mode            (wm)          %s\t  write-mode=append, wm=r, wm s\n", mode.c_str());
    printf("\tnvm-write             (nw)            %-3s \t  nvm-write=on, nw off\n", NVM_WRITE ? "ON" : "OFF");
    printf("\ttest-cycle            (tc)           %3d  \t  test-cycle=5, tc 10\n", TEST_CYCLE);
  //printf("\tdatapath              (dp)          %s\tdatapath=path, dp=path\n", DATAPATH);
    printf("\t\033[1mregression-test       (rt)                        start regression test\033[0m\n");
    printf("\t\033[1mrun                   (r)                         start testing\033[0m\n");
    printf("\t\033[1mquit                  (q)                         quit testing\033[0m\n");
    printf("\t===============================================================================\n");
}

void handle_user_input()
{
    string option_buf;

    printf("\n\toptions > ");
    getline(cin, option_buf);

    // commands without parameters
    if (option_buf == "run" || option_buf == "r") {
        run = true; 
    } else if (option_buf == "regression-test" || option_buf == "rt") {
        reg_test = true;
    } else if (option_buf == "quit" || option_buf == "q") {
        quit = true;

    // commands with parameters
    } else {
        string option_name;
        string option_value;
   
        size_t pos;
        if ((pos=option_buf.find("=")) == string::npos && (pos=option_buf.find(" ")) == string::npos) {
            ;
        }
        option_name = option_buf.substr(0, pos);
        option_value = option_buf.substr(pos + 1);
        transform(option_value.begin(), option_value.end(), option_value.begin(), ::toupper);

        /*  1. nvm-size */
        if (option_name == "nvm_size" || option_name == "ns") {

            uint64_t mult;

            if ((pos = option_value.find("G")) != string::npos) {
                mult = 1024 * 1024 * 1024LLU;
            } else if ((pos = option_value.find("M")) != string::npos) {
                mult = 1024 * 1024;
            } else {
                return;
            }

            NVM_SIZE = stoi(option_value.substr(0,pos)) * mult;

        /*  2. num-flush */
        } else if (option_name == "num-flush" || option_name == "nf") {

            NUM_FLUSH = stoi(option_value);

        /*  3. sync */
        } else if (option_name == "sync" || option_name == "s") {

            if (option_value == "ON") {
                SYNC_OPTION = 1;
            } else if (option_value == "OFF") {
                SYNC_OPTION = 0;
            } else {
                SYNC_OPTION = 1 - SYNC_OPTION;
            }

        /*  4. monitor */
        } else if (option_name == "monitor" || option_name == "m") {

            if (option_value == "ON") {
                MONITOR_OPTION = 1;
            } else if (option_value == "OFF") {
                MONITOR_OPTION = 0;
            } else {
                MONITOR_OPTION = 1 - MONITOR_OPTION;
            }

        /*  5. total-file-size */
        } else if (option_name == "total-file-size" || option_name == "tfs") {

            uint64_t mult;

            if ((pos = option_value.find("G")) != string::npos) {
                mult = 1024 * 1024 * 1024LLU;
            } else if ((pos = option_value.find("M")) != string::npos) {
                mult = 1024 * 1024;
            } else {
                return;
            }

            TOTAL_FILE_SIZE = stoi(option_value.substr(0,pos)) * mult;

        /*  6. test-file-range */
        } else if (option_name == "test-file-range" || option_name == "tfr") {

            pos = option_value.find("-");
            if (pos == string::npos) {
                TEST_FILE_RANGE_START = stoi(option_value);
                TEST_FILE_RANGE_END   = stoi(option_value);
            } else {
                TEST_FILE_RANGE_START = stoi(option_value.substr(0, pos));
                TEST_FILE_RANGE_END   = stoi(option_value.substr(pos + 1));
            }

        /*  7. bytes-per-write */
        } else if (option_name == "bytes-per-write" || option_name == "bpw") {

            uint64_t mult;

            if ((pos = option_value.find("M")) != string::npos) {
                mult = 1024 * 1024;
            } else if ((pos = option_value.find("K")) != string::npos) {
                mult = 1024;
            } else {
                mult = 1;
            }

            BYTES_PER_WRITE = stoi(option_value.substr(0,pos)) * mult;

        /*  8. write-mode */
        } else if (option_name == "write-mode" || option_name == "wm") {

            if (option_value == "APPEND" || option_value == "A") {
                WRITE_MODE = WRITE_MODE_APPEND;
            } else if (option_value == "RANDOM" || option_value == "R") {
                WRITE_MODE = WRITE_MODE_RANDOM;
            } else if (option_value == "SKEWED" || option_value == "S") {
                WRITE_MODE = WRITE_MODE_SKEWED;
            } else {
                WRITE_MODE = (WRITE_MODE + 1) % 3;
            }

        /*  9. nvm-write */
        } else if (option_name == "nvm-write" || option_name == "nw") {

            if (option_value == "ON") {
                NVM_WRITE = 1;
            } else if(option_value == "OFF") {
                NVM_WRITE = 0;
            } else {
                NVM_WRITE = 1 - NVM_WRITE;
            }

        /* 10. test-cycle */
        } else if (option_name == "test-cycle" || option_name == "tc") {

            TEST_CYCLE = stoi(option_value);

        } else if (option_name == "datapath" || option_name == "dp") {
            
        }
    }
}

void start_test()
{
    system("clear");
    //start_recording_report();
    
    if (NVM_WRITE) {
        printf("=================== NVM WRITE TESTING START ===================\n");
    } else {
        printf("=============== SYSTEM CALL WRITE TESTING START ===============\n");
    }

    for (uint32_t i = 0; i < TEST_CYCLE; i++) {
        //TODO: dprintf cycle (1/5) like this
        for (kNumThread = TEST_FILE_RANGE_START; kNumThread <= TEST_FILE_RANGE_END; kNumThread*= 2) {
            printf(">> %u files start to be written <<\n", kNumThread);
            if (NVM_WRITE) {
                test_nvm_durable_write();
            } else {
                test_write();
            }
        }            
    }
}

/**
 * nvm-size         = 1, 4, 16, 64G
 * num-flush        = 16
 * sync             = on, off
 * monitor          = off
 * total-file-size  = 80G
 * test-file-range  = 1, 4, 16, 64
 * bytes-per-write  = 256, 16K, 1M
 * write-mode       = append, random, skewed
 * nvm-write        = on
 * test-cycle       = 5
 */
void start_regression_test()
{
    int fd = open("./test/regtest.txt", O_RDWR | O_CREAT | O_APPEND, 0666);

    printf("=================== REGRESSION TESTING START ===================\n");
    
    for (NVM_SIZE = 1024 * 1024 * 1024LLU; NVM_SIZE <= 64 * 1024 * 1024 * 1024LLU; NVM_SIZE *= 4) {
        NUM_FLUSH = 16;
        for (SYNC_OPTION = 0; SYNC_OPTION <= 1; SYNC_OPTION++) {
            MONITOR_OPTION = 0;
            TOTAL_FILE_SIZE = 80 * 1024 * 1024 * 1024LLU;
            TEST_FILE_RANGE_START = 1;
            TEST_FILE_RANGE_END = 64;
            for (kNumThread = TEST_FILE_RANGE_START; kNumThread <= TEST_FILE_RANGE_END; kNumThread *= 4) {
                for (BYTES_PER_WRITE = 256; BYTES_PER_WRITE <= 1024 * 1024; BYTES_PER_WRITE *= 64) {
                    for (WRITE_MODE = 0; WRITE_MODE <= 0/*1*/; WRITE_MODE++) {
                        NVM_WRITE = 1;
                        TEST_CYCLE = 5;

                        double time_avg = 0;

                        for(uint32_t i = 0; i < TEST_CYCLE; i++) {
                            printf(">> %u files start to be written <<\n", kNumThread);
                            if(NVM_WRITE) {
                                test_nvm_durable_write();
                            } else {
                                test_write();
                            }

                            time_avg += kTimeSec;
                        }

                        time_avg /= TEST_CYCLE;
                        dprintf(fd, "*************************\n");
                        dprintf(fd, "NVM SIZE        = %lu\n", NVM_SIZE);
                        dprintf(fd, "SYNC OPTION     = %d\n", SYNC_OPTION);
                        dprintf(fd, "NUM FILES       = %u\n", kNumThread);
                        dprintf(fd, "BYTES PER WRITE = %u\n", BYTES_PER_WRITE);
                        dprintf(fd, "WRITE MODE      = %u\n", WRITE_MODE);
                        dprintf(fd, "TIME TAKEN      = %f\n", time_avg);
                        dprintf(fd, "*************************\n");
                    }
                }
            }
        }
    }
    close(fd);
}

void start_recording_report()
{
    time_t timer = time(NULL);
    struct tm *t = localtime(&timer);
    string report_time_YMD = to_string(t->tm_year + 1900) + "-" + to_string(t->tm_mon + 1) + "-" + to_string(t->tm_mday);
    string report_time_HMS = to_string(t->tm_hour) + ":" + to_string(t->tm_min) + ":" + to_string(t->tm_sec);

    report_fd = open( ("./report/" + report_time_YMD + "-" + report_time_HMS).c_str() , O_RDWR | O_CREAT | O_APPEND, 0666 ); 

    //TODO: dprintf important testing options 
}

int main()
{
    do {
        print_notice_board();
        handle_user_input();

        if (run) {
            start_test();
            run = false;
        } else if (reg_test) {
            start_regression_test();
            reg_test = false;
        }
    } while (!quit);
}
