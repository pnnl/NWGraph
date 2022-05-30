.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

|Build with gcc-11| |image1| |image2| |Codacy Badge|

Quickstart
==========

NWGraph is a high-performance header-only generic C++ graph library,
based on C++20 ``concept`` and ``range`` language feature. It consists
of multiple graph algorithms for well-known graph kernels and supporting
data structures. Both sequential and parallel algorithms are available.

Project Organization
--------------------

The organization of our library is shown as follow:

.. code:: text

   $NWGraph_HOME/
   ├── README.md
   ├── CMakeLists.txt
   ├── apb/
   ├── bench/
   ├── examples/
   │   └── imdb/
   ├── include/
   │   └── nwgraph/
   │       ├── adaptors/
   │       ├── algorithms/
   │       ├── containers/
   │       ├── experimental/
   │       │   └── algorithms/
   │       ├── graphs/
   │       ├── io/
   │       ├── util/
   │       ├── graph_concepts.hpp
   │       └── ...
   ├── test/
   └── ...

The genericity of different algorithms available in the NWGraph library
stems from a taxonomy of graph concepts. The definition of these
concepts can be found in the ``include/nwgraph/graph_concepts.hpp``
file. The header files containing various sequential and parallel graph
algorithms for well-known graph kernels can be found under the
``$NWGraph_HOME/include/nwgraph/algorithms/`` directory (some of the
experimental algorithms are located in
the\ ``$NWGraph_HOME/include/nwgraph/experimental/algorithms/``
subdirectory). The header files for the range adaptors are under
``$NWGraph_HOME/include/nwgraph/adaptors/`` directory. The code for the
applications is located in the ``$NWGraph_HOME/bench/`` diretory. The
abstraction penalty benchmark for benchmarking different containers and
a variety of different ways to iterate through a graph (including the
use of graphadaptors) are under the ``$NWGraph_HOME/apb/`` directory.
Various examples of how to use NWGraph can be found in the
``$NWGraph_HOME/example/imdb/`` directory.

How to Compile
--------------

NWGraph uses `Intel OneTBB <https://github.com/oneapi-src/oneTBB>`__ as
the parallel backend.

Requirements
~~~~~~~~~~~~

-  CMake >= 3.20
-  g++ >= 11 with support for OneTBB as parallel backend
-  oneTBB >= 2021

You should be able to install cmake and g++ with your system’s package
manager (e.g., apt or homebrew). oneTBB appears to be available on
homebrew for MacOS 11.6 and later (and perhaps earlier).

Instructions for installing oneTBB with various Linux package managers
can be found here:

::

   https://www.intel.com/content/www/us/en/developer/articles/tool/oneapi-standalone-components.html

Installation packages for oneAPI for Linux are available on intel.com:

::

   https://www.intel.com/content/www/us/en/developer/articles/tool/oneapi-standalone-components.html#onetbb

Compilation
~~~~~~~~~~~

.. code:: bash

   $ mkdir build; cd build
   $ cmake ..
   $ make -j4

Once compiled, the drivers of the graph benchmarks can be found under
the ``$NWGraph_HOME/build/bench/`` folder. The binary files of the
abstraction penalty benchmarks are under the
``$NWGraph_HOME/build/abp/`` folder. The binaries of the IMDB examples
are under the ``$NWGraph_HOME/build/examples/`` folder. The binary files
of the examples to show case the features of NWGraph library are under
the ``$NWGraph_HOME/build/test/`` folder.

Useful things to know
~~~~~~~~~~~~~~~~~~~~~

To specify compiler:

.. code:: bash

   $ cmake .. -DCMAKE_CXX_COMPILER=g++-11

To specify build type as Release or Debug, default is Release:

.. code:: bash

   $ cmake .. -DCMAKE_BUILD_TYPE=Release (or Debug)

To enable test cases and examples under build/test directory:

.. code:: bash

   $ cmake .. -DNW_GRAPH_BUILD_TESTS=ON (or OFF)

To generate applications under build/bench/ directory:

.. code:: bash

   $ cmake .. -DNW_GRAPH_BUILD_BENCH=ON (or OFF)

To generate abstraction penalty under build/abp/ directory:

.. code:: bash

   $ cmake .. -DNW_GRAPH_BUILD_APBS=OFF (or ON)

To generate tools under build/example/ directory:

