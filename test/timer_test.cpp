/**
 * @file timer_test.cpp
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

#include <chrono>
#include <thread>

#include "common/test_header.hpp"

#include "nwgraph/util/timer.hpp"

using namespace nw::util;

TEST_CASE("Timer basic operations", "[timer]") {

  SECTION("Timer measures elapsed time in microseconds") {
    // Default timer is microseconds
    timer t;
    t.start();

    // Sleep for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    t.stop();

    // Elapsed time should be at least 10000 microseconds (10ms)
    REQUIRE(t.elapsed() >= 10000.0);
    // But not too long (allow some slack for system scheduling)
    REQUIRE(t.elapsed() < 500000.0);  // 500ms max
  }

  SECTION("Timer can be restarted") {
    timer t;

    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    t.stop();
    double first_elapsed = t.elapsed();

    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    t.stop();
    double second_elapsed = t.elapsed();

    // Both measurements should be positive
    REQUIRE(first_elapsed > 0.0);
    REQUIRE(second_elapsed > 0.0);
  }

  SECTION("Millisecond timer") {
    ms_timer t;
    t.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    t.stop();

    // Elapsed time should be at least 10 milliseconds
    REQUIRE(t.elapsed() >= 10.0);
    REQUIRE(t.elapsed() < 500.0);  // 500ms max
  }

  SECTION("Seconds timer") {
    seconds_timer t;
    t.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    t.stop();

    // Elapsed time should be 0 seconds (truncated) or 1 second
    REQUIRE(t.elapsed() >= 0.0);
    REQUIRE(t.elapsed() <= 1.0);
  }
}

TEST_CASE("Life timer RAII", "[timer]") {

  SECTION("Life timer outputs on destruction") {
    // This test mainly verifies compilation and basic usage
    // The output goes to cout which we can't easily capture
    {
      life_timer lt("test scope");
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    // Timer should have printed output at this point
    REQUIRE(true);  // If we get here, no crash occurred
  }

  SECTION("Life timer in nested scopes") {
    {
      life_timer outer("outer");
      {
        life_timer inner("inner");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    REQUIRE(true);  // If we get here, no crash occurred
  }

  SECTION("Life timer with empty name") {
    {
      life_timer lt("");  // Empty name should not print
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    REQUIRE(true);
  }
}

TEST_CASE("Timer precision", "[timer]") {

  SECTION("Timer has reasonable precision") {
    timer t;

    // Measure very short operation
    t.start();
    volatile int x = 0;
    for (int i = 0; i < 1000; ++i) {
      x += i;
    }
    t.stop();

    // Should measure something (not negative)
    REQUIRE(t.elapsed() >= 0.0);
  }

  SECTION("Multiple timers are independent") {
    timer t1, t2;

    t1.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    t2.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    t1.stop();
    t2.stop();

    // t1 should have run longer than t2
    REQUIRE(t1.elapsed() > t2.elapsed());
  }
}

TEST_CASE("Timer accumulation", "[timer]") {

  SECTION("Sum multiple timer measurements") {
    ms_timer t;  // Use millisecond timer for clearer values
    double total = 0.0;

    for (int i = 0; i < 3; ++i) {
      t.start();
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      t.stop();
      total += t.elapsed();
    }

    // Total should be roughly 15ms (allow slack)
    REQUIRE(total >= 15.0);
    REQUIRE(total < 500.0);  // Less than 500ms
  }
}

TEST_CASE("Timer lap functionality", "[timer]") {

  SECTION("Lap stops and returns elapsed") {
    timer t;
    t.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    double lap_time = t.lap();

    // Lap should return time and have stopped the timer
    REQUIRE(lap_time >= 10000.0);  // At least 10ms in microseconds
    REQUIRE(t.elapsed() == lap_time);  // elapsed() should return same value
  }
}

TEST_CASE("Timer name functionality", "[timer]") {

  SECTION("Timer name can be retrieved") {
    timer t("my timer");
    REQUIRE(t.name() == "my timer");
  }

  SECTION("Empty timer name") {
    timer t;
    REQUIRE(t.name() == "");
  }
}
