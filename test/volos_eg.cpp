/**
 * @file volos_eg.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   liux238
 *
 */

#include <forward_list>
#include <tuple>
#include <vector>

#include "nwgraph/volos.hpp"

#include "nwgraph/graph_concepts.hpp"

int main() {

  using T = nw::graph::adj_list<0>;
  static_assert(nw::graph::graph<T>);
  static_assert(nw::graph::adjacency_list_graph<T>);

  return 0;
}
