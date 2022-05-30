/**
 * @file cstar_eg.cpp
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


#include "nwgraph/csr.hpp"
#include "nwgraph/csc.hpp"
#include "nwgraph/coo.hpp"

using namespace nw::graph;

int main() {
  size_t N = 5;

  csc_graph A(N);
  csr_graph B(N);
  coo_graph C(N);

  csc_matrix<> D(N);
  csr_matrix<> E(N);
  coo_matrix<double> F(N);  // coo_matrix F(N) causes internal compiler error gcc 10.2.0

  csc_matrix<float> G(N);
  csr_matrix<float> H(N);
  coo_matrix<float> I(N);

  return 0;
}
