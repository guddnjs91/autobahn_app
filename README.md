# NVM Burst Buffer

## Getting Started

### Install

```
$ git clone -recursive http://166.104.29.119:3389/guddnjs91/atomic-durable-nvm.git
```
or
```
$ git clone http://166.104.140.26:3389/guddnjs91/atomic-durable-nvm.git
$ update
```

### Compile
Compile : make

Execute : bin/test

After Execution, there are leftover data in shared memory.
So need to bin/shm-remove for deallocating shared memory.


***So Far Implementation

- write data blocks to nvm checked
- inodes for syncing managed by sync-list implemented
- write 200MB * 3 thread : fine (shared memory size 1GiB now)
- write > 500MB * 1 thread : Seg fault (need calculation for data block)

Issues
- cache line flush not added
- sync thread routine loops forever
- sync policy need update (need discussion)
- atomicity not implemented yet

