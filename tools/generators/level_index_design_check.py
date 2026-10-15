#!/usr/bin/env python3
# level_index_design_check.py: reproduce every numeric claim in docs/level-index-design.md.
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT
#
# This file is part of the universal numbers project, which is released under an MIT Open Source license.
#
# The design doc for the level-index type (#1349, #1692) states numbers: level boundaries,
# how far double and binary128 reach in psi, the underflow floor, epsilon, the value-domain
# gap at each level, how many reciprocals underflow, the bounds on the Clenshaw-Olver
# sequences.  Each section below is tagged [Cn] and the doc cites the same tag, so a claim
# can be traced to the line that computes it.
#
# Usage:  python3 tools/generators/level_index_design_check.py
#
# Requires mpmath.  Prints a report and exits non-zero if any checked identity fails.
# Not run during the build.
import math
import random
import sys
import mpmath as mp

mp.mp.prec = 400

failures = 0


def check(label, ok):
    global failures
    if not ok:
        failures += 1
        print('  FAIL: ' + label)


def phi(x):
    """generalized exponential: phi(x) = x on [0,1), e^phi(x-1) above"""
    x = mp.mpf(x)
    if x < 1:
        return x
    return mp.exp(phi(x - 1))


def psi(X):
    """generalized logarithm, the inverse of phi, for X >= 0"""
    X = mp.mpf(X)
    level = 0
    while X >= 1:
        X = mp.log(X)
        level += 1
    return level + X


def encode(X, fbits):
    """round psi(X) to the fixed-point grid 2^-fbits, ties to even"""
    scaled = psi(X) * mp.mpf(2) ** fbits
    return int(mp.nint(scaled))


def fbits_of(nbits, lbits):
    return nbits - 1 - lbits


def section(tag, title):
    print()
    print('[%s] %s' % (tag, title))


# --------------------------------------------------------------------------------------
section('C1', 'level boundaries phi(l)')
for level in range(0, 5):
    print('  phi(%d) = %s' % (level, mp.nstr(phi(level), 12)))
# phi(5) = e^phi(4) is not representable in a useful way; report its decimal exponent
log10_phi5 = phi(4) / mp.log(10)
print('  log10 phi(5) = %s' % mp.nstr(log10_phi5, 8))
print('  log10 log10 phi(6) ~= log10(phi(5) * log10(e)) = %s'
      % mp.nstr(log10_phi5 + mp.log10(mp.log10(mp.e)), 8))

# --------------------------------------------------------------------------------------
section('C2', 'how far native formats reach in psi')
flt_max = (2 - mp.mpf(2) ** -23) * mp.mpf(2) ** 127
dbl_max = (2 - mp.mpf(2) ** -52) * mp.mpf(2) ** 1023
x87_max = (2 - mp.mpf(2) ** -63) * mp.mpf(2) ** 16383
f128_max = (2 - mp.mpf(2) ** -112) * mp.mpf(2) ** 16383
for name, v in (('FLT_MAX', flt_max), ('DBL_MAX', dbl_max),
                ('x87 LDBL_MAX', x87_max), ('binary128 max', f128_max)):
    print('  psi(%-13s) = %s' % (name, mp.nstr(psi(v), 8)))
print('  -> no native format reaches level 5: levels 5..7 exist only in the encoding,')
print('     so the oracle has to emit encodings, not values')

# --------------------------------------------------------------------------------------
section('C3', 'underflow floor and resolution per configuration (lbits = 3)')
print('  %-14s %6s %24s %18s' % ('type', 'fbits', 'minpos = 2^-fbits', 'encodings / level'))
for nbits in (8, 16, 32, 64):
    fb = fbits_of(nbits, 3)
    print('  lindex<%2d,3>   %6d %24s %18d'
          % (nbits, fb, mp.nstr(mp.mpf(2) ** -fb, 6), 2 ** fb))
print('  values below minpos/2 round to 0; minpos/2 itself ties to even, also 0')

# --------------------------------------------------------------------------------------
section('C4', 'epsilon: (phi(1 + u) - 1) / u with u = 2^-fbits')
for nbits in (8, 16, 32, 64):
    fb = fbits_of(nbits, 3)
    u = mp.mpf(2) ** -fb
    ratio = (phi(1 + u) - 1) / u
    print('  lindex<%2d,3>  u = 2^-%-2d  ratio = %s' % (nbits, fb, mp.nstr(ratio, 10)))
    check('epsilon ratio within u of 1', abs(ratio - 1) < u)

