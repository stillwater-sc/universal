#!/usr/bin/env bash
#
# check-ascii.sh - enforce the ASCII-only rule for source and build files.
#
# Rationale: Unicode characters (box-drawing, smart quotes, math symbols,
# em-dashes, superscripts, etc.) do not render correctly on Windows consoles
# and can break tooling. All C/C++ source, CMake, and build/automation scripts
# in this repository must contain only 7-bit ASCII bytes.
#
# This script scans the git-tracked source and build files and fails (exit 1)
# if any non-ASCII byte is found, printing the offending file:line:char so the
# problem is easy to locate. Documentation (*.md) and binary assets are NOT
# checked - the rule applies to source and build/script files only.
#
# Usage:
#   tools/scripts/check-ascii.sh            # scan the whole tracked tree
#   tools/scripts/check-ascii.sh file ...   # scan specific files
#
# Wired into CI by .github/workflows/ascii-guard.yml.

set -euo pipefail

# Non-ASCII = any byte outside 0x00-0x7F. grep -P with this negated class
# matches such bytes; grep -I skips binary files.
PATTERN='[^\x00-\x7F]'

# File globs that constitute "source and build files".
is_in_scope() {
	case "$1" in
		*.hpp|*.cpp|*.h|*.c|*.cc|*.cxx|*.hxx|*.inc|*.cpp_) return 0 ;;
		*.cmake|*/CMakeLists.txt|CMakeLists.txt)           return 0 ;;
		*.sh|*.py|*.mjs|*.js|*.bash)                       return 0 ;;
		*Dockerfile|*Dockerfile.*|*/Dockerfile)            return 0 ;;
		*) return 1 ;;
	esac
}

# Build the candidate list: explicit args, else all git-tracked files.
if [ "$#" -gt 0 ]; then
	candidates=("$@")
else
	mapfile -t candidates < <(git ls-files)
fi

violations=0
for f in "${candidates[@]}"; do
	[ -f "$f" ] || continue
	is_in_scope "$f" || continue
	if LC_ALL=C grep -InP "$PATTERN" -- "$f" >/dev/null 2>&1; then
		violations=$((violations + 1))
		echo "Non-ASCII bytes in: $f"
		# Show line:col context for each offending line (cap to keep output sane).
		LC_ALL=C grep -nP "$PATTERN" -- "$f" | head -20 | sed 's/^/    /'
	fi
done

if [ "$violations" -ne 0 ]; then
	echo ""
	echo "ERROR: $violations source/build file(s) contain non-ASCII characters."
	echo "Replace them with ASCII equivalents (see CLAUDE.md 'No Unicode Characters')."
	echo "  ~= for approx, != not-equal, <= >= for inequalities, -> for arrow,"
	echo "  -- for em-dash, +/- for plus-minus, pi for greek pi, ^2 for superscript, etc."
	exit 1
fi

echo "OK: all source/build/script files are pure ASCII."
