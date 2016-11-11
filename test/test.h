#pragma once
#include <cstdint>
#include <string>

using namespace std;

/* options */
#define DEFAULT_TOTAL_FILE_SIZE         (80 * 1024 * 1024 * 1024LLU)
#define DEFAULT_TEST_FILE_RANGE_START   (1)
#define DEFAULT_TEST_FILE_RANGE_END     (64)
#define DEFAULT_BYTES_PER_WRITE         (16 * 1024)
#define DEFAULT_WRITE_MODE              (WRITE_MODE_APPEND)
#define DEFAULT_NVM_WRITE               (1)
#define DEFAULT_TEST_CYCLE              (1)
//const string DEFAULT_DATAPATH        ="./opt/";

#define WRITE_MODE_APPEND (0)
#define WRITE_MODE_RANDOM (1)
#define WRITE_MODE_SKEWED (2)

extern uint64_t TOTAL_FILE_SIZE;
extern uint32_t TEST_FILE_RANGE_START;
extern uint32_t TEST_FILE_RANGE_END;
extern uint32_t BYTES_PER_WRITE;
extern uint32_t WRITE_MODE;
extern uint32_t NVM_WRITE;
extern uint32_t TEST_CYCLE;
//extern string   DATAPATH;

/* data for testing */
extern uint32_t kNumThread;
extern double   kTimeSec;
extern char* buffer;
extern int report_fd;

/* utility functions */
void fill_buf(char *buf, size_t size);
void fill_buf_append(char *buf, size_t size);
void fill_buf_random(char *buf, size_t size);
void remove_files(int num_files);
double TimeSpecToSeconds(struct timespec* ts);
