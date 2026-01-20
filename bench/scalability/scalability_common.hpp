/**
 * @file scalability_common.hpp
 *
 * @brief Common utilities for scalability benchmarks.
 *
 * Provides utilities for measuring parallel scalability including:
 * - Thread count control
 * - Timing and statistics
 * - Output formatting for analysis
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

#ifndef NW_GRAPH_SCALABILITY_COMMON_HPP
#define NW_GRAPH_SCALABILITY_COMMON_HPP

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

#include "nwgraph/util/backend.hpp"

#if !defined(NWGRAPH_BACKEND_HPX)
  #include <tbb/global_control.h>
#endif

namespace nw::graph::scalability {

/**
 * @brief Statistics from a benchmark run.
 */
struct BenchStats {
  double min_ms;
  double max_ms;
  double mean_ms;
  double stddev_ms;
  size_t ntrials;
  size_t nthreads;

  void print(std::ostream& os = std::cout) const {
    os << std::fixed << std::setprecision(3);
    os << "threads=" << nthreads
       << " mean=" << mean_ms << "ms"
       << " min=" << min_ms << "ms"
       << " max=" << max_ms << "ms"
       << " stddev=" << stddev_ms << "ms"
       << " trials=" << ntrials
       << std::endl;
  }

  void print_csv(std::ostream& os = std::cout) const {
    os << nthreads << "," << mean_ms << "," << min_ms << ","
       << max_ms << "," << stddev_ms << "," << ntrials << std::endl;
  }
};

/**
 * @brief Compute statistics from a vector of times.
 */
inline BenchStats compute_stats(const std::vector<double>& times, size_t nthreads) {
  BenchStats stats;
  stats.ntrials = times.size();
  stats.nthreads = nthreads;

  if (times.empty()) {
    stats.min_ms = stats.max_ms = stats.mean_ms = stats.stddev_ms = 0;
    return stats;
  }

  stats.min_ms = *std::min_element(times.begin(), times.end());
  stats.max_ms = *std::max_element(times.begin(), times.end());
  stats.mean_ms = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

  double sum_sq = 0;
  for (double t : times) {
    sum_sq += (t - stats.mean_ms) * (t - stats.mean_ms);
  }
  stats.stddev_ms = times.size() > 1 ? std::sqrt(sum_sq / (times.size() - 1)) : 0;

  return stats;
}

/**
 * @brief RAII class to limit thread count for TBB.
 */
class ThreadLimiter {
public:
  explicit ThreadLimiter(size_t nthreads)
#if !defined(NWGRAPH_BACKEND_HPX)
    : control_(tbb::global_control::max_allowed_parallelism, nthreads)
#endif
  {
    // For HPX, thread count is set at startup via command line
    // (e.g., --hpx:threads=N)
    (void)nthreads;
  }

private:
#if !defined(NWGRAPH_BACKEND_HPX)
  tbb::global_control control_;
#endif
};

/**
 * @brief Get available hardware concurrency.
 */
inline size_t hardware_threads() {
  return std::thread::hardware_concurrency();
}

/**
 * @brief Generate thread counts for scaling study.
 *
 * Returns powers of 2 from 1 up to max_threads.
 */
inline std::vector<size_t> thread_counts(size_t max_threads = 0) {
  if (max_threads == 0) {
    max_threads = hardware_threads();
  }

  std::vector<size_t> counts;
  for (size_t t = 1; t <= max_threads; t *= 2) {
    counts.push_back(t);
  }
  // Always include max_threads if not a power of 2
  if (counts.back() != max_threads) {
    counts.push_back(max_threads);
  }
  return counts;
}

/**
 * @brief Run a benchmark with timing.
 *
 * @tparam Setup  Callable for per-trial setup (not timed)
 * @tparam Work   Callable for timed work
 * @param name    Benchmark name for output
 * @param ntrials Number of trials
 * @param nthreads Number of threads
 * @param setup   Setup function called before each trial
 * @param work    Work function to time
 * @return Statistics for the benchmark
 */
template <typename Setup, typename Work>
BenchStats bench(const std::string& name, size_t ntrials, size_t nthreads,
                 Setup&& setup, Work&& work) {
  std::vector<double> times;
  times.reserve(ntrials);

  // Warmup run
  setup();
  work();

  // Timed runs
  for (size_t i = 0; i < ntrials; ++i) {
    setup();

    auto start = std::chrono::high_resolution_clock::now();
    work();
    auto end = std::chrono::high_resolution_clock::now();

    double elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    times.push_back(elapsed);
  }

  auto stats = compute_stats(times, nthreads);
  std::cout << "[" << name << "] ";
  stats.print();

  return stats;
}

