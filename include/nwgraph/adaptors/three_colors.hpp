/**
 * @file three_colors.hpp
 * @brief Vertex coloring shared by the BFS/DFS range adaptors.
 *
 * Factored out of bfs_edge_range.hpp and new_dfs_range.hpp so those adaptor
 * headers can coexist in one translation unit. Both previously defined an
 * identical `enum three_colors` at namespace scope, which is an ODR clash when
 * both headers are included together.
 */

#ifndef NW_GRAPH_THREE_COLORS_HPP
#define NW_GRAPH_THREE_COLORS_HPP

namespace nw {
namespace graph {

/// @brief Vertex coloring for BFS/DFS traversal.
enum three_colors { black, white, grey };

}    // namespace graph
}    // namespace nw

#endif    // NW_GRAPH_THREE_COLORS_HPP
