# 8-4 Compression Algorithm

## Expansion Algebra Education Summary

### Core Principle: Error-Free Transformations

  Expansion algebra is built on error-free transformations like two_sum and fast_two_sum:

```cpp
  fast_two_sum(a, b, sum, error)
  // sum = a + b (rounded)
  // error = (a + b) - sum (the bits that were lost to rounding)
  // Guarantee: a + b = sum + error EXACTLY
```

### The 8→4 Compression Algorithm

  The basic algorithm has two phases:

   - Phase 1: Bottom-Up Accumulation

  Starting from the least significant components, accumulate upward using error-free operations:

```cpp
  fast_two_sum(r6, r7, s0, r7);  // s0 = r6+r7, error stored back in r7
  fast_two_sum(r5, s0, s0, r6);  // s0 = r5+s0, error stored back in r6
  fast_two_sum(r4, s0, s0, r5);  // Continue pattern...
  fast_two_sum(r3, s0, s0, r4);
  fast_two_sum(r2, s0, s0, r3);
  fast_two_sum(r1, s0, s0, r2);
  fast_two_sum(r0, s0, r0, r1);  // Final: r0 = r0+s0, error in r1
```

  Why bottom-up? Because smaller values are added to larger ones, minimizing lost precision.

   - Phase 2: Conditional Extraction

  Extract the 4 most significant non-overlapping components using conditional logic to handle zeros:

```cpp
  fast_two_sum(r0, r1, s0, s1);
  if (s1 != 0.0) {
      fast_two_sum(s1, r2, s1, s2);
      if (s2 != 0.0) {
          // ... continue extracting non-zero components
      }
  }
```

  Why conditional? Components might cancel to zero. The algorithm dynamically shifts remaining precision into available slots.

### What Was Wrong

The original bug (line 195 of qd_cascade_impl.hpp):

```cpp
compressed[3] = result[3] + result[4] + result[5] + result[6] + result[7];  // WRONG!
```

This naive addition discards rounding errors from intermediate sums, destroying the 212-bit precision of quad-double arithmetic.

### The Fix

We added compress_8to4() to floatcascade.hpp that implements the proven QD library algorithm, then used it in qd_cascade::operator+= and operator-=:

```cpp
  auto result = expansion_ops::add_cascades(cascade, neg_rhs);  // 8 components
  cascade = expansion_ops::compress_8to4(result);  // Properly compress to 4
```

## Follow-up (universal#1317, 2026-08-15): compression was necessary but not sufficient

`compress_8to4()` fixed the naive `result[3] + result[4] + ... + result[7]` summation described
above, but the quad-double addition path was still losing its fourth component on roughly one
addition in eight. The reason is a precondition, not an algorithm:

`compress_8to4()` is the QD library's `renorm`, and every step of it is a `fast_two_sum` chain.
`fast_two_sum(a, b)` is only error-free when `|a| >= |b|`, and the QD algorithm as a whole assumes
its input expansion is **non-overlapping** - each component below the ulp of the one before it.

`add_cascades()` did not produce such an expansion. It merges the 8 input components by magnitude,
accumulates them smallest-to-largest with `two_sum`, and stores the running sum plus the collected
errors. The *value* of that expansion is exact, but its components overlap badly - consecutive
terms can be within a factor of two of each other:

```
e[0] = 1.743e+00
e[1] = 1.110e-16      <- e[2] is only 3x smaller, not 2^53x smaller
e[2] = 3.385e-17
e[3] = 1.233e-32
```

Handed that, `renorm`'s conditional extraction runs out of places to put the tail and returns a
result whose fourth component is exactly zero: a relative error of 2^-160 in a format that carries
2^-212, i.e. 48 correct digits instead of 63.

### The fix

`expansion_ops::compress_expansion<M>()` implements Shewchuk's COMPRESS (Robust Adaptive
Floating-Point Geometric Predicates, Figure 22): a downward sweep that carries each rounding
remainder into the next component, then an upward sweep that leaves the leading components
correctly rounded. It is value-preserving, so it changes nothing except the *shape* of the
expansion - which is exactly what the precondition is about. `add_cascades(fc<4>, fc<4>)` now
returns a compressed expansion.

Verified against an exact oracle (Python `decimal` at 140 digits, sharing no code with the
library) over 200 random full-precision operand pairs:

