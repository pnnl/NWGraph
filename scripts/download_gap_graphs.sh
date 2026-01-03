#!/bin/bash
# SPDX-FileCopyrightText: 2022 Battelle Memorial Institute
# SPDX-FileCopyrightText: 2022 University of Washington
# SPDX-License-Identifier: BSD-3-Clause
#
# download_gap_graphs.sh - Download GAP Benchmark Suite graphs for NWGraph
#
# Usage: ./scripts/download_gap_graphs.sh [OPTIONS] [GRAPH...]
#
# Options:
#   -h, --help      Show this help message
#   -d, --dir DIR   Download directory (default: data/graphs)
#   -l, --list      List available graphs with sizes
#   -c, --clean     Remove raw/temporary files after download
#   -s, --small     Download only small test graphs
#   -v, --verbose   Verbose output
#
# Graphs:
#   twitter   Twitter social network (1.47B edges, ~26GB compressed)
#   web       sk-2005 web crawl (1.95B edges, ~7GB compressed)
#   road      USA road network (58M edges, ~1GB compressed)
#   kron      Kronecker synthetic (2.11B edges, generated)
#   urand     Uniform random (2.15B edges, generated)
#   all       Download all graphs
#
# Example:
#   ./scripts/download_gap_graphs.sh road web     # Download road and web graphs
#   ./scripts/download_gap_graphs.sh --small      # Download small test graphs only
#   ./scripts/download_gap_graphs.sh all          # Download all GAP graphs

set -e

# Default settings
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NWGRAPH_ROOT="$(dirname "$SCRIPT_DIR")"
DOWNLOAD_DIR="${NWGRAPH_ROOT}/data/graphs"
RAW_DIR="${DOWNLOAD_DIR}/raw"
CLEAN_RAW=false
VERBOSE=false
SMALL_ONLY=false

# Color output (if terminal supports it)
if [ -t 1 ]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    NC='\033[0m' # No Color
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    NC=''
fi

# GAP graph information
# Format: name|vertices|edges|compressed_size|description|url
declare -A GRAPH_INFO
GRAPH_INFO[twitter]="61.6M|1.47B|~26GB|Twitter social network with skewed degree distribution"
GRAPH_INFO[web]="50.6M|1.95B|~7GB|sk-2005 web crawl from .sk domain"
GRAPH_INFO[road]="23.9M|58.3M|~1GB|USA road network (high diameter, small)"
GRAPH_INFO[kron]="134.2M|2.11B|generated|Kronecker synthetic graph (Graph 500 parameters)"
GRAPH_INFO[urand]="134.2M|2.15B|generated|Erdos-Renyi uniform random graph"

# URLs for downloading
TWITTER_URLS=(
    "https://github.com/ANLAB-KAIST/traces/releases/download/twitter_rv.net/twitter_rv.net.00.gz"
    "https://github.com/ANLAB-KAIST/traces/releases/download/twitter_rv.net/twitter_rv.net.01.gz"
    "https://github.com/ANLAB-KAIST/traces/releases/download/twitter_rv.net/twitter_rv.net.02.gz"
    "https://github.com/ANLAB-KAIST/traces/releases/download/twitter_rv.net/twitter_rv.net.03.gz"
)
WEB_URL="https://sparse.tamu.edu/MM/LAW/sk-2005.tar.gz"
ROAD_URL="http://www.dis.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.USA.gr.gz"

# Small test graphs from SuiteSparse (for quick testing)
SMALL_GRAPHS=(
    "https://sparse.tamu.edu/MM/SNAP/email-Eu-core.tar.gz"      # 1K vertices, 25K edges
    "https://sparse.tamu.edu/MM/Newman/karate.tar.gz"           # 34 vertices, 78 edges
    "https://sparse.tamu.edu/MM/Newman/football.tar.gz"         # 115 vertices, 613 edges
)

