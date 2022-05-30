/**
 * @file flattened.hpp
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

#ifndef NW_GRAPH_FLATTENED_HPP
#define NW_GRAPH_FLATTENED_HPP

#if 0

template <bool is_const = false, std::size_t... Attrs>
class my_flat_iterator {
public:
  using difference_type = std::ptrdiff_t;
  using value_type      = nw::graph::select_t<std::tuple<index_t, Attributes...>, 0, 1, (Attrs + 2)...>;
  using reference       = std::conditional<is_const, const nw::graph::select_t<std::tuple<index_t, const Attributes&...>, 0, 1, (Attrs + 2)...>,
                                     nw::graph::select_t<std::tuple<index_t, const Attributes&...>, 0, 1, (Attrs + 2)...>>;
  using pointer         = std::conditional<is_const, const nw::graph::select_t<std::tuple<index_t, const Attributes*...>, 0, 1, (Attrs + 2)...>,
                                   nw::graph::select_t<std::tuple<index_t, const Attributes*...>, 0, 1, (Attrs + 2)...>>;
  using iterator_category = std::random_access_iterator_tag;

private:

  using graph_to_be_indexed_type =
      typename std::conditional<is_const, typename std::vector<index_t>::const_iterator, typename std::vector<index_t>::iterator>::type;
  using graph_indices_type = typename std::conditional<is_const, typename struct_of_arrays<Attributes...>::const_iterator,
                                                       typename struct_of_arrays<Attributes...>::iterator>::type;

  graph_to_be_indexed_type graph_to_be_indexed_;
  graph_indices_type       graph_indices_;

  index_t         graph_source_;
  index_t         graph_indices_size_;
  index_t         u_;    // the current source vertex id
  difference_type j_;    // the current edge

public:
  my_flat_iterator(index_t graph_source, graph_to_be_indexed_type graph_to_be_indexed, graph_indices_type graph_indices,
                   index_t graph_indices_size, index_t i, difference_type j)
      : graph_source_(graph_source), graph_to_be_indexed_(graph_to_be_indexed), graph_indices_(graph_indices),
        graph_indices_size_(graph_indices_size), u_(i), j_(j) {}

  reference operator*() { return {u_, std::get<0>(graph_.to_be_indexed_)[j_], std::get<Attrs + 1>(graph_.to_be_indexed_)[j_]...}; }

  reference operator[](difference_type n) {
    return {graph_.source(j_ + n), std::get<0>(graph_.to_be_indexed_)[j_ + n], std::get<Attrs + 1>(graph_.to_be_indexed_)[j_]...};
  }

  my_flat_iterator& operator++() {
    for (++j_; j_ >= graph_.indices_[u_ + 1] && u_ < graph_.indices_.size() - 1; ++u_)
      ;
    return *this;
  }

  my_flat_iterator& operator--() {
    for (--j_; j_ < graph_.indices_[u_] && u_ > 0; --u_)
      ;
    return *this;
  }

  my_flat_iterator operator++(int) {
    my_flat_iterator i = *this;
    ++(*this);
    return i;
  }

  my_flat_iterator operator--(int) {
    my_flat_iterator i = *this;
    ++(*this);
    return i;
  }

  my_flat_iterator& operator+=(difference_type n) {
    j_ += n;
    u_ = graph_.source(j_);
    return *this;
  }

  my_flat_iterator& operator-=(difference_type n) {
    j_ -= n;
    u_ = graph_.source(j_);
    return *this;
  }

  my_flat_iterator operator+(difference_type n) const { return {graph_, graph_.source(j_ + n), j_ + n}; }

  my_flat_iterator operator-(difference_type n) const { return {graph_, graph_.source(j_ - n), j_ - n}; }

  difference_type operator-(const my_flat_iterator& b) const { return j_ - b.j_; }

  bool operator==(const my_flat_iterator& b) const { return j_ == b.j_; }

  bool operator!=(const my_flat_iterator& b) const { return j_ != b.j_; }

  bool operator<(const my_flat_iterator& b) const { return j_ < b.j_; }

  bool operator>(const my_flat_iterator& b) const { return j_ > b.j_; }

  bool operator<=(const my_flat_iterator& b) const { return j_ <= b.j_; }

  bool operator>=(const my_flat_iterator& b) const { return j_ >= b.j_; }
};

template <std::size_t... Attrs>
using flat_iterator = my_flat_iterator<false, Attrs...>;

template <std::size_t... Attrs>
using const_flat_iterator = my_flat_iterator<true, Attrs...>;

/// Provide a tbb split-able range interface to the edge iterators.
template <std::size_t... Attrs>
class flat_range {
  flat_iterator<Attrs...> begin_;
  flat_iterator<Attrs...> end_;
  std::ptrdiff_t          cutoff_;

public:
  flat_range(flat_iterator<Attrs...> begin, flat_iterator<Attrs...> end, std::ptrdiff_t cutoff) : begin_(begin), end_(end), cutoff_(cutoff) {}

  flat_range(flat_range& rhs, tbb::split) : begin_(rhs.begin_), end_(rhs.begin_ += rhs.size() / 2), cutoff_(rhs.cutoff_) {}

  flat_iterator<Attrs...> begin() { return begin_; }
  flat_iterator<Attrs...> end() { return end_; }

  std::ptrdiff_t size() const { return end_ - begin_; }
  bool           empty() const { return begin_ == end_; }
  bool           is_divisible() const { return size() >= cutoff_; }
};

/// Provide a tbb split-able range interface to the edge iterators.
template <std::size_t... Attrs>
class const_flat_range {
  const_flat_iterator<Attrs...> begin_;
  const_flat_iterator<Attrs...> end_;
  std::ptrdiff_t                cutoff_;

public:
  const_flat_range(const_flat_iterator<Attrs...> begin, const_flat_iterator<Attrs...> end, std::ptrdiff_t cutoff)
      : begin_(begin), end_(end), cutoff_(cutoff) {}

  const_flat_range(const_flat_range& rhs, tbb::split) : begin_(rhs.begin_), end_(rhs.begin_ += rhs.size() / 2), cutoff_(rhs.cutoff_) {}

  const_flat_iterator<Attrs...> begin() { return begin_; }
  const_flat_iterator<Attrs...> end() { return end_; }

  std::ptrdiff_t size() const { return end_ - begin_; }
  bool           empty() const { return begin_ == end_; }
  bool           is_divisible() const { return size() >= cutoff_; }
};

/// Get a tbb split-able edge range with the passed cutoff.
template <std::size_t... Attrs>
flat_range<Attrs...> edges(std::ptrdiff_t cutoff = std::numeric_limits<std::ptrdiff_t>::max()) {
  flat_iterator<Attrs...> begin = {source(), to_be_indexed_.begin(), indices_.begin(), size(), 0, 0};
  flat_iterator<Attrs...> end   = {source(), to_be_indexed_.begin(),       indices_.begin(),
                                 size(),   index_t(indices_.size() - 1), index_t(to_be_indexed_.size())};
  return {begin, end, cutoff};
}

template <std::size_t... Attrs>
const_flat_range<Attrs...> edges(std::ptrdiff_t cutoff = std::numeric_limits<std::ptrdiff_t>::max()) const {
  const_flat_iterator<Attrs...> begin = {source(), to_be_indexed_.begin(), indices_.begin(), size(), 0, 0};
  const_flat_iterator<Attrs...> end   = {source(), to_be_indexed_.begin(),       indices_.begin(),
                                       size(),   index_t(indices_.size() - 1), index_t(to_be_indexed_.size())};
  return {begin, end, cutoff};
}

#endif 

#endif // NW_GRAPH_FLATTENED_HPP

