#!/usr/bin/env python3
# lindex_math2_gen.py: vectors for the rest of the lindex math library (#1349 part 5b, #1715):
# the trigonometric, hyperbolic, error and gamma functions, atan2, fmod and remainder.
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT
#
# This file is part of the universal numbers project, which is released under an MIT Open Source license.
#
# Writes include/sw/universal/verification/lindex_math2_vectors.hpp:
#
#   lindex<8,3>  for every one of the 256 encodings, the correctly rounded encoding of each
#                unary function; atan2, fmod and remainder for every one of the 65536 pairs
#   sampled      encodings of lindex<16,3>, <32,3> and <64,3> across every level, with every
#                unary function, and pairs with each binary function
#
# Each result is the exact psi of the function value, rounded once to nearest even on the
# psi grid and saturated, from mpmath.  The domain decisions, which math/*.hpp document:
#
#   sin cos tan      NaR past the double range (psi > psi(DBL_MAX) = 4.632): X mod 2 pi is not
#                    determined by anything the format can compute.  Within it the value X is
#                    formed at 1600 bits, enough for X up to 2^1024 and the 2^-fbits grid.
#   asin acos        NaR for |X| > 1
#   acosh            NaR for X < 1;  atanh NaR for |X| >= 1 (there is no infinity)
#   lgamma tgamma    NaR at the poles 0 and -1 (the only negative integer a lindex holds), and
#                    for negative X past the double range (the reflection needs sin(pi X))
#   fmod remainder   NaR for Y = 0, and for |X| >= |Y| with X past the double range
#
# Past psi = 4.6 (DIRECT) the values are not formed; the results come from identities in the
# psi domain, by the plain Clenshaw-Olver sequences of lindex_arithmetic_gen.py:
#
#   ln sinh X, ln cosh X = X - ln 2 + log1p(-+e^-2X)      the correction is below e^-968
#   asinh X, acosh X     = ln X + ln 2 + O(X^-2)           ln X = phi(x - 1)
#   lgamma X             = X (ln X - 1) (1 + O(1/X))       X > 0
#   tgamma X             = e^lgamma X                      one level up when lgamma >= 0
#   atan X               = pi/2 - O(1/X),  tanh, erf -> 1,  erfc -> 0 or 2
#
# A result of the form c - tiny or c + tiny with tiny below e^-484 rounds as c does, which the
# generator asserts by checking that psi(c) lies far from a midpoint.  Where a value-domain
# route and an identity both apply they must agree to 2^-200.  NaR is the encoding 100...0.
#
# Usage:  python3 tools/generators/lindex_math2_gen.py > include/sw/universal/verification/lindex_math2_vectors.hpp
#
# Requires mpmath.  Not run during the build; the output is checked in.
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import lindex_arithmetic_gen as lag   # noqa: E402  (imports level_index_gen, sets mp.prec = 600)
import mpmath as mp                   # noqa: E402

lig = lag.lig
DIRECT = lag.DIRECT
NAR = 'nar'
WIDE = 1600                           # bits for values up to 2^1024 on a 2^-60 grid

UNARY = ['sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'sinh', 'cosh', 'tanh',
         'asinh', 'acosh', 'atanh', 'erf', 'erfc', 'tgamma', 'lgamma']
BINARY = ['atan2', 'fmod', 'remainder']
SAMPLED = [(16, 3), (32, 3), (64, 3)]

DBL_MAX_PSI = lig.psi(mp.mpf(sys.float_info.max))
LN2 = mp.log(2)
FBITS = None                          # set per format, for the midpoint assertions


def value_result(v):
    """(psi |v|, negative) of a value, or zero"""
    if v == 0:
        return (mp.mpf(0), False)
    return (lig.psi(abs(v)), v < 0)


