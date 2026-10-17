#!/usr/bin/env python3
# lindex_conversion_gen.py: conversion vectors for lindex<nbits, lbits> (#1349 part 3, #1694).
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT
#
# This file is part of the universal numbers project, which is released under an MIT Open Source license.
#
# Writes include/sw/universal/verification/lindex_conversion_vectors.hpp:
#
#   from native  doubles, signed and unsigned 64-bit integers, each with the correctly
#                rounded encoding in every configuration below
#   to native    encodings of every configuration, each with the correctly rounded double
#                and float of its value, or +-inf where the value is past that type
#
# The encoding of a value X is round-to-nearest-even of psi(|X|) on the 2^-fbits grid,
# saturated at maxpos, and negated as the two's complement of the whole pattern; see
# docs/level-index-design.md.  psi is evaluated directly at 600 bits: a native value
# never reaches level 5, so no value is too large for mpmath here.  The value of an
# encoding at level 5 or above is past every native type, and is written as inf without
# being formed.
#
# The vectors are the oracle, so the test compares encodings and doubles for equality and
# never round trips: a conversion that is wrong in both directions would pass a round trip.
#
# Usage:  python3 tools/generators/lindex_conversion_gen.py > include/sw/universal/verification/lindex_conversion_vectors.hpp
#
# Requires mpmath.  Not run during the build; the output is checked in.
import random
import mpmath as mp

mp.mp.prec = 600

# (nbits, lbits), in the order of the enc[] columns
CONFIGS = [(8, 3), (16, 3), (32, 3), (64, 3), (16, 2), (16, 4)]

DBL_MAX = mp.mpf(2) ** 1023 * (2 - mp.mpf(2) ** -52)
FLT_MAX = mp.mpf(2) ** 127 * (2 - mp.mpf(2) ** -23)


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


def round_half_even(t):
    """Round the exact (or 600-bit) t >= 0 to the nearest integer, ties to even."""
    fl = int(mp.floor(t))
    rem = t - fl
    if rem > 0.5 or (rem == 0.5 and fl % 2 == 1):
        fl += 1
    return fl


def magnitude_of(X, nbits, lbits):
    """Encode |X| as the magnitude bits of lindex<nbits, lbits>."""
    fbits = nbits - 1 - lbits
    X = abs(mp.mpf(X))
    if X == 0:
        return 0
    mag = round_half_even(psi(X) * mp.mpf(2) ** fbits)
    return min(mag, (1 << (nbits - 1)) - 1)       # saturate at maxpos


def encoding_of(X, nbits, lbits):
    """Encode X, negatives as the two's complement of the whole pattern."""
    mag = magnitude_of(X, nbits, lbits)
    if mp.mpf(X) < 0 and mag != 0:
        return ((1 << nbits) - mag) & ((1 << nbits) - 1)
    return mag


def value_of(mag, nbits, lbits):
    """phi of a magnitude encoding at 600 bits, or None when it is past every native type."""
    fbits = nbits - 1 - lbits
    level = mag >> fbits
    f = mp.mpf(mag & ((1 << fbits) - 1)) / mp.mpf(2) ** fbits
    if level <= 3:
        return phi(level + f)
    if level == 4:
        L = phi(3 + f)
        return mp.exp(L) if L < 720 else None   # ln(DBL_MAX) = 709.78
    return None


def rounded(v, prec, vmax):
    """v rounded to nearest at prec bits, or None for an overflow."""
    if v is None:
        return None
    with mp.workprec(prec):
        r = +v
    return None if r > vmax else r


def double_literal(r, negative):
    if r is None:
        return '-inf_' if negative else 'inf_'
    d = float(r)
    d = -d if negative else d
    return d.hex() if d != 0.0 else ('-0.0' if negative else '0.0')


def float_literal(r, negative):
    if r is None:
        return '-float(inf_)' if negative else 'float(inf_)'
    d = float(r)
    d = -d if negative else d
    return (d.hex() + 'f') if d != 0.0 else '0.0f'


def enc_list(X):
    return '{ ' + ', '.join('0x%xull' % encoding_of(X, n, l) for n, l in CONFIGS) + ' }'


def doubles(rng):
    xs = [0.0, 1.0, -1.0, 0.5, 2.0, 0.75, 1.0 - 2.0 ** -53, 1.0 + 2.0 ** -52,
          2.0 ** -60, 2.0 ** -61, 3 * 2.0 ** -61, 2.0 ** -62, 5e-324, 2.2250738585072014e-308,
          1.7976931348623157e308, -1.7976931348623157e308]
    # the level boundaries phi(2), phi(3), phi(4) and their neighbouring doubles
    for b in (2, 3, 4):
        d = float(phi(b))
        xs += [d, float(mp.mpf(d) * (1 - mp.mpf(2) ** -52)), float(mp.mpf(d) * (1 + mp.mpf(2) ** -52))]
    # exact ties at level 0, where psi is the identity: (2k + 1) / 2^(fbits + 1)
    for n, l in CONFIGS:
        fbits = n - 1 - l
        for k in (0, 1, 2, 5):
            xs.append((2 * k + 1) * 2.0 ** -(fbits + 1))
    for _ in range(60):
        xs.append(rng.random())                                   # level 0
    for _ in range(160):
        xs.append(2.0 ** rng.uniform(-70, 1023.9))                # underflow .. level 4
    for _ in range(30):
        xs.append(-2.0 ** rng.uniform(-30, 1023.9))               # negatives
    return xs


def signed_integers():
    return [0, 1, -1, 2, 3, 15, 16, -16, 1000, 3814279, 3814280, (1 << 53) + 1,
            -((1 << 53) + 1), (1 << 62) + 12345, (1 << 63) - 1, -(1 << 63)]


