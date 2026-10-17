#!/usr/bin/env python3
# lindex_arithmetic_gen.py: arithmetic vectors for lindex<nbits, lbits> (#1349 part 4, #1695).
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT
#
# This file is part of the universal numbers project, which is released under an MIT Open Source license.
#
# Writes include/sw/universal/verification/lindex_arithmetic_vectors.hpp:
#
#   lindex<8,3>  for every pair of magnitude encodings a, b in 0..127, the correctly rounded
#                magnitude of |A| + |B|, ||A| - |B||, |A| * |B| and |A| / |B|.  The test derives
#                all 65536 signed pairs from these with the sign rules.
#   sampled      signed pairs of lindex<12,3>, <16,3>, <32,3>, <64,3> and <16,2>, each with
#                the correctly rounded x + y, x - y, x * y and x / y
#
# The exact psi of a result comes from one of two independent routes:
#
#   direct     the values themselves, psi(phi(x) op phi(y)), wherever mpmath can form them
#              (every operand below psi = 4.6, about 10^210)
#   sequences  the plain Clenshaw-Olver sequences of [C9] in the psi domain, through the
#              level-down identities for * and /:
#                  psi(XY)  = 1 + psi(phi(x - 1) + phi(y - 1))      x, y >= 1
#                  psi(X/Y) = 1 + psi(phi(x - 1) - phi(y - 1))      x >= y >= 1
#              with ln y < 0 standing in for phi(y - 1) when y is at level 0
#
# Where both routes apply they must agree to 2^-200, and the generator stops if they do not.
# The sequences are those of level_index_gen.py (#1693), NOT the expm1 form the kernel
# uses, so the oracle and the implementation do not share a formulation.
#
# A result is rounded to nearest, ties to even, on the 2^-fbits grid, and saturated at
# maxpos.  A tie needs an exact psi on a midpoint, which only a product of two level-0
# operands can produce: it is an exact rational at 600 bits.  A level-0 quotient a / b with
# a < b < 2^fbits cannot tie, since 2^(fbits + 1) a / b odd needs 2^(fbits + 1) to divide b.
#
# Usage:  python3 tools/generators/lindex_arithmetic_gen.py > include/sw/universal/verification/lindex_arithmetic_vectors.hpp
#
# Requires mpmath.  Not run during the build; the output is checked in.
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import level_index_gen as lig   # noqa: E402  (sets mp.prec = 600)
import mpmath as mp             # noqa: E402

DIRECT = mp.mpf('4.6')          # below this every operand value is computable
TOL = mp.mpf(2) ** -200

SAMPLED = [(12, 3), (16, 3), (32, 3), (64, 3), (16, 2)]

# lindex<32,3> magnitude pairs whose double evaluation lands within 2^-49 of a midpoint and
# rounds the wrong way; the dd_cascade retry rounds them correctly.  Found by a sweep of
# 4e8 random pairs (13 of them changed by the retry); they are here so that the retry is
# tested, and every operation is evaluated for each pair, not only the one that needed it.
RETRY_32 = [(0x1b8de406, 0x320255e), (0x35543699, 0x2e054fb4), (0x288be3f8, 0x19276d8a),
            (0x39d8182b, 0x311434be), (0x323d20d5, 0x7f3c7cb), (0x3da9c3bf, 0x22f36817),
            (0x32f489b4, 0x12739648), (0x2091389f, 0x1cbd3c6), (0x23cd7830, 0x13b09c83),
            (0x252a7fe0, 0x16d2706a), (0x3171f5c4, 0x34ad99d), (0x2b570149, 0xda3434b)]


def agree(a, b, what):
    if abs(a - b) > TOL:
        sys.exit('routes disagree for %s: %s vs %s' % (what, mp.nstr(a, 30), mp.nstr(b, 30)))
    return a


def psi_add(x, y):
    """psi(phi(x) + phi(y)) for psi values x, y >= 0"""
    hi, lo = max(x, y), min(x, y)
    return lig.checked(hi, lo, False)


def psi_sub(x, y):
    """psi(|phi(x) - phi(y)|)"""
    hi, lo = max(x, y), min(x, y)
    return lig.checked(hi, lo, True)


def psi_mul(x, y):
    """psi(phi(x) * phi(y))"""
    hi, lo = max(x, y), min(x, y)
    if lo == 0:
        return mp.mpf(0)
    if hi < 1:
        return hi * lo                                        # exact
    direct = lig.psi(lig.phi(hi) * lig.phi(lo)) if hi < DIRECT else None
    if lo < 1:
        lg = -mp.log(lo)                                      # ln(XY) = phi(hi - 1) - lg
        if direct is not None:
            if lig.phi(hi - 1) > lg:                          # XY > 1: the identity applies too
                agree(direct, 1 + lig.sequences(hi - 1, lig.psi(lg), True), 'mul by level 0')
            return direct
        # hi >= 4.6: phi(hi - 1) >= phi(3.6) = 484 > lg, so XY > 1
        return 1 + lig.sequences(hi - 1, lig.psi(lg), True)
    seq = 1 + lig.sequences(hi - 1, lo - 1, False)
    return agree(direct, seq, 'mul') if direct is not None else seq


