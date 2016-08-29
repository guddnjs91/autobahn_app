#pragma once
#include <atomic>
#include <cstdint>
#include <time.h>
using namespace std;

struct monitor
{
    atomic<uint_fast64_t> free;
    atomic<uint_fast64_t> dirty;
    atomic<uint_fast64_t> clean;
    atomic<uint_fast64_t> sync;

    struct timespec       time_recorder;
};

extern struct monitor monitor;
extern void *monitor_thread_func(void* data);
extern void nvm_monitor();
extern void printLFQueueGauge();
extern void printThroughput();
extern void coloring(double state);
extern void reset();
extern double TimeSpecToSeconds(struct timespec* ts);
