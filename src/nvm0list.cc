#include "nvm0list.h"

/**
 * Constructor for list
 */
struct hash_node_list*
new_list()
{
    struct hash_node_list* list = (struct hash_node_list*) malloc( sizeof(struct hash_node_list) );
    list->head = nullptr;
    list->tail = nullptr;
    list->count = 0;

    return list;
}

/**
 * push_back (enqueue) - Inserts a hash node at the end of a list.
 */
void
push_back_list_node(
    struct hash_node_list* list,
    struct hash_node* node)
{
    if(list->tail == nullptr) {
        node->prev = nullptr;
        node->next = nullptr;

        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        node->prev = list->tail;
        node->next = nullptr;

        list->tail = node;
    }

    list->count++;
}

/**
 * pop_front (dequeue) - Gets the first hash node from the beginning of a list. 
 * @return the first hash node or nullptr if empty. */
struct hash_node*
pop_front_list_node(
    struct hash_node_list* list )
{
    struct hash_node* node = list->head;

    if(node != nullptr) {

        list->head = list->head->next;

        if(list->head == nullptr) {
            list->tail = nullptr;
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
    struct hash_node_list* list,
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

    list->count--;
}
