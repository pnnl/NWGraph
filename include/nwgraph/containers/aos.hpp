/**
 * @file aos.hpp
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

/**
 * @brief 
 * 
 * @todo Add emplace functions
 * 
 * @file aos.hpp
 * @author your name
 * @date 2018-08-21
 */

#ifndef NW_GRAPH_AOS_HPP
#define NW_GRAPH_AOS_HPP

#include <execution>

#include <algorithm>
#include <cassert>

#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <ostream>
#include <vector>

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/execution>
#else
#include <execution>
#endif

namespace nw {
namespace graph {

// Bare bones array of structs (vector of tuples)
template <typename... Attributes>
class array_of_structs : public std::vector<std::tuple<Attributes...>> {

public:
  using storage_type = std::vector<std::tuple<Attributes...>>;
  using base         = std::vector<std::tuple<Attributes...>>;
  using reference    = typename std::iterator_traits<typename storage_type::iterator>::reference;

  //  storage_type storage_;

public:
#if 0

  void clear() { storage_.clear(); }
  void resize(size_t n) { storage_.resize(n); }
  void reserve(size_t n) { storage_.reserve(n); }

  auto begin() { return storage_.begin(); }
  auto end() { return storage_.end(); }

  template <typename It>
  void erase(It b, It e) { storage_.erase(b, e); }
  size_t size() const { return base::size(); }

#endif

  void push_back(const std::tuple<Attributes...>& attrs) { base::push_back(attrs); }
  void push_back(const std::tuple<Attributes&...>& attrs) { base::push_back(attrs); }
  void push_back(const Attributes&... attrs) { base::push_back({attrs...}); }

  bool operator==(array_of_structs& a) { return std::equal(std::execution::par, base::begin(), base::end(), a.begin()); }
  bool operator!=(const storage_type& a) { return !operator==(a); }

  template <size_t... Is>
  void print_helper(std::ostream& output_stream, std::tuple<Attributes...> attrs, std::index_sequence<Is...>) {
    output_stream << "( ";
    (..., (output_stream << (0 == Is ? "" : ", ") << std::get<Is>(attrs)));
    output_stream << " )" << std::endl;
  }

  void stream(std::ostream& output_stream, const std::string& msg = "") {
    output_stream << msg;
    for (auto& element : *this) {
      print_helper(output_stream, element, std::make_index_sequence<sizeof...(Attributes)>());
    }
  }

  void stream(const std::string& msg = "") { stream(std::cout, msg); }

  // size (number of elements)
  // sizeof data elements
  // data elements

  void serialize(std::ostream& outfile) const {
    size_t st_size = base::size();
    size_t el_size = sizeof(dynamic_cast<const base&>(*this)[0]);

    outfile.write(reinterpret_cast<const char*>(&st_size), sizeof(size_t));
    outfile.write(reinterpret_cast<const char*>(&el_size), sizeof(size_t));
    outfile.write(reinterpret_cast<const char*>(base::data()), st_size * el_size);
  }

  void serialize(const std::string& outfile_name) {
    std::ofstream out_file(outfile_name, std::ofstream::binary);
    serialize(out_file);
  }

  void deserialize(std::istream& infile) {
    size_t st_size = -1;
    size_t el_size = -1;

    infile.read(reinterpret_cast<char*>(&st_size), sizeof(size_t));
    infile.read(reinterpret_cast<char*>(&el_size), sizeof(size_t));
    base::resize(st_size);
    infile.read(reinterpret_cast<char*>(base::data()), st_size * el_size);
  }

  void deserialize(const std::string& infile_name) {
    std::ifstream infile(infile_name, std::ifstream::binary);
    deserialize(infile);
  }
};
}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_AOS_HPP