| operation | before | after | classic `qd` |
|---|---|---|---|
| add (4-component operands) | 1.7e-49 | exact | 6.2e-65 |
| sqrt | 1.3e-49 | 6.1e-64 | 5.5e-65 |
| exp | 4.8e-51 | 4.0e-65 | 4.0e-65 |
| log | 1.3e-50 | 2.3e-63 | 2.3e-63 |

### Why only `floatcascade<4>`

The `<2>` and `<3>` overloads share the same accumulation and were measured over 5000 random
full-precision additions each: both stay within 0.49 ulp of their own format with and without
compression. Fewer merged terms compressed into fewer output components leaves enough slack that
the overlap never reaches a surviving component. Compressing them anyway would have doubled the
cost of `dd_cascade` addition for no accuracy gain, so the two overloads carry a comment saying
so, and `static/highprecision/td_cascade/arithmetic/addition_oracle.cpp` pins the triple-double
case in case that ever stops being true.


## Follow-up (universal#1322, 2026-08-16): multiplication

With the addition fixed, multiplication became the remaining gap: about two decimal digits behind
`qd` (1.7e-63 vs 1.6e-65 worst case on full-width operands) and 8x slower.

The old implementation computed the 16 partial products with `two_prod`, built a 32-term expansion,
**bubble sorted it** - up to 496 compare-and-swap steps - and folded it into 4 components with a
carry loop that discarded the error term of every sub-ulp carry. Both problems came from the same
place: sorting throws away the structure that makes the reference algorithm work.

`multiply_cascades(floatcascade<4>, floatcascade<4>)` is now a transliteration of the classic
qd_mul (`qd::accurate_multiplication`), written against the volatile-hardened error-free
transformations. It needs no sorting at all: `a[i]*b[j]` contributes at order eps^(i+j), so the
products are generated in order of decreasing significance by construction, and the groups are
accumulated with `three_sum`/`two_sum` chains that keep every error term. Terms below eps^4 cannot
move a 4-component result and are summed in plain arithmetic, exactly as the reference does. The
five-term accumulation is closed by `renorm5()`, the QD renormalization companion to that schedule.

Two smaller pieces came with it:

- `multiply_cascade_by_double()`, the equivalent of qd's separate `operator*=(double)`. A
  one-component operand is common - scaling by a double, and the initial quotient estimate inside
  every division - and running it through the full 4x4 schedule wastes ten of the sixteen partial
  products. `multiply_cascades` dispatches to it on three comparisons.
- A non-finite guard. `two_prod(inf, x)` computes `fma(inf, x, -inf)`, which is NaN, and the sums
  downstream smear it across the expansion. The old sort-based multiply turned `inf * 3` into a
  **NaN**; a faithful qd_mul returns `inf` with `-nan` in the trailing components, which is what
  classic `qd` does to this day. Both are now short-circuited to the IEEE result with cleared
  trailing components.

| measurement | before | after | classic `qd` |
|---|---|---|---|
| multiply, worst rel err (full-width operands) | 1.7e-63 | **1.6e-65** | 1.6e-65 |
| multiply, nsec/op | 581 | **82** | 73 |
| exp, nsec/op | 26036 | **6642** | 4165 |
| log, nsec/op | 75321 | **21814** | 13183 |
| sqrt residual, ulps of 2^-212 | 15 | **8.6** | 1.0 |
| `inf * 3` | NaN | inf | inf |

### N=3

The same schedule was then ported to `floatcascade<3>`, one order shorter: products through eps^2
are accumulated exactly with `three_sum`/`two_sum` chains, everything at eps^3 and below is summed
in plain arithmetic, and `renorm4()` closes four terms into three components. `multiply_cascade_by_double`
has an N=3 form for the same reason it has an N=4 one.

Measured over 400 random full-width triple-double products against the exact dyadic oracle, and on
the benchmark suite:

| measurement | sorted accumulation | qd_mul schedule |
|---|---|---|
| multiply, worst error | 5.53 ulps of 2^-159 | **0.23 ulps** |
| multiply, products past 1 ulp | 102 of 400 | **0 of 400** |
| square, worst error | 9.72 ulps | **0.22 ulps** |
| multiply, nsec/op | 214 | **63** |
| exp, nsec/op | 13406 | **3280** |
| log, nsec/op | 26224 | **7009** |
| sin, nsec/op | 9763 | **2846** |

Division is untouched by this and remains the weakest `td_cascade` operation at 3.2 ulps worst
case; it uses a Newton refinement whose residual path has its own rounding behaviour.
