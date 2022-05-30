/**
 * @file mmio.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_MMIO_HPP
#define NW_GRAPH_MMIO_HPP

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <fcntl.h>
#include <future>
#include <iostream>
#include <sstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "MatrixMarketFile.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

#include <tbb/concurrent_vector.h>

namespace nw {
namespace graph {

void mm_fill(std::istream& inputStream, bi_edge_list<directedness::directed>& A, size_t nNonzeros, bool file_symmetry, bool pattern) {
  A.reserve((file_symmetry ? 2 : 1) * nNonzeros);
  A.open_for_push_back();
  for (size_t i = 0; i < nNonzeros; ++i) {
    std::string buffer;
    size_t      d0, d1;

    std::getline(inputStream, buffer);
    std::stringstream(buffer) >> d0 >> d1;

    A.push_back(d0-1, d1-1);

    if (file_symmetry && (d0 != d1)) {
      A.push_back(d1-1, d0-1);
    }
  }
  A.close_for_push_back();
}

template <typename T>
void mm_fill(std::istream& inputStream, bi_edge_list<directedness::directed, T>& A, size_t nNonzeros, bool file_symmetry, bool pattern) {

  A.reserve((file_symmetry ? 2 : 1) * nNonzeros);
  A.open_for_push_back();
  for (size_t i = 0; i < nNonzeros; ++i) {
    std::string buffer;
    size_t      d0, d1;
    T           v(1.0);

    std::getline(inputStream, buffer);
    if (pattern) {
      std::stringstream(buffer) >> d0 >> d1;
    } else {
      std::stringstream(buffer) >> d0 >> d1 >> v;
    }

    A.push_back(d0-1, d1-1, v);

    if (file_symmetry && (d0 != d1)) {
      A.push_back(d1-1, d0-1, v);
    }
  }
  A.close_for_push_back();
}

void mm_fill(std::istream& inputStream, edge_list<directedness::directed>& A, size_t nNonzeros, bool file_symmetry, bool pattern) {
  A.reserve((file_symmetry ? 2 : 1) * nNonzeros);
  A.open_for_push_back();
  for (size_t i = 0; i < nNonzeros; ++i) {
    std::string buffer;
    size_t      d0, d1;

    std::getline(inputStream, buffer);
    std::stringstream(buffer) >> d0 >> d1;

    A.push_back(d0-1, d1-1);

    if (file_symmetry && (d0 != d1)) {
      A.push_back(d1-1, d0-1);
    }
  }
  A.close_for_push_back();
}

template <typename T>
void mm_fill(std::istream& inputStream, edge_list<directedness::directed, T>& A, size_t nNonzeros, bool file_symmetry, bool pattern) {

  A.reserve((file_symmetry ? 2 : 1) * nNonzeros);
  A.open_for_push_back();
  for (size_t i = 0; i < nNonzeros; ++i) {
    std::string buffer;
    size_t      d0, d1;
    T           v(1.0);

    std::getline(inputStream, buffer);
    if (pattern) {
      std::stringstream(buffer) >> d0 >> d1;
    } else {
      std::stringstream(buffer) >> d0 >> d1 >> v;
    }

    A.push_back(d0-1, d1-1, v);

    if (file_symmetry && (d0 != d1)) {
      A.push_back(d1-1, d0-1, v);
    }
  }
  A.close_for_push_back();
}

void mm_fill(std::istream& inputStream, edge_list<directedness::undirected>& A, size_t nNonzeros, bool file_symmetry, bool pattern) {

  A.reserve(nNonzeros);
  A.open_for_push_back();

  for (size_t i = 0; i < nNonzeros; ++i) {
    size_t d0, d1;
    double d2;

    if (pattern) {
      inputStream >> d0 >> d1;
    } else {
      inputStream >> d0 >> d1 >> d2;
    }

    A.push_back(d0-1, d1-1);
  }
  A.close_for_push_back();
}

template <typename T>
void mm_fill(std::istream& inputStream, edge_list<directedness::undirected, T>& A, size_t nNonzeros, bool file_symmetry, bool pattern) {
  // assert(file_symmetry);
  A.reserve(nNonzeros);
  A.open_for_push_back();
  for (size_t i = 0; i < nNonzeros; ++i) {
    std::string buffer;
    size_t      d0, d1;
    T           v(1.0);
    std::getline(inputStream, buffer);
    if (pattern) {
      std::stringstream(buffer) >> d0 >> d1;
    } else {
      std::stringstream(buffer) >> d0 >> d1 >> v;
    }
    A.push_back(d0-1, d1-1, v);
  }
  A.close_for_push_back();
}

template <directedness sym, typename... Attributes>
edge_list<sym, Attributes...> read_mm(std::istream& inputStream) {
  std::string              string_input;
  bool                     file_symmetry = false;
  std::vector<std::string> header(5);

  // %%MatrixMarket matrix coordinate integer symmetric
  std::getline(inputStream, string_input);
  std::stringstream h(string_input);
  for (auto& s : header)
    h >> s;

  if (header[0] != "%%MatrixMarket") {
    std::cerr << "Unsupported format" << std::endl;
    throw;
  }
  if (header[4] == "symmetric") {
    file_symmetry = true;
  } else if (header[4] == "general") {
    file_symmetry = false;
  } else {
    std::cerr << "Bad format (symmetry): " << header[4] << std::endl;
    throw;
  }

  while (std::getline(inputStream, string_input)) {
    if (string_input[0] != '%') break;
  }
  size_t n0, n1, nNonzeros;
  std::stringstream(string_input) >> n0 >> n1 >> nNonzeros;

  // assert(n0 == n1);

  edge_list<sym, Attributes...> A(n0);
  mm_fill(inputStream, A, nNonzeros, file_symmetry, (header[3] == "pattern"));

  return A;
}

template <directedness sym, typename... Attributes>
edge_list<sym, Attributes...> read_mm(const std::string& filename) {
  std::ifstream inputFile(filename);

  edge_list<sym, Attributes...> A = read_mm<sym, Attributes...>(inputFile);

  return A;
}

template <directedness sym, typename... Attributes, edge_list_graph edge_list_t>
edge_list_t read_mm(const std::string& filename) {
  std::ifstream            inputStream(filename);
  std::string              string_input;
  bool                     file_symmetry = false;
  std::vector<std::string> header(5);

  // %%MatrixMarket matrix coordinate integer symmetric
  std::getline(inputStream, string_input);
  std::stringstream h(string_input);
  for (auto& s : header)
    h >> s;

  if (header[0] != "%%MatrixMarket") {
    std::cerr << "Unsupported format" << std::endl;
    throw;
  }
  if (header[4] == "symmetric") {
    file_symmetry = true;
  } else if (header[4] == "general") {
    file_symmetry = false;
  } else {
    std::cerr << "Bad format (symmetry): " << header[4] << std::endl;
    throw;
  }

  while (std::getline(inputStream, string_input)) {
    if (string_input[0] != '%') break;
  }
  size_t n0, n1, nNonzeros;
  std::stringstream(string_input) >> n0 >> n1 >> nNonzeros;

  if (n0 == n1 && is_unipartite<edge_list_t>::value) {
    //unipartite edge list
    //edge_list<sym, Attributes...> A(n0);
    //mm_fill(inputStream, A, nNonzeros, file_symmetry, (header[3] == "pattern"));

    //return A;
    std::cerr << "Can not populate unipartite graph with symmetric matrix" << std::endl;
    throw;
  }
  else if (n0 != n1 && false == is_unipartite<edge_list_t>::value){
    //bipartite edge list
    if (file_symmetry) {
      std::cerr << "Can not populate bipartite graph with symmetric matrix" << std::endl;
      throw;
    }
    bi_edge_list<sym, Attributes...> A(n0, n1);
    mm_fill(inputStream, A, nNonzeros, file_symmetry, (header[3] == "pattern"));

    return A;
  }
  else {
    std::cerr << "Mismatch edge list type with matrix symmetry" << std::endl;
    throw;    
  }
}

template <typename T>
auto read_mm_vector(std::istream& inputStream) {
  std::string string_input;
  bool        file_symmetry = false;
  (void)file_symmetry;    // silence warnings
  std::vector<std::string> header(5);

  // %%MatrixMarket matrix array real general
  // 64 1
  std::getline(inputStream, string_input);
  std::stringstream h(string_input);
  for (auto& s : header)
    h >> s;

  if (header[0] != "%%MatrixMarket") {
    std::cerr << "Unsupported format" << std::endl;
    throw;
  }
  if (header[4] == "symmetric") {
    file_symmetry = true;
  } else if (header[4] == "general") {
    file_symmetry = false;
  } else {
    std::cerr << "Bad format (symmetry): " << header[4] << std::endl;
    throw;
  }

  while (std::getline(inputStream, string_input)) {
    if (string_input[0] != '%') break;
  }
  size_t n0, n1;
  std::stringstream(string_input) >> n0 >> n1;

  assert(1 == n1);

  std::vector<T> vec(n0);

  if (header[3] == "real") {
    double d;
    for (size_t i = 0; i < n0; ++i) {
      inputStream >> d;
      vec[i] = d - 1;
    }
  } else {
    size_t d;
    for (size_t i = 0; i < n0; ++i) {
      inputStream >> d;
      vec[i] = d - 1;
    }
  }

  return vec;
}

template <typename T>
auto read_mm_vector(const std::string& filename) {
  std::ifstream inputFile(filename);

  auto v = read_mm_vector<T>(inputFile);

  return v;
}

bool is_mm(const std::string& filename) {
  std::ifstream            input_stream(filename);
  std::vector<std::string> header(5);

  std::string buf;
  std::getline(input_stream, buf);
  std::stringstream h(buf);
  for (auto& s : header) {
    h >> s;
  }

  return header[0] == "%%MatrixMarket";
}

directedness get_mm_symmetry(const std::string& filename) {
  std::ifstream            inputStream(filename);
  std::string              string_input;
  std::vector<std::string> header(5);

  // %%MatrixMarket matrix coordinate integer symmetric
  std::getline(inputStream, string_input);
  std::stringstream h(string_input);
  for (auto& s : header) {
    h >> s;
  }

  if (header[0] != "%%MatrixMarket") {
    std::cerr << "Unsupported format: " << header[0] << std::endl;
    throw;
  }
  if (header[4] == "symmetric") {
    return directedness::undirected;
  } else {
    return directedness::directed;
  }
}

template <size_t w_idx, directedness sym, typename... Attributes>
void aos_stream(std::ofstream& outputStream, edge_list<sym, Attributes...> A, const std::string& file_symmetry, std::string& w_type) {
  outputStream << "%%MatrixMarket matrix coordinate " << w_type << " " << file_symmetry << "\n%%\n";

  if constexpr (is_unipartite<edge_list<sym, Attributes...>>::value) {
    outputStream << num_vertices(A) << " " << num_vertices(A) << " ";    // + 1 ???
  } else {
    outputStream << num_vertices(A, 0) << " " << num_vertices(A, 1) << " ";    // + 1 ???
  }

  if (file_symmetry == "general" && sym == directedness::undirected)
    outputStream << 2 * (A.end() - A.begin()) << std::endl;
  else
    outputStream << A.end() - A.begin() << std::endl;

  for (auto&& element : A) {
    outputStream << std::get<0>(element) + 1 << " " << std::get<1>(element) + 1;
    if (w_idx != 0) outputStream << " " << std::get<w_idx>(element);
    outputStream << std::endl;
    if (file_symmetry == "general" && sym == directedness::undirected) {
      outputStream << std::get<1>(element) + 1 << " " << std::get<0>(element) + 1;
      if (w_idx != 0) outputStream << " " << std::get<w_idx>(element);
      outputStream << std::endl;
    }
  }
}

template <size_t w_idx = 0, typename idxtype = void, directedness sym, typename... Attributes>
void write_mm(const std::string& filename, edge_list<sym, Attributes...>& A, const std::string& file_symmetry = "general") {
  if (file_symmetry == "symmetric" && sym == directedness::directed) {
    std::cerr << "cannot save directed matrix as symmetric matrix market" << std::endl;
  }

  std::string w_type = "pattern";
  if (std::numeric_limits<idxtype>::is_integer)
    w_type = "integer";
  else if (std::is_floating_point<idxtype>::value)
    w_type = "real";

  std::ofstream outputStream(filename);
  aos_stream<w_idx>(outputStream, A, file_symmetry, w_type);
}

template <size_t w_idx, int idx, typename... Attributes>
void adjacency_stream(std::ofstream& outputStream, adjacency<idx, Attributes...>& A, const std::string& file_symmetry, std::string& w_type) {
  outputStream << "%%MatrixMarket matrix coordinate " << w_type << " " << file_symmetry << "\n%%\n";

  outputStream << num_vertices(A) << " " << num_vertices(A) << " "
               << std::accumulate(A.begin(), A.end(), 0, [&](int a, auto b) { return a + (int)(b.end() - b.begin()); }) << std::endl;

  for (auto first = A.begin(); first != A.end(); ++first) {
    for (auto v = (*first).begin(); v != (*first).end(); ++v) {
      outputStream << first - A.begin() + (1 - idx) << " " << std::get<0>(*v) + (1 - idx);
      if (w_idx != 0) outputStream << " " << std::get<w_idx>(*v);
      outputStream << std::endl;
    }
  }
}

template <size_t w_idx = 0, typename idxtype = void, int idx, typename... Attributes>
void write_mm(const std::string& filename, adjacency<idx, Attributes...>& A, const std::string& file_symmetry = "general") {
  /*if (file_symmetry == "symmetric" && sym == directedness::directed) {
    std::cerr << "cannot save directed matrix as symmetric matrix market" << std::endl;
  }*/

  std::string w_type = "pattern";
  if (std::numeric_limits<idxtype>::is_integer)
    w_type = "integer";
  else if (std::is_floating_point<idxtype>::value)
    w_type = "real";

  std::ofstream outputStream(filename);
  adjacency_stream<w_idx>(outputStream, A, file_symmetry, w_type);
}

