.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

=========================
Benchmarking with NWGraph
=========================

This guide covers how to build and run the NWGraph benchmarks, including the
GAP Benchmark Suite implementations.

Building Benchmarks
===================

NWGraph benchmarks are disabled by default. To build them:

.. code-block:: bash

   # From the NWGraph root directory
   mkdir -p build && cd build

   # Configure with benchmarks enabled
   cmake .. -DNWGRAPH_BUILD_BENCH=ON

   # Build all benchmarks
   make bench -j8

   # Or build individual benchmarks
   make bfs bc cc pr sssp tc -j8

The benchmark executables are placed in ``build/bench/gapbs/``.

Quick Test
----------

To verify benchmarks are working, run with a small test graph:

.. code-block:: bash

   # From the build directory
   ./bench/gapbs/bfs -f ../test/data/karate.mtx -n 3 --verify
   ./bench/gapbs/cc -f ../test/data/karate.mtx --verify
   ./bench/gapbs/tc -f ../test/data/karate.mtx --verify

GAP Benchmark Suite
===================

NWGraph implements the six kernels from the GAP Benchmark Suite:

.. list-table:: GAP Benchmark Kernels
   :header-rows: 1
   :widths: 15 35 20 30

   * - Kernel
     - Description
     - Best Version
     - Notes
   * - **BFS**
     - Breadth-First Search
     - ``--version 11``
     - Direction-optimizing BFS
   * - **SSSP**
     - Single-Source Shortest Path
     - ``--version 12``
     - Delta-stepping algorithm
   * - **PR**
     - PageRank
     - ``--version 11``
     - 1000 iterations typical
   * - **CC**
     - Connected Components
     - ``--version 7``
     - Afforest algorithm
   * - **BC**
     - Betweenness Centrality
     - ``--version 5``
     - Brandes' algorithm
   * - **TC**
     - Triangle Counting
     - ``--version 4``
     - Requires ``--relabel --upper``

GAP Benchmark Datasets
======================

The GAP Benchmark Suite uses five standard graphs for evaluation. These graphs
are available from the `SuiteSparse Matrix Collection <https://sparse.tamu.edu/GAP>`_.

Dataset Overview
----------------

.. list-table:: GAP Benchmark Graphs
   :header-rows: 1
   :widths: 12 15 15 15 43

   * - Name
     - Vertices
     - Edges
     - Download Size
     - Description
   * - **twitter**
     - 61,578,415
     - 1,468,365,182
     - ~26 GB
     - Twitter follower network snapshot. Exhibits realistic social network
       properties with highly skewed degree distribution (power-law).
   * - **web**
     - 50,636,154
     - 1,949,412,601
     - ~7 GB
     - Web crawl of the .sk (Slovakia) domain from 2005. From the Laboratory
       for Web Algorithmics. Shows substantial locality.
   * - **road**
     - 23,947,347
     - 58,333,344
     - ~1 GB
     - Road network of the entire USA. Notable for being much smaller but
       having very high diameter (~6000 hops).
   * - **kron**
     - 134,217,726
     - 2,111,632,938
     - Generated
     - Kronecker synthetic graph using Graph 500 parameters (scale 27, edge
       factor 16). Exhibits scale-free properties.
   * - **urand**
     - 134,217,728
     - 2,147,483,648
     - Generated
     - Erdos-Renyi uniform random graph (2^27 vertices, degree 16). Represents
       worst-case memory access locality.

Detailed Dataset Information
----------------------------

Twitter (GAP-twitter)
~~~~~~~~~~~~~~~~~~~~~

:Source: ANLAB-KAIST GitHub repository
:Original: Twitter follower graph (Kwak et al., WWW 2010)
:Format: Edge list (.el)
:Type: Directed, unweighted
:Characteristics:
   - Power-law degree distribution
   - Maximum in-degree: 2,997,469
   - Maximum out-degree: 770,155
   - Average degree: ~48

The Twitter graph is split into 4 compressed parts for download. Use the
download script to automatically fetch and combine them.

Web (GAP-web / sk-2005)
~~~~~~~~~~~~~~~~~~~~~~~

:Source: `SuiteSparse Matrix Collection <https://sparse.tamu.edu/LAW/sk-2005>`_
:Original: Laboratory for Web Algorithmics
:Format: Matrix Market (.mtx)
:Type: Directed, unweighted
:Characteristics:
   - Web crawl locality patterns
   - Maximum out-degree: 8,563
   - High clustering coefficient
   - Contains many small SCCs

Road (GAP-road / USA-road-d)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:Source: `9th DIMACS Implementation Challenge <http://www.dis.uniroma1.it/challenge9/>`_
:Original: TIGER/Line road network data
:Format: DIMACS graph format (.gr)
:Type: Directed, weighted (distances in meters)
:Characteristics:
   - Very high diameter (~6000)
   - Low average degree (~2.4)
   - Nearly planar structure
   - Weights represent travel distances

This graph tests algorithms on high-diameter, low-degree networks.

Kron (GAP-kron)
~~~~~~~~~~~~~~~

:Source: Generated using Graph 500 Kronecker generator
:Parameters: Scale 27, edge factor 16 (``-g27 -k16``)
:Format: Generated directly to binary
:Type: Undirected, unweighted
:Characteristics:
   - Scale-free degree distribution
   - Small-world properties
   - Self-similar structure
   - Synthetic but realistic

