/**
 * @file build.hpp
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

#ifndef NW_GRAPH_BUILD_HPP
#define NW_GRAPH_BUILD_HPP

#include "nwgraph/util/proxysort.hpp"

#include "nwgraph/graph_base.hpp"
#include "nwgraph/graph_traits.hpp"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <execution>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


#include "graph_concepts.hpp"

#include "nwgraph/containers/zip.hpp"


namespace nw {
namespace graph {

using default_execution_policy = std::execution::parallel_unsequenced_policy;

template <int idx, edge_list_graph edge_list_t, class ExecutionPolicy = default_execution_policy>
void sort_by(edge_list_t& el, ExecutionPolicy&& policy = {}) {
  std::sort(std::execution::seq, el.begin(), el.end(),
            [](const auto& a, const auto& b) -> bool { return (std::get<idx>(a) < std::get<idx>(b)); });
}

template <int idx, edge_list_graph edge_list_t, class ExecutionPolicy = default_execution_policy>
void stable_sort_by(edge_list_t& el, ExecutionPolicy&& policy = {}) {
  std::stable_sort(policy, el.begin(), el.end(), [](const auto& a, const auto& b) -> bool { return (std::get<idx>(a) < std::get<idx>(b)); });
}

template <int idx, edge_list_graph edge_list_t, class ExecutionPolicy = default_execution_policy>
void lexical_sort_by(edge_list_t& el, ExecutionPolicy&& policy = {}) {
  static_assert(std::is_same_v<decltype(el.begin()), typename edge_list_t::iterator>);

  const int jdx = (idx + 1) % 2;

  if constexpr (idx == 0) {
    std::sort(policy, el.begin(), el.end());
  } else {
    std::sort(policy, el.begin(), el.end(), [](const auto& a, const auto& b) -> bool {
      return std::tie(std::get<1>(a), std::get<0>(a)) < std::tie(std::get<1>(b), std::get<0>(b));
    });
  }
}

template <int idx, edge_list_graph edge_list_t, class ExecutionPolicy = default_execution_policy>
void lexical_stable_sort_by(edge_list_t& el, ExecutionPolicy&& policy = {}) {

  const int jdx = (idx + 1) % 2;

  std::stable_sort(policy, el.begin(), el.end(), [](const auto& a, const auto& b) -> bool {
    return std::tie(std::get<idx>(a), std::get<jdx>(a)) < std::tie(std::get<idx>(b), std::get<jdx>(b));
  });
}


template <adjacency_list_graph adjacency_t, typename... Ts>
auto push_back_fill_helper(adjacency_t& cs, std::tuple<Ts...> const& theTuple) {

  std::apply([&](Ts const&... args) { cs.push_back(args...); }, theTuple);
}

template <edge_list_c edge_list_t, adjacency_list_graph adjacency_t>
void push_back_fill(edge_list_t& el, adjacency_t& cs) {
  cs.open_for_push_back();

  std::for_each(el.begin(), el.end(), [&](auto&& elt) { push_back_fill_helper(cs, elt); });

  cs.close_for_push_back();
}

/**
 * Fill a plain or non-plain graph from edge list
 */
template <edge_list_graph EdgeList, adjacency_list_graph Adjacency>
void push_back_fill(const EdgeList& edge_list, Adjacency& adj, bool directed, size_t idx) {
  const size_t jdx = (idx + 1) % 2;

  for (auto&& e : edge_list) {
    if (0 == idx) {
      std::apply([&](auto... properties) { adj[std::get<0>(e)].emplace_back(std::get<1>(e), properties...); }, props(e));
      if (!directed) {
        std::apply([&](auto... properties) { adj[std::get<1>(e)].emplace_back(std::get<0>(e), properties...); }, props(e));
      }
    } else {
      std::apply([&](auto... properties) { adj[std::get<1>(e)].emplace_back(std::get<0>(e), properties...); }, props(e));
      if (!directed) {
        std::apply([&](auto... properties) { adj[std::get<0>(e)].emplace_back(std::get<1>(e), properties...); }, props(e));
      }
    }
  }
}

template <edge_list_graph edge_list_t, adjacency_list_graph adj_list_t>
auto fill_adj_list(edge_list_t& el, adj_list_t& al) {
  size_t num_edges = 0;

  al.open_for_push_back();

  std::for_each(el.begin(), el.end(), [&](auto elt) {
    push_back_fill_helper(al, elt);
    ++num_edges;
    if constexpr (edge_list_t::edge_directedness == directedness::undirected) {
      std::swap(std::get<0>(elt), std::get<1>(elt));
      push_back_fill_helper(al, elt);
      ++num_edges;
    }
  });

  al.close_for_push_back();

  return num_edges;
}


