/**
 * @file abstract_test.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   liux238
 *
 */

#ifndef SGL_ABSTRACT_TEST
#define SGL_ABSTRACT_TEST

#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/io/mmio.hpp"
#include "test_header.hpp"

using namespace nw::graph;
using namespace nw::util;

typedef adjacency<0> directed_csr_graph_t;
class test_util {
public:
  // Constructs a directed graph with the mmio file
  // pointed by the file path DATA_FILE
  directed_csr_graph_t generate_directed_graph() {
    auto         aos_a = read_mm<nw::graph::directedness::directed>(DATA_FILE);
    adjacency<0> A(aos_a);
    return A;
  }

  // Constructs directed graph with the given
  // mmio file path
  directed_csr_graph_t generate_directed_graph(const std::string& mmiofile) {
    auto         aos_a = read_mm<nw::graph::directedness::directed>(mmiofile);
    adjacency<0> A(aos_a);
    return A;
  }

  // Constructs a directed graph with the mmio file
  // pointed by the file path DATA_FILE
  auto generate_directed_aos() { return read_mm<nw::graph::directedness::directed>(DATA_FILE); }

  // Constructs directed graph with the given
  // mmio file path
  auto generate_directed_aos(const std::string& mmiofile) { return read_mm<nw::graph::directedness::directed>(mmiofile); }
};
#endif
