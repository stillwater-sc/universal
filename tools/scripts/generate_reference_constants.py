#!/usr/bin/env python3
"""
generate_reference_constants.py

Generate ~340-decimal-digit reference strings for mathematical constants
used by Universal's multi-component types (ereal<N>, elreal, ...).

Background
==========
An IEEE 754 double has 53 bits of significand and a maximum exponent of
~ 2^1023, so the natural ceiling for a multi-component representation
over double components (with the Shewchuk non-overlapping property) is
floor(1023 / 53) = 19 components for values near 1. In decimal digits,
that is roughly 1023 / 3.32 ~ 308 digits. We emit 340 digits: margin
beyond what any 19-component expansion can capture for the round-trip
through string -> ereal<19>, and twenty digits beyond the 320-digit cap
that agreement is measured to, so that no reference's own rounding is
ever held against the value being checked (see EMIT_DPS below).

Reproducibility
===============
This script is the canonical source of truth for the strings in
`include/sw/math/constants/reference_constants.hpp`. To regenerate:

    pip install mpmath
    python3 tools/scripts/generate_reference_constants.py > /tmp/ref.txt

Then copy the relevant strings into the header. Each constant is
emitted on a single line for direct paste.

Validation
==========
After regeneration, the cross-check test at
`elastic/ereal/math/constants/reference_constants.cpp` parses each
string into `ereal<19>` and verifies the leading 4 components match
the precomputed quad-double expansions in `qd_constants.hpp`. Any drift
between the two sources should be investigated -- both ultimately
derive from the same mathematical constants, so a mismatch indicates a
bug or a transcription error.

Generation source
=================
mpmath provides arbitrary-precision floating-point arithmetic in pure
Python. It is the standard tool for generating reference constants in
the scientific-computing community. Constants are computed at 380
working digits and printed at 340 to avoid trailing-digit rounding
artifacts.

License
=======
mpmath is BSD-licensed. The constants themselves are mathematical
truths and carry no license. Universal's MIT license applies to this
script.
"""

import sys

try:
    import mpmath as mp
except ImportError:
    print(
        "ERROR: mpmath is not installed.\n"
        "Install with: pip install mpmath",
        file=sys.stderr,
    )
    sys.exit(1)


# Target output precision in decimal digits.
#
# 340, not 320, although nothing certifies agreement past 320 digits (the default
# cap of agreed_decimal_digits). A reference cannot certify its OWN last digit: a
# value correctly rounded to 320 digits is off by up to half a unit in that digit,
# which for a constant in [1, 10) is up to 5e-320 -- more than the 1e-320 relative
# error that 320 digits of agreement allows. So a PERFECT evaluation scored 319
# against s_sqrt2, whose digits after the 320th are 2964..., and every benchmark
# row that stops at 320 digits ran to the end of its depth ladder (#1397). pi
# reached 320 only because its tail past the 320th digit happens to be small.
# Twenty guard digits put each reference's own error far below anything measured.
EMIT_DPS = 340

# Working precision: 40 digits of headroom past what we emit. Trailing digits in
# the printout are guaranteed correctly rounded; the large-argument trig values
# (sin(1e20) etc.) spend some of it on argument reduction.
mp.mp.dps = EMIT_DPS + 40


def fmt(name: str, value, comment: str) -> str:
    """Format one constant: name, value to EMIT_DPS digits, trailing comment."""
    s = mp.nstr(value, EMIT_DPS, strip_zeros=False)
    return f'inline constexpr std::string_view s_{name} =\n    "{s}";  // {comment}'


# The long table: a few constants at LONG_EMIT_DPS digits, for values that carry
# thousands of them. ereal with x87 or binary128 limbs reaches ~4900 digits (#1355), and
# a 340-digit reference cannot certify any of it. Emitted to a SEPARATE header
# (long_reference_constants.hpp) so that the everyday one stays small: this table is
# ~45 KB of string literals, and only the tests that need it include it.
LONG_EMIT_DPS = 5000
LONG_CAP = 4900          # what these references certify: see agreed_decimal_digits


def fmt_long(name: str, value, comment: str) -> str:
    s = mp.nstr(value, LONG_EMIT_DPS, strip_zeros=False)
    return f'inline constexpr std::string_view s_long_{name} =\n    "{s}";  // {comment}'


def emit_long() -> None:
    """The long table. Run: generate_reference_constants.py --long > the header."""
    mp.mp.dps = LONG_EMIT_DPS + 40
    constants = [
        ("pi",          mp.pi,                  "pi"),
        ("e",           mp.e,                   "e"),
        ("sqrt2",       mp.sqrt(2),             "sqrt(2)"),
        ("sqrt3",       mp.sqrt(3),             "sqrt(3)"),
        ("sqrt5",       mp.sqrt(5),             "sqrt(5)"),
        ("phi",         (1 + mp.sqrt(5)) / 2,   "phi (golden ratio)"),
        ("ln2",         mp.log(2),              "ln(2)"),
        ("ln10",        mp.log(10),             "ln(10)"),
        ("euler_gamma", mp.euler,               "Euler-Mascheroni gamma"),
    ]
    print("#pragma once")
    print("// long_reference_constants.hpp: reference constants at " + str(LONG_EMIT_DPS) + " decimal digits")
    print("//")
    print("// Auto-generated by tools/scripts/generate_reference_constants.py --long")
    print(f"// mpmath dps = {mp.mp.dps}, emitted dps = {LONG_EMIT_DPS}")
    print("//")
    print("// For values that carry more digits than reference_constants.hpp's 340 can certify:")
    print("// ereal with x87 or binary128 limbs reaches about 4900 (#1355, #1566). Read these at")
    print(f"// a cap of {LONG_CAP}, below their own digit count -- a reference correctly rounded to D")
    print("// digits cannot certify its own last one (#1397).")
    print("//")
    print("// This header is ~45 KB of string literals and is NOT included by any number system;")
    print("// include it only where a multi-thousand-digit claim is being checked.")
    print("//")
    print("// Copyright (C) 2017 Stillwater Supercomputing, Inc.")
    print("// SPDX-License-Identifier: MIT")
    print("//")
    print("// This file is part of the universal numbers project, which is released under an MIT Open Source license.")
    print("#include <string_view>")
    print()
    print("namespace sw { namespace universal {")
    print()
    print(f"// what these references can certify, for the `cap` argument of agreed_decimal_digits")
    print(f"inline constexpr int kLongReferenceCap = {LONG_CAP};")
    print()
    for name, value, comment in constants:
        print(fmt_long(name, value, comment))
        print()
    print("}}  // namespace sw::universal")