def far_from_midpoint(z, what):
    """a constant c that a result c +- tiny rounds as: assert c is far from every midpoint"""
    t = z * mp.mpf(2) ** FBITS
    off = abs((t - mp.floor(t)) - mp.mpf(0.5))
    if off < mp.mpf(2) ** -400:
        sys.exit('%s: psi %s lies within 2^-400 of a midpoint at fbits %d' % (what, mp.nstr(z, 30), FBITS))
    return z


def value_of(a, neg):
    v = lig.phi(a)
    return -v if neg else v


# ---------------------------------------------------------------- trigonometric

def f_trig(a, neg, f, odd):
    if a == 0:
        return (mp.mpf(0), False) if odd else (mp.mpf(1), False)
    if a > DBL_MAX_PSI:
        return NAR
    with mp.workprec(WIDE):
        X = lig.phi(a)
        r = f(-X if neg else X)
        return value_result(r)


def f_asin(a, neg):
    if a > 1:
        return NAR
    v = value_of(a, neg)
    return value_result(mp.asin(v))


def f_acos(a, neg):
    if a > 1:
        return NAR
    v = value_of(a, neg)
    return value_result(mp.acos(v))


def f_atan(a, neg):
    if a == 0:
        return (mp.mpf(0), False)
    if a < DIRECT:
        return value_result(mp.atan(value_of(a, neg)))
    # 1 / X < e^-484: atan X = pi/2 - tiny
    return (far_from_midpoint(lig.psi(mp.pi / 2), 'atan'), neg)


# ---------------------------------------------------------------- hyperbolic

def ln_sinh_cosh_psi(a, plus):
    """psi(ln cosh X) (plus) or psi(ln sinh X) for psi a >= 1"""
    if a < DIRECT:
        X = lig.phi(a)
        w = X - LN2 + (mp.log1p(mp.exp(-2 * X)) if plus else mp.log1p(-mp.exp(-2 * X)))
        return lig.psi(w)
    # X >= e^484: the correction is below e^-968, and X - ln 2 > 0 is a psi difference
    return lag.psi_sub(a, lig.psi(LN2))


def f_sinh(a, neg):
    if a == 0:
        return (mp.mpf(0), False)
    if a < 1:
        return value_result(mp.sinh(value_of(a, neg)))
    z = 1 + ln_sinh_cosh_psi(a, False)
    if a < 3:
        lag.agree(z, lig.psi(mp.sinh(lig.phi(a))), 'sinh')
    return (z, neg)


def f_cosh(a, neg):
    if a == 0:
        return (mp.mpf(1), False)
    if a < 1:
        return value_result(mp.cosh(lig.phi(a)))
    z = 1 + ln_sinh_cosh_psi(a, True)
    if a < 3:
        lag.agree(z, lig.psi(mp.cosh(lig.phi(a))), 'cosh')
    return (z, False)


def f_tanh(a, neg):
    if a == 0:
        return (mp.mpf(0), False)
    if a < DIRECT:
        return value_result(mp.tanh(value_of(a, neg)))
    return (mp.mpf(1), neg)            # 1 - 2 e^-2X: rounds to 1 at every fbits


def ln_x_plus_ln2_psi(a):
    """psi(ln X + ln 2) for psi a >= DIRECT: ln X = phi(a - 1) > 484"""
    return lag.psi_add(a - 1, lig.psi(LN2))


def f_asinh(a, neg):
    if a == 0:
        return (mp.mpf(0), False)
    if a < DIRECT:
        return value_result(mp.asinh(value_of(a, neg)))
    return (ln_x_plus_ln2_psi(a), neg)  # + O(X^-2) < e^-968


def f_acosh(a, neg):
    if neg or a < 1:
        return NAR
    if a == 1:
        return (mp.mpf(0), False)
    if a < DIRECT:
        return value_result(mp.acosh(lig.phi(a)))
    return (ln_x_plus_ln2_psi(a), False)  # - O(X^-2)


def f_atanh(a, neg):
    if a >= 1:
        return NAR
    return value_result(mp.atanh(value_of(a, neg)))


# ---------------------------------------------------------------- error and gamma

