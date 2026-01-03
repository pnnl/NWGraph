# BGL Book Examples
# These examples are based on the Boost Graph Library book by Siek, Lee, and Lumsdaine

# Chapter 3: File Dependencies (Topological Sort)
add_executable(ch3_toposort ch3_toposort.cpp)
target_link_libraries(ch3_toposort nwgraph)

# Chapter 4: Basic Graph Algorithms
add_executable(ch4_kevin_bacon ch4_kevin_bacon.cpp)
target_link_libraries(ch4_kevin_bacon nwgraph)

add_executable(ch4_loop_detection ch4_loop_detection.cpp)
target_link_libraries(ch4_loop_detection nwgraph)

# Chapter 5: Shortest Paths
add_executable(ch5_dijkstra ch5_dijkstra.cpp)
target_link_libraries(ch5_dijkstra nwgraph)

add_executable(ch5_bellman_ford ch5_bellman_ford.cpp)
target_link_libraries(ch5_bellman_ford nwgraph)

# Chapter 6: Minimum Spanning Tree
add_executable(ch6_kruskal ch6_kruskal.cpp)
target_link_libraries(ch6_kruskal nwgraph)

add_executable(ch6_prim ch6_prim.cpp)
target_link_libraries(ch6_prim nwgraph)

# Chapter 7: Connected Components
add_executable(ch7_connected ch7_connected.cpp)
target_link_libraries(ch7_connected nwgraph)

add_executable(ch7_strongly_connected ch7_strongly_connected.cpp)
target_link_libraries(ch7_strongly_connected nwgraph)

# Chapter 8: Maximum Flow
add_executable(ch8_maxflow ch8_maxflow.cpp)
target_link_libraries(ch8_maxflow nwgraph)

# Chapter 9: Implicit Graphs (Knight's Tour)
add_executable(ch9_knights_tour ch9_knights_tour.cpp)
target_link_libraries(ch9_knights_tour nwgraph)
