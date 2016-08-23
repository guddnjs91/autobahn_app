#pragma once
#include <atomic>
#include <cstdint>
using namespace std;

struct monitor
{
    atomic<uint_fast64_t> free;
    atomic<uint_fast64_t> dirty;
    atomic<uint_fast64_t> clean;
    atomic<uint_fast64_t> sync;
};

extern struct monitor monitor;
