#ifndef __NVM_LFQUEUE_
#define __NVM_LFQUEUE_

#include <atomic>
#include <stdint.h>
#include <iostream>
#include <new>

/**
 * @class latch-free queue */
template <class T>
class LFqueue {
    
private:

    uint32_t q_size; /* <in: size of lfqueue */
    T* queue; /* <out: lfqueue array */
    std::atomic<uint64_t> p_count; /* <out: global producer counter */
    std::atomic<uint64_t> c_count; /* <out: global consumer counter */
    std::atomic<uint64_t>* lp_count; /* <out: local producer counter for each lfqueue element */

public:

    LFqueue(uint32_t size = 500);
    ~LFqueue(void);

    void enqueue(const T& inode);
    T& dequeue(void);
};

/**
 * Constructor of LFqueue */
template <class T>
LFqueue<T>::LFqueue(uint32_t size) :
    q_size(size)
{
    queue = new T[q_size];
    p_count = 0;
    c_count = 0;
    lp_count = new uint64_t[q_size]; // Is it okay??

    for(int i = 0; i < q_size; i++) {
        lp_count[i] = 0;
    }
}

/**
 * Destructor of LFqueue*/
template <class T>
LFqueue<T>::~LFqueue(void)
{

}

/**
 * Producer enqueues the node to lfqueue.
 * Since maximum enqueued nodes are no more than queue size, 
 * there is no need to check when queue is full. */
template <class T>
void LFqueue<T>::enqueue(const T& node)
{
    uint64_t idx = p_count.fetch_add(1); // get the producer count atomically.

    //while(queue[(idx+1)%q_size].state == IN_USE) {}

    queue[(idx+1)%q_size] = node; // node assigned to the given position of queue.
    lp_count[(idx+1)%q_size] = idx + 1; // add local producer counter.

    std::cout << "Enqueued one" << std::endl;
}

/**
 * Consumer dequeues the node in lfqueue.
 * Check if given position from lfqueue is okay to be dequeued.
 * If not, spin-wait for producer to enqueue nodes first. */
template <class T>
T& LFqueue<T>::dequeue(void)
{
    uint64_t idx = c_count.fetch_add(1); // get the consumer count atomatically.

    /* This while-loop checks if lfqueue is not empty */
    while((idx+1) != lp_count[(idx+1)%q_size].load(std::memory_order_seq_cst)) {
        //loop when empty
    }

    std::cout << "Dequeued one" << std::endl;

    return queue[(idx+1)%q_size];
}


#endif
