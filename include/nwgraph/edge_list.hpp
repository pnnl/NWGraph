/**
 * @file edge_list.hpp
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

#ifndef NW_GRAPH_EDGE_LIST_HPP
#define NW_GRAPH_EDGE_LIST_HPP

#include "nwgraph/containers/soa.hpp"
#include "nwgraph/graph_base.hpp"
#include "nwgraph/graph_concepts.hpp"
#include "nwgraph/graph_traits.hpp"

#if defined(CL_SYCL_LANGUAGE_VERSION)
#include <dpstd/algorithm>
#include <dpstd/execution>
#include <dpstd/numeric>
#else
#include <execution>
#include <tbb/parallel_sort.h>
#endif

#include "nwgraph/util/defaults.hpp"
#include "nwgraph/util/demangle.hpp"
#include "util/timer.hpp"

#include <algorithm>
#include <concepts>
#include <limits>
#include <numeric>
#include <tuple>
#include <type_traits>


namespace nw {
namespace graph {

static bool g_debug_edge_list = false;
static bool g_time_edge_list  = false;

void debug_edge_list(bool flag = true) { g_debug_edge_list = flag; }
void time_edge_list(bool flag = true) { g_time_edge_list = flag; }

/**
 * Index edge list structure.  This variadic data structure stores edges with their
 * properties, using a structure of arrays for storage.
 *
 * An edge in this case is notionally a tuple (u, v, p0, p1, ...), where u and v are
 * vertex indices and p0, p1, ... are edge properties.  If the * directedness tag is set
 * to `directed`, the edges are directed.  If the directedness * tag is set to
 * `undirected`, the edges are undirected.  In either case, a given edge * (u, v, ...) is
 * only stored once in the edge list.
 *
 * @tparam vertex_id The data type used to represent a vertex index, required to be os unsigned integral type.
 * @tparam graph_base_t The class of graph represented by the edge list.  May be unipartite or bipartite.
 * @tparam directed The directness of the tag.  May be directed or undirected.
 * @tparam Attributes A variadic list of edge property types.
 */
template <std::unsigned_integral vertex_id, typename graph_base_t, directedness direct = directedness::undirected, typename... Attributes>
class index_edge_list : public graph_base_t, public struct_of_arrays<vertex_id, vertex_id, Attributes...> {

public:
  // private:
  using graph_base     = graph_base_t;
  using vertex_id_type = vertex_id;
  using base           = struct_of_arrays<vertex_id_type, vertex_id_type, Attributes...>;
  using element        = std::tuple<vertex_id_type, vertex_id_type, Attributes...>;
  using reference      = base::reference;


  static const directedness edge_directedness = direct;
  using attributes_t                          = std::tuple<Attributes...>;
  using num_vertices_type                     = typename graph_base::vertex_cardinality_t;
  using num_edges_type                        = typename base::difference_type;

  using my_type         = index_edge_list<vertex_id_type, graph_base_t, direct, Attributes...>;
  using directed_type   = index_edge_list<vertex_id_type, graph_base_t, directedness::directed, Attributes...>;
  using undirected_type = index_edge_list<vertex_id_type, graph_base_t, directedness::undirected, Attributes...>;

public:
  constexpr index_edge_list(const index_edge_list&) = default;
  constexpr index_edge_list& operator=(const index_edge_list&) = default;
  constexpr index_edge_list(index_edge_list&&)                 = default;
  constexpr index_edge_list& operator=(index_edge_list&&) = default;

  index_edge_list(size_t N = 0) requires(std::is_same<graph_base, unipartite_graph_base>::value) : graph_base(N) { open_for_push_back(); }
  index_edge_list(size_t M = 0, size_t N = 0) requires(std::is_same<graph_base, bipartite_graph_base>::value) : graph_base(M, N) {
    open_for_push_back();
  }

  index_edge_list(std::initializer_list<element> l) {
    open_for_push_back();

    for_each(l.begin(), l.end(), [&](element x) { push_back(x); });

    if (g_debug_edge_list) {
      if constexpr (is_unipartite<graph_base>::value) {
        std::cout << " max " << graph_base::vertex_cardinality[0] << std::endl;
      } else {
        std::cout << " max " << graph_base::vertex_cardinality[0] << " " << graph_base::vertex_cardinality[1] << std::endl;
      }
    }

    close_for_push_back();

    if (g_debug_edge_list) {
      if constexpr (is_unipartite<graph_base>::value) {
        std::cout << " max " << graph_base::vertex_cardinality[0] << std::endl;
      } else {
        std::cout << " max " << graph_base::vertex_cardinality[0] << " " << graph_base::vertex_cardinality[1] << std::endl;
      }
    }
  }