/**
 * @brief Run a scaling study across multiple thread counts.
 *
 * @tparam BenchFn Callable that takes nthreads and returns BenchStats
 * @param name     Benchmark name
 * @param fn       Benchmark function
 * @param counts   Thread counts to test
 * @return Vector of statistics for each thread count
 */
template <typename BenchFn>
std::vector<BenchStats> scaling_study(const std::string& name, BenchFn&& fn,
                                       const std::vector<size_t>& counts) {
  std::cout << "\n=== Scaling study: " << name << " ===" << std::endl;
  std::cout << "Backend: " << backend::name() << std::endl;
  std::cout << "Hardware threads: " << hardware_threads() << std::endl;
  std::cout << std::endl;

  std::vector<BenchStats> results;
  results.reserve(counts.size());

  for (size_t nthreads : counts) {
    auto stats = fn(nthreads);
    results.push_back(stats);
  }

  // Print summary table
  std::cout << "\n--- Summary ---" << std::endl;
  std::cout << "Threads,Mean(ms),Min(ms),Max(ms),Stddev(ms),Trials" << std::endl;
  for (const auto& s : results) {
    s.print_csv();
  }

  // Compute speedups
  if (!results.empty()) {
    double baseline = results[0].mean_ms;
    std::cout << "\n--- Speedup (vs 1 thread) ---" << std::endl;
    for (const auto& s : results) {
      double speedup = baseline / s.mean_ms;
      double efficiency = speedup / s.nthreads * 100;
      std::cout << "threads=" << s.nthreads
                << " speedup=" << std::fixed << std::setprecision(2) << speedup << "x"
                << " efficiency=" << std::setprecision(1) << efficiency << "%"
                << std::endl;
    }
  }

  return results;
}

/**
 * @brief Command-line argument parser for scalability benchmarks.
 */
struct Args {
  std::string file;
  size_t      ntrials     = 5;
  size_t      max_threads = 0;  // 0 = auto-detect
  size_t      problem_size = 10000000;  // For synthetic benchmarks
  bool        verbose     = false;
  bool        csv_output  = false;
  std::string output_file;

  Args(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
      std::string arg(argv[i]);

      if (arg == "-f" || arg == "--file") {
        if (++i >= argc) usage(argv[0], "Missing argument for " + arg);
        file = argv[i];
      } else if (arg == "-n" || arg == "--ntrials") {
        if (++i >= argc) usage(argv[0], "Missing argument for " + arg);
        ntrials = std::stoul(argv[i]);
      } else if (arg == "-t" || arg == "--threads") {
        if (++i >= argc) usage(argv[0], "Missing argument for " + arg);
        max_threads = std::stoul(argv[i]);
      } else if (arg == "-s" || arg == "--size") {
        if (++i >= argc) usage(argv[0], "Missing argument for " + arg);
        problem_size = std::stoul(argv[i]);
      } else if (arg == "-v" || arg == "--verbose") {
        verbose = true;
      } else if (arg == "--csv") {
        csv_output = true;
      } else if (arg == "-o" || arg == "--output") {
        if (++i >= argc) usage(argv[0], "Missing argument for " + arg);
        output_file = argv[i];
      } else if (arg == "-h" || arg == "--help") {
        usage(argv[0]);
      } else {
        usage(argv[0], "Unknown option: " + arg);
      }
    }

    if (max_threads == 0) {
      max_threads = hardware_threads();
    }
  }

  static void usage(const std::string& prog, const std::string& msg = "") {
    if (!msg.empty()) {
      std::cerr << "Error: " << msg << "\n";
    }
    std::cerr << "Usage: " << prog << " [OPTIONS]\n";
    std::cerr << "  -f, --file FILE      Input graph file (optional)\n";
    std::cerr << "  -n, --ntrials N      Number of trials [default: 5]\n";
    std::cerr << "  -t, --threads N      Max threads [default: auto-detect]\n";
    std::cerr << "  -s, --size N         Problem size [default: 10000000]\n";
    std::cerr << "  -v, --verbose        Verbose output\n";
    std::cerr << "  --csv                CSV output format\n";
    std::cerr << "  -o, --output FILE    Output file for results\n";
    exit(1);
  }
};

}  // namespace nw::graph::scalability

#endif  // NW_GRAPH_SCALABILITY_COMMON_HPP