# --------------------------------------------------------------------------------------
section('C5', 'value-domain gap of one psi-ulp, at the top of each level, lindex<16,3>')
fb = fbits_of(16, 3)
u = mp.mpf(2) ** -fb
print('  level  relative gap phi(x+u)/phi(x) - 1 at x = level + 1 - u')
for level in range(0, 5):
    x = level + 1 - u
    if level == 0:
        # at level 0 phi is the identity, so the gap is u / x, worst at minpos
        print('    0    u / x: %s at x = 1-u, 1.0 at x = minpos'
              % mp.nstr(u / x, 6))
        continue
    # ln(phi(x+u)/phi(x)) = phi(x+u-1) - phi(x-1); exponentiate only when it is small
    dlog = phi(x + u - 1) - phi(x - 1)
    if dlog < 50:
        print('    %d    %s' % (level, mp.nstr(mp.expm1(dlog), 6)))
    else:
        print('    %d    e^%s  (log10 = %s)'
              % (level, mp.nstr(dlog, 6), mp.nstr(dlog / mp.log(10), 6)))
print('  -> a correctly rounded result near the top of level 4 can be off in value by')
print('     a factor of e^%s; a value-domain ulp test calls that a total failure'
      % mp.nstr(dlog, 6))

# --------------------------------------------------------------------------------------
section('C6', 'crossover against IEEE for X >= 1 (relative gap of one psi-ulp)')
for nbits, pbits, name in ((32, 24, 'binary32'), (64, 53, 'binary64')):
    fb = fbits_of(nbits, 3)
    u = mp.mpf(2) ** -fb
    best = mp.mpf(2) ** -pbits        # IEEE relative gap ranges over [2^-p, 2^(1-p)]
    worst = mp.mpf(2) ** (1 - pbits)

    def rel_gap(x):
        if x < 1:
            return u / x
        return mp.expm1(phi(x + u - 1) - phi(x - 1))

    # rel_gap is increasing in x for x >= 1; bisect for each threshold in psi
    def first_above(threshold):
        lo, hi = mp.mpf(1), mp.mpf('4.5')
        for _ in range(200):
            mid = (lo + hi) / 2
            if rel_gap(mid) > threshold:
                hi = mid
            else:
                lo = mid
        return lo
    better_until = phi(first_above(best))
    worse_from = phi(first_above(worst))
    print('  lindex<%d,3> vs %s: finer than every %s gap for X < 2^%s,'
          % (nbits, name, name, mp.nstr(mp.log(better_until, 2), 4)))
    print('      coarser than every %s gap for X > 2^%s'
          % (name, mp.nstr(mp.log(worse_from, 2), 4)))

# --------------------------------------------------------------------------------------
section('C7', 'exact exp and log: exp(phi(x)) = phi(x+1), ln(phi(x)) = phi(x-1)')
rng = random.Random(1349)
worst_exp = mp.mpf(0)
worst_log = mp.mpf(0)
for _ in range(2000):
    x = mp.mpf(rng.uniform(0.0, 3.6))
    worst_exp = max(worst_exp, abs(psi(mp.exp(phi(x))) - (x + 1)))
    y = mp.mpf(rng.uniform(1.0, 4.6))
    worst_log = max(worst_log, abs(psi(mp.log(phi(y))) - (y - 1)))
print('  max |psi(exp(phi(x))) - (x+1)| over 2000 x in [0, 3.6) = %s' % mp.nstr(worst_exp, 3))
print('  max |psi(ln(phi(y)))  - (y-1)| over 2000 y in [1, 4.6) = %s' % mp.nstr(worst_log, 3))
check('exp shift identity', worst_exp < mp.mpf(2) ** -300)
check('log shift identity', worst_log < mp.mpf(2) ** -300)
print('  -> on the encoding, exp adds 1 and log subtracts 1: no rounding at all')

# --------------------------------------------------------------------------------------
section('C8', 'reciprocal: how many positive encodings of lindex<16,3> have 1/X round to 0')
fb = fbits_of(16, 3)
total = 8 * 2 ** fb - 1               # positive encodings 1 .. 0x7FFF
# 1/X rounds to 0 when 1/X <= 2^-(fbits+1) (the tie goes to even, which is 0)
threshold = mp.mpf(2) ** (fb + 1)
first = encode(threshold, fb)         # encodings at or above this have X >= threshold
if phi(mp.mpf(first) / 2 ** fb) < threshold:
    first += 1
