/**
 * @file provenance.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese	
 *
 */

#ifndef NW_GRAPH_PROVENANCE_HPP
#define NW_GRAPH_PROVENANCE_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace nw {
namespace graph {

std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ") { return ltrim(rtrim(str, chars), chars); }

class provenance {
public:
  provenance(const provenance&) = default;
  provenance()                  = default;
  explicit provenance(size_t n) : info(n) {}
  void push_back(const std::string& fn, const std::string& str) {
    std::string f(fn), g(str);
    trim(f);
    trim(g);
    info.push_back(fn + ": " + str);
    trim(info.back());
  }

  void push_back(const std::string& str) {
    info.push_back(str);
    trim(info.back());
  }

  void stream(std::ostream& os, const std::string& prefix = "%") const {
    for (auto s : info) {
      os << prefix + " " << s << std::endl;
    }
  }
  auto operator=(const provenance& x) {
    for (auto& j : x.info) {
      info.push_back(j);
    }
  }

private:
  std::vector<std::string> info;
};

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_PROVENANCE_HPP
