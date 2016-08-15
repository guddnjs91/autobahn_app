#ifndef test_h
#define test_h

#define TOTAL_FILE_SIZE 2 * 1024 * 1024 * 1024LLU
#define MAX_THREADS 128
#define WRITE_BYTES1 16 * 1024
#define WRITE_BYTES2 1024 

#define _WRITE_APPEND_ 1
#define _WRITE_RANDOM_ 2

extern void fill_buf(char *buf, size_t size);
extern void remove_files(int n);

extern long long unsigned int filesize;
extern int nthread;
extern size_t nbytes;
extern char* buffer;
extern double* durations;

#endif
