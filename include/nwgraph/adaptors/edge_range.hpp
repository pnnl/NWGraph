/**
 * @file edge_range.hpp
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

#ifndef NW_GRAPH_EDGE_RANGE_HPP
#define NW_GRAPH_EDGE_RANGE_HPP

#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/util/arrow_proxy.hpp"
#include "nwgraph/util/print_types.hpp"
#include "nwgraph/util/util.hpp"
//#include <tbb/tbb_stddef.h>
#include <tuple>

#include <ranges>
#include <utility>

namespace nw {
namespace graph {


template <class Graph, std::size_t... Is>
class edge_range {
  //  static_assert(((Is < Graph::getNAttr()) && ...), "Attribute index out of range");
  static constexpr size_t cutoff_ = 16;

  using vertex_id_type = vertex_id_t<Graph>;

  using graph_iterator       = std::conditional_t<std::is_const_v<Graph>, const_iterator_t<const Graph>, iterator_t<Graph>>;
  using graph_inner_iterator = std::conditional_t<std::is_const_v<Graph>, inner_const_iterator_t<Graph>, inner_iterator_t<Graph>>;

  graph_iterator outer_base_;
  graph_iterator outer_begin_;
  graph_iterator outer_end_;

public:
  edge_range(Graph& g, std::size_t offset, std::index_sequence<Is...> = {})
      : outer_base_(g.begin()), outer_begin_(g.begin() + offset), outer_end_(g.end()) {}

  edge_range(Graph& g, std::index_sequence<Is...> is = {}) : edge_range(g, 0, is) {}

  // Split a range.
  edge_range(edge_range& b, tbb::split) : edge_range(b) {
    auto i = (outer_end_ - outer_begin_) / 2;
    outer_begin_ += i;
    b.outer_end_ = b.outer_begin_ + i;
  }

  // Copy constructors and assignment operators are fine.
  edge_range(const edge_range&) = default;
  edge_range& operator=(const edge_range&) = default;

  template <bool is_const>
  class my_iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = std::tuple<
      vertex_id_type, vertex_id_type,
      typename std::tuple_element_t<Is, std::conditional_t<is_const, const attributes_t<Graph>, attributes_t<Graph>>> ...>;
    using difference_type = std::ptrdiff_t;
    using reference       = std::tuple<
        vertex_id_type, vertex_id_type,
      typename std::tuple_element_t<Is, std::conditional_t<is_const, const attributes_t<Graph>, attributes_t<Graph>>>&...>;
    using pointer = arrow_proxy<reference>;

  private:
    graph_iterator       base_;            //!<
    graph_iterator       first_;           //!<
    graph_iterator       last_;            //!<
    graph_inner_iterator u_begin_ = {};    //!<
    graph_inner_iterator u_end_   = {};    //!<

    my_iterator(const my_iterator& b, unsigned long step) : my_iterator(b) { first_ += step; }

    void check() {
      while (u_begin_ == u_end_ && first_ != last_) {    // make sure we start at a valid dereference point
        if (++first_ != last_) {
          u_begin_ = (*first_).begin();
          u_end_   = (*first_).end();
        } else break;
      }
    }

  public:
    my_iterator(graph_iterator base, graph_iterator begin, graph_iterator end)
        : base_(base)
        , first_(begin)
        , last_(end)
    {
      if (first_ != last_) {
        u_begin_ = (*first_).begin();
        u_end_   = (*first_).end();
        check();
      }
    }

    my_iterator(const my_iterator&) = default;
    my_iterator(const my_iterator<false>& rhs) requires(is_const)
      : base_(rhs.base_)
      , first_(rhs.first_)
      , last_(rhs.last_)
      , u_begin_(rhs.u_begin_)
      , u_end_(rhs.u_end_)
    {
    }

    my_iterator& operator=(const my_iterator&) = default;
    my_iterator& operator=(const my_iterator<false>& rhs) requires(is_const)
    {
      base_    = rhs.base_;
      first_   = rhs.first_;
      last_    = rhs.last_;
      u_begin_ = rhs.u_begin_;
      u_end_   = rhs.u_end_;
      return *this;
    }

    friend bool operator==(const my_iterator& a, const my_iterator& b) {
      return a.first_ == b.first_;
    }

    friend auto operator<=>(const my_iterator& a, const my_iterator& b) {
      return a.first_ <=> b.first_;
    }

    my_iterator& operator++() {

      ++u_begin_;

      check();
      return *this;
    }

    my_iterator operator++(int) const {
      my_iterator it = *this;
      ++(*this);
      return it;
    }

    reference operator*() const { return reference(first_ - base_, std::get<0>(*u_begin_), std::ref(std::get<Is + 1>(*u_begin_))...); }

    pointer operator->() const { return {**this}; }

    difference_type operator-(const my_iterator& b) const { return first_ - b.first_; }
    my_iterator     operator+(difference_type step) const { return my_iterator(*this, step); }
  };

  using iterator       = my_iterator<false>;
  using const_iterator = my_iterator<true>;

  iterator       begin() { return {outer_base_, outer_begin_, outer_end_}; }
  const_iterator begin() const { return {outer_base_, outer_begin_, outer_end_}; }

  iterator       end() { return {outer_base_, outer_end_, outer_end_}; }
  const_iterator end() const { return {outer_base_, outer_end_, outer_end_}; }

  std::size_t size() const { return outer_end_ - outer_begin_; }
  bool        empty() const { return begin() == end(); }
  bool        is_divisible() const { return size() > cutoff_; }
};

template <std::size_t... Is, class Graph>
static inline edge_range<Graph, Is...> make_edge_range(Graph& g, std::size_t offset) {
  return {g, offset};
}

template <std::size_t... Is, class Graph>
static inline edge_range<Graph, Is...> make_edge_range(Graph& g) {
  return {g};
}

template <std::size_t... Is, class Graph>
static inline edge_range<Graph, Is...> make_edge_range(Graph&& g) {
  return {g};
}

}    // namespace graph
}    // namespace nw

#endif    // EDGE_RANGE_HPP
