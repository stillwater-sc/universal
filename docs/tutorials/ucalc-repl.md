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

## CLI Flags

| Flag | Description |
|------|-------------|
| `--json` | JSON output for all commands |
| `--csv` | CSV output for tabular commands |
| `--quiet` | Value only, no decoration |
| `--mcp` | Run as MCP server (JSON-RPC over stdio) |
| `-t <type>` | Set active type from command line |
| `-f <file>` | Execute a script file (batch mode) |

## Commands Reference

### Expression Features

Expressions support standard arithmetic (`+`, `-`, `*`, `/`, `^`), parentheses,
variables (`x = expr`), constants (`pi`, `e`, `phi`, `ln2`, `ln10`, `sqrt2`,
`sqrt3`, `sqrt5`), and functions (`sqrt`, `abs`, `log`, `exp`, `sin`, `cos`,
`tan`, `asin`, `acos`, `atan`, `pow`).

Constants are sourced at quad-double precision (~64 decimal digits) and
converted to the active type at its native precision.

### Type Inspection

| Command | Description |
|---------|-------------|
| `type <name>` | Set the active arithmetic type |
| `types` | List all 42+ available types |
| `show <expr>` | Value + decimal + binary + components |
| `compare <expr>` | Evaluate across all types in a table |
| `bits <expr>` | Raw bit pattern |
| `range` | Symmetry range: [maxneg ... minneg] 0 [minpos ... maxpos] |
| `precision` | Binary/decimal digits, epsilon, minpos, maxpos |
| `ulp <value>` | Unit in the last place at a given value |
| `sweep <expr> for <var> in [a, b, n]` | Error analysis across a range |
| `faithful <expr>` | Check faithful rounding vs qd reference |
| `oracle <type> <expr>` | Canonical result with rounding verification |
| `increment <expr>` | Show value and next representable value |
| `decrement <expr>` | Show value and previous representable value |
| `color on/off` | Toggle ANSI color-coded bit fields |
| `vars` | List defined variables |

### Numerical Forensics

| Command | Description |
|---------|-------------|
| `steps <expr>` | Step-by-step arithmetic (align, add, normalize, round) |
| `trace <expr>` | Show each operation with ULP error and rounding direction |
| `cancel <expr>` | Detect catastrophic cancellation in subtractions |
| `audit <expr>` | Rounding audit trail with signed ULP drift and ties-to-even detection |
| `diverge <expr> <t1> <t2> <tol> for <var> in [a, b]` | Find where two types first disagree |
| `suggest <expr>` | Find unstable patterns and suggest rewrites |
| `rewrites` | List available numerical rewrite patterns |
| `ast <expr>` | Show expression tree structure with provenance tags |
| `numberline [lo, hi]` | ASCII visualization of representable value density |
| `heatmap` | Precision (significant bits) vs magnitude bar chart |

### Quantization Workbench

| Command | Description |
|---------|-------------|
| `quantize <fmt> [data] \| -f <file>` | Quantize data, report RMSE/QSNR/errors |
| `block <fmt> [data] \| -f <file>` | MX/NV block decomposition (scale + elements) |
| `dot [v1] [v2] [accum=<type>]` | Mixed-precision dot product with configurable accumulation |
| `clip <type> [data] \| -f <file>` | Overflow/underflow map for a distribution |

### Statistics and Verification

| Command | Description |
|---------|-------------|
| `testvec <type> <func> [a, b, n]` | Generate golden test vectors (C++/JSON/CSV) |
| `errordist <expr> for <var> in [a, b, n]` | ULP error distribution histogram |
| `stochastic <expr> N` | Simulate stochastic rounding N times |
| `histogram [lo, hi, bins]` | Representable value distribution |
| `cond [[a,b],[c,d]]` | Condition number estimation (2x2 or 3x3) |

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

## Example 2: How 0.1 Looks Across 42 Types

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
fp8e2m5                   0.00e+00  0b0.00.00000
fp8e3m4                   9.38e-02  0b0.000.0110
fp8e4m3                   1.02e-01  0b0.0011.101
fp8e5m2                    9.4e-02  0b0.01011.10
fixpnt16                0.10156250  0b00000000.00011010
fixpnt32        0.1000061035156250  0b0000000000000000.0001100110011010
lns8                       0.10511  0b0.11100.11
int8                             0  0b00000000
int16                            0  0b0000000000000000
int32                            0  0b00000000000000000000000000000000
takum8                       0.125  0b0.0.110.0.00
takum16                   0.099976  0b0.0.101.11.100110011
takum32              0.09999999963  0b0.0.101.11.1001100110011001100110011
hfloat32               0.099999964  0b0.1000000.000110011001100110011001
decimal32                0.1000000  0b0.01001.011110.00000000000000000000
rational8                      0.1  0b0000'0001 / 0b0000'1010
rational16                     0.1  0b0000'0000'0000'0001 / 0b0000'0000'0000'1010
rational32                     0.1  0b0000'0000'0000'0000'0000'0000'0000'0001 / 0b0000'0000'0000'0000'0000'0000'0000'1010

