/**
 * @file atomic.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Luke D'Alessandro	
 *
 */

#ifndef NW_GRAPH_ATOMIC
#define NW_GRAPH_ATOMIC

#include "nwgraph/util/traits.hpp"
#include <atomic>
#include <type_traits>

namespace nw {
namespace graph {
/// @returns            The loaded value.
template <std::memory_order order, class T>
constexpr auto load(T&& t) {
  if constexpr (is_atomic_v<std::decay_t<T>>) {
    return std::forward<T>(t).load(order);
  } else {
    return load<order>(std::atomic_ref(t));
  }
}

/// Atomic store operation.
///
/// This wraps the store member functions for atomics, and simulates atomic_ref
/// for non-atomic integral types using gcc's atomic builtins.
///
/// @tparam       order The memory order for the operation [default: release]
/// @tparam           T The type of the variable to store to.
/// @tparam           U The type of the value to store.
///
/// @param            t The variable to store to.
/// @param            u The value to store.
template <std::memory_order order, class T, class U>
constexpr void store(T&& t, U&& u) {
  if constexpr (is_atomic_v<std::decay_t<T>>) {
    std::forward<T>(t).store(std::forward<U>(u), order);
  } else {
    store<order>(std::atomic_ref(t), std::forward<U>(u));
  }
}

/// CAS an atomic.
///
/// This makes it easy to use acquire-release semantics in a compare_exchange
/// operation. It wraps the compare exchange operation for atomics, and
/// simulates atomic_ref for non-atomic integral types using gcc's atomic
/// builtins.
///
/// @tparam      success The memory order for successful operations [default: acquire/release]
/// @tparam      failure The memory order for the read operation on failure [default: acquire]
/// @tparam            T The type of the variable to operate on.
/// @tparam            U The type of the expected value.
/// @tparam            V The type of the value to store.
///
/// @param             t The variable to store to.
/// @param[in/out]     u The expected value (updated on failure).
/// @param             v The new value.
///
/// @returns        true If the compare and exchange succeeded
///                false Otherwise (`u` is updated)
template <std::memory_order success = std::memory_order_acq_rel, std::memory_order failure = std::memory_order_acquire, class T, class U,
          class V>
constexpr bool cas(T&& t, U&& u, V&& v) {
  if constexpr (is_atomic_v<std::decay_t<T>>) {
    return std::forward<T>(t).compare_exchange_strong(std::forward<U>(u), std::forward<V>(v), success, failure);
  } else {
    return cas<success, failure>(std::atomic_ref(t), std::forward<U>(u), std::forward<V>(v));
  }
}

/// Load the value using acquire semantics.
///
/// @tparam           T The type of the variable to load from.
///
/// @param            t The variable to load from.
///
/// @return             The loaded value.
template <class T>
constexpr auto acquire(T&& t) {
  return load<std::memory_order_acquire>(std::forward<T>(t));
}

template <class T>
constexpr auto relaxed(T&& t) {
  return load<std::memory_order_relaxed>(std::forward<T>(t));
}

/// Store the value using release semantics.
///
/// @tparam           T The type of the variable to store to.
/// @tparam           U The type of the value to store.
///
/// @param            t The variable to store to.
/// @param            u The value to store.
template <class T, class U>
constexpr void release(T&& t, U&& u) {
  store<std::memory_order_release>(std::forward<T>(t), std::forward<U>(u));
}

template <class T, class U>
constexpr void relaxed(T&& t, U&& u) {
  store<std::memory_order_relaxed>(std::forward<T>(t), std::forward<U>(u));
}

/// Atomic fetch-and-add.
///
/// This provides a uniform atomic fetch-and-add API interface that works for
/// both atomic integer and floating point integral types, and simulates
/// atomic_ref for non-atomic integral types.
///
/// @tparam       order The memory order for the operation [default: acquire/release]
/// @tparam           T The type of the variable.
/// @tparam           U The type of the value to add.
///
/// @param            t The variable to modify.
/// @param            u The value to add.
///
/// @returns            The value of the variable prior to the add operation.
template <std::memory_order order = std::memory_order_acq_rel, class T, class U>
constexpr auto fetch_add(T&& t, U&& u) {
  if constexpr (is_atomic_v<std::decay_t<T>>) {
    if constexpr (std::is_floating_point_v<remove_atomic_t<std::decay_t<T>>>) {
      auto&& e = acquire(t);
      while (!cas<order>(std::forward<T>(t), e, e + u))
        ;
      return e;
    } else {
      return t.fetch_add(std::forward<U>(u), order);
    }
  } else {
    /// fallback to compiler atomics here... C++20 has atomic_ref.
    if constexpr (std::is_floating_point_v<std::decay_t<T>>) {
      auto e = acquire(std::forward<T>(t));
      for (auto f = e + u; !cas<order>(std::forward<T>(t), e, f); f = e + u)
        ;
      return e;
    } else {
      return fetch_add<order>(std::atomic_ref(t), std::forward<U>(u));
    }
  }
}

template <std::memory_order order = std::memory_order_acq_rel, class T, class U>
constexpr auto fetch_or(T&& t, U&& u) {
  static_assert(!std::is_floating_point_v<std::decay_t<T>>, "Logical fetch_or invalid for floating point types.");
  if constexpr (is_atomic_v<std::decay_t<T>>) {
    return std::forward<T>(t).fetch_or(std::forward<U>(u), order);
  } else {
    return fetch_or<order>(std::atomic_ref(t), std::forward<U>(u));
  }
}
}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_ATOMIC_HPP
