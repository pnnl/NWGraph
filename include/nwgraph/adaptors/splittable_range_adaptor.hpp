/**
 * @file splittable_range_adaptor.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Luke D'Alessandro
 *   Kevin Deweese
 *   Tony Liu
 *
 */


#ifndef NW_GRAPH_SPLITTABLE_RANGE_ADAPTOR_HPP
#define NW_GRAPH_SPLITTABLE_RANGE_ADAPTOR_HPP

#include <cstddef>
#include <oneapi/tbb.h>

#include <ranges>

namespace nw {
namespace graph {
/**
 * @brief Splittable_range_adaptor can split a range into sub_views of a range.
 * 
 * @tparam Iterator iterator type of the range passed in
 */
template <class Iterator>
class splittable_range_adaptor : public std::ranges::view_base {
  Iterator    begin_;
  Iterator    end_;
  std::size_t cutoff_ = 16384;

public:
  using iterator   = Iterator;
  using value_type = typename iterator::value_type;

  template <class Range>
  explicit splittable_range_adaptor(Range&& range) : splittable_range_adaptor(range.begin(), range.end()) {}

  template <class Range>
  splittable_range_adaptor(Range&& range, std::size_t cutoff) : splittable_range_adaptor(range.begin(), range.end(), cutoff) {}

  splittable_range_adaptor(Iterator begin, Iterator end) : begin_(begin > end ? end : begin), end_(end) {}

  splittable_range_adaptor(Iterator begin, Iterator end, std::size_t cutoff) : begin_(begin > end ? end : begin), end_(end), cutoff_(cutoff) {}

  splittable_range_adaptor(splittable_range_adaptor& rhs, tbb::split)
      : begin_(rhs.begin_), end_(rhs.begin_ += rhs.size() / 2), cutoff_(rhs.cutoff_) {}

  // We need the weird ref version to disambiguate the explicit range
  // initializer, which would otherwise get called incorrectly during a tbb
  // split event.
  splittable_range_adaptor(splittable_range_adaptor&)       = default;
  splittable_range_adaptor(const splittable_range_adaptor&) = default;
  splittable_range_adaptor(splittable_range_adaptor&&)      = default;

  splittable_range_adaptor& operator=(const splittable_range_adaptor&) = default;
  splittable_range_adaptor& operator=(splittable_range_adaptor&&)      = default;

  splittable_range_adaptor() = default;

  decltype(auto) begin() const { return begin_; }
  decltype(auto) end() const { return end_; }

  decltype(auto) cutoff(std::size_t cutoff) {
    cutoff_ = cutoff;
    return *this;
  }

  size_t size() const { return end_ - begin_; }
  bool   empty() const { return size() == 0; }
  bool   is_divisible() const { return size() > cutoff_; }
};

template <class Range>
splittable_range_adaptor(Range&& range) -> splittable_range_adaptor<decltype(range.begin())>;

template <class Range>
splittable_range_adaptor(Range&& range, std::size_t) -> splittable_range_adaptor<decltype(range.begin())>;

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_SPLITTABLE_RANGE_ADAPTOR_HPP
