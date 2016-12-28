/**
  * This is the header file of class Random 
  * which carries Random numbers with various distribution.
  * Supporting distribution is uniform and skewed form.
  * NOTE !!: Must call skew_init() before calling skew_rand() or skew_rand64() methods.
  *
  * @author Hyeongwon Jang
  * @since  2016-10-11
  */

#ifndef __Random_H_
#define __Random_H_

#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
//#define __WT_RAND_SUPPORT_ 1
#ifdef __WT_RAND_SUPPORT_
#include "wt_rand.h"
#endif

#define DEFAULT_DISTR_N (5 * 1024 * 1024)
#define THETA (0.25)                // For zipf distribution
#define DEFAULT_N (5 * 1024 * 1024) // For zipf distribution

class Random {

public:
//    Random();
//    ~Random();
//    
//    uint32_t unif_rand();
//    uint64_t unif_rand64();
//    
//    int skew_init(double t, uint64_t n);
//    uint32_t skew_rand();
//    uint64_t skew_rand64();
//    uint64_t skew_rand_at(uint64_t i);
//    void skew_print_dist();
    /**
      * Constructor
      * Initialize dist_ and n_ to default value.
      */
    Random() {
        dist_ = 0;
        //dist_ = new uint64_t[DEFAULT_DISTR_N];
        n_ = 0;
        srand(time(NULL));
    };
    
    /**
      * Destructor
      * Deallocate dist_ and re-initialize n_.
      */
    ~Random() {
        delete[] dist_;
        //free(dist_);
        n_ = 0;
    };
    
    /**
      * Return 32 bit Random number uniformly.
      *
      * @return     unsigned 32 bit Random number. 
      *             0 if not initialized yet.
      */
    uint32_t unif_rand() {
#ifdef __WT_RAND_SUPPORT_
        RAND_STATE rnd;
        uniform_rand_init(&rnd);
    
        return uniform_rand(&rnd);
#else
        return (uint32_t) rand();
#endif
    };
    
    /**
      * Return 64 bit Random number uniformly.
      *
      * @return     unsigned 64 bit Random number. 
      *             0 if not initialized yet.
      */
    uint64_t unif_rand64() {
#ifdef __WT_RAND_SUPPORT_
        RAND_STATE rnd;
        uniform_rand_init(&rnd);
        uint64_t ret;
        ret = uniform_rand(&rnd) << 32;
        ret |= (uint64_t) uniform_rand(&rnd);
    
        return ret;
#else
        uint64_t ret;
        ret = (uint64_t) rand() << 32;
        ret |= rand();
        
        return ret;
#endif
    };
    
    /**
      * Initiailze dist_ with skewed distributed number.
      * Zipf distribution using p(i) = c / (i)^(1-theta).
      *
      * @param[in]  t   theta range from (0, 1).
      *                 pure zipf when t goes to 0, while
      *                 uniform when t goes to 1.
      * @param[in]  n   distribution range size.
      * @return     0 if success,
      *             -1 if error occurs.
      */
    int skew_init(double t, uint64_t n) {
        uint64_t i;
        double sum = 0.0;
        double c = 0.0;
        double exp = 1 - t;
        double csum = 0.0;
        
        if (n <= 1) {
            return -1; // error
        } else {
            n_ = n;
        }
        
        //printf("Hello\n");
        dist_ = new uint64_t[n];
        //dist_ = (uint64_t *)malloc(sizeof(uint64_t) * n);
        //printf("Hello\n");

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
    
        return 0;
    };
    
    /**
      * Return elements indexed in Random position from skewed dist_.
      *
      * @return     unsigned 32 bit Random number.
      *             0 if not initialized yet.
      */
    uint32_t skew_rand() {
        if (n_ == 0) {
            return 0;
        }
#ifdef __WT_RAND_SUPPORT_
        RAND_STATE rnd;
        uniform_rand_init(&rnd);
    
        return (uint32_t) dist_[uniform_rand(&rnd) % n_];
#else
        return dist_[rand() % n_];
#endif
    };
    
    /**
      * Return elements indexed in Random position from skewed dist_.
      *
      * @return     unsigned 64 bit Random number. 
      *             0 if not initialized yet.
      */
    uint64_t skew_rand64() {
        if (n_ == 0) {
            return 0;
        }
#ifdef __WT_RAND_SUPPORT_
        RAND_STATE rnd;
        uniform_rand_init(&rnd);
    
        return dist_[uniform_rand(&rnd) % n_];
#else
        return dist_[rand() % n_];
#endif
    };
    
    /**
      * Return elements indexed in i-th position from skewed dist_.
      *
      * @return     unsigned i-th number from zipf distribution. 
      *             0 if not initialized yet.
      */
    uint64_t skew_rand_at(uint64_t i) {
        if (n_ == 0) {
            return 0;
        }
        return dist_[i];
    };
    
    /**
      * Print out distribution partitioned into 10 groups. 
      */
    void skew_print_dist() {
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
    };

private:

    uint64_t    *dist_;
    uint64_t    n_;
};

#endif
