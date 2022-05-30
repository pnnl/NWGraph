/**
 * @file proxysort.hpp
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


#ifndef NW_UTIL_PROXYSORT_HPP
#define NW_UTIL_PROXYSORT_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <numeric>
#include <vector>
#include <execution>

namespace nw {
namespace util {

// template<typename ThingToSort, typename Comparator>
// void proxysort(const ThingToSort& x, std::vector<size_t>& perm) {
//   std::iota(perm.begin(), perm.end(), 0);
//  std::sort(perm.begin(), perm.end(), [&](auto a, auto b) { return x(a) > x(b); });
//}

template <typename ThingToSort, typename Comparator, typename IntT, class ExecutionPolicy = std::execution::parallel_unsequenced_policy>
void proxysort(const ThingToSort& x, std::vector<IntT>& perm, Comparator comp = std::less<IntT>(), ExecutionPolicy policy = {}) {
  assert(perm.size() == x.size());

#if 0
  tbb::parallel_for(tbb::blocked_range(0ul, perm.size()), [&](auto&& r) {
    for (auto i = r.begin(), e = r.end(); i != e; ++i) {
      perm[i] = i;
    }
  });
#else
  std::iota(perm.begin(), perm.end(), 0);  // Parallelize Me!!
#endif

  assert(perm.begin() != perm.end());

  std::sort(/*policy,*/ perm.begin(), perm.end(), [&](auto a, auto b) { 
    assert(perm.size() == x.size());
    assert(a < x.size());
    assert(b < x.size());
    return comp(x[a], x[b]); 
  });
}

template <typename IntT = uint32_t, typename Comparator, typename ThingToSort, class ExecutionPolicy = std::execution::parallel_unsequenced_policy>
auto proxysort(const ThingToSort& x, Comparator comp = std::less<IntT>(), ExecutionPolicy policy = {}) {
  std::vector<IntT> perm(x.size());
  proxysort(x, perm, comp, policy);
  return perm;
}

}    // namespace util
}    // namespace nw

#endif    // NW_UTIL_PROXYSORT_HPP
