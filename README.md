# NVM Burst Buffer

## Getting Started

### Install
```
$ git clone --recursive http://166.104.29.119:3389/guddnjs91/atomic-durable-nvm.git
```
or
```
$ git clone http://166.104.140.26:3389/guddnjs91/atomic-durable-nvm.git
$ git submodule init
$ git submodule update
```
#### libcuckoo
refer to https://github.com/efficient/libcuckoo

### Compile
```
$ make
```

### Run
```
$ bin/test
```

### Clean
```
$ make clean
$ bin/shm-remove
```