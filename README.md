<h1 align="center">SharedMap - Shared-Memory Hierarchical Multisection for Process Mapping</h1>


About SharedMap
-----------
SharedMap is a parallel shared-memory algorithm for hierarchical process mapping.
Process Mapping concerns itself with mapping tasks of a task graph, where weighted vertices represent tasks and weighted edges the amount of communicated data, to the cores of a supercomputer.
In hierarchical process mapping the hierarchy of the supercomputer (often islands, racks, nodes and cores) is exploited for more efficient mapping algorithms.
The figure shows a more formal definition.

SharedMap offers State-Of-The-Art solution quality among available parallel mapping algorithms.
It has better quality while also being slightly faster.
See the left figure.

Even in the serial case, it is stronger and faster than the previous best algorithm.
See the right figure.
For more information on the algorithm, we refer to our work...


Requirements
-----------
This project utilizes the [KaHIP](https://github.com/KaHIP/KaHIP) and [Mt-KaHyPar](https://github.com/kahypar/mt-kahypar) library.
The requirements needed for both of these projects carry over to this project.

- A Linux operating system (others have not yet been tested).
- A modern compiler that supports C++17, such as `g++` (others have not yet been tested).
- The [cmake](https://cmake.org/) build system (>=3.21).
- The [Portable Hardware Locality](https://www.open-mpi.org/projects/hwloc/) library.
- The [Boost - Program Options](https://www.boost.org/doc/libs/1_58_0/doc/html/program_options.html) library will be automatically downloaded by Mt-KaHyPar (`-DKAHYPAR_DOWNLOAD_BOOST=ON`).
- The [Intel Thread Building Blocks](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html) library will be automatically downloaded by Mt-KaHyPar (`-DKAHYPAR_DOWNLOAD_TBB=ON`) and SharedMap (`-DSHAREDMAP_DOWNLOAD_TBB=ON`).

The following command will install (most of) the required dependencies on a Ubuntu machine:
```
sudo apt-get install libtbb-dev libhwloc-dev libboost-program-options-dev
```

Installation
-----------
### Automatic
The script `build.sh` will automatically install the binary and the library.
It will first install the Mt-KaHyPar library and afterward build the project.

### Manually
Alternatively, you can build the project by hand.
First install Mt-KaHyPar into `extern/mt_kahypar_local` via
```
mkdir -p extern/mt_kahypar_local
mkdir -p extern/mt-kahypar/build
cd extern/mt-kahypar/build
cmake .. -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DCMAKE_BUILD_TYPE=Release -DKAHYPAR_DOWNLOAD_TBB=ON -DKAHYPAR_DOWNLOAD_BOOST=ON -DKAHYPAR_ENABLE_THREAD_PINNING=OFF -DKAHYPAR_DISABLE_ASSERTIONS=ON -DCMAKE_INSTALL_PREFIX=$(pwd)/../../mt_kahypar_local
make -j$(nproc) install.mtkahypar
cd ../../../
```
then build SharedMap via
```
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSHAREDMAP_DOWNLOAD_TBB=ON
cmake --build . --parallel $(nproc) --target SharedMap # the executable
cmake --build . --parallel $(nproc) --target sharedmap # the library
```

The binary `SharedMap` and the library `libsharedmap.so` will be present in the `build` folder.
The include files for the library are present in the `include` folder.


Usage
-----------
Call `SharedMap` in the `build` folder:

    ./build/SharedMap -g <inpath-graph> -m <outpath-parition> -h <hierarchy> -d <distance> -e <imbalance (e.g. 0.03)> -c {fast|eco|strong} -t <# threads> -s {naive|layer|queue|nb_layer} --seed <seed>

### Configuration
The available command line arguments and a short description.
You can also use `./build/SharedMap --help` for a list of available parameters.
```
[ -g | --graph     ] <inpath-graph>      : Filepath to a graph in Metis graph format.
[ -m | --mapping   ] <outpath-partition> : Path to the file that will hold the resulting partition. Any existing file will be overwritten.
[ -h | --hierarchy ] <hierarchy>         : The hierarchy of the supercomputer in the format a:b:c: ... e.g., 4:8:6 .
[ -d | --distance  ] <distance>          : The distance of the processors in the format a:b:c: ... e.g., 1:10:100 .
[ -e | --imbalance ] <imbalance>         : The maximum allowed imbalance per block e.g., 0.03 allows fo a maximum imbalance of 3%.
[ -c | --config    ] <config>            : Which partitioning configuration to use. Allowed value are {fast, eco, strong}.
[ -t | --threads   ] <# threads>         : The number of threads to use.
[ -s | --strategy  ] <strategy>          : Which thread distribution strategy to use. Allowed value are {naive, layer, queue, nb_layer}.

Optional:
[ --seed           ] <seed>              : Seed to diversiy partitioning results. If no seed is provided, a rradnom one will be generated at each call.
```
### Example
The graph `graphs/big.graph` is partitioned on a supercomputer with a hierarchy of `4:8:6` with distances `1:10:100` and an allowed imbalance of `4%`.
As the configuration we choose `fast` with `10` threads and the `queue` distribution strategy.
As the seed we choose `13`.
The resulting partition is stored in `results/mapping.txt`.


    ./build/SharedMap -g graphs/big.graph -m results/mapping.txt -h 4:8:6 -d 1:10:100 -e 0.04 -c fast -t 10 -s queue --seed 13


C++ Interface
-----------
SharedMap offers a C++ interface via `include/libsharedmap.h`. Here is a small example:
```

```

## Bugs, Questions, Comments and Ideas

If any bugs arise, questions occur, comments want to be shared, or ideas discussed, please do not hesitate to contact the current repository owner (henning.woydt@informatik.uni-heidelberg.de) or leave a GitHub [Issue](https://github.com/HenningWoydt/SharedMap/issues) or [Discussion](https://github.com/HenningWoydt/SharedMap/discussions). Thanks!


Licensing
---------
SharedMap is a free software provided under the MIT License. For more information see the [LICENSE file][LF]. This algorithm is available to everyone, welcoming all who wish to make use of it. If you use SharedMap in an academic setting please cite
```
TODO
```

[LF]: https://github.com/HenningWoydt/SharedMap/blob/master/LICENSE "License"