template <class Vector1, class Vector2, class Perm>
void permute(const Vector1& vec1, Vector2& vec2, const Perm& perm) {
  tbb::parallel_for(tbb::blocked_range(0ul, perm.size()), [&](auto&& r) {
    for (auto i = r.begin(), e = r.end(); i != e; ++i) {
      vec2[i] = vec1[perm[i]];
    }
  });
}

template <edge_list_graph edge_list_t, adjacency_list_graph adjacency_t, class Perm, size_t... Is>
void permute_helper(edge_list_t& el, adjacency_t& cs, std::index_sequence<Is...> is, const Perm& perm) {
  (..., (permute(std::get<Is + 2>(dynamic_cast<typename edge_list_t::base&>(el)), std::get<Is + 1>(cs.to_be_indexed_), perm)));
}

template <edge_list_graph edge_list_t, adjacency_list_graph adjacency_t, class Perm, size_t... Is>
void permute_helper_all(edge_list_t& el, adjacency_t& cs, std::index_sequence<Is...> is, const Perm& perm) {
  (..., (permute(std::get<Is + 1>(dynamic_cast<typename edge_list_t::base&>(el)), std::get<Is>(cs.to_be_indexed_), perm)));
}


template <edge_list_graph edge_list_t, adjacency_list_graph adjacency_t, class T, class Perm, size_t... Is>
void permute_helper(edge_list_t& el, adjacency_t& cs, std::index_sequence<Is...> is, T& Tmp, Perm& perm) {
  (..., (permute(std::get<Is + 2>(dynamic_cast<typename edge_list_t::base&>(Tmp)), std::get<Is + 1>(cs.to_be_indexed_), perm)));
}


template <edge_list_graph edge_list_t, adjacency_list_graph adjacency_t, class ExecutionPolicy = default_execution_policy, size_t... Is>
void fill_helper(edge_list_t& el, adjacency_t& cs, std::index_sequence<Is...> is, ExecutionPolicy&& policy = {}) {
  (..., (std::copy(policy, std::get<Is + 2>(dynamic_cast<typename edge_list_t::base&>(el)).begin(),
                   std::get<Is + 2>(dynamic_cast<typename edge_list_t::base&>(el)).end(), std::get<Is + 1>(cs.to_be_indexed_).begin())));
}

template <edge_list_graph edge_list_t, adjacency_list_graph adjacency_t, class ExecutionPolicy = default_execution_policy, size_t... Is>
void copy_helper(edge_list_t& el, adjacency_t& cs, std::index_sequence<Is...> is, size_t offset, ExecutionPolicy&& policy = {}) {
  (...,
   (std::copy(policy, std::get<Is + 2>(dynamic_cast<typename edge_list_t::base&>(el)).begin(),
              std::get<Is + 2>(dynamic_cast<typename edge_list_t::base&>(el)).end(), std::get<Is + 1>(cs.to_be_indexed_).begin() + offset)));
}

template <edge_list_graph edge_list_t, adjacency_list_graph adjacency_t, class T, class ExecutionPolicy = default_execution_policy,
          size_t... Is>
void fill_helper_tmp(edge_list_t& el, adjacency_t& cs, std::index_sequence<Is...> is, T& Tmp, ExecutionPolicy&& policy = {}) {
  (..., (std::copy(policy, std::get<Is + 2>(dynamic_cast<typename edge_list_t::base&>(Tmp)).begin(),
                   std::get<Is + 2>(dynamic_cast<typename edge_list_t::base&>(Tmp)).end(), std::get<Is + 1>(cs.to_be_indexed_).begin())));
}

/**
 * @brief This function fills an adjacency list graph structure from edges contained in a directed edge list.
 *
 * If edge (u,v) is the edge list, the edge (u,v) will be inserted into the adjacency list -- that is, 
 * v will be inserted into the neighborhood of u.
 *
 * If the edges in the edge list have properties, those properties will be copied to the adjacency list.
 * 
 * The adjacency list graph can either be a unipartite or bipartite graph.
 *
 * 
 * @tparam idx Which end point to fill in the edge list.
 * @tparam edge_list_t The type of the edge list.
 * @tparam adjacency_t The type of the adjacency list.
 * @tparam Int The type of number of vertices.
 * @tparam ExecutionPolicy The type of the execution policy.
 * @param el The edge list.
 * @param N  Number of vertices at [idx] partition.
 * @param cs The adjacency list.
 * @param policy The parallel execution policy to use in calls to standard library algorithms called by this function.
 */
