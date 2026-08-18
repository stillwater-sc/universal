#!/usr/bin/env python3
"""Generate the shared multi-component sin/cos tables and the trigonometry oracle vectors.

Two outputs, both derived from mpmath at 200 decimal digits:

  tables    include/sw/universal/internal/floatcascade/sincos_tables.hpp
            sin(k*pi/1024) and cos(k*pi/1024) for k = 1..256, each as four
            doubles.  The values are the ones the qd sincos tables already
            carry (Bailey/Hida); this script verifies them rather than
            replacing them, so qd and qd_cascade evaluate from identical
            constants.  Every truncation prefix is checked: the first two
            limbs must hold double-double accuracy, the first three
            triple-double, all four quad-double -- that is what lets one
            table serve all three cascade widths.

  vectors   the reference block pasted into
            static/highprecision/qd_cascade/math/trigonometry_oracle.cpp
            sin(x) and cos(x) for a set of arguments, each as six doubles
            (~318 bits) so the reference stays well clear of the widest
            format under test, plus the guard bits each argument needs.

            The guard is the condition number of the function at that point,
            log2|x f'(x) / f(x)|, rounded up.  Near a zero of sin or cos the
            result is tiny while the argument is not, so no implementation can
            hold relative accuracy there: sin(3.141592653589793) is 1.2e-16 and
            an input perturbation of one ulp moves it by half of that.  Charging
            each argument its own condition number is what lets the suite demand
            full precision everywhere else without exempting the hard cases by
            hand.  It also covers the argument reduction, whose error grows with
            |x| at the same rate.

Usage:  python3 tools/generators/cascade_trig_gen.py tables | vectors

Requires mpmath.  Not run during the build; the generated header is checked in.
"""
import re
import sys

import mpmath as mp

mp.mp.dps = 200

QD_TABLES = 'include/sw/universal/number/qd/math/constants/sincos_tables.hpp'
TABLE_SIZE = 256

# how many decimal digits each truncation prefix has to deliver
PREFIX_REQUIREMENT = {2: 31.9, 3: 47.8, 4: 63.8}


def split(x, n):
    """Split a high-precision value into n doubles, most significant first."""
    limbs = []
    for _ in range(n):
        d = float(x)
        limbs.append(d)
        x = x - mp.mpf(d)
    return limbs


def digits(got, exact):
    if exact == 0:
        return 200.0
    rel = abs((got - exact) / exact)
    return 200.0 if rel == 0 else float(-mp.log10(rel))


def read_qd_table(src, name):
    m = re.search(r'qd_' + name + r'_table\s*\[\]\s*=\s*\{(.*?)\n\t\t\};', src, re.S)
    if m is None:
        raise SystemExit('could not find qd_%s_table in %s' % (name, QD_TABLES))
    rows = re.findall(r'qd\(([^)]*)\)', m.group(1))
    return [[float(v) for v in row.split(',')] for row in rows]


def read_qd_pi1024(src):
    m = re.search(r'qd_pi1024\s*=\s*\n?\s*qd\(([^)]*)\)', src, re.S)
    if m is None:
        raise SystemExit('could not find qd_pi1024 in %s' % QD_TABLES)
    return [float(v) for v in m.group(1).split(',')]


def verify(name, table, fn):
    """Check every entry at every truncation prefix; abort on a short one."""
    if len(table) != TABLE_SIZE:
        raise SystemExit('%s has %d entries, expected %d' % (name, len(table), TABLE_SIZE))
    for prefix, need in sorted(PREFIX_REQUIREMENT.items()):
        worst, worst_k = 1e9, None
        for k, row in enumerate(table, start=1):
            exact = fn(k * mp.pi / 1024)
            d = digits(sum(mp.mpf(v) for v in row[:prefix]), exact)
            if d < worst:
                worst, worst_k = d, k
        if worst < need:
            raise SystemExit('%s prefix %d: only %.1f digits at k=%d, need %.1f'
                             % (name, prefix, worst, worst_k, need))
        sys.stderr.write('  %s first %d limbs: worst %.1f digits (k=%d), need %.1f  OK\n'
                         % (name, prefix, worst, worst_k, need))


