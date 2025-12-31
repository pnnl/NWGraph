# BGL Book Examples
# These examples are based on the Boost Graph Library book by Siek, Lee, and Lumsdaine

# Chapter 3: File Dependencies (Topological Sort)
add_executable(ch3_toposort.exe ch3_toposort.cpp)
target_link_libraries(ch3_toposort.exe nwgraph)

# Chapter 4: Basic Graph Algorithms
add_executable(ch4_kevin_bacon.exe ch4_kevin_bacon.cpp)
target_link_libraries(ch4_kevin_bacon.exe nwgraph)

add_executable(ch4_loop_detection.exe ch4_loop_detection.cpp)
target_link_libraries(ch4_loop_detection.exe nwgraph)

# Chapter 5: Shortest Paths
add_executable(ch5_dijkstra.exe ch5_dijkstra.cpp)
target_link_libraries(ch5_dijkstra.exe nwgraph)

# Chapter 6: Minimum Spanning Tree
add_executable(ch6_kruskal.exe ch6_kruskal.cpp)
target_link_libraries(ch6_kruskal.exe nwgraph)

add_executable(ch6_prim.exe ch6_prim.cpp)
target_link_libraries(ch6_prim.exe nwgraph)

# Chapter 7: Connected Components
add_executable(ch7_connected.exe ch7_connected.cpp)
target_link_libraries(ch7_connected.exe nwgraph)

# Chapter 8: Maximum Flow
add_executable(ch8_maxflow.exe ch8_maxflow.cpp)
target_link_libraries(ch8_maxflow.exe nwgraph)