template <int idx, edge_list_graph edge_list_t, adjacency_list_graph adjacency_t, class Int, class ExecutionPolicy = default_execution_policy>
void fill_directed(edge_list_t& el, Int N, adjacency_t& cs, ExecutionPolicy&& policy = {}) {

  auto degree = degrees<idx>(el);

  cs.indices_.resize(N + 1);
  cs.indices_[0] = 0;
  std::inclusive_scan(policy, degree.begin(), degree.end(), cs.indices_.begin() + 1);
  cs.to_be_indexed_.resize(el.size());

#if 0

  sort_by<idx>(el);  // Need to do this in a way that will let us have const el
                     // If not, we should steal (move) the vectors rather than copy

  // Copy kdx (the other index)
  const int kdx = (idx + 1) % 2;

  std::copy(policy, std::get<kdx>(dynamic_cast<typename edge_list_t::base&>(el)).begin(),
	    std::get<kdx>(dynamic_cast<typename edge_list_t::base&>(el)).end(), std::get<0>(cs.to_be_indexed_).begin());
  
  // Copy properties
  if constexpr (std::tuple_size<typename edge_list_t::attributes_t>::value > 0) {
    fill_helper(el, cs, std::make_integer_sequence<size_t, std::tuple_size<typename edge_list_t::attributes_t>::value>(), policy);
  }

  // auto perm = proxysort(std::get<idx>(el));
  // permute(cs.to_be_indexed_, perm, policy);

#else
  // Best: parallel insertion sort -- need concurrent container safe for push_at


  // Better yet
  const int kdx = (idx + 1) % 2;
  std::copy(policy, std::get<kdx>(dynamic_cast<typename edge_list_t::base&>(el)).begin(),
            std::get<kdx>(dynamic_cast<typename edge_list_t::base&>(el)).end(), std::get<0>(cs.to_be_indexed_).begin());

  // Copy properties
  if constexpr (std::tuple_size<typename edge_list_t::attributes_t>::value > 0) {
    fill_helper(el, cs, std::make_integer_sequence<size_t, std::tuple_size<typename edge_list_t::attributes_t>::value>(), policy);
  }


  // which is faster?
  std::vector<nw::graph::vertex_id_t<adjacency_t>> tmp(std::get<idx>(dynamic_cast<typename edge_list_t::base&>(el)));

  // this dumps core for some reason
  //std::vector<nw::graph::vertex_id_t<adjacency_t>> tmp(std::get<idx>(dynamic_cast<typename edge_list_t::base&>(el)).size());
  //std::copy(policy, std::get<idx>(dynamic_cast<typename edge_list_t::base&>(el)).begin(),
  //	    std::get<kdx>(dynamic_cast<typename edge_list_t::base&>(el)).end(), tmp.begin());

  auto a = make_zipped(tmp, cs.to_be_indexed_);
  std::sort(policy, a.begin(), a.end(), [](auto&& a, auto&& b) { return std::get<0>(a) < std::get<0>(b); });

#endif
}


