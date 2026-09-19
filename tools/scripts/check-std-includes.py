#!/usr/bin/env python3
#
# check-std-includes.py - every header includes the standard headers it uses.
#
# A header that uses std::string, a string stream, an iomanip manipulator or a
# standard stream object without including the header that declares it compiles
# only because something else in the include graph happens to pull that header in.
# It breaks the day an unrelated include moves -- and often on one platform only:
# the RISC-V cross build broke in #1386 because riscv_long_double.hpp used
# std::cerr with no <iostream>, and on x86 an #if selects a different file, so gcc
# and clang never saw it. The header-layering work (#1334) moves includes routinely,
# which is why this is checked rather than hoped for (#1389).
#
# The rule is deliberately literal: a header that names one of the facilities
# below on a line that is not a // comment must itself contain the matching
# #include. Transitive satisfaction does not count; that is the whole point.
#
# Usage:
#   tools/scripts/check-std-includes.py              # scan include/sw
#   tools/scripts/check-std-includes.py file ...     # scan specific headers
#
# Exits 1 and lists each header with what it is missing. Wired into CI next to
# check-ascii (.github/workflows/ascii-guard.yml).

import glob
import re
import sys

# standard header -> the uses that require it
REQUIRED = {
    "<string>":   re.compile(r"\bstd::string\b"),
    "<sstream>":  re.compile(r"\bstd::(stringstream|ostringstream|istringstream)\b"),
    "<iomanip>":  re.compile(r"\bstd::(setw|setprecision|setfill|setbase|quoted)\b"),
    "<iostream>": re.compile(r"\bstd::(cout|cerr|clog|cin)\b"),
}


def code_lines(text):
    # drop // comments (whole-line and trailing); block comments are rare in the tree
    # and a use inside one is at worst a false positive that an include silences
    for line in text.split("\n"):
        stripped = line.strip()
        if stripped.startswith("//"):
            continue
        yield line.split("//", 1)[0]


def missing_includes(path):
    text = open(path, encoding="utf-8", errors="replace").read()
    included = set(re.findall(r"^\s*#\s*include\s*(<[^>]+>)", text, flags=re.M))
    code = list(code_lines(text))
    return [hdr for hdr, use in REQUIRED.items()
            if hdr not in included and any(use.search(line) for line in code)]


def main(argv):
    paths = argv[1:] or sorted(glob.glob("include/sw/**/*.hpp", recursive=True))
    if not paths:
        print("error: no headers found -- run from the repository root", file=sys.stderr)
        return 2
    violations = 0
    for path in paths:
        miss = missing_includes(path)
        if miss:
            violations += 1
            print(f"{path}: uses {', '.join(miss)} without including it")
    if violations:
        print()
        print(f"ERROR: {violations} header(s) use a standard facility without including its header.")
        print("Add the #include to the header itself; relying on another header to pull it in")
        print("breaks when that header changes (#1386, #1389).")
        return 1
    print(f"OK: {len(paths)} headers include the standard headers they use.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
