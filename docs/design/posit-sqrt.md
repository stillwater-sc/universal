# posit sqrt: one native, correctly rounded algorithm

Design for [#1776](https://github.com/stillwater-sc/universal/issues/1776), with
[#1846](https://github.com/stillwater-sc/universal/issues/1846) as its wide-posit sub-issue.

## Decision

#1776 asked for three algorithms behind a compile switch: native, a shim through `double`,
and a `dd/td/qd_cascade` intermediate. The prototypes below measured all three. Native is
correctly rounded and also the fastest, so it is the only algorithm the library keeps.

- The **shim** is not faster. Its cost is the conversion of the posit to `double`, not the
  square root. And it misrounds past nbits = 27 + es.
- The **cascade** is 7 to 10 times slower than native.

This follows the policy of #1724 for the multi-double types: native full-precision algorithms
only, no switches that route through `double`.

The decision covers sqrt alone. The other posit math functions also compute through `double`,
but for them the hardware path is the fast one. They are
[#1855](https://github.com/stillwater-sc/universal/issues/1855).

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
| `posit<32,es>`, es = 1..3 (#1776) | 27 - 29 | 56 - 60 | 53 | misrounded near ties only: 5 of the 268,435,456 `posit<32,2>` in [1, 4) |
| `posit<64,3>` (#1846) | 59 | 120 | 53 | 97.7% of random encodings misrounded; #1846 measures a mean of 17.5 ulp, max 53 |
| `posit<128,4>` | 122 | 246 | 53 | about 70 bits lost; and maxpos = 2^2016 is past double's range |
| `posit<256,5>` | 250 | 502 | 53 | about 200 bits lost; range as above |

Also:

- **The `POSIT_FAST_POSIT_*` specializations in `math/sqrt.hpp` are dead code.** No header
  defines those macros any more; a few benchmarks and the `mixedprecision/tensor/cg` drivers still
  define them, to no effect. The 64-, 128- and 256-bit ones `throw` a string literal (#1846).
- **The `POSIT_NATIVE_SQRT == 1` path** is two Newton steps from a linear seed, on the legacy
  `internal::value<>`. It was never measured for correct rounding.
- **posit has no math regression suite.** Nothing tests `sqrt`.

## Measurements

Prototypes for nbits <= 64, with the default `bt = uint8_t`, on x86-64 with gcc 15.2 and
clang 21.1 at -O2. The random and near-tie columns, and the costs, are over 200,000 sampled
inputs each. The "every positive encoding" column is exhaustive: native over all 2^31 - 1, the
shim over the 2^28 in [1, 4).

Correct rounding was checked against the posit rounding rule itself, independently of the
algorithms. y is the correctly rounded sqrt(a) exactly when mid(y-1, y)^2 <= a <= mid(y, y+1)^2,
where an equality is a tie and y must then be the even encoding. The midpoints are the
(nbits+1)-bit patterns 2y - 1 and 2y + 1, which is how posit rounding is defined, and the
comparison is exact in integers. minpos has no lower boundary, since nothing rounds to zero, and
maxpos no upper one. Ties happen only where the regime has crowded out the fraction: in
`posit<3,1>`, sqrt(1/4) = 1/2 is the midpoint of 1/4 and 1.

**Misrounded results:**

| | `posit<32,2>` random | `posit<32,2>` near ties | `posit<32,2>` every positive encoding | `posit<64,3>` random | `posit<64,3>` near ties |
|---|---|---|---|---|---|
| shim | 0 | 1 | 5 in [1, 4) | 195,315 | 196,778 |
| native | 0 | 0 | **0 of 2,147,483,647** | 0 | 0 |
| cascade (dd or td, plus an exact check) | 0 | 0 | - | 0 | 0 |

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

1. **The shim's cost is the conversion, not the square root.**
2. **Native beats the shim only with posit's `uint64` encoder.** With the generic `convert_`, it
   is slower than the shim on clang (329 against 263 ns). So the encoder must be reachable from the
   math library; today it is private.
3. **The cascade square roots alone cost more than native's whole computation.** They are 7 times
   slower than `dd` at the same width, which may deserve its own issue.

## Design

### The function

```cpp
template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> sqrt(const posit<nbits, es, bt>& a);   // correctly rounded
```

No selector and no named alternatives. sqrt(0) = 0, sqrt(NaR) = NaR, and a negative argument
gives NaR. The table lookups for `posit<3,0>`, `posit<3,1>`, `posit<4,0>`, `posit<5,0>`,
`posit<8,0>` and `posit<8,1>` are removed: the native algorithm is exhaustively correct at those
sizes, and the `posit<3,1>` table was not (it gives sqrt(1/4) = 1/4, where the tie goes to 1).

`rsqrt` stays `sqrt` followed by a reciprocal, which rounds twice; a correctly rounded `rsqrt` is
outside this design.

### The algorithm

The square root of the significand as an integer, with the remainder kept as the sticky bit, then
one rounding:

1. Decode a = S * 2^e, where S is the significand as an integer, hidden bit included.
2. Shift S left so that the exponent is even and N = S << k has at least 2(p + 1) bits.
   Then sqrt(a) = sqrt(N) * 2^((e - k) / 2).
3. R = isqrt(N), the integer square root, and sticky = (N != R^2). R carries at least p + 2 bits:
   the result's bits, the round bit, and at least one more below it.
4. Append sticky to R as one more, lowest, bit and encode that with
   scale = floor_log2(R) + (e - k) / 2, where k has the parity of e so (e - k) / 2 is an integer,
   rounding once to nearest, ties to even. For nbits <= 64, N is in [2^126, 2^128), so
   floor_log2(R) = 63; wider posits take it from the normalized multi-limb root. The encoder sees an exact tie only when the round bit is set and nothing
   below it is, which with the sticky bit appended means sqrt(a) really is the midpoint. So R and
   sticky together decide which side of every rounding boundary sqrt(a) lies on.

**nbits <= 64.** N is a 128-bit integer in [2^126, 2^128) and R a 64-bit one:

1. seed: R0 = sqrt(double(N)), good to about 52 bits;
2. one correction from the exact residual: R1 = R0 + (N - R0^2) / (2 R0), with the residual exact
   in 128 bits and the quotient in `double`, because only its top bits matter;
3. an exact fix-up: decrement while R^2 > N, increment while (R + 1)^2 <= N.

Correctness rests on the fix-up alone, so a poor seed costs time, never accuracy. The 128-bit
products are built from 32-bit halves: no `__int128`, no intrinsics.

The encoder is posit's `uint64_t` encoder, `encode_positive_with_scale_and_fraction`, which rounds
to nearest even on every bit it is given past the posit's precision. posit gains a public member
that wraps it, `setpositive(scale, fraction, num_fbits)`. sqrt passes the 63 bits of R below its
hidden bit followed by sticky: 64 fraction bits, at least 3 more than any posit<=64 keeps.

**nbits > 64.** The same construction on 64-bit limbs, with R of n = ceil((fbits + 3) / 64)
limbs and N of 2n. The planned Newton steps were not needed: `limb_kernels::sqrtrem`, the exact
square root with remainder that efloat already uses (Zimmermann's Karatsuba square root, one
division and one square of half the length per level), returns R and N - R^2 together, and a
nonzero remainder is the sticky bit.

Neither of posit's generic conversions is fast enough here. Measured for `posit<128,4>` (gcc,
ns): decoding through `to_value()` costs 289 and encoding through `convert_` 344, while `sqrtrem`
itself costs 21. A first version built on them took 1500 ns, twice the shim's 800. So sqrt
decodes the encoding straight from its limbs, and encodes with a limb version of the
`uint64_t` encoder: the regime, exponent and R below its hidden bit are laid out on limbs, the
top nbits - 1 bits kept, and the result rounded to nearest even from the guard bit, the dropped
bits and sticky, saturating at minpos and maxpos.

| ns per sqrt, gcc | native | shim |
|---|---|---|
| `posit<80,3>` | 95 | 632 |
| `posit<128,4>` | 100 | 808 |
| `posit<256,5>` | 148 | 1248 |

## Verification

A new `static/tapered/posit/math/sqrt.cpp`, in the standard MANUAL_TESTING + REGRESSION_LEVEL
skeleton. Its oracle is the midpoint rule above, independent of the algorithm.

| level | inputs |
|---|---|
| 1 | every encoding of `posit<2,0>`, `<3,0>`, `<3,1>`, `<4,0>`, `<4,1>`, `<5,0>`, `<5,2>`, `<6,3>`, `<8,0..2>`, `<8,5>`, `<10,1>`, `<12,2>`, `<16,1>`, `<16,2>` and `<16,1,uint16_t>`; for `posit<32,2>`, the encodings 0x40000003, 0x4cc71534 and 0x4dff6575 that the shim misrounds and 0x40000000 to 0x4000FFFF; 20,000 random, near-tie and binade-top inputs each for `posit<24,2>`, `<32,2>`, `<32,3>`, `<32,2,uint32_t>`, `<40,2>`, `<48,2>`, `<56,2>`, `<64,0>`, `<64,2>`, `<64,3>`, `<64,3,uint32_t>` and `<64,3,uint64_t>` |
| 2 | every `posit<32,2>` in [1, 4) (268 M) |
| 3 | the extremes of each configuration's range |
| 4 | every positive `posit<32,2>`, minpos through maxpos (2^31 - 1) |

For nbits > 64 the oracle is the same midpoint rule on 32-bit words, with its own schoolbook
square, independent of the limb kernels. Level 1 checks it against the narrow oracle
(exhaustively for `posit<12,2>` and `<16,1>`, randomly for `<32,2>` and `<64,3>`), then runs the
random, near-tie and binade-top inputs for `posit<65,2>`, `<72,6>`, `<80,3>`, `<100,3>`, `<128,0>`,
`<128,4>` with `uint8_t`, `uint32_t` and `uint64_t` blocks, `<192,4>` and `<256,5>`, and
`posit<128,4>`'s maxpos = 2^2016, past `double`'s range. Level 2 adds 100,000 `posit<128,4>` and
20,000 `posit<256,5>`; level 3 their extremes.

## Plan

| PR | content | fixes |
|---|---|---|
| 1 | native for nbits <= 64; the public encoder wrapper; `POSIT_NATIVE_SQRT`, its `value<>` path, the `POSIT_FAST_POSIT_*` sqrt blocks and their dead defines removed; the regression test. **BEHAVIOUR-CHANGE**: `sqrt` of posits past nbits = 27 + es changes to the correctly rounded result. Posits wider than 64 bits keep the `double` path until PR 2 | #1776 |
| 2 | native for nbits > 64, multi-limb; the `double` path removed | #1846 |