def psi_div(x, y):
    """psi(phi(x) / phi(y)), y > 0"""
    if x == 0:
        return mp.mpf(0)
    if x < 1 and y < 1:
        return lig.psi(x / y)                                 # a rational; exact when it is a tie
    if y < 1:                                                 # x >= 1: ln(X/Y) = phi(x - 1) + lg
        lg = -mp.log(y)
        direct = lig.psi(lig.phi(x) / y) if x < DIRECT else None
        hi, lo = (x - 1, lig.psi(lg)) if x - 1 >= lig.psi(lg) else (lig.psi(lg), x - 1)
        seq = 1 + lig.sequences(hi, lo, False)
        return agree(direct, seq, 'div by level 0') if direct is not None else seq
    if x < 1:                                                 # y >= 1: X/Y < 1
        if y >= DIRECT:
            return mp.mpf(0)                                  # below 2^-698
        return x / lig.phi(y)
    direct = lig.psi(lig.phi(x) / lig.phi(y)) if (x < DIRECT and y < DIRECT) else None
    if x >= y:
        seq = 1 + lig.sequences(x - 1, y - 1, True) if x > y else mp.mpf(1)
    else:
        # X/Y = e^-D with D = phi(y - 1) - phi(x - 1) > 0, which lands at level 0
        d = lig.sequences(y - 1, x - 1, True)
        seq = mp.exp(-lig.phi(d)) if d < DIRECT else mp.mpf(0)
    return agree(direct, seq, 'div') if direct is not None else seq


def magnitude(z, nbits, lbits):
    """the correctly rounded magnitude encoding of psi z >= 0, saturated"""
    fbits = nbits - 1 - lbits
    t = z * mp.mpf(2) ** fbits
    fl = int(mp.floor(t))
    rem = t - fl
    if rem > 0.5 or (rem == 0.5 and fl % 2 == 1):
        fl += 1
    return min(fl, (1 << (nbits - 1)) - 1)


def psi_of(mag, nbits, lbits):
    return mp.mpf(mag) / mp.mpf(2) ** (nbits - 1 - lbits)


def signed(mag, negative, nbits):
    return ((1 << nbits) - mag) & ((1 << nbits) - 1) if (negative and mag != 0) else mag


def decode(enc, nbits):
    """(magnitude, negative) of a non-NaR encoding"""
    if enc & (1 << (nbits - 1)):
        return ((1 << nbits) - enc) & ((1 << nbits) - 1), True
    return enc, False


def signed_ops(ex, ey, nbits, lbits):
    """x + y, x - y, x * y, x / y as encodings, by the sign rules over the magnitude results"""
    ma, sa = decode(ex, nbits)
    mb, sb = decode(ey, nbits)
    a, b = psi_of(ma, nbits, lbits), psi_of(mb, nbits, lbits)

    def addition(sb_eff):
        if mb == 0:
            return signed(ma, sa, nbits)
        if ma == 0:
            return signed(mb, sb_eff, nbits)
        if sa == sb_eff:
            return signed(magnitude(psi_add(a, b), nbits, lbits), sa, nbits)
        if ma == mb:
            return 0
        larger_negative = sa if ma > mb else sb_eff
        return signed(magnitude(psi_sub(a, b), nbits, lbits), larger_negative, nbits)

    s = addition(sb)
    d = addition(not sb)
    p = signed(magnitude(psi_mul(a, b), nbits, lbits), sa != sb, nbits)
    q = signed(magnitude(psi_div(a, b), nbits, lbits), sa != sb, nbits) if mb != 0 else (1 << (nbits - 1))
    return s, d, p, q


def emit_table8(name, f):
    print('constexpr const char* %s[128] = {' % name)
    for a in range(128):
        print('\t"%s",' % ''.join('%02x' % f(a, b) for b in range(128)))
    print('};')
    print()