def unsigned_integers():
    return [0, 1, 7, (1 << 63) + 1, (1 << 64) - 2048, (1 << 64) - 1]


def encodings(nbits, lbits, rng):
    """Encodings to convert to native: every one of an 8-bit type, a sample otherwise."""
    fbits = nbits - 1 - lbits
    top = (1 << (nbits - 1)) - 1
    if nbits <= 8:
        mags = list(range(0, top + 1))
    else:
        mags = set()
        for level in range(1 << lbits):                           # around every level boundary
            lo = level << fbits
            mags.update(m for m in (lo - 1, lo, lo + 1) if 0 <= m <= top)
        mags.update((1, top))
        if nbits <= 16:
            mags.update(range(0, top + 1, 61))
        else:
            mags.update(rng.randint(0, top) for _ in range(250))
            # levels 0..4, where the value is finite, get extra weight
            mags.update(rng.randint(0, (5 << fbits) - 1) for _ in range(250))
        if fbits > 53:
            mags.update(float_midpoints(nbits, lbits))
        mags = sorted(mags)
    out = []
    for m in mags:
        out.append((m, False))
        if m != 0 and m % 3 == 0:                                 # a third of them negated too
            out.append((m, True))
    return out


def float_midpoints(nbits, lbits):
    """Encodings whose value lies within 2^-60 of a midpoint between two floats.

    The double nearest such a value is the midpoint itself, so converting the double to
    float would round twice; a correct conversion must look past it to the low limb.
    Only an index wider than a double resolves values that close.  A separate generator
    keeps the rest of the sample as it was.
    """
    fbits = nbits - 1 - lbits
    rng = random.Random(16941)
    mags = set()
    for _ in range(40):
        level = rng.randint(1, 3)
        lo, hi = phi(level), phi(level + 1)
        with mp.workprec(24):
            f = +(lo + (hi - lo) * mp.mpf(rng.random()))
        ulp = mp.mpf(2) ** (mp.floor(mp.log(f, 2)) - 23)
        mid = f + ulp / 2
        enc = round_half_even(psi(mid) * mp.mpf(2) ** fbits)
        mags.update((enc - 1, enc, enc + 1))
    # the case the review of #1708 found: exp(68719474689 * 2^-60) is just above 1 + 2^-24
    mags.add(0x1000000ffffff801)
    return mags


def emit_to_native(nbits, lbits, rng):
    name = 'to_native_%d_%d' % (nbits, lbits)
    print('// lindex<%d,%d>: an encoding, its correctly rounded double and float' % (nbits, lbits))
    print('constexpr ToNativeVector %s[] = {' % name)
    for m, negative in encodings(nbits, lbits, rng):
        enc = (((1 << nbits) - m) & ((1 << nbits) - 1)) if negative else m
        v = value_of(m, nbits, lbits)
        print('\t{ 0x%xull, %s, %s },' % (enc, double_literal(rounded(v, 53, DBL_MAX), negative),
                                         float_literal(rounded(v, 24, FLT_MAX), negative)))
    print('};')
    print()


def main():
    rng = random.Random(1694)
    print('#pragma once')
    print('// lindex_conversion_vectors.hpp: reference conversions for lindex (#1694)')
    print('//')
    print('// Copyright (C) 2017 Stillwater Supercomputing, Inc.')
    print('// SPDX-License-Identifier: MIT')
    print('//')
    print('// This file is part of the universal numbers project, which is released under an MIT Open Source license.')
    print('//')
    print('// GENERATED by tools/generators/lindex_conversion_gen.py from mpmath at %d bits: do not edit.' % mp.mp.prec)
    print('// enc[] holds one encoding per configuration, in the order of lindex_configs below.')
    print('#include <cstdint>')
    print('#include <limits>')
    print()
    print('namespace sw { namespace universal { namespace lindex_verification {')
    print()
    print('constexpr double inf_ = std::numeric_limits<double>::infinity();')
    print()
    print('struct Config { unsigned nbits, lbits; };')
    print('constexpr Config lindex_configs[] = { %s };' % ', '.join('{ %d, %d }' % c for c in CONFIGS))
    print('constexpr unsigned nr_configs = %d;' % len(CONFIGS))
    print()
    print('struct FromDoubleVector   { double x;             std::uint64_t enc[%d]; };' % len(CONFIGS))
    print('struct FromSignedVector   { std::int64_t x;       std::uint64_t enc[%d]; };' % len(CONFIGS))
    print('struct FromUnsignedVector { std::uint64_t x;      std::uint64_t enc[%d]; };' % len(CONFIGS))
    print('struct ToNativeVector     { std::uint64_t enc;    double d; float f; };')
    print()
    print('constexpr FromDoubleVector from_double[] = {')
    for x in doubles(rng):
        print('\t{ %s, %s },' % (x.hex() if x != 0.0 else '0.0', enc_list(x)))
    print('};')
    print()
    print('constexpr FromSignedVector from_signed[] = {')
    for x in signed_integers():
        # INT64_MIN has no literal of its own: write it as an expression
        lit = '(-0x7fffffffffffffffll - 1)' if x == -(1 << 63) else ('%dll' % x)
        print('\t{ %s, %s },' % (lit, enc_list(x)))
    print('};')
    print()
    print('constexpr FromUnsignedVector from_unsigned[] = {')
    for x in unsigned_integers():
        print('\t{ 0x%xull, %s },' % (x, enc_list(x)))
    print('};')
    print()
    for n, l in CONFIGS:
        emit_to_native(n, l, rng)
    print('}}}  // namespace sw::universal::lindex_verification')


if __name__ == '__main__':
    main()
