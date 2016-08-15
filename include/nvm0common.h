/**
 * nvm0common.h - include all system header files */

#ifndef nvm0common_h
#define nvm0common_h

#include <atomic>
#include <cstdint>
#include <unistd.h>
#include "nvm0lfqueue.h"
#include "../src/nvm0lfqueue.cc"
#include "nvm0avltree.h"
#include "nvm0volume.h"
#include "nvm0inode.h"
#include "nvm0metadata.h"
#include "nvm0prototypes.h"
#include "nvm0hash.h"
#include "nvm0list.h"
#include "libcuckoo/src/cuckoohash_map.hh"

#endif
