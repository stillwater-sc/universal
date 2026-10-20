#!/usr/bin/env python3
# mpmath_gap.py: mpmath's and gmpy2's cost per operation at 128, 512, 2048 and 8192 bits, the
# same table mpmath_gap.cpp prints for efloat (#1714, #1763).
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT
#
# This file is part of the universal numbers project, which is released under an MIT Open Source license.
#
# The comparison in #1714 is against mpmath's pure-Python backend; the backend is printed, and
# MPMATH_NOGMPY=1 in the environment forces it when gmpy is installed.
#
# When gmpy2 is importable, a second table times its MPFR functions directly (gmpy2.exp etc.
# at get_context().precision), the target of #1763. Those figures include the Python call,
# about 0.04 us, which is most of the cost of a+b and of everything at 128 bits.
#
# Usage:  python3 benchmark/performance/arithmetic/efloat/mpmath_gap.py
import timeit

import mpmath
from mpmath import mp, mpf

A = "0.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274"
B = "1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276415727"

PRECISIONS = (128, 512, 2048, 8192)
HEADER = f"{'bits':>6} {'a+b':>9} {'a*b':>9} {'b/a':>9} {'a/7':>9} {'exp':>10} {'log':>10} {'sin':>10} {'sqrt':>9}"


def microseconds(f, reps):
    return timeit.timeit(f, number=reps) / reps * 1e6


def repetitions(bits):
    # (arithmetic, functions): fewer at high precision, so the 8192-bit row takes seconds, not minutes
    scale = max(1, (bits // 512) ** 2)
    return max(200, 20000 // scale), max(20, 2000 // scale)


def report(bits, row):
    print(f"{bits:>6} " + " ".join(f"{v:9.2f}" if i < 4 else (f"{v:10.2f}" if i < 7 else f"{v:9.2f}") for i, v in enumerate(row)))


def time_row(add, mul, div, div7, exp, log, sin, sqrt, bits):
    n, m = repetitions(bits)
    exp(), log(), sin()   # constants (pi, ln 2) are cached at first use: time the steady state
    return [
        microseconds(add, n), microseconds(mul, n), microseconds(div, n), microseconds(div7, n),
        microseconds(exp, m), microseconds(log, m), microseconds(sin, m), microseconds(sqrt, m),
    ]


def mpmath_table():
    print(f"mpmath {mpmath.__version__}, backend {mpmath.libmp.BACKEND}: cost per operation in microseconds")
    print(HEADER)
    for bits in PRECISIONS:
        mp.prec = bits
        a, b = mpf(A), mpf(B)
        seven = mpf(7)
        x = 1 + a
        report(bits, time_row(lambda: a + b, lambda: a * b, lambda: b / a, lambda: a / seven,
                              lambda: mp.exp(a), lambda: mp.log(x), lambda: mp.sin(a), lambda: mp.sqrt(b), bits))


def gmpy2_table():
    try:
        import gmpy2
    except ImportError:
        print("\ngmpy2 not installed: no MPFR table (pip install gmpy2)")
        return
    print(f"\ngmpy2 {gmpy2.version()}, {gmpy2.mp_version()}, {gmpy2.mpfr_version()}: cost per operation in microseconds")
    print(HEADER)
    for bits in PRECISIONS:
        gmpy2.get_context().precision = bits
        a, b = gmpy2.mpfr(A), gmpy2.mpfr(B)
        seven = gmpy2.mpfr(7)
        x = 1 + a
        report(bits, time_row(lambda: a + b, lambda: a * b, lambda: b / a, lambda: a / seven,
                              lambda: gmpy2.exp(a), lambda: gmpy2.log(x), lambda: gmpy2.sin(a), lambda: gmpy2.sqrt(b), bits))


def main():
    mpmath_table()
    gmpy2_table()


if __name__ == "__main__":
    main()