template <size_t w_idx, int idx, typename... Attributes>
void adjacency_stream(std::ofstream& outputStream, biadjacency<idx, Attributes...>& A, const std::string& file_symmetry, std::string& w_type) {
  outputStream << "%%MatrixMarket matrix coordinate " << w_type << " " << file_symmetry << "\n%%\n";

  outputStream << num_vertices(A, 0) << " " << num_vertices(A, 1) << " "
               << std::accumulate(A.begin(), A.end(), 0, [&](int a, auto b) { return a + (int)(b.end() - b.begin()); }) << std::endl;

  for (auto first = A.begin(); first != A.end(); ++first) {
    for (auto v = (*first).begin(); v != (*first).end(); ++v) {
      outputStream << first - A.begin() + (1 - idx) << " " << std::get<0>(*v) + (1 - idx);
      if (w_idx != 0) outputStream << " " << std::get<w_idx>(*v);
      outputStream << std::endl;
    }
  }
}

template <size_t w_idx = 0, typename idxtype = void, int idx, typename... Attributes>
void write_mm(const std::string& filename, biadjacency<idx, Attributes...>& A, const std::string& file_symmetry = "general") {
  /*if (file_symmetry == "symmetric" && sym == directedness::directed) {
    std::cerr << "cannot save directed matrix as symmetric matrix market" << std::endl;
  }*/

  std::string w_type = "pattern";
  if (std::numeric_limits<idxtype>::is_integer)
    w_type = "integer";
  else if (std::is_floating_point<idxtype>::value)
    w_type = "real";

  std::ofstream outputStream(filename);
  adjacency_stream<w_idx>(outputStream, A, file_symmetry, w_type);
}
#if 0

