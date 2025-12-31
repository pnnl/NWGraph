/**
 * @file provenance_test.cpp
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

#include <sstream>
#include <string>

#include "common/test_header.hpp"

#include "nwgraph/util/provenance.hpp"

using namespace nw::graph;

TEST_CASE("String trimming functions", "[provenance]") {

  SECTION("ltrim removes leading whitespace") {
    std::string s = "   hello";
    ltrim(s);
    REQUIRE(s == "hello");
  }

  SECTION("ltrim with tabs and newlines") {
    std::string s = "\t\n  hello";
    ltrim(s);
    REQUIRE(s == "hello");
  }

  SECTION("rtrim removes trailing whitespace") {
    std::string s = "hello   ";
    rtrim(s);
    REQUIRE(s == "hello");
  }

  SECTION("rtrim with tabs and newlines") {
    std::string s = "hello\t\n  ";
    rtrim(s);
    REQUIRE(s == "hello");
  }

  SECTION("trim removes both") {
    std::string s = "  hello  ";
    trim(s);
    REQUIRE(s == "hello");
  }

  SECTION("trim preserves internal whitespace") {
    std::string s = "  hello world  ";
    trim(s);
    REQUIRE(s == "hello world");
  }
}

TEST_CASE("Provenance basic operations", "[provenance]") {

  SECTION("Default construction") {
    provenance p;
    std::ostringstream os;
    p.stream(os);
    REQUIRE(os.str().empty());
  }

  SECTION("Construction with size") {
    provenance p(10);
    std::ostringstream os;
    p.stream(os);
    // Should be empty - size just reserves capacity
  }

  SECTION("Push back with function name and string") {
    provenance p;
    p.push_back("test_function", "some info");

    std::ostringstream os;
    p.stream(os);

    REQUIRE(os.str().find("test_function") != std::string::npos);
    REQUIRE(os.str().find("some info") != std::string::npos);
  }

  SECTION("Push back single string") {
    provenance p;
    p.push_back("standalone info");

    std::ostringstream os;
    p.stream(os);

    REQUIRE(os.str().find("standalone info") != std::string::npos);
  }

  SECTION("Stream with custom prefix") {
    provenance p;
    p.push_back("info");

    std::ostringstream os;
    p.stream(os, "#");

    REQUIRE(os.str().find("# info") != std::string::npos);
  }

  SECTION("Multiple entries") {
    provenance p;
    p.push_back("func1", "info1");
    p.push_back("func2", "info2");
    p.push_back("standalone");

    std::ostringstream os;
    p.stream(os);

    std::string result = os.str();
    REQUIRE(result.find("func1") != std::string::npos);
    REQUIRE(result.find("func2") != std::string::npos);
    REQUIRE(result.find("standalone") != std::string::npos);
  }
}

TEST_CASE("Provenance copy and assignment", "[provenance]") {

  SECTION("Copy construction") {
    provenance p1;
    p1.push_back("original");

    provenance p2(p1);

    std::ostringstream os;
    p2.stream(os);
    REQUIRE(os.str().find("original") != std::string::npos);
  }

  SECTION("Assignment operator") {
    provenance p1;
    p1.push_back("source");

    provenance p2;
    p2 = p1;

    std::ostringstream os;
    p2.stream(os);
    REQUIRE(os.str().find("source") != std::string::npos);
  }
}
