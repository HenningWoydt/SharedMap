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

Installation
-----------
### Automatic
The script `build.sh` will automatically install the binary and the library.
It will first install the Mt-KaHyPar library and afterward build the project.

### Manually
If you want to 



Usage
-----------
Call `SharedMap` in the `build` folder:

    ./build/SharedMap -g <inpath-graph> -m <outpath-parition> -h <hierarchy> -d <distance> -e <imbalance (e.g. 0.03)> -c {fast|eco|strong} -t <# threads> -s {naive|layer|queue|nb_layer}

### Configuration
The available command line arguments and a short description.
```
[ -g | --graph     ] <inpath-graph>      : Filepath to a graph in Metis format.
[ -m | --mapping   ] <outpath-partition> : Path to the file that will hold the resulting partition. An existing file will be overwritten.
[ -h | --hierarchy ] <hierarchy>         : The hierarchy of the supercomputer in the format a:b:c:.. e.g., 4:8:6 .
[ -d | --distance  ] <distance>          : The distance of the processors in the format d:e:f... e.g., 1:10:100 .
[ -e | --imbalance ] <imbalance>         : The maximum allowed imbalance per blocke e.g., 0.03 allows fo a maximum imbalance of 3%.
[ -c | --config    ] <config>            : Which partitioning configuration to use. Allowed value are {fast, eco, strong}.
[ -t | --threads   ] <# threads>         : The number of threads to use.
[ -s | --strategy  ] <strategy>          : Which thread distribution strategy to use. Allowed value are {naive, layer, queue, nb_layer}.

Optional:
[--seed          ] <seed>              : Seed to diversiy partitioning results.
```
### Example
The graph `graphs/big.graph` is partitioned on a supercomputer with a hierarchy of `4:8:6` with distances `1:10:100` and an allowed imbalance of `4%`.
As the configuration we choose `fast` with `10` threads and the `queue` distribution strategy.
The resulting partition is stored in `results/mapping.txt`.


    ./build/SharedMap -g graphs/big.graph -m results/mapping.txt -h 4:8:6 -d 1:10:100 -e 0.04 -c fast -t 10 -s queue


C++ Interface
-----------

## Bugs, Questions, Comments and Ideas

If any bugs arise, questions occur, comments want to be shared, or ideas discussed, please do not hesitate to contact the current repository owner (henning.woydt@informatik.uni-heidelberg.de) or leave a GitHub Issue or Discussion. Thanks!


Licensing
---------