def limbs(row):
    return ', '.join('%+.17e' % v for v in row)


def emit_tables():
    src = open(QD_TABLES).read()
    sin_table = read_qd_table(src, 'sin')
    cos_table = read_qd_table(src, 'cos')
    pi1024 = read_qd_pi1024(src)

    sys.stderr.write('verifying against mpmath at %d digits:\n' % mp.mp.dps)
    verify('sin', sin_table, mp.sin)
    verify('cos', cos_table, mp.cos)
    d = digits(sum(mp.mpf(v) for v in pi1024), mp.pi / 1024)
    if d < PREFIX_REQUIREMENT[4]:
        raise SystemExit('pi/1024: only %.1f digits' % d)
    sys.stderr.write('  pi/1024: %.1f digits  OK\n' % d)

    out = []
    w = out.append
    w('#pragma once')
    w('// sincos_tables.hpp: sin/cos tables for the multi-component cascade types')
    w('//')
    w('// Copyright (C) 2017 Stillwater Supercomputing, Inc.')
    w('// SPDX-License-Identifier: MIT')
    w('//')
    w('// This file is part of the universal numbers project, which is released under an MIT Open Source license.')
    w('//')
    w('// GENERATED by tools/generators/cascade_trig_gen.py -- do not edit by hand.')
    w('//')
    w('// sin(k*pi/1024) and cos(k*pi/1024) for k = 1..256, four doubles each.')
    w('//')
    w('// One table serves every cascade width. A correctly rounded expansion stays a valid,')
    w('// correctly rounded expansion when you drop its trailing limbs, so dd_cascade reads the')
    w('// first two doubles, td_cascade the first three, qd_cascade all four. The generator')
    w('// verifies each prefix against mpmath: 32, 48 and 64 decimal digits respectively.')
    w('//')
    w('// The values are the ones the qd sincos tables carry (Bailey/Hida), so qd and qd_cascade')
    w('// evaluate sin and cos from identical constants.')
    w('//')
    w('// Why pi/1024 and not the pi/16 the double-double path uses: the Taylor series is')
    w('// evaluated on the reduced argument, and a 15-entry inverse-factorial table only reaches')
    w('// quad-double precision if that argument is below pi/2048. Reducing to pi/32 leaves a')
    w('// series that would need terms out to 1/65! -- which is why the cascade types capped at')
    w('// double-double accuracy no matter how wide they were (universal#1318).')
    w('')
    w('namespace sw { namespace universal { namespace cascade {')
    w('')
    w('constexpr unsigned sincos_table_size = %d;' % TABLE_SIZE)
    w('')
    w('// pi/1024')
    w('constexpr double pi1024[4] = { %s };' % limbs(pi1024))
    w('')
    w('// sin(k * pi/1024), k = 1..%d' % TABLE_SIZE)
    w('constexpr double sin_pi1024[sincos_table_size][4] = {')
    for row in sin_table:
        w('\t{ %s },' % limbs(row))
    w('};')
    w('')
    w('// cos(k * pi/1024), k = 1..%d' % TABLE_SIZE)
    w('constexpr double cos_pi1024[sincos_table_size][4] = {')
    for row in cos_table:
        w('\t{ %s },' % limbs(row))
    w('};')
    w('')
    w('} } }  // namespace sw::universal::cascade')
    w('')
    sys.stdout.write('\n'.join(out))