def main() -> None:
    if len(sys.argv) > 1 and sys.argv[1] == "--long":
        emit_long()
        return
    constants = []

    # Pi multiples and fractions
    pi = mp.pi
    constants.append(("pi",       pi,                    "pi"))
    constants.append(("pi_2",     pi / 2,                "pi/2"))
    constants.append(("pi_3",     pi / 3,                "pi/3"))
    constants.append(("pi_4",     pi / 4,                "pi/4"))
    constants.append(("two_pi",   2 * pi,                "2*pi"))
    constants.append(("three_pi", 3 * pi,                "3*pi"))
    constants.append(("inv_pi",   1 / pi,                "1/pi"))
    constants.append(("two_inv_pi", 2 / pi,              "2/pi"))

    # Euler's number
    e = mp.e
    constants.append(("e",      e,           "e"))
    constants.append(("inv_e",  1 / e,       "1/e"))

    # Golden ratio
    phi = (1 + mp.sqrt(5)) / 2
    constants.append(("phi",     phi,        "phi (golden ratio)"))
    constants.append(("inv_phi", 1 / phi,    "1/phi"))

    # Square roots
    constants.append(("sqrt2",     mp.sqrt(2),     "sqrt(2)"))
    constants.append(("sqrt3",     mp.sqrt(3),     "sqrt(3)"))
    constants.append(("sqrt5",     mp.sqrt(5),     "sqrt(5)"))
    constants.append(("inv_sqrt2", 1 / mp.sqrt(2), "1/sqrt(2)"))

    # Logarithms
    constants.append(("ln2",     mp.log(2),         "ln(2)"))
    constants.append(("ln10",    mp.log(10),        "ln(10)"))
    constants.append(("log2e",   1 / mp.log(2),     "log2(e) = 1/ln(2)"))
    constants.append(("log2_10", mp.log(10) / mp.log(2), "log2(10)"))
    constants.append(("log10e",  1 / mp.log(10),    "log10(e) = 1/ln(10)"))
    constants.append(("log10_2", mp.log(2) / mp.log(10), "log10(2)"))

    # erf-related
    constants.append(("two_over_sqrt_pi", 2 / mp.sqrt(mp.pi),
                      "2/sqrt(pi) (the erf scaling factor)"))

    # Transcendental spot-check values at the exactly-representable argument 1/2
    # (and pi/6 = asin(1/2)), for the high-precision transcendental hardening
    # suite (#1049). 1/2 is an exact double, so mpf('0.5') is the same value the
    # elreal evaluation sees -- no decimal-vs-binary ambiguity. exp(1)=e, log(2)=ln2,
    # atan(1)=pi/4, acos(1/2)=pi/3 are covered by existing strings above.
    half = mp.mpf('0.5')
    constants.append(("pi_6",      pi / 6,         "pi/6 = asin(1/2)"))
    constants.append(("sin_half",  mp.sin(half),   "sin(1/2)"))
    constants.append(("cos_half",  mp.cos(half),   "cos(1/2)"))
    constants.append(("tan_half",  mp.tan(half),   "tan(1/2)"))
    constants.append(("sinh_half", mp.sinh(half),  "sinh(1/2)"))
    constants.append(("cosh_half", mp.cosh(half),  "cosh(1/2)"))
    constants.append(("tanh_half", mp.tanh(half),  "tanh(1/2)"))

    # Large-argument sin/cos spot-checks for the Payne-Hanek range-reduction
    # suite (#1050). 1e3/1e6/1e12/1e20 are all exact doubles (5^k < 2^53), so
    # mpf(v) is the same value the elreal evaluation sees. These stress the
    # accurate-quotient reduction where a host-double estimate of x/(pi/2) has
    # lost all its low bits.
    for e in (3, 6, 12, 20):
        v = mp.mpf('1e%d' % e)
        constants.append(("sin_1e%d" % e, mp.sin(v), "sin(1e%d)" % e))
        constants.append(("cos_1e%d" % e, mp.cos(v), "cos(1e%d)" % e))

    # Euler-Mascheroni gamma
    constants.append(("euler_gamma", mp.euler, "Euler-Mascheroni gamma"))

    # Banner
    print("// Auto-generated by tools/scripts/generate_reference_constants.py")
    print(f"// mpmath dps = {mp.mp.dps}, emitted dps = {EMIT_DPS}")
    print(f"// Total: {len(constants)} constants")
    print()
    for name, value, comment in constants:
        print(fmt(name, value, comment))
        print()


if __name__ == "__main__":
    main()
