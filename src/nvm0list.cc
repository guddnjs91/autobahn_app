#include "nvm0common.h"
#include <stdlib.h>

/**
Insert hash node to the tail of the list. */
void
push_back_list_node(
    list*       l,
    hash_node*  node)
{
    if(l->tail == nullptr)
    {
        l->head = node;
        l->tail = node;
    }
    else
    {
        node->prev = l->tail;
        l->tail = node;
    }

    l->count++;
}

/**
Get the first hash node from the head of the list. 
@return the first hash node or nullptr if empty. */
hash_node*
pop_front_list_node(
    list* l)
{
    if(l->head == nullptr)
    {
        return nullptr;
    }
    else
    {
        hash_node* node = l->head;
        l->head = l->head->next;
        if(l->head != nullptr)
        {
            l->head->prev = nullptr;
        }
        l->count--;
        return node;
    }

}

/**
Remove hash node from the listi.
@return removed hash node. */
hash_node*
remove_list_node(
    list*       l,
    hash_node*  node)
{
    if(node->prev == nullptr && node->next == nullptr)
    {   // only one element
        l->head = nullptr;
        l->tail = nullptr;
    }
    else if(node->prev == nullptr)
    {
        l->head = node->next;
        node->next->prev = nullptr;
    }
    else if(node->next == nullptr)
    {
        l->tail = node->prev;
        node->prev->next = nullptr;
    }
    else
    {
        node->prev->next = node->next;
    }
    node->prev = nullptr;
    node->next = nullptr;
    l->count--;

    return node;
}

