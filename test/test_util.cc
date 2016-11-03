#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include "nvm0volume.h"

using namespace std;

/**
 * Fill in buffer with given size 
 * write each element with random characters
 */
void fill_buf(char *buf, size_t size)
{
    srand(time(NULL));

    for (int i = 0; i < (int)size - 1; i++) {
       if (rand() % 5 == 0) {
            buf[i] = ' ';
        } else {
            buf[i] = rand() % 26 + 'A';
        }
    }
    
    buf[size-1] = '\n';
}

/**
 * Fill in buffer with given size 
 * append mode fills all 'A'
 */
void fill_buf_append(char *buf, size_t size)
{
    for (int i = 0; i < (int)size - 1; i++) {
        buf[i] = 'A';
    }
    
    buf[size-1] = '\n';
}

/**
 * Fill in buffer with given size 
 * random mode fills all 'B'
 */
void fill_buf_random(char *buf, size_t size)
{
    for (int i = 0; i < (int)size - 1; i++) {
        buf[i] = 'B';
    }
    
    buf[size-1] = '\n';
}

void remove_files(int num_files)
{
    for (int i = 1; i <= num_files; i++) {

        string filename = get_filename(i);

        int x = remove(filename.c_str());
        if(x == -1) {
            printf("%s file remove fail\n", filename.c_str());

        }
    }
}

double TimeSpecToSeconds(struct timespec* ts)
{
    return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}