.. code:: bash

   $ cmake .. -DNW_GRAPH_BUILD_EXAMPLES=OFF (or ON)

If cmake is not able to find TBB in its expected places, you may get an
error during the cmake step. In this case, you need to set the
``TBBROOT`` environment variable to the location where oneTBB was
installed. For example:

.. code:: bash

   $ TBBROOT=/opt/intel/oneapi/tbb/2021.5.1 cmake .. 

To see verbose information during compilation:

.. code:: bash

   $ make VERBOSE=1

Running code in NWGraph
-----------------------

NWGraph uses command-line interface description language
`DOCOPT <http://docopt.org/>`__ to define the interface of our
command-line applications and abstraction penalty experiments.

A typical interface of a benchmark driver looks like this:

.. code:: bash

   bfs.exe: breadth first search benchmark driver.
     Usage:
         bfs.exe (-h | --help)
         bfs.exe -f FILE [-r NODE | -s FILE] [-i NUM] [-a NUM] [-b NUM] [-B NUM] [-n NUM] [--seed NUM] [--version ID...] [--log FILE] [--log-header] [-dvV] [THREADS]...

     Options:
         -h, --help              show this screen
         -f FILE                 input file path
         -i NUM                  number of iteration [default: 1]
         -a NUM                  alpha parameter [default: 15]
         -b NUM                  beta parameter [default: 18]
         -B NUM                  number of bins [default: 32]
         -n NUM                  number of trials [default: 1]
         -r NODE                 start from node r (default is random)
         -s, --sources FILE      sources file
         --seed NUM              random seed [default: 27491095]
         --version ID            algorithm version to run [default: 0]
         --log FILE              log times to a file
         --log-header            add a header to the log file
         -d, --debug             run in debug mode
         -v, --verify            verify results
         -V, --verbose           run in verbose mode

The applications takes options followed by the arguments of the options
as inputs. A minimal example takes a graph as input is as follow:

::

   $ bfs.exe -f karate.mtx

Supported graph file format
---------------------------

NWGraph recogonizes the following types of file format: \* `Matrix
Market Exchange
Formats <https://math.nist.gov/MatrixMarket/formats.html>`__

Running benchmarks
------------------

We have six main benchmarks: Breadth-first Search, Betweenness Centrality, Connected Component
Decomposition, Page rank, Single Source Shortest Path, and Triangle
Counting.

Breadth-first Search
~~~~~~~~~~~~~~~~~~~~

The default sequential version of BFS is version 0 (default). The
fastest parallel version of BFS is version 11, the direction-optimizing
BFS. As an alternative to specifying one seed at a time, one or more
sources can be provided in a Matrix Market format file as an input of
BFS driver. Also, number of trials can be specified with ``-n``. In this
way, if no seed or seed file is provided, each trial will generate one
random number from 0 to \|V|-1 as the random source for BFS as an input.

::

   $ bench/bfs.exe -f karate.mtx --seed 0 --version 11 -n 3

Connected Component Decomposition
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default sequential version of CC is version 0 (default). The fastest
parallel version of CC is version 7, Afforest.

::

   $ bench/cc.exe -f karate.mtx --relabel --direction ascending

Page Rank
~~~~~~~~~

The fastest parallel version of PR is version 11 (default). The max
iterations can be set with ``-i``.

::

   $ bench/pr.exe -f karate.mtx -i 1000

Single Source Shortest Path
~~~~~~~~~~~~~~~~~~~~~~~~~~~

The default sequential version of CC SSSP version 0 (default). The
fastest parallel version of SSSP is version 12, Delta-stepping. As an
alternative to specifying one seed at a time, one or more sources can be
provided in a Matrix Market format file as an input of SSSP driver.
Also, number of trials can be specified with ``-n``. In this way, if no
seed or seed file is provided, each trial will generate one random
number from 0 to \|V|-1 as the random source for SSSP as an input.

::

   $ bench/sssp.exe -f karate.mtx --seed 0 -n 3

Triangle Counting
~~~~~~~~~~~~~~~~~

The default sequential version of TC is version 0 (default). The fastest
parallel version of TC is version 4.

::

   $ bench/tc.exe -f karate.mtx --version 4 --relabel --upper

Betweenness Centrality
~~~~~~~~~~~~~~~~~~~~~~

