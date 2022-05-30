/**
 * @file AtomicBitVector.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Eric Silk
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_ATOMIC_BIT_VECTOR_HPP
#define NW_GRAPH_ATOMIC_BIT_VECTOR_HPP

#include "nwgraph/util/atomic.hpp"
#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/algorithm>
#include <dpstd/execution>
#else
#include <algorithm>
#include <execution>
#endif
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>

namespace nw {
namespace graph {
/// Really basic concurrent bit vector for use as a bitmap.
///
/// This bitmap supports a simple get/set API, where set operations are always
/// implemented using acquire/release, while get operations can either be
/// acquire or relaxed. Though simple to provide, resetting, clearing, and
/// resizing are not supported as those operations are not currently needed in
/// BGL17.
///
/// Ideally this data structure or something equivalent would be provided by the
/// standard library in the future.
template <class Word = std::uint64_t>
class AtomicBitVector {
  static constexpr int BITS = sizeof(Word) * 8;

  std::size_t bits_;
  Word*       data_;

public:
  /// Allocate a bitmap.
  ///
  /// The bitmap bits can be allocated in an uninitialized state, which can lead
  /// to simpler code in some algorithms that would like to avoid initialization
  /// on some code paths.
  ///
  /// @param       bits The number of bits to allocate.
  /// @param       init If true, we'll initialize the bitmap data to 0.
  AtomicBitVector(std::size_t bits, bool init = true) : bits_(bits), data_(new Word[words(bits)]) {
    if (init) clear();
  }

  /// Delete the bitmap.
  ~AtomicBitVector() { delete[] data_; }

  /// The move constructor needs to be explicit.
  AtomicBitVector(AtomicBitVector&& rhs) : bits_(std::exchange(rhs.bits_, 0)), data_(std::exchange(rhs.data_, nullptr)) {}

  /// The move assignment operator needs to be explicit.
  AtomicBitVector& operator=(AtomicBitVector&& rhs) {
    bits_ = std::exchange(rhs.bits_, 0);
    data_ = std::exchange(rhs.data_, nullptr);
    return *this;
  }

  /// Clear the bitmap.
  void clear() { std::fill(std::execution::par_unseq, data_, data_ + words(bits_), Word(0)); }

  /// Get a bit from the vector.
  ///
  /// This reads a single bit from the vector, but for performance reasons it
  /// returns the bit un-shifted in a word. This word can be used in a boolean
  /// expression and the compiler will deal with efficiently testing it for 0.
  ///
  /// By default it uses the memory_order_relaxed. This is not safe if there are
  /// ever concurrent set operations to the same underlying word, which can be
  /// hard to predict, and thus should mainly be used in sequential code. The
  /// `atomic_get` operation will propery synchronize with concurrent
  /// `atomic_set` operations.
  ///
  /// @tparam     order The memory model for the load operation.
  ///
  /// @param          i The bit to set.
  ///
  /// @returns          The masked value of the word containing the bit.
  template <std::memory_order order = std::memory_order_relaxed>
  Word get(std::size_t i) const {
    auto&& [word, mask] = split(i);
    return nw::graph::load<order>(data_[word]) & mask;
  }

  /// Set a bit in the vector.
  ///
  /// This sets a single bit in the vector, and returns the previous value of
  /// that bit as a masked word. This returned workd can be used in a boolean
  /// expression and the compiler will deal with efficiently testing it for 0.
  ///
  /// By default it uses the memory_order_relaxed. This is not safe if there
  /// are ever concurrent set or get operations to the same underlying word,
  /// which can be hard to predict, and thus should be used only in sequential
  /// code. The `atomic_set` operation will properly synchronize with concurrent
  /// `atomic_get` and `atomic_set` operations.
  ///
  /// @tparam     order The memory model for the load operation.
  ///
  /// @param          i The bit to set.
  ///
  /// @returns          The masked value of the word containing the bit prior to
  ///                   the set operation.
  template <std::memory_order order = std::memory_order_relaxed>
  Word set(std::size_t i) {
    auto&& [word, mask] = split(i);
    return nw::graph::fetch_or<order>(data_[word], mask) & mask;
  }

  /// Get a bit from the vector.
  ///
  /// This forwards to the get operation with a stronger memory model and is
  /// safe for use in concurrent regions of code.
  ///
  /// @param          i The bit to set.
  ///
  /// @returns          The masked value of the word containing the bit.
  Word atomic_get(std::size_t i) const { return get<std::memory_order_acquire>(i); }

  /// Set a bit in the vector.
  ///
  /// This forward to the set operation with a stronger memory model and is safe
  /// to use in concurrent regions of code.
  ///
  /// @param          i The bit to set.
  ///
  /// @returns          The masked value of the word containing the bit prior to
  ///                   the set operation.
  Word atomic_set(std::size_t i) { return set<std::memory_order_acq_rel>(i); }

  class non_zero_iterator {
    Word*       data_;       //!< Base data array
    std::size_t n_;          //!< Size of the data array
    std::size_t word_;       //!< The current word
    std::size_t bit_ = 0;    //!< The current bit in the word

  public:
    using value_type        = std::size_t;
    using difference_type   = std::ptrdiff_t;
    using reference         = const std::size_t;    // need something better
    using pointer           = void;
    using iterator_category = std::input_iterator_tag;

    non_zero_iterator(Word* data, std::size_t n, std::size_t word) : data_(data), n_(n), word_(word) { search_next(); }

    reference operator*() { return word_ * BITS + bit_; }

    // auto operator->() {
    //   // what should this do?
    // }

    non_zero_iterator& operator++() {
      // if there is possibly another non-zero in the current word then use it
      if (word_ < n_ && ++bit_ < BITS) {
        if (Word w = data_[word_] >> bit_) {
          bit_ += __builtin_ctzl(w);
          return *this;
        }
      }

      // otherwise we're on a slow path because we have to search for the next
      // non-zero word
      ++word_;
      return search_next();
    }

    non_zero_iterator operator++(int) {
      non_zero_iterator i(*this);
      ++(*this);
      return i;
    }

    bool operator!=(const non_zero_iterator& b) const { return (word_ != b.word_ || bit_ != b.bit_ || data_ != b.data_); }

    bool operator==(const non_zero_iterator& b) const { return !operator!=(b); }

  private:
    non_zero_iterator& search_next() {
      // search for the next non-zero word, starting with word_
      for (; word_ < n_; ++word_) {
        if (Word w = data_[word_]) {
          bit_ = __builtin_ctzl(w);
          return *this;
        }
      }

      // saturate for comparisons with end()
      bit_  = 0;
      word_ = n_;
      return *this;
    }
  };

  non_zero_iterator begin() { return {data_, words(bits_), 0}; }
  non_zero_iterator end() { return {data_, words(bits_), words(bits_)}; }

  // Our non_zero_iterators are basically input iterators and thus can't be
  // split using the splittable range adapter, which only handles random-access
  // ranges at the moment. This custom range can be split on word boundaries.
  class non_zero_range {
    Word*       data_;
    std::size_t n_;
    std::size_t begin_;
    std::size_t end_;
    std::size_t cutoff_;

  public:
    non_zero_range(Word* data, std::size_t n, std::size_t begin, std::size_t end, std::size_t cutoff)
        : data_(data), n_(n), begin_(begin), end_(end), cutoff_(cutoff) {}

    non_zero_range(const non_zero_range&) = default;
    non_zero_range(non_zero_range&&)      = default;

    non_zero_range(non_zero_range& a, tbb::split)
        : data_(a.data_), n_(a.n_), begin_(a.begin_), end_(a.begin_ += a.size() / 2), cutoff_(a.cutoff_) {}

    non_zero_iterator begin() { return {data_, n_, begin_}; }
    non_zero_iterator end() { return {data_, n_, end_}; }

    std::size_t size() const { return end_ - begin_; }
    bool        empty() const { return size() == 0; }
    bool        is_divisible() const { return size() > cutoff_; }
  };

  non_zero_range non_zeros(std::size_t cutoff) { return {data_, words(bits_), 0, words(bits_), cutoff}; }

private:
  static constexpr std::tuple<std::size_t, Word> split(std::size_t i) {
    std::size_t word   = i / BITS;
    std::size_t offset = i % BITS;
    Word        mask   = Word(1) << offset;
    return {word, mask};
  }

  static constexpr std::size_t words(std::size_t n) { return n / BITS + ((n % BITS) ? 1 : 0); }
};
}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_ATOMIC_BIT_VECTOR_HPP
