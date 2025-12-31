/**
 * @file ch9_knights_tour.cpp
 *
 * @brief Implicit Graphs: A Knight's Tour (BGL Book Chapter 9)
 *
 * This example demonstrates solving the knight's tour problem using an
 * implicit graph representation. The knight's tour is finding a path for
 * a chess knight to visit every square on an n×n chessboard exactly once.
 *
 * Key concepts demonstrated:
 * - Implicit graph representation (no explicit adjacency list)
 * - Backtracking search algorithm
 * - Warnsdorff's heuristic for faster solution
 *
 * The knight's tour is an example of a Hamiltonian path problem, which is
 * NP-complete. Warnsdorff's heuristic makes it tractable for small boards.
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

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <queue>
#include <stack>
#include <vector>

/**
 * @brief Position on the chessboard
 */
struct Position {
  int row, col;

  Position() : row(0), col(0) {}
  Position(int r, int c) : row(r), col(c) {}

  Position operator+(const Position& other) const { return Position(row + other.row, col + other.col); }

  bool operator==(const Position& other) const { return row == other.row && col == other.col; }
};

/**
 * @brief The 8 possible knight jumps
 */
const Position knight_jumps[8] = {Position(2, -1), Position(1, -2),  Position(-1, -2), Position(-2, -1),
                                  Position(-2, 1), Position(-1, 2), Position(1, 2),   Position(2, 1)};

/**
 * @brief Implicit graph representation of knight's moves on a chessboard
 *
 * This class models the knight's possible moves as a graph without
 * explicitly storing the adjacency list. The graph is computed on-the-fly.
 */
class KnightsTourGraph {
public:
  KnightsTourGraph(int n) : board_size(n) {}

  int size() const { return board_size; }

  /**
   * @brief Check if a position is valid (on the board)
   */
  bool is_valid(const Position& p) const { return p.row >= 0 && p.row < board_size && p.col >= 0 && p.col < board_size; }

  /**
   * @brief Convert position to linear index
   */
  int to_index(const Position& p) const { return p.row * board_size + p.col; }

  /**
   * @brief Convert linear index to position
   */
  Position to_position(int idx) const { return Position(idx / board_size, idx % board_size); }

  /**
   * @brief Get all valid knight moves from a position
   */
  std::vector<Position> adjacent_vertices(const Position& p) const {
    std::vector<Position> neighbors;
    for (int i = 0; i < 8; ++i) {
      Position next = p + knight_jumps[i];
      if (is_valid(next)) {
        neighbors.push_back(next);
      }
    }
    return neighbors;
  }

  int num_vertices() const { return board_size * board_size; }

private:
  int board_size;
};

/**
 * @brief Backtracking search for knight's tour
 *
 * This is a brute-force DFS approach that tries all possible paths.
 * It's slow but guaranteed to find a solution if one exists.
 */
bool backtracking_search(KnightsTourGraph& g, Position start, std::vector<int>& visit_time) {
  int num_verts = g.num_vertices();

  // Initialize all positions as unvisited (-1)
  visit_time.assign(num_verts, -1);

  // Stack contains (timestamp, position) pairs
  std::stack<std::pair<int, Position>> S;
  S.push({0, start});

  while (!S.empty()) {
    auto [timestamp, pos] = S.top();
    int idx               = g.to_index(pos);

    // Record the visit time
    visit_time[idx] = timestamp;

    // Check if we've visited all vertices
    if (timestamp == num_verts - 1) {
      return true;  // Found a complete tour!
    }

    // Try to find an unvisited neighbor
    bool found_next = false;
    auto neighbors  = g.adjacent_vertices(pos);

    for (const auto& next : neighbors) {
      int next_idx = g.to_index(next);
      if (visit_time[next_idx] == -1) {
        S.push({timestamp + 1, next});
        found_next = true;
        break;  // DFS: go deep first
      }
    }

    if (!found_next) {
      // Dead end: backtrack
      visit_time[idx] = -1;
      S.pop();
    }
  }

  return false;  // No solution found
}

/**
 * @brief Count unvisited neighbors of a position
 */
int count_unvisited_neighbors(KnightsTourGraph& g, const Position& pos, const std::vector<int>& visit_time) {
  int count      = 0;
  auto neighbors = g.adjacent_vertices(pos);
  for (const auto& next : neighbors) {
    if (visit_time[g.to_index(next)] == -1) {
      ++count;
    }
  }
  return count;
}

/**
 * @brief Warnsdorff's heuristic for knight's tour
 *
 * The heuristic chooses the next square as the one with the fewest
 * onward moves. This greedy approach dramatically improves performance
 * by visiting constrained squares first, avoiding dead ends.
 */
