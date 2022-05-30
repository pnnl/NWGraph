/**
 * @file tbb_eg.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *   liux238
 *
 */


#include <iostream>
#include <tbb/parallel_for.h>
#include <tuple>

#if defined(CL_SYCL_LANGUAGE_VERSioN)
#include <dpstd/execution>
#else
#include <execution>
#endif

#include "nwgraph/adaptors/edge_range.hpp"
#include "nwgraph/adaptors/plain_range.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/containers/compressed.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;
using namespace nw::util;

int main() {

  edge_list<directedness::directed> E_list{{3, 4}, {3, 6}, {4, 6}, {7, 8}, {9, 3}};
  adjacency<0>                      A_0(E_list);

  std::for_each(edge_range(A_0).begin(), edge_range(A_0).end(),
                [&](auto&& x) { std::cout << std::get<0>(x) << " " << std::get<1>(x) << std::endl; });

  tbb::parallel_for(edge_range(A_0), [&](auto&& x) {
    //std::cout << std::get<0>(x) << " " << std::get<1>(x) << std::endl;

    //  std::cout << "x is " << x.size() << std::endl;
    std::for_each(x.begin(), x.end(),
                  [&](auto&& x) { std::cout << "( " + std::to_string(std::get<0>(x)) + ", " + std::to_string(std::get<1>(x)) + " )\n"; });
  });

  return 0;
}
