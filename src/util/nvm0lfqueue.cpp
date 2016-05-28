#include <atomic>
#include <cstdint>

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

    T values[];                     //value for each element in a queue
    uint_fast64_t p_counts[];       //p_count for each element in a queue

    uint32_t capacity;              //capacity of queue

  public:
    lfqueue(uint32_t);
    void enqueue(T value);
    T dequeue();
}

/* Constructor
 * Takes capacity as an argument and initializes a queue. */
template <typename T>
lfqueue<T>::lfqueue(uint32_t capacity)
{
    uint32_t i;

	p_count = 0;                            //initialize p_count
    c_count = 0;                            //initialize c_count

    values = new T[capacity];               //initialize values[]
	p_counts = new uint_fast64_t[capacity]; //initialize pcounts[]
	for(i=0; i<capacity; i++) {
		p_counts[i] = 0;
	}
    
	this->capacity = capacity;              //initialize capacity
}

/**
 * Enqueues an element with given value in a queue.
 * Note there is an invariant: The number of elements enqueued never exeeds the capacity of a queue. */
template <typename T>
void lfqueue<T>::enqueue(const T value)
{
	uint_fast64_t p_count = (this->p_count++) + 1;  //atomic operation

	values[p_count%capacity] = value;
	p_counts[p_count%capacity] = p_count;
}

/**
 * Dequeues and returns the value of an element in a queue. 
 * @return a value of an element dequeued */
template <typename T>
T lfqueue<T>::dequeue()
{
	uint_fast64_t c_count = (this->c_count++) + 1;   //atomic operation

	while ( c_count != p_counts[c_count%size].p_count) {
        __sync_synchronize();
    }

	return values[c_count%capacity];
}

