/**
 * @file reverse.hpp
 * @brief Range adaptor for traversing paths in reverse order.
 *
 * Provides a range-based interface for iterating over a shortest path
 * from destination back to source, following predecessor pointers.
 * Useful for reconstructing paths after running Dijkstra or BFS.
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

#ifndef NW_GRAPH_REVERSE_HPP
#define NW_GRAPH_REVERSE_HPP

#include "nwgraph/util/util.hpp"
#include <concepts>
#include <vector>

namespace nw {
namespace graph {

/**
 * @brief Range adaptor for reverse path traversal.
 * @tparam path_edge The edge type stored in the path (must have predecessor field).
 * @tparam vertex_id The vertex identifier type.
 *
 * Given a predecessor array from shortest path computation, iterates from
 * a start vertex back to a stop vertex following predecessor pointers.
 */
template <typename path_edge, std::integral vertex_id>
class reverse_path {
public:
  /**
   * @brief Construct a reverse path iterator.
   * @param path The predecessor array from shortest path computation.
   * @param start The starting vertex (destination of original path).
   * @param stop The ending vertex (source of original path).
   */
  reverse_path(std::vector<path_edge>& path, vertex_id start, vertex_id stop) : path_(path), start_(start), stop_(stop) {
  }

  using vertex_id_type = vertex_id;
  std::vector<path_edge> path_;
  double                 update_;
  vertex_id_type         start_, stop_;
  bool                   done = false;

  bool found(vertex_id_type current) {
    return current == stop_;
  }

  class reverse_iterator {
  private:
    reverse_path&  rev_;
    vertex_id_type curr_;

  public:
    reverse_iterator(reverse_path& rev) : rev_(rev), curr_(rev.start_) {
    }

    reverse_iterator& operator++() {
      curr_ = rev_.path_[curr_].predecessor;
      if (rev_.found(curr_)) {
        rev_.done = true;
      }

      return *this;
    }

    auto operator*() {
      return std::tuple<vertex_id_type>(curr_);
    }

    class end_sentinel_type {
    public:
      end_sentinel_type() {
      }
    };

    auto operator==(const end_sentinel_type&) const {
      return rev_.done;
    }
    bool operator!=(const end_sentinel_type&) const {
      return !rev_.done;
    }
  };

  auto begin() {
    return reverse_iterator(*this);
  }
  auto end() {
    return typename reverse_iterator::end_sentinel_type();
  }
};

}    // namespace graph
}    // namespace nw
#endif    // NW_GRAPH_REVERSE_HPP
