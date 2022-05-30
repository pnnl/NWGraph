/**
 * @file spMatspMat_test.cpp
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


#include <tuple>
#include <vector>

#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/adaptors/edge_range.hpp"
#include "common/test_header.hpp"

#include "nwgraph/algorithms/spMatspMat.hpp"

TEST_CASE("Row times Row", "[row-row") {

  // Create A, B and known correct answer
  //   By hand
  //   Generate random A, B compute C with known, slow, approach
  //     (e.g., convert A, B to dense
  // Compute C = A * B with spMatspMat
  // Compare C with known correct answer
    
  /* 
     [ 3, 1, 4 ]   [ 8, 6, 7 ]     [  65, 21, 57 ]
     [ 1, 5, 9 ]   [ 5, 3, 0 ]  =  [ 114, 21, 88 ]
     [ 2, 6, 7 ]   [ 9, 0, 9 ]     [ 109, 30, 77 ]
   */

  using SparseMatrix = std::vector<std::vector<std::tuple<int, double>>>;

  SparseMatrix A {
    { { 0, 3 }, { 1, 1 }, { 2, 4 } },
    { { 0, 1 }, { 1, 5 }, { 2, 9 } },
    { { 0, 2 }, { 1, 6 }, { 2, 7 } },
  };
  SparseMatrix B {
    { { 0, 8 }, { 1, 6 }, { 2, 7 } },
    { { 0, 5 }, { 1, 3 }, { 2, 0 } },
    { { 0, 9 }, { 1, 0 }, { 2, 9 } },
  };
  SparseMatrix C {
    { { 0,  65 }, { 1, 21 }, { 2, 57 } },
    { { 0, 114 }, { 1, 21 }, { 2, 88 } },
    { { 0, 109 }, { 1, 30 }, { 2, 77 } },
  };

  auto d = nw::graph::spMatspMat<double>(A, B);

  REQUIRE(std::equal(begin(d), end(d), begin(nw::graph::make_edge_range<0>(C))));
}
