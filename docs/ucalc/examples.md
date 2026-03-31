# ucalc Worked Examples

These examples demonstrate ucalc's capabilities across precision analysis,
type comparison, error forensics, quantization, and verification. Each
example is self-contained and can be run directly in the ucalc REPL.

For command reference, see the [main ucalc page](README.md).
For step-by-step arithmetic visualization, see the [dedicated guide](step-by-step.md).
For AI agent integration, see the [MCP server guide](mcp-server.md).

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
