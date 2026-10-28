# posit sqrt: three selectable algorithms

Design for [#1776](https://github.com/stillwater-sc/universal/issues/1776), with
[#1846](https://github.com/stillwater-sc/universal/issues/1846) as its wide-posit sub-issue.

## Requirements

From #1776, three algorithms selectable with a compile switch:

1. **native**: a square root computed in the posit's own precision, correctly rounded.
2. **shim**: a fast approximation through the hardware `double`, so that emulated code is not
   unduly slow.
3. **cascade**: a high-precision intermediate (`dd_cascade`, `td_cascade` or `qd_cascade`, as the
   width needs), expected to be faster than native and slower than the shim.

## What is wrong today

With `POSIT_NATIVE_SQRT == 0`, the default, every posit takes the same path:

```cpp
return posit<nbits, es, bt>(std::sqrt((double)a));
```

That rounds twice. A square root rounded first to an intermediate format of q bits and then to p
bits is correctly rounded for every input only when q >= 2p + 2. For a posit, p is largest near 1:
p = nbits - 2 - es.

| posit | p near 1 | bits needed | binary64 gives | result |
|---|---|---|---|---|
| `posit<16,1>`, `posit<24,es>` | <= 22 | <= 46 | 53 | correctly rounded |
| `posit<32,es>` (#1776) | 28 - 30 | 58 - 62 | 53 | misrounded near ties only: 5 of the 268,435,456 `posit<32,2>` in [1, 4) |
| `posit<64,3>` (#1846) | 59 | 120 | 53 | 97.7% of random encodings misrounded; #1846 measures a mean of 17.5 ulp, max 53 |
| `posit<128,4>` | 122 | 246 | 53 | about 70 bits lost; and maxpos = 2^2016 is past double's range |
| `posit<256,5>` | 250 | 502 | 53 | about 200 bits lost; range as above |

So the shim is correctly rounded whenever nbits <= 27 + es. It also needs posit's range to
fit in double's: (nbits - 2) * 2^es <= 1023. `posit<128,4>` and `posit<256,5>` fail that test, so
their extremes overflow to infinity or underflow to zero on the way through `double`.

Two more facts about the current code:

- **The `POSIT_FAST_POSIT_*` specializations in `math/sqrt.hpp` are dead code.** No header defines
  those macros any more. The 16- and 32-bit ones are SoftPosit's correctly rounded integer
  algorithm. The 64-, 128- and 256-bit ones `throw` a string literal (#1846); they run only for a
  user who defines the macro by hand.
- **The `POSIT_NATIVE_SQRT == 1` path** is two Newton steps from a linear seed, built on the
  legacy `internal::value<>`. It has not been measured for correct rounding.
- **posit has no math regression suite.** `static/tapered/posit/` has `api`, `arithmetic`,
  `conversion` and `logic` only. Nothing tests `sqrt`.

## Measurements

A prototype of each algorithm, for nbits <= 64. The posits use the default `bt = uint8_t`.
Measured on x86-64 with gcc 15.2 and clang 21.1 at -O2, over 200,000 inputs.

Correct rounding was checked against the posit rounding rule itself, independently of any of the
algorithms. y is the correctly rounded sqrt(a) exactly when mid(y-1, y)^2 <= a < mid(y, y+1)^2.
The midpoints are the (nbits+1)-bit patterns 2y - 1 and 2y + 1, which is how posit rounding is
defined, and the comparison is exact in integers.

**Misrounded results:**

| | `posit<32,2>` random | `posit<32,2>` near ties | `posit<32,2>` every positive encoding | `posit<64,3>` random | `posit<64,3>` near ties |
|---|---|---|---|---|---|
| shim | 0 | 1 | 5 in [1, 4) | 195,315 | 196,778 |
| native | 0 | 0 | **0 of 2,147,483,646** | 0 | 0 |
| cascade (dd or td, plus the exact check below) | 0 | 0 | - | 0 | 0 |

**Cost, ns per sqrt (gcc / clang):**

| | `posit<32,2>` | `posit<64,3>` |
|---|---|---|
| shim | 184 / 263 | 341 / 459 |
| native, posit's `uint64` encoder | **93 / 245** | **144 / 438** |
| native, the generic `convert_` encoder | 181 / 329 | 315 / 597 |
| cascade, dd_cascade | 1354 / 1836 | 2782 / 3777 |
| cascade, td_cascade | 1683 / 2201 | 3431 / 4576 |
| *for scale: one posit multiply* | *303 / 486* | *593 / 894* |

**Where the time goes (gcc):**

| operation | ns |
|---|---|
| `std::sqrt(double)` | 1.6 |
| `posit<32,2>` -> `double` | 143 |
| `posit<64,3>` -> `double` | 276 |
| `double` -> `posit<64,3>` | 48 |
| `dd` sqrt | 72 |
| `dd_cascade` sqrt | 542 |
| `td_cascade` sqrt | 668 |
| `qd` sqrt | 1058 |

What the numbers say:

1. **The shim's cost is the conversion, not the square root.** Reading a posit into a `double`
   takes 143 ns at 32 bits and 276 ns at 64; `std::sqrt` takes 1.6.
2. **Native costs no more than the shim, and with posit's `uint64` encoder it costs less.** Its
   square root is a 128-bit integer computation of a few nanoseconds. The rest is decoding and
   encoding the posit, which the shim also pays for.
3. **The cascade is 7 to 10 times slower than native**, contrary to the expectation in #1776. The
   cascade square roots themselves cost 542 to 1058 ns, more than native's whole computation.
   Better glue code (splitting the significand into doubles with integer operations rather than
   posit arithmetic) cannot bring it below the cost of the cascade sqrt. `dd` takes 72 ns, so the
   cascade types are 7 times slower than `dd` at the same width, which is worth an issue of its own.

## Design

### The selector

The pattern is the one `td_cascade` uses for `UNIVERSAL_TD_CASCADE_SQRT_ALGORITHM`, in
`include/sw/universal/number/posit/math/sqrt.hpp`:

```cpp
#define POSIT_SQRT_NATIVE  1   // correctly rounded, the posit's own precision
#define POSIT_SQRT_SHIM    2   // posit(std::sqrt(double(a))): fast, approximate past nbits = 27 + es
#define POSIT_SQRT_CASCADE 3   // a dd/td/qd_cascade intermediate, then rounded exactly
#ifndef POSIT_SQRT_ALGORITHM
#define POSIT_SQRT_ALGORITHM POSIT_SQRT_NATIVE
#endif
```

Each algorithm is also a named function, so code can mix them. For example, a solver can use the
shim in an inner loop and native for the final residual:

```cpp
template<unsigned nbits, unsigned es, typename bt> posit<nbits, es, bt> sqrt_native (const posit<nbits, es, bt>&);
template<unsigned nbits, unsigned es, typename bt> posit<nbits, es, bt> sqrt_shim   (const posit<nbits, es, bt>&);
template<unsigned nbits, unsigned es, typename bt> posit<nbits, es, bt> sqrt_cascade(const posit<nbits, es, bt>&);

template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> sqrt(const posit<nbits, es, bt>& a);   // the one POSIT_SQRT_ALGORITHM selects
```

`POSIT_NATIVE_SQRT`, the old switch, keeps working: when it is defined to 1 and
`POSIT_SQRT_ALGORITHM` is not defined, it selects `POSIT_SQRT_NATIVE`.

All three share the special cases: sqrt(0) = 0, sqrt(NaR) = NaR, and a negative argument gives NaR.

### 1. native

The square root of the significand as an integer, with the remainder kept as the sticky bit, then
one rounding through posit's encoder:

1. Decode a = S * 2^e, where S is the significand as an integer, hidden bit included.
2. Shift S left so that the exponent is even and N = S << k has at least 2(p + 1) bits.
   Then sqrt(a) = sqrt(N) * 2^((e - k) / 2).
3. R = isqrt(N), the integer square root, and sticky = (N != R^2). R carries at least p + 1 bits:
   the result's bits, then the round bit. sticky is everything below them.
4. Encode R, the sticky bit and the scale with one round to nearest, ties to even. The result is
   correctly rounded because R and sticky together determine which side of every rounding
   boundary sqrt(a) lies on.

**Integer square root for nbits <= 64.** N fits in 128 bits and R in 64. The prototype's version:

1. seed: R0 = sqrt(double(N)), good to about 52 bits;
2. one correction from the exact residual: R1 = R0 + (N - R0^2) / (2 R0). The residual is computed
   exactly in 128 bits, and the quotient in `double`, because only its top 10 or so bits matter;
3. an exact fix-up: decrement while R^2 > N, increment while (R + 1)^2 <= N. Correctness rests on
   the fix-up alone, so a poor seed costs time, never accuracy.

The 128-bit products are 64 x 64 -> 128 built from 32-bit halves, so the code is portable: no
`__int128`, and no compiler intrinsics needed (MSVC's `_umul128` can be a later optimisation).

**Integer square root for nbits > 64.** The same three steps, on multi-limb integers. Each
correction step doubles the correct bits, so 128 bits need 2 steps and 256 bits need 3. Each step
needs a full residual N - R^2 (one multi-limb square) and a quotient of about half the bits. The
existing integer `<>` and blockbinary multiply and division can do this. The fix-up stays exact.

**The encoder.** posit already has a `uint64_t` encoder, `encode_positive_with_scale_and_fraction`,
which rounds to nearest even with a sticky bit for nbits <= 64. It is private. The sqrt needs it
(or a public `detail` wrapper of it). It halves native's cost against the generic
`blocksignificand` `convert_`. For nbits > 64, `convert_` is the encoder.

### 2. shim

Today's code, unchanged: `posit(std::sqrt(double(a)))`. Its contract is documented where it is
defined:

- correctly rounded for nbits <= 27 + es;
- otherwise within about 2^(p - 53) ulps of the posit's precision, plus 1 ulp near rounding ties,
  as long as p <= 53. Past p = 53 the input itself is rounded on the way into `double`;
- for posits whose range exceeds double's ((nbits - 2) * 2^es > 1023), the extremes overflow.

That makes it a choice the user makes knowingly, never the default.

### 3. cascade

The estimate comes from a cascade sqrt, and the result is then made correctly rounded by the
exact midpoint check:

1. Split the significand into doubles exactly, with integer operations: the top 53 bits, the next
   53, and so on. Two doubles cover p <= 106 and three cover p <= 159.
2. Take the cascade's sqrt. The estimate only has to be within 1 ulp of the posit: a `double`
   suffices while p <= 52 (the shim plus the check below), `dd_cascade` while p <= 104,
   `td_cascade` while p <= 156 and `qd_cascade` up to 208.
3. Round the cascade's components into the posit in one step: sum them as an integer significand
   with a sticky bit, then encode once.
4. The exact check: if a < mid(y-1, y)^2, step down; if a >= mid(y, y+1)^2, step up. The midpoint
   squares are exact integer products of (p+1)-bit significands. This is what makes the result
   correctly rounded at every width. The cascade only has to be close.

Limits:

- A posit whose range exceeds double's ((nbits - 2) * 2^es > 1023: `posit<128,4>`,
  `posit<256,5>`) cannot be represented in a cascade. For those, `sqrt_cascade` falls back to
  native at compile time.
- p > 212 does not fit in `qd_cascade`'s significand. There too, it falls back to native.

On the measurements above it is the slowest of the three. Whether to build it is the first open
question below.

## Verification

A new `static/tapered/posit/math/sqrt.cpp`, in the standard MANUAL_TESTING + REGRESSION_LEVEL
skeleton. Its oracle is the midpoint rule above, so it is independent of all three algorithms.

| level | native and cascade: correctly rounded | shim: within its documented bound |
|---|---|---|
| 1 | every encoding of `posit<8,0..2>` and `posit<16,1..2>`; the 173 tie cases of #1776; 10,000 random `posit<64,3>` | the same inputs |
| 2 | every `posit<32,2>` in [1, 4) (268 M, about 25 s); #1846's 100,000 inputs; random `posit<128,4>` | |
| 3 | the extremes of the range for each configuration; random `posit<256,5>` | |
| 4 | every positive `posit<32,2>` (2^31, about 4 min) | |

Plus the existing users of posit sqrt in the tree: the complex and tracked tests.

## Plan

| PR | content | fixes |
|---|---|---|
| 1 | the selector; native for nbits <= 64; the shim documented; the dead `POSIT_FAST_POSIT_*` sqrt blocks and the legacy `value<>` path removed; the regression test. **BEHAVIOUR-CHANGE**: `sqrt` of posits past nbits = 27 + es changes, to the correctly rounded result | #1776; #1846 for `posit<64,3>` |
| 2 | native for nbits > 64, multi-limb | #1846 for `posit<128,4>` and `posit<256,5>` |
| 3 | cascade, if wanted | |

`rsqrt` is `sqrt` followed by a reciprocal, and so rounds twice. It is outside this design; a
correctly rounded `rsqrt` would use the same integer square root with a division folded in.

## Open questions

1. **Option 3 is the slowest of the three, not between native and shim.** Should it still be
   built? Options: build it as specified, as a correctly rounded alternative that keeps the work
   in floating point; defer it until the cascade square roots are faster (a separate issue: 542
   to 1058 ns against `dd`'s 72); or drop it.
2. **The default.** This design makes native the default because it is correctly rounded and no
   slower than the shim. That changes `sqrt` results for every posit past nbits = 27 + es (the
   BEHAVIOUR-CHANGE above). Should the shim stay the default instead, for compatibility?
3. **Selection per type.** A single macro selects the algorithm for every posit in a translation
   unit. Is a per-type choice needed, for example native for `posit<64,3>` and the shim for
   `posit<16,1>`? The named functions cover that case without a second mechanism.
