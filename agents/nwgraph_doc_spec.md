# NWGraph Documentation Specification

## Overview

NWGraph uses Sphinx with reStructuredText for documentation, integrated with Doxygen via Breathe for API reference generation. The documentation is designed for GitHub Pages deployment.

## Documentation Tools

### Primary Tools

| Tool | Version | Purpose |
|------|---------|---------|
| Sphinx | Current | Documentation generator |
| Doxygen | - | C++ API extraction |
| Breathe | - | Doxygen-Sphinx bridge |
| nw_exhale | Custom | Automated API doc generation |

### Sphinx Extensions

- `sphinx.ext.mathjax` - Mathematical equation rendering
- `sphinx.ext.intersphinx` - Cross-project linking
- `sphinx.ext.viewcode` - Source code viewing
- `sphinx.ext.graphviz` - Graph visualization
- `sphinxcontrib.bibtex` - Bibliography management
- `breathe` - Doxygen integration

### Output Theme

- Primary: `sphinx_rtd_theme` (Read the Docs)
- Alternative: `sphinx-book-theme`
- Custom CSS: `_static/css/custom.css`

## Directory Structure

```
doc-src/sphinx/
├── conf.py                    # Sphinx configuration
├── index.rst                  # Documentation root
├── README.rst                 # Build instructions
├── Makefile                   # Build automation
├── requirements.txt           # Python dependencies
│
├── userguide/                 # User-facing documentation
│   ├── introduction.rst       # Library introduction
│   ├── getting_started.rst    # Quick start guide
│   ├── graph-background-ii.rst# Graph theory fundamentals
│   ├── generic-programming.rst# Generic programming concepts
│   ├── graph-concepts.rst     # NWGraph concept taxonomy
│   ├── algorithms.rst         # Algorithm reference
│   ├── adaptors.rst           # Range adaptors reference
│   ├── data-structures.rst    # Data structure docs
│   ├── performance.rst        # Performance evaluation
│   ├── related.rst            # Related work
│   ├── references.rst         # Bibliography
│   ├── conclusion.rst         # Concluding remarks
│   ├── footnote.rst           # Footnotes
│   ├── refs.bib               # Bibliography references
│   └── archive/               # Archived docs with pedagogical figures
│
├── examples/                  # Example code documentation
│   ├── index.rst              # Examples index
│   ├── degrees/               # Six Degrees example
│   └── bglbook/               # Boost Graph examples
│
├── nwgraph_api.rst            # API reference index
│
├── _extensions/               # Custom Sphinx extensions
│   ├── hydeme.py              # API documentation helper
│   └── nw_exhale/             # Custom Exhale extension
│
├── _scripts/                  # Build scripts
│   ├── generate_hyde.sh       # Hyde-based API generation
│   └── indexify.bash          # Index generation
│
├── _static/                   # Static assets
│   ├── css/                   # Stylesheets
│   └── images/                # Diagrams (PDFs, SVGs)
│
├── _templates/                # Custom templates
│   └── layout.html            # HTML layout
│
└── _themes/                   # Custom themes

docs/                          # Generated output (GitHub Pages)
└── .nojekyll                  # Disable Jekyll processing
```

## User Guide Structure

The user guide follows a logical progression:

| Order | Document | Content |
|-------|----------|---------|
| 1 | introduction.rst | Design philosophy, library overview |
| 2 | getting_started.rst | Installation, CMake, compilation |
| 3 | graph-background-ii.rst | Graph theory foundations |
| 4 | generic-programming.rst | C++20 concepts, STL patterns |
| 5 | graph-concepts.rst | NWGraph concept taxonomy |
| 6 | algorithms.rst | Algorithm specifications |
| 7 | adaptors.rst | Range adaptor reference |
| 8 | data-structures.rst | Container documentation |
| 9 | performance.rst | Benchmarking results |
| 10 | related.rst | Comparison with other libraries |
| 11 | references.rst | Bibliography |

## API Documentation Generation

### Two-Phase Process

**Phase 1: Doxygen XML Generation**
- Input: C++ headers from `include/nwgraph/`
- Output: XML in `_doxygen/xml/`
- Configuration: Embedded in `conf.py`
- Features:
  - `CLANG_ASSISTED_PARSING = YES`
  - `CLANG_OPTIONS = -std=c++20`
  - `EXTRACT_ALL = NO`
  - `BUILTIN_STL_SUPPORT = YES`