Type                            Value  Binary
--------------------------------------------------------------------------------
double            0.10000000000000001  0b0.01111111011.1001100110011001100110011001100110011001100110011010
posit64     1.0000000000000000002e-01  0b0.01.00.10011001100110011001100110011001100110011001100110011001101
fp64          9.99999999999999917e-02  0b0.01111111011.1001100110011001100110011001100110011001100110011001
lns16         0.100112047230168338396  0b0.1111100.10101110
int64                               0  0b0000000000000000000000000000000000000000000000000000000000000000
takum64        0.10000000000000000555  0b0.0.101.11.100110011001100110011001100110011001100110011001101000000
dfixpnt8_4                     0.1000  0.0000000000000000.0001000000000000
dfixpnt16_8                0.10000000  0.00000000000000000000000000000000.00010000000000000000000000000000
hfloat64         0.099999999999999992  0b0.1000000.00011001100110011001100110011001100110011001100110011001
decimal64          0.1000000000000000  0b0.01001.01111110.00000000000000000000000000000000000000000000000000

Type        Value / Binary
--------------------------------------------------------------------------------
fp128       9.99999999999999999999999999999999928e-02
            0b0.011111111111011.1001100110011001100110011001100110011001100110011001100110011001100110011001100110011001100110011001100110011001
lns32       0.09999987268604650092473917766255908645689487457275390625
            0b0.111111111111100.1010110110010110
dd          1.0000000000000000000000000000000e-01
            0b0.01111111011.1001100110011001100110011001100110011001100110011010|01100110011001100110011001100110011001100110011001101
dd_cascade  9.999999999999999999999999999999969e-02
            0b0.01111111011.1001100110011001100110011001100110011001100110011010|01100110011001100110011001100110011001100110011001101
td_cascade  1.0000000000000000000000000000000000000000000000002e-01
            0b0.01111111011.1001100110011001100110011001100110011001100110011010|00000000000000000000000000000000000000000000000000000|00000000000000000000000000000000000000000000000000000
qd          1.000000000000000000000000000000000000000000000000000000000000000e-01
            0b0.01111111011.1001100110011001100110011001100110011001100110011010|00000000000000000000000000000000000000000000000000000|00000000000000000000000000000000000000000000000000000|00000000000000000000000000000000000000000000000000000
qd_cascade  9.99999999999999999999999999999999999999999999999999999999999999991e-02
            0b0.01111111011.1001100110011001100110011001100110011001100110011010|00000000000000000000000000000000000000000000000000000|00000000000000000000000000000000000000000000000000000|00000000000000000000000000000000000000000000000000000
```

Notice that `decimal32` and `rational32` represent 0.1 exactly -- decimal
uses base-10 encoding and rational stores the fraction 1/10 directly. Every
binary type introduces rounding error, but the magnitude differs by orders
of magnitude across types.

Now try:
```text
double> compare 0.1
```

The results are different as we are giving the number systems a double precision 
floating-point number, `0.1`, which approximates `1/10`, and that approximation 
propagates through all the number systems.

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

## Example 9: Exact Decimal Arithmetic for Financial Calculations

Binary floating-point cannot represent 0.1, 0.01, or most decimal
fractions exactly. In financial software this causes accumulation errors
that violate accounting identities. Decimal fixed-point (`dfixpnt`)
uses BCD encoding and carries every decimal digit without rounding:

```text
double> show 0.1 + 0.2 - 0.3
  value:      5.5511151231257827e-17
  binary:     0b0.01111001001.0000000000000000000000000000000000000000000000000000
  components: sign: +, scale: -54, significand: 1
  type:       double (IEEE-754 binary64)

double> type dfixpnt16_8
Active type: dfixpnt16_8 (dfixpnt< 16,   8, BCD,     Modulo, uint8_t>)
dfixpnt16_8> show 0.1 + 0.2 - 0.3
  value:      0.00000000
  binary:     0.00000000000000000000000000000000.00000000000000000000000000000000
  components: dfixpnt< 16,   8, BCD,     Modulo, uint8_t>: 0
  type:       dfixpnt< 16,   8, BCD,     Modulo, uint8_t>
```

Double produces a non-zero residual (~5.55e-17) because 0.1 and 0.2
are rounded on entry. `dfixpnt16_8` yields exactly zero.

This matters when totals must balance to the penny. Consider an invoice
with three items at $19.99, two at $5.99, and one at $1.50, plus
7.25% sales tax:

```text
dfixpnt16_8> tax = 0.0725
0.07250000
dfixpnt16_8> subtotal = 19.99 * 3 + 5.99 * 2 + 1.50
73.45000000
dfixpnt16_8> show subtotal + subtotal * tax
  value:      78.77512500
  binary:     0.00000000000000000000000001111000.01110111010100010010010100000000
  components: dfixpnt< 16,   8, BCD,     Modulo, uint8_t>: 78.7751
  type:       dfixpnt< 16,   8, BCD,     Modulo, uint8_t>
