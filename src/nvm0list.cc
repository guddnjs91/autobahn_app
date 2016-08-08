#include "nvm0common.h"
#include <stdlib.h>

/**
 * Constructor for list
 */
struct list*
list_init()
{
    struct list* list = (struct list*) malloc( sizeof(struct list) );
    list->head = nullptr;
    list->tail = nullptr;
    count = 0;
}

/**
 * push_back (enqueue) - Inserts a hash node at the end of a list.
 */
void
push_back_list_node(
    struct list* list,
    struct hash_node* node )
{
    if(list->tail == nullptr) {
        node->prev = nullptr;
        node->next = nullptr;

        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = nullptr;
        node->prev = list->tail;
        node->next = null;

        list->tail = node;
    }

    list->count++;
}

/**
 * pop_front (dequeue) - Gets the first hash node from the beginning of a list. 
 * @return the first hash node or nullptr if empty. */
struct hash_node*
pop_front_list_node(
    struct list* list )
{
    struct hash_node* node = list->head;

    if(node != nullptr) {

        list->head = list->head->next;

        if(list->head == nullptr) {
            list->tail == nullptr;
        } else {
            list->head->prev = nullptr;
        }

        list->count--;
    }

    return node;
}

/**
 * remove - Removes a given hash node from the list. The given hash node MUST exist in the list.
 */
void
remove_list_node(
    struct list*        list,
    struct hash_node*   node )
{
    if(node == list->head && node == list->tail) {
        list->head = nullptr;
        list->tail = nullptr;
    } else if(node == list->head) {
        list->head = list->head->next;
        list->head->prev = nullptr;
    } else if(node == list->tail) {
        list->tail = list->tail->prev;
        list->tail->next = nullptr;
    } else
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    l->count--;
}

