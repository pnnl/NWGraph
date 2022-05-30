/**
 * @file degree_eg.cpp
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


#include "nwgraph/adjacency.hpp"
#include "karate.hpp"
#include "nwgraph/util/print_types.hpp"


#if 0

namespace nw::graph {
template <class Iterator>
auto tag_invoke(const degree_tag, const splittable_range_adapter<Iterator>& n) {
  return n.size();
}

template <int idx, std::unsigned_integral index_type, std::unsigned_integral vertex_id_type, typename... Attributes>
auto tag_invoke(const degree_tag, const index_adjacency<idx, index_type, vertex_id_type, Attributes...>& g,
		const typename index_adjacency<idx, index_type, vertex_id_type, Attributes...>::sub_view& v) {
  return v.size();
}
}
#endif

int main() {

  nw::graph::adjacency<0> A(karate_d);

  for (size_t i = 0; i < A.size(); ++i) {
    std::cout << nw::graph::degree(A, i) << std::endl;
  }

  for (auto&& j : A) {
    std::cout << nw::graph::degree(j) << std::endl;    
  }

  for (auto&& j : A) {
    std::cout << nw::graph::degree(A, j) << std::endl;    
  }

}
