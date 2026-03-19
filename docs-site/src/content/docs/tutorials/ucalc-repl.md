---
title: "ucalc: Mixed-Precision REPL Calculator"
description: Interactive exploration of number system behavior across 30+ arithmetic types
---

`ucalc` is an interactive calculator for exploring and comparing arithmetic
across Universal number types. Instead of writing, compiling, and running C++
for each experiment, you can compare representations, measure precision, and
analyze errors interactively.

## Quick Start

```bash
# Build
cmake -DUNIVERSAL_BUILD_TOOLS_UCALC=ON ..
make ucalc

# Interactive
ucalc

# One-shot
ucalc "type posit32; show 1/3"

# Pipe mode
echo "compare sqrt(2)" | ucalc
```

## Commands Reference

| Command | Description |
|---------|-------------|
| `type <name>` | Set the active arithmetic type |
| `types` | List all available types |
| `show <expr>` | Value + binary decomposition + components |
| `compare <expr>` | Evaluate across all types in a table |
| `range` | Symmetry range: [maxneg ... minneg] 0 [minpos ... maxpos] |
| `precision` | Binary/decimal digits, epsilon, minpos, maxpos |
| `ulp <value>` | Unit in the last place at a given value |
| `bits <expr>` | Raw bit pattern |
| `sweep <expr> for <var> in [a, b, n]` | Error analysis across a range |
| `faithful <expr>` | Check faithful rounding vs higher-precision reference |
| `color on/off` | Toggle ANSI color-coded bit fields |
| `vars` | List defined variables |
| `help` | Command reference |

Expressions support standard arithmetic (`+`, `-`, `*`, `/`, `^`), parentheses,
variables (`x = expr`), constants (`pi`, `e`, `phi`, `ln2`, `ln10`, `sqrt2`),
and functions (`sqrt`, `abs`, `log`, `exp`, `sin`, `cos`, `pow`).

Constants are sourced at quad-double precision (~64 decimal digits) and
converted to the active type at its native precision.

---

## Example 1: Posit Closure -- Why Does 1/3 + 1/3 + 1/3 = 1?

The classic demonstration of posit arithmetic's tapered precision. Posits
allocate more fraction bits near 1.0, so the three rounded thirds sum
exactly to 1 -- something IEEE floats cannot guarantee.

```text
posit32> show 1/3 + 1/3 + 1/3
  value:      1.00000000000000000e+00
  binary:     0b0.10.00.000000000000000000000000000
  components: sign: +, regime: 0, exponent: 1, significand: 1
  type:       posit< 32, 2, uint32_t>
```

Compare with IEEE single precision, which also rounds to 1.0 for this
expression but through a different mechanism (lucky cancellation of the
rounding errors in float, not a structural guarantee):

```text
float> show 1/3 + 1/3 + 1/3
  value:      1
  binary:     0b0.01111111.00000000000000000000000
  components: sign: +, scale: 0, significand: 1
  type:       float
```

Use `compare 1/3` to see how each type represents the fraction and where
the rounding errors lie.

---

## Example 2: How 0.1 Looks Across 30 Types

The decimal value 0.1 cannot be represented exactly in binary floating-point.
The `compare` command reveals how each type approximates it:

```text
double> compare 1/10
Type                            Value  Binary
--------------------------------------------------------------------------------
float             0.10000000149011612  0b0.01111011.10011001100110011001101
double            0.10000000000000001  0b0.01111111011.100110011001100110...
posit8        1.01562500000000000e-01  0b0.01.00.101
posit16       1.00006103515625000e-01  0b0.01.00.10011001101
posit32       1.00000000093132257e-01  0b0.01.00.100110011001100110011001101
fp128         1.00000000000000000e-01  0b0.011111111111011.10011001100110...
decimal32                         0.1  0b0.01001.100100.00000...
decimal64                         0.1  0b0.01001.10001101.00000...
```

Notice that `decimal32` and `decimal64` represent 0.1 exactly -- they use
base-10 encoding, so 1/10 is representable. Every binary type introduces
rounding error, but the magnitude differs by orders of magnitude across types.

---

## Example 3: The Golden Ratio Identity -- Measuring Arithmetic Fidelity

The golden ratio satisfies phi^2 - phi - 1 = 0. How close does each
type get?

```text
posit32> x = phi
1.61803399026393890e+00
posit32> show x * x - x - 1
  value:      0.00000000000000000e+00
  binary:     0b0.0000000000000000000000000000000..
  type:       posit< 32, 2, uint32_t>
```

Posit32 evaluates to exactly zero. Now try IEEE single:

```text
fp32> y = phi
1.61803400516510010e+00
fp32> show y * y - y - 1
  value:      1.19209289550781250e-07
  binary:     0b0.01101000.00000000000000000000000
  components: sign: +, scale: -23, significand: 1.000000000e+00
  type:       fp32 (IEEE-754 binary32)
```

The residual is 1.2e-7 -- one ULP at the scale of the computation. Double-double
eliminates the residual entirely by carrying twice the precision:

```text
dd> z = phi
1.61803398874989485e+00
dd> show z * z - z - 1
  value:      0.00000000000000000e+00
  type:       double-double
```

---

## Example 4: Dynamic Range Comparison Across 16-bit Types

The `range` command reveals how different 16-bit types trade precision for
range. Compare IEEE fp16, Google bfloat16, posit16, and lns16:

