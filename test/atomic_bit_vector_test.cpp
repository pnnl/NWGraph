/**
 * @file atomic_bit_vector_test.cpp
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

#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/util/AtomicBitVector.hpp"

using namespace nw::graph;

TEST_CASE("AtomicBitVector construction", "[atomic_bit_vector]") {

  SECTION("Default initialized to zero") {
    AtomicBitVector<> bv(100);

    // All bits should be zero
    for (size_t i = 0; i < 100; ++i) {
      REQUIRE(bv.get(i) == 0);
    }
  }

  SECTION("Uninitialized construction") {
    AtomicBitVector<> bv(100, false);
    // No guarantee about values, just that it doesn't crash
    (void)bv.get(0);
    (void)bv.get(99);
  }
}

TEST_CASE("AtomicBitVector set and get", "[atomic_bit_vector]") {

  SECTION("Set and get individual bits") {
    AtomicBitVector<> bv(100);

    // Set bit 5
    auto was_set = bv.set(5);
    REQUIRE(was_set == 0);  // Was not set before

    // Get bit 5
    REQUIRE(bv.get(5) != 0);

    // Get bit 6 (not set)
    REQUIRE(bv.get(6) == 0);
  }

  SECTION("Double set returns previous value") {
    AtomicBitVector<> bv(64);

    bv.set(10);
    auto was_set = bv.set(10);

    REQUIRE(was_set != 0);  // Was already set
  }

  SECTION("Set multiple bits") {
    AtomicBitVector<> bv(200);

    bv.set(0);
    bv.set(63);
    bv.set(64);
    bv.set(127);
    bv.set(199);

    REQUIRE(bv.get(0) != 0);
    REQUIRE(bv.get(63) != 0);
    REQUIRE(bv.get(64) != 0);
    REQUIRE(bv.get(127) != 0);
    REQUIRE(bv.get(199) != 0);

    REQUIRE(bv.get(1) == 0);
    REQUIRE(bv.get(62) == 0);
    REQUIRE(bv.get(65) == 0);
  }
}

TEST_CASE("AtomicBitVector atomic operations", "[atomic_bit_vector]") {

  SECTION("Atomic set") {
    AtomicBitVector<> bv(100);

    auto was_set = bv.atomic_set(42);
    REQUIRE(was_set == 0);

    was_set = bv.atomic_set(42);
    REQUIRE(was_set != 0);
  }

  SECTION("Atomic get") {
    AtomicBitVector<> bv(100);

    bv.atomic_set(50);
    REQUIRE(bv.atomic_get(50) != 0);
    REQUIRE(bv.atomic_get(51) == 0);
  }
}

TEST_CASE("AtomicBitVector clear", "[atomic_bit_vector]") {

  SECTION("Clear resets all bits") {
    AtomicBitVector<> bv(100);

    bv.set(10);
    bv.set(50);
    bv.set(90);

    bv.clear();

    REQUIRE(bv.get(10) == 0);
    REQUIRE(bv.get(50) == 0);
    REQUIRE(bv.get(90) == 0);
  }
}

TEST_CASE("AtomicBitVector non_zero_iterator", "[atomic_bit_vector]") {

  SECTION("Iterate over set bits") {
    AtomicBitVector<> bv(200);

    bv.set(5);
    bv.set(42);
    bv.set(100);
    bv.set(150);

    std::vector<size_t> set_bits;
    for (auto it = bv.begin(); it != bv.end(); ++it) {
      set_bits.push_back(*it);
    }

    REQUIRE(set_bits.size() == 4);
    REQUIRE(set_bits[0] == 5);
    REQUIRE(set_bits[1] == 42);
    REQUIRE(set_bits[2] == 100);
    REQUIRE(set_bits[3] == 150);
  }

  SECTION("Empty bit vector iteration") {
    AtomicBitVector<> bv(100);

    std::vector<size_t> set_bits;
    for (auto it = bv.begin(); it != bv.end(); ++it) {
      set_bits.push_back(*it);
    }

    REQUIRE(set_bits.empty());
  }

  SECTION("All bits set in first word") {
    AtomicBitVector<> bv(64);

    for (size_t i = 0; i < 64; ++i) {
      bv.set(i);
    }

    size_t count = 0;
    for (auto it = bv.begin(); it != bv.end(); ++it) {
      ++count;
    }

    REQUIRE(count == 64);
  }
}

TEST_CASE("AtomicBitVector move semantics", "[atomic_bit_vector]") {

  SECTION("Move construction") {
    AtomicBitVector<> bv1(100);
    bv1.set(25);

    AtomicBitVector<> bv2(std::move(bv1));

    REQUIRE(bv2.get(25) != 0);
  }

  SECTION("Move assignment") {
    AtomicBitVector<> bv1(100);
    bv1.set(30);

    AtomicBitVector<> bv2(50);
    bv2 = std::move(bv1);

    REQUIRE(bv2.get(30) != 0);
  }
}