ERF_EDGE = lig.psi(mp.mpf(40))       # erfc(40) < 2^-2300: erf is 1, erfc 0 or 2, on every grid


def f_erf(a, neg):
    if a == 0:
        return (mp.mpf(0), False)
    if a < ERF_EDGE:
        return value_result(mp.erf(value_of(a, neg)))
    return (mp.mpf(1), neg)


def f_erfc(a, neg):
    if a == 0:
        return (mp.mpf(1), False)
    if a < ERF_EDGE:
        return value_result(mp.erfc(value_of(a, neg)))
    return (lig.psi(mp.mpf(2)), False) if neg else (mp.mpf(0), False)


def is_pole(a, neg):
    return a == 0 or (neg and a == 1)  # 0 and -1: no other negative integer is a lindex value


def reflection(a):
    """(ln |Gamma(-X)|, sign of Gamma(-X)) for X = phi(a) within the double range, -X not a pole"""
    with mp.workprec(WIDE):
        X = lig.phi(a)
        s = mp.sin(mp.pi * -X)
        L = mp.log(mp.pi) - mp.log(abs(s)) - mp.loggamma(1 + X)
        return L, s < 0


def lgamma_positive(a):
    """psi and sign of lgamma X for X = phi(a) > 0"""
    if a < DIRECT:
        return value_result(mp.loggamma(lig.phi(a)))
    # lgamma X = X (ln X - 1) (1 + O(1/X)), ln X - 1 = phi(a - 1) - 1 >= 483
    return (lag.psi_mul(a, lag.psi_sub(a - 1, mp.mpf(1))), False)


def f_lgamma(a, neg):
    if is_pole(a, neg):
        return NAR
    if not neg:
        return lgamma_positive(a)
    if a > DBL_MAX_PSI:
        return NAR
    L, _ = reflection(a)
    return value_result(L)


def exp_of_signed_psi(z, negative):
    """psi(e^v) for v = +-phi(z): one level up, or a level-0 value"""
    if not negative:
        return 1 + z
    return mp.exp(-lig.phi(z)) if z < DIRECT else mp.mpf(0)


def f_tgamma(a, neg):
    if is_pole(a, neg):
        return NAR
    if not neg:
        z, n = lgamma_positive(a)
        return (exp_of_signed_psi(z, n), False)
    if a > DBL_MAX_PSI:
        return NAR
    L, negative = reflection(a)
    with mp.workprec(WIDE):
        if L >= 0:
            return (1 + lig.psi(L), negative)
        m = mp.exp(L)
        return (m, negative) if m > 0 else (mp.mpf(0), False)


def unary(name, a, neg):
    if name in ('sin', 'cos', 'tan'):
        f = {'sin': mp.sin, 'cos': mp.cos, 'tan': mp.tan}[name]
        return f_trig(a, neg, f, name != 'cos')
    return globals()['f_' + name](a, neg)


# ---------------------------------------------------------------- binary

def f_atan2(a, sa, b, sb):
    """atan2(Y, X) for Y = +-phi(a), X = +-phi(b)"""
    if a == 0:
        if b == 0 or not sb:
            return (mp.mpf(0), False)
        return (lig.psi(mp.pi), False)
    if b == 0:
        return (lig.psi(mp.pi / 2), sa)
    r = lag.psi_div(a, b)                 # psi(|Y| / |X|)
    if r < DIRECT:
        theta = mp.atan(lig.phi(r))
        v = (mp.pi - theta) if sb else theta
        z, _ = value_result(v)
        return (z, sa)
    # |Y| / |X| > e^484: theta = pi/2 - tiny
    c = lig.psi(mp.pi / 2)
    return (far_from_midpoint(c, 'atan2'), sa)


def remainder_parts(X, Y):
    """r = X - nY, n = trunc(X / Y), as exact values; and n"""
    n = mp.floor(abs(X) / abs(Y))
    r = abs(X) - n * abs(Y)
    while r < 0:
        r += abs(Y)
        n -= 1
    while r >= abs(Y):
        r -= abs(Y)
        n += 1
    return r, n


