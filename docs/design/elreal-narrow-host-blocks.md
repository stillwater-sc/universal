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

## Proposal: scale-normalised blocks

Maintain `v` in `[1,2)` (or `[1,2)` in magnitude) and carry all scale in the
`integer<256>` `exp` field, which is unbounded by construction.

Then `v` is always normal by construction, `is_normalised()` cannot fail for a
scale reason, and the host's `min_exponent` is never approached. The three
`min_exponent + 2*k` floors become dead code. A narrow host's reach is then
bounded only by the number of blocks one is willing to carry -- that is, by `k`
-- rather than by exponent range.

Expected consequences, stated so they can be falsified:

- `bfloat16` and `float` should reach any target precision `double` reaches,
  needing roughly `53/k` times as many blocks (~7.6x for `bfloat16`, ~2.2x for
  `float`).
- The measured ceilings (33 / 37 / 307) should all lift.
- `double` results should be **bit-identical**, since a wide host never
  approached its floor in the first place. This is the primary regression check.

### Where the work is

| area | change |
|------|--------|
| `block.hpp` | a `normalise()` that splits `v` into `[1,2)` mantissa plus exponent delta, folded into `exp`; apply at block construction and after each EFT result |
| `block_eft.hpp` | renormalise `two_sum` / `two_prod` / `two_div` outputs before returning |
| `divide.hpp`, `online_divide.hpp` | drop `exp_floor` / `host_exp_floor`; the remainder sequence no longer decays toward subnormal |
| `math/constants.hpp` | drop the narrow-host branch of `series_stop_exp`; the target term becomes the only stop |
| `zbcl.hpp` | confirm the 0-overlap comparison is expressed in combined exponents, not raw `v` scales |

### Risks

- **The 0-overlap invariant is stated in terms of block exponents.** If any
  comparison uses `scale_of_v()` rather than `combined_exponent()`,
  renormalisation silently changes its meaning. This is the main correctness
  risk and should be settled by reading before writing.
- **Cost.** A renormalisation per EFT result is a `frexp`/`ldexp` pair on the
  hot path. `double` must not regress; if it does, gate renormalisation to
  `k < 24` hosts, which is where it is needed.
- **Exact-zero and non-finite blocks** have no meaningful mantissa scale and
  must bypass renormalisation.
- **`from_native`** already asserts `is_normalised()`; it would need to
  renormalise rather than reject, which changes an existing contract (#1136).

### Validation

- `double` bit-identity across the existing suites is the gate. If `double`
  moves at all, the change is wrong.
- The dyadic oracle sweep (`elastic/elreal/oracle/sweep.cpp`) extended to
  `bfloat16`, asserting exact `add`/`sub` as it already does for `float`.
- The block-shape study (`benchmark/performance/arithmetic/elreal/performance.cpp`)
  re-run; section B's narrow-host rows are the headline result and section H's
  matrix should change qualitatively.
- A depth sweep showing `bfloat16` past 33 digits is the minimum bar for calling
  this done.

## What this would settle

If it works, the block-shape study can finally answer the question it exists to
ask -- what a narrow block shape costs in blocks and time at a *fixed* precision
target -- instead of reporting that narrow hosts cannot reach the target at all.
That is the missing half of #1188's design matrix, and it does it without
computing anything in a wider type.
