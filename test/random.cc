/**
  * This is the implementation file of class random 
  * which carries random numbers with various distribution.
  * Supporting distribution is uniform and zipf.
  *
  * @author Hyeongwon Jang
  * @since  2016-10-11
  */


#include "random.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
//#define __WT_RAND_SUPPORT_ 1
#ifdef __WT_RAND_SUPPORT_
#include "wt_rand.h"
#endif

#define DEFAULT_DISTR_N (5 * 1024 * 1024)

/**
  * Constructor (default)
  * Allocate dist_ with size DEFAULT_DISTR_N and
  * initialize theta_ and n_ to default value.
  */
random::random() {
    dist_ = new uint64_t[DEFAULT_DISTR_N];
    theta_ = 0;
    n_ = DEFAULT_DISTR_N;
}

/**
  * Constructor (with parameter)
  * Allocate dist_ and n_ with size n (parameter).
  *
  * @param[in]   n   input range size.
  */
random::random(uint64_t n) {
    dist_ = new uint64_t[n];
    theta_ = 0;
    n_ = n;
}

/**
  * Destructor
  * Deallocate dist_ and re-initialize theta_ and n_.
  */
random::~random() {
    delete[] dist_;
    theta_ = 0;
    n_ = 0;
}

/**
  * Initiailze dist_ with uniformly picked random numbers.
  */
void random::unif_init() {
#ifdef __WT_RAND_SUPPORT_
    RAND_STATE rnd;
    uniform_rand_init(&rnd);
    
    for (uint64_t i = 0; i < n_; i++) {
        dist_[i] = uniform_rand(&rnd);
    }
#endif
    srand(time(NULL));

    for (uint64_t i = 0; i < n_; i++) {
        dist_[i] = (uint64_t)rand();
    }
}

/**
  * Return elements indexed in random position from uniform dist_.
  *
  * @return     unsigned 32 bit random number. 
  *             0 if not initialized yet.
  */
uint32_t random::unif_rand() {
    if (n_ == 0) {
        return 0;
    }
#ifdef __WT_RAND_SUPPORT_
    RAND_STATE rnd;
    uniform_rand_init(&rnd);

    return (uint32_t) dist_[uniform_rand(&rnd) / n_];
#else
    return (uint32_t) dist_[rand() % n_];
#endif
}

/**
  * Return elements indexed in random position from uniform dist_.
  *
  * @return     unsigned 64 bit random number. 
  *             0 if not initialized yet.
  */
uint64_t random::unif_rand64() {
    if (n_ == 0) {
        return 0;
    }
#ifdef __WT_RAND_SUPPORT_
    RAND_STATE rnd;
    uniform_rand_init(&rnd);
    uint64_t ret;
    ret = dist_[uniform_rand(&rnd) / n_] << 32;
    ret |= (uint32_t) dist_[uniform_rand(&rnd) / n_];

    return ret;
#else
    uint64_t ret;
    ret = dist_[rand() % n_] << 32;
    ret |= (uint32_t) dist_[rand() % n_];
    
    return ret;
#endif
}

/**
  * Initiailze dist_ with skewed distributed number.
  * Zipf distribution using p(i) = c / (i)^(1-theta).
  *
  * @param[in]  t   theta range from (0, 1).
  *                 pure zipf when t goes to 0, while
  *                 uniform when t goes to 1.
  */
void random::zipf_init(double t) {
    uint64_t i;
    double sum = 0.0;
    double c = 0.0;
    double exp = 1 - t;
    double csum = 0.0;
    
    srand(time(NULL));

    /* sum = {1/1 + 1/2 + 1/3 + ... + 1/n} ,
       when theta goes to 0. */
    for (i = 1; i <= n_; i++) {
        sum += 1.0 / (double)pow((double)i, (double)exp);
    }
    
    c = 1.0 / sum;

    /* dist_[i] = c * {1/1 + 1/2 + 1/3 + ... + 1/i} * n,
       when thata goes to 0. */
    for (i = 1; i <= n_; i++) {
        csum += c / (double)pow((double)i, (double)exp);
        dist_[i-1] = csum * n_;
    }
}

/**
  * Return elements indexed in random position from skewed dist_.
  *
  * @return     unsigned 32 bit random number.
  *             0 if not initialized yet.
  */
uint32_t random::zipf_rand() {
    if (n_ == 0) {
        return 0;
    }
#ifdef __WT_RAND_SUPPORT_
    RAND_STATE rnd;
    uniform_rand_init(&rnd);

    return (uint32_t) dist_[uniform_rand(&rnd) / n_];
#else
    return dist_[rand() % n_];
#endif
}

/**
  * Return elements indexed in random position from skewed dist_.
  *
  * @return     unsigned 64 bit random number. 
  *             0 if not initialized yet.
  */
uint64_t random::zipf_rand64() {
    if (n_ == 0) {
        return 0;
    }
#ifdef __WT_RAND_SUPPORT_
    RAND_STATE rnd;
    uniform_rand_init(&rnd);

    return dist_[uniform_rand(&rnd) / n_];
#else
    return dist_[rand() % n_];
#endif
}

/**
  * Return elements indexed in i-th position from skewed dist_.
  *
  * @return     unsigned i-th number from zipf distribution. 
  *             0 if not initialized yet.
  */
uint64_t random::zipf_rand_at(uint64_t i) {
    if (n_ == 0) {
        return 0;
    }
    return dist_[i];
}

/**
  * Print out distribution partitioned into 10 groups. 
  */
void random::print_dist() {
    uint64_t unit = n_ / 10;
    uint64_t *section = new uint64_t[10];
    for (int i = 0; i < 10; i++) {
        section[i] = 0;
    }

    for (uint64_t i = 0; i < n_; i++) {
        section[dist_[i] / unit]++;
    }

    for (int i = 0; i < 10; i++) {
        printf("From %10llu to %10llu : %10llu\n",
                i * unit, (i + 1) * unit, section[i]);
    }
}

