/**
 * @file spmm_eg.cpp
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

//#pragma once

#include <algorithm>
#include <tuple>

#include "nwgraph/algorithms/spMatspMat.hpp"
#include "nwgraph/containers/aos.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/io/mmio.hpp"
#include "nwgraph/util/util.hpp"

using namespace nw::graph;
using namespace nw::util;

//****************************************************************************
int main(int argc, char* argv[]) {
  // call with data/spmatA.mmio and data/spmatB.mmio
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <matA.mmio> <matB.mmio>" << std::endl;
    return -1;
  }

  auto aos_A = read_mm<directedness::directed, float>(argv[1]);
  auto aos_B = read_mm<directedness::directed, float>(argv[2]);
  std::cout << "A = " << std::endl;
  aos_A.stream(std::cout);
  std::cout << "B = " << std::endl;
  aos_B.stream(std::cout);

  adjacency<0, float> A(aos_A);
  adjacency<0, float> B(aos_B);

  std::cout << "C = A * B = " << std::endl;
  auto edges(spMatspMat<float>(A, B));
  edges.stream(std::cout);

  std::cout << "C = A * B' = " << std::endl;
  auto edgesABT(spMatspMatT<float>(A, B));
  edgesABT.stream(std::cout);
}
