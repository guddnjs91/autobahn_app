#pragma once
#include <cstdio>

#define MAX_THREADS 128
#define WRITE_BYTES1 (1 << 14)
#define WRITE_BYTES2 (1 << 20)

#define WRITE_MODE_APPEND   (0)
#define WRITE_MODE_RANDOM   (1)

extern void fill_buf(char *buf, size_t size);
extern void remove_files(int n);

extern long long unsigned int filesize;
extern int nthread;
extern size_t nbytes;
extern char* buffer;
extern double* durations;

extern int report_fd;