template <int idx, edge_list_graph edge_list_t, class Int, adjacency_list_graph adjacency_t, class ExecutionPolicy = default_execution_policy>
void fill_undirected(edge_list_t& el, Int N, adjacency_t& cs, ExecutionPolicy&& policy = {}) {
  //if the edge is undirected, it means the edge list must be a unipartite graph
  assert(is_unipartite<typename edge_list_t::unipartite_graph_base>::value);

#if 1

  using vertex_id_type = vertex_id_t<edge_list_t>;

  std::vector<vertex_id_type> Tmp(2 * el.size());
  const int                   kdx = (idx + 1) % 2;

  std::copy(policy, std::get<idx>(el).begin(), std::get<idx>(el).end(), Tmp.begin());
  std::copy(policy, std::get<kdx>(el).begin(), std::get<kdx>(el).end(), Tmp.begin() + el.size());

  {
    std::vector<vertex_id_type> degrees(N);
    cs.indices_.resize(N + 1);
    cs.indices_[0] = 0;
    std::for_each(/* policy, */ Tmp.begin(), Tmp.end(), [&](auto&& i) { ++degrees[i]; });
    std::inclusive_scan(policy, degrees.begin(), degrees.end(), cs.indices_.begin() + 1);
  }

  cs.to_be_indexed_.resize(Tmp.size());

  std::copy(policy, std::get<kdx>(el).begin(), std::get<kdx>(el).end(), std::get<0>(cs.to_be_indexed_).begin());
  std::copy(policy, std::get<idx>(el).begin(), std::get<idx>(el).end(), std::get<0>(cs.to_be_indexed_).begin() + el.size());

  if constexpr (std::tuple_size<typename edge_list_t::attributes_t>::value > 0) {
    copy_helper(el, cs, std::make_integer_sequence<size_t, std::tuple_size<typename edge_list_t::attributes_t>::value>(), 0, policy);
  }
  if constexpr (std::tuple_size<typename edge_list_t::attributes_t>::value > 0) {
    copy_helper(el, cs, std::make_integer_sequence<size_t, std::tuple_size<typename edge_list_t::attributes_t>::value>(), el.size(), policy);
  }

  auto a = make_zipped(Tmp, cs.to_be_indexed_);
  std::sort(policy, a.begin(), a.end(), [](auto&& a, auto&& b) { return std::get<0>(a) < std::get<0>(b); });

#else


  typename edge_list_t::directed_type Tmp(N);    // directedness doesn't matter for the Tmp, so just use same type as el
                                                 // EXCEPT -- degrees does something different if undirected

  Tmp.resize(2 * el.size());

  std::copy(policy, el.begin(), el.end(), Tmp.begin());

  std::transform(policy, el.begin(), el.end(), Tmp.begin() + el.size(), [&](auto&& elt) {
    auto flt = elt;
    std::swap(std::get<0>(flt), std::get<1>(flt));
    return flt;
  });

  sort_by<idx>(Tmp);    // stable_sort may allocate extra memory

  {
    auto degree = degrees<idx>(Tmp);    // Can have a fast version if we know it is sorted -- using equal_range
    cs.indices_.resize(N + 1);
    std::inclusive_scan(policy, degree.begin(), degree.end(), cs.indices_.begin() + 1);
  }

  cs.to_be_indexed_.resize(Tmp.size());

  const int kdx = (idx + 1) % 2;
  std::copy(policy, std::get<kdx>(dynamic_cast<typename edge_list_t::base&>(Tmp)).begin(),
            std::get<kdx>(dynamic_cast<typename edge_list_t::base&>(Tmp)).end(), std::get<0>(cs.to_be_indexed_).begin());

  if constexpr (std::tuple_size<typename edge_list_t::attributes_t>::value > 0) {
    fill_helper_tmp(el, cs, std::make_integer_sequence<size_t, std::tuple_size<typename edge_list_t::attributes_t>::value>(), Tmp, policy);
  }
#endif
}


template <int idx, edge_list_graph edge_list_t, adjacency_list_graph adjacency_t, class ExecutionPolicy = default_execution_policy>
auto fill(edge_list_t& el, adjacency_t& cs, bool sort_adjacency = false, ExecutionPolicy&& policy = {}) {
  if constexpr (edge_list_t::edge_directedness == nw::graph::directedness::directed) {
    fill_directed<idx>(el, num_vertices(el), cs, policy);
  } else {    // undirected -- this cannot be a bipartite graph
    fill_undirected<idx>(el, num_vertices(el), cs, policy);
  }
  if (sort_adjacency) {
    //make adjacency sorted
    cs.sort_to_be_indexed();
  }
  return cs.to_be_indexed_.size();
}

template <int idx, edge_list_graph edge_list_t, adjacency_list_graph adjacency_t, class ExecutionPolicy = default_execution_policy>
auto fill(edge_list_t& el, adjacency_t& cs, directedness dir, bool sort_adjacency = false, ExecutionPolicy&& policy = {}) {
  if (dir == nw::graph::directedness::directed) {
    fill_directed<idx>(el, num_vertices(el), cs, policy);
  } else {    // undirected -- this cannot be a bipartite graph
    fill_undirected<idx>(el, num_vertices(el), cs, policy);
  }
  if (sort_adjacency) {
    //make adjacency sorted
    cs.sort_to_be_indexed();
  }
  return cs.to_be_indexed_.size();
}

template <int idx, edge_list_graph bi_edge_list_t, adjacency_list_graph biadjacency_t, class ExecutionPolicy = default_execution_policy>
auto fill_biadjacency(bi_edge_list_t& el, biadjacency_t& cs, bool sort_adjacency = false, ExecutionPolicy&& policy = {}) {
  if constexpr (bi_edge_list_t::edge_directedness == nw::graph::directedness::directed) {
    fill_directed<idx>(el, num_vertices(el, idx), cs, policy);
  } else {    // undirected -- this cannot be a bipartite graph
    fill_undirected<idx>(el, num_vertices(el, idx), cs, policy);
  }
  if (sort_adjacency) {
    //make adjacency sorted
    cs.sort_to_be_indexed();
  }
  return cs.to_be_indexed_.size();
}

template <int idx, edge_list_graph edge_list_t>
void swap_to_triangular(edge_list_t& el, succession cessor) {
  if (cessor == succession::predecessor) {
    swap_to_triangular<idx, edge_list_t, succession::predecessor>(el);
  } else if (cessor == succession::successor) {
    swap_to_triangular<idx, edge_list_t, succession::successor>(el);
  } else {
    std::cout << "Bad succession: " << std::endl;
  }
}

