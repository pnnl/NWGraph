/**
 * @file csr.hpp
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

#ifndef NW_GRAPH_CSR_HPP
#define NW_GRAPH_CSR_HPP

#include "nwgraph/adjacency.hpp"

namespace nw {
  namespace graph {

using csr_graph = adjacency<0>;

template <typename T = double>
using csr_matrix = adjacency<0, T>;

  }
}

#endif // NW_GRAPH_CSR_HPP
