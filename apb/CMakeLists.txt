
add_executable(plain.exe plain.cpp)
target_link_Libraries(plain.exe nwgraph)

add_executable(spmv.exe spmv.cpp)
target_link_Libraries(spmv.exe nwgraph)

add_executable(exec.exe exec.cpp)
target_link_Libraries(exec.exe nwgraph)

add_executable(tbb.exe tbb.cpp)
target_link_Libraries(tbb.exe nwgraph)

add_executable(bfs_apb.exe bfs.cpp)
target_link_Libraries(bfs_apb.exe nwgraph)

add_executable(dijkstra.exe dijkstra.cpp)
target_link_Libraries(dijkstra.exe nwgraph)

add_executable(dfs.exe dfs.cpp)
target_link_Libraries(dfs.exe nwgraph)

add_executable(containers.exe containers.cpp)
target_link_Libraries(containers.exe nwgraph docopt)