template <int idx, edge_list_graph edge_list_t>
void swap_to_triangular(edge_list_t& el, const std::string& cessor = "predecessor") {
  if (cessor == "predecessor") {
    swap_to_triangular<idx, edge_list_t, succession::predecessor>(el);
  } else if (cessor == "successor") {
    swap_to_triangular<idx, edge_list_t, succession::successor>(el);
  } else {
    std::cout << "Bad succession: " + cessor << std::endl;
  }
}

template <int idx, edge_list_graph edge_list_t, succession cessor = succession::predecessor, class ExecutionPolicy = default_execution_policy>
void swap_to_triangular(edge_list_t& el, ExecutionPolicy&& policy = {}) {

  if constexpr ((idx == 0 && cessor == succession::predecessor) || (idx == 1 && cessor == succession::successor)) {
    std::for_each(policy, el.begin(), el.end(), [](auto&& f) {
      if (std::get<0>(f) < std::get<1>(f)) {
        std::swap(std::get<0>(f), std::get<1>(f));
      }
    });
  } else if constexpr ((idx == 0 && cessor == succession::successor) || (idx == 1 && cessor == succession::predecessor)) {
    std::for_each(policy, el.begin(), el.end(), [](auto&& f) {
      if (std::get<1>(f) < std::get<0>(f)) {
        std::swap(std::get<1>(f), std::get<0>(f));
      }
    });
  }
}

// Make entries unique -- in place -- remove adjacent redundancies
// Requires entries to be sorted in both dimensions
template <edge_list_graph edge_list_t, class ExecutionPolicy = default_execution_policy>
void uniq(edge_list_t& el, ExecutionPolicy&& policy = {}) {

  auto past_the_end = std::unique(policy, el.begin(), el.end(),
                                  [](auto&& x, auto&& y) { return std::get<0>(x) == std::get<0>(y) && std::get<1>(x) == std::get<1>(y); });

  // el.erase(past_the_end, el.end());
  el.resize(past_the_end - el.begin());
}

template <edge_list_graph edge_list_t>
void remove_self_loops(edge_list_t& el) {
  auto past_the_end =
      std::remove_if(/*std::execution::par_unseq,*/ el.begin(), el.end(), [](auto&& x) { return std::get<0>(x) == std::get<1>(x); });
  // el.erase(past_the_end, el.end());
  el.resize(past_the_end - el.begin());
}


template <degree_enumerable_graph Graph, class ExecutionPolicy = default_execution_policy>
auto degrees(const Graph& graph, ExecutionPolicy&& policy = {}) {
  std::vector<vertex_id_t<Graph>> degree_v(num_vertices(graph));

  tbb::parallel_for(tbb::blocked_range(0ul, degree_v.size()), [&](auto&& r) {
    for (auto i = r.begin(), e = r.end(); i != e; ++i) {
      degree_v[i] = degree(graph[i]);
    }
  });
  return degree_v;
}


template <int d_idx = 0, edge_list_graph edge_list_t, class ExecutionPolicy = default_execution_policy>
requires(is_unipartite<typename edge_list_t::unipartite_graph_base>::value) auto degrees(
    edge_list_t& el, ExecutionPolicy&& policy = {}) requires(!degree_enumerable_graph<edge_list_t>) {

  size_t d_size = 0;
  if constexpr (is_unipartite<typename edge_list_t::unipartite_graph_base>::value) {
    //for unipartite graph
    d_size = num_vertices(el);
  } else {
    //for bipartite graph
    d_size = num_vertices(el, d_idx);
  }
  using vertex_id_type = typename edge_list_t::vertex_id_type;

  std::vector<vertex_id_type> degree(d_size);

  if constexpr (edge_list_t::edge_directedness == directedness::directed) {
    std::vector<std::atomic<vertex_id_type>> tmp(degree.size());

    std::for_each(policy, el.begin(), el.end(), [&](auto&& x) { ++tmp[std::get<d_idx>(x)]; });

    std::copy(policy, tmp.begin(), tmp.end(), degree.begin());

  } else if constexpr (edge_list_t::edge_directedness == directedness::undirected) {
    std::vector<std::atomic<vertex_id_type>> tmp(degree.size());

    std::for_each(policy, el.begin(), el.end(), [&](auto&& x) {
      ++tmp[std::get<0>(x)];
      ++tmp[std::get<1>(x)];
    });
    std::copy(policy, tmp.begin(), tmp.end(), degree.begin());
  }
  return degree;
}

