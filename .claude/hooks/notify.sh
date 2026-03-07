#!/bin/bash
# notify.sh — Notification hook for Claude Code
# Sends a notification when Claude needs attention.
# Works across SSH sessions, local terminals, and desktop environments.
#
# Sends:
#   1. OSC 9 escape (iTerm2, Windows Terminal, many modern terminals)
#   2. Terminal bell (\a) — triggers SSH client bell
#   3. notify-send (if desktop session available)

# Read hook input from stdin
input=$(cat)

# Find a writable PTY by walking up the process tree
find_terminal() {
    local pid=$$
    while [ "$pid" -gt 1 ] 2>/dev/null; do
        local fd1
        fd1=$(readlink /proc/$pid/fd/1 2>/dev/null)
        if [ -n "$fd1" ] && echo "$fd1" | grep -q '^/dev/pts/'; then
            if [ -w "$fd1" ]; then
                echo "$fd1"
                return
            fi
        fi
        pid=$(awk '{print $4}' /proc/$pid/stat 2>/dev/null)
    done
}

term=$(find_terminal)

if [ -n "$term" ]; then
    # OSC 9 escape sequence (native notification in modern terminals)
    printf '\033]9;Claude Code needs attention\033\\' > "$term" 2>/dev/null
    # Terminal bell
    printf '\a' > "$term" 2>/dev/null
fi

# Desktop notification (only if a display server is available)
if [ -n "$DISPLAY" ] || [ "$XDG_SESSION_TYPE" = "x11" ] || [ "$XDG_SESSION_TYPE" = "wayland" ]; then
    notify-send "Claude Code" "Needs your attention" --icon=dialog-information 2>/dev/null
fi

exit 0