The default sequential version of BC is version 0 (default). The fastest
parallel version of BC is version 5. As an alternative to specifying one
seed at a time, one or more sources can be provided in a Matrix Market
format file as an input of BC driver.

::

   $ bench/bc.exe -f karate.mtx --version 5 --seed 0

Other useful things
~~~~~~~~~~~~~~~~~~~

Note that the following features may or may be available to every
benchmark.

Relabel-by-degree
^^^^^^^^^^^^^^^^^

Relabel vertex by degree (also known as column/row permutation in
matrix-matrix multiplication) may speed up the performance of the graph
algorithm. It can improve the workload distribution and memory access
pattern of the algorithm itself. To enable relabel-by-degree and relabel
the degree of vertices in ascending order:

::

   $ bench/cc.exe -f karate.mtx --relabel --direction ascending

Upper Triangular Order
~~~~~~~~~~~~~~~~~~~~~~

In triangle counting, it allows to relabel the graph in upper/lower
triangular order. This will greatly improve the performance of the
algorithm. To enable relabel-by-degree and relabel the degree of
vertices in upper triangular order:

.. code:: bash

   $ bench/tc.exe -f karate.mtx --relabel --upper

Verifier
~~~~~~~~

We implement a verifier in each benchmark to verify the correctness of
the algorithms. To enable the verification of the algorithm:

.. code:: bash

   $ bench/cc.exe -f karate.mtx -v

or

.. code:: bash

   $ bench/cc.exe -f karate.mtx --verify

Multi-threading
~~~~~~~~~~~~~~~

Each algorithm/benchmark has both sequential version and parallel
version. When a parallel algorithm is selected, multi-threading is
enable by default. The number of threads is set to be the maximum
available core on the machine. To enable multi-threading with different
thread number, such as 128 threads:

.. code:: bash

   $ bench/cc.exe -f karate.mtx 128

Benchmarking with GAP Datasets
------------------------------

To obtain the performance results reported in the PVLDB paper for
NWGraph, “NWGraph: A Library of Generic Graph Algorithms and
DataStructures in C++20”, please follow the following steps.

-  Download the GAP datasets from `Suitesparse Matrix
   Collection <https://sparse.tamu.edu/GAP/>`__ in Matrix Market format
-  Run different graph benchmarks with the GAP datasets

Note that BFS and SSSP are run with 64 sources provided in a Matrix
Market file, and BC are run with 4 sources. For PR, the max iterations
has been set to 1000.

Benchmarking abstraction penalties
----------------------------------

What is abstraction penalty?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are two types of abstraction penalties here. Using a range-based
interface introduces a variety of different ways to iterate through a
graph (including the use of graph adaptors). While ranges and range
based for loops are useful programming abstractions, it is important to
consider any performance abstraction penalties associated with their
use. We benchmark these penalties to ensure they will not significantly
limit performance compared to a raw for loop implementation.

We also evaluated the abstraction penalty incurred for storing a graph
in different containers. In particular, we have selected
``struct_of_array``, ``vector_of_vector``, ``vector_of_list``,
``vector_of_forward_list`` containers.

Running abstraction penalty experiments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For example let us consider the sparse matrix-dense vector
multiplication (SpMV) kernel used in page rank, which multiplies the
adjacency matrix representation of a graph by a dense vector x and
stores the result in another vector y. To experimentally evaluate the
abstraction penalty of different ways to iterate through a graph:

.. code:: bash

   $ apb/spmv.exe -f karate.mtx

To experimentally evaluate the abstraction penalty of different
containers for storing a graph:

.. code:: bash

   $ apb/containers -f karate.mtx --format CSR --format VOV --format VOL --format VOF

.. |Build with gcc-11| image:: https://github.com/NWmath/NWgr/workflows/Build%20with%20gcc-11/badge.svg
.. |image1| image:: https://github.com/NWmath/NWgr/workflows/Build%20with%20gcc-11/badge.svg?branch=sc_release
.. |image2| image:: https://github.com/NWmath/NWgr/workflows/Build%20with%20gcc-11%20(Mac)/badge.svg?branch=sc_release
.. |Codacy Badge| image:: https://app.codacy.com/project/badge/Grade/0788903a1d134b47b351e6a346123875
   :target: https://www.codacy.com?utm_source=github.com&utm_medium=referral&utm_content=NWmath/NWgr&utm_campaign=Badge_Grade
