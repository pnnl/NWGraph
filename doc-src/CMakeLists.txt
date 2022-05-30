#
#
#
find_package(Doxygen REQUIRED)
find_package(Sphinx REQUIRED)

# Find all the public headers
get_target_property(NW_GRAPH_PUBLIC_HEADER_DIR nw_graph INTERFACE_INCLUDE_DIRECTORIES)

file(GLOB_RECURSE NW_GRAPH_PUBLIC_HEADERS ${NW_GRAPH_PUBLIC_HEADER_DIR}/nwgraph/*.hpp)

message("NW_GRAPH_PUBLIC_HEADER_DIR ${NW_GRAPH_PUBLIC_HEADER_DIR}")
message("NW_GRAPH_PUBLIC_HEADERS ${NW_GRAPH_PUBLIC_HEADERS}")

message("PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
file(GLOB_RECURSE NW_GRAPH_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/include/nwgraph *.hpp)

message("NW_GRAPH_PUBLIC_HEADERS ${NW_GRAPH_PUBLIC_HEADERS}")

set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/include/nwgraph)

# set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
# set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)

set(DOXYGEN_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/docs/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)

set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

# Replace variables inside @@ with the current values
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR}) #Doxygen won't create this for us
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${NW_GRAPH_PUBLIC_HEADERS}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                   COMMENT "Generating docs")

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})


set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/sphinx)
set(SPHINX_BUILD ${PROJECT_SOURCE_DIR}/docs)
set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

# Only regenerate Sphinx when:
# - Doxygen has rerun
# - Our doc files have been updated
# - The Sphinx config has been updated
add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
                   COMMAND
                     ${SPHINX_EXECUTABLE} -b html
                     # Tell Breathe where to find the Doxygen output
                     -Dbreathe_projects.CatCutifier=${DOXYGEN_OUTPUT_DIR}/xml
                   ${SPHINX_SOURCE} ${SPHINX_BUILD}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   DEPENDS
                   # Other docs files you want to track should go here (or in some variable)
                   ${SPHINX_SOURCE}/index.rst
                   ${DOXYGEN_INDEX_FILE}
                   MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
                   COMMENT "Generating documentation with Sphinx")

# Nice named target so we can run the job easily
add_custom_target(Sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})

# Add an install target to install the docs
include(GNUInstallDirs)
install(DIRECTORY ${SPHINX_BUILD}
DESTINATION ${CMAKE_INSTALL_DOCDIR})
