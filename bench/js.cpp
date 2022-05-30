/**
 * @file js.cpp
 *
 * @copyright Copyright 2014, Software Engineering Institute, Carnegie Mellon University
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *
 */

static constexpr const char USAGE[] =
    R"(js.exe: NW Graph jaccard similarity benchmark driver.
  Usage:
      tc.exe (-h | --help)
      tc.exe -f FILE... [--version ID...] [-n NUM] [--lower | --upper] [--relabel] [--heuristic] [--log FILE] [--log-header] [--format FORMAT] [-dvV] [THREADS...]

  Options:
      -h, --help            show this screen
      --version ID          algorithm version to run [default: 0]
      -f FILE               input file path
      -n NUM                number of trials [default: 1]
      --lower               lower triangular order [default: false]
      --upper               upper triangular order [default: true]
      --relabel             relabel the graph
      --heuristic           use heuristic to decide whether to relabel or not
      --format FORMAT       specify which graph storage format [default: CSR]
      --log FILE            log times to a file
      --log-header          add a header to the log file
      -d, --debug           run in debug mode
      -v, --verify          verify results
      -V, --verbose         run in verbose mode
)";

#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"
#include "nwgraph/volos.hpp"
#include "nwgraph/vovos.hpp"

#include "common.hpp"
#include "nwgraph/algorithms/jaccard.hpp"
#include "nwgraph/experimental/algorithms/jaccard.hpp"
#include <docopt.h>
#include <tuple>

#include "config.h"
#include <date/date.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace nw::graph::bench;
using namespace nw::graph;
using namespace nw::util;

template <class Vector>
static void tc_relabel(edge_list<nw::graph::directedness::undirected>& A, Vector&& degrees, const std::string& direction) {
  life_timer _(__func__);
  relabel_by_degree<0>(A, direction, degrees);
}

template <std::size_t id = 0>
static void clean(edge_list<nw::graph::directedness::undirected>& A, const std::string& succession) {
  life_timer _(__func__);
  swap_to_triangular<id>(A, succession);
  lexical_sort_by<id>(A);
  uniq(A);
  remove_self_loops(A);
}

template <typename Graph>
auto compress(edge_list<nw::graph::directedness::undirected>& A) {
  life_timer _(__func__);
  Graph      B(num_vertices(A));
  push_back_fill(A, B);
  return B;
}

// heuristic to see if sufficently dense power-law graph
template <class EdgeList, class Vector>
static bool worth_relabeling(const EdgeList& el, const Vector& degree) {
  using vertex_id_type = typename EdgeList::vertex_id_type;

  int64_t average_degree = el.size() / (el.num_vertices()[0]);
  if (average_degree < 10) return false;

  int64_t              num_samples  = std::min<int64_t>(1000L, el.num_vertices()[0]);
  int64_t              sample_total = 0;
  std::vector<int64_t> samples(num_samples);

  std::mt19937                                  rng;
  std::uniform_int_distribution<vertex_id_type> udist(0, el.num_vertices()[0] - 1);

  for (int64_t trial = 0; trial < num_samples; trial++) {
    samples[trial] = degree[udist(rng)];
    sample_total += samples[trial];
  }
  std::sort(std::execution::par_unseq, samples.begin(), samples.end());
  double sample_average = static_cast<double>(sample_total) / num_samples;
  double sample_median  = samples[num_samples / 2];
  return sample_average / 1.3 > sample_median;
}

// Taken from GAP and adapted to NW Graph
template <class Graph>
static std::size_t TCVerifier(Graph& graph) {
  using vertex_id_type = typename Graph::vertex_id_type;

  life_timer                              _(__func__);
  std::size_t                             total = 0;
  std::vector<std::tuple<vertex_id_type>> intersection;
  intersection.reserve(graph.size());
  for (auto&& [u, v] : edge_range(graph)) {
    auto u_out = graph[u];
    auto v_out = graph[v];
    auto end   = std::set_intersection(u_out.begin(), u_out.end(), v_out.begin(), v_out.end(), intersection.begin());
    intersection.resize(end - intersection.begin());
    total += intersection.size();
  }
  return total;    // note that our processed Graph doesn't produce extra counts
                   // like the GAP verifier normally would
}

