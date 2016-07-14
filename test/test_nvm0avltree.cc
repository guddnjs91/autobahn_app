#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "nvm0common.h"

#define NUMBER_OF_TEST 10000

inode_entry inode[NUMBER_OF_TEST];

void test_tree_insert(struct tree_root *tree)
{
    printf("---------- insert test start ----------\n");

    //initialize inode_entry with random number without duplication
    for(int i = 0; i < NUMBER_OF_TEST; i++)
    {
        again:

        inode[i].lbn = rand() % NUMBER_OF_TEST;

        for(int j = 0; j < i; j++)
        {
            if(inode[i].lbn == inode[j].lbn)
                goto again;
        }

        printf("inodes generating...(%6d/%6d)\r", i+1, NUMBER_OF_TEST);       
    }

    printf("\n");

    for(int i = 0; i < NUMBER_OF_TEST; i++)
    {
        struct tree_node *node = init_tree_node(&inode[i]);
        insert_tree_node(tree, node);

        printf("inserting nodes in tree...(%6d/%6d)\r", i+1, NUMBER_OF_TEST);

        if(!search_tree_node(tree,node->lbn))
        {
            printf("\ninsert test failed.\n");
            break;            
        }
    }

    printf("\n---------- insert test done ----------\n");

}


void test_tree_logical_delete(struct tree_root *tree)
{
    printf("\n---------- logical delete test start ----------\n");
    
    for(int i = 0; i < NUMBER_OF_TEST; i++)
    {
        int random_index = rand() % NUMBER_OF_TEST;
        struct tree_node *node = init_tree_node(&inode[random_index]);
        logical_delete_tree_node(tree, node);
        tree_node *searched_node = search_tree_node(tree, node->lbn);

        if(searched_node->valid == TREE_INVALID)
        {
            printf("logical deleting node in tree...(%6d/%6d)\r", i+1, NUMBER_OF_TEST);
        }            

        else
        {
            printf("\nlogical delete test failed.\n");
            break;
        }            
    }

    printf("\n---------- logical delete test done ----------\n");
}

/*
void test_tree_physical_delete(struct tree_root *tree)
{
    printf("\n---------- physical delete test start ----------\n");
    
    for(int i = 0; i < NUMBER_OF_TEST; i++)
    {
        int random_index = rand() % NUMBER_OF_TEST;
        struct tree_node *node = init_tree_node(&inode[random_index]);
        tree->root = delete_tree_node(tree, node);
        tree_node *searched_node = search_tree_node(tree, node->lbn);

        if(!searched_node)
        {
            printf("physical deleting node in tree...(%6d/%6d)\r", i+1, NUMBER_OF_TEST);
        }            

        else
        {
            printf("\nphysical delete test failed.\n");
            break;
        }            
    }

    printf("\n---------- physical delete test done ----------\n");
}
*/

int main()
{
    struct tree_root TestTree;
    TestTree.root = NULL;

    system("clear");
    test_tree_insert(&TestTree);
    test_tree_logical_delete(&TestTree);
    //test_tree_physical_delete(&TestTree);

    return 0;
}
