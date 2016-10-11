#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include "random.h"

#define TIMEOUT     (10)
#define SECTION_NUM (10)
#define N           (5 * 1024 * 1024)
#define THETA       (0.1)

uint64_t g_total;
uint64_t g_section[SECTION_NUM];

int main(void) {
    uint64_t num;
    uint64_t section_unit = N / SECTION_NUM;
    clock_t endwait;

    random *ur = new random(N);
    //random *zr = new random(N);

    ur->unif_init();
    //zr->zipf_init(THETA);

    endwait = clock() + TIMEOUT * CLOCKS_PER_SEC;
    uint64_t i = 0;
    while(clock() < endwait) {
        //num = ur->unif_rand() % N;
        num = ur->unif_rand64();
        //num = zr->zipf_rand();

        printf("%llu\n", num);
    }

    ur->print_dist();
    //zr->print_dist();

}
