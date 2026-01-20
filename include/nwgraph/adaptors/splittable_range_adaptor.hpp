/**
 * @file splittable_range_adaptor.hpp
 * @brief Range adaptor that supports splitting for parallel processing.
 *
 * Provides a range wrapper that can be split into sub-ranges for parallel
 * processing. Compatible with both TBB and HPX parallel backends.
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
#include <ranges>

// Only include TBB when using TBB backend
#if !defined(NWGRAPH_BACKEND_HPX)
  #include <oneapi/tbb.h>
#endif

namespace nw {
namespace graph {

/**
 * @brief Tag type for generic range splitting (HPX or other backends).
 */
struct split_tag {};

/**
 * @brief Splittable_range_adaptor can split a range into sub_views of a range.
 *
 * This adaptor wraps a range and provides the interface required by parallel
 * backends (TBB's Range concept, or HPX's iteration patterns) to split work
 * across multiple threads.
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

  /**
   * @brief Construct from a range.
   * @param range The range to adapt.
   */
  template <class Range>
  explicit splittable_range_adaptor(Range&& range) : splittable_range_adaptor(range.begin(), range.end()) {
  }

  /**
   * @brief Construct from a range with custom cutoff.
   * @param range The range to adapt.
   * @param cutoff Minimum size below which the range won't be split.
   */
  template <class Range>
  splittable_range_adaptor(Range&& range, std::size_t cutoff) : splittable_range_adaptor(range.begin(), range.end(), cutoff) {
  }

  /**
   * @brief Construct from iterators.
   * @param begin Start iterator.
   * @param end End iterator.
   */
  splittable_range_adaptor(Iterator begin, Iterator end) : begin_(begin > end ? end : begin), end_(end) {
  }

  /**
   * @brief Construct from iterators with custom cutoff.
   * @param begin Start iterator.
   * @param end End iterator.
   * @param cutoff Minimum size below which the range won't be split.
   */
  splittable_range_adaptor(Iterator begin, Iterator end, std::size_t cutoff) : begin_(begin > end ? end : begin), end_(end), cutoff_(cutoff) {
  }

#if !defined(NWGRAPH_BACKEND_HPX)
  /**
   * @brief TBB split constructor.
   *
   * Creates a new range representing the second half of the original range,
   * and modifies the original to represent only the first half.
   * Required by TBB's Range concept.
   *
   * @param rhs The range to split (will be modified to represent first half).
   */
  splittable_range_adaptor(splittable_range_adaptor& rhs, tbb::split)
      : begin_(rhs.begin_), end_(rhs.begin_ += rhs.size() / 2), cutoff_(rhs.cutoff_) {
  }
#endif

  /**
   * @brief Generic split constructor.
   *
   * Creates a new range representing the second half of the original range,
   * and modifies the original to represent only the first half.
   * Used by backends other than TBB.
   *
   * @param rhs The range to split (will be modified to represent first half).
   */
  splittable_range_adaptor(splittable_range_adaptor& rhs, split_tag)
      : begin_(rhs.begin_), end_(rhs.begin_ += rhs.size() / 2), cutoff_(rhs.cutoff_) {
  }

  // We need the weird ref version to disambiguate the explicit range
  // initializer, which would otherwise get called incorrectly during a tbb
  // split event.
  splittable_range_adaptor(splittable_range_adaptor&)       = default;
  splittable_range_adaptor(const splittable_range_adaptor&) = default;
  splittable_range_adaptor(splittable_range_adaptor&&)      = default;

  splittable_range_adaptor& operator=(const splittable_range_adaptor&) = default;
  splittable_range_adaptor& operator=(splittable_range_adaptor&&)      = default;

  splittable_range_adaptor() = default;

  /**
   * @brief Get the begin iterator.
   */
  decltype(auto) begin() const {
    return begin_;
  }

  /**
   * @brief Get the end iterator.
   */
  decltype(auto) end() const {
    return end_;
  }

  /**
   * @brief Set the cutoff for splitting.
   * @param cutoff New cutoff value.
   * @return Reference to this adaptor for chaining.
   */
  decltype(auto) cutoff(std::size_t cutoff) {
    cutoff_ = cutoff;
    return *this;
  }

  /**
   * @brief Get the current cutoff value.
   * @return The cutoff value.
   */
  std::size_t cutoff() const {
    return cutoff_;
  }

  /**
   * @brief Get the size of the range.
   * @return Number of elements in the range.
   */
  std::size_t size() const {
    return end_ - begin_;
  }

  /**
   * @brief Check if the range is empty.
   * @return true if the range has no elements.
   */
  bool empty() const {
    return size() == 0;
  }

  /**
   * @brief Check if the range can be split.
   *
   * A range is divisible if its size exceeds the cutoff threshold.
   * Required by TBB's Range concept.
   *
   * @return true if the range can be split into sub-ranges.
   */
  bool is_divisible() const {
    return size() > cutoff_;
  }
};

// Deduction guides
template <class Range>
splittable_range_adaptor(Range&& range) -> splittable_range_adaptor<decltype(range.begin())>;

template <class Range>
splittable_range_adaptor(Range&& range, std::size_t) -> splittable_range_adaptor<decltype(range.begin())>;

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_SPLITTABLE_RANGE_ADAPTOR_HPP