Generate with the GAPBS converter:

.. code-block:: bash

   ./converter -g27 -k16 -b kron.sg

Urand (GAP-urand)
~~~~~~~~~~~~~~~~~

:Source: Generated using Erdos-Renyi random generator
:Parameters: 2^27 vertices, degree 16 (``-u27 -k16``)
:Format: Generated directly to binary
:Type: Undirected, unweighted
:Characteristics:
   - Uniform random edge placement
   - Worst-case memory locality
   - No clustering structure
   - Binomial degree distribution around 16

Generate with the GAPBS converter:

.. code-block:: bash

   ./converter -u27 -k16 -b urand.sg

Downloading Graphs
==================

Using the Download Script
-------------------------

NWGraph provides a script to download the GAP benchmark graphs:

.. code-block:: bash

   # Show available graphs and sizes
   ./scripts/download_gap_graphs.sh --list

   # Download road network only (smallest, ~1GB)
   ./scripts/download_gap_graphs.sh road

   # Download multiple graphs
   ./scripts/download_gap_graphs.sh road web

   # Download all real-world graphs
   ./scripts/download_gap_graphs.sh all

   # Download small test graphs (for quick testing)
   ./scripts/download_gap_graphs.sh --small

   # Specify custom download directory
   ./scripts/download_gap_graphs.sh -d /data/gap road web

Manual Download
---------------

Graphs can also be downloaded manually from SuiteSparse:

1. Visit https://sparse.tamu.edu/GAP
2. Click on the desired graph
3. Download in Matrix Market format
4. Extract and place in your data directory

Storage Requirements
--------------------

.. list-table:: Disk Space Requirements
   :header-rows: 1
   :widths: 20 20 20 40

   * - Graph
     - Compressed
     - Uncompressed
     - Notes
   * - Small test set
     - ~5 MB
     - ~10 MB
     - Good for development
   * - road
     - ~1 GB
     - ~2 GB
     - Recommended starting point
   * - web
     - ~7 GB
     - ~20 GB
     - Moderate size
   * - twitter
     - ~26 GB
     - ~80 GB
     - Largest real-world graph
   * - kron (generated)
     - N/A
     - ~25 GB
     - Generated in memory
   * - urand (generated)
     - N/A
     - ~25 GB
     - Generated in memory

**Total for all graphs:** ~150 GB (plus ~64 GB RAM for generation)

File Formats
============

NWGraph supports multiple graph file formats:

Matrix Market (.mtx)
--------------------

The standard format for sparse matrices. Human-readable but slower to load.

.. code-block:: text

   %%MatrixMarket matrix coordinate pattern general
   % Comment lines start with %
   5 5 8
   1 2
   1 3
   2 3
   ...

Edge List (.el)
---------------

Simple format with one edge per line:

.. code-block:: text

   0 1
   0 2
   1 2
   ...

DIMACS (.gr)
------------

Format from the DIMACS Implementation Challenges:

.. code-block:: text

   c Comment lines
   p sp 5 8
   a 1 2 10
   a 1 3 5
   ...

NWGraph Binary (.nw)
--------------------

Native binary format for fast loading. Convert using:

.. code-block:: bash

   ./bench/gapbs/process_edge_list -d output.nw input.mtx

Running Benchmarks
==================

Standard Benchmark Parameters
-----------------------------

For reproducible benchmarking matching the GAP methodology:

.. code-block:: bash

   # BFS: 64 random sources
   ./bench/gapbs/bfs -f graph.mtx -n 64 --version 11

   # SSSP: 64 random sources, delta=2 (or 50000 for road)
   ./bench/gapbs/sssp -f graph.wsg -n 64 -d 2 --version 12

   # PR: 1000 iterations
   ./bench/gapbs/pr -f graph.mtx -i 1000 --version 11

   # CC: 16 trials
   ./bench/gapbs/cc -f graph.mtx -n 16 --version 7

   # BC: 4 sources, 16 trials
   ./bench/gapbs/bc -f graph.mtx -i 4 -n 16 --version 5

   # TC: Requires undirected, upper triangular
   ./bench/gapbs/tc -f graph.mtx -n 3 --relabel --upper --version 4

Multi-threading
---------------

Thread count is specified as a positional argument:

.. code-block:: bash

   # Run with 16 threads
   ./bench/gapbs/bfs -f graph.mtx -n 10 16

   # Run with 64 threads
   ./bench/gapbs/cc -f graph.mtx 64

Verification
------------

Add ``--verify`` to check correctness:

.. code-block:: bash

   ./bench/gapbs/bfs -f ../test/data/karate.mtx --verify

Logging Results
---------------

Output results to a file:

.. code-block:: bash

   ./bench/gapbs/bfs -f graph.mtx -n 64 --log results.csv --log-header

References
==========

- Beamer, S., Asanovic, K., & Patterson, D. (2015). *The GAP Benchmark Suite*.
  arXiv:1508.03619. https://arxiv.org/abs/1508.03619

- SuiteSparse Matrix Collection: https://sparse.tamu.edu/

- GAP Benchmark Reference Implementation: https://github.com/sbeamer/gapbs

- 9th DIMACS Implementation Challenge (Shortest Paths):
  http://www.dis.uniroma1.it/challenge9/

See Also
========

- :doc:`getting_started` - Installation and basic usage
- :doc:`../examples/bglbook/index` - BGL Book examples
