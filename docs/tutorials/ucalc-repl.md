# ucalc: Mixed-Precision REPL Calculator

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

## Example 1: Precision Near 1.0 -- Where Posit Outshines IEEE

Posit's tapered precision allocates more fraction bits near 1.0 than
IEEE float does at the same bit width. This means posit32 has a smaller
epsilon (7.45e-9 vs 1.19e-7), and can resolve smaller perturbations.

```text
double> type float
Active type: float (float (IEEE-754 binary32))
float> precision
  type:           float (IEEE-754 binary32)
  binary digits:  23
  decimal digits: 6.9
  epsilon:        1.1920929e-07
  minpos:         1.40129846e-45
  maxpos:         3.40282347e+38
float> type posit32
Active type: posit32 (posit< 32, 2, uint32_t>)
posit32> precision
  type:           posit< 32, 2, uint32_t>
  binary digits:  27
  decimal digits: 8.1
  epsilon:        7.450580597e-09
  minpos:         7.523163845e-37
  maxpos:         1.329227996e+36

```

---

## Example 2: How 0.1 Looks Across 36 Types

The decimal value 0.1 cannot be represented exactly in binary floating-point.
The `compare` command reveals how each type approximates it, grouped by
bit width (small <=32, medium 33-80, large >80):

```text
double> compare 1/10
Type                         Value  Binary
----------------------------------------------------------------------
float                  0.100000001  0b0.01111011.10011001100110011001101
posit8                    1.02e-01  0b0.01.00.101
posit16                 1.0001e-01  0b0.01.00.10011001101
posit32            1.000000001e-01  0b0.01.00.100110011001100110011001101
bfloat16                       0.1  0x0.01111011.1001100
fp16                    9.9976e-02  0b0.01011.1001100110
fp32                9.99999940e-02  0b0.01111011.10011001100110011001100
...
decimal32                0.1000000  0b0.01001.011110.00000000000000000000
rational32                     0.1  0b0000'...0001 / 0b0000'...1010
```

Notice that `decimal32` and `rational32` represent 0.1 exactly -- decimal
uses base-10 encoding and rational stores the fraction 1/10 directly. Every
binary type introduces rounding error, but the magnitude differs by orders
of magnitude across types.

---

## Example 3: The Golden Ratio Identity -- Measuring Arithmetic Fidelity

The golden ratio satisfies phi^2 - phi - 1 = 0. With native-precision
arithmetic, each type reveals its true residual:

```text
posit32> x = phi
1.618033990e+00
posit32> show x * x - x - 1
  value:      0.000000000e+00
  binary:     0b0.0000000000000000000000000000000..
  components: sign: +, regime: -31, exponent: 1, significand: 1
  type:       posit< 32, 2, uint32_t>
```

Posit32 evaluates to exactly zero (lucky cancellation at this precision).
IEEE single shows a residual of one ULP:

```text
fp32> y = phi
1.61803401e+00
fp32> show y * y - y - 1
  value:      1.19209290e-07
  binary:     0b0.01101000.00000000000000000000000
  components: sign: +, scale: -23, significand: 1.000000000e+00
  type:       fp32 (IEEE-754 binary32)
```

Double-double reveals a residual at its own machine epsilon (~1e-33):

```text
dd> z = phi
1.6180339887498948482045868343656e+00
dd> show z * z - z - 1
  value:      -6.1629758220391547297791294162718e-33
  binary:     0b1.01110010100.000...0|000...0
  components: double-double: -6.16298e-33
  type:       double-double
```

Posit32 is lucky in this expression as the rounding of phi and phi^2
are in the same direction and yield values exactly 1.0 apart:

```text
Active type: posit32 (posit< 32, 2, uint32_t>)
posit32> x = phi
1.618033990e+00
posit32> xsqr = phi * phi
2.618033990e+00
posit32> vars
  posit32     x          = 1.618033990e+00
  posit32     xsqr       = 2.618033990e+00

posit32> show x
  value:      1.618033990e+00
  color:      01000100111100011011101111001110
  components: sign: +, regime: 0, exponent: 1, significand: 1.61803399026393890381
  type:       posit< 32, 2, uint32_t>
posit32> show xsqr
  value:      2.618033990e+00
  color:      01001010011110001101110111100111
  components: sign: +, regime: 0, exponent: 2, significand: 1.3090169951319694519
  type:       posit< 32, 2, uint32_t>
posit32> show xsqr - x
  value:      1.000000000e+00
  color:      01000000000000000000000000000000
  components: sign: +, regime: 0, exponent: 1, significand: 1
  type:       posit< 32, 2, uint32_t>
```

The Priest-based `dd_cascade` shows the same dynamic:

```text
posit32> type dd_cascade
Active type: dd_cascade (double-double Priest)
dd_cascade> y = phi
1.618033988749894848204586834365637e+00
dd_cascade> ysqr = phi * phi
2.618033988749895023991527441632691e+00
dd_cascade> vars
  dd_cascade  y          = 1.618033988749894848204586834365637e+00
  dd_cascade  ysqr       = 2.618033988749895023991527441632691e+00

dd_cascade> show y
  value:      1.618033988749894848204586834365637e+00
  color:      dd_cascade[ high: 1.61803, low: -5.43212e-17 ]
  components: double-double Priest: 1.61803
  type:       double-double Priest
dd_cascade> show ysqr
  value:      2.618033988749895023991527441632691e+00
  color:      dd_cascade[ high: 2.61803, low: 1.21466e-16 ]
  components: double-double Priest: 2.61803
  type:       double-double Priest
dd_cascade> show ysqr - y
  value:      1.000000000000000000000000000000000e+00
  color:      dd_cascade[ high: 1, low: 0 ]
  components: double-double Priest: 1
  type:       double-double Priest
```

