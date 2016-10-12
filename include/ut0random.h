/**
  * This is the header file of class random 
  * which carries random numbers with various distribution.
  * Supporting distribution is uniform and skewed form.
  *
  * @author Hyeongwon Jang
  * @since  2016-10-11
  */

#ifndef __RANDOM_H_
#define __RANDOM_H_

#include <cstdint>

class random {

public:
    random();
    ~random();
    
    uint32_t unif_rand();
    uint64_t unif_rand64();
    
    int skew_init(double t, uint64_t n);
    uint32_t skew_rand();
    uint64_t skew_rand64();
    uint64_t skew_rand_at(uint64_t i);
    void skew_print_dist();

private:

    uint64_t    *dist_;
    uint64_t    n_;
};

#endif