```

The subtotal is exactly $73.45, tax is exactly $5.325125, and the grand
total is exactly $78.775125. The same calculation in double:

```text
double> tax = 0.0725
0.072499999999999995
double> subtotal = 19.99 * 3 + 5.99 * 2 + 1.50
73.450000000000003
double> show subtotal + subtotal * tax
  value:      78.775125000000003
  binary:     0b0.10000000101.0011101100011001101110100101111000110101001111111000
  components: sign: +, scale: 6, significand: 1.230861328125
  type:       double (IEEE-754 binary64)
```

Double's subtotal is already 73.450000000000003 -- off by 3e-15. These
errors are invisible in a single calculation but accumulate across
thousands of line items in a ledger, eventually causing reconciliation
failures. Decimal fixed-point eliminates this class of error entirely.

---

## Example 10: Takum's Uniform Precision Across the Dynamic Range

Posit arithmetic concentrates precision near 1.0 by using a variable-length
regime field: values close to 1.0 get many fraction bits, but extreme
values consume most bits on the regime, leaving few for the significand.
Takum (Hunhold, 2024) replaces the variable-length regime with a bounded
characteristic field, giving a more uniform precision distribution and
a dramatically wider dynamic range.

At 32 bits, both types deliver identical precision near 1.0:

```text
takum32> precision
  type:           takum< 32, 3, uint32_t>
  binary digits:  27
  decimal digits: 8.1
  epsilon:        7.450580597e-09
  minpos:         1.727235358e-77
  maxpos:         5.789601701e+76

posit32> precision
  type:           posit< 32, 2, uint32_t>
  binary digits:  27
  decimal digits: 8.1
  epsilon:        7.450580597e-09
  minpos:         7.523163845e-37
  maxpos:         1.329227996e+36
```

Same epsilon, same 27 binary digits at 1.0. But takum32 spans 10^77
while posit32 reaches only 10^36 -- over twice the dynamic range in
decades.

The difference becomes dramatic away from 1.0. Compare the ULP at
increasing scales:

| Scale | takum32 ULP | posit32 ULP | Relative ULP (takum) | Relative ULP (posit) |
|-------|------------|------------|---------------------|---------------------|
| 1     | 7.45e-9    | 7.45e-9    | 7.45e-9             | 7.45e-9             |
| 1e5   | 5.96e-3    | 5.96e-3    | 5.96e-8             | 5.96e-8             |
| 1e10  | 1,192      | 9,537      | 1.19e-7             | 9.54e-7             |
| 1e15  | 1.19e8     | 1.53e10    | 1.19e-7             | 1.53e-5             |
| 1e20  | 2.38e13    | 2.44e16    | 2.38e-7             | 2.44e-4             |
| 1e30  | 2.38e23    | 6.44e28    | 2.38e-7             | 6.44e-2             |

Takum's relative precision stays nearly constant (~2e-7) across 30
decades of scale. Posit's degrades from 7.45e-9 at 1.0 to 0.064 at
1e30 -- a factor of 8.6 million. At 1e30, posit32 has barely one
significant digit left.

This is visible in the representations themselves:

```text
takum32> show 1e20
  value:      1.00000002e+20
  binary:     0b0.1.110.000011.010110101111000111011
  components: ... Characteristic :    66 Scale :    66
  type:       takum< 32, 3, uint32_t>

posit32> show 1e20
  value:      1.000159405e+20
  binary:     0b0.111111111111111110.10.01011011000
  components: sign: +, regime: 16, exponent: 4, significand: 1.35546875
  type:       posit< 32, 2, uint32_t>
```

Takum32 represents 1e20 to 8 significant digits (1.00000002e+20).
Posit32 manages only 4 (1.000159405e+20 -- off by 1.6e16). The
posit's regime field has expanded to 18 bits, leaving only 11 for
exponent and significand. Takum's characteristic field stays bounded,
preserving fraction bits at every scale.

A sweep of sqrt(x) across a wide range confirms the pattern:

```text
takum16> sweep sqrt(x) for x in [0.001, 1e12, 8]
x                                      result               double ref      ULP error
-------------------------------------------------------------------------------------
0.001                                0.031616     0.031622776601683791           0.21
1.4285714e+11                      3.7888e+05       377964.47300922842           0.31
4.2857143e+11                      6.5536e+05       654653.67070797761           0.14
8.5714286e+11                       9.257e+05       925820.09977255156           0.03
1e+12                              9.9942e+05       1000000.0000000001           0.29

