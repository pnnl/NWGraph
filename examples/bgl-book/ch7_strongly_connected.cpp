/**
 * @file ch7_strongly_connected.cpp
 *
 * @brief Strongly Connected Components and Web Page Links (BGL Book Chapter 7.3)
 *
 * This example demonstrates Tarjan's algorithm for finding strongly connected
 * components (SCCs) in a directed graph. A strongly connected component is a
 * maximal set of vertices where every vertex is reachable from every other vertex.
 *
 * Application: Web page analysis - groups of pages that link to each other form
 * strongly connected components. A study of 200 million web pages found that
 * 56 million pages formed one large SCC.
 *
 * Tarjan's algorithm runs in O(V + E) time using a single DFS traversal.
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *
 */

#include <algorithm>
#include <iostream>
#include <map>
#include <stack>
#include <vector>

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;

/**
 * @brief Tarjan's algorithm for strongly connected components
 *
 * This implementation uses a single DFS traversal to find all SCCs.
 * Each vertex is assigned a component ID (0, 1, 2, ...).
 *
 * The algorithm maintains:
 * - Discovery time for each vertex
 * - Low-link value (smallest discovery time reachable)
 * - A stack of vertices in the current DFS path
 */
template <typename Graph>
class TarjanSCC {
public:
  TarjanSCC(const Graph& g) : G(g), N(g.size()), index_counter(0), num_components(0) {
    index.assign(N, -1);
    lowlink.assign(N, -1);
    on_stack.assign(N, false);
    component.assign(N, -1);
  }

  size_t compute() {
    for (size_t v = 0; v < N; ++v) {
      if (index[v] == -1) {
        strongconnect(v);
      }
    }
    return num_components;
  }

  const std::vector<int>& get_components() const { return component; }

private:
  void strongconnect(size_t v) {
    // Set the discovery index and lowlink value
    index[v]   = index_counter;
    lowlink[v] = index_counter;
    ++index_counter;

    S.push(v);
    on_stack[v] = true;

    // Consider successors of v
    for (auto&& [w] : G[v]) {
      if (index[w] == -1) {
        // Successor w has not yet been visited; recurse on it
        strongconnect(w);
        lowlink[v] = std::min(lowlink[v], lowlink[w]);
      } else if (on_stack[w]) {
        // Successor w is on the stack and hence in the current SCC
        lowlink[v] = std::min(lowlink[v], index[w]);
      }
    }

    // If v is a root node, pop the stack and generate an SCC
    if (lowlink[v] == index[v]) {
      // Start a new strongly connected component
      size_t w;
      do {
        w            = S.top();
        S.pop();
        on_stack[w]  = false;
        component[w] = num_components;
      } while (w != v);
      ++num_components;
    }
  }

  const Graph& G;
  size_t N;
  int index_counter;
  size_t num_components;

  std::vector<int> index;     // Discovery time
  std::vector<int> lowlink;   // Lowest reachable discovery time
  std::vector<bool> on_stack;
  std::vector<int> component;
  std::stack<size_t> S;
};

/**
 * @brief Convenience function to compute strongly connected components
 */
template <typename Graph>
std::pair<size_t, std::vector<int>> strong_components(const Graph& G) {
  TarjanSCC<Graph> tarjan(G);
  size_t num_comp = tarjan.compute();
  return {num_comp, tarjan.get_components()};
}

