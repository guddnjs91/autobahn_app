#ifndef __NVM_LFQUEUE_
#define __NVM_LFQUEUE_

#include <atomic>
#include <stdint.h>
#include <iostream>

/**
* @class latch-free queue */
template <class T>
class LFqueue {
    
private:

    uint32_t q_size;
    T* queue;
    std::atomic<uint64_t> p_count;
    std::atomic<uint64_t> c_count;
    uint64_t* lp_count;

public:

    LFqueue(uint32_t size = 500);
    ~LFqueue(void);

    void enque(const T& inode);
    T& deque(void);
};

template <class T>
LFqueue<T>::LFqueue(uint32_t size) :
    q_size(size)
{
    queue = new T[q_size];
    p_count = 0;
    c_count = 0;
    lp_count = new uint64_t[q_size];

    for(int i = 0; i < q_size; i++) {
        lp_count[i] = 0;
    }
}

template <class T>
LFqueue<T>::~LFqueue(void)
{

}

template <class T>
void LFqueue<T>::enque(const T& inode)
{
    uint64_t idx = p_count.fetch_add(1);

    //while(queue[(idx+1)%q_size].state == IN_USE) {}

    lp_count[(idx+1)%q_size] = idx + 1;
    queue[(idx+1)%q_size] = inode;

    std::cout << "Enqueued one" << std::endl;
}

template <class T>
T& LFqueue<T>::deque(void)
{
    uint64_t idx = c_count.fetch_add(1);

    while((idx+1) != lp_count[(idx+1)%q_size]) {
        //loop
    }

    std::cout << "Dequeued one" << std::endl;

    return queue[(idx+1)%q_size];
}


#endif