posit16> sweep sqrt(x) for x in [0.001, 1e12, 8]
x                                      result               double ref      ULP error
-------------------------------------------------------------------------------------
0.001                              3.1616e-02     0.031622776601683791           0.43
1.4285714e+11                      3.7069e+05       377964.47300922842           2.46
4.2857143e+11                      6.4307e+05       654653.67070797761           2.26
8.5714286e+11                      9.0931e+05       925820.09977255156           4.56
1e+12                              9.7894e+05       1000000.0000000001          10.78
```

At 1e12, posit16's sqrt has 10.78 ULP error vs takum16's 0.29 -- a 37x
improvement. Takum maintains sub-ULP accuracy across the entire range
because it never runs out of fraction bits.

---

## Available Types

ucalc registers 42 types spanning the major number system families:

| Family | Types |
|--------|-------|
| Integer | int8, int16, int32, int64 |
| Fixed-point | fixpnt16, fixpnt32 |
| Decimal fixed-point | dfixpnt8_4, dfixpnt16_8 |
| Native IEEE | float, double |
| Classic float | fp16, fp32, fp64, fp128 |
| Google Brain Float | bfloat16 |
| FP8 (Deep Learning) | fp8e2m5, fp8e3m4, fp8e4m3, fp8e5m2 |
| Logarithmic | lns8, lns16, lns32 |
| Posit | posit8, posit16, posit32, posit64 |
| Takum | takum8, takum16, takum32, takum64 |
| Decimal float | decimal32, decimal64 |
| Hexadecimal float | hfloat32, hfloat64 |
| Rational | rational8, rational16, rational32 |
| Multi-component | dd, dd_cascade, td_cascade, qd, qd_cascade |

---

## Example 11: Tracing Error Propagation

The `trace` command shows each arithmetic operation with its ULP error and
rounding direction, using quad-double as the reference.

```
float> trace 1/3 + 1/3 + 1/3
  step 1: 1 / 3
          result:    0.333333343
          reference: 3.333333333...e-01
          ROUNDED UP  0.50 ULP
  step 3: 0.333333343 + 0.333333343
          = 0.666666687  (exact)
  step 5: 0.666666687 + 0.333333343
          result:    1
          reference: 1.0000000298...
          ROUNDED DOWN  0.25 ULP
  result: 1
  reference precision: quad-double
```

---

## Example 12: Rounding Audit with Cumulative Drift

The `audit` command tracks signed ULP error and detects ties-to-even rounding.

```
float> audit 1/3 + 1/3 + 1/3
  step 1: TIES-TO-EVEN  ulp: +0.50  cumulative: +0.50
  step 2: TIES-TO-EVEN  ulp: +0.50  cumulative: +1.00
  step 3: exact
  step 4: TIES-TO-EVEN  ulp: +0.50  cumulative: +1.50
  step 5: ROUNDED DOWN  ulp: -0.25  cumulative: +1.25
  --------
  rounding events:  4 of 5 operations
  max |ulp| error:  0.50
  cumulative drift: +1.25 ULPs
```

---

## Example 13: Quantization Quality for ML Weights

Compare quantization quality across formats using QSNR (dB):

```bash
for fmt in fp8e4m3 fp8e5m2 bfloat16 posit8 fp16; do
  echo -n "$fmt: "; ucalc --quiet "quantize $fmt -f weights.csv"
done
```

```
fp8e4m3:  0.0131171 31.6dB 10000
fp8e5m2:  0.0267517 25.4dB 10000
bfloat16: 0.00165137 49.6dB 10000
posit8:   0.0131249 31.6dB 10000
fp16:     0.000103695 73.7dB 10000
```

---

## Example 14: Precision Heatmap

```
posit16> heatmap
  magnitude     sig_bits  bar
  1e-12              2.0  ######
  1e-8               6.0  ####################
  1e-4               9.0  ##############################
  1e+0              11.0  ####################################
  1e+4               9.0  ##############################
  1e+8               6.0  ####################
  1e+12              3.0  ##########

  tapered precision: peaks near 1, falls off at extremes
```

---

## Example 15: Finding Type Divergence Points

```
ucalc> diverge sin(x) posit32 float 1ulp for x in [0, 6.28]
  first divergence at x = 0.003198...
  posit32       3.198280232e-03
  float         0.00319828046
  abs diff:     2.3283064e-10
  ulp diff:     4.885 ULPs