usage() {
    cat << EOF
${BLUE}NWGraph GAP Benchmark Graph Downloader${NC}

Usage: $0 [OPTIONS] [GRAPH...]

${YELLOW}Options:${NC}
  -h, --help      Show this help message
  -d, --dir DIR   Download directory (default: data/graphs)
  -l, --list      List available graphs with sizes
  -c, --clean     Remove raw/temporary files after download
  -s, --small     Download only small test graphs (for quick testing)
  -v, --verbose   Verbose output

${YELLOW}Available Graphs:${NC}
  twitter   Twitter social network (61.6M vertices, 1.47B edges)
  web       sk-2005 web crawl (50.6M vertices, 1.95B edges)
  road      USA road network (23.9M vertices, 58.3M edges)
  kron      Kronecker synthetic - requires generator (134.2M vertices, 2.11B edges)
  urand     Uniform random - requires generator (134.2M vertices, 2.15B edges)
  all       Download all real-world graphs (twitter, web, road)

${YELLOW}Examples:${NC}
  $0 road                    # Download road network only
  $0 road web                # Download road and web graphs
  $0 --small                 # Download small test graphs
  $0 -d /data/gap all        # Download all to custom directory
  $0 --list                  # Show graph information

${YELLOW}Disk Space Requirements:${NC}
  Small test graphs:  ~10 MB
  Road network:       ~2 GB (compressed ~1 GB)
  Web (sk-2005):      ~20 GB (compressed ~7 GB)
  Twitter:            ~80 GB (compressed ~26 GB)
  All real-world:     ~100 GB total

${YELLOW}Notes:${NC}
  - Synthetic graphs (kron, urand) must be generated using the converter tool
  - Downloaded graphs are in Matrix Market (.mtx) or edge list format
  - Use process_edge_list to convert to NWGraph binary format for faster loading

EOF
}

list_graphs() {
    echo -e "${BLUE}GAP Benchmark Suite Graphs${NC}"
    echo "========================================"
    printf "%-10s %-12s %-12s %-10s %s\n" "Name" "Vertices" "Edges" "Size" "Description"
    echo "------------------------------------------------------------------------"
    for name in twitter web road kron urand; do
        IFS='|' read -r vertices edges size desc <<< "${GRAPH_INFO[$name]}"
        printf "%-10s %-12s %-12s %-10s %s\n" "$name" "$vertices" "$edges" "$size" "$desc"
    done
    echo ""
    echo -e "${YELLOW}Reference:${NC} https://sparse.tamu.edu/GAP"
    echo -e "${YELLOW}Paper:${NC} Beamer et al., 'The GAP Benchmark Suite', arXiv:1508.03619"
}

log() {
    if [ "$VERBOSE" = true ]; then
        echo -e "${GREEN}[INFO]${NC} $1"
    fi
}

warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

check_disk_space() {
    local required_mb=$1
    local dir=$2

    # Get available space in MB
    if [[ "$OSTYPE" == "darwin"* ]]; then
        available_mb=$(df -m "$dir" | awk 'NR==2 {print $4}')
    else
        available_mb=$(df -m "$dir" | awk 'NR==2 {print $4}')
    fi

    if [ "$available_mb" -lt "$required_mb" ]; then
        error "Insufficient disk space. Required: ${required_mb}MB, Available: ${available_mb}MB"
        return 1
    fi
    log "Disk space check passed: ${available_mb}MB available"
}

download_file() {
    local url=$1
    local output=$2

    if [ -f "$output" ]; then
        log "File already exists: $output"
        return 0
    fi

    log "Downloading: $url"
    if command -v wget &> /dev/null; then
        wget -q --show-progress -O "$output" "$url"
    elif command -v curl &> /dev/null; then
        curl -L -# -o "$output" "$url"
    else
        error "Neither wget nor curl found. Please install one of them."
        exit 1
    fi
}

download_twitter() {
    echo -e "${BLUE}Downloading Twitter graph...${NC}"

    mkdir -p "$RAW_DIR"

    # Download all parts
    for url in "${TWITTER_URLS[@]}"; do
        filename=$(basename "$url")
        download_file "$url" "${RAW_DIR}/${filename}"
    done

    # Concatenate and decompress
    echo "Decompressing Twitter graph..."
    gunzip -c "${RAW_DIR}"/twitter_rv.net.*.gz > "${DOWNLOAD_DIR}/twitter.el"

    echo -e "${GREEN}Twitter graph downloaded to: ${DOWNLOAD_DIR}/twitter.el${NC}"
}

download_web() {
    echo -e "${BLUE}Downloading Web (sk-2005) graph...${NC}"

    mkdir -p "$RAW_DIR"

    download_file "$WEB_URL" "${RAW_DIR}/sk-2005.tar.gz"

    echo "Extracting sk-2005..."
    tar -xzf "${RAW_DIR}/sk-2005.tar.gz" -C "$RAW_DIR"

    # Move to final location
    mv "${RAW_DIR}/sk-2005/sk-2005.mtx" "${DOWNLOAD_DIR}/web.mtx"

    echo -e "${GREEN}Web graph downloaded to: ${DOWNLOAD_DIR}/web.mtx${NC}"
}

