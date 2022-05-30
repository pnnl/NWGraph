/**
 * @file math_eg.cpp
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


#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"
#include "linalg.hpp"

using namespace nw::graph;
using namespace nw::util;

template <typename Mat, typename EdgeList>
void run_test(EdgeList&& E) {
  std::vector<double> x = {3.1, -1.4, 1.59};
  std::vector<double> y = {1.0, 0.0, -1.0};

  Mat B(E);
  matvec(B, x, y);

  std::cout << std::endl;
  for (auto&& j : y) {
    std::cout << j << std::endl;
  }
}

int main() {

  run_test<compressed_sparse<0, directed, double>>(edge_list<directedness::directed, double>({{0, 0, 1}, {1, 1, 1}, {2, 2, 1}, {0, 1, -3.7}}));

  edge_list<directedness::directed, double> E({{0, 0, 1}, {1, 1, 1}, {2, 2, 1}, {0, 1, -3.7}});

  compressed_sparse<0, directed, double> A(E);

  auto& A_range = A;    //= make_edge_range<0>(A);
  // auto diag = A_range.begin();
  //  auto&& [ii, jj, vv] = *diag++;

  std::vector<double> x(4);

  vertex_id_type i = 0;
  std::for_each(A_range.begin(), A_range.end(), [&](auto&& j) {
    auto diag       = j.begin();
    auto&& [jj, vv] = *diag++;
    std::for_each(j.begin(), j.end(), [&](auto&& elt) {
      auto&& [k, v] = elt;
      x[i] -= v * x[k];
    });
    ++i;
  });

#if 0
  {
    edge_list<directedness::directed, double>            A = {{0, 0, 1}, {1, 1, 1}, {2, 2, 1}};
    compressed_sparse<0, directed, double> B(A);

    matvec(B, x, y);

    for (auto&& j : y) {
      std::cout << j << std::endl;
    }
  }

#endif

  return 0;
}
