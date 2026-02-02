/**
 * @file parallel_backend_test.cpp
 *
 * @brief Unit tests for NWGraph parallel backend functionality.
 *
 * Tests the execution policy compatibility layer and parallel operations
 * that work with both TBB and HPX backends.
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

#include <algorithm>
#include <numeric>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/util/execution_policy.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/util/backend.hpp"

#include <tbb/blocked_range.h>

using namespace nw::graph;

// =============================================================================
// Execution Policy Tests
// =============================================================================

TEST_CASE("Execution policy types are defined", "[parallel][execution_policy]") {
  // These should compile and be valid types regardless of backend
  SECTION("Policy types exist") {
    [[maybe_unused]] execution::sequenced_policy seq_type;
    [[maybe_unused]] execution::parallel_policy par_type;
    [[maybe_unused]] execution::parallel_unsequenced_policy par_unseq_type;
    SUCCEED("All execution policy types are defined");
  }

  SECTION("Policy instances exist") {
    [[maybe_unused]] auto seq = execution::seq;
    [[maybe_unused]] auto par = execution::par;
    [[maybe_unused]] auto par_unseq = execution::par_unseq;
    SUCCEED("All execution policy instances are accessible");
  }

  SECTION("Default execution policy is defined") {
    [[maybe_unused]] default_execution_policy policy;
    SUCCEED("default_execution_policy is defined");
  }
}

// =============================================================================
// Parallel Algorithm Wrapper Tests
// =============================================================================

TEST_CASE("par_sort works correctly", "[parallel][algorithms]") {
  std::vector<int> v = {5, 2, 8, 1, 9, 3, 7, 4, 6};
  std::vector<int> expected = {1, 2, 3, 4, 5, 6, 7, 8, 9};

  SECTION("Sort with default comparison") {
    par_sort(execution::par, v.begin(), v.end());
    REQUIRE(v == expected);
  }

  SECTION("Sort with custom comparison (descending)") {
    std::vector<int> desc = {5, 2, 8, 1, 9};
    par_sort(execution::par, desc.begin(), desc.end(), std::greater<int>());
    REQUIRE(desc == std::vector<int>{9, 8, 5, 2, 1});
  }

  SECTION("Sort empty vector") {
    std::vector<int> empty;
    par_sort(execution::par, empty.begin(), empty.end());
    REQUIRE(empty.empty());
  }

  SECTION("Sort single element") {
    std::vector<int> single = {42};
    par_sort(execution::par, single.begin(), single.end());
    REQUIRE(single == std::vector<int>{42});
  }
}

TEST_CASE("par_copy works correctly", "[parallel][algorithms]") {
  std::vector<int> src = {1, 2, 3, 4, 5};
  std::vector<int> dst(5);

  SECTION("Copy to destination") {
    par_copy(execution::par, src.begin(), src.end(), dst.begin());
    REQUIRE(dst == src);
  }

  SECTION("Copy empty range") {
    std::vector<int> empty_src;
    std::vector<int> empty_dst;
    par_copy(execution::par, empty_src.begin(), empty_src.end(), empty_dst.begin());
    REQUIRE(empty_dst.empty());
  }
}

TEST_CASE("par_for_each works correctly", "[parallel][algorithms]") {
  std::vector<int> v = {1, 2, 3, 4, 5};

  SECTION("Modify elements in place") {
    par_for_each(execution::par, v.begin(), v.end(), [](int& x) { x *= 2; });
    REQUIRE(v == std::vector<int>{2, 4, 6, 8, 10});
  }

  SECTION("Empty range") {
    std::vector<int> empty;
    par_for_each(execution::par, empty.begin(), empty.end(), [](int& x) { x *= 2; });
    REQUIRE(empty.empty());
  }
}

TEST_CASE("par_fill works correctly", "[parallel][algorithms]") {
  std::vector<int> v(10);

  SECTION("Fill with value") {
    par_fill(execution::par, v.begin(), v.end(), 42);
    REQUIRE(std::all_of(v.begin(), v.end(), [](int x) { return x == 42; }));
  }

  SECTION("Fill empty range") {
    std::vector<int> empty;
    par_fill(execution::par, empty.begin(), empty.end(), 42);
    REQUIRE(empty.empty());
  }
}

TEST_CASE("par_inclusive_scan works correctly", "[parallel][algorithms]") {
  std::vector<int> src = {1, 2, 3, 4, 5};
  std::vector<int> dst(5);
  std::vector<int> expected = {1, 3, 6, 10, 15};

  SECTION("Compute prefix sums") {
    par_inclusive_scan(execution::par, src.begin(), src.end(), dst.begin());
    REQUIRE(dst == expected);
  }

  SECTION("Empty range") {
    std::vector<int> empty_src;
    std::vector<int> empty_dst;
    par_inclusive_scan(execution::par, empty_src.begin(), empty_src.end(), empty_dst.begin());
    REQUIRE(empty_dst.empty());
  }
}

TEST_CASE("par_equal works correctly", "[parallel][algorithms]") {
  std::vector<int> a = {1, 2, 3, 4, 5};
  std::vector<int> b = {1, 2, 3, 4, 5};
  std::vector<int> c = {1, 2, 3, 4, 6};

  SECTION("Equal ranges") {
    REQUIRE(par_equal(execution::par, a.begin(), a.end(), b.begin()));
  }

  SECTION("Unequal ranges") {
    REQUIRE_FALSE(par_equal(execution::par, a.begin(), a.end(), c.begin()));
  }
}

// =============================================================================
// Parallel For Tests (NWGraph specific)
// =============================================================================

TEST_CASE("parallel_for_each with index range", "[parallel][parallel_for]") {
  const std::size_t N = 1000;
  std::vector<int> v(N, 0);

  SECTION("Set values based on index") {
    parallel_for_each(0ul, N, [&](std::size_t i) { v[i] = static_cast<int>(i); });

    for (std::size_t i = 0; i < N; ++i) {
      REQUIRE(v[i] == static_cast<int>(i));
    }
  }

  SECTION("Empty range") {
    parallel_for_each(0ul, 0ul, [&](std::size_t i) { v[i] = 42; });
    REQUIRE(std::all_of(v.begin(), v.end(), [](int x) { return x == 0; }));
  }
}

// parallel_reduce uses TBB's blocked_range which is TBB-specific
#if !defined(NWGRAPH_BACKEND_HPX)
TEST_CASE("parallel_reduce works correctly", "[parallel][parallel_reduce]") {
  const std::size_t N = 1000;
  std::vector<int> v(N);
  std::iota(v.begin(), v.end(), 1);  // 1, 2, 3, ..., N

  SECTION("Sum reduction with blocked_range") {
    auto sum = parallel_reduce(
        tbb::blocked_range<std::size_t>(0, N),
        [&](const tbb::blocked_range<std::size_t>& r) {
          int local_sum = 0;
          for (std::size_t i = r.begin(); i < r.end(); ++i) {
            local_sum += v[i];
          }
          return local_sum;
        },
        std::plus<int>{},
        0);

    // Sum of 1 to N = N*(N+1)/2
    int expected = static_cast<int>(N * (N + 1) / 2);
    REQUIRE(sum == expected);
  }

  SECTION("Max reduction with blocked_range") {
    auto max_val = parallel_reduce(
        tbb::blocked_range<std::size_t>(0, N),
        [&](const tbb::blocked_range<std::size_t>& r) {
          int local_max = std::numeric_limits<int>::min();
          for (std::size_t i = r.begin(); i < r.end(); ++i) {
            local_max = std::max(local_max, v[i]);
          }
          return local_max;
        },
        [](int a, int b) { return std::max(a, b); },
        std::numeric_limits<int>::min());

    REQUIRE(max_val == static_cast<int>(N));
  }
}
#endif

// =============================================================================
// Backend Detection Tests
// =============================================================================

TEST_CASE("Backend is correctly detected", "[parallel][backend]") {
  SECTION("Backend detection") {
#if defined(NWGRAPH_BACKEND_HPX)
    REQUIRE(backend::is_hpx());
    INFO("Running with HPX backend");
#else
    REQUIRE_FALSE(backend::is_hpx());
    INFO("Running with TBB backend");
#endif
    SUCCEED("Backend detection works correctly");
  }
}

// =============================================================================
// Concurrent Correctness Tests
// =============================================================================

TEST_CASE("Parallel operations produce correct results", "[parallel][correctness]") {
  const std::size_t N = 10000;

  SECTION("Parallel increment is correct") {
    std::vector<std::atomic<int>> counters(N);
    for (auto& c : counters) c = 0;

    // Each index should be visited exactly once
    parallel_for_each(0ul, N, [&](std::size_t i) {
      counters[i].fetch_add(1, std::memory_order_relaxed);
    });

    for (std::size_t i = 0; i < N; ++i) {
      REQUIRE(counters[i].load() == 1);
    }
  }

// parallel_reduce uses TBB's blocked_range which is TBB-specific
#if !defined(NWGRAPH_BACKEND_HPX)
  SECTION("No race conditions in reduction") {
    std::vector<int> v(N, 1);

    // Sum should be exactly N
    auto sum = parallel_reduce(
        tbb::blocked_range<std::size_t>(0, N),
        [&](const tbb::blocked_range<std::size_t>& r) {
          int local_sum = 0;
          for (std::size_t i = r.begin(); i < r.end(); ++i) {
            local_sum += v[i];
          }
          return local_sum;
        },
        std::plus<int>{},
        0);

    REQUIRE(sum == static_cast<int>(N));
  }
#endif
}