def sampled_pairs(nbits, lbits, rng):
    """signed encoding pairs across every level: random, same level, near cancellation,
    straddling and on level boundaries, level 0 against everything, and the extremes"""
    fbits = nbits - 1 - lbits
    S = 1 << fbits
    top = (1 << (nbits - 1)) - 1
    levels = 1 << lbits
    mags = []

    def add(a, b):
        if 0 <= a <= top and 0 <= b <= top:
            mags.append((a, b))

    for l in range(levels):
        lo, hi = l * S, min((l + 1) * S - 1, top)
        for _ in range(6):
            add(rng.randint(lo, hi), rng.randint(0, top))                   # anywhere
        for _ in range(4):
            add(rng.randint(lo, hi), rng.randint(lo, hi))                   # same level
        for _ in range(3):
            add(rng.randint(lo, hi), rng.randint(0, S - 1))                 # against level 0
        for k in (1, 2, 1 << (fbits // 2)):
            x = rng.randint(lo + k, hi)
            add(x, x - k)                                                   # near cancellation
        if l >= 1:
            add(lo, lo - 1)                                                 # across the boundary
            add(lo + 1, lo - 1)
            add(lo, rng.randint(0, top))                                    # on the boundary
            add(lo + S // 2, rng.randint((l - 1) * S, lo - 1))              # one level down
            add(rng.randint(lo, hi), S)                                     # times / over 1.0
    for _ in range(4):
        add(rng.randint(0, S - 1), rng.randint(0, S - 1))                   # level 0 by level 0
    add(top, top)
    add(top, 1)
    add(1, 1)
    add(1, top)
    out = []
    for a, b in mags:
        sa, sb = rng.random() < 0.3, rng.random() < 0.3
        out.append((signed(a, sa, nbits), signed(b, sb, nbits)))
    if (nbits, lbits) == (32, 3):
        out += RETRY_32
    if nbits == 64:
        out += structured_pairs(nbits, lbits)
    return out


def structured_pairs(nbits, lbits):
    """Operands one or two ulps from 1/2, 1 and the level boundaries, against each other.

    Their results have exact rational leading terms -- e^(2^-60) = 1 + 2^-60 + 2^-121 -- that
    put them within 2^-120 of a midpoint, closer than dd_cascade resolves; they are the
    reason for the qd_cascade stage of the rounding retry (#1696).
    """
    fbits = nbits - 1 - lbits
    S = 1 << fbits
    top = (1 << (nbits - 1)) - 1
    anchors = [1, 2, S // 2 - 1, S // 2 + 1]
    for l in range(1, 4):
        anchors += [l * S - 2, l * S - 1, l * S + 1, l * S + 2]
    anchors = sorted(set(m for m in anchors if 0 < m <= top))
    return [(a, b) for a in anchors for b in anchors]


def main():
    rng = random.Random(1695)
    print('#pragma once')
    print('// lindex_arithmetic_vectors.hpp: reference arithmetic for lindex (#1695)')
    print('//')
    print('// Copyright (C) 2017 Stillwater Supercomputing, Inc.')
    print('// SPDX-License-Identifier: MIT')
    print('//')
    print('// This file is part of the universal numbers project, which is released under an MIT Open Source license.')
    print('//')
    print('// GENERATED by tools/generators/lindex_arithmetic_gen.py from mpmath at %d bits: do not edit.' % mp.mp.prec)
    print('#include <cstdint>')
    print()
    print('namespace sw { namespace universal { namespace lindex_verification {')
    print()
    print('// lindex<8,3>: row a, characters 2b and 2b + 1 hold the magnitude encoding (hex) of the')
    print('// result for magnitudes a and b.  Division by 0 is not in the table.')
    emit_table8('lindex8_add', lambda a, b: magnitude(psi_add(psi_of(a, 8, 3), psi_of(b, 8, 3)), 8, 3))
    emit_table8('lindex8_sub', lambda a, b: magnitude(psi_sub(psi_of(a, 8, 3), psi_of(b, 8, 3)), 8, 3))
    emit_table8('lindex8_mul', lambda a, b: magnitude(psi_mul(psi_of(a, 8, 3), psi_of(b, 8, 3)), 8, 3))
    emit_table8('lindex8_div', lambda a, b: magnitude(psi_div(psi_of(a, 8, 3), psi_of(b, 8, 3)), 8, 3) if b else 0)
    print('// signed pairs: x, y, then x + y, x - y, x * y, x / y (NaR for a zero divisor)')
    print('struct ArithmeticVector { std::uint64_t x, y, sum, difference, product, quotient; };')
    print()
    for nbits, lbits in SAMPLED:
        print('constexpr ArithmeticVector arithmetic_%d_%d[] = {' % (nbits, lbits))
        for ex, ey in sampled_pairs(nbits, lbits, rng):
            s, d, p, q = signed_ops(ex, ey, nbits, lbits)
            print('\t{ 0x%xull, 0x%xull, 0x%xull, 0x%xull, 0x%xull, 0x%xull },' % (ex, ey, s, d, p, q))
        print('};')
        print()
    print('}}}  // namespace sw::universal::lindex_verification')


if __name__ == '__main__':
    main()
