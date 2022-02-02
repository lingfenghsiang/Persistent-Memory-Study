# Case studies
## How to build
```
mkdir build
cd build && cmake ..
make
```

## How to run
To run the code, just use

`cceh_test/fastfair_original_test/fastfair_rap_mod_test/fastfair_unmodified_test -[options] [value]`

The executable files are:
- cceh_test: CCEH test
- fastfair_original_test: The FAST&FAIR we fixed by adding clflush between each write opertation.
- fastfair_rap_mod_test: The FAST&FAIR that we optimized by writing to a new place.
- fastfair_unmodified_test Original code for FAST&FAIR, not modified.


The options include:
- loadfile: The location of the file that populate the key value store data structure.
- runfile: The location of the file that the data structure uses for running.
- pool_dir: Where to put the mmapped file as persistent memory pool. You just need to specify the folder, and the file name should not be speficied.
- pool_size: The size of mmapped file.
- thread: How many threads may simultaneously do the work?
- preread: Whether to run a preread thread. If yes type "-preread", otherwise type "-nopreread". If we use preread option, each worker will be given a preread thread for assistance.
- prereadnum: How much should the preread thread be faster? For example, "-prereadnum 4", means each preread thread will be 4 key-value pairs faster than the working thread, given the limited L3 cache size.

## Code files
Important files are listed in the file tree shown below:  
.  
├── CMakeLists.txt  
├── external_repo  
│   ├── cceh  
│   │   ├── CCEH-PMDK  
│   │   │   ├── CMakeLists.txt  
│   │   │   ├── Makefile  
│   │   │   ├── README.md  
│   │   │   ├── src  
│   │   │   │   ├── CCEH.cpp  
│   │   │   │   ├── CCEH.h  
│   │   │   │   ├── hash.h  
│   │   │   │   ├── pair.h  
│   │   │   │   ├── test.cpp  
│   │   │   │   └── util.h  
│   │   │   └── wrap.cpp  
│   ├── fastfair  
│   │   ├── CMakeLists.txt  
│   │   ├── fastfair.h  
│   │   ├── persist.h  
│   │   └── wrap.cpp  
│   └── FAST_FAIR  
│       └── concurrent  
│           ├── Makefile  
│           └── src  
│               ├── btree.h  
│               └── test.cpp  
├── global.hpp  
├── include  
│   ├── cceh_pmdk.hpp  
│   └── fastfair.hpp  
├── kv_wrap.hpp  
├── main.cpp  
├── path.cmake  
├── README.md  
└── tests.hpp  

Among these files, `main.cpp` is responsible for reading the workload file and generate a large vector that holds operations to the index data structures. `tests.hpp` includes the main frame of test (distribute work to different threads, control preread threads, collect results after tests).
All the operations of data operations are wrapped as a class "Index" in file `kv_wrap.hpp`. CCEH and FAST &FAIR inherits class "Index", and the child classes call the functions in file `fastfair/wrap.cpp` and `cceh/CCEH-PMDK/wrap.cpp`.
## How does the workload file look like?
The workload is generated via YCSB and formated by [generate_workload.py](../tools/generate_workload.py).
Each line is an operation and an unsigned 64bit integer.
```
INSERT 3341382696024442574
INSERT 513445856493174041
INSERT 8920877428775656744
INSERT 1150730008844191335
INSERT 7745948095805038430
INSERT 6938526047896506085
```
## Miscellaneous
**Note**: the original code for FastFair has some bugs when multiple threads insert then read the key value pairs.
