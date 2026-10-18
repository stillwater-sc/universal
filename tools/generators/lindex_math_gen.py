#!/usr/bin/env python3
# lindex_math_gen.py: math library vectors for lindex<nbits, lbits> (#1349 part 5, #1696).
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT
#
# This file is part of the universal numbers project, which is released under an MIT Open Source license.
#
# Writes include/sw/universal/verification/lindex_math_vectors.hpp:
#
#   lindex<8,3>  for every one of the 256 encodings, the correctly rounded encoding of each
#                unary function; pow for every one of the 65536 pairs; hypot for every pair
#                of magnitudes (it does not depend on the signs)
#   sampled      encodings of lindex<16,3>, <32,3> and <64,3> across every level, with
#                every unary function, and pairs with pow and hypot
#
# Each result is the exact psi of the function value, rounded once to nearest even on the
# psi grid and saturated, from mpmath at 600 bits.  Wherever mpmath can form the values
# (psi < 4.6) the function is evaluated directly; above that it goes through the level-down
# identities, by the plain Clenshaw-Olver sequences of lindex_arithmetic_gen.py:
#
#   e^X = phi(x + 1)                    exp, and the step up of exp2, exp10, pow
#   ln X = phi(x - 1)                   log, and the step down of log2, log10, log1p
#   psi(c Y) = 1 + psi(phi(y - 1) + ln c)
#
# Where both routes apply they must agree to 2^-200.  NaR is the encoding 100...0.
#
# Usage:  python3 tools/generators/lindex_math_gen.py > include/sw/universal/verification/lindex_math_vectors.hpp
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

UNARY = ['exp', 'exp2', 'exp10', 'expm1', 'log', 'log2', 'log10', 'log1p',
         'sqrt', 'cbrt', 'floor', 'ceil', 'trunc', 'round']
SAMPLED = [(16, 3), (32, 3), (64, 3)]


def value(a, negative):
    """the value of a signed psi magnitude a < DIRECT"""
    v = lig.phi(a)
    return -v if negative else v


def psi_signed(v):
    """(psi |v|, negative) of a value"""
    return (lig.psi(abs(v)), v < 0)


def check(direct, route, what):
    return lag.agree(direct, route, what)


def exp_t(t, negative):
    """e^t for t = +-phi(t): the step up, or a level-0 value"""
    if not negative:
        return 1 + t
    return mp.exp(-lig.phi(t)) if t < DIRECT else mp.mpf(0)


def f_exp(a, neg):
    if a == 0:
        return (mp.mpf(1), False)
    if not neg:
        return (a + 1, False)
    return (exp_t(a, True), False)


def f_exp_base(a, neg, lnb):
    if a == 0:
        return (mp.mpf(1), False)
    t = lag.psi_mul(a, lig.psi(lnb))                 # psi(|v| ln b)
    r = exp_t(t, neg)
    if a < DIRECT and neg:
        check(r, lig.psi(mp.exp(value(a, True) * lnb)), 'exp_base')
    elif a < 3:
        check(r, lig.psi(mp.exp(value(a, neg) * lnb)), 'exp_base')
    return (r, False)


def f_expm1(a, neg):
    if a == 0:
        return (mp.mpf(0), False)
    if not neg:
        r = lag.psi_sub(a + 1, mp.mpf(1))
        if a < 3:
            check(r, lig.psi(mp.expm1(value(a, False))), 'expm1')
        return (r, False)
    m = -mp.expm1(-lig.phi(a)) if a < DIRECT else mp.mpf(1)
    return (lig.psi(m), True)


def f_log(a, neg):
    if a == 0 or neg:
        return NAR
    if a >= 1:
        return (a - 1, False)
    return (lig.psi(-mp.log(a)), True)


def f_log_base(a, neg, lnb):
    if a == 0 or neg:
        return NAR
    if a >= 1:
        r = lag.psi_div(a - 1, lig.psi(lnb))
        if a < DIRECT:
            check(r, lig.psi(mp.log(lig.phi(a)) / lnb), 'log_base')
        return (r, False)
    return (lig.psi(-mp.log(a) / lnb), True)


