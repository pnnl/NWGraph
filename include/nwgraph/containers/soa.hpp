/**
 * @file soa.hpp
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
 *   Scott McMillan
 */

#ifndef NW_GRAPH_SOA_HPP
#define NW_GRAPH_SOA_HPP

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
#include "nwgraph/util/traits.hpp"
#include "nwgraph/util/util.hpp"

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/algorithm>
#include <dpstd/execution>
#else
#include <algorithm>
#include <execution>
#endif

namespace nw::graph {


/**
 * Bare-bones variadic structure of arrays class.
 *
 * @tparam Attributes Parameter pack of types.  The `struct_of_arrays` will have one
 * component vector for each type in `Attributes`.  The order of the vectors will be
 * the same as the order of the types in `Attributes`.  The `struct_of_arrays` is a
 * random-access range and provides an iterator that returns a tuple of referenes to
 * the underlying vectors, with each component of the tuple corresponding to one
 * underlying vector (again, in the order given by `Attributes`.
 */
template <class... Attributes>
struct struct_of_arrays : std::tuple<std::vector<Attributes>...> {
  using storage_type = std::tuple<std::vector<Attributes>...>;
  using base         = std::tuple<std::vector<Attributes>...>;

  /**
   * Member class defining the iterator for `struct_of_arrays`.  We use a template
   * parameter `is_const` to allow us to define both the iterator type and the
   * const_iterator type.
   *
   * Rather than maintaining a tuple of iterators (one for each underlying vector), we
   * maintain a pointer to the `struct_of_arrays` and maintain a cursor in the
   * iterator to indicate our current location.  We use this approach for efficiency.
   * Rather than incrementing a set of iterators when we advance the
   * `struct_or_arrays` iterator, we only need to advance the cursor.  This only works
   * for the random access case, which we have for `struct_of_arrays`.
   */
  template <bool is_const = false>
  class soa_iterator {
    friend class soa_iterator<!is_const>;

    using soa_t = std::conditional_t<is_const, const struct_of_arrays, struct_of_arrays>;

    std::size_t i_ { 0 };
    soa_t*      soa_ { nullptr };

  public:
    using value_type        = std::conditional_t<is_const, std::tuple<const typename std::vector<Attributes>::value_type...>,
                                                 std::tuple<typename std::vector<Attributes>::value_type...>>;
    using difference_type   = std::ptrdiff_t;
    using reference         = std::conditional_t<is_const, std::tuple<select_access_type<typename std::vector<Attributes>::const_iterator>...>,
                                                 std::tuple<select_access_type<typename std::vector<Attributes>::iterator>...>>;
    using pointer           = arrow_proxy<reference>;
    using iterator_category = std::random_access_iterator_tag;

    soa_iterator() = default;

    explicit soa_iterator(soa_t* soa, std::size_t i = 0) : i_(i), soa_(soa) {
    }

    soa_iterator(const soa_iterator&) = default;
    explicit soa_iterator(const soa_iterator<false>& b)
      requires(is_const)
        : i_(b.i_), soa_(b.soa_) {
    }

    soa_iterator& operator=(const soa_iterator&) = default;

    soa_iterator& operator=(const soa_iterator<false>& b)
      requires(is_const)
    {
      i_   = b.i_;
      soa_ = b.soa_;
      return *this;
    }

    bool operator==(const soa_iterator&) const  = default;
    auto operator<=>(const soa_iterator&) const = default;

    soa_iterator operator++(int) {
      return soa_iterator { i_++, soa_ };
    }