```text
fp16> range
fp16 (IEEE-754 binary16)
[ -6.55040e+04 ... -5.96046e-08  0  5.96046e-08 ... 6.55040e+04 ]

bfloat16> range
bfloat16
[ -3.38953e+38 ... -1.17549e-38  0  1.17549e-38 ... 3.38953e+38 ]

posit16> range
posit< 16, 2, uint16_t>
[ -7.20576e+16 ... -1.38778e-17  0  1.38778e-17 ... 7.20576e+16 ]

lns16> range
lns< 16, 8, uint16_t, Saturating>
[ -1.83969e+19 ... -5.43571e-20  0  5.43571e-20 ... 1.83969e+19 ]
```

All four are 16 bits, but their tradeoffs are dramatic:

| Type | Dynamic Range (decades) | Precision (digits) |
|------|------------------------|--------------------|
| fp16 | ~13 decades | 3.0 |
| bfloat16 | ~76 decades | 2.1 |
| posit16 | ~34 decades | 3.3 |
| lns16 | ~39 decades | ~2.4 |

bfloat16 matches float's range but sacrifices precision. posit16 delivers
more precision than fp16 AND more range. lns16 achieves the widest range
of any 16-bit format by encoding values as logarithms.

---

## Example 5: Precision Ladder -- From 8-bit to 32-bit

The `precision` command measures each type's effective precision at 1.0.
This reveals how bit-width translates to decimal accuracy:

```text
fp8e4m3> precision
  type:           fp8e4m3 (OFP 8-bit e4m3)
  binary digits:  3
  decimal digits: 0.9
  epsilon:        1.25000000000000000e-01

posit16> precision
  type:           posit< 16, 2, uint16_t>
  binary digits:  11
  decimal digits: 3.3
  epsilon:        4.88281250000000000e-04

fp32> precision
  type:           fp32 (IEEE-754 binary32)
  binary digits:  23
  decimal digits: 6.9
  epsilon:        1.19209289550781250e-07

posit32> precision
  type:           posit< 32, 2, uint32_t>
  binary digits:  27
  decimal digits: 8.1
  epsilon:        7.45058059692382813e-09
```

At 32 bits, posit delivers 27 binary digits near 1.0 compared to IEEE
float's 23 -- a 16x smaller epsilon. This extra precision is what enables
the 1/3 + 1/3 + 1/3 = 1 property.

---

## Example 6: Catastrophic Cancellation

Subtracting nearly equal quantities destroys significant digits. The
expression `1 + 1e-8 - 1` should yield 1e-8 but exercises catastrophic
cancellation:

```text
fp32> show 1 + 1e-8 - 1
  value:      0.00000000000000000e+00
  components: sign: +, zero
  type:       fp32 (IEEE-754 binary32)
```

IEEE single loses the 1e-8 term entirely -- it's below the ULP at 1.0
(which is ~1.2e-7). posit32 preserves the term:

```text
posit32> show 1 + 1e-8 - 1
  value:      7.45058059692382813e-09
  components: sign: +, regime: -7, exponent: 2, significand: 1
  type:       posit< 32, 2, uint32_t>
```

The posit result (7.45e-9) is the nearest posit representable to 1e-8.
Double-double recovers nearly full accuracy:

```text
dd> show 1 + 1e-8 - 1
  value:      9.99999993922529029e-09
  type:       double-double
```

This pattern is critical in numerical methods where loss-of-significance
in intermediate results cascades into large final errors.

---

## Example 7: Faithful Rounding Verification

A result is *faithfully rounded* if it equals one of the two representable
values adjacent to the exact answer. The `faithful` command checks this
against a quad-double reference:

```text
posit32> faithful sqrt(2)
  result:    1.4142135605216026
  reference: 1.4142135623730951
  rounded:   1.4142135605216026
  neighbor:  1.4142135679721832
  faithful:  yes

fp32> faithful sqrt(2)
  result:    1.4142135381698608
  reference: 1.4142135623730951
  rounded:   1.4142135381698608
  neighbor:  1.4142136573791504
  faithful:  yes
```

Both posit32 and fp32 produce faithfully rounded sqrt(2). The `rounded`
value is the nearest representable, and `neighbor` is the next representable
in the opposite direction. The result must equal one of them.

---

## Example 8: Transcendental Error Profiles with Sweep

The `sweep` command evaluates an expression across a range and reports
ULP error vs a double-precision reference. This reveals where type-specific
approximation errors concentrate:

```text
posit32> sweep sin(x) for x in [0, 3.14159, 6]
x                    result               double ref      ULP error
---------------------------------------------------------------------
0                         0                      0           0.00
0.628318          0.587785            0.587785           0.04
1.256636          0.951056            0.951056           0.44
1.884954          0.951057            0.951057           0.31
2.513272          0.587787            0.587787           0.85
3.14159           2.654e-06           2.654e-06       20261.95
```

The error is sub-ULP through most of the range but explodes near pi
where argument reduction subtracts nearly equal quantities. This is a
fundamental limitation shared by all binary types -- the result near
sin(pi) depends on how many digits of pi the type can represent.

---

## Available Types

ucalc registers 35 types spanning the major number system families:

| Family | Types |
|--------|-------|
| Native IEEE | float, double |
| Posit | posit8, posit16, posit32, posit64 |
| Classic float | bfloat16, fp16, fp32, fp64, fp128 |
| FP8 (Deep Learning) | fp8e2m5, fp8e3m4, fp8e4m3, fp8e5m2 |
| Fixed-point | fixpnt16, fixpnt32 |
| Logarithmic | lns8, lns16, lns32 |
| Integer | int8, int16, int32, int64 |
| Hexadecimal float | hfloat32, hfloat64 |
| Decimal float | decimal32, decimal64 |
| Rational | rational8, rational16, rational32 |
| Multi-component | dd, dd_cascade, td_cascade, qd, qd_cascade |
