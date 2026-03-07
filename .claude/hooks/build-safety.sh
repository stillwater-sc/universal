#!/bin/bash
# build-safety.sh — PreToolUse hook for Bash commands
# Prevents concurrent builds and excessive parallelism.
#
# Reads tool input from stdin (JSON with .tool_input.command field).
# Exit 0 = allow, Exit 2 = block (stderr shown to Claude).

# Read the JSON input from stdin and extract the command using python3
input=$(cat)
cmd=$(echo "$input" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('tool_input',{}).get('command',''))" 2>/dev/null)

# If we couldn't parse or no command, allow
if [ -z "$cmd" ]; then
    exit 0
fi

# Only check commands that invoke make or cmake --build
if ! echo "$cmd" | grep -qE '\b(make|cmake --build)\b'; then
    exit 0
fi

# Check if a build process is already running (make, cmake, or ninja)
if pgrep -x 'make|cmake|ninja' > /dev/null 2>&1; then
    echo "BLOCKED: A build process is already running." >&2
    echo "Check with: pgrep -a 'make|cmake|ninja'" >&2
    echo "Wait for it to finish before starting another build." >&2
    exit 2
fi

# Check for excessive parallelism (both -j and --parallel, use last occurrence)
if echo "$cmd" | grep -qE '(^|[[:space:]])(-j[[:space:]]*[0-9]+|--parallel(=|[[:space:]])[0-9]+)'; then
    jobs=$(echo "$cmd" \
        | grep -oE '(-j[[:space:]]*[0-9]+|--parallel(=|[[:space:]])[0-9]+)' \
        | tail -1 \
        | grep -oE '[0-9]+' \
        | tail -1)
    if [ "$jobs" -gt 8 ]; then
        echo "BLOCKED: -j$jobs is too aggressive. Use -j4 or -j8 max." >&2
        echo "High parallelism caused a load=400 incident previously." >&2
        exit 2
    fi
fi

# Check for -j $(nproc), -j$(nproc), --parallel $(nproc), --parallel=$(nproc)
if echo "$cmd" | grep -qE '(-j\s*|--parallel[= ])\$\(nproc\)'; then
    echo "BLOCKED: -j \$(nproc) / --parallel \$(nproc) is too aggressive. Use -j4 or -j8 max." >&2
    echo "High parallelism caused a load=400 incident previously." >&2
    exit 2
fi

exit 0