def f_log1p(a, neg):
    if a == 0:
        return (mp.mpf(0), False)
    if not neg:
        r = lag.psi_add(a, mp.mpf(1)) - 1
        if a < DIRECT:
            check(r, lig.psi(mp.log1p(lig.phi(a))), 'log1p')
        return (r, False)
    if a >= 1:
        return NAR
    return (lig.psi(-mp.log1p(-a)), True)


def f_root(a, neg, n):
    if a == 0:
        return (mp.mpf(0), False)
    if a >= 1:
        r = 1 + lag.psi_mul(a - 1, mp.mpf(1) / n)
        if a < DIRECT:
            check(r, lig.psi(lig.phi(a) ** (mp.mpf(1) / n)), 'root')
        return (r, neg)
    return (a ** (mp.mpf(1) / n), neg)


def f_sqrt(a, neg):
    if neg and a != 0:
        return NAR
    return f_root(a, False, 2)


def f_integral(a, neg, mode):
    if a == 0:
        return (mp.mpf(0), False)
    if a >= DIRECT:
        return (a, neg)            # 1 is far below the spacing: the value rounds back to itself
    v = value(a, neg)
    if mode == 'floor':
        i = mp.floor(v)
    elif mode == 'ceil':
        i = mp.ceil(v)
    elif mode == 'trunc':
        i = mp.floor(v) if v > 0 else mp.ceil(v)
    else:                          # half away from zero
        i = mp.floor(abs(v) + mp.mpf(0.5))
        i = -i if v < 0 else i
    return psi_signed(i) if i != 0 else (mp.mpf(0), False)


def unary(name, a, neg):
    if name == 'exp':
        return f_exp(a, neg)
    if name == 'exp2':
        return f_exp_base(a, neg, mp.log(2))
    if name == 'exp10':
        return f_exp_base(a, neg, mp.log(10))
    if name == 'expm1':
        return f_expm1(a, neg)
    if name == 'log':
        return f_log(a, neg)
    if name == 'log2':
        return f_log_base(a, neg, mp.log(2))
    if name == 'log10':
        return f_log_base(a, neg, mp.log(10))
    if name == 'log1p':
        return f_log1p(a, neg)
    if name == 'sqrt':
        return f_sqrt(a, neg)
    if name == 'cbrt':
        return f_root(a, neg, 3)
    return f_integral(a, neg, name)


def f_pow(a, sa, b, sb):
    """X^Y for signed psi magnitudes, by the rules of math/pow.hpp"""
    if b == 0:
        return (mp.mpf(1), False)
    if a == 0:
        return NAR if sb else (mp.mpf(0), False)
    if b == 1 and not sb:
        return (a, sa)
    if b == 1 and sb:
        return (lag.psi_div(mp.mpf(1), a), sa)        # 1 / x
    if sa:
        return NAR
    if a == 1:
        return (mp.mpf(1), False)
    ln_negative = a < 1
    ln_psi = lig.psi(-mp.log(a)) if ln_negative else a - 1
    t = lag.psi_mul(b, ln_psi)
    r = exp_t(t, sb != ln_negative)
    if a < DIRECT and b < 3:
        check(r, lig.psi(mp.power(lig.phi(a), value(b, sb))), 'pow')
    return (r, False)


