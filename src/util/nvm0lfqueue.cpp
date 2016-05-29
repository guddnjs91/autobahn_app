#include <atomic>
#include <cstdint>
#include "nvm0lfqueue.h"

/**
 * Constructor
 * Takes capacity as an argument and initializes a queue. */
template <typename T>
lfqueue<T>::lfqueue(uint32_t capacity)
{
    uint32_t i;

    p_count.store(0);   //initialize p_count
    c_count.store(0);   //initialize c_count

    values = new T[capacity];                       //initialize values[]
    p_counts = new atomic<uint_fast64_t>[capacity]; //initialize pcounts[]
    for(i=0; i<capacity; i++) {
        p_counts[i].store(0);
    }
    
    this->capacity = capacity;  //initialize capacity
}

/**
 * Destructor
 * Deletes the dynamically allocated arrays: values and p_counts. */
template <typename T>
lfqueue<T>::~lfqueue()
{
    delete[] values;
    delete[] p_counts;
}

/**
 * Enqueues an element with given value in a queue.
 * Note there is an invariant: The number of elements enqueued never exeeds the capacity of a queue. */
template <typename T>
void lfqueue<T>::enqueue(const T value)
{
    //atomically increments the global p_count and saves the incremented value to the local p_count
    uint_fast64_t p_count = this->p_count.fetch_add(1) + 1;

    //stores the new value first, and atomically update the pcount of the new element
    values[p_count%capacity] = value;
    p_counts[p_count%capacity].store(p_count);
}

/**
 * Dequeues and returns the value of an element in a queue. 
 * @return a value of an element dequeued */
template <typename T>
T lfqueue<T>::dequeue()
{
    //atomically increments the global c_count and saves the incremented value to the local c_count
    uint_fast64_t c_count = this->c_count.fetch_add(1) + 1;

    //waits until a new value is added to the (c_count)th element
    while ( c_count != p_counts[c_count%capacity].load()) {
        //do nothing
    }
    return values[c_count%capacity];
}
/**
 * Checks if a queue is empty.
 * @return true if a queue is empty, false otherwise */
template <typename T>
bool lfqueue<T>::is_empty()
{
    return p_count.load() <= c_count.load();
}
