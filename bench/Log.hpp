/**
 * @file Log.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   K. Deweese
 *
 */

#ifndef NW_GRAPH_LOG_HPP
#define NW_GRAPH_LOG_HPP

#include "config.h"

#include <chrono>
#include <cstdlib>
#include <date/date.h>

#if CXX_FILESYSTEM_IS_EXPERIMENTAL
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <unistd.h>

namespace nw::graph {
namespace bench {

struct Log {
  std::string   uuid_;
  char          host_[20];
  std::string   date_;
  std::string   git_branch_;
  std::string   git_version_;
  std::size_t   uuid_size_ = 24;
  std::size_t   cxx_size_;
  std::size_t   cxx_id_size_;
  std::size_t   cxx_ver_size_;
  std::size_t   build_size_;
  std::size_t   tbb_malloc_size_;
  std::ofstream out_;
  std::ostream& os_;

  Log(std::string path) : out_(), os_(path == "-" ? std::cout : out_) {
    auto seed = std::random_device();
    auto gen  = std::mt19937(seed());
    auto dis  = std::uniform_int_distribution<char>(97, 122);
    uuid_.resize(uuid_size_);
    std::generate(uuid_.begin(), uuid_.end(), [&] { return dis(gen); });

    if (int e = gethostname(host_, sizeof(host_))) {
      std::cerr << "truncated host name\n";
      //exit(e);
    }

    if (path != "-") {
      CXX_FILESYSTEM_NAMESPACE::path full = path;
      if (full.has_parent_path()) {
        CXX_FILESYSTEM_NAMESPACE::create_directory(full.parent_path());
      }
      out_.open(path, std::ofstream::out | std::ofstream::app);
    }

    {
      std::stringstream    ss;
      date::year_month_day date = date::floor<date::days>(std::chrono::system_clock::now());
      ss << date;
      date_ = ss.str();
    }

    // Read the git branch
    if (!std::system("git rev-parse --abbrev-ref HEAD > git_branch.txt")) {
      std::ifstream("git_branch.txt") >> git_branch_;
    }

    // Read the git version
    if (!std::system("git log --pretty=format:'%h' -n 1 > git_version.txt")) {
      std::ifstream("git_version.txt") >> git_version_;
      if (std::system("git diff --quiet --exit-code")) {
        git_version_ += "+";
      }
    }

    // Establish column sizes
    cxx_size_        = std::max(sizeof(CXX_COMPILER), sizeof("CXX")) + 1;
    cxx_id_size_     = std::max(sizeof(CXX_COMPILER_ID), sizeof("CXX_ID")) + 1;
    cxx_ver_size_    = std::max(sizeof(CXX_VERSION), sizeof("CXX_VER")) + 1;
    build_size_      = std::max(sizeof(BUILD_TYPE), sizeof("Build")) + 1;
    tbb_malloc_size_ = std::max(sizeof(USE_TBBMALLOC), sizeof("TBBMALLOC")) + 1;
  }

  template <class... Times>
  void log_result_header(Times... times) {
    auto p = os_.precision(15);
    auto f = os_.setf(std::ios::fixed, std::ios::floatfield);    // OA

    os_ << std::setw(uuid_size_ + 2) << std::left << "UUID";
    os_ << std::setw(10) << std::left << "Library";
    os_ << std::setw(10) << std::left << "Branch";
    os_ << std::setw(10) << std::left << "Revision";
    os_ << std::setw(cxx_size_) << std::left << "CXX";
    os_ << std::setw(cxx_id_size_) << std::left << "CXX_ID";
    os_ << std::setw(cxx_ver_size_) << std::left << "CXX_VER";
    os_ << std::setw(build_size_) << std::left << "Build";
    os_ << std::setw(tbb_malloc_size_) << std::left << "TBBMALLOC";
    os_ << std::setw(15) << std::left << "Date";
    os_ << std::setw(sizeof(host_)) << std::left << "Host";
    os_ << std::setw(10) << std::left << "Benchmark";
    os_ << std::setw(10) << std::left << "Version";
    os_ << std::setw(10) << std::left << "Threads";
    os_ << std::setw(32) << std::left << "Graph";
    ((os_ << std::setw(20) << std::left << times), ...);
    os_ << "\n";

    os_.precision(p);
    os_.setf(f, std::ios::floatfield);
  }

  template <class Benchmark, class Version, class Threads, class... Times>
  void operator()(Benchmark benchmark, Version version, Threads threads, CXX_FILESYSTEM_NAMESPACE::path graph, Times&... times) {
    auto p = os_.precision(8);
    auto f = os_.setf(std::ios::scientific, std::ios::floatfield);

    auto stem = graph.stem().string();
    auto n    = stem.find('_');
    if (n < stem.size()) {
      stem.resize(n);
    }

    os_ << std::setw(uuid_size_ + 2) << std::left << uuid_;
    os_ << std::setw(10) << std::left << "NWGRAPH";
    os_ << std::setw(10) << std::left << git_branch_;
    os_ << std::setw(10) << std::left << git_version_;
    os_ << std::setw(cxx_size_) << std::left << CXX_COMPILER;
    os_ << std::setw(cxx_id_size_) << std::left << CXX_COMPILER_ID;
    os_ << std::setw(cxx_ver_size_) << std::left << CXX_VERSION;
    os_ << std::setw(build_size_) << std::left << BUILD_TYPE;
    os_ << std::setw(tbb_malloc_size_) << std::left << USE_TBBMALLOC;
    os_ << std::setw(15) << std::left << date_;
    os_ << std::setw(sizeof(host_)) << std::left << host_;
    os_ << std::setw(10) << std::left << benchmark;
    os_ << std::setw(10) << std::left << version;
    os_ << std::setw(10) << std::left << threads;
    os_ << std::setw(32) << std::left << stem;
    ((os_ << std::setw(20) << std::left << times), ...);
    os_ << "\n";

    os_.precision(p);
    os_.setf(f, std::ios::floatfield);
  }

  template <class Samples, class... Headers>
  void print(std::string algorithm, Samples&& samples, bool header, Headers&&... headers) {
    if (header) {
      log_result_header(std::forward<Headers>(headers)...);
    }

    for (auto&& [config, samples] : samples) {
      auto [file, id, threads] = config;
      for (auto&& sample : samples) {
        std::apply(
            [&, file = file, id = id, threads = threads](auto&&... values) {
              (*this)(algorithm, "v" + std::to_string(id), threads, file, std::forward<decltype(values)>(values)...);
            },
            sample);
      }
    }
  }
};

template <class Samples, class... Headers>
void log(std::string algorithm, std::string path, Samples&& samples, bool header, Headers&&... headers) {
  Log log(path);
  log.print(algorithm, std::forward<Samples>(samples), header, std::forward<Headers>(headers)...);
}
}    // namespace bench
}    // namespace nw::graph

#endif
