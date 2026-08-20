# elreal on narrow hosts: the limit is exponent range, not precision

**Status:** design proposal. Nothing here is implemented.
**Context:** #1051, #1188, #933, epic #923.

## Summary

`elreal<FpType>` saturates on narrow hosts -- `bfloat16` stops near 33 decimal
digits, `float` near 37, while `double` reaches ~307. The standing explanation
is that the transcendental *series arithmetic* degrades below `float`, and #1051
proposes to fix it by evaluating the series on a wider intermediate host and
rounding the result down into narrow blocks.

Measurement does not support that explanation, and the proposed fix would not
lift the ceiling.

The limit is the **host's exponent range**. It is not the host's precision, and
it is not the error-free transforms. It is also not inherent to McCleeary's
algorithm: the block representation already carries the field needed to remove
it, and simply is not using it.

## Evidence

### The EFTs are exact on every host

200,000 random operand pairs per host, scored against the exact dyadic-rational
oracle (`verification/dyadic_exact.hpp`):

| host | `two_sum` | `two_prod` |
|------|-----------|------------|
| `bfloat16` (k=7) | 0 inexact | 0 inexact |
| `float` (k=24) | 0 inexact | 0 inexact |
| `double` (k=53) | 0 inexact | 0 inexact |

Whatever caps the narrow hosts, it is not the primitives.

### Precision is not the binding constraint

`float` carries 24 significand bits and `bfloat16` carries 7 -- a factor of 3.4.
They saturate 4 decimal digits apart, at 37 and 33. If precision per block were
the limit, the gap would be enormous. It is not, because they share an 8-bit
exponent.

### Every host's ceiling matches its exponent range

| host | `min_exponent` | predicted ceiling | measured |
|------|----------------|-------------------|----------|
| `double` | -1021 | ~307 digits | 306 |
| `float` | -125 | ~37 digits | 37 |
| `bfloat16` | -125, plus the `+2k` narrow-host margin -> -111 | ~33 digits | 33 |

The entire `float`-vs-`bfloat16` difference is the `2*k` margin the narrow path
adds (14 bits for `bfloat16`, none for `float`): 4 decimal digits.

### The mechanism

A `block<FpType>` is a pair `(v: FpType, exp: integer<256>)` with

```
combined_exponent = scale_of_v() + exp
```

and the McCleeary invariant, from `block.hpp`:

```
// is_normalised(): true iff `v` is a finite, non-zero, non-subnormal value.
// Subnormals fail this predicate; McCleeary blocks must avoid them
// because 0-overlap accounting assumes the leading bit is set.
```

`v` is **not** renormalised into `[1,2)`; it carries its own scale. As a series
or division refines, each successive block's `v` is smaller than the last, until
`v` goes subnormal, `is_normalised()` fails, and refinement arrests. That is the
`min_exponent + 2*k` floor, which appears in three places, all gated to `k < 24`:
`series_stop_exp` (`math/constants.hpp`), `exp_floor` (`divide.hpp`), and
`host_exp_floor` (`online_divide.hpp`).

Instrumenting a depth-24 `e_zbcl` confirms it directly:

| host | `v` scale range | min combined exponent | `min_exponent` |
|------|-----------------|-----------------------|----------------|
| `bfloat16` | `2^2` down to `2^-117` | -118 | -128 |
| `float` | `2^2` down to `2^-123` | -124 | -125 |
| `double` | `2^2` down to `2^-988` | -989 | -1021 |

Two things follow. Each host runs `v` down to within a few bits of its own
subnormal wall -- and `min combined exponent` is within 1 of the minimum `v`
scale, which means the `integer<256>` `exp` field is carrying **approximately
zero**. The unbounded exponent added in #1061 exists but is not being used to
hold scale in these paths.

## Why #1051's proposed fix does not work

Evaluating the series at `double` and rounding into `bfloat16`-hosted blocks
targets precision, which was never the constraint. The resulting blocks still
carry their scale in `v`, so they still hit `min_exponent` at the same place, and
the ceiling does not move. It also costs the thing the study exists to measure:
a block-shape study whose interior arithmetic runs at `double` is not measuring a
narrow datapath.

The storage arithmetic is unfavourable too. Table A of the block-shape study
measures `sizeof(block)` at 36 B for `bfloat16` and 40 B for `double`, because
the wide exponent field dominates the struct. Per byte that is 0.19 payload bits
for `bfloat16` against 1.33 for `double` -- `double` is 6.8x more
storage-efficient. A narrow host reaching high precision would need ~7.6x more
blocks at ~0.9x the size each.

## Status: DONE. The ceiling is gone on both narrow hosts.

| host | k | before | after | blocks | digits/block |
|------|---|--------|-------|--------|--------------|
| `double` | 53 | ~306 | ~306, **bit-identical** | 19 | 16.1 |
| `float` | 24 | **37 (hard cap)** | **319** | 50 | 6.4 |
| `bfloat16` | 7 | **33 (hard cap)** | **146 and climbing** | 53 | 2.8 |

float now reaches essentially the full 320-digit reference. bfloat16 grows
linearly with depth with no ceiling in sight: 28 / 45 / 78 / 112 / 146 digits at
depths 8 / 16 / 32 / 48 / 64. The prediction in the section above was that a
narrow host would need roughly `53/k` times as many blocks -- 2.2x for float
against 2.6x measured, 7.6x for bfloat16 against 5.9x measured.

