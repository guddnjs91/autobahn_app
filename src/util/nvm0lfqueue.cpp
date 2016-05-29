#include <atomic>
#include <cstdint>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

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
    atomic<uint_fast64_t>* p_counts;//p_count for each element in a queue

    uint32_t capacity;              //capacity of queue

  public:
    lfqueue(uint32_t);
    ~lfqueue();
    void enqueue(T value);
    T dequeue();
    bool is_empty();
};

/**
 * Constructor
 * Takes capacity as an argument and initializes a queue. */
template <typename T>
lfqueue<T>::lfqueue(uint32_t capacity)
{
    uint32_t i;

    p_count.store(0);                       //initialize p_count
    c_count.store(0);                       //initialize c_count

    values = new T[capacity];               //initialize values[]
    p_counts = new uint_fast64_t[capacity]; //initialize pcounts[]
    for(i=0; i<capacity; i++) {
        p_counts[i].store(0);
    }
    
    this->capacity = capacity;              //initialize capacity
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

//////////////////////////////////TEST_lfqueue////////////////////////////////

void test_queue1()
{
    printf("\nTest 1\n");
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    queue->enqueue(1);
    printf("value dequeued: %d\n", queue->dequeue());

    delete queue;
}

void test_queue10()
{
    printf("\nTest 10\n");
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    int i;
    for(i=0; i<10; i++) {
        queue->enqueue(i);
    }
    for(i=0; i<10; i++) {
        printf("value dequeued: %d\n", queue->dequeue());
    }

    delete queue;
}

void test_is_empty()
{
    printf("\nTest is_empty\n");
    lfqueue<int>* queue = new lfqueue<int>(10);
    
    printf("At first, is_empty() = %d\n", queue->is_empty());
    queue->enqueue(1);
    printf("enqueue 1, is_empty() = %d\n", queue->is_empty());
    queue->dequeue();
    printf("dequeue 1, is_empty() = %d\n", queue->is_empty());

    delete queue;
}

lfqueue<int>* gqueue;
atomic<int> gcount;
int timeout;
int queuesize = 10000;
void* producer(void* what)
{
    while(!timeout) {
        int value = gcount;
        if(value < queuesize) {
            gcount++;
            gqueue->enqueue(value);
            printf("Producer: %d enqueued\n", value);
        }
    }
    printf("producer timeout!\n");
}
void* consumer(void* what)
{
    while(!timeout) {
        gcount--;
        printf("Consumer: %d dequeued\n", gqueue->dequeue());
    }
    printf("consumer timeout!\n");
}

void test_concurrency()
{
    printf("\nTest concurrency(1 producers, 10 consumers)\n");
    gqueue = new lfqueue<int>(queuesize);
    
    int i;
    gcount = 0;
    timeout = 0;
    pthread_t p_thread;
    pthread_create(&p_thread, NULL, producer, NULL);

    pthread_t c_thread[10];
    for(i=0; i<10; i++) {
        pthread_create(&c_thread[i], NULL, consumer, NULL);
    }

    usleep(5000000);
    timeout = 1;

    pthread_join(p_thread, NULL);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    gqueue->enqueue(9999);
    for(i=0; i<10; i++) {
        pthread_join(c_thread[i], NULL);
    }

    delete gqueue;
}

void test_lfqueue()
{
    test_concurrency();
    test_queue1();
    test_queue10();
    test_is_empty();
}

//g++ -std=c++11 -o test_lfqueue nvm0lfqueue.cpp -lpthread
int main()
{
    test_lfqueue();
}
