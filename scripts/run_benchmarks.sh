#!/bin/bash
# =============================================================================
# NWGraph Benchmark Runner
# =============================================================================
# This script runs the NWGraph benchmarks and collects performance data.
#
# Usage:
#   ./scripts/run_benchmarks.sh [OPTIONS]
#
# Options:
#   --build-dir DIR     Build directory (default: ./build)
#   --output-dir DIR    Output directory for results (default: ./benchmark_results)
#   --ntrials N         Number of trials per benchmark (default: 5)
#   --max-threads N     Maximum threads to test (default: auto-detect)
#   --size N            Problem size for scalability benchmarks (default: 10000000)
#   --graph FILE        Graph file for GAPBS benchmarks
#   --suite SUITE       Benchmark suite to run: all, scalability, gapbs, apb
#   --help              Show this help message
#
# Examples:
#   ./scripts/run_benchmarks.sh --suite scalability
#   ./scripts/run_benchmarks.sh --suite gapbs --graph data/twitter.mtx
#   ./scripts/run_benchmarks.sh --suite all --ntrials 10 --output-dir results/
#
# =============================================================================

set -e

# Default values
BUILD_DIR="./build"
OUTPUT_DIR="./benchmark_results"
NTRIALS=5
MAX_THREADS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
PROBLEM_SIZE=10000000
GRAPH_FILE=""
SUITE="all"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --output-dir)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --ntrials)
            NTRIALS="$2"
            shift 2
            ;;
        --max-threads)
            MAX_THREADS="$2"
            shift 2
            ;;
        --size)
            PROBLEM_SIZE="$2"
            shift 2
            ;;
        --graph)
            GRAPH_FILE="$2"
            shift 2
            ;;
        --suite)
            SUITE="$2"
            shift 2
            ;;
        --help)
            head -40 "$0" | tail -35
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Create output directory
mkdir -p "$OUTPUT_DIR"

# Get timestamp for results
TIMESTAMP=$(date +%Y%m%d_%H%M%S)

# Log file
LOG_FILE="$OUTPUT_DIR/benchmark_${TIMESTAMP}.log"

# Function to log messages
log() {
    echo -e "$1" | tee -a "$LOG_FILE"
}

# Function to run a benchmark and save output
run_benchmark() {
    local name="$1"
    local cmd="$2"
    local output_file="$OUTPUT_DIR/${name}_${TIMESTAMP}.txt"

    log "${BLUE}Running: $name${NC}"
    log "Command: $cmd"

    if eval "$cmd" > "$output_file" 2>&1; then
        log "${GREEN}  [PASS]${NC} Results saved to $output_file"
        return 0
    else
        log "${RED}  [FAIL]${NC} Benchmark failed. Check $output_file for details"
        return 1
    fi
}

# Function to check if executable exists
check_executable() {
    local exe="$1"
    if [[ ! -x "$exe" ]]; then
        log "${YELLOW}Warning: $exe not found or not executable${NC}"
        return 1
    fi
    return 0
}

# Print header
log "=============================================="
log "NWGraph Benchmark Runner"
log "=============================================="
log "Date: $(date)"
log "Build directory: $BUILD_DIR"
log "Output directory: $OUTPUT_DIR"
log "Trials: $NTRIALS"
log "Max threads: $MAX_THREADS"
log "Problem size: $PROBLEM_SIZE"
log "Suite: $SUITE"
[[ -n "$GRAPH_FILE" ]] && log "Graph file: $GRAPH_FILE"
log ""

# Check build directory
if [[ ! -d "$BUILD_DIR" ]]; then
    log "${RED}Error: Build directory not found: $BUILD_DIR${NC}"
    log "Run 'cmake -B build && cmake --build build' first"
    exit 1
fi

# Run scalability benchmarks
run_scalability_benchmarks() {
    log "\n${BLUE}=== Scalability Benchmarks ===${NC}\n"

    local bench_dir="$BUILD_DIR/bench/scalability"

    # parallel_for scaling
    if check_executable "$bench_dir/parallel_for_scaling"; then
        run_benchmark "parallel_for_scaling" \
            "$bench_dir/parallel_for_scaling -n $NTRIALS -t $MAX_THREADS -s $PROBLEM_SIZE"
    fi

    # parallel_reduce scaling
    if check_executable "$bench_dir/parallel_reduce_scaling"; then
        run_benchmark "parallel_reduce_scaling" \
            "$bench_dir/parallel_reduce_scaling -n $NTRIALS -t $MAX_THREADS -s $PROBLEM_SIZE"
    fi
}

