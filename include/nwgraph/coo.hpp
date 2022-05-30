/**
 * @file coo.hpp
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

#ifndef NW_GRAPH_COO_HPP
#define NW_GRAPH_COO_HPP

#include "nwgraph/edge_list.hpp"

namespace nw {
  namespace graph {

using coo_graph = edge_list<directedness::directed>;

template <typename T = double>
using coo_matrix = edge_list<directedness::directed, T>;

  }
}

#endif // NW_GRAPH_COO_HPP
