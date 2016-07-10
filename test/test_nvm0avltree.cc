#include <stdio.h>
#include "nvm0common.h"

inode_entry inode[100];

int main()
{
    tree_node* root = nullptr;

    for(int i = 0; i < 100; i++)
    {
        inode[i].lbn = i;
        tree_node* t =  allocate_tree_node(&inode[i]);
        root = insert_tree_node(root, t);
    }

    tree_node* t = search_tree_node(root, 30);
    printf("tree node lbn : %u\n", t->inode->lbn);
}
