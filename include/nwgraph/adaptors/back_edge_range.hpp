/**
 * @file back_edge_range.hpp
 *
 * @copyright SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
 * @copyright SPDX-FileCopyrightText: 2022 University of Washington
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * @authors
 *   Andrew Lumsdaine
 *   Tony Liu
 *
 */

#ifndef NW_GRAPH_BACK_EDGE_RANGE_HPP
#define NW_GRAPH_BACK_EDGE_RANGE_HPP

#include "nwgraph/graph_traits.hpp"
#include "nwgraph/util/util.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <tuple>

namespace nw {
namespace graph {

template <typename Graph>
class back_edge_range {
  /// Transform a reference to a tuple to a tuple of references.
  template <class Tuple>
  static constexpr decltype(auto) ref_tuple(Tuple&& tuple) {
    return std::apply([](auto&&... vals) { return std::forward_as_tuple(std::forward<decltype(vals)>(vals)...); }, std::forward<Tuple>(tuple));
  }

public:
  back_edge_range(Graph& graph, bool as_needed = false)
      : the_graph_(graph), back_address(graph.end() - graph.begin()), back_address_extra(graph.end() - graph.begin()),
        address_extra(graph.end() - graph.begin()) {
    if (!as_needed) {
      for (auto outer_it = graph.begin(); outer_it != graph.end(); ++outer_it) {
        for (auto inner = (*outer_it).begin(); inner != (*outer_it).end(); ++inner) {
          vertex_id_type neighbor = std::get<0>(*inner);
          auto           back_it  = graph[neighbor].begin();
          std::size_t end = outer_it - graph.begin();
          if (back_address[end].find(neighbor) != back_address[end].end()) {
            continue;
          }
          for (; back_it != graph[neighbor].end() && std::get<0>(*back_it) != end; ++back_it)
            ;
          if (back_it != graph[neighbor].end()) {
            if (end < std::get<0>(*inner)) {
              back_address[end].insert_or_assign(std::get<0>(*inner), *back_it);
              back_address[std::get<0>(*inner)].insert_or_assign(end, *inner);
            }
          } else {
            edge extra_edge;
            std::get<0>(extra_edge) = end;
            back_address_extra[end].emplace(std::get<0>(*inner), extra_edge);
            address_extra[std::get<0>(*inner)].emplace(end, extra_edge);
            back_address[std::get<0>(*inner)].emplace(end, *inner);
          }
        }
      }
    }
  }

  back_edge_range(const back_edge_range&)  = delete;
  back_edge_range(const back_edge_range&&) = delete;

  using edge           = typename std::iterator_traits<typename Graph::inner_iterator>::value_type;
  using vertex_id_type = vertex_id_t<Graph>;

  class back_edge_range_iterator {
  private:
    typename Graph::outer_iterator            G;
    vertex_id_type                            v_;
    typename Graph::inner_iterator            u_begin, u_end;
    typename std::map<size_t, edge>::iterator e_begin, e_end;

  public:
    back_edge_range_iterator(back_edge_range<Graph>& range, vertex_id_type v)
        : G(range.the_graph_.begin()), v_(v), u_begin(G[v_].begin()), u_end(G[v_].end()), e_begin(range.address_extra[v_].begin()),
          e_end(range.address_extra[v_].end()) {}

    back_edge_range_iterator& operator++() {
      if (u_begin != u_end) {
        ++u_begin;
      } else if (e_begin != e_end) {
        ++e_begin;
      }

      return *this;
    }

    auto operator*() {
      if (u_begin != u_end) {
        return *u_begin;
      }
      return ref_tuple(e_begin->second);
    }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };

    auto operator==(const end_sentinel_type&) const { return u_begin == u_end && e_begin == e_end; }
    bool operator!=(const end_sentinel_type&) const { return u_begin != u_end || e_begin != e_end; }
    auto operator==(const back_edge_range_iterator& b) const { return u_begin == b.u_begin && e_begin == b.e_begin; }
    auto operator!=(const back_edge_range_iterator& b) const { return u_begin != b.u_begin || e_begin != b.e_begin; }
  };

public:
  class sub_view {
  public:
    sub_view(back_edge_range<Graph>& range, vertex_id_type v) : the_range_(range), v_(v) {}

    auto begin() { return back_edge_range_iterator(the_range_, v_); }
    auto end() { return typename back_edge_range_iterator::end_sentinel_type(); }

