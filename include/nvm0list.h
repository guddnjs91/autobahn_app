#ifndef nvm0list_h
#define nvm0list_h

#include "nvm0hash.h"

struct hash_node_list
{
    struct hash_node*   head;
    struct hash_node*   tail;
    uint64_t            count;
};

#endif
