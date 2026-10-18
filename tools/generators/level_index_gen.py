#!/usr/bin/env python3
# level_index_gen.py: reference vectors for the level-index kernel (#1349 part 2, #1693).
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT
#
# This file is part of the universal numbers project, which is released under an MIT Open Source license.
#
# Writes include/sw/universal/verification/level_index_vectors.hpp:
#
#   arithmetic  pairs x >= y >= 0 of psi values on the grids of lindex<32,3> (fbits 28)
#               and lindex<64,3> (fbits 60), each given as its integer encoding, with the
#               exact psi(phi(x) + phi(y)) and psi(phi(x) - phi(y)).
#   psi         doubles X with the exact psi(X)
#   phi         psi values x (as doubles) with the exact phi(x)
#
# Every exact value is written as double limbs, peeled off a 600-bit result one double at
# a time, so a test can measure an error far below a double's own rounding.
#
# No value of a level-5..7 operand fits in any floating-point format, mpmath's included:
# phi(5) has 1.6 million decimal digits in its EXPONENT.  So the arithmetic is evaluated
# the way the design doc prescribes (docs/level-index-design.md, "Oracle"): through the
# Clenshaw-Olver a/b/c sequences in the psi domain, which only ever hold quantities in
# [0, 2].  These are the plain sequences of [C9] at 600 bits -- deliberately NOT the
# expm1 form the kernel uses for subtraction, so the oracle and the kernel do not share a
# formulation.  Where psi <= 4.6 the sequence result is also checked against direct
# evaluation psi(phi(x) +- phi(y)) in the value domain, an independent route.
#
# The one approximation: an a_j below 2^-CUTOFF_BITS is taken as 0, because the next term
# exp(-1/a_j) would need an exponent too large to compute.  The true a_(j-1) is then below
# exp(-2^CUTOFF_BITS), and everything a zeroed a_j touches moves psi by less than
# 2^-(CUTOFF_BITS - 16) (see bounded_by_cutoff below), far under the 2^-200 the vectors
# are checked to.
#
# Usage:  python3 tools/generators/level_index_gen.py > include/sw/universal/verification/level_index_vectors.hpp
#
# Requires mpmath.  Not run during the build; the output is checked in.
import random
import sys
import mpmath as mp

mp.mp.prec = 600
CUTOFF_BITS = 1500
LIMBS = 3                     # ~159 bits: enough to measure a dd_cascade result at fbits 60
LEVELS = 8                    # lbits = 3
TOL = mp.mpf(2) ** -200       # agreement demanded of the two routes


def phi(x):
    x = mp.mpf(x)
    return x if x < 1 else mp.exp(phi(x - 1))


def psi(X):
    X = mp.mpf(X)
    level = 0
    while X >= 1:
        X = mp.log(X)
        level += 1
    return level + X


def relevel(w, n):
    """the psi whose level-n part phi(psi - n) is the value w >= 0"""
    while w >= 1:
        w = mp.log(w)
        n += 1
    return n + w


def tiny(a):
    return a < mp.mpf(2) ** -CUTOFF_BITS


def sequences(x, y, subtract):
    """psi(phi(x) +- phi(y)) for x >= y >= 0 by the Clenshaw-Olver sequences"""
    if subtract and x == y:
        return mp.mpf(0)
    l = int(mp.floor(x))
    f = x - l
    m = int(mp.floor(y))
    g = y - m
    if l == 0:
        return relevel(x - y if subtract else x + y, 0)
    a = [mp.mpf(0)] * l
    a[l - 1] = mp.exp(-f)
    for j in range(l - 1, 0, -1):
        a[j - 1] = mp.mpf(0) if tiny(a[j]) else mp.exp(-1 / a[j])
    if m == 0:
        b0 = a[0] * g
    else:
        b = a[m - 1] * mp.exp(g)
        for j in range(m - 1, 0, -1):
            b = mp.mpf(0) if tiny(a[j]) else mp.exp(-(1 - b) / a[j])
        b0 = b
    c = 1 - b0 if subtract else 1 + b0
    for j in range(0, l):
        if j > 0:
            c = 1 + a[j] * mp.log(c)
        if c < a[j]:
            return j + c / a[j]
    return relevel(f + mp.log(c), l)


def bounded_by_cutoff(x):
    """True when no a_j of x is zeroed, so the sequence result is exact to working precision"""
    l = int(mp.floor(x))
    a = mp.exp(-(x - l))
    for _ in range(l - 1, 0, -1):
        if tiny(a):
            return False
        a = mp.exp(-1 / a)
    return True


def checked(x, y, subtract):
    z = sequences(x, y, subtract)
    # the value-domain route, wherever the values themselves are computable
    if x < mp.mpf('4.6'):
        X, Y = phi(x), phi(y)
        direct = psi(X - Y) if subtract else psi(X + Y)
        if abs(direct - z) > TOL:
            sys.exit('sequence and direct evaluation disagree at x=%s y=%s %s: %s'
                     % (mp.nstr(x, 20), mp.nstr(y, 20), '-' if subtract else '+',
                        mp.nstr(abs(direct - z), 5)))
    return z


def limbs(v, n=LIMBS):
    out = []
    r = mp.mpf(v)
    for _ in range(n):
        d = float(r)
        out.append(d)
        r -= mp.mpf(d)
    return out


def hexd(d):
    return d.hex() if d != 0.0 else '0.0'


def limb_literal(v):
    return '{ ' + ', '.join(hexd(d) for d in limbs(v)) + ' }'


