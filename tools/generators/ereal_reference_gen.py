#!/usr/bin/env python3
# ereal_reference_gen.py: generate high-precision multi-component (Priest/Shewchuk)
# expansions for ereal<> constants and mathlib reference values.
#
# Copyright (C) 2017 Stillwater Supercomputing, Inc.
# SPDX-License-Identifier: MIT
#
# This file is part of the universal numbers project, which is released under an MIT Open Source license.
#
# PURPOSE
# -------
# An ereal<maxlimbs> value is a non-overlapping expansion of IEEE-754 doubles
# (a Priest/Shewchuk multi-component number): v == sum(limb[i]), with the limbs
# in strictly decreasing magnitude and non-overlapping. This generator produces,
# for an arbitrary arbitrary-precision value, the exact leading double-component
# expansion that an ereal would hold -- ready to paste as a C++ `static const
# double[]` initializer.
#
# It is used to produce two things the ereal subsystem depends on:
#
#   1. BASE CONSTANTS (pi, ln2, ln10) stored in
#      include/sw/universal/number/ereal/math/constants/ereal_constants.hpp .
#      These are summed back into an ereal<maxlimbs> at runtime, which is exact
#      and -- crucially -- bypasses ereal::parse(). parse() loses precision past
#      ~130 digits and cannot represent a 300-digit decimal string, so a parsed
#      constant silently caps every dependent transcendental well below full
#      precision (issue #1002, deeper root cause). A stored expansion does not.
#
#   2. GROUND-TRUTH REFERENCES for the progressive-precision regression test
#      elastic/ereal/api/progressive_precision.cpp . Each reference must be more
#      precise than the ereal level being measured, and must match the value the
#      computation actually sees (see EXACT-DOUBLE INPUTS below).
#
# EXACT-DOUBLE INPUTS (critical correctness point)
# ------------------------------------------------
# The mathlib tests feed inputs through std::stod, so e.g. cos(0.3) evaluates
# cos(double(0.3)) where double(0.3) = 0.299999999999999988... , NOT cos(3/10).
# A reference generated from the exact decimal would diverge from the
# computation after ~16 digits and produce false precision-loss reports. Every
# reference here is therefore evaluated at mpf(python_float) -- the exact double
# value of the input -- not at the exact rational/decimal.
#
# DEPENDENCY
# ----------
# Pure-Python arbitrary precision via mpmath (https://mpmath.org), used here as a
# stand-in for MPFR. Install: `pip install mpmath`. Run: `python3 ereal_reference_gen.py`.
#
# The non-overlapping expansion is extracted by the standard greedy round-to-
# nearest-double / subtract-exact loop: limb[k] = float(residual), residual -=
# mpf(limb[k]). This yields exactly the components two_sum/two_prod arithmetic
# would accumulate, in decreasing magnitude.

from mpmath import mp, mpf, sin, cos, tan, atan, asin, acos, exp, log, \
    sinh, cosh, tanh, asinh, acosh, atanh, sqrt, power, pi

# Working precision: well above the ~316 decimal digits that 19 doubles carry,
# so the extracted limbs are correct to the last representable component.
mp.dps = 400

# Smallest normal double; stop before limbs underflow into the subnormal range
# (Shewchuk's arithmetic requires normal doubles -- this is why maxlimbs <= 19).
DBL_MIN = 2.2250738585072014e-308

MAX_LIMBS = 19


def expansion(value, max_limbs=MAX_LIMBS):
    """Return the non-overlapping double-component expansion of an mpf value.

    Greedy decomposition: each limb is the nearest double to the running
    residual; the residual is then reduced by that limb (exactly, in mpf).
    Stops at max_limbs or when the residual is no longer a normal double.
    """
    residual = mpf(value)
    limbs = []
    for _ in range(max_limbs):
        d = float(residual)
        if d == 0.0 or abs(d) < DBL_MIN * 4:
            break
        limbs.append(d)
        residual = residual - mpf(d)
    return limbs


