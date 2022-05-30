/**
 * @file compressed_test.cpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese
 *   liux238
 *
 */


#include <iostream>

#include "common/test_header.hpp"
#include "nwgraph/adjacency.hpp"
#include "nwgraph/edge_list.hpp"

using namespace nw::graph;
using namespace nw::util;

template <typename... Attributes>
using compressed = indexed_struct_of_arrays<unsigned, unsigned, Attributes...>;

const size_t N = 5;

TEST_CASE("indexed_struct_of_arrays construct", "[construct]") {
  SECTION("construct") { compressed<> A(N); }

  SECTION("push_back diagonal") {
    compressed<> A(N);
    A.open_for_push_back();
    for (size_t i = 0; i < A.size(); ++i) {
      A.push_back(i, i);
    }
    A.close_for_push_back();
  }

  SECTION("push_back diagonal less 0") {
    compressed<> A(N);
    A.open_for_push_back();
    for (size_t i = 1; i < A.size(); ++i) {
      A.push_back(i, i);
    }
    A.close_for_push_back();
  }

  SECTION("push_back diagonal less N-1") {
    compressed<> A(N);
    A.open_for_push_back();
    for (size_t i = 0; i < A.size() - 1; ++i) {
      A.push_back(i, i);
    }
    A.close_for_push_back();
  }
}

TEST_CASE("indexed_struct_of_arrays<double> construct", "[construct<double>]") {
  SECTION("construct") { compressed<double> A(N); }
  SECTION("push_back diagonal") {
    compressed<double> A(N);
    A.open_for_push_back();
    for (size_t i = 0; i < A.size(); ++i) {
      A.push_back(i, i, i * 3.14159);
    }
    A.close_for_push_back();
  }

  SECTION("push_back diagonal less 0") {
    compressed<double> A(N);
    A.open_for_push_back();
    for (size_t i = 1; i < A.size(); ++i) {
      A.push_back(i, i, i * 3.14159);
    }
    A.close_for_push_back();
  }

  SECTION("push_back diagonal less N-1") {
    compressed<double> A(N);
    A.open_for_push_back();
    for (size_t i = 0; i < A.size() - 1; ++i) {
      A.push_back(i, i, i * 3.14159);
    }
    A.close_for_push_back();
  }
}

TEST_CASE("indexed_struct_of_arrays outer iteration", "[outer]") {
  SECTION("") {
    compressed<double> A(N);
    A.open_for_push_back();
    for (size_t i = 0; i < A.size(); ++i) {
      A.push_back(i, i, i * 3.14159);
      if (i > 0) {
        A.push_back(i, i - 1, i * 3.14159);
      }
    }
    A.close_for_push_back();
    size_t i = 0;
    for (auto&& j : A) {
      if (i++ > 0) {
        REQUIRE(j.size() == 2);
      } else {
        REQUIRE(j.size() == 1);
      }
    }
  }
}

TEST_CASE("indexed_struct_of_arrays inner iteration", "[inner]") {
  SECTION("") {
    compressed<double> A(N);
    A.open_for_push_back();
    for (size_t i = 0; i < A.size(); ++i) {
      A.push_back(i, i, i * 3.14159);
      if (i > 0) {
        A.push_back(i, i - 1, i * 3.14159);
      }
    }
    A.close_for_push_back();
    size_t i = 0;
    for (auto&& j : A) {
      for (auto&& k : j) {
        std::cout << std::get<0>(k) << " " << std::get<1>(k) << std::endl;
      }
    }
  }
}

TEST_CASE("indexed_struct_of_arrays flat iteration", "[flat]") {
  SECTION("") {}
}

template <typename SOA>
void foo(SOA& s) {

  size_t i = 0;
  for (auto&& j : s) {
    for (auto&& k : j) {
      std::cout << std::get<0>(k) << " " << std::get<1>(k) << std::endl;
    }
  }
}

template <typename SOA>
void bar(const SOA& s) {

  size_t i = 0;
  for (auto&& j : s) {
    for (auto&& k : j) {
      std::cout << std::get<0>(k) << " " << std::get<1>(k) << std::endl;
    }
  }
}

TEST_CASE("indexed_struct_of_arrays const iteration", "[const]") {
  SECTION("") {
    compressed<double> A(N);
    A.open_for_push_back();
    for (size_t i = 0; i < A.size(); ++i) {
      A.push_back(i, i, i * 3.14159);
      if (i > 0) {
        A.push_back(i, i - 1, i * 3.14159);
      }
    }
    A.close_for_push_back();
    foo(A);
    bar(A);
  }
}

#if 0

// data/karate.mtx:%%MatrixMarket matrix coordinate pattern symmetric
// data/karate.mtx:%%MatrixMarket matrix coordinate pattern symmetric

// data/tree.mmio:%%MatrixMarket matrix coordinate real symmetric
// data/USAir97.mtx:%%MatrixMarket matrix coordinate real symmetric

TEST_CASE("compressed class I/O", "[compressed_io]") {
  SECTION("I/O (read real symmetric to edge_list and convert to compressed graph)") {
    auto A = read_mm<directedness::directed>(DATA_DIR "tree.mmio");
    auto B = read_mm<directedness::undirected>(DATA_DIR "tree.mmio");

    auto C = read_mm<directedness::directed>(DATA_DIR "USAir97.mtx");
    auto D = read_mm<directedness::undirected>(DATA_DIR "USAir97.mtx");
  }
  SECTION("I/O (read pattern symmetric to edge_list and convert to compressed graph)") {
    auto A = read_mm<directedness::directed>(DATA_DIR "karate.mtx");
    auto B = read_mm<directedness::undirected>(DATA_DIR "karate.mtx");
  }
  SECTION("I/O (read real unsymmetric to edge_list and convert to compressed graph)") {
    auto A = read_mm<directedness::directed>(DATA_DIR "tree.mmio");
    auto B = read_mm<directedness::undirected>(DATA_DIR "tree.mmio");
  }
  SECTION("I/O (read pattern unsymmetric to edge_list and convert to compressed graph)") {}
  SECTION("I/O (read to edge_list and convert to compressed matrix)") {}
}


TEST_CASE("compressed class iteration", "[compressed]") {
  SECTION("push_back") {

  }
  SECTION("read") {

  }
  SECTION("modify value") {
  }
}


  auto C = read_mm<directedness::directed, double>(argv[1]);
  auto D = read_mm<directedness::undirected, double>(argv[1]);

  compressed_sparse<0, directed>   E(A);
  compressed_sparse<1, directed>   F(A);
  compressed_sparse<0, undirected> G(B);
  compressed_sparse<1, undirected> H(B);

  compressed_sparse<0, directed, double>   I(C);
  compressed_sparse<1, directed, double>   J(C);
  compressed_sparse<0, undirected, double> K(D);
  compressed_sparse<1, undirected, double> L(D);

}
}
#endif