def f_fmod(a, sa, b, sb):
    if b == 0:
        return NAR
    if a == 0:
        return (mp.mpf(0), False)
    if a < b:
        return (a, sa)                    # |X| < |Y|: X itself
    if a > DBL_MAX_PSI:
        return NAR
    with mp.workprec(WIDE):
        r, _ = remainder_parts(lig.phi(a), lig.phi(b))
        z, _ = value_result(r)
        return (z, sa) if r != 0 else (mp.mpf(0), False)


def f_remainder(a, sa, b, sb):
    if b == 0:
        return NAR
    if a == 0:
        return (mp.mpf(0), False)
    if a < b:
        # n is 0 or +-1: compare |X| with |Y| / 2, ties to n = 0 (even)
        half = lag.psi_div(b, lig.psi(mp.mpf(2)))
        if a <= half:
            return (a, sa)
        return (lag.psi_sub(b, a), not sa)   # X - sign(X) |Y|
    if a > DBL_MAX_PSI:
        return NAR
    with mp.workprec(WIDE):
        Y = lig.phi(b)
        r, n = remainder_parts(lig.phi(a), Y)
        if 2 * r > Y or (2 * r == Y and int(n) % 2 == 1):
            r = r - Y
        if r == 0:
            return (mp.mpf(0), False)
        z, negative = value_result(r)
        return (z, negative != sa)


def binary(name, a, sa, b, sb):
    return globals()['f_' + name](a, sa, b, sb)


# ---------------------------------------------------------------- emission

def encode(result, nbits, lbits):
    if result == NAR:
        return 1 << (nbits - 1)
    z, negative = result
    return lag.signed(lag.magnitude(z, nbits, lbits), negative, nbits)


def operand(enc, nbits, lbits):
    mag, negative = lag.decode(enc, nbits)
    return lag.psi_of(mag, nbits, lbits), negative