underflowing = total - first + 1
print('  1/X rounds to 0 when X >= 2^%d, psi(2^%d) = %s, first such encoding 0x%04X'
      % (fb + 1, fb + 1, mp.nstr(psi(threshold), 8), first))
print('  %d of %d positive encodings (%s%%) have a reciprocal of 0'
      % (underflowing, total, mp.nstr(mp.mpf(100) * underflowing / total, 4)))

# --------------------------------------------------------------------------------------
section('C9', 'Clenshaw-Olver addition: sequence ranges and agreement with psi(X +- Y)')


def co_addsub(x, y, subtract, exp=mp.exp, log=mp.log):
    """psi(phi(x) +- phi(y)) for x >= y >= 0 through the a/b/c sequences, never forming
    phi(x) or phi(y).  Returns (psi of the result, list of all a, b, c values seen).
    exp and log select the working precision: mpmath by default, math for double."""
    l = int(x)
    f = x - l
    m = int(y)
    g = y - m
    if l == 0:
        # both operands at level 0: plain fixed-point add or subtract, then re-level
        w = x - y if subtract else x + y
        n = 0
        while w >= 1:
            w = log(w)
            n += 1
        return n + w, []
    a = [0 * x] * l
    a[l - 1] = exp(-f)
    for j in range(l - 1, 0, -1):
        # 1/a_j = phi(X - j) can overflow a double; e^-inf = 0 is the right limit
        a[j - 1] = exp(-1 / a[j]) if a[j] > 0 else 0 * x
    b = [0 * x] * max(m, 1)
    if m == 0:
        b[0] = a[0] * g
    else:
        b[m - 1] = a[m - 1] * exp(g)
        for j in range(m - 1, 0, -1):
            b[j - 1] = exp(-(1 - b[j]) / a[j]) if a[j] > 0 else 0 * x
    c = [0 * x] * l
    c[0] = 1 - b[0] if subtract else 1 + b[0]
    seen = a + b + [c[0]]
    for j in range(0, l):
        if j > 0:
            c[j] = 1 + a[j] * log(c[j - 1])
            seen.append(c[j])
        if c[j] < a[j]:
            # phi(Z - j) = c_j / a_j < 1, so the result sits at level j
            return j + c[j] / a[j], seen
    # c_j >= a_j all the way: phi(Z - l + 1) = c_{l-1} e^f, so phi(Z - l) = f + ln c_{l-1}
    w = f + log(c[l - 1])
    n = l
    while w >= 1:
        w = log(w)
        n += 1
    return n + w, seen


lo_a = lo_b = lo_c_add = lo_c_sub = mp.inf
hi_a = hi_b = hi_c_add = hi_c_sub = -mp.inf
worst = mp.mpf(0)
for _ in range(3000):
    x = mp.mpf(rng.uniform(0.0, 4.2))
    y = mp.mpf(rng.uniform(0.0, float(x)))
    for subtract in (False, True):
        z, seen = co_addsub(x, y, subtract)
        ref = psi(phi(x) - phi(y)) if subtract else psi(phi(x) + phi(y))
        worst = max(worst, abs(z - ref))
        if not seen:
            continue
        l = int(mp.floor(x))
        m = max(int(mp.floor(y)), 1)
        avals = seen[:l]
        bvals = seen[l:l + m]
        cvals = seen[l + m:]
        lo_a, hi_a = min(lo_a, min(avals)), max(hi_a, max(avals))
        lo_b, hi_b = min(lo_b, min(bvals)), max(hi_b, max(bvals))
        if subtract:
            lo_c_sub, hi_c_sub = min(lo_c_sub, min(cvals)), max(hi_c_sub, max(cvals))
        else:
            lo_c_add, hi_c_add = min(lo_c_add, min(cvals)), max(hi_c_add, max(cvals))
print('  a_j in [%s, %s]' % (mp.nstr(lo_a, 4), mp.nstr(hi_a, 6)))
print('  b_j in [%s, %s]' % (mp.nstr(lo_b, 4), mp.nstr(hi_b, 6)))
print('  c_j (add) in [%s, %s]' % (mp.nstr(lo_c_add, 6), mp.nstr(hi_c_add, 6)))
print('  c_j (sub) in [%s, %s]' % (mp.nstr(lo_c_sub, 4), mp.nstr(hi_c_sub, 6)))
print('  max |CO result - psi(phi(x) +- phi(y))| over 6000 cases = %s' % mp.nstr(worst, 3))
check('CO addition agrees with direct evaluation', worst < mp.mpf(2) ** -200)
check('a_j in (0,1]', lo_a > 0 and hi_a <= 1)
check('b_j in [0,1]', lo_b >= 0 and hi_b <= 1)
check('c_j (add) in [1,2]', lo_c_add >= 1 and hi_c_add <= 2)
check('c_j (sub) in [0,1]', lo_c_sub >= 0 and hi_c_sub <= 1)