# arguments chosen to exercise the reduction: both branches of the pi/2 quadrant
# selection, the k == 0 shortcut and the table path, arguments needing a modulo
# 2*pi reduction, and values close to a zero of sin or cos where the result is
# small and the reduction has to be accurate.
VECTORS = [
    '1.0e-8', '0.001953125', '0.1', '0.5', '0.75', '1.0',
    '1.2345678901234567', '1.5', '1.5707963267948966', '1.75', '2.0',
    '2.5', '3.0', '3.141592653589793', '4.0', '5.0',
    '6.283185307179586', '7.0', '10.0', '100.0', '1000.0',
    '-0.5', '-2.5', '-12.566370614359172',
]
# arguments for the inverse functions, inside the asin/acos domain and including
# points where acos and atan approach a zero.
INVERSE_VECTORS = [
    '-0.99', '-0.75', '-0.5', '-0.125', '-1.0e-8', '1.0e-8', '0.125', '0.25',
    '0.5', '0.7071067811865476', '0.75', '0.875', '0.99', '0.9999999999999999',
]
REFERENCE_LIMBS = 6


FUNCTIONS = {
    'sin':  mp.sin,
    'cos':  mp.cos,
    'tan':  mp.tan,
    'asin': mp.asin,
    'acos': mp.acos,
    'atan': mp.atan,
}


def guard(x, fname):
    """Bits of relative accuracy the conditioning of the function costs at x.

    The condition number of f at x is |x f'(x) / f(x)|; it blows up as f
    approaches a zero -- which is where sin, tan, atan and acos each spend part
    of their range -- and grows like |x| for a large argument, where the
    reduction modulo 2*pi loses the same bits.
    """
    xm = mp.mpf(x)
    if xm == 0:
        return 0
    f = FUNCTIONS[fname]
    value = f(xm)
    if value == 0:
        raise SystemExit('%s(%s) is exactly zero: no relative accuracy to speak of' % (fname, x))
    cond = abs(xm * mp.diff(f, xm) / value)
    return max(0, int(mp.ceil(mp.log(cond, 2))))


def emit_table(w, name, struct, args, fnames, comment):
    w('\tstruct %s {' % struct)
    w('\t\tdouble   x;')
    for fname in fnames:
        w('\t\tunsigned %s_guard;' % fname)
    for fname in fnames:
        w('\t\tdouble   %s[reference_limbs];' % fname)
    w('\t};')
    w('\t// %s' % comment)
    w('\tconstexpr %s %s[] = {' % (struct, name))
    for text in args:
        x = float(text)
        w('\t\t{ %+.17e, %s,' % (x, ', '.join(str(guard(x, f)) for f in fnames)))
        for i, fname in enumerate(fnames):
            value = FUNCTIONS[fname](mp.mpf(x))
            limb = split(value, REFERENCE_LIMBS)
            if digits(sum(mp.mpf(v) for v in limb), value) < 90.0:
                raise SystemExit('reference for %s(%s) is short' % (fname, text))
            w('\t\t  { %s }%s' % (limbs(limb), ',' if i + 1 < len(fnames) else ' },'))
    w('\t};')


def emit_vectors():
    out = []
    w = out.append
    w('\t// GENERATED by tools/generators/cascade_trig_gen.py -- do not edit by hand.')
    w('\t//')
    w('\t// Reference values at %d decimal digits, carried as %d doubles (~%d bits) so the'
      % (mp.mp.dps, REFERENCE_LIMBS, REFERENCE_LIMBS * 53))
    w('\t// reference stays well clear of the widest format under test. Each argument also carries')
    w('\t// the guard bits its own conditioning costs, log2|x f\'(x)/f(x)| rounded up.')
    w('\tconstexpr unsigned reference_limbs = %d;' % REFERENCE_LIMBS)
    w('')
    emit_table(w, 'trig_reference', 'TrigReference', VECTORS, ['sin', 'cos', 'tan'],
               'sin, cos and tan: arguments spanning the reduction paths')
    w('')
    emit_table(w, 'inverse_reference', 'InverseReference', INVERSE_VECTORS, ['asin', 'acos', 'atan'],
               'asin, acos and atan: arguments inside the asin/acos domain')
    sys.stdout.write('\n'.join(out) + '\n')


if __name__ == '__main__':
    if len(sys.argv) != 2 or sys.argv[1] not in ('tables', 'vectors'):
        raise SystemExit(__doc__)
    if sys.argv[1] == 'tables':
        emit_tables()
    else:
        emit_vectors()
