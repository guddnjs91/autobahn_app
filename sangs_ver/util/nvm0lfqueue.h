#ifndef LFQUEUE_H
#define LFQUEUE_H

using namespace std;

/**
 * Concurrent Latch-Free Queue that supports multiple-producer-multiple-consumer.
 * This queue implementation has a capacity limit for the number of elements in a queue.
 * INVARIANT:
 *   The number of elements enqueued never exceeds the capacity of a queue.
 *   In other words, a queue will never overflow
 *   and enqueue operation will never have a case where it waits for a room for element. */
template <typename T> 
class lfqueue
{
  private:
    atomic<uint_fast64_t> p_count;  //global producer count
    atomic<uint_fast64_t> c_count;  //global consumer count

    T* values;                      //value for each element in a queue
    atomic<uint_fast64_t>* c_counts;//c_count for each element in a queue
    atomic<uint_fast64_t>* p_counts;//p_count for each element in a queue

    uint32_t capacity;              //capacity of queue

  public:
    lfqueue(uint32_t);
    ~lfqueue();
    void enqueue(T value);
    T dequeue();
    bool is_empty();
    uint32_t get_size();
};

#endif
