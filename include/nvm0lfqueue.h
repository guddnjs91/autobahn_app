#pragma once

#include <iostream>
#include <atomic>
#include <cstdint>
#include <stdio.h>
using namespace std;

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/**
 * Concurrent Latch-Free Queue that supports multiple-producers and multiple-consumers.
 * It uses fixed data structure and it can only store limited number of elements.
 * When a queue is full, enqueue will spinlock until it finds a space.
 * When a queue is empty, dequeue will spinlock until it finds a new element.
 * New enqueue might spinlock if the old element at the same index hasn't been dequeued.
 * This happens if the consumer of the old element preempts in the middle of dequeue. */
template <typename T> 
class lfqueue
{
  private:
    atomic<uint_fast64_t>   p_count;    //global producer count
    atomic<uint_fast64_t>   c_count;    //global consumer count

    T*                      values;     //value for each element in a queue
    atomic<uint_fast64_t>*  c_counts;   //c_count for each element in a queue
    atomic<uint_fast64_t>*  p_counts;   //p_count for each element in a queue

    uint32_t                capacity;   //capacity of queue

    bool                    is_closed;  //checker for any remaining spinlock to stop when system ends

  public:
    lfqueue(const uint32_t capacity)
    : is_closed(false)
    {
        uint32_t i;

        p_count.store(capacity-1);   //initialize p_count
        c_count.store(capacity-1);   //initialize c_count

        values   = new T[capacity];                     //initialize values[]
        c_counts = new atomic<uint_fast64_t>[capacity]; //initialize ccounts[]
        p_counts = new atomic<uint_fast64_t>[capacity]; //initialize pcounts[]
        for(i=0; i<capacity; i++) {
            c_counts[i].store(i+capacity);
            p_counts[i].store(i);
        }
        
        this->capacity = capacity;  //initialize capacity
    };

    ~lfqueue()
    {
        delete[] values;
        delete[] c_counts;
        delete[] p_counts;
    };

    void enqueue(const T value)
    {
        //atomically increments the global p_count and saves the incremented value to the local p_count
        uint_fast64_t p_count = this->p_count.fetch_add(1) + 1;

        //waits until the previous value in this element is taken by dequeue
        while ( p_count != c_counts[p_count%capacity].load() ) {
            if(unlikely(is_closed)) {
                return;
            }
        }

        //stores the new value first, and atomically update the pcount of the new element
        values[p_count%capacity] = value;
        p_counts[p_count%capacity].store(p_count);
    };

    T dequeue()
    {
        //atomically increments the global c_count and saves the incremented value to the local c_count
        uint_fast64_t c_count = this->c_count.fetch_add(1) + 1;

        //waits until a new value is added to the (c_count)th element
        while ( c_count != p_counts[c_count%capacity].load() ) {
            if(unlikely(is_closed)) {
                return 0;
            }
        }

        //take out the value and atomically update the ccount and return the value
        T value = values[c_count%capacity];
        c_counts[c_count%capacity].store(c_count+capacity);
        return value;
    };

    uint32_t get_size()
    {
        uint_fast64_t pc = p_count.load();
        uint_fast64_t cc = c_count.load();
        return ((pc-cc) & (0x0U - (pc >= cc)));
    };

    bool is_empty()
    {
        return p_count.load() <= c_count.load();
    };

    void monitor()
    {
        double fullness = (double)get_size() / (double)capacity * 100;

        this->coloring(fullness);

        for (int i = 0; i < 20; i++) {
            if(fullness < 5 * i) {
                std::cout << "-";
            } else {
                std::cout << "|";
            }
        }
       
        printf(" %7.3lf %%", fullness);
        printf("\033[0m");
    };

    void coloring(double state)
    {
        //red
        if(state > 75)
        {
            printf("\033[31m");
        }

        //yellow
        else if(state > 50)
        {
            printf("\033[33m");
        }

        //green
        else if(state > 25)
        {
            printf("\033[32m");
        }

        //blue
        else
        {
            printf("\033[34m");
        }
    };

    void close()
    {
        is_closed = true;
    };
};

