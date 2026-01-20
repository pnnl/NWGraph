/**
 * @file parallel_for_scaling.cpp
 *
 * @brief Scalability benchmark for parallel_for operations.
 *
 * Tests the scalability of NWGraph's parallel_for abstraction across
 * different thread counts. Can be compiled with either TBB or HPX backend.
 *
 * Benchmarks:
 * - Simple element-wise operation (memory bound)
 * - Compute-intensive operation (CPU bound)
 * - Graph-style iteration patterns
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
#include <numeric>
#include <vector>

#include "scalability_common.hpp"
#include "nwgraph/util/parallel_for.hpp"
#include "nwgraph/adaptors/splittable_range_adaptor.hpp"

using namespace nw::graph;
using namespace nw::graph::scalability;

/**
 * @brief Memory-bound benchmark: vector increment.
 *
 * Tests scaling for simple memory-bound operations.
 */
void bench_memory_bound(const Args& args) {
  const size_t N = args.problem_size;
  std::vector<double> data(N, 1.0);

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_for (memory bound)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    auto reset = [&] {
      std::fill(data.begin(), data.end(), 1.0);
    };

    return bench("vector_increment", args.ntrials, nthreads, reset, [&] {
      parallel_for_each(size_t(0), N, [&](size_t i) {
        data[i] = data[i] * 2.0 + 1.0;
      });
    });
  }, counts);
}

/**
 * @brief Compute-bound benchmark: trigonometric operations.
 *
 * Tests scaling for CPU-intensive computations.
 */
void bench_compute_bound(const Args& args) {
  const size_t N = args.problem_size;
  std::vector<double> input(N);
  std::vector<double> output(N);

  std::iota(input.begin(), input.end(), 0.0);
  for (auto& x : input) {
    x = x / N * 3.14159;
  }

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_for (compute bound)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    auto reset = [&] {
      std::fill(output.begin(), output.end(), 0.0);
    };

    return bench("trig_ops", args.ntrials, nthreads, reset, [&] {
      parallel_for_each(size_t(0), N, [&](size_t i) {
        output[i] = std::sin(input[i]) * std::cos(input[i]) +
                    std::tan(input[i] / 2.0);
      });
    });
  }, counts);
}

/**
 * @brief Test parallel_for with splittable_range_adaptor.
 *
 * This tests the actual range splitting mechanism used by graph algorithms.
 */
void bench_splittable_range(const Args& args) {
  const size_t N = args.problem_size;
  std::vector<double> data(N, 1.0);
  std::vector<size_t> indices(N);
  std::iota(indices.begin(), indices.end(), 0);

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_for (splittable_range)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    auto reset = [&] {
      std::fill(data.begin(), data.end(), 1.0);
    };

    return bench("splittable_range", args.ntrials, nthreads, reset, [&] {
      auto range = splittable_range_adaptor(indices);
      parallel_for(range, [&](size_t i) {
        data[i] = data[i] * 2.0 + std::sqrt(static_cast<double>(i));
      });
    });
  }, counts);
}

/**
 * @brief Mixed workload benchmark.
 *
 * Simulates graph-like access patterns with irregular work per element.
 */
void bench_irregular_work(const Args& args) {
  const size_t N = args.problem_size / 10;  // Fewer elements, more work each
  std::vector<double> data(N);
  std::vector<size_t> work_counts(N);

  // Simulate varying work per vertex (like neighbor counts in a graph)
  for (size_t i = 0; i < N; ++i) {
    work_counts[i] = (i % 100) + 1;  // 1 to 100 iterations per element
  }

  auto counts = thread_counts(args.max_threads);

  scaling_study("parallel_for (irregular work)", [&](size_t nthreads) {
    ThreadLimiter limiter(nthreads);

    auto reset = [&] {
      std::fill(data.begin(), data.end(), 1.0);
    };

    return bench("irregular_work", args.ntrials, nthreads, reset, [&] {
      parallel_for_each(size_t(0), N, [&](size_t i) {
        double sum = 0;
        for (size_t j = 0; j < work_counts[i]; ++j) {
          sum += std::sin(static_cast<double>(i + j)) *
                 std::cos(static_cast<double>(j));
        }
        data[i] = sum;
      });
    });
  }, counts);
}

int main(int argc, char* argv[]) {
  Args args(argc, argv);

  std::cout << "========================================" << std::endl;
  std::cout << "NWGraph parallel_for Scalability Benchmark" << std::endl;
  std::cout << "========================================" << std::endl;
  std::cout << "Backend: " << backend::name() << std::endl;
  std::cout << "Problem size: " << args.problem_size << std::endl;
  std::cout << "Trials: " << args.ntrials << std::endl;
  std::cout << "Max threads: " << args.max_threads << std::endl;
  std::cout << std::endl;

  // Run all benchmarks
  bench_memory_bound(args);
  bench_compute_bound(args);
  bench_splittable_range(args);
  bench_irregular_work(args);

  std::cout << "\n========================================" << std::endl;
  std::cout << "Benchmark complete" << std::endl;
  std::cout << "========================================" << std::endl;

  return 0;
}
