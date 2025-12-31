/**
 * @file demangle_test.cpp
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

#include <string>
#include <typeinfo>
#include <vector>

#include "common/test_header.hpp"

#include "nwgraph/util/demangle.hpp"

using namespace nw::graph;

namespace test_ns {
class TestClass {};

template <typename T>
class TemplateClass {};
}  // namespace test_ns

TEST_CASE("Demangle basic types", "[demangle]") {

  SECTION("Demangle int") {
    std::string mangled = typeid(int).name();
    std::string demangled = demangle(mangled.c_str(), nullptr, nullptr, nullptr);

    REQUIRE(demangled == "int");
  }

  SECTION("Demangle double") {
    std::string mangled = typeid(double).name();
    std::string demangled = demangle(mangled.c_str(), nullptr, nullptr, nullptr);

    REQUIRE(demangled == "double");
  }

  SECTION("Demangle char") {
    std::string mangled = typeid(char).name();
    std::string demangled = demangle(mangled.c_str(), nullptr, nullptr, nullptr);

    REQUIRE(demangled == "char");
  }
}

TEST_CASE("Demangle standard library types", "[demangle]") {

  SECTION("Demangle vector<int>") {
    std::string mangled = typeid(std::vector<int>).name();
    std::string demangled = demangle(mangled.c_str(), nullptr, nullptr, nullptr);

    // Should contain "vector" and "int"
    REQUIRE(demangled.find("vector") != std::string::npos);
    REQUIRE(demangled.find("int") != std::string::npos);
  }

  SECTION("Demangle string") {
    std::string mangled = typeid(std::string).name();
    std::string demangled = demangle(mangled.c_str(), nullptr, nullptr, nullptr);

    // Should contain "basic_string" or "string"
    REQUIRE((demangled.find("string") != std::string::npos ||
             demangled.find("basic_string") != std::string::npos));
  }
}

TEST_CASE("Demangle user types", "[demangle]") {

  SECTION("Demangle namespaced class") {
    std::string mangled = typeid(test_ns::TestClass).name();
    std::string demangled = demangle(mangled.c_str(), nullptr, nullptr, nullptr);

    REQUIRE(demangled.find("test_ns") != std::string::npos);
    REQUIRE(demangled.find("TestClass") != std::string::npos);
  }

  SECTION("Demangle template class") {
    std::string mangled = typeid(test_ns::TemplateClass<int>).name();
    std::string demangled = demangle(mangled.c_str(), nullptr, nullptr, nullptr);

    REQUIRE(demangled.find("TemplateClass") != std::string::npos);
    REQUIRE(demangled.find("int") != std::string::npos);
  }
}

TEST_CASE("Demangle pointers and references", "[demangle]") {

  SECTION("Demangle pointer type") {
    std::string mangled = typeid(int*).name();
    std::string demangled = demangle(mangled.c_str(), nullptr, nullptr, nullptr);

    REQUIRE(demangled.find("int") != std::string::npos);
    REQUIRE(demangled.find("*") != std::string::npos);
  }

  SECTION("Demangle const pointer") {
    std::string mangled = typeid(const int*).name();
    std::string demangled = demangle(mangled.c_str(), nullptr, nullptr, nullptr);

    REQUIRE(demangled.find("int") != std::string::npos);
  }
}