def f_hypot(a, b):
    if a == 0:
        return b
    if b == 0:
        return a
    hi, lo = max(a, b), min(a, b)
    if hi < DIRECT:
        return lig.psi(mp.sqrt(lig.phi(hi) ** 2 + lig.phi(lo) ** 2))
    if lo < 1:
        return hi                  # lo / hi < 10^-210: the sum moves psi by far less than 2^-600
    d = lag.psi_sub(hi - 1, lo - 1)
    if d >= DIRECT:
        return hi
    delta = mp.log1p(mp.exp(-2 * lig.phi(d))) / 2  # ln hypot = ln hi + delta
    return 1 + lag.psi_add(hi - 1, lig.psi(delta))


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
        a = rng.randint(1, min(top, 5 * S))           # levels 0..4 weighted
        b = rng.randint(1, min(top, 3 * S)) if rng.random() < 0.7 else rng.randint(1, top)
        out.append((lag.signed(a, rng.random() < 0.15, nbits), lag.signed(b, rng.random() < 0.4, nbits)))
    out += [(S, S), (2 * S, S // 2), (S // 2, 3 * S), (top, S // 4), (top, top), (1, 1)]
    return out


def main():
    rng = random.Random(1696)
    print('#pragma once')
    print('// lindex_math_vectors.hpp: reference math library results for lindex (#1696)')
    print('//')
    print('// Copyright (C) 2017 Stillwater Supercomputing, Inc.')
    print('// SPDX-License-Identifier: MIT')
    print('//')
    print('// This file is part of the universal numbers project, which is released under an MIT Open Source license.')
    print('//')
    print('// GENERATED by tools/generators/lindex_math_gen.py from mpmath at %d bits: do not edit.' % mp.mp.prec)
    print('#include <cstdint>')
    print()
    print('namespace sw { namespace universal { namespace lindex_verification {')
    print()
    print('// the unary functions, in the column order of the tables below')
    print('constexpr const char* math_unary_names[%d] = { %s };' % (len(UNARY), ', '.join('"%s"' % n for n in UNARY)))
    print('constexpr unsigned math_nr_unary = %d;' % len(UNARY))
    print()
    print('// lindex<8,3>: row f, characters 2e and 2e + 1 hold the encoding (hex) of function f of')
    print('// encoding e')
    print('constexpr const char* lindex8_unary[%d] = {' % len(UNARY))
    for name in UNARY:
        row = []
        for e in range(256):
            row.append(0x80 if e == 0x80 else encode(unary(name, *operand(e, 8, 3)), 8, 3))
        print('\t"%s",   // %s' % (''.join('%02x' % r for r in row), name))
    print('};')
    print()
    print('// lindex<8,3>: pow(x, y) for row x, characters 2y and 2y + 1')
    print('constexpr const char* lindex8_pow[256] = {')
    for x in range(256):
        row = []
        for y in range(256):
            if x == 0x80 or y == 0x80:
                row.append(0x80)
            else:
                a, sa = operand(x, 8, 3)
                b, sb = operand(y, 8, 3)
                row.append(encode(f_pow(a, sa, b, sb), 8, 3))
        print('\t"%s",' % ''.join('%02x' % r for r in row))
    print('};')
    print()
    print('// lindex<8,3>: hypot of magnitudes a, b in 0..127 (row a, characters 2b and 2b + 1)')
    print('constexpr const char* lindex8_hypot[128] = {')
    for a in range(128):
        print('\t"%s",' % ''.join('%02x' % lag.magnitude(f_hypot(lag.psi_of(a, 8, 3), lag.psi_of(b, 8, 3)), 8, 3)
                                  for b in range(128)))
    print('};')
    print()
    print('// sampled: an encoding and each unary function of it, in math_unary_names order')
    print('struct MathUnaryVector { std::uint64_t x; std::uint64_t f[%d]; };' % len(UNARY))
    print('// sampled: x, y, pow(x, y), hypot(x, y)')
    print('struct MathBinaryVector { std::uint64_t x, y, pow, hypot; };')
    print()
    for nbits, lbits in SAMPLED:
        print('constexpr MathUnaryVector math_unary_%d_%d[] = {' % (nbits, lbits))
        for e in sampled_encodings(nbits, lbits, rng):
            a, neg = operand(e, nbits, lbits)
            fs = [encode(unary(name, a, neg), nbits, lbits) for name in UNARY]
            print('\t{ 0x%xull, { %s } },' % (e, ', '.join('0x%xull' % f for f in fs)))
        print('};')
        print()
        print('constexpr MathBinaryVector math_binary_%d_%d[] = {' % (nbits, lbits))
        for ex, ey in sampled_pairs(nbits, lbits, rng):
            a, sa = operand(ex, nbits, lbits)
            b, sb = operand(ey, nbits, lbits)
            p = encode(f_pow(a, sa, b, sb), nbits, lbits)
            h = lag.magnitude(f_hypot(a, b), nbits, lbits)
            print('\t{ 0x%xull, 0x%xull, 0x%xull, 0x%xull },' % (ex, ey, p, h))
        print('};')
        print()
    print('}}}  // namespace sw::universal::lindex_verification')


if __name__ == '__main__':
    main()
