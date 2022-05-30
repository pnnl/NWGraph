### SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${OPT_FLAGS} -DTBB_USE_THREADING_TOOLS -g -debug inline-debug-info")

add_executable(plain plain.cpp)
target_link_Libraries(plain nw_graph)

add_executable(spmv spmv.cpp)
target_link_Libraries(spmv nw_graph)

add_executable(exec exec.cpp)
target_link_Libraries(exec nw_graph)

add_executable(tbb tbb.cpp)
target_link_Libraries(tbb nw_graph)

add_executable(bfs bfs.cpp)
target_link_Libraries(bfs nw_graph)

add_executable(dijkstra dijkstra.cpp)
target_link_Libraries(dijkstra nw_graph)

add_executable(dfs dfs.cpp)
target_link_Libraries(dfs nw_graph)

add_executable(containers containers.cpp)
target_link_Libraries(containers nw_graph docopt)
