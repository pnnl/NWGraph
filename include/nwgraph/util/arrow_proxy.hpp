/**
 * @file arrow_proxy.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Luke D'Alessandro
 *
 */

#ifndef NW_GRAPH_UTIL_ARROW_PROXY_HPP
#define NW_GRAPH_UTIL_ARROW_PROXY_HPP

namespace nw::graph {

template <class Reference>
struct arrow_proxy {
  Reference  r;
  Reference* operator->() { return &r; }
};

}    // namespace nw::graph


#endif // NW_GRAPH_UTIL_ARROW_PROXY_HPP
