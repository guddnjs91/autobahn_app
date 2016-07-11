#include <stdio.h>
#include "nvm0common.h"

inode_entry inode[100];

int main()
{
    tree_node* root = nullptr;

    for(int i = 0; i < 100; i++)
    {
        inode[i].lbn = i;
        tree_node* t =  alloc_tree_node(&inode[i]);
        root = insert_tree_node(root, t);
    }

    while(1)
    {
        int lbn;
        printf("Search lbn (0 to exit): ");
        scanf("%d", &lbn);
        printf("lbn is %d\n", lbn);
        if(lbn == 0)
            break;
        tree_node* t = search_tree_node(root, lbn);
        if(t == nullptr)
        {
            printf("There are no such tree node\n");
        }
        else
        {
            printf("Searched Tree node lbn : %u\n", t->inode->lbn);
            root = delete_tree_node(root, t);
            printf("Deleted\n");
        }
    }
}
