#!/usr/bin/env python3
#
# check-ungated-throws.py - every throw in the library compiles away without exceptions.
#
# Firmware and other bare-metal targets build with -fno-exceptions, and a header that
# compiles a `throw`, `try` or `catch` there fails to build even when nothing calls it
# (#1925). The integration/noexceptions test builds every number system that way, but it
# only reaches the functions it instantiates: a throw in a function template nobody calls
# compiles there and still breaks the application that does call it. This scan covers the
# rest. In the library headers each of those keywords must be either
#
#   - inside a preprocessor branch whose condition tests an exception macro
#     (a *_THROW_ARITHMETIC_EXCEPTION or *_THROW_DIVIDE_BY_ZERO switch, which
#     utility/exceptions.hpp rejects with #error in a build without exceptions, or
#     UNIVERSAL_HAS_EXCEPTIONS itself), or
#   - replaced by throw_or_abort(e), which throws when it can and aborts otherwise.
#
# The #else of such a branch is the non-throwing side, so a throw there is reported.
#
# Usage:
#   tools/scripts/check-ungated-throws.py              # scan the library headers
#   tools/scripts/check-ungated-throws.py file ...     # scan specific headers
#
# Exits 1 and lists each ungated site. Wired into CI next to check-ascii
# (.github/workflows/ascii-guard.yml).

import glob
import os
import re
import sys

# the headers an application includes: the number systems and their internal storage
ROOTS = ["include/sw/universal/number", "include/sw/universal/internal"]
# headers nothing includes, kept for reference
SKIP = {"include/sw/universal/internal/bitblock/ubb.h",
        "include/sw/universal/internal/bitblock/bitblock_v2.hpp"}

GATE = re.compile(r"THROW_ARITHMETIC_EXCEPTION|THROW_DIVIDE_BY_ZERO|UNIVERSAL_HAS_EXCEPTIONS")
KEYWORD = re.compile(r"\b(throw|try|catch)\b")
DIRECTIVE = re.compile(r"#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)")


def strip_comments_and_strings(text):
    # replace comments, string and character literals with spaces, keeping the newlines
    # so that line numbers survive
    out = []
    i, n = 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i))
            i = j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append("".join(ch if ch == "\n" else " " for ch in text[i:j]))
            i = j
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(c + " " * (j - i - 2) + c if j - i >= 2 else c)
            i = j
        else:
            out.append(c)
            i += 1
    return "".join(out)


def gated(condition, positive):
    # a branch throws only when its condition says exceptions are on: the positive side
    # of a test of an exception macro, not its #else or a #if !... / #ifndef
    if not GATE.search(condition):
        return False
    negated = condition.lstrip().startswith("!") or condition.startswith("ndef")
    return positive != negated


def scan(path):
    text = strip_comments_and_strings(open(path, encoding="utf-8", errors="replace").read())
    stack = []  # one entry per open #if: (condition, on its positive branch)
    sites = []
    for number, line in enumerate(text.split("\n"), 1):
        stripped = line.strip()
        m = DIRECTIVE.match(stripped)
        if m:
            kind, condition = m.group(1), m.group(2)
            if kind in ("if", "ifdef"):
                stack.append((condition, True))
            elif kind == "ifndef":
                stack.append(("ndef" + condition, True))
            elif kind == "elif" and stack:
                stack[-1] = (condition, True)
            elif kind == "else" and stack:
                stack[-1] = (stack[-1][0], False)
            elif kind == "endif" and stack:
                stack.pop()
            continue
        if stripped.startswith("#"):
            continue
        for k in KEYWORD.finditer(line):
            rest = line[k.end():]
            # throw() is a dynamic exception specification, not a throw
            if k.group(1) == "throw" and re.match(r"\s*\(\s*\)", rest):
                continue
            if any(gated(c, p) for c, p in stack):
                continue
            sites.append((number, k.group(1), stripped))
    return sites


def main(argv):
    if len(argv) > 1:
        files = argv[1:]
    else:
        files = []
        for root in ROOTS:
            files += glob.glob(os.path.join(root, "**", "*.hpp"), recursive=True)
            files += glob.glob(os.path.join(root, "**", "*.h"), recursive=True)
    failures = 0
    for path in sorted(files):
        if path.replace(os.sep, "/") in SKIP:
            continue
        for number, keyword, line in scan(path):
            print(f"{path}:{number}: ungated '{keyword}': {line}")
            failures += 1
    if failures:
        print(f"\n{failures} ungated throw/try/catch site(s). Gate each behind an exception macro, "
              "or call throw_or_abort() from <universal/utility/exceptions.hpp> (#1925).")
        return 1
    print(f"check-ungated-throws: {len(files)} headers, every throw is gated")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
