.. SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
.. SPDX-FileCopyrightText: 2022 University of Washington
..
.. SPDX-License-Identifier: BSD-3-Clause

============================================
Chapter 7.2: Strongly Connected Components
============================================

Overview
--------

This example demonstrates Tarjan's algorithm for finding strongly connected
components (SCCs) in a directed graph. An SCC is a maximal set of vertices
where every vertex is reachable from every other vertex following edge
directions.

**Applications:**

- Web page analysis (finding clusters of mutually linking pages)
- Dependency analysis (detecting circular dependencies)
- Model checking (decomposing state spaces)
- Social network analysis (identifying cohesive groups)

Algorithm Description
--------------------

Tarjan's algorithm finds all SCCs in a single DFS traversal using the
concept of "lowlink" values:

**Key concepts:**

- **Discovery time**: When a vertex is first visited
- **Lowlink**: Smallest discovery time reachable from this vertex's subtree

**Algorithm steps:**

1. Perform DFS, recording discovery times
2. Track vertices on the current DFS path (on stack)
3. When backtracking, update lowlink values
4. A vertex is an SCC root if lowlink equals discovery time
5. Pop vertices from stack to form SCC when root is found

**Time complexity**: O(V + E) - single DFS traversal

NWGraph Implementation
---------------------

The example implements Tarjan's algorithm with the BGL book's web graph
application:

.. literalinclude:: ../../../../examples/bgl-book/ch7_strongly_connected.cpp
   :language: cpp
   :linenos:
   :lines: 1-40
   :caption: File header, includes, and algorithm explanation

The core Tarjan's algorithm:

.. literalinclude:: ../../../../examples/bgl-book/ch7_strongly_connected.cpp
   :language: cpp
   :linenos:
   :lines: 42-120
   :caption: Tarjan's SCC algorithm

The web graph example:

.. literalinclude:: ../../../../examples/bgl-book/ch7_strongly_connected.cpp
   :language: cpp
   :linenos:
   :lines: 150-200
   :caption: Web page graph analysis

Running the Example
------------------

.. code-block:: bash

   cd build/examples/bgl-book
   ./ch7_strongly_connected

Sample Output
------------

.. code-block:: text

   === Strongly Connected Components ===
   Based on BGL Book Chapter 7

   Web Graph Analysis
   -----------------
   Modeling web pages as vertices and hyperlinks as directed edges.
   An SCC represents a cluster of pages that all link to each other
   (directly or through a chain of links).

   Graph structure:
     A -> B (A links to B)
     B -> C, E
     C -> D, F
     D -> C (cycle: C-D)
     E -> A, F (cycle: A-B-E)
     F -> G
     G -> F (cycle: F-G)

   Finding strongly connected components...

   Found 4 SCCs:

   SCC 1: {A, B, E}
     These pages form a cycle and mutually reference each other.

   SCC 2: {C, D}
     These pages form a smaller cycle.

   SCC 3: {F, G}
     These pages reference each other.

   SCC 4: {H}
     This page is isolated (no incoming or outgoing links in cycles).

Key NWGraph Features Demonstrated
--------------------------------

- **Recursive DFS**: Natural implementation with NWGraph's adjacency
- **Stack-based SCC identification**: Track vertices on current path
- **edge_list<directedness::directed>**: Directed edges for web links
- **Discovery and lowlink arrays**: O(V) auxiliary storage

Tarjan's Algorithm Details
--------------------------

The algorithm uses three arrays:

.. code-block:: cpp

   std::vector<size_t> disc;     // Discovery time
   std::vector<size_t> low;      // Lowlink value
   std::vector<bool> on_stack;   // Is vertex on current DFS stack?
   std::stack<size_t> st;        // The DFS stack

The key insight is the lowlink update:

.. code-block:: cpp

   void tarjan_dfs(size_t u) {
       disc[u] = low[u] = time++;
       st.push(u);
       on_stack[u] = true;

       for (auto&& [v] : G[u]) {
           if (disc[v] == UNVISITED) {
               tarjan_dfs(v);
               low[u] = std::min(low[u], low[v]);
           } else if (on_stack[v]) {
               low[u] = std::min(low[u], disc[v]);
           }
       }

       // If u is root of SCC
       if (low[u] == disc[u]) {
           // Pop all vertices in this SCC
           while (st.top() != u) {
               scc_id[st.top()] = current_scc;
               on_stack[st.top()] = false;
               st.pop();
           }
           scc_id[u] = current_scc++;
           on_stack[u] = false;
           st.pop();
       }
   }

Web Graph Applications
----------------------

SCCs in web graphs reveal:

- **Link farms**: Pages that artificially boost each other's rankings
- **Topic clusters**: Pages about related subjects
- **Navigation structures**: Site sections with internal linking
- **Spam detection**: Unusual SCC patterns may indicate manipulation

SCC DAG
-------

After finding SCCs, we can contract each SCC to a single vertex, creating
a DAG (directed acyclic graph). This "SCC DAG" is useful for:

- Finding the order to process components
- Understanding the high-level structure of directed graphs
- Simplifying reachability queries

See Also
--------

- :doc:`ch7_connected` - Connected components for undirected graphs
- :doc:`ch4_loop_detection` - Cycle detection using DFS
- :doc:`ch3_toposort` - Topological sort (requires DAG, no SCCs with >1 vertex)
