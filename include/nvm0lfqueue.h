/**
 * nvm0lfqueue.h - header file for nvm0lfqueue.cpp */

#ifndef nvm0lfqueue_h
#define nvm0lfqueue_h

using namespace std;

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
    lfqueue(const uint32_t capacity);
    ~lfqueue();
    void enqueue(const T value);
    T dequeue();
    bool is_empty();
    uint32_t get_size();
    void close();
};

#endif
