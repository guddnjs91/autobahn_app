#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

int main()
{
    int     shmid;
    int     pid;

    int     *cal_num;
    void    *shared_memory = (void *)0;

    // Make Shared Memory space.
    shmid = shmget((key_t)1234, SHM_SIZE, 0666 | IPC_CREAT);

    if(shmid == -1)
    {
        perror("shmget failed : ");
        exit(0);
    }

    // Attach Shared Memory to the Process.
    shared_memory = shmat(shmid, (void *)0, 0);
    if(shared_memory == (void *)-1)
    {
        perror("shmat failed : ");
        exit(0);
    }

    // Detach Shared Memory
    if(shmdt(shared_memory) == -1)
    {
        perror("shmdt failed : ");
        exit(0);
    }

    /* get rid of shared memory allocated space*/
    if(shmctl(shmid, IPC_RMID, 0) < 0)
    {
        printf("shmctl error");
        exit(0);
    }

    printf("Shared Memory Destroyed !!\n");

}
