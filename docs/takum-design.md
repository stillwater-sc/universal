# Takum: linear and logarithmic variants -- design assessment

Status: assessment / proposal
Date: 2026-08-11

## Question

The current Universal `takum<nbits, rbits, bt>` implements the *linear* takum. The
takum literature defines both a linear and a logarithmic variant. Do they need to
be two distinct types, or can they be parameterized into one type?

## Sources and definition numbering

Definition numbers differ between the two papers, so every citation below is
source-qualified.

- **[arXiv:2404.18603](https://arxiv.org/abs/2404.18603)** -- Laslo Hunhold,
  "Beating Posits at Their Own Game: Takum Arithmetic" (2024). This paper defines
  **both** variants:
  - **Definition 2** -- the **logarithmic** takum encoding (base sqrt(E)).
  - **Definition 8**, Section 4.7 "Linear Takums" -- the **linear** takum encoding
    (base 2). Algorithms 4 and 5 give the float <-> linear-takum conversions.
- **[arXiv:2408.10594](https://arxiv.org/abs/2408.10594)** -- "Design and
  Implementation of a Takum Arithmetic Hardware Codec". Restates the same two
  encodings as its own **Definition 1** (logarithmic) and **Definition 2** (linear),
  and builds a codec for both.
- **[libtakum](https://github.com/takum-arithmetic/libtakum)** -- the C99 reference
  implementation by the same author.

Throughout this document `E` denotes Euler's number, to keep it distinct from the
linear takum's exponent variable `e`.

## Which variant is "the" takum?

The specification and the reference implementation disagree, and Universal has to
pick a side explicitly. arXiv:2404.18603, Section 4.7, is unambiguous:

> "Implementers are at liberty to adopt either variant for takums, albeit the
> logarithmic significand in Definition 2 is designated as the standard. It is
> incumbent upon implementations to explicitly specify whether they support 'linear
> takums' or 'logarithmic takums'. In the absence of such clarification, the
> logarithmic variant is to be presumed."

libtakum, however, names the **linear** variant `takum8/16/32/64` and the
**logarithmic** variant `takum_log8/16/32/64` -- the opposite default.

Universal's `takum<>` is linear, which matches libtakum's naming but *not* the
specification's stated default. Per the spec's own requirement, we must say which we
implement rather than let the bare name imply it. Recommendation: keep
`takum<>` = linear (it matches the reference implementation, and renaming would break
the existing regression suite and `tools/ucalc`), add `takum_log<>`, and state the
choice prominently in the type documentation and `type_tag`.

## Shared encoding (identical in both variants)

An n-bit string, n >= 12:

```text
[S:1][D:1][R:3][C:r][M or F:p]        r in {0..7},  p = n - r - 5

r := 7 - uint(R)          if D=0,   uint(R)                if D=1
c := -2^(r+1) + 1 + uint(C) if D=0, 2^r - 1 + uint(C)      if D=1     in [-255, 254]
m (or f) := 2^-p * uint(M or F)   in [0,1)

zero: S=0, D=R=C=M=0
NaR:  S=1, D=R=C=M=0
```

Interpreted as a two's-complement signed integer, the encodings of *real* values are
unique (Prop. 3), monotonically ordered (Prop. 4), and negated by integer negation
(Prop. 6) -- and arXiv:2404.18603 Section 4.7 states explicitly that all three
propositions "remain applicable regardless of the significand being linear or
logarithmic". `NaR` is a non-real encoding outside these guarantees: it is the
most-negative integer, and its ordering is set by a separate convention (Definition 7,
NaR total-ordering) rather than falling out of the integer comparison. Negating it is
likewise a no-op.

So: the field layout, the `r`/`c`/`m` decode, the two's-complement properties, and the
zero/NaR encodings are byte-for-byte the same in both variants. That is the entire
shared codec.

## The divergence: the value map

| | logarithmic (2404.18603 Def. 2) | linear (2404.18603 Def. 8) |
|---|---|---|
| trailing field | mantissa `M`, `m`, `p` mantissa bits | fraction `F`, `f`, `p` fraction bits |
| scale | `l := (-1)^S (c + m)`, in `(-255, 255)` | `e := (-1)^S (c + S)`, in `{-255..254}` |
| value | `(-1)^S * sqrt(E)^l` | `[(1 - 3S) + f] * 2^e` |
| base | `sqrt(E)` ~= 1.6487 | 2 |
| dynamic range | `+/-(sqrt(E)^-255, sqrt(E)^255)` ~ `+/-(4.2e-56, 2.4e55)` | `+/-(2^-255, 2^255)` ~ `+/-(1.7e-77, 5.8e76)` |
| trailing-field bits | `p` in `{n-12 .. n-5}` | `p` in `{n-12 .. n-5}` (same) |
| relative spacing | tapered (`p` varies with the regime); uniform *within* a regime | tapered, plus a sawtooth within each binade |
| machine precision | `lambda(p) = sqrt(E)^(2^-p-1) - 1` (Prop. 11) | `epsilon(p)`, with `lambda(p) < (2/3) epsilon(p)` |
| mul / div / sqrt / pow | fixed-point add/sub/shift on `l`; exact before re-encoding rounding | significand multiply |
| add / sub | Gaussian-logarithm tables | standard align-and-add |
| bitwise inversion `1/x` | yes -- integer increment of the bit string (Prop. 7) | **no** |

Two consequences the paper calls out directly (Section 4.7): linear takums "lack a
straightforward bitwise inversion mechanism, as demonstrated in Proposition 7" and
"their machine precision is at least two-thirds inferior to that of logarithmic
takums, as indicated in Proposition 11."

Note on "exact": in the logarithmic domain, multiply/divide/root reduce to fixed-point
add/sub/shift on `l`, which removes the transcendental step. The result still has to
be range-handled and rounded back into `p` bits on re-encoding, so it is exact *before*
output rounding, not unconditionally exact.

## Assessment: two types, one shared codec

**Recommendation: two distinct class templates, factored over a single shared codec --
not one type parameterized by an encoding flag.**

The author already made this call. libtakum has exactly one `src/codec.c`, and it
implements linear decode by routing through the logarithmic path:

> "For linear takums, instead of duplicating all the machinery, we simply treat a
> linear takum as a logarithmic one: We obtain c and m from the logarithmic value l
> and compute f and e, which are then converted to g and h as in Algorithm 5 from the
> takum paper, yielding the final floating point."

...and yet exposes them as **eight distinct types** with distinct entry points, distinct
constant tables, and explicit cross-conversion functions (`takum16_from_takum_log16`).
Shared implementation, separate types. The hardware codec paper mirrors this: a shared
predecoder/postencoder with separate encoder/decoder modules per variant.

### Why an encoding template parameter is the wrong axis

The codec is 100% shared, but it is the small part. The divergence lands above it:

1. **Arithmetic.** These are genuinely different number systems. Logarithmic takum:
   multiply is a fixed-point add, sqrt is a shift, inversion is an integer increment.
   Linear takum: the reverse, and no bitwise inversion at all. When Universal replaces
   the current double-mediated operators with native arithmetic, the two implementations
   share *zero* code. `if constexpr` in every operator is not parameterization; it is
   two classes wearing one name.
2. **mathlib.** ~30 functions in `takum/math/`. For the logarithmic variant,
   `log`/`log2`/`ln`/`exp`/`pow`/`sqrt`/`cbrt` are near-free; for the linear variant
   they are the usual polynomial/table implementations. The good implementations have
   nothing in common.
3. **numeric_limits.** `min`/`max`/`epsilon`, `min_exponent`/`max_exponent`, `digits`,
   and the rounding character all differ. See the `radix` note below.
4. **Quire / fused dot product.** Universal advertises exact dot products. A linear
   takum takes a conventional fixed-point quire over `2^c`. A logarithmic takum cannot
   accumulate in the log domain at all -- it needs a different accumulator entirely.
   This is a structural fork, not a flag.
5. **Constants.** libtakum duplicates 100+ constants (`TAKUM32_2_PI` vs
   `TAKUM_LOG32_2_PI`, etc.). The same applies to any `takum/table.hpp` work.

Against that, parameterization buys deduplication of the field-extraction code only --
which the shared-codec factoring gives us anyway, without dragging the divergence into
a single class body.

Universal precedent: `lns` and `cfloat` are already separate types despite both being
"sign + exponent + fraction" bit strings. Linear vs. logarithmic takum is that same
distinction.

### `numeric_limits<takum_log<...>>::radix`

`std::numeric_limits<T>::radix` is `static constexpr int`, so it cannot express the
logarithmic value base `sqrt(E)`. Do not try. The standard's `radix` is the radix of
the *integer representation* of the significand, and the `takum_log` specialization
should keep it an integer (2, matching the base-2 fixed-point `l` field) with a comment
explaining the distinction. Expose the value base separately as a takum-specific
constant on the class, e.g.:

```cpp
// value base is sqrt(E); numeric_limits::radix is int and cannot express it
static constexpr double value_base = 1.6487212707001281;  // sqrt(E)
```

The same care applies to `min_exponent`/`max_exponent`, which for `takum_log` bound the
characteristic `c`, not a power-of-two exponent.

## Proposed structure

```text
include/sw/universal/number/takum/
  takum_codec.hpp     # shared: dr_to_r, dr_to_c_bias, find_dr, field pack/unpack,
                      #         two's-complement magnitude, min/max_characteristic
  takum_impl.hpp      # takum<nbits, rbits, bt>       -- linear (existing API unchanged)
  takum_log_impl.hpp  # takum_log<nbits, rbits, bt>   -- logarithmic
  numeric_limits.hpp  # two specializations
  takum_traits.hpp    # is_takum, is_takum_log, is_any_takum
  math/               # per-variant overloads
```

Keeping `takum<>`'s existing three-parameter signature avoids a breaking change to the
test files under `static/tapered/takum/` and to `tools/ucalc/registry.hpp`.

## Practical notes

- The `rbits` generalization Universal added (both specifications fix `rbits=3`, giving
  r in {0..7} and c in [-255, 254]) is a pure codec-level property. It carries to the
  logarithmic variant unchanged -- another argument for the shared-codec factoring
  rather than duplication.
- Because Universal's takum arithmetic currently routes through `double`
  (`takum_impl.hpp` `operator+=` .. `operator/=`), the near-term cost of adding
  `takum_log` is small: the value map, `numeric_limits`, constants, and manipulators.
  The full divergence only materializes when native arithmetic and the quire land --
  which is exactly why the type boundary should be drawn now, before that code is
  written against a single class.
- The file header of `takum_impl.hpp` originally cited arXiv:2404.18603 without a
  definition number, implying that paper defines only the linear encoding. It defines
  both (Definition 2 logarithmic, Definition 8 linear). Corrected.

## References

- [Beating Posits at Their Own Game: Takum Arithmetic (arXiv:2404.18603)](https://arxiv.org/abs/2404.18603)
  -- Definition 2 (logarithmic), Definition 8 / Section 4.7 (linear)
- [Design and Implementation of a Takum Arithmetic Hardware Codec (arXiv:2408.10594)](https://arxiv.org/abs/2408.10594)
  -- Definition 1 (logarithmic), Definition 2 (linear)
- [libtakum C99 reference implementation](https://github.com/takum-arithmetic/libtakum)
- [Spectral Methods via FFTs in Emerging Machine Number Formats (arXiv:2504.21197)](https://arxiv.org/abs/2504.21197)
- [Streamlining SIMD ISA Extensions with Takum Arithmetic: A Case Study on Intel AVX10.2 (arXiv:2503.14067)](https://arxiv.org/abs/2503.14067)
