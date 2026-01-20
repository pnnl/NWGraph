#!/bin/bash
# =============================================================================
# NWGraph Backend Comparison Script
# =============================================================================
# This script compares performance between TBB and HPX backends.
#
# Usage:
#   ./scripts/compare_backends.sh [OPTIONS]
#
# Options:
#   --tbb-build DIR     TBB build directory (default: ./build-tbb)
#   --hpx-build DIR     HPX build directory (default: ./build-hpx)
#   --output-dir DIR    Output directory for results (default: ./comparison_results)
#   --ntrials N         Number of trials per benchmark (default: 5)
#   --max-threads N     Maximum threads to test (default: auto-detect)
#   --size N            Problem size for scalability benchmarks (default: 10000000)
#   --graph FILE        Graph file for graph benchmarks
#   --help              Show this help message
#
# Prerequisites:
#   Build TBB version:  cmake -B build-tbb && cmake --build build-tbb
#   Build HPX version:  cmake -B build-hpx -DNWGRAPH_BACKEND_HPX=ON && cmake --build build-hpx
#
# =============================================================================

set -e

# Default values
TBB_BUILD_DIR="./build-tbb"
HPX_BUILD_DIR="./build-hpx"
OUTPUT_DIR="./comparison_results"
NTRIALS=5
MAX_THREADS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
PROBLEM_SIZE=10000000
GRAPH_FILE=""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --tbb-build)
            TBB_BUILD_DIR="$2"
            shift 2
            ;;
        --hpx-build)
            HPX_BUILD_DIR="$2"
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
        --help)
            head -35 "$0" | tail -30
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

TIMESTAMP=$(date +%Y%m%d_%H%M%S)
COMPARISON_FILE="$OUTPUT_DIR/comparison_${TIMESTAMP}.csv"

# Log function
log() {
    echo -e "$1"
}

# Function to check if build exists
check_build() {
    local build_dir="$1"
    local name="$2"

    if [[ ! -d "$build_dir" ]]; then
        log "${RED}$name build directory not found: $build_dir${NC}"
        return 1
    fi
    return 0
}

# Function to run a benchmark and extract timing
run_and_time() {
    local exe="$1"
    local args="$2"
    local output_file="$3"

    if [[ ! -x "$exe" ]]; then
        echo "N/A"
        return
    fi

    "$exe" $args > "$output_file" 2>&1

    # Extract mean time from output (assumes format "mean=XXX.XXms")
    local mean=$(grep -oP 'mean=\K[\d.]+' "$output_file" | head -1)
    if [[ -z "$mean" ]]; then
        echo "N/A"
    else
        echo "$mean"
    fi
}

# Print header
log "=============================================="
log "NWGraph Backend Comparison"
log "=============================================="
log "Date: $(date)"
log "TBB build: $TBB_BUILD_DIR"
log "HPX build: $HPX_BUILD_DIR"
log "Output: $OUTPUT_DIR"
log "Trials: $NTRIALS"
log "Max threads: $MAX_THREADS"
log "Problem size: $PROBLEM_SIZE"
log ""

# Check builds exist
TBB_OK=false
HPX_OK=false

if check_build "$TBB_BUILD_DIR" "TBB"; then
    TBB_OK=true
    log "${GREEN}TBB build found${NC}"
fi

if check_build "$HPX_BUILD_DIR" "HPX"; then
    HPX_OK=true
    log "${GREEN}HPX build found${NC}"
fi

if ! $TBB_OK && ! $HPX_OK; then
    log "${RED}No builds found. Please build NWGraph first.${NC}"
    log ""
    log "For TBB backend:"
    log "  cmake -B build-tbb && cmake --build build-tbb"
    log ""
    log "For HPX backend:"
    log "  cmake -B build-hpx -DNWGRAPH_BACKEND_HPX=ON && cmake --build build-hpx"
    exit 1
fi

# Initialize comparison file
echo "Benchmark,Threads,TBB_ms,HPX_ms,Speedup" > "$COMPARISON_FILE"

# Run comparison for scalability benchmarks
compare_scalability() {
    log "\n${BLUE}=== Comparing Scalability Benchmarks ===${NC}\n"

    local thread_counts=(1 2 4 8)
    [[ $MAX_THREADS -gt 8 ]] && thread_counts+=(16)
    [[ $MAX_THREADS -gt 16 ]] && thread_counts+=($MAX_THREADS)

    for threads in "${thread_counts[@]}"; do
        [[ $threads -gt $MAX_THREADS ]] && continue

        log "${CYAN}Testing with $threads threads...${NC}"

        # parallel_for scaling
        local tbb_time="N/A"
        local hpx_time="N/A"

        if $TBB_OK; then
            local exe="$TBB_BUILD_DIR/bench/scalability/parallel_for_scaling"
            if [[ -x "$exe" ]]; then
                tbb_time=$(run_and_time "$exe" "-n $NTRIALS -t $threads -s $PROBLEM_SIZE" \
                    "$OUTPUT_DIR/tbb_pfor_t${threads}.txt")
            fi
        fi

        if $HPX_OK; then
            local exe="$HPX_BUILD_DIR/bench/scalability/parallel_for_scaling"
            if [[ -x "$exe" ]]; then
                hpx_time=$(run_and_time "$exe" "-n $NTRIALS -t $threads -s $PROBLEM_SIZE" \
                    "$OUTPUT_DIR/hpx_pfor_t${threads}.txt")
            fi
        fi

        # Calculate speedup
        local speedup="N/A"
        if [[ "$tbb_time" != "N/A" && "$hpx_time" != "N/A" ]]; then
            speedup=$(echo "scale=2; $tbb_time / $hpx_time" | bc 2>/dev/null || echo "N/A")
        fi

        echo "parallel_for,$threads,$tbb_time,$hpx_time,$speedup" >> "$COMPARISON_FILE"
        log "  parallel_for: TBB=${tbb_time}ms, HPX=${hpx_time}ms, HPX/TBB speedup=${speedup}x"
    done
}

# Print comparison results
print_results() {
    log "\n${BLUE}=== Comparison Results ===${NC}\n"
    log "Results saved to: $COMPARISON_FILE"
    log ""

    if [[ -f "$COMPARISON_FILE" ]]; then
        log "Summary:"
        column -t -s',' "$COMPARISON_FILE" 2>/dev/null || cat "$COMPARISON_FILE"
    fi
}

# Run comparison
compare_scalability
print_results

log "\n${GREEN}Comparison complete!${NC}"
log "Results: $COMPARISON_FILE"
