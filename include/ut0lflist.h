/*
* This is the header file for latch-free singly linked list. */

#ifndef lflist_h
#define lflist_h

#include <cstdint>
#include <atomic>

using namespace std;

template <typename T> class lflist;

/*
* Node contains specific general-data and next pointer. */
template <typename T>
class node
{
    friend class lflist<T>; // lflist class can access node class members

    private:
        T           data;   // general data in node
        node<T>*    next;   // next pointer

    public:

        /* Constructor */
        node(){
            data = 0;
            next = nullptr;
        };
        node(T input) {
            data = input;
            next = nullptr;
        };

        /* Getter */
        T get_data()        { return data; };
        node<T>* get_next() { return next; };
};

/* Lflist contains head and tail node pointer and count.
*  Class provides three main methods;
* (1) search : traverse and return node containing data if found.
* (2) remove : logically remove certain node if found.
* (3) append : add new node to the end of lflist. */
template <typename T>
class lflist
{
    private:
        node<T>*        head;   // points to first dummy node.
        node<T>*        tail;   // points to last node.
        uint64_t        count;  // size of lflist.

    public:
        
        /* Constructor and Destructor. */
        lflist();
        ~lflist();

        /* Getter */
        node<T>*    get_head()  { return head; }
        node<T>*    get_tail()  { return tail; }
        uint64_t    get_count() { return count; }
        
        /* Main methods.  */
        node<T>*    search(T data, node<T>** left);
        int         remove(T data);
        void        append(node<T>* new_node);
};

#endif
