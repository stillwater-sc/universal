# Development Session: The numeric_limits Arc, and the Paths Not Taken

**Date:** 2026-09-24 to 2026-09-25
**Releases:** v5.0.0, v5.1.0
**Issues closed:** #1597, #1599, #1601, #1602, #1603, #1604, #1608
**Issues opened:** #1597, #1599, #1601, #1602, #1603, #1604, #1608
**PRs merged:** #1594, #1595, #1596, #1598, #1600, #1605, #1606, #1607, #1609, #1610

## Session Overview

Two releases and one theme. v5.0.0 cut the accumulated correctness work of the previous
cycle; then a documentation pass over the type inventory turned up a defect in
`numeric_limits`, and pulling that thread produced seven issues and six PRs, all of them the
same shape: a trait that had been approximated rather than computed.

The through-line worth recording is not the arithmetic. It is that **every defect in this
session's own work came from verifying the code path that runs rather than the path that
exists**, and the test suites were green through all of them.

## v5.0.0, and a CHANGELOG five releases deep

The release itself was mechanical -- push an annotated tag, `release.yml` generates notes with
git-cliff and pushes a Docker image. The version was worth checking rather than accepting: 127
commits with **no** `!` or `BREAKING CHANGE` marker, so conventional-commit tooling would have
computed 4.11.0. The major was earned by breadth instead -- 56 behaviour-change trailers across
18 scopes, plus three genuine API removals -- which is a judgement the commits do not declare
on their own.

Stamping the CHANGELOG exposed something worse. `[Unreleased]` held **94 bullets, of which 12
belonged to v5.0.0**. The rest cited PRs from #559 to #1384, shipped across v4.6.0 through
v4.10.1; the section had never been stamped, through five releases. Relabelling it wholesale
would have credited five releases of work to v5.0.0.

Each entry was attributed by mapping its pull request to the earliest tag containing that
commit. Two needed a judgement the mechanical rule got wrong: taking the *highest* PR number
in an entry picks up incidental cross-references rather than the work, so the elreal
host-ceiling entry landed in v4.9.1 on a follow-up though v4.9.0's tag names that feature as
its headline. The check that caught it was comparing placements against the tag messages --
v4.8.9's tag names three headline items, and the three entries filed under it are exactly
those three.

A second body of 1764 lines, a dated engineering log from the v3.88-v3.104 series, was
**deliberately not restructured**. Its headings do not nest consistently and re-parenting 251
entries across irregular levels would risk changing what they claim. The defects were the
false "Unreleased" label and missing attribution, and both were fixable without touching a
line of content.

## The documentation inventory, and what it surfaced

37 number systems ship in v5.0.0. The site published 28, documented three that no longer
exist, and had no page for any of the six adaptive types.

The gap that mattered was subtler. The IEEE-754 aliases -- `quarter` through `octo`, and their
`fpNN` spellings -- existed only in `cfloat.hpp`. The formats most likely to be hardware-backed
were the ones a reader had to reverse-engineer from a six-parameter `cfloat<>`, and the landing
page used `half` and `single` in its quick example **without saying they were predefined
aliases**. A reader saw them appear from nowhere.

Building that page required measuring the field widths rather than deriving them, because the
obvious approach is wrong: printing `quad` or `octo` through a `double` reports `inf`. And
measuring them is what turned up #1597.

## The arc: six PRs, one approximation

`numeric_limits<T>::digits10` was `static_cast<int>(digits / 3.3f)` in twelve specializations.
`3.3` stands in for `1/log10(2) = 3.3219`, and `max_digits10` was `digits10 + 1`, which is not
the relation. The consequence:

| type | `digits10` was | standard |
|---|---|---|
| `single` (binary32) | 7 | **6** |
| `duble` (binary64) | 16 | **15** |
| `integer<64>` | 19 | **18** |
| `lns<32,8>` | 1,271,003 | **2** |

`single` reporting 7 where native `float` reports 6 defeats the entire reason the alias exists.
And `lns<32,8>` was not a rounding quibble: `digits` was derived from the exponent **range**,
so a 32-bit type claimed **4,194,312 significand bits**.

### Three shapes, three formulas, and `is_exact` decides

