.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================
IMDB Network Analysis Examples
============================

These examples demonstrate real-world graph analysis using data from the
Internet Movie Database (IMDB) and other large-scale networks.

Overview
--------

The IMDB examples show how to:

- Process large datasets and build graphs from raw data
- Compute shortest paths between entities (actors, papers, etc.)
- Analyze network structure and connectivity
- Handle multiple related networks

Example Programs
----------------

imdb.cpp - Core IMDB Processing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The main IMDB graph loading and processing example. Reads actor-movie
relationships and builds the co-star graph.

**Key features:**

- Reading JSON-formatted IMDB data
- Building bipartite actor-movie graph
- Computing the co-star graph (actors who appeared together)
- Shortest path queries

oracle.cpp - Oracle of Bacon
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Implementation of the "Oracle of Bacon" - computing the Bacon number
(shortest path to Kevin Bacon) for any actor.

**Key features:**

- BFS-based shortest path computation
- Path reconstruction with movie names
- Interactive query interface

.. code-block:: cpp

   // Query example
   path_to_bacon("Tom Hanks");
   // Output: Tom Hanks -> Apollo 13 -> Kevin Bacon (Bacon number: 1)

dblp.cpp - DBLP Academic Network
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Analysis of the DBLP computer science bibliography network, where authors
are connected through co-authorship.

**Key features:**

- Academic collaboration network
- "Erdős number" style analysis
- Community detection in co-author networks

dns.cpp - DNS Network Topology
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Analysis of DNS (Domain Name System) network topology.

**Key features:**

- Network infrastructure analysis
- Connectivity and path analysis
- Critical node identification

oracle+dblp.cpp - Combined Analysis
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Demonstrates analyzing multiple networks together, finding connections
across different domains.

Building the Examples
---------------------

The IMDB examples require additional data files not included in the
repository. To build:

.. code-block:: bash

   mkdir build && cd build
   cmake .. -DNWGRAPH_BUILD_EXAMPLES=ON
   make

Data Requirements
-----------------

These examples require external data files:

- ``oracle.json`` - IMDB movie database in JSON format
- ``dblp.json`` - DBLP bibliography data

See the ``examples/imdb/download.sh`` script for data download instructions.

Performance Considerations
--------------------------

These examples demonstrate NWGraph's ability to handle large graphs:

- IMDB: ~500K actors, millions of edges
- DBLP: ~2M authors, millions of co-authorships

Key techniques for large-scale analysis:

1. **Efficient data structures**: CSR representation for minimal memory
2. **Parallel algorithms**: TBB-based parallelism for BFS
3. **Incremental construction**: Building graphs from streaming data

See Also
--------

- :doc:`../degrees/index` - Simpler Six Degrees example
- :doc:`../bglbook/ch4_kevin_bacon` - BGL book version of Bacon numbers