`double` is untouched **by construction**, not by testing: the change sits behind
`if constexpr (needs_scale_normalisation)` and compiles away entirely for a
wide-exponent host. Verified bit-for-bit all the same -- every block's mantissa
and exponent, across pi, e, sqrt2, division and ln2 -- against the pre-change
build.

46/46 elreal tests pass under gcc 13.3 and clang 18.1, in 17s against a 7s
baseline. The increase is real work: float now computes 319 digits where it used
to stop at 37.

## What it took

Three things. The order they were found in is the useful part, because two of the
wrong turns came from reasoning ahead of the measurement.

### 1. Normalise OPERANDS, not results

This is the general rule, and it is the whole fix in one line. An EFT that runs at
the operands' natural scale has already lost bits to the subnormal range by the
time it returns; normalising its outputs cannot put them back.

```
x.normalise();
y.normalise();
auto se = block_two_div_rem(x, y);
```

Applied at all three sites -- `twoSumRN`, `block_two_mult` through
`singleMultHelper`, and `block_two_div_rem` through `twoDivZBCL`. Fixing only the
divide lifted bfloat16 from 33 to 40 digits and then plateaued; only with all
three does it become unbounded.

### 2. The nonadjacent (k+1) shortcut in twoSumRN

Blocks already `k+1` apart need no arithmetic at all: their exact sum is the pair,
in decreasing order. Skipping there bounds every surviving alignment shift, which
is what makes normalised operands safe to align.

The threshold is `k+1`, not `k`. `twoSumRN` owes its callers its property 5 --
the residual is at most half an ulp, i.e. the round-to-nearest decomposition --
and `threeAdd` (Definition 4.2.1) is a fixed chain whose 0-overlap proof rests on
it. Plain 0-overlap allows `b` up to just under `ulp(a)`, so anything above half
an ulp carries and `RN(a+b) != a`; returning `{a,b}` there is exact but not
round-to-nearest, and the chain does not survive it. Measured on a `double` host
at `E(a) = 0`:

| `b` | `RN(a+b) == a`? |
|-----|------------------|
| quarter-ulp | yes |
| half-ulp | yes |
| 0.75 ulp | **no** |
| just under 1 ulp | **no** |

This is the non-overlapping versus **nonadjacent** distinction that also governs
the Shewchuk COMPRESS step (#1340) -- the same one-bit margin, for the same
reason.

It also turned out to be the performance fix. Without it the alignment shift is
unbounded and the suite took 1469s; with it, 17s.

### 3. Gate on EXPONENT RANGE, not on k

The first gate copied the floors' `k >= 24` predicate. That was wrong: it excluded
`float`, whose k is exactly 24 but whose ceiling is squarely an exponent ceiling.
float and bfloat16 both carry an 8-bit exponent (`min_exponent` -125) and both
exhaust it -- v runs to 2^-123 and 2^-117 respectively, within a couple of bits of
the wall. double's 11-bit exponent gives ~1000 binades of headroom that it never
approaches, so it is limited by block count and has nothing to gain.

```
static constexpr bool needs_scale_normalisation =
    (std::numeric_limits<FpType>::min_exponent > -500);
```

## Wrong turns, kept

**Normalise alone.** Moving scale out of `v` spreads the `exp` fields, and
`twoSumRN`'s then-unconditional alignment underflowed the operand before the EFT
saw it. The lesson became rule 2.

**The shortcut at threshold `k`.** Refuted on its own, before normalise was
involved, by `el_math_trigonometry`. The lesson is in rule 2 above.

**"The producer is `addRec_step`."** Reported from a backtrace, and wrong: the
assertion fires there because `addRec_step` is the first thing to *consume* the
offending stream via `tail()`. Instrumented per state rather than through a
shared thread_local -- which was contaminating the first pass with cross-stream
pairs -- its output is clean, as is its workspace over every consecutive pair. The
real producer was `twoDivZBCL`, reproducible with no `add` and no `infsum` at all.

**The plateau at 40 digits** was the add path: `twoSumRN` had neither the shortcut
nor operand normalisation, because the script meant to add them asserted on a
stale function name and never wrote the file.

## A note on the test suite

`el_math_sqrt` went from 2.3s to not finishing in 500s, and the fix was in the
test, not the library: it took sqrt's default `depth = 64` while checking a 1e-4
tolerance. That was only ever cheap because the refinement floor stopped the
underlying division early. Passing the depth the test actually needs restores it
to 2.3s.

That is the same accidental coupling this whole exercise is about -- something
that was fast because of an implementation limit rather than because it asked for
little. Worth watching for elsewhere.

## What this unblocks

The block-shape study can now ask its real question -- what a narrow block shape
costs in blocks and time at a *fixed* precision target -- instead of reporting
that narrow hosts cannot reach the target at all. That is the missing half of
#1188's design matrix, and it is answered without computing anything in a wider
type: the limbs stay true to their FpType and every EFT runs in host arithmetic.

Note that this does not change the user-facing recommendation in section H of the
block-shape study. Narrow hosts remain dominated for any actual precision target
-- `dd` and `qd` cover their range as compile-time constants and beat them on
throughput. What changes is that the silicon question can now be measured.
