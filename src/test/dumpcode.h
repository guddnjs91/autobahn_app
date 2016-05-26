#include <stdio.h>
#include <ctype.h>

void
printchar(
    char c)
{
    if(isprint(c)) {
        printf("%c", c);
    } else {
        printf(".");
    }
}

/**
 * Print memory dump for given address with size len */
void
data_dump(
    unsigned char *buff,
    int len)
{
    printf("\n--------------------DATA DUMP---------------------------\n");
    
    int i;
    for (i=0; i<len; i++) {
        if(i%16 == 0) {
            printf("0x%08lx ", (long unsigned int)&buff[i]);
        }

        printf("%02x ", buff[i]);

        if(i%16-15 == 0) {
            int j;
            printf(" ");
            for(j = i-15; j <= i; j++) {
                printchar(buff[j]);
            }
            printf("\n");
        }
    }
    
    if(i%16 != 0) {
        int j;
        int spaces = (len-i+16-i%16)*3+2;
        for(j=0; j<spaces; j++) {
            printf(" ");
        }
        for(j=i-i%16; j<len; j++) {
            printchar(buff[j]);
        }
    }
    
    printf("\n");
}