  void open_for_push_back() { 
    graph_base::is_open = true; 
    graph_base::vertex_cardinality[0] = graph_base::vertex_cardinality[0];

    if constexpr (false == is_unipartite<graph_base>::value) {
      graph_base::vertex_cardinality[1] = graph_base::vertex_cardinality[1];
    }
  }

  void close_for_push_back() {
    graph_base::vertex_cardinality[0] = graph_base::vertex_cardinality[0];

    if constexpr (false == is_unipartite<graph_base>::value) {
      graph_base::vertex_cardinality[1] = graph_base::vertex_cardinality[1];
    }

    graph_base::is_open = false;
  }

  void push_back(vertex_id_type i, vertex_id_type j, Attributes... attrs) {
    assert(graph_base::is_open == true);

    if constexpr (is_unipartite<graph_base>::value) {
      graph_base::vertex_cardinality[0] = std::max<vertex_id_type>(std::max(i, j)+1, graph_base::vertex_cardinality[0]);
    } else {
      graph_base::vertex_cardinality[0] = std::max<vertex_id_type>(i+1, graph_base::vertex_cardinality[0]);
      graph_base::vertex_cardinality[1] = std::max<vertex_id_type>(j+1, graph_base::vertex_cardinality[1]);
    }

    base::push_back(i, j, attrs...);
  }

  void push_back(const element& elem) {
    vertex_id_type i = std::get<0>(elem);
    vertex_id_type j = std::get<1>(elem);

    if constexpr (is_unipartite<graph_base>::value) {
      graph_base::vertex_cardinality[0] = std::max<vertex_id_type>(std::max(i, j)+1, graph_base::vertex_cardinality[0]);
    } else {
      graph_base::vertex_cardinality[0] = std::max<vertex_id_type>(i + 1, graph_base::vertex_cardinality[0]);
      graph_base::vertex_cardinality[1] = std::max<vertex_id_type>(j + 1, graph_base::vertex_cardinality[1]);
    }

    base::push_back(elem);
  }

  size_t size() const { return base::size(); }
  // size_t length() const { return base::size(); }
  // auto max() const { return graph_base::vertex_cardinality; }


  /**
   * Return the number of edges in the edge list graph
   */
  auto num_edges() const {
    return base::size();
  }

  /**
   * Return the number of vertices in the edge list graph.  This will be the largest vertex id 
   * among the edges (either source or target) in the edge list 
   */
  auto num_vertices() const {
    return graph_base::vertex_cardinality;
  }

  constexpr static const char magic[27] = "NW Graph index_edge_list";

  /**
   * @brief Serialize the index_edge_list into binary. Can handle unipartite and bipartite graphs.
   * 
   * @param outfile The output ostream object.
   */
  void serialize(std::ostream& outfile) const {
    outfile.write(reinterpret_cast<const char*>(magic), sizeof(magic));
    directedness d = edge_directedness;
    outfile.write(reinterpret_cast<const char*>(&d), sizeof(d));
    graph_base::serialize(outfile);
    base::serialize(outfile);
  }

  void serialize(const std::string& filename) const {
    std::ofstream outfile(filename, std::ofstream::binary);
    serialize(outfile);
  }

  /**
   * @brief Deserialize the binary into index_edge_list object. Can handle unipartite and bipartite graphs.
   * 
   * @param infile The input istream object.
   */
  void deserialize(std::istream& infile) {
    char spell[sizeof(magic) + 1];
    infile.read(reinterpret_cast<char*>(spell), sizeof(magic));
    directedness d = edge_directedness;
    infile.read(reinterpret_cast<char*>(&d), sizeof(d));
    if (d != edge_directedness) {
      std::cout << std::string("Warning: Expected directedness ") + std::to_string((int)edge_directedness) + " but got " +
                       std::to_string((int)d)
                << std::endl;
    }
    graph_base::deserialize(infile);
    base::deserialize(infile);
    close_for_push_back();
  }

  void deserialize(const std::string& filename) {
    std::ifstream infile(filename, std::ifstream::binary);
    deserialize(infile);
  }

