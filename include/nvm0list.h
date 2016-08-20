#pragma once

#include "nvm0hash.h"
#include <cstdint>

struct hash_node_list
{
    struct hash_node*   head;
    struct hash_node*   tail;
    uint64_t            count;
};

/* functions */
struct hash_node_list* new_list();
void push_back_list_node(struct hash_node_list* list, struct hash_node* node);
struct hash_node* pop_front_list_node(struct hash_node_list* list);
void remove_list_node(struct hash_node_list* list, struct hash_node* node);