//for bipartite graph
template <int d_idx, edge_list_graph edge_list_t, class ExecutionPolicy = default_execution_policy>
requires(false == is_unipartite<typename edge_list_t::bipartite_graph_base>::value) auto degrees(
    edge_list_t& el, ExecutionPolicy&& policy = {}) requires(!degree_enumerable_graph<edge_list_t>) {

  size_t d_size        = num_vertices(el, d_idx);
  using vertex_id_type = typename edge_list_t::vertex_id_type;

  std::vector<vertex_id_type> degree(d_size);

  if constexpr (edge_list_t::edge_directedness == directedness::directed) {
    std::vector<std::atomic<vertex_id_type>> tmp(degree.size());

    std::for_each(policy, el.begin(), el.end(), [&](auto&& x) { ++tmp[std::get<d_idx>(x)]; });

    std::copy(policy, tmp.begin(), tmp.end(), degree.begin());
  }
  return degree;
}

template <int idx = 0, edge_list_graph edge_list_t>
auto perm_by_degree(edge_list_t& el, std::string direction = "ascending") {
  auto degree = degrees<idx>(el);
  return perm_by_degree<idx>(el, degree, direction);
}

template <int idx = 0, edge_list_graph edge_list_t, class Vector, class ExecutionPolicy = default_execution_policy>
auto perm_by_degree(edge_list_t& el, const Vector& degree, std::string direction = "ascending", ExecutionPolicy&& policy = {}) {

  std::vector<typename edge_list_t::vertex_id_type> perm(degree.size());

  tbb::parallel_for(tbb::blocked_range(0ul, perm.size()), [&](auto&& r) {
    for (auto i = r.begin(), e = r.end(); i != e; ++i) {
      perm[i] = i;
    }
  });

  auto d = degree.begin();

  if (direction == "descending") {
    std::sort(policy, perm.begin(), perm.end(), [&](auto a, auto b) { return d[a] > d[b]; });
  } else if (direction == "ascending") {
    std::sort(policy, perm.begin(), perm.end(), [&](auto a, auto b) { return d[a] < d[b]; });
  } else {
    std::cout << "Unknown direction: " << direction << std::endl;
  }

  return perm;
}

/**
 * @brief This function relabels edge list of unipartite graph. It will relabel both endpoints.
 * 
 * @tparam edge_list_t, edge list type
 * @tparam Vector, permutation array type
 * @tparam ExecutionPolicy, execution polity type
 * @tparam edge_list_t, edge list
 * @tparam Vector, permutation array of the IDs of vertices
 * @tparam ExecutionPolicy, excution policy
 * @return the new IDs of the vertices after permutation
 */
template <edge_list_graph edge_list_t, class Vector, class ExecutionPolicy = default_execution_policy>
requires(true == is_unipartite<typename edge_list_t::unipartite_graph_base>::value) auto relabel(edge_list_t& el, const Vector& perm,
                                                                                                 ExecutionPolicy&& policy = {}) {
  std::vector<typename edge_list_t::vertex_id_type> iperm(perm.size());

  tbb::parallel_for(tbb::blocked_range(0ul, iperm.size()), [&](auto&& r) {
    for (auto i = r.begin(), e = r.end(); i != e; ++i) {
      iperm[perm[i]] = i;
    }
  });

  std::for_each(policy, el.begin(), el.end(), [&](auto&& x) {
    std::get<0>(x) = iperm[std::get<0>(x)];
    std::get<1>(x) = iperm[std::get<1>(x)];
  });
  return iperm;
}

/**
 * @brief This function relabels edge list of bipartite graph. It only relabels one endpoint.
 * 
 * @tparam idx, which end point to relabel
 * @tparam edge_list_t, edge list type
 * @tparam Vector, permutation array type
 * @tparam ExecutionPolicy, execution polity type
 * @param el, edge list
 * @param perm, permutation array of the IDs of vertices
 * @param policy, excution policy
 * @return the new IDs of the vertices after permutation
 */
template <int idx, edge_list_graph edge_list_t, class Vector, class ExecutionPolicy = default_execution_policy>
requires(false == is_unipartite<typename edge_list_t::bipartite_graph_base>::value) auto relabel(edge_list_t& el, const Vector& perm,
                                                                                                 ExecutionPolicy&& policy = {}) {
  std::vector<typename edge_list_t::vertex_id_type> iperm(perm.size());

  tbb::parallel_for(tbb::blocked_range(0ul, iperm.size()), [&](auto&& r) {
    for (auto i = r.begin(), e = r.end(); i != e; ++i) {
      iperm[perm[i]] = i;
    }
  });

  std::for_each(policy, el.begin(), el.end(), [&](auto&& x) { std::get<idx>(x) = iperm[std::get<idx>(x)]; });
  return iperm;
}

/**
 * @brief This relabel function for edge list handles unipartite graph. It will relabel both endpoints.
 * 
 * @tparam edge_list_t, edge list type
 * @tparam Vector, degree array type
 * @param el, edge list
 * @param direction, sort the degrees of vertices in which direction
 * @param degree, the degree array
 */
