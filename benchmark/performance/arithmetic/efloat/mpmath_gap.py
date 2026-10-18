#!/usr/bin/env python3
# mpmath_gap.py: mpmath's cost per operation at 128, 608 and 2048 bits, the same table
# mpmath_gap.cpp prints for efloat (issue #1714).
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT
#
# This file is part of the universal numbers project, which is released under an MIT Open Source license.
#
# The comparison in #1714 is against mpmath's pure-Python backend; the backend is printed, and
# MPMATH_NOGMPY=1 in the environment forces it when gmpy is installed.
#
# Usage:  python3 benchmark/performance/arithmetic/efloat/mpmath_gap.py
import timeit

import mpmath
from mpmath import mp, mpf

A = "0.7182818284590452353602874713526624977572470936999595749669676277240766303535475945713821785251664274"
B = "1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276415727"


def microseconds(f, reps):
    return timeit.timeit(f, number=reps) / reps * 1e6


def main():
    print(f"mpmath {mpmath.__version__}, backend {mpmath.libmp.BACKEND}: cost per operation in microseconds")
    print(f"{'bits':>6} {'a+b':>9} {'a*b':>9} {'b/a':>9} {'a/7':>9} {'exp':>10} {'log':>10} {'sin':>10} {'sqrt':>9}")
    for bits in (128, 608, 2048):
        mp.prec = bits
        a, b = mpf(A), mpf(B)
        seven = mpf(7)
        x = 1 + a
        row = [
            microseconds(lambda: a + b, 20000),
            microseconds(lambda: a * b, 20000),
            microseconds(lambda: b / a, 20000),
            microseconds(lambda: a / seven, 20000),
            microseconds(lambda: mp.exp(a), 2000),
            microseconds(lambda: mp.log(x), 2000),
            microseconds(lambda: mp.sin(a), 2000),
            microseconds(lambda: mp.sqrt(b), 2000),
        ]
        print(f"{bits:>6} " + " ".join(f"{v:9.2f}" if i < 4 else (f"{v:10.1f}" if i < 7 else f"{v:9.1f}") for i, v in enumerate(row)))


if __name__ == "__main__":
    main()
