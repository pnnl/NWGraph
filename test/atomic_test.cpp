/**
 * @file atomic_test.cpp
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

#include <atomic>
#include <cstdint>

#include "common/test_header.hpp"

#include "nwgraph/util/atomic.hpp"

using namespace nw::graph;

TEST_CASE("Atomic load operations", "[atomic]") {

  SECTION("Relaxed load") {
    uint64_t value = 42;
    auto result = load<std::memory_order_relaxed>(value);
    REQUIRE(result == 42);
  }

  SECTION("Acquire load") {
    uint64_t value = 100;
    auto result = load<std::memory_order_acquire>(value);
    REQUIRE(result == 100);
  }

  SECTION("Sequential consistent load") {
    uint64_t value = 999;
    auto result = load<std::memory_order_seq_cst>(value);
    REQUIRE(result == 999);
  }
}

TEST_CASE("Atomic fetch_or operations", "[atomic]") {

  SECTION("Relaxed fetch_or") {
    uint64_t value = 0b1010;
    auto old = fetch_or<std::memory_order_relaxed>(value, 0b0101);

    REQUIRE(old == 0b1010);      // Returns old value
    REQUIRE(value == 0b1111);    // Value is ORed
  }

  SECTION("Acquire-release fetch_or") {
    uint64_t value = 0b1100;
    auto old = fetch_or<std::memory_order_acq_rel>(value, 0b0011);

    REQUIRE(old == 0b1100);
    REQUIRE(value == 0b1111);
  }

  SECTION("fetch_or with no change") {
    uint64_t value = 0b1111;
    auto old = fetch_or<std::memory_order_relaxed>(value, 0b1111);

    REQUIRE(old == 0b1111);
    REQUIRE(value == 0b1111);
  }
}

TEST_CASE("Atomic operations on different types", "[atomic]") {

  SECTION("uint32_t load") {
    uint32_t value = 12345;
    auto result = load<std::memory_order_relaxed>(value);
    REQUIRE(result == 12345);
  }

  SECTION("uint32_t fetch_or") {
    uint32_t value = 0xF0;
    auto old = fetch_or<std::memory_order_relaxed>(value, 0x0F);

    REQUIRE(old == 0xF0);
    REQUIRE(value == 0xFF);
  }
}

TEST_CASE("Atomic bit manipulation patterns", "[atomic]") {

  SECTION("Setting individual bits") {
    uint64_t bitmap = 0;

    // Set bit 0
    fetch_or<std::memory_order_relaxed>(bitmap, 1ULL << 0);
    REQUIRE(bitmap == 1);

    // Set bit 3
    fetch_or<std::memory_order_relaxed>(bitmap, 1ULL << 3);
    REQUIRE(bitmap == 9);  // 0b1001

    // Set bit 7
    fetch_or<std::memory_order_relaxed>(bitmap, 1ULL << 7);
    REQUIRE(bitmap == 137);  // 0b10001001
  }

  SECTION("Check if bit was already set") {
    uint64_t bitmap = 0b0100;

    // Try to set bit 2 (already set)
    auto old = fetch_or<std::memory_order_relaxed>(bitmap, 0b0100);
    bool was_set = (old & 0b0100) != 0;

    REQUIRE(was_set == true);

    // Try to set bit 0 (not set)
    old = fetch_or<std::memory_order_relaxed>(bitmap, 0b0001);
    was_set = (old & 0b0001) != 0;

    REQUIRE(was_set == false);
  }
}
