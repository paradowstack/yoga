#!/bin/bash

# Yoga Benchmark Comparison Script
# Usage:
#   ./run_benchmark_compare.sh baseline         - Run 10x baseline and save averages
#   ./run_benchmark_compare.sh compare          - Run benchmark and compare to baseline
#   ./run_benchmark_compare.sh copy-sources     - Copy yoga sources from react-native-fork
#   ./run_benchmark_compare.sh copy-and-compare - Copy sources and run comparison
#   ./run_benchmark_compare.sh run              - Run single benchmark (no comparison)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
CAPTURES_DIR="$SCRIPT_DIR/captures"
BASELINE_FILE="$SCRIPT_DIR/baseline.json"
BENCHMARK_BIN="$BUILD_DIR/benchmark"
NUM_RUNS_BASELINE=10
NUM_RUNS_COMPARE=3

# Source paths configuration
RN_FORK_YOGA_DIR="$HOME/projects/react-native-fork/packages/react-native/ReactCommon/yoga/yoga"
YOGA_TARGET_DIR="$(dirname "$SCRIPT_DIR")/yoga"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'
BOLD='\033[1m'

# Check for ccache
USE_CCACHE=""
if command -v ccache &> /dev/null; then
    USE_CCACHE="-D CMAKE_C_COMPILER_LAUNCHER=ccache -D CMAKE_CXX_COMPILER_LAUNCHER=ccache"
    echo -e "${GREEN}Using ccache for faster rebuilds${NC}"
fi

build_verbose() {
    local need_configure=0
    
    # Only configure if CMakeCache doesn't exist or CMakeLists.txt changed
    if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
        need_configure=1
    elif [[ "$SCRIPT_DIR/CMakeLists.txt" -nt "$BUILD_DIR/CMakeCache.txt" ]]; then
        need_configure=1
    fi
    
    if [[ $need_configure -eq 1 ]]; then
        echo -e "${BLUE}Configuring build...${NC}"
        cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR" -D CMAKE_BUILD_TYPE=Release $USE_CCACHE
    fi
    
    echo -e "${BLUE}Building benchmark (incremental)...${NC}"
    cmake --build "$BUILD_DIR" --parallel > /dev/null
    echo -e "${GREEN}Build complete.${NC}"
}
build_benchmark() {
    local need_configure=0
    
    # Only configure if CMakeCache doesn't exist or CMakeLists.txt changed
    if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]]; then
        need_configure=1
    elif [[ "$SCRIPT_DIR/CMakeLists.txt" -nt "$BUILD_DIR/CMakeCache.txt" ]]; then
        need_configure=1
    fi
    
    if [[ $need_configure -eq 1 ]]; then
        echo -e "${BLUE}Configuring build...${NC}"
        cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR" -D CMAKE_BUILD_TYPE=Release $USE_CCACHE > /dev/null 2>&1
    fi
    
    echo -e "${BLUE}Building benchmark (incremental)...${NC}"
    cmake --build "$BUILD_DIR" --parallel > /dev/null 2>&1
    echo -e "${GREEN}Build complete.${NC}"
}

run_single_benchmark() {
    "$BENCHMARK_BIN" "$CAPTURES_DIR"
}