  private:
    back_edge_range<Graph>& the_range_;
    vertex_id_type          v_;
  };

  class outer_back_edge_range_iterator {
  public:
    back_edge_range<Graph>&        the_range_;
    typename Graph::outer_iterator G;

  public:
    outer_back_edge_range_iterator(back_edge_range<Graph>& range, typename Graph::outer_iterator outer) : the_range_(range), G(outer) {}

    auto& operator=(const outer_back_edge_range_iterator& b) {
      G = b.G;
      the_range_ = b.the_range_;
      return *this;
    }

    auto& operator++() {
      ++G;
      return *this;
    }
    auto operator-(const outer_back_edge_range_iterator& b) const { return G - b.G; }

    bool operator==(const outer_back_edge_range_iterator& b) const { return G == b.G; }
    bool operator!=(const outer_back_edge_range_iterator& b) const { return G != b.G; }
    bool operator<(const outer_back_edge_range_iterator& b) const { return G < b.G; }

    auto operator*() { return sub_view(the_range_, get_index()); }

    class end_sentinel_type {
    public:
      end_sentinel_type() {}
    };
    auto operator==(const end_sentinel_type&) const { return G == the_range_.the_graph_.end(); }
    bool operator!=(const end_sentinel_type&) const { return G != the_range_.the_graph_.end(); }

    auto  operator[](size_t i) { return sub_view(the_range_, i); }
    auto& operator[](size_t i) const { return sub_view(the_range_, i); }

    auto get_index() { return G - the_range_.the_graph_.begin(); }
  };

  using reference = typename std::iterator_traits<typename Graph::inner_iterator>::reference;
  auto get_back_edge(typename Graph::outer_iterator& outer, typename Graph::inner_iterator& inner) {
    auto vtx  = outer - the_graph_.begin();
    auto vtx2 = std::get<0>(*inner);
    return get_back_edge(vtx, vtx2);
  }
  auto get_back_edge(vertex_id_type vtx, back_edge_range_iterator& inner) {
    vertex_id_type vtx2 = std::get<0>(*inner);
    return get_back_edge(vtx, vtx2);
  }

  auto get_back_edge(vertex_id_type vtx, vertex_id_type vtx2) {
    auto it = back_address[vtx].find(vtx2);
    if (it != back_address[vtx].end()) return it->second;

    auto it2 = back_address_extra[vtx].find(vtx2);
    if (it2 != back_address_extra[vtx].end()) {
      return ref_tuple(it2->second);
    }

    for (auto it = the_graph_[vtx].begin(); it != the_graph_[vtx].end(); ++it) {
      vertex_id_type neighbor = std::get<0>(*it);
      if (neighbor != vtx2) {
        continue;
      }
      auto back_it = the_graph_[neighbor].begin();
      for (; back_it != the_graph_[neighbor].end() && std::get<0>(*back_it) != vtx; ++back_it)
        ;
      if (back_it != the_graph_[neighbor].end()) {
        back_address[vtx2].insert_or_assign(vtx, *it);
        back_address[vtx].insert_or_assign(vtx2, *back_it);
      } else {
        edge extra_edge;
        std::get<0>(extra_edge) = vtx;
        back_address_extra[vtx].emplace(vtx2, extra_edge);
        address_extra[vtx2].emplace(vtx, extra_edge);
        back_address[vtx2].emplace(vtx, *it);
      }
      break;
    }

    it = back_address[vtx].find(vtx2);
    if (it != back_address[vtx].end()) return it->second;

    it2 = back_address_extra[vtx].find(vtx2);
    return ref_tuple(it2->second);
  }

  using outer_iterator = outer_back_edge_range_iterator;
  using inner_iterator = back_edge_range_iterator;
  auto begin() { return outer_back_edge_range_iterator(*this, the_graph_.begin()); }
  auto end() { return outer_back_edge_range_iterator(*this, the_graph_.end()); }
  auto size() { return the_graph_.size(); }

private:
  Graph&                                   the_graph_;
  std::vector<std::map<size_t, reference>> back_address;
  std::vector<std::map<size_t, edge>>      back_address_extra;
  std::vector<std::map<size_t, edge>>      address_extra;
};

}    // namespace graph
}    // namespace nw
#endif    //  NW_GRAPH_BACK_EDGE_RANGE_HPP
