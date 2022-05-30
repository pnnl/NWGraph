
#include <forward_list>
#include <list>
#include <nwgraph/build.hpp>
#include <nwgraph/edge_list.hpp>
#include <nwgraph/graph_adaptor.hpp>
#include <vector>


int main() {

  nw::graph::graph_adaptor<std::vector<std::vector<int>>> u(0);
  nw::graph::graph_adaptor<std::vector<std::vector<int>>> v(10);

  nw::graph::graph_adaptor<std::vector<std::vector<int>>> w(10);

  nw::graph::edge_list<nw::graph::directedness::directed, int> e;

  fill_undirected<0>(e, 10, w);
}
