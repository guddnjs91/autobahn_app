#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include "ut0random.h"

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

    Random *r = new Random();

    r->skew_init(THETA, N);

    endwait = clock() + TIMEOUT * CLOCKS_PER_SEC;
    
    uint64_t unit = N / SECTION_NUM;
    while(clock() < endwait) {
        //num = r->unif_rand() % N;
        //num = r->unif_rand64();
        num = r->skew_rand();

        g_section[(num % N)/ unit]++;
        g_total++;

        printf("%llu\n", num);
    }
    r->skew_print_dist();
    for (int i = 0; i < 10; i++) {
        printf("From %10llu to %10llu : %10llu\n",
                i * unit, (i + 1) * unit, g_section[i]);
    }
}
