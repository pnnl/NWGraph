<!--
SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
SPDX-FileCopyrightText: 2022 University of Washington

SPDX-License-Identifier: BSD-3-Clause
-->

## Experimental Setup

Abstraction penalty benchmarks were run on 2019 MacBook Pro with 2.4 GHz 8-Core Intel® Core i9 processor with 64 GB DDR4 memory running at 2.6GHz.  We used GCC 10 with ``-Ofast'' and TBB 2020.3 to compile.

All performance results of the large-scale graphs were obtained on the following machine.
Each server contains two Intel® Xeon® Platinum 8153 processors, each with 16 physical cores (32 logical cores) running at 2.0 GHz. Each processor has 22 MB L3 cache. The total system memory of each server is 384 GB DDR4 running at 2.6 GHz.
We used GCC 10 with ``-Ofast'' and TBB 2020.3 to compile.

## Benchmarking with GAP Datasets

To obtain the performance results of the large-scale graphs reported in the PVLDB paper for NWGraph, "NWGraph: A Library of Generic Graph Algorithms and DataStructures in C++20", please follow the following steps.

* Download the GAP datasets from [Suitesparse Matrix Collection](https://sparse.tamu.edu/GAP/) in Matrix Market format 
(The datasets include GAP-twitter, GAP-web, GAP-urand, GAP-road, and GAP-kron).
* Extract the downloaded tar file (which include the matrix files and the "sources" file). 
* Run different graph benchmarks with the GAP datasets for multiple trials.

### Breadth-first Search
The default sequential version of BFS is version 0 (default). The fastest parallel version of BFS is version 11, the direction-optimizing BFS. As an alternative to specifying one seed at a time, one or more sources can be provided in a Matrix Market format file as an input of BFS driver. Also, number of trials can be specified with `-n`. In this way, if no seed or seed file is provided, each trial will generate one random number from 0 to |V|-1 as the random source for BFS as an input.
```
$ bench/bfs.exe -f GAP-twitter.mtx --sources GAP-twitter_sources.mtx --version 11 -n 5
$ bench/bfs.exe -f GAP-road.mtx --sources GAP-road_sources.mtx --version 11 -n 5
$ bench/bfs.exe -f GAP-urand.mtx --sources GAP-urand_sources.mtx --version 11 -n 5
$ bench/bfs.exe -f GAP-kron.mtx --sources GAP-kron_sources.mtx --version 11 -n 5
$ bench/bfs.exe -f GAP-web.mtx --sources GAP-web_sources.mtx --version 11 -n 5
```
### Connected Component Decomposition
The default sequential version of CC is version 0 (default). The fastest parallel version of CC is version 7, Afforest.
```
$ bench/cc.exe -f GAP-twitter.mtx --version 7
$ bench/cc.exe -f GAP-road.mtx --version 7
$ bench/cc.exe -f GAP-urand.mtx --version 7
$ bench/cc.exe -f GAP-kron.mtx --version 7
$ bench/cc.exe -f GAP-web.mtx --version 7
```
### Page Rank
The fastest parallel version of PR is version 11 (default). The max iterations can be set with `-i`.
```
$ bench/pr.exe -f GAP-twitter.mtx -i 1000 --version 11 -n 5
$ bench/pr.exe -f GAP-road.mtx -i 1000 --version 11 -n 5
$ bench/pr.exe -f GAP-urand.mtx -i 1000 --version 11 -n 5
$ bench/pr.exe -f GAP-kron.mtx -i 1000 --version 11 -n 5
$ bench/pr.exe -f GAP-web.mtx -i 1000 --version 11 -n 5
```
### Single Source Shortest Path
The default sequential version of CC SSSP version 0 (default). The fastest parallel version of SSSP is version 12, Delta-stepping. As an alternative to specifying one seed at a time, one or more sources can be provided in a Matrix Market format file as an input of SSSP driver. Also, number of trials can be specified with `-n`. In this way, if no seed or seed file is provided, each trial will generate one random number from 0 to |V|-1 as the random source for SSSP as an input.
```
$ bench/sssp.exe -f GAP-twitter.mtx --sources GAP-twitter_sources.mtx --version 12 -n 5
$ bench/sssp.exe -f GAP-road.mtx --sources GAP-road_sources.mtx --version 12 -n 5
$ bench/sssp.exe -f GAP-urand.mtx --sources GAP-urand_sources.mtx --version 12 -n 5
$ bench/sssp.exe -f GAP-kron.mtx --sources GAP-kron_sources.mtx --version 12 -n 5
$ bench/sssp.exe -f GAP-web.mtx --sources GAP-web_sources.mtx --version 12 -n 5
```
### Triangle Counting
The default sequential version of TC is version 0 (default). The fastest parallel version of TC is version 4.
```
$ bench/tc.exe -f GAP-twitter.mtx --version 4 --relabel --upper -n 5
$ bench/tc.exe -f GAP-road.mtx --version 4 --relabel --upper -n 5
$ bench/tc.exe -f GAP-urand.mtx --version 4 --relabel --upper -n 5
$ bench/tc.exe -f GAP-kron.mtx --version 4 --relabel --upper -n 5
$ bench/tc.exe -f GAP-web.mtx --version 4 --relabel --upper -n 5
```
### Betweenness Centrality
The default sequential version of BC is version 0 (default). The fastest parallel version of BC is version 5. As an alternative to specifying one seed at a time, one or more sources can be provided in a Matrix Market format file as an input of BC driver. 
```
$ bench/bc.exe -f GAP-twitter.mtx --sources GAP-twitter_sources.mtx --version 5 -n 5
$ bench/bc.exe -f GAP-road.mtx --sources GAP-road_sources.mtx --version 5 -n 5
$ bench/bc.exe -f GAP-urand.mtx --sources GAP-urand_sources.mtx --version 5 -n 5
$ bench/bc.exe -f GAP-kron.mtx --sources GAP-kron_sources.mtx --version 5 -n 5
$ bench/bc.exe -f GAP-web.mtx --sources GAP-web_sources.mtx --version 5 -n 5
```

Note that BFS and SSSP are run with 64 sources provided in a Matrix Market file, and BC are run with 4 sources. For PR, the max iterations has been set to 1000.

### Performance results of other libraries/frameworks

Note that only the performance results of NWGraph can be generated. The performance results of the other graph frameworks can be generated using their source code with the same GAP datasets.  The results reported in our paper are recapitulated from "Ariful Azad, Mohsen Mahmoudi Aznaveh, Scott Beamer, Mark Blanco, Jinhao Chen, Luke D’Alessandro, Roshan Dathathri, Tim Davis, Kevin Deweese, Jesun Firoz, et al.2020. Evaluation of Graph Analytics Frameworks Using the GAP Benchmark Suite. In 2020 IEEE International Symposium on Workload Characterization (IISWC). IEEE, 216–227".


## Benchmarking abstraction penalties

To obtain the performance results of the abstraction penalty benchmarks reported in the PVLDB paper for NWGraph,please download the graphs: [circuit5M](https://sparse.tamu.edu/Freescale/circuit5M), [GAP-road](https://sparse.tamu.edu/GAP/GAP-road) and [hugebubbles](https://sparse.tamu.edu/DIMACS10/hugebubbles-00000); extract the matrix market files.

### Benchmarking different ways to iterate through a graph
To experimentally evaluate the abstraction penalty of different ways to iterate through a graph:
```
$ apb/spmv -f circuit5M.mtx
$ apb/spmv -f GAP-road.mtx
$ apb/spmv -f hugebubbles-00000.mtx
```

### Benchmarking different containers for storing a graph
To experimentally evaluate the abstraction penalty of different containers for storing a graph:
```
$ apb/containers -f circuit5M.mtx  --format CSR --format VOV --format VOL --format VOF
$ apb/containers -f GAP-road.mtx  --format CSR --format VOV --format VOL --format VOF
$ apb/containers -f hugebubbles-00000.mtx  --format CSR --format VOV --format VOL --format VOF
```