copy_sources() {
    if [[ ! -d "$RN_FORK_YOGA_DIR" ]]; then
        echo -e "${RED}Error: Source directory not found at $RN_FORK_YOGA_DIR${NC}"
        exit 1
    fi

    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo -e "${BOLD}${CYAN}                    Copying Yoga Sources${NC}"
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo
    echo -e "${BLUE}From: ${NC}$RN_FORK_YOGA_DIR"
    echo -e "${BLUE}To:   ${NC}$YOGA_TARGET_DIR"
    echo
    
    # Use rsync to only copy changed files (much faster for incremental changes)
    if command -v rsync &> /dev/null; then
        rsync -a --delete "$RN_FORK_YOGA_DIR"/ "$YOGA_TARGET_DIR"/
        echo -e "${GREEN}Sources synced (rsync - only changed files).${NC}"
    else
        cp -r "$RN_FORK_YOGA_DIR"/* "$YOGA_TARGET_DIR"/
        echo -e "${GREEN}Sources copied.${NC}"
    fi
}

copy_back() {
    if [[ ! -d "$YOGA_TARGET_DIR" ]]; then
        echo -e "${RED}Error: Source directory not found at $YOGA_TARGET_DIR${NC}"
        exit 1
    fi

    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo -e "${BOLD}${CYAN}                    Copying Yoga Sources${NC}"
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo
    echo -e "${BLUE}From: ${NC}$YOGA_TARGET_DIR"
    echo -e "${BLUE}To:   ${NC}$RN_FORK_YOGA_DIR"
    echo
    
    # Use rsync to only copy changed files (much faster for incremental changes)
    if command -v rsync &> /dev/null; then
        rsync -a --delete "$YOGA_TARGET_DIR"/ "$RN_FORK_YOGA_DIR"/
        echo -e "${GREEN}Sources synced (rsync - only changed files).${NC}"
    else
        cp -r "$YOGA_TARGET_DIR"/* "$RN_FORK_YOGA_DIR"/
        echo -e "${GREEN}Sources copied.${NC}"
    fi
}

run_baseline() {
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo -e "${BOLD}${CYAN}       Running Baseline Benchmarks ($NUM_RUNS_BASELINE iterations)${NC}"
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo

    build_benchmark

    python3 - "$NUM_RUNS_BASELINE" "$BENCHMARK_BIN" "$CAPTURES_DIR" "$BASELINE_FILE" << 'PYTHON_SCRIPT'
import subprocess, sys, json, re
from collections import defaultdict

num_runs = int(sys.argv[1])
benchmark_bin = sys.argv[2]
captures_dir = sys.argv[3]
baseline_file = sys.argv[4]

data = defaultdict(lambda: defaultdict(list))

for i in range(num_runs):
    print(f"\033[33mRun {i+1}/{num_runs}...\033[0m")
    result = subprocess.run([benchmark_bin, captures_dir], capture_output=True, text=True)
    for line in result.stdout.strip().split("\n"):
        if "median:" in line:
            match = re.match(r"^(\S+)\s+(tree creation|layout|total):\s+median:\s+([0-9.]+)\s+ms", line)
            if match:
                data[match.group(1)][match.group(2)].append(float(match.group(3)))

baseline = {}
for profile, metrics in data.items():
    baseline[profile] = {}
    for metric, values in metrics.items():
        if values:
            baseline[profile][metric] = {"average": round(sum(values)/len(values), 6), "min": round(min(values), 6), "max": round(max(values), 6), "runs": len(values)}

with open(baseline_file, "w") as f:
    json.dump(baseline, f, indent=2)

print(f"\n\033[32mBaseline saved to {baseline_file}\033[0m\n")
for profile in sorted(baseline.keys()):
    m = baseline[profile]
    print(f"  {profile:<22} Tree: {m.get('tree creation',{}).get('average',0):>7.2f}ms  Layout: {m.get('layout',{}).get('average',0):>6.2f}ms  Total: {m.get('total',{}).get('average',0):>7.2f}ms")
PYTHON_SCRIPT
}

run_compare() {
    if [[ ! -f "$BASELINE_FILE" ]]; then
        echo -e "${RED}Error: Baseline file not found at $BASELINE_FILE${NC}"
        echo -e "${YELLOW}Run './run_benchmark_compare.sh baseline' first.${NC}"
        exit 1
    fi

    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo -e "${BOLD}${CYAN}              Running Benchmark Comparison${NC}"
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo

    build_benchmark

    python3 - "$NUM_RUNS_COMPARE" "$BENCHMARK_BIN" "$CAPTURES_DIR" "$BASELINE_FILE" << 'PYTHON_SCRIPT'
import subprocess, sys, json, re
from collections import defaultdict

num_runs = int(sys.argv[1])
benchmark_bin = sys.argv[2]
captures_dir = sys.argv[3]
baseline_file = sys.argv[4]

with open(baseline_file, "r") as f:
    baseline = json.load(f)

data = defaultdict(lambda: defaultdict(list))

for i in range(num_runs):
    print(f"\033[33mRun {i+1}/{num_runs}...\033[0m")
    result = subprocess.run([benchmark_bin, captures_dir], capture_output=True, text=True)
    for line in result.stdout.strip().split("\n"):
        if "median:" in line:
            match = re.match(r"^(\S+)\s+(tree creation|layout|total):\s+median:\s+([0-9.]+)\s+ms", line)
            if match:
                data[match.group(1)][match.group(2)].append(float(match.group(3)))

current = {}
for profile, metrics in data.items():
    current[profile] = {m: sum(v)/len(v) for m, v in metrics.items() if v}

R, G, Y, C, N, B = "\033[31m", "\033[32m", "\033[33m", "\033[1;36m", "\033[0m", "\033[1m"

def diff_color(base, curr):
    if base == 0: return Y, 0
    pct = ((curr - base) / base) * 100
    return (R if pct > 2 else G if pct < -2 else Y), pct

print(f"\n{C}{'=' * 95}{N}")
print(f"{C}{'YOGA BENCHMARK COMPARISON RESULTS':^95}{N}")
print(f"{C}{'=' * 95}{N}\n")
print(f"{'Profile':<29} {'Tree Creation':<31} {'Layout':<22} {'Total':<10}")
print(f"{'':<22} {'Base':>7}  {'Curr':>7}  {'Diff':>7}   {'Base':>7}  {'Curr':>7}  {'Diff':>7}   {'Result':>12}")
print("-" * 95)

for profile in sorted(set(list(baseline.keys()) + list(current.keys()))):
    bt = baseline.get(profile, {}).get("tree creation", {}).get("average", 0)
    bl = baseline.get(profile, {}).get("layout", {}).get("average", 0)
    bT = baseline.get(profile, {}).get("total", {}).get("average", 0)
    ct = current.get(profile, {}).get("tree creation", 0)
    cl = current.get(profile, {}).get("layout", 0)
    cT = current.get(profile, {}).get("total", 0)
    tc, tp = diff_color(bt, ct)
    lc, lp = diff_color(bl, cl)
    Tc, Tp = diff_color(bT, cT)
    print(f"{profile:<22} {bt:6.2f}ms {ct:6.2f}ms {tc}{tp:+6.1f}%{N}   {bl:6.2f}ms {cl:6.2f}ms {lc}{lp:+6.1f}%{N}   {cT:6.2f}ms {Tc}({Tp:+.1f}%){N}")

print("-" * 95)
print(f"\n{B}Legend:{N} {G}Faster (>2%){N}  {Y}Neutral{N}  {R}Slower (>2%){N}\n")
PYTHON_SCRIPT
}

run_single() {
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo -e "${BOLD}${CYAN}                    Running Single Benchmark${NC}"
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo
    build_benchmark
    run_single_benchmark
}
run_hard_baseline() {
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo -e "${BOLD}${CYAN}                 Running Hard Baseline Benchmark${NC}"
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo
    
    echo -e "${BLUE}Checking out main branch ...${NC}"
    local stash_created=0
    if ! git diff --quiet || ! git diff --cached --quiet || [[ -n $(git ls-files --others --exclude-standard) ]]; then
        git stash push -u
        stash_created=1
    fi
    local original_branch=$(git rev-parse --abbrev-ref HEAD)
    git checkout main
    echo -e "${BLUE}Running baseline benchmark ...${NC}"
    run_baseline
    mv baseline.json baseline_copy.json
    echo -e "${BLUE}Reverting to current version ...${NC}"
    git checkout "$original_branch"
    if [[ $stash_created -eq 1 ]]; then
        git stash pop
    fi
    mv baseline_copy.json baseline.json
    echo -e "${GREEN}Hard baseline complete.${NC}"
}
run_compare_baseline() {
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo -e "${BOLD}${CYAN}              Running Baseline Comparison Benchmark${NC}"
    echo -e "${BOLD}${CYAN}===============================================================${NC}"
    echo
    
    echo -e "${BLUE}Checking out main branch ...${NC}"
    local stash_created=0
    if ! git diff --quiet || ! git diff --cached --quiet || [[ -n $(git ls-files --others --exclude-standard) ]]; then
        git stash push -u
        stash_created=1
    fi
    cp baseline.json baseline_copy.json
    git checkout b43d7d999e7f5f4a978f2547786526303b46bb1e
    mv baseline_copy.json baseline.json
    echo -e "${BLUE}Running comparison benchmark ...${NC}"
    run_compare
    echo -e "${BLUE}Reverting to current version ...${NC}"
    git restore baseline.json
    git checkout -
    if [[ $stash_created -eq 1 ]]; then
        git stash pop
    fi
    echo -e "${GREEN}Baseline comparison complete.${NC}"
}
show_help() {
    echo -e "${BOLD}Yoga Benchmark Comparison Tool${NC}"
    echo
    echo -e "${YELLOW}Usage:${NC}"
    echo "  $0 baseline             Run $NUM_RUNS_BASELINE benchmarks and save averages as baseline"
    echo "  $0 hard-baseline        Run benchmark on original baseline commit"
    echo "  $0 compare              Run $NUM_RUNS_COMPARE benchmark(s) and compare against baseline"
    echo "  $0 compare-baseline     Run $NUM_RUNS_COMPARE benchmark(s) on baseline commit"
    echo "  $0 copy-sources         Copy yoga sources from react-native-fork to yoga repo"
    echo "  $0 copy-and-compare     Copy sources and run comparison"
    echo "  $0 run                  Run a single benchmark (no comparison)"
    echo "  $0 help                 Show this help message"
    echo
    echo -e "${YELLOW}Workflow:${NC}"
    echo "  1. Checkout baseline code: git checkout yoga/"
    echo "  2. Run '$0 baseline' to save baseline measurements"
    echo "  3. Run '$0 copy-and-compare' to test your changes"
    echo
    echo -e "${YELLOW}Build Optimizations:${NC}"
    echo "  - Uses rsync to copy only changed files"
    echo "  - Incremental builds (only recompiles changed files)"
    echo "  - Parallel compilation"
    echo "  - Install ccache for even faster rebuilds: brew install ccache"
}

case "${1:-help}" in
    build) build_verbose ;;
    baseline) run_baseline ;;
    hard-baseline) run_hard_baseline ;;
    compare) run_compare ;;
    compare-baseline) run_compare_baseline ;;
    copy-sources) copy_sources ;;
    copy-back) copy_back ;;
    copy-and-compare) copy_sources; echo; run_compare ;;
    run) run_single ;;
    help|--help|-h) show_help ;;
    *) echo -e "${RED}Unknown command: $1${NC}"; show_help; exit 1 ;;
esac