```

---

## Step-by-Step Arithmetic Visualization

The `steps` command is one of ucalc's most powerful educational features.
It decomposes every arithmetic operation into the hardware-level stages
specific to the active number system, revealing the fundamental differences
in how each number system performs arithmetic.

Different number systems have radically different internal mechanics:
binary floating-point aligns exponents and normalizes; posit decodes a
variable-length regime field; logarithmic types convert multiplication
into addition; fixed-point works with scaled integers; double-double
uses error-free transformations to extend precision without wider hardware.
The `steps` command makes all of this visible.

### IEEE Binary Floating-Point (fp16, fp32, fp64, fp128, bfloat16)

Binary floating-point arithmetic follows the textbook pipeline: decompose
operands into sign/exponent/significand, align exponents by shifting the
smaller operand, perform integer arithmetic on significands, normalize
the result, and round to fit.

```
fp16> steps 1.5 + 0.375
  1.5000e+00 + 3.7500e-01 = 1.8750e+00
    1. Decompose operands
       A = +1.1000000000 * 2^0
                  B = +1.1000000000 * 2^-2
    2. Align exponents
       shift B right by 2 positions to match exponent 2^0
    3. Add significands
       1.5 + 0.375 = 1.875 (at exponent 2^0)
    4. Normalize
       already normalized (1.xxx form)
    5. Round
       round to 11 significand bits
                  result: +1.1110000000 * 2^0 = 1.875
```

Students see exactly how the alignment shift can discard low-order bits
(the source of rounding error in floating-point addition), and how
normalization can shift the result to maintain the leading-1 convention.

### Posit (posit8, posit16, posit32, posit64)

Posit arithmetic is similar to IEEE at the significand level, but the
regime field makes encoding and decoding more complex. The steps show
how the variable-length regime determines the scale, and how the
remaining bits are divided between exponent and fraction.

```
posit16> steps 1.5 * 0.375
  1.5000e+00 * 3.7500e-01 = 5.6250e-01
    1. Decode posit operands
       A: regime=10 (k=0), scale=2^0, 11 fraction bits
                  B: regime=01 (k=-1), scale=2^-2, 11 fraction bits
    2. Add exponents (multiply scales)
       2^0 * 2^-2 = 2^(0+-2) = 2^-2
    3. Multiply significands
       1.5 * 1.5 = 1.125
    4. Re-encode regime
       result scale: 2^-1 -> regime k=-1 (01), exponent=3
                  11 fraction bits available
    5. Round to fit
       round to 11 fraction bits
                  result: 0.5625
```

Notice the re-encoding step: posit must map the result scale back to a
regime encoding, and this determines how many fraction bits remain.
Values near 1.0 get short regimes (more fraction bits = more precision),
while extreme values get long regimes (fewer fraction bits = less precision).

### Fixed-Point (fixpnt16, fixpnt32)

Fixed-point arithmetic is the simplest: scale both operands to integers,
perform integer arithmetic, and check for overflow. No alignment,
normalization, or rounding needed (unless the result overflows).

```
fixpnt16> steps 1.5 + 0.375
  1.50000000 + 0.37500000 = 1.87500000
    1. Fixed-point format
       fixpnt<16,8>: 8 integer bits, 8 fraction bits
                  radix point at bit 8, resolution = 2^-8 = 0.00390625
                  overflow mode: Modulo
    2. Convert to fixed-point integers
       A = 1.5 -> 384 (x 2^8)
                  B = 0.375 -> 96 (x 2^8)
    3. Add integers
       384 + 96 = 480
    4. Check overflow
       no overflow (480 in [-32768, 32767])
    5. Convert to real value
       480 * 2^-8 = 1.875
```

The key insight: every fixed-point operation is just integer arithmetic on
the underlying encoding. The radix point never moves, so there is no
alignment step and no normalization. Overflow behavior (Modulo vs Saturating)
is the only thing to watch.

### Logarithmic Number System (lns8, lns16, lns32)

LNS stores values as a sign bit plus a fixed-point logarithm. This makes
multiplication trivial (add the log-values) but addition expensive
(requires evaluating the Gaussian logarithm). The steps reveal this
dramatic asymmetry.

```
lns16> steps 1.5 * 0.375
  1.50101... * 0.37525... = 0.56326...
    1. Decompose LNS operands
       A = +2^0.585938 = 1.50101407
                  B = +2^-1.41406 = 0.3752535174
    2. Determine sign (XOR)
       + * + = +
    3. Add log-values (multiplication is FREE in LNS!)
       log2(|A*B|) = log2(|A|) + log2(|B|)
                  = 0.585938 + -1.41406 = -0.828125
                  (just one fixed-point addition -- no multiplier hardware needed)
    4. Result
       +2^-0.828125 = 0.5632608093
```

Multiplication in LNS is a single fixed-point addition -- no multiplier
hardware needed. This is why LNS is attractive for DSP and neural network
inference where multiplication dominates.

### Double-Base Number System (dbns8, dbns16)

DBNS represents values as sign * 2^a * 3^b, using two integer exponents.
Like LNS, multiplication is trivial: just add both exponent pairs. But
DBNS provides a denser representable number grid than LNS because it
uses two bases.

```
dbns16> steps 1.5 * 0.375
  1.5 * 0.375 = 0.5625
    1. Decompose DBNS operands
       A = +2^-1 * 3^1 = 1.5
                  B = +2^-3 * 3^1 = 0.375
    2. Determine sign (XOR)
       + * + = +
    3. Add exponent pairs (multiplication is two integer additions!)
       base-2 exponent: -1 + -3 = -4
                  base-3 exponent: 1 + 1 = 2
                  (two integer adds -- no multiplier needed, like LNS but denser grid)
    4. Result
       +2^-4 * 3^2 = 0.5625