The first instinct -- sweep the expression -- would have traded one wrong answer for another.
A binary float takes `floor((digits-1) * log10(2))` because a decimal literal must survive a
rounding step. An integer takes `floor(digits * log10(2))` with no `-1`, and `max_digits10 == 0`
because there is no decimal round trip to size. `fixpnt`, whose maximum can sit below 1, needs
its own handling: `fixpnt<8,7>` tops out at 127/128, so the largest representable power of ten
is `10^-1`.

The discriminator was already in the types: `is_exact`. An earlier note on #1601 said the
contract for `fixpnt`/`quire`/`rational` "needs deciding"; it did not. The types declare what
they are.

### What was deliberately left out, and why

Six of the eleven types in #1601 were **not** fixed in the sweep, because their `digits10` is
downstream of a `digits` that is itself wrong. Correcting the formula alone would have produced
a differently wrong number and closed the issue on a false clean bill:

- `lns`/`dbns` -- `digits` is a range, not a width (#1602)
- `bfloat16` -- `digits` omits the implicit bit, so the exact formula on `digits=7` gives
  `digits10 = 1`, **worse** than the accidentally-correct 2 that `digits/3.3` produced (#1603)
- `fixpnt`/`quire`/`rational` -- resolved later once `is_exact` settled it

## The dependents, measured rather than argued

#1608 asked for before/after numbers on `min_exponent`'s consumers rather than a green suite,
on the grounds that `ereal`'s limb budget and `elreal`'s `haveBelow` **degrade silently**. That
was the right demand, and holding to it is the one piece of verification in this session that
worked as intended:

| quantity | main | branch | crosses a boundary? |
|---|---|---|---|
| `bfloat16` `is_expansion_limb` | no | no | -- |
| `haveBelow` (`elreal/block.hpp`) | 129 | 126 | no |
| `shortfall` (`2k - haveBelow`) | -113 | -110 | **no** -- both `<= 0`, `eft_scale_bias()` returns 0 either way |
| `zero_exponent` (`expansion_ops`) | -162 | -159 | no -- bfloat16 is not an expansion limb |
| **`elreal<bfloat16>` sqrt(2)** | 1.4142135623730949 | **bit-identical** | -- |

A passing suite would not have established that. The derived internals moved; nothing crossed
a decision boundary.

## Challenges & Solutions

### Verifying the path that runs, not the path that exists

This produced four separate defects, every one caught by review or by a negative control
rather than by the checks written for it:

1. **Both `exponent10` helpers were inverted in the opposite-sign branch.**
   `decimal_max_exponent10(-125)` returned -37 where `floor(-125 * log10 2)` is -38. Missed
   because every number system has `min_exponent <= 1` and `max_exponent >= 0`, so no
   `numeric_limits` specialization reaches the other branch. "Verified against all four native
   formats" and a sweep over ~6200 exponents both passed while half of each function was wrong.

2. **The `fixpnt` below-1 case.** `decimal_max_exponent10(nbits-1-rbits)` passes **zero** for
   `fixpnt<8,7>`, which no common configuration hits.

3. **An `Octo` guard built on a truncated grep.** `grep | head -12` cut off before the third
   use site, so guarding it on `MANUAL_TESTING` alone broke levels 2 and 3.

4. **An assertion true of the bug as well as the fix.** `!std::isinf(double(max()))` was meant
   to catch `xtndd`'s old shape, but `cfloat<80,11>` carries 68 fraction bits, so *its* maxpos
   also exceeds a double's and also converts to `inf`. The check was correct and inert. What
   separates the shapes is the binade -- 16383 against 1023 -- and that is what it asserts now.

The negative control is what found (1) and (4): building each suite against `main` and requiring
it to **fail**. A test that passes before and after proves nothing.

### The same guard mistake, three times

A helper whose only call site sits inside `#if REGRESSION_LEVEL_2`, with its *definition*
unguarded, compiles unused in the level-1-only configuration CI builds. It happened in
`decimal_digits.cpp`, then `sqrt_precision.cpp`, then `decimal_digits.cpp` again. The rule is
one line: **guard the definition with the same condition as the call site**, and check every
level combination rather than the one CI uses.

### Three measurement artifacts reported as results

Worse than the code defects, because each was announced as a conclusion:

- `exit=1` from a **missing binary** after sending the compiler's stderr to `/dev/null`,
  reported as "the test now FAILS".
- `exit=1` from **`grep -c`'s** status -- it exits 1 on zero matches -- because `$?` sat after a
  command substitution in the same `echo`.
- A CI watcher gating on `.conclusion // "RUNNING"`, which only substitutes on *null*, while
  GitHub reports an in-progress check with `conclusion: ""`. Empty string is truthy to `//`, so
  17 running checks read as finished and the watcher printed "ALL CHECKS DONE".

All three share a shape: **absence of a signal read as presence of the opposite one.**

### Replying to review before pushing the fix

Twice. On #1600 and again on #1610, a reply described a fix that was not yet on the branch;
CodeRabbit read the old head and correctly declined to verify. The rule was written down after
the first occurrence and not followed: **push, then reply.**

## The CI slowdown: a limit that was not the limit

The fast tier had gone from ~6 minutes to ~16. Two hypotheses were wrong before the right one:

1. **3,428 `sccache` entries** looked like the problem by count -- and are 0.38GB. The
   `cache-prune.yml` comment saying "the limit is on bytes, not entry count" is correct.
2. **CodeQL's 6.39GB, 62% of everything stored,** looked like eviction pressure. But the
   eviction ceiling is 20GB and usage was 10.29GB, so nothing was being evicted. Inferring a
   10GB cap from a reservation failure was the error.

The cause was in GitHub's own words, in a warning on every main run since 2026-09-23:

```
Cache reservation failed: You have reached your configured budget,
your cache is now read only to prevent additional charges.
```

A **spend budget**, not the eviction limit. Read-only means restores work and saves do not, so
every run restored the frozen 2026-09-22 generation and **hits stayed pinned at 296 objects**
while the denominator grew with each new test -- 62% decaying to 57%, and 41%/22% on a branch
touching widely-included headers. `cmake.yml` had re-enabled saving for seven more platforms
on the reasoning that "the limit is now 20GB", which was true and still broke.

## Testing & Validation

| suite | gcc | clang |
|---|---|---|
| cfloat (102 tests) | pass | pass |
| ereal + elreal + bfloat16 + microfloat + posit + lns + dbns (181) | pass | -- |
| cfloat + areal + posit + integer + lns + dbns + fixpnt + rational + bfloat16 + microfloat + utility (286) | pass | pass |
| `test_decimal_digits_surface` (27 checks) | pass | pass |

Reference values throughout are hand-computed from the standard definitions and checked against
`log10(2)` at 60-140 significant digits, **not** recomputed with the helper under test -- a test
that reruns the implementation's own formula cannot detect a wrong formula.

## Next Steps

- **The Actions spend budget** needs raising in org Billing. Deleting CodeQL's 6.39GB cut usage
  to 3.80GB but cannot un-spend a period cap; until saves succeed, ccache stays frozen at the
  2026-09-22 generation and the fast tier stays ~16 minutes. The check: a push to main should
  produce a `ccache-linux-x64-gcc-fast-2026-09-25T...` entry.
- **`cache-prune.yml` cannot see SHA-keyed caches.** It groups generations by stripping a
  trailing ISO-8601 timestamp. Nothing keyed by commit will ever be pruned.
- **`min_exponent10`/`max_exponent10` for `lns`/`dbns` above `nbits-1-rbits == 33`** saturate
  because `numeric_limits` types them as `int`. That is the most the standard traits can say.
- **`quire`'s `Traits::qbits` resolves to 0** for `quire<posit<32,2>,30>`, so its `digits` is 0.
  Separate from the digit-count work; unfiled.

## References

- Releases: [v5.0.0](https://github.com/stillwater-sc/universal/releases/tag/v5.0.0),
  [v5.1.0](https://github.com/stillwater-sc/universal/releases/tag/v5.1.0)
- `include/sw/universal/utility/decimal_digits.hpp` -- the four helpers and the split-multiply
- `static/utility/test_decimal_digits_surface.cpp` -- cross-type digit and exponent traits
- `static/float/cfloat/api/decimal_digits.cpp` -- cfloat traits and the x87 shape
- `docs/number-systems/standard-types.md` -- the IEEE-754 aliases
