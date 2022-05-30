
# find_package
# fetch_content
# external project add


include(FetchContent)

# set(XTENSOR_USE_XSIMD 1)
set(XTENSOR_USE_TBB 1)

# xsimd and TBB dependencies are automatically
# searched when the following is executed

find_package(xtensor QUIET)

if (NOT xtensor_FOUND)
  message("Installed xtensor not found, using cmake config fetched version")
  FetchContent_Declare(
    xtl
    GIT_REPOSITORY https://github.com/xtensor-stack/xtl
  )

  FetchContent_GetProperties(xtl)
  if(NOT xtl_POPULATED)
    message("  Fetching xtl...")

    FetchContent_Populate(xtl)
    add_subdirectory(${xtl_SOURCE_DIR} ${xtl_BINARY_DIR})
  endif()

  FetchContent_Declare(xtensor
    GIT_REPOSITORY https://github.com/xtensor-stack/xtensor
  )
  FetchContent_GetProperties(xtensor)
  if(NOT xtensor_POPULATED)
    message("  Fetching xtensor...")
    FetchContent_Populate(xtensor)
    add_subdirectory(${xtensor_SOURCE_DIR} ${xtensor_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
endif()


find_package(nlohmann_json 3.2.0 QUIET)
if (NOT nlohmann_json_FOUND)
  message("Installed json not found -- fetching")
  FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.7.3)

  FetchContent_GetProperties(json)
  if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
endif()


find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(nw_graph INTERFACE OpenMP::OpenMP_CXX)
endif()

# set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)

add_executable(imdb.exe imdb.cpp)
target_link_libraries(imdb.exe nw_graph xtensor)

add_executable(dns.exe dns.cpp)
target_link_libraries(dns.exe nw_graph xtensor)

add_executable(oracle.exe oracle.cpp)
target_link_libraries(oracle.exe nw_graph nlohmann_json::nlohmann_json)

add_executable(dblp.exe dblp.cpp)
target_link_libraries(dblp.exe nw_graph nlohmann_json::nlohmann_json)

add_executable(oracle+dblp.exe oracle+dblp.cpp)
target_link_libraries(oracle+dblp.exe nw_graph nlohmann_json::nlohmann_json)