```

Two integer additions replace a full multiplication -- and because the
2^a * 3^b grid is denser than 2^a alone, DBNS can represent more values
in the same bit width than pure LNS.

### Double-Double (dd, dd_cascade, qd, qd_cascade)

Double-double extends precision by storing each value as a pair of doubles
(hi, lo) where the low word carries the bits that don't fit in the high
word. Arithmetic uses error-free transformations (`two_sum`, `two_prod`)
to capture rounding errors that ordinary double arithmetic would discard.

```
dd> steps 1e16 + 1.5
  1.000...e+16 + 1.500...e+00 = 1.000000000000000150...e+16
    1. Decompose double-double operands
       A = (hi: 10000000000000000, lo: 0)
                  B = (hi: 1.5, lo: 0)
    2. two_sum(a.hi, b.hi) -- error-free transformation
       two_sum(10000000000000000, 1.5)
                  sum = 10000000000000002
                  err = -0.5
                  (sum + err = a.hi + b.hi EXACTLY -- no rounding loss!)
    3. Accumulate error terms
       e_total = two_sum_error + a.lo + b.lo
               = -0.5 + 0 + 0 = -0.5
    4. Renormalize with two_sum
       two_sum(10000000000000002, -0.5)
                  result.hi = 10000000000000002
                  result.lo = -0.5
                  (restores non-overlapping property)
    5. Result
       dd = (hi: 10000000000000002, lo: -0.5)
                  value ~= 10000000000000002
```

The critical insight: `two_sum` reveals that adding 1e16 + 1.5 in double
produces 1e16 + 2 (off by 0.5), and the -0.5 error is captured in the
low word. Ordinary double would lose that 0.5 forever. This is how
double-double achieves ~31 decimal digits from two 53-bit doubles.

### Decimal Floating-Point (decimal32, decimal64)

Decimal floating-point stores a decimal coefficient and a power-of-10
exponent. Arithmetic aligns decimal exponents (by scaling coefficients
by powers of 10) and performs decimal integer arithmetic. No binary
rounding artifacts -- 0.1 + 0.2 = 0.3 exactly.

```
decimal32> steps 1.5 + 0.375
  1.5 + 0.375 = 1.875
    1. Decompose decimal operands
       A = +15 * 10^-1 (7 significant digits)
                  B = +375 * 10^-3
    2. Align decimal exponents
       scale A coefficient up by 10^2 -> 1500 * 10^-3
    3. Add coefficients
       1500 + 375 = 1875 (decimal integer arithmetic)
    4. Normalize coefficient
       coefficient has 4 digit(s), fits in 7
                  result: +1875 * 10^-3
    5. Result
       1.875000
```

The alignment step scales by powers of 10, not powers of 2. This is why
decimal types can represent 0.1 exactly (it's 1 * 10^-1) while binary
types cannot.

### Hexadecimal Floating-Point (hfloat32, hfloat64)

IBM System/360 hex floats use base-16 exponents and hex digit fractions.
The steps reveal hex float's distinctive "wobbling precision": because
normalization aligns to hex digit boundaries (4 bits at a time), up to
3 leading bits can be wasted, causing precision to vary depending on the
value.

```
hfloat32> steps 1.5 + 0.375
  1.5 + 0.375 = 1.875
    1. Decompose hex float operands
       A = +16^1 * 0x0.180000
                  B = +16^0 * 0x0.600000
    2. Align hex exponents
       shift B right by 1 hex digit (= 4 bits)
                  (hex float shifts by whole hex digits, not single bits)
    3. Add hex fractions
       0x0.180000 + 0x0.600000 (hex digit arithmetic)
    4. Hex-normalize (source of wobbling precision!)
       already hex-normalized (leading hex digit non-zero)
                  leading hex digit: 1 (1 significant bits)
                  wasted bits: 3 of 4 (IEEE binary normalization would waste 0)
    5. Truncate (hfloat uses truncation, never rounds up)
       truncate to 6 hex fraction digits
    6. Result
       +16^1 * 0x0.1E0000 = 1.875
```

Step 4 reveals the wobbling precision problem: the leading hex digit is 1
(binary 0001), which wastes 3 of 4 bits. A value with leading hex digit 8
(binary 1000) would waste 0 bits. This means hfloat's effective precision
varies by up to 3 bits depending on the value -- a fundamental limitation
that IEEE binary normalization eliminates.

---

## Example 16: Rewrite Suggestions with Verification

The `suggest` command identifies numerically unstable patterns in expressions
and proposes stable alternatives, verified with actual error comparison.

```
float> a = 1000001; b = 1000000
float> suggest sqrt(a) - sqrt(b)
  pattern:     Square root difference (sqrt_diff)
  matched:     (sqrt(a) - sqrt(b))
  alternative: ((a - b) / (sqrt(a) + sqrt(b)))
  condition:   a, b exact inputs, a ~= b > 0
  original:    0.00048828125  (rel error: 2.3437e-02)
  rewritten:   0.000499999849  (rel error: 5.1749e-08)
  VERIFIED: 452905.6x better