template <edge_list_graph edge_list_t, class Vector = std::vector<int>>
requires(is_unipartite<typename edge_list_t::unipartite_graph_base>::value) void relabel_by_degree(edge_list_t&  el,
                                                                                                   std::string   direction = "ascending",
                                                                                                   const Vector& degree = std::vector<int>(0)) {

  std::vector<typename edge_list_t::vertex_id_type> perm =
      degree.size() == 0 ? perm_by_degree<0>(el, direction) : perm_by_degree<0>(el, degree, direction);

  relabel(el, perm);
}

/**
 * @brief This function relabels edge list of either unipartite graph or bipartite graph.
 * 
 * @tparam idx, which end point to relabel. Unipartite graph will ignore idx.
 * @tparam edge_list_t, edge list type
 * @tparam Vector, degree array type
 * @param el, edge list
 * @param direction, sort the degrees of vertices in which direction
 * @param degree, the degree array
 * @return the new IDs of the vertices after permutation
 */
template <int idx, edge_list_graph edge_list_t, class Vector = std::vector<int>>
requires(is_unipartite<typename edge_list_t::unipartite_graph_base>::value) auto relabel_by_degree(edge_list_t&  el,
                                                                                                   std::string   direction = "ascending",
                                                                                                   const Vector& degree = std::vector<int>(0)) {

  std::vector<typename edge_list_t::vertex_id_type> perm =
      degree.size() == 0 ? perm_by_degree<idx>(el, direction) : perm_by_degree<idx>(el, degree, direction);
  if constexpr (true == is_unipartite<typename edge_list_t::unipartite_graph_base>::value) {    // Compress idx
    //unipartite graph relabels both endpoints of the edge
    return relabel(el, perm);
    //return perm;
  } else {
    //bipartite graph relabels one endponit of the edge
    return relabel<idx>(el, perm);
  }
}

/**
 *  Make a map from data to the index value of each element in its container
 */
template <std::ranges::random_access_range R>
auto make_index_map(const R& range) {
  using value_type = std::ranges::range_value_t<R>;

  std::map<value_type, size_t> the_map;
  for (size_t i = 0; i < size(range); ++i) {
    the_map[range[i]] = i;
  }
  return the_map;
}

/**
 * Make an edge list without properties from original data, e.g., vector<tuple<size_t, size_t>>
 */
template <class M, std::ranges::random_access_range E, class Edge = decltype(std::tuple_cat(std::tuple<size_t, size_t>())),
          class EdgeList = std::vector<Edge>>
auto make_plain_edges(M& map, const E& edges) {
  EdgeList index_edges;

  for (Edge&& e : edges) {
    std::apply([&](auto&& u, auto&& v, auto... props_) { index_edges.push_back(std::make_tuple(map[u], map[v])); }, e);
  }

  return index_edges;
}

/**
 * Make an edge list with properties copied from original data, e.g., vector<tuple<size_t, size_t, props...>>
 */
template <class M, std::ranges::random_access_range E, class Edge = decltype(std::tuple_cat(std::tuple<size_t, size_t>(), props(E()[0]))),
          class EdgeList = std::vector<Edge>>
auto make_property_edges(M& map, const E& edges) {
  EdgeList index_edges;

  for (auto&& e : edges) {
    std::apply([&](auto&& u, auto&& v, auto... props_) { index_edges.push_back(std::make_tuple(map[u], map[v], props_...)); }, e);
  }

  return index_edges;
}

/**
 * Make an edge list indexing back to the original data, e.g., vector<tuple<size_t, size_t, size_t>>
 */
template <class I = std::vector<std::tuple<size_t, size_t, size_t>>, class M, std::ranges::random_access_range E>
auto make_index_edges(M& map, const E& edges) {

  auto index_edges = I();

  for (size_t i = 0; i < size(edges); ++i) {

    auto left  = std::get<0>(edges[i]);
    auto right = std::get<1>(edges[i]);

    index_edges.push_back(std::make_tuple(map[left], map[right], i));
  }

  return index_edges;
}

/**  
 *  Make a plain graph from data, e.g., vector<vector<index>>
 */
template <std::ranges::random_access_range V, std::ranges::random_access_range E, adjacency_list_graph Graph = std::vector<std::vector<size_t>>>
auto make_plain_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {
  auto vertex_map  = make_index_map(vertices);
  auto index_edges = make_plain_edges(vertex_map, edges);

  Graph G(size(vertices));
  push_back_fill(index_edges, G, directed, idx);

  return G;
}

/**  
 *  Make an index graph from data, e.g., vector<vector<tuple<index, index>>>
 */