# --------------------------------------------------------------------------------------
section('C10', 'multiplication one level down, including a level-0 operand')
worst = mp.mpf(0)
for _ in range(2000):
    x = mp.mpf(rng.uniform(1.0, 4.3))
    y = mp.mpf(rng.uniform(0.0, 4.3))
    ref = psi(phi(x) * phi(y))
    if y >= 1:
        # ln(XY) = phi(x-1) + phi(y-1), an addition one level down, then one level up
        lo, hi = (x - 1, y - 1) if x >= y else (y - 1, x - 1)
        w, _ = co_addsub(lo, hi, False)
        z = 1 + w
    else:
        # ln(XY) = phi(x-1) + ln(y): a subtraction one level down when XY >= 1,
        # otherwise the product falls into level 0 and is y * phi(x) directly
        t = phi(x - 1) + mp.log(y)
        z = 1 + psi(t) if t >= 0 else psi(y * phi(x))
    worst = max(worst, abs(z - ref))
print('  max |level-down product - psi(phi(x) * phi(y))| over 2000 cases = %s'
      % mp.nstr(worst, 3))
check('level-down multiply agrees', worst < mp.mpf(2) ** -200)

# --------------------------------------------------------------------------------------
section('C11', 'closure: how far squaring moves an encoding, in psi-ulps of lindex<16,3>')
fb = fbits_of(16, 3)
u = mp.mpf(2) ** -fb


def log10_phi(y):
    """log10 phi(y) without forming phi(y) when it is too large"""
    if y < 5:
        return mp.log10(phi(y))
    return phi(y - 1) / mp.log(10)


def square_moves(x):
    """psi(X^2) - psi(X) in psi-ulps.  Exact while phi(x) is an mpf (x < 5); above that
    the first-order term, as a log10: psi(X^2) = 2 + psi(phi(x - 2) + ln 2), so the ln 2 moves psi by
    ln 2 / phi'(x - 2), and phi'(y) is the product of phi(y - k) over the levels of y."""
    if x < 5:
        return (psi(phi(x) ** 2) - x) * 2 ** fb
    y = x - 2
    log10_deriv = sum(log10_phi(y - k) for k in range(int(mp.floor(y))))
    # returned as a log10: at level 7 the exponent itself has a million digits, and
    # raising 10 to it would build a multi-megabit integer
    return mp.log10(mp.log(2)) - log10_deriv + fb * mp.log10(2)


def square_moves_log10(x):
    return mp.log10(square_moves(x)) if x < 5 else square_moves(x)


for level in range(2, 8):
    for x in (mp.mpf(level), level + 1 - u):
        print('  x = %-14s psi(X^2) - psi(X) = 10^%s psi-ulp'
              % (mp.nstr(x, 10), mp.nstr(square_moves_log10(x), 6)))
check('squaring maxpos of lindex<16,3> rounds back to maxpos', square_moves_log10(8 - u) < mp.log10(0.5))
check('squaring the bottom of level 6 rounds back', square_moves_log10(mp.mpf(6)) < mp.log10(0.5))
print('  -> from level 6 up X * X rounds back to X: the top two levels are never left by')
print('     multiplication, so lbits = 3 (levels 0..7) is closed.  With lbits = 2 the last')
print('     level is 3, and squaring maxpos lands past the last encoding:')
top = 4 - mp.mpf(2) ** -fbits_of(16, 2)
print('     lindex<16,2>: psi(maxpos) = %s, psi(maxpos^2) = %s'
      % (mp.nstr(top, 10), mp.nstr(psi(phi(top) ** 2), 10)))