```

The `rewrites` command lists all 7 available patterns:

```
ucalc> rewrites
  1. Square root difference (sqrt_diff)
     sqrt(a) - sqrt(b) -> (a - b) / (sqrt(a) + sqrt(b))
  2. Quadratic formula (unstable root)
     (-b + sqrt(b^2 - 4*a*c)) / (2*a) -> 2*c / (-b - sqrt(b^2 - 4*a*c))
  3. Logarithm near 1: log(1 + x) -> log1p(x)
  4. Exponential minus 1: exp(x) - 1 -> expm1(x)
  5. One minus cosine: 1 - cos(x) -> 2 * sin(x/2)^2
  6. Sine difference: sin(a) - sin(b) -> product-to-sum
  7. Cosine deviation ratio
```

---

## Example 17: Expression Tree with Provenance

The `ast` command shows the expression tree structure with provenance tags
indicating which values are exact inputs vs computed intermediates.

```
ucalc> ast (-b + sqrt(b^2 - 4*a*c)) / (2*a)
  `-- op:/ [computed]
      |-- op:+ [computed]
      |   |-- unary:negate [computed]
      |   |   `-- var:b [exact]
      |   `-- fn:sqrt [computed]
      |       `-- op:- [computed]
      |           |-- op:^ [computed]
      |           |   |-- var:b [exact]
      |           |   `-- 2 [exact]
      |           `-- op:* [computed]
      |               |-- op:* [computed]
      |               |   |-- 4 [exact]
      |               |   `-- var:a [exact]
      |               `-- var:c [exact]
      `-- op:* [computed]
          |-- 2 [exact]
          `-- var:a [exact]
```

---

## Example 18: Oracle -- Canonical Type Results

The `oracle` command gives the authoritative result for any expression in any
type, with rounding verification against a quad-double reference.

```
ucalc> oracle posit32 sin(0.1)
  type:       posit< 32, 2, uint32_t>
  expression: sin(0.1)
  value:      9.983341675e-02
  binary:     0b0.01.00.100110001110101011101100110
  reference:  9.983341664682815...e-02
  rounding:   correctly rounded (nearest)

ucalc> oracle decimal32 0.1 + 0.2
  type:       dfloat<  7,   6, BID, uint32_t>
  value:      0.3
  reference:  3.000000000...e-01
  rounding:   correctly rounded (nearest)
```

---

## Example 19: Stochastic Rounding Simulation

The `stochastic` command simulates stochastic rounding over N trials to
understand rounding bias.

```
bfloat16> stochastic 0.1 + 0.2 10000
  unique results: 2
  0.2988                  4024 (40.2%)
  0.3008                  5976 (59.8%)
  mean:  0.2999953125
  exact: 3.000000000...e-01
  bias:  -4.6875e-06
```

The near-zero bias confirms stochastic rounding is unbiased -- the mean
converges to the exact value over many trials.

---

## Example 20: Error Distribution Analysis

The `errordist` command evaluates a function at many points and histograms
the ULP error distribution.

```
posit32> errordist sin(x) for x in [0, 6.28, 1000]
  ulp_error        count  bar
  0                    1
  (0, 0.5]           391  ########################################
  (0.5, 1]           189  ###################
  (1, 2]             158  ################
  (2, 4]             118  ############
  (4, 8]              68  ######
  (8, +)              75  #######

  max ULP:    176.29 at x = 3.1431431
  mean ULP:   2.97
  faithful:   58.1% (581/1000)
```

---

## Example 21: Condition Number Estimation

The `cond` command estimates the condition number of small matrices to
predict precision loss.

```
float> cond [[1, 2], [1.0001, 2]]
  condition (1-norm): 59992.04
  determinant:        -0.00020003319
  WARNING: ill-conditioned
  type precision:     ~6.9 decimal digits
  digits lost:        ~4.8
  effective precision: ~2.1 decimal digits
