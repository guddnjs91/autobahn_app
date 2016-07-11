#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <string>
#include "nvm0common.h"

#define FILE_SIZE 80 * 1024 * 1024 * 1024
#define MAX_THREAD 2
#define MAX_BUF_SIZE 1000

char BUFFER[MAX_BUF_SIZE];

void create_files(int n_thread)
{
	for(int i = 1; i <= n_thread; i++)
	{
		creat(get_filename(i), 0644);
	}
}

void delete_files(int n_thread)
{
    for(int i = 1; i <= n_thread; i++)
    {
        remove(get_filename(i));
    }
}

void create_buffer()
{
    int i;
    for(i = 0; i < MAX_BUF_SIZE - 1; i++)
    {
        if(rand()%10 == 0) {
            BUFFER[i] = '\n';
            continue;
        }

        if(rand()%5 == 0) {
            BUFFER[i] = ' ';
        } else {
            BUFFER[i] = rand() % 26 + 'A';
        }
    }
    BUFFER[i] = '\0';

//    printf("%s%n", BUFFER, &n);
//    printf("\nbuffer contains %d bytes\n", n+1);
}

//void test_nvmwrite()
//{
//    int n_thread, i;
//
//    for( n_thread = 1; n_thread < MAX_THREAD; n_thread*=2) {
//        
//        int fd[n_thread];
//
//        //create (n_thread) files that totals (FILE_SIZE)
//        create_files(n_thread);
//
//        //Construct the NVM structure
//        nvm_structure_build();
//
//        //start the nvm system
//        nvm_system_init();
//
//        //TEST START
//        create_buffer();
//
////        //TODO TIMER
////        pthread_t write_thread[n_thread];
////        int t_id[n_thread];
////        for(i=0; i<n_thread; i++) {
////            t_id[i] = i+1;
////            pthread_create(&write_thread[i], NULL, thread_write_func, (void *)&tid[i]);
////        }
////                
////        //TEST END
////        //TODO TIMER
////        for(i=0; i<n_thread; i++) {
////            pthread_join(write_thread[i], NULL);
////        }
//
//        //TODO (FILE_SIZE) file delete
//        delete_files(n_thread);
//
//        //terminate nvm system
//        nvm_system_close();
//    }
//}

void test_create_and_delete_files()
{
    puts("\nUNIT TEST : create_files()");
    create_files(1); //tested
    create_buffer(); //tested
    delete_files(1); //tested
}

void test_alloc_volume_entry_idx()
{
    puts("\n[UNIT TEST] : alloc_volume_entry_idx()");
    nvm_structure_build(); //tested
    nvm_system_init(); //tested
    print_nvm_info(); 
    
    volume_entry* vt = nvm->volume_table;
    
    volume_idx_t i = alloc_volume_entry_idx(1);
    printf("\nAllocated volume entry : %p\n", &vt[i]);
    printf("[vid : %5u | fd : %5d | root: %p]\n", vt[i].vid, vt[i].fd, vt[i].root);

    nvm_system_close(); //tested
}

void test_search_volume_entry_idx()
{
    puts("\n[UNIT TEST] : search_volume_entry_idx()");
    nvm_structure_build(); //tested
    nvm_system_init(); //tested
    
    print_nvm_info(); 
    volume_entry* vt = nvm->volume_table;
    volume_idx_t i = search_volume_entry_idx(1);
    if(i == nvm->max_volume_entry)
        printf("Volume entry cannot be searched\n");

    i = alloc_volume_entry_idx(1);
    printf("Allocated volume entry : %p\n", &vt[i]);
    printf("[vid : %5u | fd : %5d | root: %p]\n", vt[i].vid, vt[i].fd, vt[i].root);
    
    i = search_volume_entry_idx(1);
    printf("searched volume entry : %p\n", &vt[i]);
    printf("[vid : %5u | fd : %5d | root: %p]\n", vt[i].vid, vt[i].fd, vt[i].root);
    
    nvm_system_close(); //tested
}

void test_get_volume_entry_idx()
{
    puts("\n[UNIT TEST] : search_volume_entry_idx()");
    nvm_structure_build(); //tested
    nvm_system_init(); //tested
    print_nvm_info(); 
    volume_entry* vt = nvm->volume_table;
    volume_idx_t i = get_volume_entry_idx(1);
    printf("searched volume entry : %p\n", &vt[i]);
    printf("[vid : %5u | fd : %5d | root: %p]\n", vt[i].vid, vt[i].fd, vt[i].root);
    
    nvm_system_close(); //tested
}

void test_alloc_inode_entry_idx()
{
    puts("\n[UNIT TEST] : alloc_inode_entry_idx()");
    nvm_structure_build(); //tested
    nvm_system_init(); //tested
    print_nvm_info();

    inode_idx_t idx1 = alloc_inode_entry_idx(10);
    inode_idx_t idx2 = alloc_inode_entry_idx(20);
    printf("allocated idx : %u %u\n", idx1, idx2);
    printf("%d %d\n", nvm->inode_table[idx1].lbn, nvm->inode_table[idx2].lbn);
    
    nvm_system_close(); //tested
}

void test_get_inode_entry_idx()
{
    puts("\n[UNIT TEST] : get_inode_entry_idx()");
    nvm_structure_build(); //tested
    nvm_system_init(); //tested
    print_nvm_info(); 
    volume_entry* vt = nvm->volume_table;
    volume_idx_t v = get_volume_entry_idx(1);
    
    inode_idx_t idx1 = get_inode_entry_idx(&vt[v], 30);
    inode_idx_t idx2 = get_inode_entry_idx(&vt[v], 40);
    inode_idx_t idx3 = get_inode_entry_idx(&vt[v], 50);
    inode_idx_t idx4 = get_inode_entry_idx(&vt[v], 30);
    printf("%u %u %u %u\n", idx1, idx2, idx3, idx4);

    nvm_system_close(); //tested
}


int main()
{
    //test_create_and_delete_files(); //OK
    //test_alloc_volume_entry_idx();  //OK
    //test_search_volume_entry_idx(); //OK
    //test_get_volume_entry_idx();    //OK
    //test_alloc_inode_entry_idx();   //OK
    test_get_inode_entry_idx();       //NOT DONE
}