# --------------------------------------------------------------------------------------
section('C12', 'working precision: Clenshaw-Olver addition in double against 400 bits')
# double covers psi up to 4.63 [C2], and past that the sequences only underflow toward 0,
# so the sweep runs to x = 5.5 (the 400-bit reference stays cheap there).
worst = {False: 0.0, True: 0.0}
where = {False: None, True: None}
for _ in range(20000):
    x = rng.uniform(0.0, 5.5)
    y = rng.uniform(0.0, x)
    for subtract in (False, True):
        if subtract and x - y < 1e-6:
            continue        # catastrophic cancellation is a separate study (#1693)
        zd, _ = co_addsub(x, y, subtract, math.exp, math.log)
        zm, _ = co_addsub(mp.mpf(x), mp.mpf(y), subtract)
        err = float(abs(mp.mpf(zd) - zm))
        if err > worst[subtract]:
            worst[subtract] = err
            where[subtract] = (x, y)
for subtract in (False, True):
    e = worst[subtract]
    print('  %s: max psi error %.3g = 2^%.1f  (%.3g ulp of fbits 28, %.3g ulp of fbits 60)'
          % ('sub' if subtract else 'add', e, math.log2(e) if e > 0 else -math.inf,
             e * 2 ** 28, e * 2 ** 60))
    print('       worst at x = %.6f, y = %.6f' % where[subtract])
print('  -> double carries fbits 28 with margin here, but not fbits 60; C13 shows the')
print('     cancellation the 1e-6 cut above leaves out')

# --------------------------------------------------------------------------------------
section('C13', 'near cancellation: c_0 = 1 - b_0 in double, and the expm1 form that fixes it')


def co_sub_stable(x, y):
    """double subtraction that carries d_j = 1 - b_j through expm1 when b_0 is near 1:
    the operands share a level (m = l), or y sits just below x's level (m = l - 1)"""
    l = int(x)
    f = x - l
    m = int(y)
    g = y - m
    if l == 0 or m < l - 1 or m == 0:
        return co_addsub(x, y, True, math.exp, math.log)[0]
    a = [0.0] * l
    a[l - 1] = math.exp(-f)
    for j in range(l - 1, 0, -1):
        a[j - 1] = math.exp(-1 / a[j]) if a[j] > 0 else 0.0
    if m == l:
        d = -math.expm1(g - f)                      # 1 - e^(g - f), g - f exact
    else:
        # b_{l-2} = a_{l-2} e^g = e^(g - e^f), and g - e^f = (g - 1) - expm1(f)
        d = -math.expm1((g - 1) - math.expm1(f))
    for j in range(m - 1, 0, -1):
        d = -math.expm1(-d / a[j]) if a[j] > 0 else 1.0
    c = d                                            # c_0 = 1 - b_0
    for j in range(0, l):
        if j > 0:
            c = 1 + a[j] * math.log(c)
        if c < a[j]:
            return j + c / a[j]
    w = f + math.log(c)
    n = l
    while w >= 1:
        w = math.log(w)
        n += 1
    return n + w


print('  error in ulps of fbits 28 (lindex<32,3>), 300 pairs per row')
print('  %-26s %14s %14s' % ('pair', 'naive c_0', 'expm1 form'))
for label, gen in (
        ('same level, x - y = 2^-12', lambda: (lambda x: (x, x - 2.0 ** -12))(rng.uniform(1.01, 4.99))),
        ('same level, x - y = 2^-28', lambda: (lambda x: (x, x - 2.0 ** -28))(rng.uniform(1.01, 4.99))),
        ('same level, x - y = 2^-40', lambda: (lambda x: (x, x - 2.0 ** -40))(rng.uniform(1.01, 4.99))),
        ('across a level, 2^-28 each side',
         lambda: (lambda n: (n + 2.0 ** -28, n - 2.0 ** -28))(rng.choice((2, 3, 4))))):
    naive = stable = 0.0
    for _ in range(300):
        x, y = gen()
        ref, _ = co_addsub(mp.mpf(x), mp.mpf(y), True)
        zn, _ = co_addsub(x, y, True, math.exp, math.log)
        zs = co_sub_stable(x, y)
        naive = max(naive, float(abs(mp.mpf(zn) - ref)) * 2 ** 28)
        stable = max(stable, float(abs(mp.mpf(zs) - ref)) * 2 ** 28)
    print('  %-31s %10.3g %14.3g' % (label, naive, stable))
    check('expm1 form within 1e-3 ulp of fbits 28: ' + label, stable < 1e-3)
print('  -> one ulp apart (second row), the naive form is already several ulps off; the expm1')
print('     form stays near 2^-47 whatever the separation')

print()
if failures:
    print('%d check(s) FAILED' % failures)
    sys.exit(1)
print('all checks passed')