def pairs(fbits, rng):
    """structured pairs of encodings x >= y >= 0, per level"""
    S = 1 << fbits
    top = LEVELS * S - 1
    out = []

    def add(x, y):
        if 0 <= y <= x <= top:
            out.append((x, y))

    for l in range(LEVELS):
        lo, hi = l * S, (l + 1) * S - 1
        for _ in range(14):                                   # anywhere below x
            x = rng.randint(lo, hi)
            add(x, rng.randint(0, x))
        for _ in range(8):                                    # the same level
            x = rng.randint(lo, hi)
            add(x, rng.randint(lo, x))
        for k in (1, 2, 3, 1 << 5, 1 << (fbits // 2)):        # near cancellation, same level
            x = rng.randint(lo + k, hi)
            add(x, x - k)
        if l >= 1:
            for k in (1, 2, 1 << 5):                          # straddling the level boundary
                add(lo + k, lo - k)
            for k in (0, 1, 7):                               # y one level down, just below x
                x = rng.randint(lo, hi)
                add(x, x - S + k)
            add(lo, rng.randint(0, lo))                       # x on the boundary
            add(lo, lo - 1)
        x = rng.randint(lo, hi)
        add(x, 0)                                             # y = 0
        add(x, x)                                             # y = x: x - x = 0, x + x
    for l in range(LEVELS):                                   # maxpos against every level
        add(top, rng.randint(l * S, (l + 1) * S - 1))
    return out


def emit_arithmetic(fbits, name, rng):
    S = mp.mpf(2) ** fbits
    print('// fbits = %d: x and y as encodings (psi = encoding * 2^-%d), then the exact psi' % (fbits, fbits))
    print('// of phi(x) + phi(y) and of phi(x) - phi(y)')
    print('constexpr ArithmeticVector %s[] = {' % name)
    worst_gap = 0
    for x, y in pairs(fbits, rng):
        xs, ys = mp.mpf(x) / S, mp.mpf(y) / S
        zp = checked(xs, ys, False)
        zm = checked(xs, ys, True)
        if not bounded_by_cutoff(xs):
            worst_gap += 1
        print('\t{ 0x%xull, 0x%xull, %s, %s },' % (x, y, limb_literal(zp), limb_literal(zm)))
    print('};')
    print()
    return worst_gap


def emit_psi(rng):
    xs = [0.0, 0.5, 1.0, 2.0, 1.0 - 2.0 ** -53, 1.0 + 2.0 ** -52, 1.7976931348623157e308]
    for boundary in (2, 3, 4):                                # e, phi(3), phi(4)
        b = float(phi(boundary))
        xs += [b, float(mp.mpf(b) * (1 - mp.mpf(2) ** -52)), float(mp.mpf(b) * (1 + mp.mpf(2) ** -52))]
    for _ in range(30):
        xs.append(rng.random())                               # level 0
    for _ in range(50):
        xs.append(float(mp.mpf(2) ** rng.uniform(0, 1023.9)))  # levels 1..4
    print('// psi(X) for doubles X')
    print('constexpr PsiVector psi_vectors[] = {')
    for X in xs:
        print('\t{ %s, %s },' % (hexd(X), limb_literal(psi(mp.mpf(X)))))
    print('};')
    print()


def emit_phi(rng):
    limit = float(psi(mp.mpf(1.7976931348623157e308)))
    xs = [0.0, 0.5, 1.0, 2.0, 3.0, 4.0, 1.0 - 2.0 ** -52, 2.0 - 2.0 ** -51, 3.0 - 2.0 ** -51, 4.0 - 2.0 ** -50]
    for level in range(5):
        for _ in range(16):
            x = level + rng.random()
            if x < limit:
                xs.append(x)
    print('// phi(x) for psi values x below psi(DBL_MAX) = %.10f' % limit)
    print('constexpr PhiVector phi_vectors[] = {')
    for x in xs:
        print('\t{ %s, %s },' % (hexd(x), limb_literal(phi(mp.mpf(x)))))
    print('};')
    print()


def main():
    rng = random.Random(1693)
    print('#pragma once')
    print('// level_index_vectors.hpp: reference values for the level-index kernel (#1693)')
    print('//')
    print('// Copyright (C) 2017 Stillwater Supercomputing, Inc.')
    print('// SPDX-License-Identifier: MIT')
    print('//')
    print('// This file is part of the universal numbers project, which is released under an MIT Open Source license.')
    print('//')
    print('// GENERATED by tools/generators/level_index_gen.py from mpmath at %d bits: do not edit.' % mp.mp.prec)
    print('// Each exact value is %d double limbs (~%d bits). The arithmetic is evaluated in the psi' % (LIMBS, 53 * LIMBS))
    print('// domain by the Clenshaw-Olver sequences; see the generator for how levels 5..7 are reached.')
    print('#include <cstdint>')
    print()
    print('namespace sw { namespace universal { namespace level_index_verification {')
    print()
    print('struct ArithmeticVector {')
    print('\tstd::uint64_t x, y;                 // encodings: psi = encoding * 2^-fbits')
    print('\tdouble sum[%d], difference[%d];      // psi(phi(x) + phi(y)), psi(phi(x) - phi(y))' % (LIMBS, LIMBS))
    print('};')
    print('struct PsiVector { double X; double psi[%d]; };' % LIMBS)
    print('struct PhiVector { double x; double phi[%d]; };' % LIMBS)
    print()
    cut28 = emit_arithmetic(28, 'arithmetic_fbits28', rng)
    cut60 = emit_arithmetic(60, 'arithmetic_fbits60', rng)
    emit_psi(rng)
    emit_phi(rng)
    print('}}}  // namespace sw::universal::level_index_verification')
    sys.stderr.write('pairs that reached the a_j cutoff: %d (fbits 28), %d (fbits 60)\n' % (cut28, cut60))


if __name__ == '__main__':
    main()