def sampled_encodings(nbits, lbits, rng):
    fbits = nbits - 1 - lbits
    S = 1 << fbits
    top = (1 << (nbits - 1)) - 1
    mags = set()
    for l in range(1 << lbits):
        lo, hi = l * S, min((l + 1) * S - 1, top)
        mags.update(rng.randint(lo, hi) for _ in range(6))
        mags.update(m for m in (lo - 1, lo, lo + 1, lo + S // 2) if 0 < m <= top)
    # the double range edge, where the trigonometric functions turn to NaR
    edge = lag.magnitude(DBL_MAX_PSI, nbits, lbits)
    mags.update(m for m in (edge - 1, edge, edge + 1) if 0 < m <= top)
    mags.update((1, top, S, S // 2, S // 2 + 1))
    out = []
    for m in sorted(mags):
        out.append(m)
        out.append(lag.signed(m, True, nbits))
    return out


def sampled_pairs(nbits, lbits, rng):
    fbits = nbits - 1 - lbits
    S = 1 << fbits
    top = (1 << (nbits - 1)) - 1
    out = []
    for _ in range(120):
        a = rng.randint(1, min(top, 5 * S))
        b = rng.randint(1, min(top, 5 * S)) if rng.random() < 0.8 else rng.randint(1, top)
        out.append((lag.signed(a, rng.random() < 0.3, nbits), lag.signed(b, rng.random() < 0.3, nbits)))
    # level-0 pairs (exact dyadic remainders), equal and halving pairs, the extremes
    for _ in range(12):
        a, b = rng.randint(1, S - 1), rng.randint(1, S - 1)
        out.append((a, b))
    out += [(S, S), (S // 2, S), (S // 4 * 3, S // 2), (2 * S, S // 2), (top, S), (S, top), (top, top), (1, 1)]
    return out


def main():
    global FBITS
    rng = random.Random(1715)
    print('#pragma once')
    print('// lindex_math2_vectors.hpp: reference results for the rest of the lindex math library (#1715)')
    print('//')
    print('// Copyright (C) 2017 Stillwater Supercomputing, Inc.')
    print('// SPDX-License-Identifier: MIT')
    print('//')
    print('// This file is part of the universal numbers project, which is released under an MIT Open Source license.')
    print('//')
    print('// GENERATED by tools/generators/lindex_math2_gen.py from mpmath at %d (values to %d) bits: do not edit.'
          % (mp.mp.prec, WIDE))
    print('#include <cstdint>')
    print()
    print('namespace sw { namespace universal { namespace lindex_verification {')
    print()
    print('// the unary and binary functions, in the column order of the tables below')
    print('constexpr const char* math2_unary_names[%d] = { %s };' % (len(UNARY), ', '.join('"%s"' % n for n in UNARY)))
    print('constexpr unsigned math2_nr_unary = %d;' % len(UNARY))
    print('constexpr const char* math2_binary_names[%d] = { %s };' % (len(BINARY), ', '.join('"%s"' % n for n in BINARY)))
    print('constexpr unsigned math2_nr_binary = %d;' % len(BINARY))
    print()
    FBITS = 4
    print('// lindex<8,3>: row f, characters 2e and 2e + 1 hold the encoding (hex) of function f of')
    print('// encoding e')
    print('constexpr const char* lindex8_unary2[%d] = {' % len(UNARY))
    for name in UNARY:
        row = []
        for e in range(256):
            row.append(0x80 if e == 0x80 else encode(unary(name, *operand(e, 8, 3)), 8, 3))
        print('\t"%s",   // %s' % (''.join('%02x' % r for r in row), name))
    print('};')
    print()
    for name in BINARY:
        print('// lindex<8,3>: %s(x, y) for row x, characters 2y and 2y + 1' % name)
        print('constexpr const char* lindex8_%s[256] = {' % name)
        for x in range(256):
            row = []
            for y in range(256):
                if x == 0x80 or y == 0x80:
                    row.append(0x80)
                else:
                    a, sa = operand(x, 8, 3)
                    b, sb = operand(y, 8, 3)
                    row.append(encode(binary(name, a, sa, b, sb), 8, 3))
            print('\t"%s",' % ''.join('%02x' % r for r in row))
        print('};')
        print()
    print('// sampled: an encoding and each unary function of it, in math2_unary_names order')
    print('struct Math2UnaryVector { std::uint64_t x; std::uint64_t f[%d]; };' % len(UNARY))
    print('// sampled: x, y and each binary function of them, in math2_binary_names order')
    print('struct Math2BinaryVector { std::uint64_t x, y; std::uint64_t f[%d]; };' % len(BINARY))
    print()
    for nbits, lbits in SAMPLED:
        FBITS = nbits - 1 - lbits
        print('constexpr Math2UnaryVector math2_unary_%d_%d[] = {' % (nbits, lbits))
        for e in sampled_encodings(nbits, lbits, rng):
            a, neg = operand(e, nbits, lbits)
            fs = [encode(unary(name, a, neg), nbits, lbits) for name in UNARY]
            print('\t{ 0x%xull, { %s } },' % (e, ', '.join('0x%xull' % f for f in fs)))
        print('};')
        print()
        print('constexpr Math2BinaryVector math2_binary_%d_%d[] = {' % (nbits, lbits))
        for ex, ey in sampled_pairs(nbits, lbits, rng):
            a, sa = operand(ex, nbits, lbits)
            b, sb = operand(ey, nbits, lbits)
            fs = [encode(binary(name, a, sa, b, sb), nbits, lbits) for name in BINARY]
            print('\t{ 0x%xull, 0x%xull, { %s } },' % (ex, ey, ', '.join('0x%xull' % f for f in fs)))
        print('};')
        print()
    print('}}}  // namespace sw::universal::lindex_verification')


if __name__ == '__main__':
    main()
