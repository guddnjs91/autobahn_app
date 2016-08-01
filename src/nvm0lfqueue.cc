#include <atomic>
#include <cstdint>
#include "nvm0lfqueue.h"

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

/**
 * Constructor
 * Takes capacity as an argument and initializes a queue. */
template <typename T>
lfqueue<T>::lfqueue(const uint32_t capacity)
: is_closed(false)
{
    uint32_t i;

    p_count.store(capacity-1);   //initialize p_count
    c_count.store(capacity-1);   //initialize c_count

    values = new T[capacity];                       //initialize values[]
    c_counts = new atomic<uint_fast64_t>[capacity]; //initialize ccounts[]
    p_counts = new atomic<uint_fast64_t>[capacity]; //initialize pcounts[]
    for(i=0; i<capacity; i++) {
        c_counts[i].store(i+capacity);
        p_counts[i].store(i);
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
    delete[] c_counts;
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

    //waits until the previous value in this element is taken by dequeue
    while ( p_count != c_counts[p_count%capacity].load() ) {
        if(unlikely(is_closed))
        {
            return;
        }
    }

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
    while ( c_count != p_counts[c_count%capacity].load() ) {
        if(unlikely(is_closed))
        {
            return 0;
        }
    }

    //take out the value and atomically update the ccount and return the value
    T value = values[c_count%capacity];
    c_counts[c_count%capacity].store(c_count+capacity);
    return value;
}
/**
 * Checks if a queue is empty.
 * @return true if a queue is empty, false otherwise */
template <typename T>
bool lfqueue<T>::is_empty()
{
    return p_count.load() <= c_count.load();
}

/**
 * Calculate and returns the size of a queue.
 * @return the size of a queue. */
template <typename T>
uint32_t lfqueue<T>::get_size()
{
    uint_fast64_t pc = p_count.load();
    uint_fast64_t cc = c_count.load();
    return ((pc-cc) & (0x0U - (pc >= cc)));
}

template <typename T>
void lfqueue<T>::monitor()
{
    double fullness = (double)get_size() / (double)capacity * 100;

    for (int i = 0; i < 50; i++) 
    {
        if(fullness < 2 * i)
            std::cout << "-";
        else
            std::cout << "|";
    }

    std::cout << "  " << fullness << "% \r"; 
}

template <typename T>
bool lfqueue<T>::isQuiteFull()
{
    double fullness = (double)get_size() / (double)capacity * 100;

    return fullness > 70 ? true : false; 
}

template <typename T>
bool lfqueue<T>::isQuiteEmpty()
{
    double fullness = (double)get_size() / (double)capacity * 100;

    return fullness < 10 ? true : false; 
}

template <typename T>
void lfqueue<T>::close()
{
    is_closed = true;
}