---

## Example 4: Dynamic Range Comparison Across 16-bit Types

The `range` command reveals how different 16-bit types trade precision for
range:

```text
fp16> range
fp16 (IEEE-754 binary16)
[ -6.5504e+04 ... -5.9605e-08  0  5.9605e-08 ... 6.5504e+04 ]

bfloat16> range
bfloat16
[ -3.4e+38 ... -1.2e-38  0  1.2e-38 ... 3.4e+38 ]

posit16> range
posit< 16, 2, uint16_t>
[ -7.2058e+16 ... -1.3878e-17  0  1.3878e-17 ... 7.2058e+16 ]

lns16> range
lns< 16, 8, uint16_t, Saturating>
[ -18396865112328554496 ... -5.436e-20  0  5.436e-20 ... 18396865112328554496 ]
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
  epsilon:        1.25e-01
  minpos:         1.95e-03
  maxpos:         4.16e+02

posit16> precision
  type:           posit< 16, 2, uint16_t>
  binary digits:  11
  decimal digits: 3.3
  epsilon:        4.8828e-04
  minpos:         1.3878e-17
  maxpos:         7.2058e+16

fp32> precision
  type:           fp32 (IEEE-754 binary32)
  binary digits:  23
  decimal digits: 6.9
  epsilon:        1.19209290e-07
  minpos:         1.40129846e-45
  maxpos:         3.40282347e+38

posit32> precision
  type:           posit< 32, 2, uint32_t>
  binary digits:  27
  decimal digits: 8.1
  epsilon:        7.450580597e-09
  minpos:         7.523163845e-37
  maxpos:         1.329227996e+36
```

At 32 bits, posit delivers 27 binary digits near 1.0 compared to IEEE
float's 23 -- a 16x smaller epsilon. This means posit32 can resolve
perturbations near 1.0 that fp32 cannot (see Example 1).

---

## Example 6: Catastrophic Cancellation

Subtracting nearly equal quantities destroys significant digits. The
expression `(1 + 1e-8) - 1` should yield 1e-8 but exercises catastrophic
cancellation:

```text
fp32> show (1 + 1e-8) - 1
  value:      0.00000000e+00
  binary:     0b0.00000000.00000000000000000000000
  components: sign: +, zero
  type:       fp32 (IEEE-754 binary32)
```

IEEE single loses the 1e-8 term entirely -- it's below the ULP at 1.0
(which is ~1.2e-7). posit32 preserves the term:

```text
posit32> show (1 + 1e-8) - 1
  value:      7.450580597e-09
  binary:     0b0.00000001.01.000000000000000000000
  components: sign: +, regime: -7, exponent: 2, significand: 1
  type:       posit< 32, 2, uint32_t>
```

The posit result (7.45e-9) is the nearest posit representable to 1e-8.
Double-double recovers nearly full accuracy:

```text
dd> show (1 + 1e-8) - 1
  value:      1.0000000000000000209225608301285e-08
  binary:     0b0.01111100100.0101011110011000111011100010001100001000110000111010|000...0
  components: double-double: 1e-08
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
  result:    1.414213561e+00
  reference: 1.414213562373095048801688724209698...e+00
  rounded:   1.414213561e+00
  neighbor:  1.414213568e+00
  faithful:  yes

fp32> faithful sqrt(2)
  result:    1.41421354e+00
  reference: 1.414213562373095048801688724209698...e+00
  rounded:   1.41421354e+00
  neighbor:  1.41421366e+00
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
x                                      result               double ref      ULP error
-------------------------------------------------------------------------------------
0                             0.000000000e+00                        0           0.00
0.628318                      5.877848230e-01      0.58778482293254253           0.04
1.256636                      9.510561898e-01      0.95105618829288086           0.44
1.884954                      9.510570094e-01      0.95105700829655349           0.31
2.513272                      5.877869688e-01      0.58778696973054001           0.85
3.14159                       2.654390983e-06     2.65358979335273e-06       20261.95
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
| Integer | int8, int16, int32, int64 |
| Fixed-point | fixpnt16, fixpnt32 |
| Native IEEE | float, double |
| Classic float | fp16, fp32, fp64, fp128 |
| Google Brain Float | bfloat16 |
| FP8 (Deep Learning) | fp8e2m5, fp8e3m4, fp8e4m3, fp8e5m2 |
| Logarithmic | lns8, lns16, lns32 |
| Posit | posit8, posit16, posit32, posit64 |
| Decimal float | decimal32, decimal64 |
| Hexadecimal float | hfloat32, hfloat64 |
| Rational | rational8, rational16, rational32 |
| Multi-component | dd, dd_cascade, td_cascade, qd, qd_cascade |