auto config_log() {
  std::string uuid_;
  char        host_[16];
  std::string date_;
  std::string git_branch_;
  std::string git_version_;
  std::size_t uuid_size_ = 24;

  auto seed = std::random_device();
  auto gen  = std::mt19937(seed());
  auto dis  = std::uniform_int_distribution<char>(97, 122);
  uuid_.resize(uuid_size_);
  std::generate(uuid_.begin(), uuid_.end(), [&] { return dis(gen); });

  if (int e = gethostname(host_, sizeof(host_))) {
    std::cerr << "truncated host name\n";
    strncpy(host_, "ghost", 15);
  }
  {
    std::stringstream    ss;
    date::year_month_day date = date::floor<date::days>(std::chrono::system_clock::now());
    ss << date;
    date_ = ss.str();
  }

  if (!std::system("git rev-parse --abbrev-ref HEAD > git_branch.txt")) {
    std::ifstream("git_branch.txt") >> git_branch_;
  }
  if (!std::system("git log --pretty=format:'%h' -n 1 > git_version.txt")) {
    std::ifstream("git_version.txt") >> git_version_;
    if (std::system("git diff --quiet --exit-code")) {
      git_version_ += "+";
    }
  }

  json config = {{"Host", host_},
                 {"Date", date_},
                 {"git_branch", git_branch_},
                 {"git_version", git_version_},
                 {"Build", BUILD_TYPE},
                 {"CXX_COMPILER", CXX_COMPILER},
                 {"CXX_COMPILER_ID", CXX_COMPILER_ID},
                 {"CXX_VERSION", CXX_VERSION}};

  return config;
}

template <typename Args>
auto args_log(const Args& args) {
  json arg_log;

  for (auto&& arg : args) {
    std::stringstream buf;
    buf << std::get<1>(arg);
    arg_log.push_back({std::get<0>(arg), buf.str()});
  }
  return arg_log;
}

