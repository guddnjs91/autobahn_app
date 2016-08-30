#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
using namespace std;
/**
 * Fill in buffer with given size 
 * write each element with random characters */
void
fill_buf(char *buf, size_t size)
{
    srand(time(NULL));

    for(int i = 0; i < (int)size - 1; i++) {
       if(rand() % 5 == 0) {
            buf[i] = ' ';
        } else {
            buf[i] = rand() % 26 + 'A';
        }
    }
    
    buf[size-1] = '\n';
}

void remove_files(int n)
{
    for (int i = 1; i <= n; i++)
    {
        string filename = "/opt";

        if(i % 2 == 1) {
            filename += "/nvm1/NVM/VOL_";
        } else {
            filename += "/nvm2/NVM/VOL_";
        }

        filename += std::to_string(i);
        filename += ".txt";

        int x = remove(filename.c_str());
        if(x == -1)
            printf("%s file remove fail\n", filename.c_str());
    }
}
