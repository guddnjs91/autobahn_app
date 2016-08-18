#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "ut0lflist.h"
#include <atomic>

using namespace std;

lflist<uint64_t>* g_lflist = new lflist<uint64_t>;
atomic<uint_fast64_t> g_counter;
atomic<uint_fast64_t> p_count;
atomic<uint_fast64_t> c_count;
atomic<uint_fast64_t> r_count;

int timeout = 0;

void test_lflist()
{
    /* Unittest for lflist. */
    int i;
    lflist<uint64_t>* list = new lflist<uint64_t>;
    node<uint64_t>* p = nullptr;
    node<uint64_t>* n = nullptr;

    /* Append 10000 new nodes to lflist. */
    printf("[Append Test]\n");
    for(i = 0; i < 10000; i++) {
        uint64_t data = i;
        node<uint64_t>* new_node = new node<uint64_t>(data);
        list->append(new_node);
    }
    if(list->get_count() == 10000) {
        printf("Append nodes with data 0 ~ 9999 success\n");
    }

    /* Search a node in lflist. */
    printf("[Search Test]\n");
    n = list->search(100, &p);
    if(n != list->get_tail()) {
        printf("search node with data 100 success\n");
    } else {
        printf("search can't find node with data 100\n");
    }
    printf("- found node : %p data : %lu next : %p\n", n, n->get_data(), n->get_next());
    printf("- prev node  : %p data : %lu next : %p\n", p, p->get_data(), p->get_next());
    
    /* Remove 100 nodes in lflist. */
    printf("[Remove Test]\n");
    for (i = 100; i < 200; i++) {
        list->remove(i);
    }
    if(list->get_count() == 9900) {
        printf("Remove nodes with data 100 ~ 199 success\n");
    }

    /* Search a removed node in lflist. */
    n = list->search(150, &p);
    if(n != list->get_tail()) {
        printf("search node with data 150 success\n");
    } else {
        printf("search can't find node with data 150\n");
    }
    printf("-found node : %p data : %lu next : %p\n", n, n->get_data(), n->get_next());
    printf("-prev node  : %p data : %lu next : %p\n", p, p->get_data(), p->get_next());

}

void* reader_func(void* data)
{
    node<uint64_t>* p = nullptr;
    node<uint64_t>* n = nullptr;

    while(!timeout) {
        //uint64_t curr_counter = (uint64_t)g_counter;
        //int d = rand() % (int)curr_counter;
        int d = rand() % 1000 + 1;
        n = g_lflist->search(d, &p);
        if(n->get_data() == d) {
            r_count++;
        }
    }

    return NULL;
}

void* produce_func(void* data)
{
    while(!timeout) {
    
        int64_t d = (int64_t)g_counter++;
        node<uint64_t>* new_node = new node<uint64_t>(d); // data , key 
        g_lflist->append(new_node);
        p_count++;
    }
    
    return NULL;
}

void* consume_func(void* data)
{
    while(!timeout) {
        
        uint64_t curr_counter = (uint64_t)g_counter;
        int d = rand() % (int)curr_counter + 1;
        if (g_lflist->remove(d)) {
            c_count++;
        }
  }

    return NULL;
}

void test_concurrency(void)
{
    int i;
    int nthread = 4;

    printf("[Concurrency Test]\n");
    printf("Producers insert to lflist\n");
    pthread_t producers[nthread];
    for(i = 0; i < nthread; i++) {
        pthread_create(&producers[i], NULL, produce_func, (void*) &i);
    }
    
    printf("Comsumers delete from lflist\n");
    pthread_t consumers[nthread];
    for(i = 0; i < nthread; i++) {
        pthread_create(&consumers[i], NULL, consume_func, (void*) &i);
    }
    
    printf("Readers search data from lflist\n");
    pthread_t readers[nthread];
    for(i = 0; i < nthread; i++) {
        pthread_create(&readers[i], NULL, reader_func, (void*) &i);
    }
    usleep(60 * 1000 * 1000);
    timeout = 1;

    for(i = 0; i < nthread; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
        pthread_join(readers[i], NULL);
    }
    printf("After 1 min...\n");
    printf("inserted : %llu\n", (unsigned long long)p_count);
    printf("deleted  : %llu\n", (unsigned long long)c_count);
    printf("searched : %llu\n", (unsigned long long)r_count);

}

int main(void)
{
    g_counter = 1;
    srand(time(NULL));
    printf("Latch-free linked list test\n");

    test_lflist();

    test_concurrency();

    return 0;
}

