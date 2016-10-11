/**
  * This is the header file of class random 
  * which carries random numbers with various distribution.
  * Supporting distribution is uniform and zipf.
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
    random(uint64_t n);
    ~random();
    
    void unif_init();
    uint32_t unif_rand();
    uint64_t unif_rand64();
    
    void zipf_init(double t);
    uint32_t zipf_rand();
    uint64_t zipf_rand64();
    uint64_t zipf_rand_at(uint64_t i);

    void print_dist();

private:

    uint64_t    *dist_;
    double      theta_;
    uint64_t    n_;
};

#endif