static size_t block_min(int thread, size_t M, int threads) {
  size_t         block = M / threads;
  int            rem = M % threads;
  return thread*block + std::min(thread, rem);
}

template <typename T>
std::tuple<size_t, std::array<vertex_id_type, 2>, std::array<vertex_id_type, 2>>
par_load_mm(mmio::MatrixMarketFile& mmio, std::vector<std::vector<std::tuple<size_t, size_t, T>>>& sub_lists, std::vector<std::vector<std::tuple<size_t, size_t, T>>>& sub_loops, bool keep_loops, size_t threads) {
  //mmio::MatrixMarketFile mmio(filename);
  std::array<vertex_id_type, 2> Gi_min = {std::numeric_limits<vertex_id_type>::max(), std::numeric_limits<vertex_id_type>::max()};
  std::array<vertex_id_type, 2> Gi_max = {0, 0};
  std::vector<std::future<std::tuple<size_t, std::array<vertex_id_type, 2>, std::array<vertex_id_type, 2>>>> futures(threads);
  for (std::size_t tid = 0; tid < threads; ++tid) {
    futures[tid] = std::async(std::launch::async, 
      [&](size_t thread) {
        size_t GN = mmio.getNCols();
        size_t GM = mmio.getNEdges();

        // figure out which edges I'm going to read from the file
        size_t min = block_min(thread, GM, threads);
        size_t max = block_min(thread + 1, GM, threads);

        std::array<vertex_id_type, 2> i_min = {std::numeric_limits<vertex_id_type>::max(), std::numeric_limits<vertex_id_type>::max()};
        std::array<vertex_id_type, 2> i_max = {0, 0};

        // slightly oversize to try and avoid resizing
        static constexpr double scale = 1.1;
        sub_lists[thread].reserve(scale*(max-min));
        size_t count = 0;

        // read edge data from the file
        for (auto&& [u, v, w] : mmio::edges<T>(mmio, min, max)) {
          i_min[0] = std::min((vertex_id_type)u, i_min[0]);
          i_min[1] = std::min((vertex_id_type)v, i_min[1]);
          i_max[0] = std::max((vertex_id_type)u, i_max[0]);
          i_max[1] = std::max((vertex_id_type)v, i_max[1]);

          if(u == v && keep_loops) {
            if(mmio.isPattern()) {
              sub_loops[thread].push_back(std::tuple(u-1, v-1, T(1)));
            } else {
              sub_loops[thread].push_back(std::tuple(u-1, v-1, w));
            }
          }
          else {
            if(mmio.isPattern()) {
              sub_lists[thread].push_back(std::tuple(u-1, v-1, T(1)));
            } else {
              sub_lists[thread].push_back(std::tuple(u-1, v-1, w));
            }
            ++count;
          }
        }
        return std::tuple(count, i_min, i_max);
      }, tid);
  }
  std::size_t entries = 0;
  for (auto&& f : futures) {
    auto res = f.get();
    entries += std::get<0>(res);
    Gi_min[0] = std::min(std::get<1>(res)[0], Gi_min[0]);
    Gi_min[1] = std::min(std::get<1>(res)[1], Gi_min[1]);
    Gi_max[0] = std::max(std::get<2>(res)[0], Gi_max[0]);
    Gi_max[1] = std::max(std::get<2>(res)[1], Gi_max[1]);
  }
  
  return std::tuple(entries, Gi_min, Gi_max);
}

