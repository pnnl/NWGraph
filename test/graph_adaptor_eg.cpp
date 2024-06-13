
#include <forward_list>
#include <list>
#include <nwgraph/build.hpp>
#include <nwgraph/edge_list.hpp>
#include <nwgraph/graph_adaptor.hpp>
#include <nwgraph/graph_concepts.hpp>
#include <vector>


int main() {

  static_assert(nw::graph::min_idx_edge_list<std::vector<std::tuple<int, int>>>);
  static_assert(nw::graph::min_idx_edge_list<std::list<std::tuple<int, int, size_t, double>>>);
  static_assert(nw::graph::min_idx_edge_list<std::forward_list<std::tuple<int, int, float>>>);

  static_assert(nw::graph::edge_list_graph<nw::graph::graph_adaptor<std::forward_list<std::tuple<size_t, size_t, double>>>>);
  static_assert(nw::graph::edge_list_graph<nw::graph::graph_adaptor<std::list<std::tuple<size_t, size_t, float, double>>>>);
  static_assert(nw::graph::edge_list_graph<nw::graph::graph_adaptor<std::vector<std::tuple<size_t, size_t>>>>);

  static_assert(nw::graph::adjacency_list_graph<nw::graph::graph_adaptor<std::vector<std::forward_list<std::tuple<size_t, size_t, double>>>>>);
  static_assert(nw::graph::adjacency_list_graph<nw::graph::graph_adaptor<std::vector<std::list<std::tuple<size_t, size_t, float, double>>>>>);
  static_assert(nw::graph::adjacency_list_graph<nw::graph::graph_adaptor<std::vector<std::vector<std::tuple<size_t, size_t>>>>>);

  nw::graph::source(std::forward_list<std::tuple<int, int, float>>(), std::tuple<int, int, float>());
  nw::graph::target(std::list<std::tuple<int, int, size_t, double>>(), std::tuple<int, int, size_t, double>());
  nw::graph::target(std::vector<std::tuple<int, int>>(), std::tuple<int, int>());

  nw::graph::graph_adaptor<std::vector<std::vector<int>>>       t(0);
  nw::graph::graph_adaptor<std::vector<std::list<int>>>         u(10);
  nw::graph::graph_adaptor<std::vector<std::forward_list<int>>> v(10);

  nw::graph::edge_list<nw::graph::directedness::directed> e;
  nw::graph::graph_adaptor<std::vector<std::vector<int>>> w(10);
  push_back_fill(e, w, false, 0);

  nw::graph::edge_list<nw::graph::directedness::directed, int, double>                  f;
  nw::graph::graph_adaptor<std::vector<std::vector<std::tuple<unsigned, int, double>>>> x(10);
  push_back_fill(f, x, false, 0);

  nw::graph::edge_list<nw::graph::directedness::undirected, int, double>                g;
  nw::graph::graph_adaptor<std::vector<std::vector<std::tuple<unsigned, int, double>>>> y(10);
  push_back_fill(g, y, true, 1);

  nw::graph::edge_list<nw::graph::directedness::undirected, int, double>              h;
  nw::graph::graph_adaptor<std::vector<std::list<std::tuple<unsigned, int, double>>>> z(10);
  push_back_fill(h, z, true, 1);

  nw::graph::edge_list<nw::graph::directedness::undirected> i;
  nw::graph::graph_adaptor<std::vector<std::list<size_t>>>  a(10);
  push_back_fill(i, a, true, 1);

  nw::graph::edge_list<nw::graph::directedness::undirected>        j;
  nw::graph::graph_adaptor<std::vector<std::forward_list<size_t>>> b(10);
  //  push_back_fill(j, b, true, 1);

  nw::graph::graph_adaptor<std::vector<std::tuple<int, int>>>               k;
  nw::graph::graph_adaptor<std::list<std::tuple<int, int, size_t, double>>> l;
  nw::graph::graph_adaptor<std::forward_list<std::tuple<int, int, float>>>  m;

  nw::graph::graph_adaptor<std::vector<std::list<int>>>                             c(0);
  nw::graph::graph_adaptor<std::vector<std::list<std::tuple<int>>>>                 cc(0);
  nw::graph::graph_adaptor<std::vector<std::list<std::tuple<int, size_t, double>>>> d(10);
  nw::graph::graph_adaptor<std::vector<std::list<std::tuple<int, float>>>>          ee(10);

  push_back_fill(k, c, true, 1);
}
