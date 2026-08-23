# Development Session: elreal Precision Without a Host Ceiling

**Date:** 2026-08-18 (into 2026-08-23)
**Branches:** per-issue branches off `main`, all merged: PRs #1346, #1348, #1351, #1352, #1353, #1354, #1356, #1357, #1358, #1359, #1361, #1362, #1365, #1366, #1367, #1368, #1369, #1374, #1375, #1376, #1377, #1379, #1380, #1381, #1382, #1384.
**Focus:** Find out why every elreal host stopped at a fixed number of digits, and remove whatever was stopping it.
**Status:** Complete -- issues #1051, #1061, #1068, #1076, #1176, #1177, #1186, #1187, #1188, #1297, #1302, #1303, #1363, #1364, #1370, #1371, #1373, #1378, #1383 closed; #1372 closed invalid. Releases v4.9.0 through v4.10.1 cut.

## Session Overview

The session opened on #1076, a single gated identity: `sin(asin(x)) == x` capped at ~234 digits
against a 300-digit bar. Three previous passes had concluded it was a conditioning defect in the
sin/cos Maclaurin recurrence. It was not. Nothing in sin/cos changed all session, and the identity
now reaches 321 digits and scales linearly with depth.

What the investigation actually found was that **elreal was carrying each block's scale in the host
significand instead of in the wide `integer<256>` exponent that #1061 added for exactly this
purpose.** That collapsed McCleeary's design -- whose exponents live in `Z` -- back onto the host's
exponent range, and it was the binding constraint on every host, on division, and on the transcendental
suite. Removing it, and then removing the three further caps that were hiding behind it, is the session.

| # | The plateau | What it actually was | Result |
|---|---|---|---|
| #1051 / #1363 | `float` 37 digits, `bfloat16` 33, `half` 20, `double` 307 -- at any depth | EFTs running at the operands' natural scale, driving `v` subnormal | all four unbounded; `double` to 1598 digits at depth 96 |
| #1076 | sin/cos round-trip 234 digits | the above, downstream | 321 digits at depth 20, linear in depth |
| #1371 | division 271 digits (`double`), 22 (`float`) | a target depth computed as `(-min_exponent - 2k)/k` | caller-controlled depth |
| #1373 | division 513 digits (`double`), 62 (`float`) | `infsum` dropped a term when the accumulator cancelled to zero | `float` reaches 1147 digits through division |
| #1377 | single-block division quadratic | `FCL.hs` followed literally, no schoolbook carry | linear; 1.70x on the high-precision suites |
| #1383 | trig and log ~372 ms where `exp` was 9.6 ms | `pi` and `ln2` recomputed from scratch on every call | `log` to ~0.01 ms, `sin`/`cos` to ~5.8 ms |

Two of those were found only because an earlier one had been fixed: #1373 was invisible while #1371
capped the quotient short of it, and #1383 was unmeasurable while the constants were bounded by
refinement floors.

## The one rule: normalise the OPERANDS, not the results

Every part of the ceiling work reduces to a single statement.

An error-free transform run at the operands' natural scale has already lost bits to the subnormal
range by the time it returns. Normalising its *outputs* cannot put them back. `block::normalise()`
rescales `v` into `[1,2)` and folds the scale it was carrying into the wide exponent -- exactly, value
and combined exponent invariant -- so the arithmetic always runs at scale ~1.

