/**
 * @file demangle.hpp
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

#ifndef NW_GRAPH_DEMANGLE_HPP
#define NW_GRAPH_DEMANGLE_HPP

#include <cxxabi.h>
#include <string>

namespace nw {
namespace graph {
template <class... Args>
std::string demangle(Args&&... args) {
  auto        cstr = abi::__cxa_demangle(std::forward<Args>(args)...);
  std::string str(cstr);
  free(cstr);
  return str;
}
}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_DEMANGLE_HPP