download_road() {
    echo -e "${BLUE}Downloading Road (USA) graph...${NC}"

    mkdir -p "$RAW_DIR"

    download_file "$ROAD_URL" "${RAW_DIR}/USA-road-d.USA.gr.gz"

    echo "Decompressing road graph..."
    gunzip -c "${RAW_DIR}/USA-road-d.USA.gr.gz" > "${DOWNLOAD_DIR}/road.gr"

    echo -e "${GREEN}Road graph downloaded to: ${DOWNLOAD_DIR}/road.gr${NC}"
}

download_small() {
    echo -e "${BLUE}Downloading small test graphs...${NC}"

    mkdir -p "$RAW_DIR"

    for url in "${SMALL_GRAPHS[@]}"; do
        filename=$(basename "$url")
        name="${filename%.tar.gz}"

        download_file "$url" "${RAW_DIR}/${filename}"

        echo "Extracting ${name}..."
        tar -xzf "${RAW_DIR}/${filename}" -C "$RAW_DIR"

        # Find and move the .mtx file
        mtx_file=$(find "${RAW_DIR}/${name}" -name "*.mtx" | head -1)
        if [ -n "$mtx_file" ]; then
            mv "$mtx_file" "${DOWNLOAD_DIR}/${name}.mtx"
            echo -e "${GREEN}Downloaded: ${DOWNLOAD_DIR}/${name}.mtx${NC}"
        fi
    done
}

clean_raw() {
    if [ "$CLEAN_RAW" = true ] && [ -d "$RAW_DIR" ]; then
        echo "Cleaning up raw files..."
        rm -rf "$RAW_DIR"
    fi
}

# Parse command line arguments
GRAPHS_TO_DOWNLOAD=()

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -d|--dir)
            DOWNLOAD_DIR="$2"
            RAW_DIR="${DOWNLOAD_DIR}/raw"
            shift 2
            ;;
        -l|--list)
            list_graphs
            exit 0
            ;;
        -c|--clean)
            CLEAN_RAW=true
            shift
            ;;
        -s|--small)
            SMALL_ONLY=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        all)
            GRAPHS_TO_DOWNLOAD+=(twitter web road)
            shift
            ;;
        twitter|web|road)
            GRAPHS_TO_DOWNLOAD+=("$1")
            shift
            ;;
        kron|urand)
            warn "Synthetic graph '$1' must be generated, not downloaded."
            warn "Use: ./converter -g27 -k16 -b kron.sg (for kron)"
            warn "Use: ./converter -u27 -k16 -b urand.sg (for urand)"
            shift
            ;;
        *)
            error "Unknown option or graph: $1"
            usage
            exit 1
            ;;
    esac
done

# Main execution
echo -e "${BLUE}NWGraph GAP Graph Downloader${NC}"
echo "=============================="

# Create download directory
mkdir -p "$DOWNLOAD_DIR"
log "Download directory: $DOWNLOAD_DIR"

if [ "$SMALL_ONLY" = true ]; then
    download_small
    clean_raw
    echo ""
    echo -e "${GREEN}Small test graphs downloaded successfully!${NC}"
    echo "Files are in: $DOWNLOAD_DIR"
    exit 0
fi

if [ ${#GRAPHS_TO_DOWNLOAD[@]} -eq 0 ]; then
    echo "No graphs specified. Use --help for usage information."
    echo ""
    list_graphs
    exit 0
fi

# Download requested graphs
for graph in "${GRAPHS_TO_DOWNLOAD[@]}"; do
    case $graph in
        twitter)
            download_twitter
            ;;
        web)
            download_web
            ;;
        road)
            download_road
            ;;
    esac
    echo ""
done

clean_raw

echo -e "${GREEN}Download complete!${NC}"
echo ""
echo "Downloaded files:"
ls -lh "${DOWNLOAD_DIR}"/*.{mtx,el,gr} 2>/dev/null || true
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "1. Build NWGraph with benchmarks enabled:"
echo "   cmake .. -DNWGRAPH_BUILD_BENCH=ON"
echo "   make -j8"
echo ""
echo "2. Run a benchmark:"
echo "   ./build/bench/gapbs/bfs -f ${DOWNLOAD_DIR}/road.gr -n 10"