# Run GAPBS benchmarks
run_gapbs_benchmarks() {
    log "\n${BLUE}=== GAPBS Benchmarks ===${NC}\n"

    if [[ -z "$GRAPH_FILE" ]]; then
        log "${YELLOW}No graph file specified. Skipping GAPBS benchmarks.${NC}"
        log "Use --graph FILE to specify a graph file."
        return
    fi

    if [[ ! -f "$GRAPH_FILE" ]]; then
        log "${RED}Graph file not found: $GRAPH_FILE${NC}"
        return
    fi

    local bench_dir="$BUILD_DIR/bench/gapbs"
    local graph_name=$(basename "$GRAPH_FILE" .mtx)

    # BFS
    if check_executable "$bench_dir/bfs"; then
        run_benchmark "gapbs_bfs_${graph_name}" \
            "$bench_dir/bfs -f $GRAPH_FILE -n $NTRIALS"
    fi

    # SSSP
    if check_executable "$bench_dir/sssp"; then
        run_benchmark "gapbs_sssp_${graph_name}" \
            "$bench_dir/sssp -f $GRAPH_FILE -n $NTRIALS"
    fi

    # PageRank
    if check_executable "$bench_dir/pr"; then
        run_benchmark "gapbs_pr_${graph_name}" \
            "$bench_dir/pr -f $GRAPH_FILE -n $NTRIALS"
    fi

    # Connected Components
    if check_executable "$bench_dir/cc"; then
        run_benchmark "gapbs_cc_${graph_name}" \
            "$bench_dir/cc -f $GRAPH_FILE -n $NTRIALS"
    fi

    # Triangle Counting
    if check_executable "$bench_dir/tc"; then
        run_benchmark "gapbs_tc_${graph_name}" \
            "$bench_dir/tc -f $GRAPH_FILE -n $NTRIALS"
    fi

    # Betweenness Centrality
    if check_executable "$bench_dir/bc"; then
        run_benchmark "gapbs_bc_${graph_name}" \
            "$bench_dir/bc -f $GRAPH_FILE -n $NTRIALS"
    fi
}

# Run Abstraction Penalty Benchmarks
run_apb_benchmarks() {
    log "\n${BLUE}=== Abstraction Penalty Benchmarks ===${NC}\n"

    if [[ -z "$GRAPH_FILE" ]]; then
        log "${YELLOW}No graph file specified. Skipping APB benchmarks.${NC}"
        log "Use --graph FILE to specify a graph file."
        return
    fi

    if [[ ! -f "$GRAPH_FILE" ]]; then
        log "${RED}Graph file not found: $GRAPH_FILE${NC}"
        return
    fi

    local bench_dir="$BUILD_DIR/bench/abstraction_penalty"
    local graph_name=$(basename "$GRAPH_FILE" .mtx)

    # Plain iteration
    if check_executable "$bench_dir/plain"; then
        run_benchmark "apb_plain_${graph_name}" \
            "$bench_dir/plain -f $GRAPH_FILE -n $NTRIALS"
    fi

    # SpMV
    if check_executable "$bench_dir/spmv"; then
        run_benchmark "apb_spmv_${graph_name}" \
            "$bench_dir/spmv -f $GRAPH_FILE -n $NTRIALS"
    fi

    # TBB parallel
    if check_executable "$bench_dir/tbb"; then
        run_benchmark "apb_tbb_${graph_name}" \
            "$bench_dir/tbb -f $GRAPH_FILE -n $NTRIALS"
    fi

    # BFS
    if check_executable "$bench_dir/bfs_apb"; then
        run_benchmark "apb_bfs_${graph_name}" \
            "$bench_dir/bfs_apb -f $GRAPH_FILE -n $NTRIALS"
    fi

    # DFS
    if check_executable "$bench_dir/dfs"; then
        run_benchmark "apb_dfs_${graph_name}" \
            "$bench_dir/dfs -f $GRAPH_FILE -n $NTRIALS"
    fi

    # Dijkstra
    if check_executable "$bench_dir/dijkstra"; then
        run_benchmark "apb_dijkstra_${graph_name}" \
            "$bench_dir/dijkstra -f $GRAPH_FILE -n $NTRIALS"
    fi
}

# Run selected suite(s)
case $SUITE in
    all)
        run_scalability_benchmarks
        run_gapbs_benchmarks
        run_apb_benchmarks
        ;;
    scalability)
        run_scalability_benchmarks
        ;;
    gapbs)
        run_gapbs_benchmarks
        ;;
    apb)
        run_apb_benchmarks
        ;;
    *)
        log "${RED}Unknown suite: $SUITE${NC}"
        log "Valid suites: all, scalability, gapbs, apb"
        exit 1
        ;;
esac

# Print summary
log "\n=============================================="
log "Benchmark run complete"
log "=============================================="
log "Results saved to: $OUTPUT_DIR"
log "Log file: $LOG_FILE"
log ""

# List result files
log "Result files:"
ls -la "$OUTPUT_DIR"/*_${TIMESTAMP}.txt 2>/dev/null | while read line; do
    log "  $line"
done

log "\nDone!"
