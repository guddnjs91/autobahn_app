#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <string>
/**
 * Fill in buffer with given size 
 * write each element with random characters */
void
fill_buf(char *buf, size_t size)
{
    srand(time(NULL));
    int i;

    for(i = 0; i < (int)size - 1; i++) {
//       if(rand()%10 == 0) {
//            buf[i] = '\n';
//        }
//       else if(rand()%5 == 0) {
       if(rand()%5 == 0) {
            buf[i] = ' ';
        } else {
            buf[i] = rand() % 26 + 'A';
        }
    }
    
    buf[size-1] = '\n';
}

void remove_files(int n)
{
    int i;
    for(i = 1; i <= n; i++)
    {
        std::string filename = "/opt";

        if(i % 2 == 1)
        {
            filename += "/nvm1/NVM/VOL_";
        }

        else
        {
            filename += "/nvm2/NVM/VOL_";
        }

        filename += std::to_string(i);
        filename += ".txt";

        int x = remove(filename.c_str());
        if(x == -1)
            printf("%s file remove fail\n", filename.c_str());
    }
}