**Phase 2: Sphinx/Breathe Integration**
- Converts XML to reStructuredText to HTML
- Creates hierarchical API browser
- Excludes internal implementation details

### API Coverage

- Concepts (graph, adjacency_list_graph, etc.)
- Algorithms (BFS, CC, PageRank, etc.)
- Data Structures (adjacency, edge_list, etc.)
- Range Adaptors (15+ types)
- Utilities (timer, sorting, iterators)
- Experimental components

## Example Code

### Documentation Examples

Located in `doc-src/sphinx/examples/`:

1. **Six Degrees of Separation** (`degrees/`)
   - Real-world data ingestion (JSON)
   - Graph construction from relational data
   - BFS for Bacon numbers

2. **Boost Graph Library Examples** (`bglbook/`)
   - NWGraph rewrites of BGL examples

### Executable Examples

Located in `examples/`:

1. **IMDB Examples** (`examples/imdb/`)
   - `main.cpp`, `imdb.cpp`, `dblp.cpp`, `oracle.cpp`

2. **BGL Book Examples** (`examples/bgl-book/`)
   - `ch4_kevin_bacon.cpp`
   - `ch5_dijkstra.cpp`
   - `ch7_connected.cpp`
   - `ch8_maxflow.cpp`

## README Files

| File | Purpose |
|------|---------|
| `README.md` | Main project overview, quick start |
| `README_VLDB.md` | PVLDB paper reproduction instructions |
| `LICENSE.rst` | BSD-3-Clause licensing |
| `DISCLAIMER.rst` | Government work disclaimers |
| `Contributors.rst` | Contributor list |
| `doc-src/sphinx/README.rst` | Documentation build instructions |

## Building Documentation

### Prerequisites

```
# See doc-src/sphinx/requirements.txt for pinned versions
sphinxcontrib-bibtex~=2.6.0
breathe~=4.36.0
exhale~=0.3.7
sphinx-rtd-theme~=3.0.0
sphinx-book-theme~=1.1.0
myst-parser~=4.0.0
jinja2-highlight~=0.6.0
sphinx-copybutton~=0.5.0
```

### Build Commands

```bash
cd doc-src/sphinx

# Build HTML documentation
make html

# Deploy to docs/ for GitHub Pages
make pages
```

### Docker-Based Build

```bash
docker run -it -v ${NWMATH_ROOT}:/home/nwmath/NWmath amath583/pages
```

## Documentation Standards

### File Format
- reStructuredText (.rst)
- SPDX license headers on all files

### Code Blocks
```rst
.. code-block:: cpp

   auto result = bfs(graph, source);
```

### Citations
- Bibtex file: `userguide/refs.bib`
- Inline: `:cite:`author2022``

### Mathematical Notation
- MathJax via `sphinx.ext.mathjax`
- Custom macros defined in `conf.py`

## Deployment

- Output: `/docs/` directory
- Platform: GitHub Pages
- `.nojekyll` file disables Jekyll processing
- URL: `https://pnnl.github.io/nwgraph/` (expected)

## Known Documentation Issues

### Content Gaps

1. **BGL Examples**: `examples/bglbook/index.rst` is minimal
2. **Experimental Components**: API section appears incomplete
3. **Performance Figures**: May reference missing PDFs
4. **Parallel Execution**: Limited TBB integration docs

### Build Issues

1. **Hyde Dependency**: External tool with hardcoded paths
2. **Docker Required**: Full build requires Docker
3. **Version Inconsistency**: CMake variable names differ between README.md and getting_started.rst

### Maintenance Issues

1. **Stale Version Numbers**: `conf.py` may have outdated version
2. **Broken References**: Some cross-references may be invalid

## Custom Extensions

### nw_exhale

Located in `_extensions/nw_exhale/`:

| File | Purpose |
|------|---------|
| `configs.py` | Configuration management |
| `graph.py` | Documentation graph structure |
| `deploy.py` | Deployment utilities |
| `parse.py` | Parsing logic |
| `utils.py` | Utility functions |
| `data/treeView/` | JavaScript tree view |

Customizes Exhale for C++20 API documentation with hierarchical navigation.
