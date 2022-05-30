/**
 * @file zip.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Kevin Deweese	
 *   Luke D'Alessandro	
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_ZIP_HPP
#define NW_GRAPH_ZIP_HPP

#include <cassert>

#include <fstream>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <istream>
#include <iterator>
#include <ostream>
#include <tuple>
#include <utility>
#include <vector>

#include "nwgraph/util/arrow_proxy.hpp"
#include "nwgraph/util/util.hpp"

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/algorithm>
#include <dpstd/execution>
#else
#include <algorithm>
#include <execution>
#endif


#include <ranges>


namespace nw {
namespace graph {


// Bare bones zipper of ranges

template <std::ranges::random_access_range... Ranges>
struct zipped : std::tuple<Ranges&...> {
  using storage_type = std::tuple<Ranges&...>;
  using base         = std::tuple<Ranges&...>;

  using attributes_t       = std::tuple<std::ranges::range_value_t<Ranges>...>;
  using const_attributes_t = std::tuple<const std::ranges::range_value_t<Ranges>...>;
  using attributes_r       = std::tuple<std::ranges::range_reference_t<Ranges>...>;
  using const_attributes_r = std::tuple<const std::ranges::range_reference_t<Ranges>...>;


  template <bool is_const = false>
  class soa_iterator {
    friend class soa_iterator<!is_const>;

    using soa_t = std::conditional_t<is_const, const zipped, zipped>;

    std::size_t i_;
    soa_t*      soa_;

  public:
    using value_type        = std::conditional_t<is_const, const_attributes_t, attributes_t>;
    using difference_type   = std::ptrdiff_t;
    using reference         = std::conditional_t<is_const, const_attributes_r, attributes_r>;
    using pointer           = arrow_proxy<reference>;
    using iterator_category = std::random_access_iterator_tag;

    soa_iterator() = default;

    explicit soa_iterator(soa_t* soa, std::size_t i = 0) : i_(i), soa_(soa) {
    }

    soa_iterator(soa_iterator&&)      = default;
    soa_iterator(const soa_iterator&) = default;

    soa_iterator(const soa_iterator<false>& b) requires(is_const) : i_(b.i_), soa_(b.soa_) {
    }

    soa_iterator& operator=(const soa_iterator&) = default;
    soa_iterator& operator=(soa_iterator&&) = default;

    soa_iterator& operator=(const soa_iterator<false>& b) requires(is_const) {
      i_   = b.i_;
      soa_ = b.soa_;
      return *this;
    }

    bool operator==(const soa_iterator&) const  = default;
    auto operator<=>(const soa_iterator&) const = default;

    soa_iterator operator++(int) {
      return soa_iterator(i_++, soa_);
    }
    soa_iterator operator--(int) {
      return soa_iterator(i_--, soa_);
    }

    soa_iterator& operator++() {
      ++i_;
      return *this;
    }
    soa_iterator& operator--() {
      --i_;
      return *this;
    }
    soa_iterator& operator+=(std::ptrdiff_t n) {
      i_ += n;
      return *this;
    }
    soa_iterator& operator-=(std::ptrdiff_t n) {
      i_ -= n;
      return *this;
    }

    soa_iterator operator+(std::ptrdiff_t n) const {
      return soa_iterator(soa_, i_ + n);
    }
    soa_iterator operator-(std::ptrdiff_t n) const {
      return soa_iterator(soa_, i_ - n);
    }

    friend soa_iterator operator+(std::ptrdiff_t n, soa_iterator i) {
      return i + n;
    }
    friend soa_iterator operator-(std::ptrdiff_t n, soa_iterator i) {
      return i - n;
    }

    std::ptrdiff_t operator-(const soa_iterator& b) const {
      return i_ - b.i_;
    }

    reference operator*() const {
      return std::apply(
          [this]<class... Vectors>(Vectors && ... v) { return reference(std::forward<Vectors>(v)[i_]...); }, *soa_);
    }

    reference operator[](std::ptrdiff_t n) const {
      return std::apply(
          [this, n]<class... Vectors>(Vectors && ... v) { return reference(std::forward<Vectors>(v)[i_ + n]...); }, *soa_);
    }

    pointer operator->() const {
      return {**this};
    }
    pointer operator->() {
      return {**this};
    }
  };

  using iterator = soa_iterator<false>;

  using value_type      = typename iterator::value_type;
  using reference       = typename iterator::reference;
  using size_type       = std::size_t;
  using difference_type = typename iterator::difference_type;
  using pointer         = typename iterator::pointer;

  using const_iterator  = soa_iterator<true>;
  using const_reference = typename const_iterator::reference;
  using const_pointer   = typename const_iterator::pointer;

  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  //  zipped() = default;

  // zipped(size_t M) : base(std::vector<Attributes>(M)...) {}
  // zipped(std::initializer_list<value_type> l) {
  // for_each(l.begin(), l.end(), [&](value_type x) { push_back(x); });
  // }


  explicit zipped(Ranges&... rs) : base(std::forward_as_tuple(rs...)) {
  }

  iterator begin() {
    return iterator(this);
  }
  const_iterator begin() const {
    return const_iterator(this);
  }

  iterator end() {
    return begin() + size();
  }

  const_iterator end() const {
    return begin() + size();
  }

  reference operator[](std::size_t i) {
    return std::apply([&](auto&&... r) { return std::forward_as_tuple(std::forward<decltype(r)>(r)[i]...); }, *this);
  }

  const_reference operator[](std::size_t i) const {
    return std::apply([&](auto&&... r) { return std::forward_as_tuple(std::forward<decltype(r)>(r)[i]...); }, *this);
  }


#if 0
  void push_back(Attributes... attrs) {
    std::apply([&](auto&... vs) { (vs.push_back(attrs), ...); }, *this);
  }

  void push_back(std::tuple<Attributes...> attrs) {
    std::apply([&](Attributes... attr) { push_back(attr...); }, attrs);
  }

  void push_at(std::size_t i, Attributes... attrs) {
    std::apply([&](auto&... vs) { ((vs[i] = attrs), ...); }, *this);
  }

  void push_at(std::size_t i, std::tuple<Attributes...> attrs) {
    std::apply([&](Attributes... attr) { push_at(i, attr...); }, attrs);
  }
#endif

  void clear() {
    std::apply([&](auto&... vs) { (vs.clear(), ...); }, *this);
  }

  void resize(size_t N) {
    std::apply([&](auto&&... vs) { (vs.resize(N), ...); }, *this);
  }

  void reserve(size_t N) {
    std::apply([&](auto&&... vs) { (vs.reserve(N), ...); }, *this);
  }

  template <class T>
  void serialize(std::ostream& outfile, const T& vs) const {
    size_t st_size = vs.size();
    size_t el_size = sizeof(vs[0]);
    outfile.write(reinterpret_cast<char*>(&st_size), sizeof(size_t));
    outfile.write(reinterpret_cast<char*>(&el_size), sizeof(size_t));
    outfile.write(reinterpret_cast<const char*>(vs.data()), st_size * el_size);
  }

  template <class T>
  void deserialize(std::istream& infile, T& vs) {
    size_t st_size = -1;
    size_t el_size = -1;
    infile.read(reinterpret_cast<char*>(&st_size), sizeof(size_t));
    infile.read(reinterpret_cast<char*>(&el_size), sizeof(size_t));
    infile.read(reinterpret_cast<char*>(vs.data()), st_size * el_size);
  }

  void serialize(std::ostream& outfile) const {
    size_t st_size = std::get<0>(*this).size();
    size_t el_size = std::tuple_size<storage_type>::value;
    outfile.write(reinterpret_cast<char*>(&st_size), sizeof(size_t));
    outfile.write(reinterpret_cast<char*>(&el_size), sizeof(size_t));
    std::apply([&](auto&... vs) { (serialize(outfile, vs), ...); }, *this);
  }

  void deserialize(std::istream& infile) {
    size_t st_size = -1;
    size_t el_size = -1;
    infile.read(reinterpret_cast<char*>(&st_size), sizeof(size_t));
    infile.read(reinterpret_cast<char*>(&el_size), sizeof(size_t));
    resize(st_size);
    std::apply([&](auto&... vs) { (deserialize(infile, vs), ...); }, *this);
  }

  template <typename index_t, typename vertex_id_type, class T, class ExecutionPolicy = std::execution::parallel_unsequenced_policy>
  void permute(const std::vector<index_t>& indices, const std::vector<index_t>& new_indices, const std::vector<vertex_id_type>& perm, T& vs,
               ExecutionPolicy&& ex_policy = {}) {
    T ws(vs.size());
    for (size_t i = 0, e = indices.size() - 1; i < e; ++i) {
      vertex_id_type j = perm[i];
      std::copy(ex_policy, vs.begin() + indices[j], vs.begin() + indices[j + 1], ws.begin() + new_indices[i]);
    }
    std::copy(ex_policy, ws.begin(), ws.end(), vs.begin());
  }

  template <typename index_t, typename vertex_id_type>
  void permute(const std::vector<index_t>& indices, const std::vector<index_t>& new_indices, const std::vector<vertex_id_type>& perm) {
    std::apply([&](auto&... vs) { (permute(indices, new_indices, perm, vs), ...); }, *this);
  }

  size_t size() const {
    return std::get<0>(*this).size();
  }

  bool operator==(zipped& a) {
    return std::equal(std::execution::par, begin(), end(), a.begin());
  }

  bool operator!=(const storage_type& a) {
    return !operator==(a);
  }
};


template <std::ranges::random_access_range... Ranges>
zipped<Ranges...> make_zipped(Ranges&... rs) {
  return zipped<Ranges...>(rs...);
}

template <std::ranges::random_access_range... Ranges>
zipped<Ranges...> make_zipped(Ranges&&... rs) {
  return zipped<Ranges...>(rs...);
}


}    // namespace graph
}    // namespace nw

namespace std {


#if 0
template <std::ranges::random_access_range... Ranges>
auto iter_swap(typename nw::graph::zipped<Ranges...>::soa_iterator<false> a, typename nw::graph::zipped<Ranges...>::soa_iterator<false> b) {
  auto tmp = *a;
  *a = *b;
  *b = *tmp;
}
#endif


template <class... Attributes>
class tuple_size<nw::graph::zipped<Attributes...>> : public std::integral_constant<std::size_t, sizeof...(Attributes)> {};

}    // namespace std


/// NB: technically we're supposed to be using `iter_swap` here on the
/// zipped iterator type, but I can't figure out how to do this.
#include "nwgraph/util/tuple_hack.hpp"


#if 0
namespace std {

template <std::ranges::random_access_range... Ranges, std::size_t... Is>
void swap(typename nw::graph::zipped<Ranges...>::iterator::reference&& x, typename nw::graph::zipped<Ranges...>::iterator::reference&& y, std::index_sequence<Is...>) {
  using std::swap;
  
  (swap(std::get<Is>(x), std::get<Is>(y)), ...);
}

template <std::ranges::random_access_range... Ranges>
void swap(typename nw::graph::zipped<Ranges...>::iterator::reference&& x, typename nw::graph::zipped<Ranges...>::iterator::reference&& y) {
  swap(std::move(x), std::move(y), std::make_index_sequence<sizeof...(Ranges)>());
}

} 
#endif




#endif    // NW_GRAPH_ZIP_HPP
