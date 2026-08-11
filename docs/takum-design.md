# Takum: linear and logarithmic variants -- design assessment

Status: assessment / proposal
Date: 2026-08-11

## Question

The current Universal `takum<nbits, rbits, bt>` implements the *linear* takum. The
takum literature defines both a linear and a logarithmic variant. Do they need to
be two distinct types, or can they be parameterized into one type?

## What the two specifications actually say

The linear/logarithmic split is *not* in the original takum paper. Sequence:

- **[arXiv:2404.18603](https://arxiv.org/abs/2404.18603)** -- "Beating Posits at Their
  Own Game: Takum Arithmetic" (Hunhold, 2024). Defines takum as a **logarithmic**
  tapered-precision format. This is Definition 1.
- **[arXiv:2408.10594](https://arxiv.org/abs/2408.10594)** -- "Design and Implementation
  of a Takum Arithmetic Hardware Codec". **Definition 2** introduces the **linear
  takum**, and the paper builds a hardware codec for both.
- **[libtakum](https://github.com/takum-arithmetic/libtakum)** -- the C99 reference
  implementation by the same author. This is the authoritative tiebreaker on how the
  author intends the split to be structured in software.

### Shared encoding (identical in both variants)

An n-bit string `T = (S, D, R, C, M)`, n >= 12:

```
[S:1][D:1][R:3][C:r][M:p]        r in {0..7},  p = n - r - 5

r := uint(~R)  if D=0,          uint(R)        if D=1
c := -2^(r+1)+1+uint(C) if D=0, 2^r-1+uint(C)  if D=1     in [-255, 254]
m := 2^-p * uint(M)   in [0,1)

zero: S=0, D=R=C=M=0
NaR:  S=1, D=R=C=M=0
```

Two's-complement storage; negating the integer negates the value; integer ordering
equals value ordering. All of this is byte-for-byte the same in both formats.

### The only divergence: the value map

| | logarithmic takum (Def. 1) | linear takum (Def. 2) |
|---|---|---|
| trailing field | mantissa `M`, `m` | fraction `F`, `f` |
| scale | `l := (-1)^S (c + m)` | `e := (-1)^S (c + S)` |
| value | `(-1)^S * sqrt(e)^l` | `[(1 - 3S) + f] * 2^e` |
| base | sqrt(e) ~= 1.6487 | 2 |
| dynamic range | ~ +/-[4.2e-56, 2.4e55] | ~ +/-[1.7e-77, 5.8e76] |
| fraction bits | n-12 .. n-5 | n-12 .. n-5 (same) |
| relative precision | constant across the whole range | sawtooth within each binade |
| mul / div / sqrt / pow | integer add/sub/shift on `l` | significand multiply |
| add / sub | Gaussian-logarithm tables | standard align-and-add |

That is the entire specification difference. Everything below the value map is one
codec.

### Naming

The reference implementation flipped the convention relative to the first paper.
`takum8/16/32/64` are the **linear** ones; `takum_log8/16/32/64` are the logarithmic
ones. Universal's existing `takum<nbits, rbits, bt>` is linear, so it already matches
libtakum -- no rename needed, just an added `takum_log`.

## Assessment: two types, one shared codec

**Recommendation: two distinct class templates, factored over a single shared codec --
not one type parameterized by an encoding flag.**

The strongest evidence is that the author already made this call. libtakum has exactly
one `src/codec.c`, and it implements linear decode by explicitly routing through the
logarithmic path:

> "For linear takums, instead of duplicating all the machinery, we simply treat a
> linear takum as a logarithmic one: We obtain c and m from the logarithmic value l
> and compute f and e, which are then converted to g and h as in Algorithm 5 from the
> takum paper, yielding the final floating point."

...and yet exposes them as **eight distinct types** with distinct entry points, distinct
constant tables, and explicit cross-conversion functions (`takum16_from_takum_log16`).
Shared implementation, separate types. The hardware codec paper mirrors this: a shared
predecoder/postencoder with separate encoder/decoder modules per variant.

### Why an encoding template parameter is the wrong axis

A `TakumEncoding` non-type parameter looks attractive because the codec is 100% shared
-- but the codec is the small part. The divergence lands everywhere *above* it:

1. **Arithmetic.** These are genuinely different number systems. Log takum: multiply is
   an integer add, sqrt is a shift, both exact; add/sub needs Gaussian-log tables.
   Linear takum: the reverse. When Universal replaces the current double-mediated
   operators with native arithmetic, the two implementations share *zero* code.
   `if constexpr` in every operator is not parameterization; it is two classes wearing
   one name.
2. **mathlib.** ~30 functions in `takum/math/`. For log takum, `log`/`log2`/`ln`/`exp`/
   `pow`/`sqrt`/`cbrt` are near-free and near-exact; for linear takum they are the usual
   polynomial/table implementations. The good implementations have nothing in common.
3. **numeric_limits.** `radix` (2 vs. sqrt(e) -- the log variant is not radix-2 at all),
   `min`/`max`/`epsilon`, `min_exponent`/`max_exponent`, `digits`, rounding character.
   Every member differs.
4. **Quire / fused dot product.** Universal advertises exact dot products. A linear takum
   takes a conventional fixed-point quire over 2^c. A logarithmic takum cannot accumulate
   in the log domain at all -- it needs a different accumulator entirely. This is a
   structural fork, not a flag.
5. **Constants.** libtakum duplicates 100+ constants (`TAKUM32_2_PI` vs
   `TAKUM_LOG32_2_PI`, etc.). The same applies to any `takum/table.hpp` work.

Against that, parameterization buys deduplication of the field-extraction code only --
which the shared-codec factoring gives us anyway, without dragging the divergence into a
single class body.

Universal's own conventions point the same way: `lns` and `cfloat` are already separate
types despite both being "sign + exponent + fraction" bit strings. Linear vs. logarithmic
takum is precisely that same distinction.

## Proposed structure

```
include/sw/universal/number/takum/
  takum_codec.hpp     # shared: dr_to_r, dr_to_c_bias, find_dr, field pack/unpack,
                      #         two's-complement magnitude, min/max_characteristic
  takum_impl.hpp      # takum<nbits, rbits, bt>       -- linear   (existing, unchanged API)
  takum_log_impl.hpp  # takum_log<nbits, rbits, bt>   -- logarithmic
  numeric_limits.hpp  # two specializations
  takum_traits.hpp    # is_takum, is_takum_log, is_any_takum
  math/               # per-variant overloads
```

Keeping `takum<>`'s existing three-parameter signature avoids a breaking change to the
test files under `static/tapered/takum/` and to `tools/ucalc/registry.hpp`.

## Practical notes

- The `rbits` generalization Universal added (the spec fixes `rbits=3`, giving
  r in {0..7} and c in [-255, 254]) is a pure codec-level property. It carries to the
  logarithmic variant unchanged -- another argument for the shared-codec factoring
  rather than duplication.
- Because Universal's takum arithmetic currently routes through `double`
  (`takum_impl.hpp` `operator+=` .. `operator/=`), the near-term cost of adding
  `takum_log` is small: the value map, `numeric_limits`, constants, and manipulators.
  The full divergence only materializes when native arithmetic and the quire land --
  which is exactly why the type boundary should be drawn now, before that code is
  written against a single class.
- The file header of `takum_impl.hpp` originally cited arXiv:2404.18603 as the source
  of the linear encoding. The linear takum is Definition 2 of arXiv:2408.10594;
  2404.18603 defines the logarithmic one. Corrected.

## References

- [Beating Posits at Their Own Game: Takum Arithmetic (arXiv:2404.18603)](https://arxiv.org/abs/2404.18603)
- [Design and Implementation of a Takum Arithmetic Hardware Codec (arXiv:2408.10594)](https://arxiv.org/abs/2408.10594)
- [libtakum C99 reference implementation](https://github.com/takum-arithmetic/libtakum)
- [Spectral Methods via FFTs in Emerging Machine Number Formats (arXiv:2504.21197)](https://arxiv.org/abs/2504.21197)
- [AVX10.2 Support for Takum Arithmetic (arXiv:2503.14067)](https://arxiv.org/abs/2503.14067)
