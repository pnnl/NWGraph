/**
 * @file k_truss.hpp
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

#ifndef NW_GRAPH_K_TRUSS_HPP
#define NW_GRAPH_K_TRUSS_HPP

#include "nwgraph/adaptors/cyclic_range_adapter.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include <atomic>
#include <future>
#include <thread>

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/util/intersection_size.hpp"
#include "util/timer.hpp"
#include "nwgraph/util/util.hpp"

#include <tuple>
#include <vector>

namespace nw {
namespace graph {}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_K_TRUSS_HPP
