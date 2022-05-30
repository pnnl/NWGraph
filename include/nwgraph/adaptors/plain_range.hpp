/**
 * @file plain_range.hpp
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

#ifndef NW_GRAPH_PLAIN_RANGE_HPP
#define NW_GRAPH_PLAIN_RANGE_HPP

#include "nwgraph/graph_traits.hpp"

#include "nwgraph/util/print_types.hpp"
#include "nwgraph/util/util.hpp"
#include <cassert>
#include <queue>
#include <tuple>
#include <vector>

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/execution>
#else
#include <execution>
#endif

namespace nw {
namespace graph {

template <class Graph, std::size_t... Is>
class plain_range {
  static constexpr size_t cutoff_ = 16;

  using vertex_id_type = vertex_id_t<Graph>;

  using graph_iterator = typename std::conditional<std::is_const_v<Graph>, typename Graph::const_iterator, typename Graph::iterator>::type;

  graph_iterator outer_base_;
  graph_iterator outer_begin_;
  graph_iterator outer_end_;

public:
  plain_range(Graph& g, std::size_t offset) : outer_base_(g.begin()), outer_begin_(g.begin() + offset), outer_end_(g.end()) {}

  plain_range(Graph& g) : plain_range(g, 0) {}

  // Split a range.
  plain_range(plain_range& b, tbb::split) : plain_range(b) {
    auto i = (outer_end_ - outer_begin_) / 2;
    outer_begin_ += i;
    b.outer_end_ = b.outer_begin_ + i;
  }

  plain_range(const plain_range&) = default;
  plain_range& operator=(const plain_range&) = default;

  template <bool is_const = false>
  class my_iterator {
  public:
    using iterator_category = std::forward_iterator_tag;
    using value_type        = std::tuple<vertex_id_type>;
    using difference_type   = std::size_t;
    using reference         = value_type&;
    using pointer           = value_type*;

  private:
    graph_iterator base_;     //!<
    graph_iterator first_;    //!<
    graph_iterator last_;     //!<

    my_iterator(const my_iterator& b, unsigned long step) : my_iterator(b) { first_ += step; }

  public:
    my_iterator(graph_iterator base, graph_iterator begin, graph_iterator end) : base_(base), first_(begin), last_(end) {}

    my_iterator(const my_iterator&) = default;
    my_iterator& operator=(const my_iterator& b) = default;

    template <bool was_const, class = std::enable_if_t<is_const && !was_const>>
    my_iterator(const my_iterator<was_const>& rhs) : base_(rhs.base_), first_(rhs.first_), last_(rhs.last_) {}

    template <bool was_const, class = std::enable_if_t<is_const && !was_const>>
    my_iterator& operator=(const my_iterator<was_const>& rhs) {
      base_  = rhs.base_;
      first_ = rhs.first_;
      last_  = rhs.last_;
      return *this;
    }

    my_iterator& operator++() {
      ++first_;
      return *this;
    }

    auto operator*() { return std::tuple(first_ - base_); }

    auto operator*() const { return std::tuple(first_ - base_); }

    bool operator==(const my_iterator& b) const { return first_ == b.first_; }
    bool operator!=(const my_iterator& b) const { return first_ != b.first_; }
    bool operator<(const my_iterator& b) const { return first_ < b.first_; }

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

template <class Graph, std::size_t... Is>
static inline plain_range<Graph, Is...> make_plain_range(Graph& g, std::size_t offset) {
  return {g, offset};
}

template <class Graph, std::size_t... Is>
static inline plain_range<Graph, Is...> make_plain_range(Graph& g) {
  return {g};
}

template <typename Graph>
class plain_degree_range {
public:
  plain_degree_range(Graph& g) : the_graph_(g) {}

  class iterator {
    typename Graph::iterator base_;
    typename Graph::iterator first_;
    typename Graph::iterator last_;

  public:
    iterator(typename Graph::iterator base, typename Graph::iterator first, typename Graph::iterator last)
        : base_(base), first_(first), last_(last) {}

    iterator(const iterator& b) = default;

    iterator& operator++() {
      ++first_;
      return *this;
    }

    auto operator*() { return std::tuple(first_ - base_, (*first_).size()); }

    bool operator==(const iterator& b) const { return first_ == b.first_; }
    bool operator!=(const iterator& b) const { return first_ != b.first_; }
  };

  std::size_t size() const { return the_graph_.size(); }

  iterator begin() { return {the_graph_.begin(), the_graph_.begin(), the_graph_.end()}; }
  iterator end() { return {the_graph_.begin(), the_graph_.end(), the_graph_.end()}; }

private:
  Graph& the_graph_;
};

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_PLAIN_RANGE_HPP
