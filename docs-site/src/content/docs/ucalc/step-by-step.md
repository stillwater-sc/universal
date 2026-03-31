---
title: "Step-by-Step Arithmetic Visualization"
description: "How each number system family performs arithmetic internally"
---


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

---

## IEEE Binary Floating-Point (fp16, fp32, fp64, fp128, bfloat16)

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

---

## Posit (posit8, posit16, posit32, posit64)

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

---

## Fixed-Point (fixpnt16, fixpnt32)

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

---

## Logarithmic Number System (lns8, lns16, lns32)

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

---

## Double-Base Number System (dbns8, dbns16)

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

---

## Double-Double (dd, dd_cascade, qd, qd_cascade)

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

---

## Decimal Floating-Point (decimal32, decimal64)

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

---

## Hexadecimal Floating-Point (hfloat32, hfloat64)

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