    soa_iterator operator--(int) {
      return soa_iterator { i_--, soa_ };
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

    std::ptrdiff_t operator-(const soa_iterator& b) const {
      return i_ - b.i_;
    }

    friend soa_iterator operator+(std::ptrdiff_t n, soa_iterator i) {
      return i + n;
    }

    friend soa_iterator operator-(std::ptrdiff_t n, soa_iterator i) {
      return i - n;
    }

    decltype(auto) operator*() const {
      return std::apply([this]<class... Vectors>(Vectors&&... v) { return reference(std::forward<Vectors>(v)[i_]...); }, *soa_);
    }

    decltype(auto) operator[](std::ptrdiff_t n) const {
      return std::apply([this, n]<class... Vectors>(Vectors&&... v) { return reference(std::forward<Vectors>(v)[i_ + n]...); }, *soa_);
    }

    pointer operator->() const {
      return { **this };
    }

    pointer operator->() {
      return { **this };
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

  struct_of_arrays() = default;

  explicit struct_of_arrays(size_t M) : base(std::vector<Attributes>(M)...) {
  }

  explicit struct_of_arrays(std::vector<Attributes>&&... l) : base(std::move(l)...) {
  }

  explicit struct_of_arrays(const std::vector<Attributes>&... l) : base(l...) {
  }

  explicit struct_of_arrays(std::tuple<std::vector<Attributes>...>&& l) : base(std::move(l)) {
  }

  explicit struct_of_arrays(const std::tuple<std::vector<Attributes>...>& l) : base(l) {
  }

  struct_of_arrays(std::initializer_list<value_type> l) {
    for_each(l.begin(), l.end(), [&](value_type x) { push_back(x); });
  }

  iterator begin() {
    return iterator(this);
  }

  const_iterator begin() const {
    return const_iterator(this);
  }

  const_iterator cbegin() const {
    return const_iterator(this);
  }

  iterator end() {
    return begin() + size();
  }

  const_iterator end() const {
    return begin() + size();
  }

  const_iterator cend() const {
    return begin() + size();
  }

  reference operator[](std::size_t i) {
    return std::apply([&](auto&... r) { return std::forward_as_tuple(std::forward<decltype(r)>(r)[i]...); }, *this);
  }

  const_reference operator[](std::size_t i) const {
    return std::apply([&](auto&... r) { return std::forward_as_tuple(std::forward<decltype(r)>(r)[i]...); }, *this);
  }

  constexpr pointer data() noexcept {
    return std::apply([&](auto&... p) { return std::forward_as_tuple(std::forward<decltype(p)>(p).data()...); }, *this);
  }

  constexpr const_pointer data() const noexcept {
    return std::apply([&](auto&... p) { return std::forward_as_tuple(std::forward<decltype(p)>(p).data()...); }, *this);
  }

  void move(std::vector<Attributes>&&... attrs) {
    std::apply([&](auto&&... vs) { (vs.swap(attrs), ...); }, *this);
  }

  void move(std::tuple<std::vector<Attributes>...>&& attrs) {
    std::apply([&](Attributes&&... attr) { move(attr...); }, attrs);
  }

  void copy(const std::vector<Attributes>&... attrs) {
    std::apply([&](auto&... vs) { (std::copy(attrs.begin(), attrs.end(), vs.begin()), ...); }, *this);
  }

  void copy(const std::tuple<std::vector<Attributes>...>& attrs) {
    std::apply([&](const std::vector<Attributes>&... attr) { copy(attr...); }, attrs);
  }

  void push_back(const Attributes&... attrs) {
    std::apply([&](auto&... vs) { (vs.push_back(attrs), ...); }, *this);
  }

  //  void push_back(Attributes&... attrs) {
  //std::apply([&](auto&... vs) { (vs.push_back(attrs), ...); }, *this);
  //  }

  void push_back(const std::tuple<Attributes...>& attrs) {
    std::apply([&](auto&... attr) { push_back(attr...); }, attrs);
  }

  void push_at(std::size_t i, Attributes... attrs) {
    std::apply([&](auto&... vs) { ((vs[i] = attrs), ...); }, *this);
  }

  void push_at(std::size_t i, std::tuple<Attributes...> attrs) {
    std::apply([&](Attributes... attr) { push_at(i, attr...); }, attrs);
  }

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

  void serialize(const std::string& outfile_name) {
    std::ofstream out_file(outfile_name, std::ofstream::binary);
    serialize(out_file);
  }

  void deserialize(std::istream& infile) {
    size_t st_size = -1;
    size_t el_size = -1;
    infile.read(reinterpret_cast<char*>(&st_size), sizeof(size_t));
    infile.read(reinterpret_cast<char*>(&el_size), sizeof(size_t));
    resize(st_size);
    std::apply([&](auto&... vs) { (deserialize(infile, vs), ...); }, *this);
  }

  void deserialize(const std::string& infile_name) {
    std::ifstream infile(infile_name, std::ifstream::binary);
    deserialize(infile);
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

  [[nodiscard]] size_t size() const {
    return std::get<0>(*this).size();
  }

  bool operator==(struct_of_arrays& a) {
    return std::equal(std::execution::par, begin(), end(), a.begin());
  }

  bool operator!=(const storage_type& a) {
    return !operator==(a);
  }
};

}    // namespace nw::graph


namespace std {
template <class... Attributes>
class tuple_size<nw::graph::struct_of_arrays<Attributes...>> : public std::integral_constant<std::size_t, sizeof...(Attributes)> {};

}    // namespace std

/// NB: technically we're supposed to be using `iter_swap` here on the
/// struct_of_array iterator type, but I can't figure out how to do this.

#include "nwgraph/util/tuple_hack.hpp"


#endif    // NW_GRAPH_SOA_HPP
