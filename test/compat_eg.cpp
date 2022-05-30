/**
 * @file compat_eg.cpp
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



#include <forward_list>
#include <list>
#include <tuple>
#include <vector>

#include "nwgraph/compat.hpp"
#include "nwgraph/containers/zip.hpp"
#include "nwgraph/algorithms/bfs.hpp"

#include "nwgraph/graph_concepts.hpp"

using namespace nw::graph;


using fot = std::forward_list<std::tuple<int,int>>;
using lot = std::list<std::tuple<int,int>>;
using vot = std::vector<std::tuple<int,int>>;

using tof = std::tuple<std::forward_list<int>,std::forward_list<int>>;
using tol = std::tuple<std::list<int>,std::list<int>>;
using tov = std::tuple<std::vector<int>,std::vector<int>>;

using vof = std::vector<std::forward_list<std::tuple<int>>>;
using vol = std::vector<std::list<std::tuple<int>>>;
using vov = std::vector<std::vector<std::tuple<int>>>;



static_assert(vertex_list_c<std::forward_list<int>>);
static_assert(vertex_list_c<std::list<int>>);
static_assert(vertex_list_c<std::vector<int>>);


static_assert(edge_list_c<fot>);
static_assert(edge_list_c<lot>);
static_assert(edge_list_c<vot>);


// Need a single iterator returning tuple
// static_assert(edge_list_c<tof>);
// static_assert(edge_list_c<tol>);
// static_assert(edge_list_c<tov>);


// Zip only defined for random access at the moment
// static_assert(edge_list_c<zipped<std::forward_list<int>,std::forward_list<int>>>);
// static_assert(edge_list_c<zipped<std::list<int>,std::list<int>>>);
static_assert(edge_list_c<zipped<std::vector<int>,std::vector<int>>>);


// static_assert(edge_list_graph<fot>); // Has neither num_edges nor num_vertices
// static_assert(edge_list_graph<lot>); // Has num_edges, does not have num_vertices
// static_assert(edge_list_graph<vot>); // Has num_edges, does not have num_vertices


static_assert(adjacency_list_graph<vof>);
static_assert(adjacency_list_graph<vol>);
static_assert(adjacency_list_graph<vov>);

int main() {

  bfs(vov(), 0);
  bfs(vol(), 0);
  bfs(vof(), 0);

  return 0;
}