```

---

## Example 22: Test Vector Generation

The `testvec` command generates golden reference vectors for regression tests,
directly pasteable into C++ test code.

```
ucalc> testvec posit16 sin [0, 3.14159, 5]
// Golden reference vectors for sin(x) in posit< 16, 2, uint16_t>
// Generated by ucalc testvec
struct TestVector { double input; double expected; };
constexpr TestVector sin_posit16[] = {
    { 0, 0 },  // 0.0000e+00
    { 0.785397, 0.707031 },  // 7.0703e-01
    { 1.57079, 1 },  // 1.0000e+00
    { 2.35619, 0.707031 },  // 7.0703e-01
    { 3.14159, -8.8811e-06 }  // -8.8811e-06
};
```

---

## MCP Server: ucalc as an AI Agent Tool

The Model Context Protocol (MCP) allows AI assistants to call external
tools directly, without the user copy-pasting output back and forth.
ucalc includes a built-in MCP server that exposes its full command set
as structured tools, making it a compute oracle that AI agents can
query programmatically.

### Why an MCP Server for Mixed-Precision Arithmetic?

AI assistants are increasingly used to analyze numerical algorithms,
debug precision issues, and recommend number types for specific
workloads. But AI models have no native ability to perform exact
arithmetic in posit, lns, dbns, or any of the 42+ types that ucalc
supports. Without a tool like ucalc, an AI assistant must guess at
numerical behavior -- it cannot verify whether an expression suffers
from catastrophic cancellation in bfloat16, or whether posit32
produces a faithfully rounded result for a given function.

The MCP server closes this gap. An AI assistant connected to ucalc can:

- **Evaluate expressions** in any number type and inspect the exact result
- **Compare representations** across all 42+ types to find the best fit
- **Trace error propagation** through compound expressions
- **Detect cancellation** and suggest stable rewrites
- **Generate test vectors** for regression suites
- **Verify rounding** against quad-double references
- **Visualize precision** with heatmaps and numberlines

All of this happens through structured JSON-RPC calls -- no parsing of
terminal output, no prompt engineering to extract numbers from text.

### Architecture

The MCP server is implemented in a single header (`mcp_server.hpp`,
~270 lines) with zero external dependencies, consistent with the
Universal library's design principle of no external dependencies.
It implements JSON-RPC 2.0 over stdio with Content-Length framing:

```
AI Assistant                     ucalc --mcp
    |                                |
    |-- Content-Length: N\r\n\r\n -->|
    |   {"method":"initialize",...}  |
    |                                |
    |<-- Content-Length: M\r\n\r\n --|
    |   {"result":{"capabilities"}}  |
    |                                |
    |-- tools/list ----------------->|
    |<-- 17 tools with JSON schemas -|
    |                                |
    |-- tools/call: ucalc.oracle --->|
    |   {"type":"posit32",           |
    |    "expression":"sin(0.1)"}    |
    |<-- result with value, binary,  |
    |    rounding verification ------|
```

### Starting the MCP Server

```bash
ucalc --mcp
```

The server reads JSON-RPC messages from stdin and writes responses to
stdout. It is designed to be launched as a subprocess by an AI assistant
framework (Claude Desktop, VS Code extensions, or any MCP-compatible client).

### Available Tools

The MCP server exposes 17 tools:

| Tool | Description |
|------|-------------|
| `ucalc.eval` | Evaluate an expression in a specified type |
| `ucalc.show` | Value with binary decomposition and components |
| `ucalc.compare` | Evaluate across all types in a table |
| `ucalc.types` | List all available number types |
| `ucalc.precision` | Type precision, epsilon, range |
| `ucalc.range` | Dynamic range of a type |
| `ucalc.ulp` | Unit in the last place at a value |
| `ucalc.trace` | Error propagation through an expression |
| `ucalc.cancel` | Catastrophic cancellation detection |
| `ucalc.audit` | Rounding audit with cumulative drift |
| `ucalc.quantize` | Quantize data and report RMSE/QSNR |
| `ucalc.suggest` | Find unstable patterns and suggest rewrites |
| `ucalc.oracle` | Canonical result with rounding verification |
| `ucalc.faithful` | Faithful rounding check |
| `ucalc.steps` | Step-by-step arithmetic visualization |
| `ucalc.heatmap` | Precision vs magnitude heatmap |
| `ucalc.rewrites` | List numerical rewrite patterns |

Each tool has a JSON schema describing its parameters, so AI assistants
can discover and call tools without prior knowledge of the ucalc command
syntax.

### Example MCP Interaction

An AI assistant investigating precision loss in a neural network might
make these calls:

```json
{"method": "tools/call", "params": {"name": "ucalc.compare",
  "arguments": {"expression": "1/3 + 1/3 + 1/3"}}}
```

The response contains the comparison table across all types, showing
which types evaluate 1/3 + 1/3 + 1/3 to exactly 1.0 and which don't.

```json
{"method": "tools/call", "params": {"name": "ucalc.oracle",
  "arguments": {"type": "bfloat16", "expression": "exp(-0.5)"}}}
```

Returns the exact bfloat16 result with rounding verification, letting
the assistant report whether bfloat16 is faithfully rounded for this
activation function input.

### Security

The MCP server sanitizes all tool arguments to prevent command injection.
Arguments containing semicolons, newlines, or carriage returns are
rejected, ensuring that MCP tool calls cannot execute arbitrary REPL
commands beyond their declared schema.
