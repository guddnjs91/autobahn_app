#pragma once
#include <cstdint>
#include <string>

using namespace std;

/* options */
#define DEFAULT_NVM_SIZE              (4 * 1024 * 1024 * 1024LLU)
#define DEFAULT_NUM_FLUSH             (16)
#define DEFAULT_SYNC_OPTION           (1)
#define DEFAULT_MONITOR_OPTION        (1)

#define DEFAULT_TOTAL_FILE_SIZE       (80 * 1024 * 1024 * 1024LLU)
#define DEFAULT_TEST_FILE_RANGE_START (1)
#define DEFAULT_TEST_FILE_RANGE_END   (128)
#define DEFAULT_WRITE_MODE            (0)
#define DEFAULT_TEST_CYCLE            (5)
#define DEFAULT_NVM_WRITE             (1)
#define DEFAULT_BYTES_PER_WRITE       (16 * 1024)
#define DEFAULT_REGRESSION_TEST       (1)
//const string DEFAULT_DATAPATH        ="./opt/";

#define WRITE_MODE_APPEND (0)
#define WRITE_MODE_RANDOM (1)

extern uint64_t TOTAL_FILE_SIZE;
extern uint32_t TEST_FILE_RANGE_START;
extern uint32_t TEST_FILE_RANGE_END;
extern uint32_t WRITE_MODE;
extern uint32_t TEST_CYCLE;
extern uint32_t NVM_WRITE;
extern uint32_t BYTES_PER_WRITE;
extern uint32_t REGRESSION_TEST;
extern string   DATAPATH;

/* data for testing */
extern int nthread;
extern char* buffer;
extern int report_fd;

void fill_buf(char *buf, size_t size);
double TimeSpecToSeconds(struct timespec* ts);