  void stream_stats(std::ostream& os = std::cout) const {
    int status = -4;
    std::cout << "% ";
    std::cout << nw::graph::demangle(typeid(*this).name(), nullptr, nullptr, &status) + ": " +
                     "graph_base::vertex_cardinality = " + std::to_string(graph_base::vertex_cardinality[0]) + " ";
    if constexpr (false == is_unipartite<graph_base>::value) {
      std::cout << std::to_string(graph_base::vertex_cardinality[1]) + " ";
    }
    std::cout << std::string("base::size() = ") + std::to_string(base::size());
    std::cout << std::endl;
  }

  void stream_edges(std::ostream& os = std::cout) {
    std::for_each(base::begin(), base::end(), [](auto&& x) {
      std::apply([&](auto&... field) { ((std::cout << field << " "), ...); }, x);
      std::cout << std::endl;
    });
  }

  void stream(std::ostream& os = std::cout) { stream_edges(os); }

  bool operator==(index_edge_list<vertex_id_type, graph_base_t, edge_directedness, Attributes...>& e) {
    return graph_base::vertex_cardinality == e.graph_base::vertex_cardinality && base::operator==(e);    //*this == e;
  }

  bool operator!=(index_edge_list<vertex_id_type, graph_base_t, edge_directedness, Attributes...>& e) { return !operator==(e); }
};

/**
 * Type alias for unipartite index edge list structure, using the default_vertex_id_type as the vertex_id type.
 * See @verbatim embed:rst:inline :cpp:class:`index_edge_list` @endverbatim .
 */
template <directedness edge_directedness = directedness::undirected, typename... Attributes>
using edge_list = index_edge_list<default_vertex_id_type, unipartite_graph_base, edge_directedness, Attributes...>;

/**
 * Type alias for bipartite index edge list structure, using the default_vertex_id_type as the vertex_id type.
 */
template <directedness edge_directedness = directedness::directed, typename... Attributes>
using bi_edge_list = index_edge_list<default_vertex_id_type, bipartite_graph_base, edge_directedness, Attributes...>;

template <std::unsigned_integral vertex_id, typename graph_base_t, directedness direct = directedness::undirected, typename... Attributes>
auto tag_invoke(const num_edges_tag, const index_edge_list<vertex_id, graph_base_t, direct, Attributes...>& b) {
  return b.num_edges();
}

// num_vertics CPO, works for both unipartite_graph_base and bipartite_graph_base
template <std::unsigned_integral vertex_id, typename graph_base_t, directedness direct = directedness::undirected, typename... Attributes>
auto tag_invoke(const num_vertices_tag, const index_edge_list<vertex_id, graph_base_t, direct, Attributes...>& b, int idx = 0) {
  if constexpr (true == is_unipartite<graph_base_t>::value)
    return b.num_vertices()[0]; //for unipartite graph ignore idx value
  else 
    return b.num_vertices()[idx];
}

template <std::unsigned_integral vertex_id, typename graph_base_t, directedness direct = directedness::undirected, typename... Attributes>
auto& tag_invoke(const source_tag, const index_edge_list<vertex_id, graph_base_t, direct, Attributes...>&, const typename index_edge_list<vertex_id, graph_base_t, direct, Attributes...>::reference e) { 
  return std::get<0>(e);
}

template <std::unsigned_integral vertex_id, typename graph_base_t, directedness direct = directedness::undirected, typename... Attributes>
auto& tag_invoke(const target_tag, const index_edge_list<vertex_id, graph_base_t, direct, Attributes...>&, const typename index_edge_list<vertex_id, graph_base_t, direct, Attributes...>::reference e) {
  return std::get<1>(e);
}

template <std::unsigned_integral vertex_id, typename graph_base_t, directedness direct, typename... Attributes>
struct graph_traits<index_edge_list<vertex_id, graph_base_t, direct, Attributes...>> {
  using G = index_edge_list<vertex_id, graph_base_t, direct, Attributes...>;

  using vertex_id_type    = typename G::vertex_id_type;
  using vertex_size_type  = typename G::vertex_id_type;
  using num_vertices_type = typename G::num_vertices_type;
  using num_edges_type    = typename G::num_edges_type;

  using outer_iterator = std::false_type;
  using inner_iterator = std::false_type;

  using const_outer_iterator = std::false_type;
  using const_inner_iterator = std::false_type;
};

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_EDGE_LIST_HPP