bool warnsdorff_search(KnightsTourGraph& g, Position start, std::vector<int>& visit_time) {
  int num_verts = g.num_vertices();

  visit_time.assign(num_verts, -1);
  visit_time[g.to_index(start)] = 0;

  Position current  = start;
  int move_count    = 1;

  while (move_count < num_verts) {
    auto neighbors = g.adjacent_vertices(current);

    // Find the unvisited neighbor with minimum onward moves
    Position best_next;
    int min_degree = 9;  // More than maximum possible (8)
    bool found     = false;

    for (const auto& next : neighbors) {
      int next_idx = g.to_index(next);
      if (visit_time[next_idx] == -1) {
        int degree = count_unvisited_neighbors(g, next, visit_time);
        if (degree < min_degree) {
          min_degree = degree;
          best_next  = next;
          found      = true;
        }
      }
    }

    if (!found) {
      return false;  // Dead end (shouldn't happen with Warnsdorff on standard boards)
    }

    current                        = best_next;
    visit_time[g.to_index(current)] = move_count;
    ++move_count;
  }

  return true;
}

/**
 * @brief Print the chessboard with move numbers
 */
void print_board(const KnightsTourGraph& g, const std::vector<int>& visit_time) {
  int n = g.size();
  std::cout << "  ";
  for (int c = 0; c < n; ++c) {
    std::cout << std::setw(3) << c;
  }
  std::cout << std::endl;

  for (int r = 0; r < n; ++r) {
    std::cout << r << " ";
    for (int c = 0; c < n; ++c) {
      int idx = r * n + c;
      if (visit_time[idx] == -1) {
        std::cout << std::setw(3) << ".";
      } else {
        std::cout << std::setw(3) << visit_time[idx];
      }
    }
    std::cout << std::endl;
  }
}

int main() {
  std::cout << "=== Knight's Tour: Implicit Graphs ===" << std::endl;
  std::cout << "Based on BGL Book Chapter 9" << std::endl << std::endl;

  std::cout << "The knight's tour problem: find a path for a knight" << std::endl;
  std::cout << "to visit every square on an n×n chessboard exactly once." << std::endl;
  std::cout << std::endl;

  std::cout << "Knight moves in an 'L' pattern:" << std::endl;
  std::cout << "  (±2, ±1) or (±1, ±2)" << std::endl;
  std::cout << std::endl;

  // Small board example with Warnsdorff (backtracking is too slow even for 5x5)
  std::cout << "=== Small Board (5×5) with Warnsdorff ===" << std::endl;
  {
    KnightsTourGraph g(5);
    std::vector<int> visit_time;
    Position start(0, 0);

    std::cout << "Starting from position (0, 0)..." << std::endl;
    std::cout << "Note: Pure backtracking is O(8^25) for 5×5 - impractical!" << std::endl;
    std::cout << "Using Warnsdorff's heuristic instead..." << std::endl;

    bool found = warnsdorff_search(g, start, visit_time);

    if (found) {
      std::cout << "Found a knight's tour!" << std::endl;
      std::cout << std::endl;
      print_board(g, visit_time);
    } else {
      std::cout << "No tour found from this starting position." << std::endl;
    }
  }

  std::cout << std::endl;

  // Standard 8x8 board with Warnsdorff's heuristic
  std::cout << "=== Standard Board (8×8) with Warnsdorff's Heuristic ===" << std::endl;
  {
    KnightsTourGraph g(8);
    std::vector<int> visit_time;
    Position start(0, 0);

    std::cout << "Starting from position (0, 0)..." << std::endl;
    std::cout << "Using Warnsdorff's heuristic (choose square with fewest onward moves)..." << std::endl;

    bool found = warnsdorff_search(g, start, visit_time);

    if (found) {
      std::cout << "Found a knight's tour!" << std::endl;
      std::cout << std::endl;
      print_board(g, visit_time);
    } else {
      std::cout << "No tour found (unexpected with Warnsdorff)." << std::endl;
    }
  }

  std::cout << std::endl;

  // Demonstrate different starting positions
  std::cout << "=== Different Starting Positions (6×6 board) ===" << std::endl;
  {
    KnightsTourGraph g(6);
    std::vector<int> visit_time;

    std::vector<Position> starts = {Position(0, 0), Position(2, 2), Position(0, 1)};

    for (const auto& start : starts) {
      bool found = warnsdorff_search(g, start, visit_time);
      std::cout << "Start (" << start.row << "," << start.col << "): " << (found ? "Tour found" : "No tour") << std::endl;
    }
  }

  std::cout << std::endl;
  std::cout << "=== Algorithm Comparison ===" << std::endl;
  std::cout << std::endl;
  std::cout << "Backtracking: O(8^(n²)) worst case - explores all possible paths" << std::endl;
  std::cout << "Warnsdorff:   O(n²) greedy - visits constrained squares first" << std::endl;
  std::cout << std::endl;
  std::cout << "Note: This example uses an implicit graph - the adjacency structure" << std::endl;
  std::cout << "is computed on-the-fly from the knight's movement rules rather" << std::endl;
  std::cout << "than stored explicitly in an adjacency list." << std::endl;

  return 0;
}
