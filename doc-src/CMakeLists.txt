# =============================================================================
# NWGraph Documentation Build
# =============================================================================
# This CMakeLists.txt provides targets for building the NWGraph documentation.
#
# Targets:
#   docs      - Build complete documentation (Doxygen + Sphinx HTML)
#   docs-html - Build Sphinx HTML documentation only
#   docs-clean - Clean built documentation
#
# Requirements:
#   - Python 3 with sphinx, breathe, exhale, sphinx_rtd_theme
#   - Doxygen (for API documentation)
#
# Usage:
#   cmake .. -DNWGRAPH_BUILD_DOCS=ON
#   make docs
# =============================================================================

# Find required programs
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_program(SPHINX_BUILD sphinx-build
    HINTS ${Python3_EXECUTABLE}/../
    DOC "Path to sphinx-build executable")

find_program(DOXYGEN_EXECUTABLE doxygen
    DOC "Path to doxygen executable")

# Check if we can build documentation
if(NOT SPHINX_BUILD)
    message(WARNING "sphinx-build not found. Documentation cannot be built.
    Install with: pip install sphinx sphinx_rtd_theme breathe exhale sphinxcontrib-bibtex")
    return()
endif()

if(NOT DOXYGEN_EXECUTABLE)
    message(WARNING "doxygen not found. API documentation will be incomplete.
    Install doxygen from https://www.doxygen.nl/ or via your package manager.")
endif()

# Documentation source and build directories
set(SPHINX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sphinx")
set(SPHINX_BUILD_DIR "${SPHINX_SOURCE_DIR}/_build")
set(SPHINX_HTML_DIR "${SPHINX_BUILD_DIR}/html")

# =============================================================================
# Documentation targets
# =============================================================================

# Main documentation target - builds everything
add_custom_target(docs
    COMMAND ${CMAKE_COMMAND} -E echo "Building NWGraph documentation..."
    COMMAND ${CMAKE_COMMAND} -E echo "Source: ${SPHINX_SOURCE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "Output: ${SPHINX_HTML_DIR}"
    COMMAND ${SPHINX_BUILD} -M html "${SPHINX_SOURCE_DIR}" "${SPHINX_BUILD_DIR}"
        -j auto
        -W --keep-going
    WORKING_DIRECTORY ${SPHINX_SOURCE_DIR}
    COMMENT "Building NWGraph documentation (Doxygen + Sphinx)"
    VERBATIM
)

# HTML-only target (faster, uses cached Doxygen output)
add_custom_target(docs-html
    COMMAND ${SPHINX_BUILD} -b html "${SPHINX_SOURCE_DIR}" "${SPHINX_HTML_DIR}"
        -j auto
    WORKING_DIRECTORY ${SPHINX_SOURCE_DIR}
    COMMENT "Building Sphinx HTML documentation"
    VERBATIM
)

# Clean documentation
add_custom_target(docs-clean
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning documentation build..."
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${SPHINX_BUILD_DIR}"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${SPHINX_SOURCE_DIR}/_api"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${SPHINX_SOURCE_DIR}/_doxygen"
    COMMENT "Cleaning built documentation"
    VERBATIM
)

# Open documentation in browser (convenience target)
if(APPLE)
    add_custom_target(docs-open
        COMMAND open "${SPHINX_HTML_DIR}/index.html"
        DEPENDS docs
        COMMENT "Opening documentation in browser"
    )
elseif(UNIX)
    add_custom_target(docs-open
        COMMAND xdg-open "${SPHINX_HTML_DIR}/index.html"
        DEPENDS docs
        COMMENT "Opening documentation in browser"
    )
endif()

# Print documentation info
message(STATUS "Documentation targets available:")
message(STATUS "  make docs       - Build complete documentation")
message(STATUS "  make docs-html  - Build HTML only (faster)")
message(STATUS "  make docs-clean - Clean built documentation")
if(APPLE OR UNIX)
    message(STATUS "  make docs-open  - Build and open in browser")
endif()
message(STATUS "Documentation will be built in: ${SPHINX_HTML_DIR}")