std::tuple<size_t, std::array<vertex_id_type, 2>, std::array<vertex_id_type, 2>>
par_load_mm(mmio::MatrixMarketFile& mmio, std::vector<std::vector<std::tuple<size_t, size_t>>>& sub_lists, std::vector<std::vector<std::tuple<size_t, size_t>>>& sub_loops, bool keep_loops, size_t threads) {
  std::array<vertex_id_type, 2> Gi_min = {std::numeric_limits<vertex_id_type>::max(), std::numeric_limits<vertex_id_type>::max()};
  std::array<vertex_id_type, 2> Gi_max = {0, 0};
  std::vector<std::future<std::tuple<size_t, std::array<vertex_id_type, 2>, std::array<vertex_id_type, 2>>>> futures(threads);
  for (std::size_t tid = 0; tid < threads; ++tid) {
    futures[tid] = std::async(std::launch::async, 
      [&](size_t thread) {
        
        size_t GN = mmio.getNCols();
        size_t GM = mmio.getNEdges();

        // figure out which edges I'm going to read from the file
        size_t min = block_min(thread, GM, threads);
        size_t max = block_min(thread + 1, GM, threads);

        std::array<vertex_id_type, 2> i_min = {std::numeric_limits<vertex_id_type>::max(), std::numeric_limits<vertex_id_type>::max()};
        std::array<vertex_id_type, 2> i_max = {0, 0};

        // slightly oversize to try and avoid resizing
        static constexpr double scale = 1.1;
        sub_lists[thread].reserve(scale*(max-min));
        size_t count = 0;

        // read edge data from the file
        for (auto&& [u, v] : mmio::edges<>(mmio, min, max)) {
          i_min[0] = std::min((vertex_id_type)u, i_min[0]);
          i_min[1] = std::min((vertex_id_type)v, i_min[1]);
          i_max[0] = std::max((vertex_id_type)u, i_max[0]);
          i_max[1] = std::max((vertex_id_type)v, i_max[1]);

          if(u == v && keep_loops) {
            sub_loops[thread].push_back(std::tuple(u-1,v-1));
          } else {
            sub_lists[thread].push_back(std::tuple(u-1, v-1));
            ++count;
          }
        }
        return std::tuple(count, i_min, i_max);
      }, tid);
  }
  std::size_t entries = 0;
  for (auto&& f : futures) {
    auto res = f.get();
    entries += std::get<0>(res);
    Gi_min[0] = std::min(std::get<1>(res)[0], Gi_min[0]);
    Gi_min[1] = std::min(std::get<1>(res)[1], Gi_min[1]);
    Gi_max[0] = std::max(std::get<2>(res)[0], Gi_max[0]);
    Gi_max[1] = std::max(std::get<2>(res)[1], Gi_max[1]);
  }
  return std::tuple(entries, Gi_min, Gi_max);
}


