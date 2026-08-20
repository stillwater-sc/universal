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

## Status: DONE. The ceiling is gone on every host, double included.

| host | k | before | after | blocks to 319 | digits/block (k log10 2) |
|------|---|--------|-------|---------------|--------------------------|
| `double` | 53 | **307 (its own wall)** | **319** | 22 | 16.0 |
| `float` | 24 | **37 (hard cap)** | **319** | 50 | 7.2 |
| `bfloat16` | 7 | **33 (hard cap)** | **146 and climbing** | -- | 2.1 |
| `half` | 11 | 20 | **20, still capped** | -- | 3.3 |

319 is the 320-digit reference's own limit, not the types': measuring past it needs
a longer reference. `half` is a separate defect, tracked on its own.

### The gate was wrong, twice, and the inconsistency is what caught it

An earlier revision of this document reported double as unchanged and
**bit-identical**, and treated that as the safety property. It was preserving a
bug.

double's ceiling was 307 decimal digits. `2^-1021` -- its `min_exponent` -- is
`1e-307`. double was sitting exactly on its own exponent wall, and excluding it
from normalisation kept it there. The earlier reading that it had "33 bits of
headroom" at `2^-988` missed that 33 is less than `k = 53`: less than one block.

What exposed it was an ordering that cannot be true. Normalised float reached 319
digits while unnormalised double stopped at 307 -- even though double's exponent
range is eight times wider, and the entire thesis of this document is that the
exponent range sets the ceiling. Two hosts limited by *different* mechanisms are
not comparable, and the physical argument said so before any further measurement
did.

Normalised, double reaches the reference at 22 blocks and its representation
extends well past its wall: trailing exponent -1097 at depth 16, -2793 (about
`1e-841`) at depth 48.

### Theory against measurement

Every host is now limited only by block count, at `k * log10(2)` digits per block.
To reach the 320-digit reference (~1063 bits):

| host | k | predicted blocks | measured | excess |
|------|---|------------------|----------|--------|
| `double` | 53 | 20 | 22 | +10% |
| `float` | 24 | 44 | 50 | +14% |
| `bfloat16` | 7 | 152 | (146 digits at 53 blocks, still climbing) | -- |

The 10-14% excess is `kSeriesGuard`, the working blocks the series helpers carry
internally. The ordering is now the physically expected one: the widest host needs
the fewest blocks.

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

```cpp
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

```cpp
static constexpr bool needs_scale_normalisation =
    (std::numeric_limits<FpType>::min_exponent > -500);
```

## How a float block reaches past 2^-126

This is the question any reader will ask, and the answer is the heart of the
design.

A chain of float *values* cannot do it. float's exponent runs to `2^-126`, so
non-overlapping float values below 1.0 give `floor(126/24) = 5` blocks, about 120
bits, about 36 decimal digits. That calculation is exactly right -- and it is
precisely the 37-digit ceiling this document set out to explain. Before the fix,
the implementation *was* that calculation.

But a block is not a float value:

```text
value = v * 2^exp        v : FpType (normal)      exp : integer<256>
```

`e_zbcl<float>(48)`, dumped block by block:

```text
  idx |        v (float)  scale(v) |        exp (wide) |  combined E
    0 |       1.359140873         0 |                 1 |           1
    1 |       1.384932399         0 |               -24 |         -24
    2 |      -1.668617368         0 |               -49 |         -49
    ... |
   49 |       1.239909887         0 |             -1256 |       -1256

  blocks whose COMBINED exponent is below float's min_exponent: 45 / 50
  blocks whose STORED v is a normal float:                      50 / 50
```

Block 49 is `1.2399 * 2^-1256`. float cannot hold that value; the *pair* holds it
exactly. Every stored `v` remains a normal float, the spacing is `k = 24`, and
the expansion spans 1280 bits -- about 385 decimal digits, comfortably covering
the 319 measured.

This is McCleeary's design, not a workaround. The dissertation's exponents live
in Z (Haskell `Integer`), and `block.hpp` records that #1061 widened `exp` from
`int32` precisely because streaming division doubles the divisor's exponent per
level and overflowed a narrow field. The pre-fix bug was that the scale was being
carried in `v` instead of `exp`, which collapsed the design back to the five-block
calculation above.

The arithmetic stays in the host: significands are normal floats, and the `k+1`
shortcut means blocks further apart than `k+1` are never aligned, so every shift
that does happen is smaller than `k`.

### The cost, and why it is the silicon finding

A block is 36 B for float against 40 B for double, because the `integer<256>`
exponent dominates the struct -- 0.30 payload bits per byte for float, 1.33 for
double. So "float blocks reach 320 digits" is true of the type and is
simultaneously why narrow block shapes lose the hardware argument: a 256-bit
exponent bought for a 24-bit significand. That is section H's dominance result,
and this is its mechanism.

## Reach: elreal against ereal

The two adaptive-precision types in this library rest on different
representations, and the difference is not one of degree.

| | `ereal` (Priest / Shewchuk) | `elreal` (McCleeary LFPERA) |
|---|---|---|
| limb | a bare `double` | `(v : FpType, exp : integer<256>)` |
| scale lives in | the limb's own IEEE exponent | the block's separate wide exponent |
| reach | bounded by the host's exponent range | bounded by block count alone |
| ceiling | ~`(1021/53)` limbs -> a few hundred digits | none in practice |
| evaluation | eager | lazy, refine on demand |

A Priest/Shewchuk expansion is a sum of ordinary floating-point values. Its
smallest limb is a real `double`, so the whole expansion bottoms out at the host's
`min_exponent` -- `2^-1021`, about 307 decimal digits. `ereal<19>` measuring out
at ~293 digits is that wall, and no limb count escapes it, because limb 20 would
have to be smaller than a `double` can be.

McCleeary's block carries its scale *outside* the host type. The significand is
still a host value and the arithmetic is still host arithmetic, but the exponent
is an unbounded integer, so a block can sit at `2^-1256` or `2^-100000` just as
easily as at `2^-1`. Nothing bottoms out.

Measured, against a 3000-digit reference for `e` generated by exact integer
arithmetic (`sum 1/n!` scaled by `10^3050`, 1160 terms) rather than by anything in
this library:

| depth | digits | blocks | digits/block |
|-------|--------|--------|--------------|
| 16 | 322 | 22 | 14.6 |
| 32 | 577 | 36 | 16.0 |
| 48 | 833 | 52 | 16.0 |
| 64 | 1089 | 68 | 16.0 |
| 96 | **1598** | 99 | 16.1 |

Linear at `k * log10(2) = 15.95` digits per block, with no ceiling in sight --
1598 digits is where the sweep was stopped, not where the type stopped. The
320-digit reference used elsewhere in this document is a limit of the *measuring
instrument*; this one only has to be longer.

So the interesting comparison is not "elreal is more accurate than ereal". At any
precision `ereal` can reach, both are exact. It is that they have different
*kinds* of limit: `ereal`'s is a wall set by the host format, `elreal`'s is a
budget you choose. Past a few hundred digits `ereal` cannot be asked the question
at all.

What that costs is in the table above: 36-40 bytes per block against 8 bytes per
limb, and laziness on every access. `ereal` is the right tool up to its wall and
`elreal` is the only tool past it.

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
