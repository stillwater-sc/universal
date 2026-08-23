#!/usr/bin/env bash
# measure-include-cost.sh -- report the include cost of a Universal header.
#
# Epic #1334 (header layering) gates each phase on three numbers. This script
# produces them the same way every time so PRs in that epic are comparable, and
# so the acceptance criteria can be checked mechanically rather than by eye.
#
#   preprocessed lines : total volume the compiler sees for this header alone
#   transitive headers : how many files the include graph pulls in
#   I/O-family headers : iostream/sstream/iomanip/ostream/istream in the graph
#
# The target for a Phase-1 core header is 0 I/O-family headers and fewer than
# 45000 preprocessed lines. That ceiling is derived, not guessed: on gcc 13.3
# <cstdint>+<type_traits>+<limits>+<cmath>+<concepts> is 24977 lines, and adding
# <string> reaches 42091 -- so 45000 leaves a core header its plausible
# dependency set and nothing more.
#
# Usage:
#   tools/scripts/measure-include-cost.sh universal/number/posit/posit.hpp
#   tools/scripts/measure-include-cost.sh --all          # the tracked baseline set
#   CXX=clang++ tools/scripts/measure-include-cost.sh --all
#
# Run from the repository root.

set -uo pipefail

CXX="${CXX:-g++}"
STD="${STD:-c++20}"
INCLUDE_DIR="${INCLUDE_DIR:-include/sw}"
IO_FAMILY='/(iostream|sstream|iomanip|ostream|istream)$'

BASELINE_SET=(
    universal/number/posit/posit.hpp
    universal/number/cfloat/cfloat.hpp
    universal/number/lns/lns.hpp
    universal/number/fixpnt/fixpnt.hpp
    universal/number/integer/integer.hpp
    universal/number/dd/dd.hpp
    universal/number/qd/qd.hpp
    universal/number/takum/takum.hpp
)

if [ ! -d "$INCLUDE_DIR" ]; then
    echo "error: '$INCLUDE_DIR' not found -- run from the repository root." >&2
    exit 2
fi

probe=$(mktemp /tmp/universal-include-probe.XXXXXX.cpp)
graph=$(mktemp /tmp/universal-include-graph.XXXXXX.txt)
trap 'rm -f "$probe" "$graph"' EXIT

measure_one() {
    local header="$1"
    printf '#include <%s>\nint main() { return 0; }\n' "$header" > "$probe"

    if ! "$CXX" -std="$STD" -I "$INCLUDE_DIR" -fsyntax-only "$probe" 2>/dev/null; then
        printf '%-46s %10s %8s %6s   DOES NOT COMPILE\n' "$header" "-" "-" "-"
        return 1
    fi

    local lines headers io
    lines=$("$CXX" -std="$STD" -I "$INCLUDE_DIR" -E "$probe" 2>/dev/null | wc -l)
    "$CXX" -std="$STD" -I "$INCLUDE_DIR" -H -fsyntax-only "$probe" 2>&1 | grep -E '^\.+ ' > "$graph" || true
    headers=$(wc -l < "$graph")
    io=$(grep -cE "$IO_FAMILY" "$graph" || true)

    printf '%-46s %10s %8s %6s\n' "$header" "$lines" "$headers" "$io"

    # name the entry point for each I/O-family header: that is the file to fix
    if [ "$io" -gt 0 ] && [ "${VERBOSE:-0}" = "1" ]; then
        awk '{ d = length($1); path = $2
               stack[d] = path
               n = split(path, parts, "/"); base = parts[n]
               if ((base ~ /^(iostream|sstream|iomanip|ostream|istream)$/) && !(base in seen)) {
                   seen[base] = 1
                   for (k = d - 1; k >= 1; k--)
                       if (stack[k] ~ /sw\/universal/) { printf "        %-12s entered via %s\n", base, stack[k]; break }
               } }' "$graph"
    fi
    return 0
}

printf '%-46s %10s %8s %6s\n' "header" "lines" "headers" "I/O"
printf '%-46s %10s %8s %6s\n' "---------------------------------------------" "----------" "--------" "------"

rc=0
if [ "${1:-}" = "--all" ] || [ $# -eq 0 ]; then
    for h in "${BASELINE_SET[@]}"; do measure_one "$h" || rc=1; done
else
    for h in "$@"; do measure_one "$h" || rc=1; done
fi

echo
echo "compiler: $($CXX --version | head -1)   std: $STD"
echo "Phase-1 core target: 0 I/O-family headers, under 45000 preprocessed lines."
echo "Set VERBOSE=1 to name the header that pulls in each I/O-family include."
exit $rc
