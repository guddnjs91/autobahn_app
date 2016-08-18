/*
* This is the Implementation of latch-free singly linked list. */

#include "ut0lflist.h"

#define BITWISE_0       0x7fffffffffffffff // all 1 bit, except the first bit.
#define BITWISE_1       0x8000000000000000 // all 0 bit, except the first bit.

/* Set up the first bit to 1 in 64-bit data. */
inline unsigned long long
set_mark(unsigned long long p)
{
    p |= BITWISE_1;
    return p;
}

/* Get the reference by ignoring mark bit. */
inline unsigned long long
get_ref(unsigned long long p)
{
    return (p & BITWISE_0);
}

/* Get the mark bit. */
inline unsigned long long
get_mark(unsigned long long p)
{
    return (p & BITWISE_1);
}

/*
* Constructor
* Generates a dummy node and make head and tail point that. */
template <typename T>
lflist<T>::lflist()
: count(0)
{
    node<T>* dummy_head = new node<T>();
    head = dummy_head;
    tail = head;
}

/*
* Destructor
* Remove all linked nodes and dummy head node. */
template <typename T>
lflist<T>::~lflist()
{
    /* Remove all linked nodes. */
    node<T>* curr = head->next;
    while(curr != nullptr) {
        node<T>* temp = curr;
        curr = curr->next;
        delete temp;
    }

    delete head;
}

/*
* Search the node that contains data. 
* Search starts from the first of the lflist.
* If we found logically deleted nodes, search 
* physically deletes them while traversing lflist. */
template <typename T>
node<T>*
lflist<T>::search(T data, node<T>** left_node)
{
    node<T>* left_node_next = nullptr;
    node<T>* right_node = nullptr;

    while(1) {

        node<T>* t = head;
        node<T>* t_next = head->next;
        
        /* Traverse lflist while found valid node containing data. */
        while(get_mark((unsigned long long)t_next) || t->data != data) {

            /* If node is valid, record left_node and left_node_next */
            if(!get_mark((unsigned long long)t_next)) {
                *left_node = t;
                left_node_next = t_next;
            }

            t = (node<T>*)get_ref((unsigned long long)t_next);

            if(t == tail || t == nullptr) {
                /* Stop traversing if end of the lflist. */
                break;
            }

            t_next = t->next;
        }

        if(t == nullptr) // append doesn't finished yet. retry
            continue;

        right_node = t;

        /* Check if found node is right node and not marked. */
        if(left_node_next == right_node) {
            if(!get_mark((unsigned long long)right_node->next)) {
                return right_node;
            }
        }
        else {
            /* Clear up logically removed nodes.
            CAS fail retries search again. */
            if(__sync_bool_compare_and_swap(
                &((*left_node)->next), left_node_next, right_node)) {
                if(!get_mark((unsigned long long)right_node->next)) {
                    return right_node;
                }
            }
        }
    }
}

/*
* Remove the nodes. Do nothing if not found.
* Note that Removal is logical. (bit change) */
template <typename T>
int
lflist<T>::remove(T data)
{
    node<T>* pred = nullptr;
    node<T>* curr = nullptr; 
    node<T>* succ = nullptr;

    while(1) {
        /* Search key */
        curr = search(data, &pred);
        
        /* If not found*/
        if(curr->next == nullptr ||curr->data != data) {
            return 0;
        }

        succ = curr->next;

        /* Check if succ has been logically removed. */
        if(!get_mark((unsigned long long)succ)) {
            if(__sync_bool_compare_and_swap(
                &(curr->next), succ, (node<T>*)set_mark((unsigned long long)succ))) {

                __sync_fetch_and_sub(&count, 1);
                return 1;
            }
        }
    }
}

/*
* Append new node to the end of lflist.
* Simply fetch tail pointer and store new node to it.
* Then link old tail's next to new node. */
template <typename T>
void
lflist<T>::append(node<T>* new_node)
{
    /* FAS tail pointer to new node
    and keep the old tail pointer. */
    node<T>* old = __sync_lock_test_and_set(&tail, new_node);

    /* Link old tail's next to new node. */
    old->next = new_node;

    /* Increment lflist size 1 */
    __sync_fetch_and_add(&count, 1);
}

template class node<uint64_t>;
template class lflist<uint64_t>;