def verify(value, limbs):
    """Return the number of correct decimal digits of sum(limbs) vs value."""
    s = mpf(0)
    for d in limbs:
        s += mpf(d)
    if value == 0:
        return mp.inf
    err = abs((s - mpf(value)) / mpf(value))
    if err == 0:
        return mp.inf
    return float(mp.floor(-mp.log10(err)))


def emit_array(name, value, comment):
    """Print a C++ `static const double name[] = { ... };` initializer."""
    limbs = expansion(value)
    digits = verify(value, limbs)
    digtxt = "exact" if digits == mp.inf else "~{} decimal digits".format(int(digits))
    print("\t// {}".format(comment))
    print("\t// {} limbs, value-accurate to {} (verified vs mpmath)".format(len(limbs), digtxt))
    print("\tstatic const double {}[] = {{".format(name))
    for i in range(0, len(limbs), 3):
        chunk = limbs[i:i + 3]
        sep = "," if i + 3 < len(limbs) else ""
        print("\t\t" + ", ".join(repr(x) for x in chunk) + sep)
    print("\t};")
    print()


def generate_base_constants():
    """Base constants stored in ereal_constants.hpp (bypassing parse())."""
    print("// ===== base constants for ereal_constants.hpp =====")
    print()
    emit_array("ereal_pi_limbs",   mpf(pi),      "pi   (Archimedes' constant)")
    emit_array("ereal_ln2_limbs",  log(mpf(2)),  "ln(2) (natural log of 2)")
    emit_array("ereal_ln10_limbs", log(mpf(10)), "ln(10) (natural log of 10)")


def generate_test_references():
    """Ground-truth references for progressive_precision.cpp.

    Each entry evaluates the function at the EXACT DOUBLE value of its input
    (mpf(python_float)), matching what the std::stod-fed computation sees.
    """
    print("// ===== ground-truth references for progressive_precision.cpp =====")
    print()
    cases = [
        ("REF_sin",   sin(mpf(0.5)),                  "sin(0.5)"),
        ("REF_cos",   cos(mpf(0.3)),                  "cos(0.3)   [note: cos(double(0.3))]"),
        ("REF_tan",   tan(mpf(0.4)),                  "tan(0.4)   [note: tan(double(0.4))]"),
        ("REF_atan",  atan(mpf(1.0)),                 "atan(1.0) == pi/4"),
        ("REF_asin",  asin(mpf(0.5)),                 "asin(0.5) == pi/6"),
        ("REF_acos",  acos(mpf(0.5)),                 "acos(0.5) == pi/3"),
        ("REF_exp",   exp(mpf(1.0)),                  "exp(1.0) == e"),
        ("REF_exp2",  power(2, mpf(3.5)),             "exp2(3.5) == 2^3.5"),
        ("REF_exp10", power(10, mpf(1.5)),            "exp10(1.5) == 10^1.5"),
        ("REF_log",   log(mpf(2.0)),                  "log(2.0) == ln(2)"),
        ("REF_log2",  log(mpf(10.0)) / log(2),        "log2(10.0)"),
        ("REF_log10", log(mpf(100.0)) / log(10),      "log10(100.0) == 2"),
        ("REF_sinh",  sinh(mpf(0.5)),                 "sinh(0.5)"),
        ("REF_cosh",  cosh(mpf(0.5)),                 "cosh(0.5)"),
        ("REF_tanh",  tanh(mpf(0.5)),                 "tanh(0.5)"),
        ("REF_asinh", asinh(mpf(1.0)),                "asinh(1.0)"),
        ("REF_acosh", acosh(mpf(2.0)),                "acosh(2.0)"),
        ("REF_atanh", atanh(mpf(0.5)),                "atanh(0.5)"),
        ("REF_sqrt",  sqrt(mpf(2.0)),                 "sqrt(2.0)"),
        ("REF_pow",   power(mpf(2.0), mpf(3.5)),      "pow(2.0, 3.5)"),
    ]
    for name, value, comment in cases:
        emit_array(name, value, comment)


if __name__ == "__main__":
    generate_base_constants()
    generate_test_references()
