/**
 * @file zip_eg.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *
 */

#include <algorithm>
#include <tuple>
#include <vector>

#include "nwgraph/util/print_types.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/containers/zip.hpp"

#include "nwgraph/graph_concepts.hpp"

// using namespace nw::graph;


namespace nw {
  namespace graph {

template <std::ranges::forward_range... Ranges>
auto iter_swap(typename nw::graph::zipped<Ranges...>::soa_iterator<false> a, typename nw::graph::zipped<Ranges...>::soa_iterator<false> b) {
  auto tmp = *a;
  *a = *b;
  *b = tmp;
}

  }
}


template <std::ranges::random_access_range... Ranges>
struct zipper : public std::tuple<Ranges&...> {

  using base = std::tuple<Ranges&...>;

  using attributes_t = std::tuple<typename std::iterator_traits<typename Ranges::iterator>::value_type...> ;

  using value_type        =  attributes_t;
  
  zipper(Ranges&... rs) : base(std::forward_as_tuple(rs...)) { }

};





int main() {

  
  static_assert(std::ranges::random_access_range<nw::graph::struct_of_arrays<int, float>>);


  nw::graph::edge_list<nw::graph::directedness::directed> el = { { 8, 3}, { 6, 1 }, { 7, 4} };


  std::vector<int> a(10, 1), b(10, 2);
  std::vector<float> c(10, 3), d(10, 4);

  zipper f(a,b, c, d);


  nw::graph::zipped g (a, b, c, d);

  static_assert(std::ranges::random_access_range<decltype(g)>);

  
  auto e = nw::graph::make_zipped(a, b, c, d);

  static_assert(std::ranges::random_access_range<decltype(e)>);

  auto i = nw::graph::make_zipped(c, a, b);

  std::iota(a.begin(), a.end(), -1);
  std::iota(b.begin(), b.end(), -2);
  std::iota(c.begin(), c.end(), -3);
  std::iota(d.begin(), d.end(), -4);

  std::reverse(c.begin(), c.end());



  std::sort(i.begin(), i.end(), [](auto&&a, auto&&b) { 
    //    print_types(a, b);
    return std::get<0>(a) < std::get<0>(b); } );

  auto j = nw::graph::make_zipped(a, i);

  // print_types(j[0]);
  j[0] = j[1];


  std::sort(j.begin(), j.end(), [](auto&&a, auto&&b) { 
    return std::get<0>(a) < std::get<0>(b); } );


#if 0
  for (auto &&j : e) {
    std::swap(std::get<0>(j), std::get<1>(j));

    std::cout << std::get<0>(j) << " " ;
    std::cout << std::get<1>(j) << " " ;
    std::cout << std::get<2>(j) << " " ;
    std::cout << std::get<3>(j) << " " ;
    std::cout << std::endl;
  }
#endif

  std::reverse(c.begin(), c.end());

  //  nw::graph::zipped h (el, c, d);


  //  print_types(h[0], h[1]);
  //  swap(h[0], h[1]);

  //using std::swap;
  //  std::sort(h.begin(), h.end(), [](auto&&a, auto&&b) { 
    // print_types(a, b);
    //return std::get<1>(a) < std::get<1>(b); } );





  return 0;
}