Applied at `twoSumRN`, `singleMultHelper`, `twoDivZBCL`, and (in #1366) the three eager
`block_two_mult` call sites the streaming work had missed. The three `min_exponent + 2k` refinement
floors are gone with it.

Three details that are not obvious and cost real time to find:

**The nonadjacent shortcut is `k+1`, not `k`.** Operands already `k+1` apart need no arithmetic --
their exact sum is the pair. At `k`, plain 0-overlap still allows the lower operand up to just under
an ulp, and anything above half an ulp carries, so `RN(a+b) != a` and `twoSumRN` would stop owing its
callers the round-to-nearest decomposition it promises. The same non-overlapping vs nonadjacent
distinction as the Shewchuk COMPRESS step in #1340, last session. It is also the performance fix: the
suite went from 1469s to 17s once it was in.

**`half` needs a bias, and only on addition.** A host needs `2k` binades of room beneath a normalised
operand for an addition residual -- alignment costs `k`, the residual another `k`. `half` has 14 for
`k=11` and is the only host short. `block::eft_scale_bias()` spends its 16 unused binades *above* 1.0
instead, shifting `v` up and `exp` down by the same amount; every other host computes 0 and the code
compiles out. It must **not** be applied to multiplication: a product's exponent is the sum of its
operands', so biasing both doubles it, and on `half` the product overflows to `inf`/`nan`.

**The gate on which hosts get the treatment was itself the last bug.** #1361 excluded `double` on the
theory that a wide-exponent host never approaches its wall, and reported the resulting bit-identical
`double` path as the safety property. It was preserving a bug: `double`'s ceiling was 307 digits, and
`1022 * log10(2) = 307.7`. It was sitting exactly on its wall. What exposed it was an *ordering that
cannot be true* -- normalised `float` reaching 319 digits while unnormalised `double` stopped at 307,
when `double`'s exponent range is eight times wider and the whole thesis is that exponent range sets
the ceiling. The physical argument said so before any further measurement did.

## The through-line: a plateau is a hypothesis, not a measurement

Four times this session, a flat line in a table was read as the type's limit and turned out to be a
defect. It is worth stating as a rule because the reasoning failed the same way each time: **the
plateau was named as its own cause.**

- #1076's three earlier passes reasoned that `sqrt(0.75)` sat at 19 blocks at every depth while
  staying 308-digit accurate, so the plateau *had* to be `double`'s physical `2^-1022` floor --
  something no downstream change could move. The floor was the bug.
- #1371's constant was written as `(-min_exponent - 2*k) / k`, deriving a *target depth* from a host
  property. #1361/#1362 swept out the `min_exponent + 2k` guards; this one survived because it was a
  budget rather than a guard.
- #1373's dropped term was written as defensive dead code, commented "McCleeary's pattern match
  assumes this cannot occur". Zero blocks are legitimate ZBCL blocks and the streaming multiply emits
  an all-zero term whenever an operand block is zero, so it fired routinely -- and Newton's reciprocal
  walked straight into it, because the cancellation in `2 - b*r` emits a *growing* run of zero blocks
  as it converges.
- #1177 was filed to read the characterization tool's "saturation knob". By the time it was worked,
  elreal had no saturation knob, and the tool had been *manufacturing* one: it picked "first knob
  within 95% of the best digits", and for a series still climbing the best **is** the last row, so it
  always fired at `maxDepth`. Every elreal function claimed to saturate at whatever depth you happened
  to sweep to, and so did most of ereal's.

Every one of these fixes deleted a constant derived from the host's exponent range -- which is
precisely what elreal's wide exponent exists to make irrelevant.

The counter-example is worth recording too. **#1372 was mine and was wrong**: I filed "long division
is quadratic on `float`, linear on `double`" off a sweep that used a single divisor (3), which divides
1/7's `double` blocks exactly and its `float` blocks not at all. The cost is quadratic on *both*.
Closed invalid.

## Testing: the suites kept passing against broken code

The most uncomfortable finding of the session, and the one most worth carrying forward. In six
separate places a check passed against an implementation that was demonstrably broken:

- **Multiplication bias overflow (#1366).** 46/46 passed, `half` still converged to 122 digits, and
  the multiply probe still read 320 -- because the `inf`/`nan` blocks fail `is_normalised()` and get
  dropped downstream, so the series limps on through other terms. *A check that passes because the
  broken value never reaches it.*
- **The sqrt block-count guard (#1369).** My first draft demanded a strict increase over depth 16's 16
  blocks. The bug's own 19-block plateau satisfies that. Rewritten to demand proportionality to depth,
  because the bug's signature was a flat line -- clearing 300 digits at a single depth proves nothing.
- **The narrow-host division bias (#1377).** My first attempt to demonstrate the defect *failed*: the
  obvious case, 1/7 divided by 3, passes either way. It took sweeping 80 random pairs to find the 13
  that diverge. I nearly reported a load-bearing fix as defensive.
- **The sparse-expansion guard (#1367).** I wrote a test that skipped short expansions with a silent
  `continue`, computed whether the expansion was actually sparse, and then discarded the answer with
  `(void)sawWideGap;`. It could have reported success having exercised nothing.
- **The characterization sweep (#1357).** I wrapped each cell in `catch (const std::exception&)` and
  labelled anything thrown a narrow-host range limit. `infsum` throws a bare `std::runtime_error`
  reading "non-convergence bug", and `std::bad_alloc` is a `std::exception` too -- every one would
  have been relabelled as a normal measurement.
- **The F2 dot-product generator (#1353).** Chunks anchored 100 bits apart put the 12-chunk row's last
  piece at `2^-1100`, below the smallest subnormal. It rounded to zero, so that row measured an
  11-chunk answer while reporting a 1152-bit span. My exactness check had verified the product at a
  single scale and never at the extremes.

**Mutation-test every guard, in both directions.** Every fix this session carries a regression verified
to *fail* against the code it was written for -- 2042/2046 encodings for the NaN classification, "0
of 125 sparse cases found" for the sparse guard, 940102 allocations for the memoization, x7.70 growth
for the division schedule. Where the test was written first and mutation-tested second, it was wrong
about half the time.

## Where measurement overturned the hypothesis

Beyond the plateaus above:

1. **Newton vs long division was not the trade I expected (#1380).** I predicted long division would
   be uniformly slower. It depends on the **divisor's width**, not on depth: 3.8x slower than Newton
   for a 2-block divisor, 0.92x -- faster -- for a 24-block one. Measuring one axis and generalising
   would have shipped the wrong recommendation. Newton stays the default because a narrow divisor is
   the common case, not because it is uniformly better.
2. **`qd` is exact on the geometric predicates (#1186).** I expected to demonstrate elreal's advantage
   and instead demonstrated that Shewchuk's bound (~2x working precision for `orient2d`, ~4x for
   `incircle`) is met by `qd`'s 4x. The honest framing is that the two are exact for different reasons:
   `qd` because someone did the error analysis and it happened to fit, elreal because it has no budget
   to exceed.
3. **Exactness downstream cannot rebuild information destroyed upstream (#1187).** The intuitive
   reading -- "elreal accumulates exactly, therefore it fixes naive Taylor" -- is wrong. The exact sum
   of `exp(-40)`'s rounded terms agrees with `exp(-40)` to *zero* digits. The benchmark now prints
   that in its own output rather than leaving a reader the flattering version.
4. **The #1364 caveat was mine, and I had put it in the v4.9.0 release notes (#1367).** I claimed
   `agreed_decimal_digits` might credit an expansion's *reach* over its *content*, on the evidence
   that agreeing digits exceeded `blocks*k*log10(2)`. But an expansion is a signed **sum**, not a
   concatenation of bit fields, so `blocks*k` bounds nothing -- and wide gaps are not a narrow-host
   phenomenon anyway (`double` shows 35 of 35 gaps above `k`). My first attempt to check it by
   bit-coverage "confirmed" the caveat; that model was simply inapplicable to a signed sum, and it
   contradicted the exact arithmetic, which is what should have been believed. Published notes corrected.
5. **The v4.10.0 docs asserted a cause I had not measured (#1383).** `elreal-ereal-precision-defaults.md`
   attributed the log/trig cost to series evaluation. It was the constants: `pi_zbcl(8)` alone cost
   371.90 ms against `sin(0.5,8)`'s total of 371.82. Corrected in v4.10.1.

## Performance summary

gcc 13.3, `-O2`/`-O3`, double host unless noted.

| what | before | after |
|---|---|---|
| `double` reach | 307 digits (hard cap) | 1598 at depth 96, linear at 16.1 digits/block |
| `float` reach | 37 digits (hard cap) | 319; 1147 through division |
| `bfloat16` / `half` reach | 33 / 20 (hard caps) | 146+ / 122+, both climbing |
| division by a single block, D=160 | 1682.7 ms | 1.423 ms (1183x) |
| ...growth per doubling of D | x6.34 | x2.02 |
| LEVEL_4 transcendental suite | 3m33.6s | 2m06.0s (1.70x) |
| `sin` / `cos` at depth 8 | ~372 ms | ~5.8 ms |
| `log` at depth 8 | ~374 ms | ~0.01 ms |
| `mul_online` leak | 243 allocations/stream, 2.3 GB over 80k streams | 0 retained, RSS flat |

The division speedup is structural: `singleDiv` divided each dividend block independently to a full
stream and `infSum`ed the D of them, which is `D^2/2` block divisions. Dividing an N-digit number by a
one-digit divisor is linear, and the schoolbook carry works here because the residual of `f_i/g` lands
at the scale of `f_{i+1}` -- so one running remainder suffices. It stays a lazy producer: one quotient
block per pull, verified to 400 blocks.

## Gotchas and process notes

**Verification**

- **Stale scratch binaries produced a false "still hangs" conclusion twice** on #1373 variants. Same
  failure mode as last session's stale benchmark. Rebuild, or check the timestamp, before believing a
  negative result.
- **A ratio of two noisy numbers is noisier than either.** #1377 shipped a scaling guard comparing
  wall-clock cost across a 4x depth span -- chosen *specifically* for machine independence. It failed
  CI at x10.99 and x12.45 against a threshold of 8 and passed on rerun, on two different runners.
  Replaced with **allocations per emitted block**, which is a property of the algorithm rather than the
  hardware: identical on every run, 48 vs 49 for the linear form against 3060 vs 23559 for the
  quadratic one.
- **A local run cannot catch a per-test timeout.** #1362 was merged with ASan/UBSan/Coverage pending,
  on the grounds that a local Debug run had confirmed the 0-overlap invariant -- which was the
  correctness risk, and was confirmed. `main` went red anyway on ctest's 300s limit (#1365). The local
  run had no time limit to hit.
- **Counting allocations requires replacing global `operator new`/`delete`,** which does not belong in
  a shared test translation unit and needs a `-Wmismatched-new-delete` pragma on GCC. Give it its own
  file.

**Working on the repo**

- **I edited `main` instead of the PR branch** during a review fix on #1381 and chased three wrong
  hypotheses before `git show --stat HEAD` showed where the commit had landed. Check the branch first
  when a change appears not to take effect.
- **Releases are automated.** `.github/workflows/release.yml` fires on tag push and builds the notes
  with git-cliff; `gh release create` afterwards returns `422 tag_name already exists`. Edit the
  auto-created release instead. This also explains why every release body is verbatim cliff output --
  it was never a style choice.
- **git-cliff `--unreleased` returns nothing once the tag exists.** Use `--latest` after tagging.
- **`jq` is not available in every shell here.** Monitors built on it silently watched nothing and
  produced one false "MAIN CI FAILED". Use `gh -q` instead.
- **Build load.** One build at a time, `-j4` at most, and check `pgrep -a make` first. Stacking a
  whole-project build with `ctest -j4` and stray probes cornered all 20 cores.

**CI**

- CodeRabbit's formatting finding recurs on space-indented headers. The measured answer differs per
  file: `constants.hpp` is 0 tab-indented lines against 111 space-indented, so a range-only reformat
  would *introduce* mixed indentation; but `applications/precision/` demos are unanimously
  tab-indented, so a new file there should be formatted (#1376 was). Measure the file and its
  neighbourhood rather than applying a blanket answer.
- Regression tests standing alone run all four `REGRESSION_LEVEL`s; a "hang" is usually level 4.

## Follow-on work

- **`half`/`fp16` is not in the characterization sweep.** `fp16` is an alias of `half`
  (`cfloat<16,5>`), and it converges as of #1366 -- the `min_exp + 2k` division floor that used to
  block it was removed with the others in #1361/#1362. What remains is that #1176 swept only
  `{double, float, bfloat16}`, so `half` has no accuracy-vs-time curve alongside them.
- **The design matrix half of #1188** stays blocked: what a narrow block shape *would* reach once its
  series stops degrading cannot be measured without an extended-precision intermediate host. The
  decision matrix, delivered here, already says narrow hosts are dominated rather than merely limited.
- **The cost spread across functions** is now the largest remaining accuracy/performance question for
  the facade. After #1383 the spread is much narrower, but a single global default precision is still a
  poor fit for code that mixes `sqrt` with `sin`.
- **`float`'s division wall clock** is linear in block divisions but x3.2 per doubling in time, against
  `double`'s x2.02, because the running remainder is larger. Worth a separate look.

## References

- McCleeary, *Lazy Floating-Point Expansion Reduction Arithmetic* -- the block representation
  `(v : FpType, exp : Z)`, ZBCL, and section 4.2.6's long division
- Shewchuk, *Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates* --
  `orient2d`/`incircle`, and the nonadjacent-vs-non-overlapping distinction the `k+1` shortcut turns on
- Design notes: `docs/design/elreal-narrow-host-blocks.md` (including the scale-normalised-block
  attempt that did *not* work, and why), `docs/design/elreal-ereal-precision-defaults.md`
- Baseline sweep: `benchmark/accuracy/adaptive/baseline-characterization.txt`
- Demonstration: `applications/precision/elreal/thousand_digit_sqrt.cpp` and its `README.md`
- OCP, *8-bit Floating Point Specification (OFP8)* -- the E4M3 conformance work in #1302

## Appendix: reproducing the measurements

Use a fresh build directory; an existing cache keeps whatever compiler and build type it was
configured with.

```bash
mkdir build_elreal && cd build_elreal
CXX=g++-13 cmake -DCMAKE_BUILD_TYPE=Release -DUNIVERSAL_BUILD_NUMBER_ELREALS=ON ..
make -j4 && ctest -j4
```

The high-precision identity suites are `REGRESSION_LEVEL_4` only, and take minutes rather than seconds:

```bash
CXX=g++-13 cmake -DCMAKE_BUILD_TYPE=Release -DUNIVERSAL_BUILD_NUMBER_ELREALS=ON \
      -DUNIVERSAL_BUILD_REGRESSION_LEVEL_4=ON ..
make -j4 el_math_transcendentals_highprecision
./elastic/elreal/el_math_transcendentals_highprecision
```

The thousand-digit demonstration and the characterization sweep are built but not registered with
ctest (`compile_all("false")`); the demo's `float` host takes ~25s to converge:

```bash
CXX=g++-13 cmake -DCMAKE_BUILD_TYPE=Release -DUNIVERSAL_BUILD_APPLICATIONS=ON \
      -DUNIVERSAL_BUILD_BENCHMARK_ACCURACY=ON ..
make -j4 elreal_demo_thousand_digit_sqrt accuracy_characterize
./applications/precision/elreal/elreal_demo_thousand_digit_sqrt
./benchmark/accuracy/adaptive/accuracy_characterize 8 5    # the sweep behind the defaults
```