template <std::ranges::random_access_range V, std::ranges::random_access_range E,
          adjacency_list_graph Graph = std::vector<std::vector<std::tuple<size_t, size_t>>>>
auto make_index_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {

  auto vertex_map  = make_index_map(vertices);
  auto index_edges = make_index_edges(vertex_map, edges);

  Graph G(size(vertices));

  push_back_fill(index_edges, G, directed, idx);

  return G;
}
/**  
 *  Make a property graph from data, e.g., vector<vector<tuple<index, properties...>>>
 */
template <std::ranges::random_access_range V, std::ranges::forward_range E,
          adjacency_list_graph Graph = std::vector<std::vector<decltype(std::tuple_cat(std::make_tuple(size_t{}), props(*(begin(E{})))))>>>
auto make_property_graph(const V& vertices, const E& edges, bool directed = true, size_t idx = 0) {

  auto vertex_map     = make_index_map(vertices);
  auto property_edges = make_property_edges(vertex_map, edges);

  Graph G(size(vertices));

  push_back_fill(property_edges, G, directed, idx);

  return G;
}

/**  
 *  Functions for building bipartite graphs
 */
template <class I = std::vector<std::tuple<size_t, size_t>>, std::ranges::random_access_range V, std::ranges::random_access_range E>
auto data_to_graph_edge_list(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto left_map  = make_index_map(left_vertices);
  auto right_map = make_index_map(right_vertices);

  std::vector<std::tuple<size_t, size_t>> index_edges;

  for (size_t i = 0; i < size(edges); ++i) {

    auto left  = std::get<0>(edges[i]);
    auto right = std::get<1>(edges[i]);

    index_edges.push_back({left_map[left], right_map[right]});
  }

  return index_edges;
}

template <std::ranges::random_access_range V1, std::ranges::random_access_range V2, std::ranges::random_access_range E,
          adjacency_list_graph Graph = std::vector<std::vector<decltype(std::tuple_cat(std::make_tuple(size_t{}), props(*(begin(E{})))))>>>
auto make_plain_bipartite_graph(const V1& left_vertices, const V2& right_vertices, const E& edges, size_t idx = 0) {

  auto index_edges = data_to_graph_edge_list(left_vertices, right_vertices, edges);
  auto graph_size  = idx == 0 ? size(left_vertices) : size(right_vertices);

  Graph G(size(left_vertices));
  push_back_fill(index_edges, G, true, idx);

  return G;
}

template <std::ranges::random_access_range V1, std::ranges::random_access_range V2, std::ranges::random_access_range E,
          class Graph = std::vector<std::vector<size_t>>>
auto make_plain_bipartite_graphs(const V1& left_vertices, const V2& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list<>(left_vertices, right_vertices, edges);

  Graph G(size(left_vertices));
  Graph H(size(right_vertices));

  push_back_fill(index_edges, G, true, 0);
  push_back_fill(index_edges, H, true, 1);

  return make_tuple(G, H);
}

template <size_t idx = 0, class Graph = std::vector<std::vector<size_t>>, std::ranges::random_access_range V,
          std::ranges::random_access_range E>
auto make_bipartite_graph(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list(left_vertices, right_vertices, edges);
  auto graph_size  = idx == 0 ? size(left_vertices) : size(right_vertices);

  Graph G(size(left_vertices));
  push_back_fill(index_edges, G, true, idx);

  return G;
}

template <std::ranges::random_access_range V, std::ranges::random_access_range E,
          adjacency_list_graph Graph = std::vector<std::vector<decltype(std::tuple_cat(std::make_tuple(size_t{}), props(*(begin(E{})))))>>>
auto make_bipartite_graphs(const V& left_vertices, const V& right_vertices, const E& edges) {

  auto index_edges = data_to_graph_edge_list<>(left_vertices, right_vertices, edges);

  Graph G(size(left_vertices));
  Graph H(size(right_vertices));

  push_back_fill(index_edges, G, true, 0);
  push_back_fill(index_edges, H, true, 1);

  return make_tuple(G, H);
}

template <class Graph1, class Graph2, class IndexGraph = std::vector<std::vector<std::tuple<size_t, size_t>>>>
auto join(const Graph1& G, const Graph2& H) {

  std::vector<std::tuple<size_t, size_t, size_t>> s_overlap;

  for (size_t i = 0; i < H.size(); ++i) {
    for (auto&& k : H[i]) {
      for (auto&& j : G[target(H, k)]) {
        if (target(G, j) != i) {
          s_overlap.push_back({i, target(G, j), target(H, k)});
        }
      }
    }
  }

  IndexGraph L(size(H));
  push_back_fill(s_overlap, L, true, 0);

  return L;
}


}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_BUILD_HPP