template <typename Graph>
void run_bench(int argc, char* argv[]) {
  std::vector<std::string> strings(argv + 1, argv + argc);
  auto                     args = docopt::docopt(USAGE, strings, true);

  // Read the easy options
  bool verify  = args["--verify"].asBool();
  bool verbose = args["--verbose"].asBool();
  bool debug   = args["--debug"].asBool();
  long trials  = args["-n"].asLong() ?: 1;

  // Read the more complex options
  std::string direction  = "ascending";
  std::string succession = "successor";
  if (args["--lower"].asBool()) {
    direction  = "descending";
    succession = "predecessor";
  }

  std::vector files   = args["-f"].asStringList();
  std::vector ids     = parse_ids(args["--version"].asStringList());
  std::vector threads = parse_n_threads(args["THREADS"].asStringList());

  json   file_log = {};
  size_t file_ctr = 0;

  for (auto&& file : files) {


    std::cout << "processing " << file << "\n";

    auto el_a   = load_graph<nw::graph::directedness::undirected, double>(file);  // fill with default
    auto degree = degrees(el_a);

    // Run and time relabeling. This operates directly on the incoming edglist.
    bool relabeled        = false;
    auto&& [relabel_time] = time_op([&] {
      if (args["--relabel"].asBool()) {
        if (args["--heuristic"].asBool() == false || worth_relabeling(el_a, degree)) {
          tc_relabel(el_a, degree, direction);
          relabeled = true;
        }
      }
    });

    // Force relabel time to 0 if we didn't relabel, this just suppresses
    // nanosecond noise from the time_op function when relabeling wasn't done.
    if (!relabeled) {
      relabel_time = 0.0;
    }

    // Clean up the edgelist to deal with the normal issues related to
    // undirectedness.
    auto&& [clean_time] = time_op([&] { clean<0>(el_a, succession); });

    auto cel_a = compress<Graph>(el_a);

#if 0
    // If we're verifying then compute the number of coefficients once for this
    // graph.
    std::size_t v_coefficients = 0;
    if (verify) {
      v_coefficients = TCVerifier(cel_a);
      std::cout << "verifier reports " << v_coefficients << " coefficients\n";
    }
#endif

    json   thread_log = {};
    size_t thread_ctr = 0;

    for (auto&& thread : threads) {
      auto _ = set_n_threads(thread);

      json   id_log = {};
      size_t id_ctr = 0;

      for (auto&& id : ids) {

        json   run_log = {};
        size_t run_ctr = 0;

        for (int j = 0; j < trials; ++j) {
          if (verbose) {
            std::cout << "running version:" << id << " threads:" << thread << "\n";
          }

          auto&& [time, coefficients] = time_op([&]() -> std::size_t {
            switch (id) {
              case 0:
                return jaccard_similarity_v0(cel_a);
              case 1:
                return jaccard_similarity_v1(cel_a);
              case 2:
                return jaccard_similarity_v2(cel_a);
#if 0
              case 3:
                return jaccard_similarity_v3(cel_a);
              case 4:
                return jaccard_similarity_v4(cel_a.begin(), cel_a.end(), thread);
              case 5:
                return jaccard_similarity_v5(cel_a.begin(), cel_a.end(), thread);
              case 6:
                return jaccard_similarity_v6(cel_a.begin(), cel_a.end(), thread);
              case 7:
                return jaccard_similarity_v7(cel_a);
              case 8:
                return jaccard_similarity_v7(cel_a, std::execution::seq, std::execution::par_unseq);
              case 9:
                return jaccard_similarity_v7(cel_a, std::execution::par_unseq, std::execution::par_unseq);
              case 10:
                return jaccard_similarity_v10(cel_a);
              case 11:
                return jaccard_similarity_v10(cel_a, std::execution::par_unseq, std::execution::par_unseq, std::execution::par_unseq);
              case 12:
                return jaccard_similarity_v12(cel_a, thread);
              case 13:
                return jaccard_similarity_v13(cel_a, thread);
              case 14:
                return jaccard_similarity_v14(cel_a);
#if 0
	    case 15:
	      return jaccard_similarity_edgesplit(cel_a, thread);
	    case 16:
	      return jaccard_similarity_edgesplit_upper(cel_a, thread);
#ifdef ONE_DIMENSIONAL_EDGE
	    case 17:
	      return jaccard_similarity_edgerange(cel_a);
	    case 18:
	      return jaccard_similarity_edgerange_cyclic(cel_a, thread);
#endif
#endif
#endif
              default:
                std::cerr << "Unknown version id " << id << "\n";
                return 0ul;
            }
          });

          if (verify && coefficients != v_coefficients) {
            std::cerr << "Inconsistent results: v" << id << " failed verification for " << file << " using " << thread << " threads (reported "
                      << coefficients << ")\n";
          }

          run_log[run_ctr++] = {{"id", id},
                                {"num_threads", thread},
                                {"trial", j},
                                {"elapsed", time},
                                {"elapsed+relabel", time + relabel_time},
                                {"coefficients", coefficients}};

        }  // for j in trials

        id_log[id_ctr++] = {{"id", id}, {"runs", std::move(run_log)}};
      }  // for id in ids

      thread_log[thread_ctr++] = {{"num_thread", thread}, {"runs", std::move(id_log)}};
    }  // for thread in threads

    file_log[file_ctr++] = {{"File", file},           {"Relabel_time", relabel_time}, {"Clean_time", clean_time},
                            {"Relabeled", relabeled}, {"Num_trials", trials},         {"Runs", std::move(thread_log)}};

  }    // for each file
  if (args["--log"]) {

    json log_log = {{"Config", config_log()}, {"Args", args_log(args)}, {"Files", std::move(file_log)}};

    if (args["--log"].asString() == "-") {
      std::cout << log_log << std::endl;
    } else {
      std::ofstream outfile(args["--log"].asString(), std::ios_base::app);
      outfile << log_log << std::endl;
    }
  }
}


int main(int argc, char* argv[]) {
  std::vector<std::string> strings(argv + 1, argv + argc);
  auto                     args = docopt::docopt(USAGE, strings, true);

  if (args["--format"].asString() == "CSR") {
    run_bench<adjacency<0>>(argc, argv);

  } else if (args["--format"].asString() == "VOV") {
    run_bench<vov<0>>(argc, argv);

  } else if (args["--format"].asString() == "VOL") {
    run_bench<adj_list<0>>(argc, argv);

  } else {
    std::cerr << "bad format" << std::endl;
    return -1;
  }

  return 0;
}