template <directedness sym, typename... Attributes>
edge_list<sym, Attributes...> par_read_mm(const std::string& filename, bool keep_loops = true, size_t threads = (size_t) std::thread::hardware_concurrency()) {
  mmio::MatrixMarketFile mmio(filename);
  if(!mmio.isSymmetric() && sym == undirected) {
    std::cerr << "warning: requested undirected edge list, but mtx file is general" << std::endl;
  }

  edge_list<sym, Attributes...> A(mmio.getNCols());
  std::vector<std::vector<std::tuple<size_t, size_t, Attributes...>>> sub_lists(threads);
  std::vector<std::vector<std::tuple<size_t, size_t, Attributes...>>> sub_loops(threads);
  
  auto info = par_load_mm(mmio, sub_lists, sub_loops, keep_loops, threads);
  if(mmio.isSymmetric() && sym == directed) {
    A.resize(std::get<0>(info)*2);
  }
  else {
    A.resize(std::get<0>(info));
  }
  A.min_ = std::get<1>(info);
  A.max_ = std::get<2>(info);

  mmio.release();
  
  std::vector<size_t> offsets(threads+1);
  offsets[0] = 0;
  for(size_t tid = 1; tid < threads; ++tid) {
    offsets[tid] = offsets[tid-1] + sub_lists[tid-1].size();
  }
  offsets[threads] = offsets[threads-1] + sub_lists[threads-1].size();
  
  std::vector<std::future<void>> futures(threads);
  for (std::size_t tid = 0; tid < threads; ++tid) {
    futures[tid] = std::async(std::launch::async,
      [&](size_t thread) {
	std::copy(std::execution::par_unseq, sub_lists[thread].begin(), sub_lists[thread].end(), A.begin() + offsets[thread]);
        if(mmio.isSymmetric() && sym == directed) {
          std::transform(std::execution::par_unseq, A.begin() + offsets[thread], A.begin() + offsets[thread+1], A.begin() + offsets[threads] + offsets[thread], [&](auto&& e){
            std::tuple<size_t, size_t, Attributes...> copy(e);
            std::swap(std::get<0>(copy), std::get<1>(copy));
            return copy;
          });
	}
      return;
      }, tid);
  }
  for (auto&& f : futures) {
    f.get();
  }

  if(keep_loops) {
    size_t loops = 0;
    for(size_t t = 0; t < threads; ++t) {
      loops += sub_loops[t].size();
    }
    A.resize(A.size() + loops);
    size_t offset = offsets[threads];
    if(mmio.isSymmetric() && sym == directed) {
      offset *= 2;
    }
    for(size_t t = 0; t < threads; ++t) {
      std::copy(std::execution::par_unseq, sub_loops[t].begin(), sub_loops[t].end(), A.begin() + offset);
      offset += sub_loops[t].size();
    }
  }
 
  A.close_for_push_back();
  A.set_origin(filename);
  
  return A;
}
#endif

}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_MMIO_HPP
