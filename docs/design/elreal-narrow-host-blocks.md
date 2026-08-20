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

## The proposal below was tried and does not work

**Status of this section: refuted by experiment.** Kept because the reason is the
useful part, and because it is the obvious idea -- anyone reading the diagnosis
above will reach for it.

`block::normalise()` was implemented and behaves exactly as specified: it rescales
`v` into `[1,2)`, folds the scale into `exp`, preserves the block's value and its
combined exponent exactly (0 changes over 20,000 random blocks per host), and
lifts subnormal `v` back to normal. The three floors were removed. The `double`
suite stayed green -- 46/46, bit-identical -- which is what the gate below asked
for.

`bfloat16` then aborted on the 0-overlap assertion.

The cause is not the 0-overlap comparison, which was the risk flagged below:
`zero_overlap` compares `exponent()`, and `normalise()` preserves that exactly.
It is **operand alignment**. `twoAdd` brings two blocks to a common scale before
the EFT (`threeAdd.hpp`):

```
auto   e_max = std::max(a.exp, b.exp);
FpType va    = (a.exp == e_max) ? a.v : ldexp_block(a.v, int(a.exp - e_max));
```

That shift is applied to the host value. In the current representation the scale
lives in `v` and the `exp` fields stay close together -- the instrumentation above
shows `min combined exponent` within 1 of the minimum `v` scale, i.e. `exp` is
carrying nearly nothing -- so the alignment shift is small and harmless.
Normalising inverts that: the scale moves into `exp`, the `exp` fields spread
apart by hundreds of binades, and the alignment shift underflows `v` to a
subnormal or to zero. The operand is destroyed before the EFT sees it.

So keeping the scale in `v` is not an oversight. It is what makes alignment
expressible in host arithmetic, and the denormal floors are the price. The two
designs are in tension, and the floors are the cheaper side of it.

A real fix has to attack the alignment, not the storage. The promising direction:
`twoAdd` only needs to align when the operands actually interact. If
`|a.exp - b.exp| >= k` the blocks are already 0-overlap and their sum *is* the
pair `{a, b}` -- no arithmetic, no shift, no underflow. Aligning unconditionally
is what forces every operand onto a common host scale. That is a much smaller
change than restructuring the block, and it is where the next attempt should
start. It is untested.

## Superseded proposal: scale-normalised blocks

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

## What the experiment did settle

The diagnosis in the first half stands: the ceiling is exponent range, not
precision, and it is not the EFTs. What is now also established is that the
ceiling cannot be lifted by moving scale out of `v`, because the multi-block
operations depend on the scale being there.

One independent bug fell out of the attempt. `bfloat16::scale()` read the biased
exponent field and subtracted the bias, which is correct for normals and wrong for
all 254 subnormals: it answered -127 for every one of them, where the true scale
runs from -127 down to -133. `block<FpType>::scale_of_v()` calls it, and the
block's combined exponent is `scale_of_v() + exp`, so any block holding a
subnormal carried a combined exponent up to 6 binades wrong -- and the 0-overlap
accounting built on it was wrong with it. Fixed and now guarded exhaustively over
all 65536 encodings (`static/float/bfloat16/api/scale.cpp`).

## What a working fix would settle

If it works, the block-shape study can finally answer the question it exists to
ask -- what a narrow block shape costs in blocks and time at a *fixed* precision
target -- instead of reporting that narrow hosts cannot reach the target at all.
That is the missing half of #1188's design matrix, and it does it without
computing anything in a wider type.
