#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include "nvm0common.h"

inode_entry *inodes;
int NUMBER_OF_TEST;
struct hash_table test_hash;
void MakeRandomNumbersOfInodes()
{
    std::cout << "How many inodes do you want to generate?" << std::endl;

    int sizeOfInodes = 0;
    std::cin >> sizeOfInodes;
    NUMBER_OF_TEST = sizeOfInodes;

    int numberSequence[sizeOfInodes];
    inodes = new inode_entry[sizeOfInodes];

    srand(time(nullptr));

    for (int i = 0; i < sizeOfInodes; i++) 
    {
       numberSequence[i] = i; 
    }


    for (int i = 0; i < sizeOfInodes; i++) 
    {
       int RandomNumber = rand() % (sizeOfInodes - i);
       
       inodes[i].lbn = numberSequence[RandomNumber];
       numberSequence[RandomNumber] = numberSequence[sizeOfInodes - i - 1];
    }

}

void test_hash_insert()
{
    printf("---------- insert test start ----------\n");

    for(int i = 0; i < NUMBER_OF_TEST; i++)
    {
        struct hash_node *node = init_hash_node(&inodes[i]);
        insert_hash_node(&test_hash, node);

        std::cout << "lbn:" << node->lbn << " "
            << "count_total:" << test_hash.count_total << "\r";

        if(!search_hash_node(&test_hash,node->lbn))
        {
            printf("\ninsert test failed.\n");
            break;            
        }
    }

    printf("\n---------- insert test done ----------\n");

}

void test_hash_logical_delete()
{
    printf("\n---------- logical delete test start ----------\n");
    
    for(int i = 0; i < NUMBER_OF_TEST; i++)
    {
        int random_index = rand() % NUMBER_OF_TEST;
        struct hash_node *node = init_hash_node(&inodes[random_index]);
        logical_delete_hash_node(&test_hash, node);
        hash_node *searched_node = search_hash_node(&test_hash, node->lbn);

        if(searched_node->valid == HASH_NODE_INVALID)
        {
            printf("lbn:%d, total count :%d, invalid count :%d, deleting...(%6d/%6d)\r",
                    searched_node->lbn, test_hash.count_total, test_hash.count_invalid, i+1, NUMBER_OF_TEST);
        }            

        else
        {
            printf("\nlogical delete test failed.\n");
            break;
        }            
    }

    printf("\n---------- logical delete test done ----------\n");
}

void test_hash_physical_delete()
{
    printf("\n---------- physical delete test start ----------\n");
    
    for(int i = 0; i < NUMBER_OF_TEST; i++)
    {
        int random_index = rand() % NUMBER_OF_TEST;
        struct hash_node *node = init_hash_node(&inodes[random_index]);
            printf("lbn:%d, total count :%d, deleting...(%6d/%6d)\r",
                    node->lbn, test_hash.count_total, i+1, NUMBER_OF_TEST);
        physical_delete_hash_node(&test_hash, node);
        hash_node *searched_node = search_hash_node(&test_hash, node->lbn);

        if(!searched_node)
        {
        }            

        else
        {
            printf("\nphysical delete test failed.\n");
            break;
        }            
    }

    printf("\n---------- physical delete test done ----------\n");
}
int main()
{
    system("clear");
    MakeRandomNumbersOfInodes();
    test_hash_insert();
    test_hash_logical_delete();
    test_hash_physical_delete();

    return 0;
}
