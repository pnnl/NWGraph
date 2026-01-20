/**
 * @file parallel_reduce_scaling.cpp
 *
 * @brief Scalability benchmark for parallel_reduce operations.
 *
 * Tests the scalability of NWGraph's parallel_reduce abstraction across
 * different thread counts. Can be compiled with either TBB or HPX backend.
 *
 * Benchmarks:
 * - Simple sum reduction
 * - Min/max reduction
 * - Complex reduction (e.g., histogram)
 * - Graph-style reductions (e.g., PageRank-like)
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

#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

#include "scalability_common.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/adaptors/splittable_range_adaptor.hpp"

using namespace nw::graph;
using namespace nw::graph::scalability;

/**
 * @brief Simple sum reduction benchmark.
 */
void bench_sum_reduction(const Args& args) {
  const size_t N = args.problem_size;
  std::vector<double> data(N);
  std::iota(data.begin(), data.end(), 1.0);

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_reduce (sum)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    volatile double result = 0;  // volatile to prevent optimization

    return bench("sum", args.ntrials, nthreads, [] {}, [&] {
      result = parallel_reduce_each(
          size_t(0), N, 0.0,
          [&](size_t i) { return data[i]; },
          std::plus<double>());
    });
  }, counts);
}

/**
 * @brief Min/max reduction benchmark.
 */
void bench_minmax_reduction(const Args& args) {
  const size_t N = args.problem_size;
  std::vector<double> data(N);

  // Initialize with pseudo-random values
  for (size_t i = 0; i < N; ++i) {
    data[i] = std::sin(static_cast<double>(i) * 0.1) * 1000.0;
  }

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_reduce (min)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    volatile double result = 0;

    return bench("min", args.ntrials, nthreads, [] {}, [&] {
      result = parallel_reduce_each(
          size_t(0), N, std::numeric_limits<double>::max(),
          [&](size_t i) { return data[i]; },
          [](double a, double b) { return std::min(a, b); });
    });
  }, counts);

  scaling_study("parallel_reduce (max)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    volatile double result = 0;

    return bench("max", args.ntrials, nthreads, [] {}, [&] {
      result = parallel_reduce_each(
          size_t(0), N, std::numeric_limits<double>::lowest(),
          [&](size_t i) { return data[i]; },
          [](double a, double b) { return std::max(a, b); });
    });
  }, counts);
}

/**
 * @brief Compute-intensive reduction (dot product style).
 */
void bench_dot_product(const Args& args) {
  const size_t N = args.problem_size;
  std::vector<double> a(N), b(N);

  for (size_t i = 0; i < N; ++i) {
    a[i] = std::sin(static_cast<double>(i) * 0.01);
    b[i] = std::cos(static_cast<double>(i) * 0.01);
  }

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_reduce (dot product)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    volatile double result = 0;

    return bench("dot_product", args.ntrials, nthreads, [] {}, [&] {
      result = parallel_reduce_each(
          size_t(0), N, 0.0,
          [&](size_t i) { return a[i] * b[i]; },
          std::plus<double>());
    });
  }, counts);
}

/**
 * @brief L2 norm computation (sqrt of sum of squares).
 */
void bench_l2_norm(const Args& args) {
  const size_t N = args.problem_size;
  std::vector<double> data(N);

  for (size_t i = 0; i < N; ++i) {
    data[i] = std::sin(static_cast<double>(i) * 0.001);
  }

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_reduce (L2 norm)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    volatile double result = 0;

    return bench("l2_norm", args.ntrials, nthreads, [] {}, [&] {
      double sum_sq = parallel_reduce_each(
          size_t(0), N, 0.0,
          [&](size_t i) { return data[i] * data[i]; },
          std::plus<double>());
      result = std::sqrt(sum_sq);
    });
  }, counts);
}

/**
 * @brief Reduction with splittable_range_adaptor.
 *
 * Tests the range-based parallel_reduce interface used by graph algorithms.
 */
void bench_range_reduce(const Args& args) {
  const size_t N = args.problem_size;
  std::vector<double> data(N);
  std::vector<size_t> indices(N);

  std::iota(indices.begin(), indices.end(), 0);
  for (size_t i = 0; i < N; ++i) {
    data[i] = static_cast<double>(i % 1000);
  }

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_reduce (splittable_range)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    volatile double result = 0;

    return bench("range_reduce", args.ntrials, nthreads, [] {}, [&] {
      auto range = splittable_range_adaptor(indices);
      result = parallel_reduce(
          range,
          [&](size_t i) { return data[i]; },
          std::plus<double>(),
          0.0);
    });
  }, counts);
}

/**
 * @brief PageRank-style reduction: convergence check.
 *
 * Simulates computing the L1 norm of the difference between iterations,
 * which is common in iterative graph algorithms.
 */
void bench_convergence_check(const Args& args) {
  const size_t N = args.problem_size;
  std::vector<double> old_rank(N), new_rank(N);

  // Initialize with slightly different values to simulate iteration
  for (size_t i = 0; i < N; ++i) {
    old_rank[i] = 1.0 / N;
    new_rank[i] = (1.0 / N) + (static_cast<double>(i % 100) - 50) * 1e-6;
  }

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_reduce (convergence)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    volatile double result = 0;

    return bench("convergence", args.ntrials, nthreads, [] {}, [&] {
      result = parallel_reduce_each(
          size_t(0), N, 0.0,
          [&](size_t i) { return std::abs(new_rank[i] - old_rank[i]); },
          std::plus<double>());
    });
  }, counts);
}

int main(int argc, char* argv[]) {
  Args args(argc, argv);

  std::cout << "========================================" << std::endl;
  std::cout << "NWGraph parallel_reduce Scalability Benchmark" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Backend: " << backend::name() << std::endl;
  std::cout << "Problem size: " << args.problem_size << std::endl;
  std::cout << "Trials: " << args.ntrials << std::endl;
  std::cout << "Max threads: " << args.max_threads << std::endl;
  std::cout << std::endl;

  // Run all benchmarks
  bench_sum_reduction(args);
  bench_minmax_reduction(args);
  bench_dot_product(args);
  bench_l2_norm(args);
  bench_range_reduce(args);
  bench_convergence_check(args);

  std::cout << "\n========================================" << std::endl;
  std::cout << "Benchmark complete" << std::endl;
  std::cout << "========================================" << std::endl;

  return 0;
}
