#!/usr/bin/env python3
"""
generate_reference_constants.py

Generate ~320-decimal-digit reference strings for mathematical constants
used by Universal's multi-component types (ereal<N>, elreal, ...).

Background
==========
An IEEE 754 double has 53 bits of significand and a maximum exponent of
~ 2^1023, so the natural ceiling for a multi-component representation
over double components (with the Shewchuk non-overlapping property) is
floor(1023 / 53) = 19 components for values near 1. In decimal digits,
that is roughly 1023 / 3.32 ~ 308 digits. We emit 320 digits so that
the round-trip through string -> ereal<19> has comfortable margin
beyond what any 19-component expansion can capture.

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
the scientific-computing community. Constants are computed at 350
working digits and printed at 320 to avoid trailing-digit rounding
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


# Working precision: 350 decimal digits gives ~ 30-digit headroom past
# the 320 we emit. Trailing digits in the printout are guaranteed
# correctly rounded.
mp.mp.dps = 350

# Target output precision in decimal digits.
EMIT_DPS = 320


def fmt(name: str, value, comment: str) -> str:
    """Format one constant: name, value to EMIT_DPS digits, trailing comment."""
    s = mp.nstr(value, EMIT_DPS, strip_zeros=False)
    return f'inline constexpr std::string_view s_{name} =\n    "{s}";  // {comment}'


def main() -> None:
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
