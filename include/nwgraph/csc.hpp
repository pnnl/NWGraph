/**
 * @file csc.hpp
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

#ifndef NW_GRAPH_CSC_HPP
#define NW_GRAPH_CSC_HPP

#include "nwgraph/adjacency.hpp"

namespace nw {
  namespace graph {

using csc_graph = adjacency<1>;

template <typename T = double>
using csc_matrix = adjacency<1, T>;

  }
}

#endif // NW_GRAPH_CSC_HPP