int main() {
  std::cout << "=== Strongly Connected Components ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 7.3" << std::endl << std::endl;

  // Web pages graph from the BGL book (Figure 7.3)
  // Vertices represent web sites:
  // 0: www.boost.org
  // 1: anubis.dkuug.dk
  // 2: sourceforge.net
  // 3: www.lsc.nd.edu
  // 4: www.hp.com
  // 5: www.lam-mpi.org
  // 6: www.yahoogroups.com
  // 7: weather.yahoo.com
  // 8: nytimes.com
  // 9: www.boston.com

  std::vector<std::string> site_names = {"www.boost.org",  "anubis.dkuug.dk",    "sourceforge.net",  "www.lsc.nd.edu",
                                         "www.hp.com",     "www.lam-mpi.org",    "www.yahoogroups.com", "weather.yahoo.com",
                                         "nytimes.com",    "www.boston.com"};

  const size_t num_sites = site_names.size();

  std::cout << "Web site link graph (directed):" << std::endl;
  std::cout << "  " << num_sites << " web sites connected by URL links" << std::endl;
  std::cout << std::endl;

  // Create directed graph of URL links
  // Links based on the BGL book example structure
  edge_list<directedness::directed> edges(num_sites);
  edges.open_for_push_back();

  // Create a structure with some strongly connected components
  // Component 1: boost.org <-> sourceforge.net <-> lsc.nd.edu (mutual links)
  edges.push_back(0, 2);  // boost.org -> sourceforge.net
  edges.push_back(2, 0);  // sourceforge.net -> boost.org
  edges.push_back(2, 3);  // sourceforge.net -> lsc.nd.edu
  edges.push_back(3, 0);  // lsc.nd.edu -> boost.org

  // Component 2: anubis.dkuug.dk (single vertex)
  edges.push_back(0, 1);  // boost.org -> anubis.dkuug.dk (one-way)

  // Component 3: hp.com <-> lam-mpi.org (mutual links)
  edges.push_back(4, 5);  // hp.com -> lam-mpi.org
  edges.push_back(5, 4);  // lam-mpi.org -> hp.com
  edges.push_back(3, 4);  // lsc.nd.edu -> hp.com (one-way into component)

  // Component 4: yahoogroups.com <-> weather.yahoo.com (mutual links)
  edges.push_back(6, 7);  // yahoogroups.com -> weather.yahoo.com
  edges.push_back(7, 6);  // weather.yahoo.com -> yahoogroups.com

  // Component 5: nytimes.com <-> boston.com (mutual links)
  edges.push_back(8, 9);  // nytimes.com -> boston.com
  edges.push_back(9, 8);  // boston.com -> nytimes.com
  edges.push_back(7, 8);  // weather.yahoo.com -> nytimes.com (one-way)

  edges.close_for_push_back();

  // Build adjacency representation
  adjacency<0> G(edges);

  // Find strongly connected components
  auto [num_components, component] = strong_components(G);

  std::cout << "Found " << num_components << " strongly connected components:" << std::endl;
  std::cout << std::endl;

  // Group sites by component
  std::map<int, std::vector<size_t>> comp_members;
  for (size_t v = 0; v < num_sites; ++v) {
    comp_members[component[v]].push_back(v);
  }

  // Display components
  for (auto&& [comp_id, members] : comp_members) {
    std::cout << "Component " << comp_id << " (" << members.size() << " site"
              << (members.size() > 1 ? "s" : "") << "):" << std::endl;
    for (auto v : members) {
      std::cout << "  - " << site_names[v] << std::endl;
    }
    std::cout << std::endl;
  }

  // Show vertex-to-component mapping
  std::cout << "Vertex to component mapping:" << std::endl;
  for (size_t v = 0; v < num_sites; ++v) {
    std::cout << "  " << site_names[v] << " -> Component " << component[v] << std::endl;
  }

  // Example: Simple directed graph with clear SCC structure
  std::cout << std::endl;
  std::cout << "=== Simple Example ===" << std::endl;
  std::cout << std::endl;

  // Graph with 8 vertices forming 3 SCCs:
  // SCC1: {0, 1, 2} - cycle
  // SCC2: {3, 4, 5, 6} - cycle
  // SCC3: {7} - single vertex
  // Plus edges: 2->3 (connects SCC1 to SCC2), 6->7 (connects SCC2 to SCC3)

  edge_list<directedness::directed> edges2(8);
  edges2.open_for_push_back();

  // SCC1: 0 -> 1 -> 2 -> 0
  edges2.push_back(0, 1);
  edges2.push_back(1, 2);
  edges2.push_back(2, 0);

  // SCC2: 3 -> 4 -> 5 -> 6 -> 3
  edges2.push_back(3, 4);
  edges2.push_back(4, 5);
  edges2.push_back(5, 6);
  edges2.push_back(6, 3);

  // Cross-SCC edges (one-way)
  edges2.push_back(2, 3);  // SCC1 -> SCC2
  edges2.push_back(6, 7);  // SCC2 -> SCC3

  edges2.close_for_push_back();

  adjacency<0> G2(edges2);

  std::cout << "Graph structure:" << std::endl;
  std::cout << "  SCC1: 0 -> 1 -> 2 -> 0 (cycle)" << std::endl;
  std::cout << "  SCC2: 3 -> 4 -> 5 -> 6 -> 3 (cycle)" << std::endl;
  std::cout << "  SCC3: 7 (isolated sink)" << std::endl;
  std::cout << "  Cross edges: 2->3, 6->7" << std::endl;
  std::cout << std::endl;

  auto [num_comp2, comp2] = strong_components(G2);

  std::cout << "Found " << num_comp2 << " strongly connected components" << std::endl;

  std::map<int, std::vector<size_t>> members2;
  for (size_t v = 0; v < 8; ++v) {
    members2[comp2[v]].push_back(v);
  }

  for (auto&& [comp_id, members] : members2) {
    std::cout << "  Component " << comp_id << ": {";
    for (size_t i = 0; i < members.size(); ++i) {
      if (i > 0) std::cout << ", ";
      std::cout << members[i];
    }
    std::cout << "}" << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Note: Tarjan's algorithm runs in O(V + E) time using DFS." << std::endl;
  std::cout << "SCCs form a DAG when contracted - useful for dependency analysis." << std::endl;

  return 0;
}
