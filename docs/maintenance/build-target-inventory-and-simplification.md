# Build-Target Inventory and Simplification Assessment

Status: assessment (2026-07-25)
Scope: `cmake -DUNIVERSAL_BUILD_ALL=ON` on `main` after the linear-algebra
extraction (Epic #1204, removal #1209).

## Why this document exists

The BLAS/linear-algebra extraction (#1204) removed roughly 84 build targets
(about 1198 -> 1114 "projects"). That was a smaller dent than hoped. This
document inventories the *entire* `BUILD_ALL` target surface, categorizes it,
and assesses -- with numbers -- where further reduction is realistically
available and what each option actually costs.

Headline conclusion: **the BLAS extraction captured nearly all of the
"pure-deletion" simplification that exists.** ~75% of the repo's build targets
are intrinsic verification coverage of the number systems, which is the
library's reason to exist. Every remaining large bucket is either blocked by
dependencies or is real coverage that would have to be *consolidated* (a
migration/build-system project), not deleted.

## Total: 1121 targets

Count method: each `add_executable` produced by `compile_all()` corresponds 1:1
to a `.cpp` in a globbed test/app directory. Numbers below are source `.cpp`
counts (the ground truth), cross-checked against the configured target set.

### By tier

| Tier | Targets | Share | Composition |
|------|--------:|------:|-------------|
| Number-system verification | 839 | 75% | `static/` 576 + `elastic/` 170 + `internal/` 93 |
| Experiments / apps (BLAS-era home) | 142 | 13% | `applications/` 88 + `benchmark/` 35 + `mixedprecision/` 19 |
| Support / infra | 75 | 7% | `numeric/` 35 + `tools/` 14 + `c_api/` 14 + `validation/` 8 + `type_hierarchy/` 4 |
| Learning / demo | 52 | 5% | `education/` 37 + `playground/` 15 |
| root misc | 13 | 1% | top-level `./` |

### static/ (576) by number system

| System | # | System | # |
|--------|--:|--------|--:|
| cfloat (float) | 96 | qd | 20 |
| posit1 (legacy tapered) | 58 | td_cascade | 19 |
| binary/blockbin | 54 | areal | 22 |
| lns (logarithmic) | 25/38* | decimal (dfixpnt) | 16 |
| dd | 29 | posit (v2) | 15 |
| dd_cascade | 22 | dfloat / zfpblock | 14 / 14 |
| qd_cascade | 21 | dbns | 13 |
| fixpnt | 40 | hfloat / bfloat16 | 10 / 10 |

*lns family (`logarithmic/`) totals 38 including dbns and variants.

### elastic/ (170)

elreal 46 - ereal 44 - efloat 31 - einteger 13 - edecimal 13 - erational 12 -
unum 11.

### internal/ (93)

blockbinary 17 - blocktriple 15 - expansion 12 - floatcascade 10 -
blocksignificand 9 - constexpr_math 6 - value 5 - remaining 19.

## Why the BLAS extraction only removed ~84

The linear-algebra layer lived entirely in `applications/*/blas`,
`benchmark/*/blas`, `linalg/`, and a few `mixedprecision/` dirs. Removing it was
correct and complete, but that surface was always a small fraction. The repo's
mass is the per-number-system regression suites (assignment, conversion,
arithmetic, logic, math, special cases), which the extraction never targeted.

## The simplification levers, assessed

### 1. Retire the legacy `posit1` implementation -- BLOCKED (migration project)

There are two live posit implementations: `number/posit/` (176 includers,
"v2") and `number/posit1/` (107 includers, legacy). `posit1` is **not** a
parallel test suite that can be deleted -- it is load-bearing. It is referenced
by **53 files outside its own test tree**, including core library headers:

- `include/sw/universal/number_systems.hpp` (the master umbrella)
- `include/sw/universal/number/valid/valid_impl.hpp` (the `valid` type is built on posit1)
- `include/sw/universal/number/posito/posito_impl.hpp`
- `include/sw/universal/adapters/adapt_integer_and_posit.hpp`
- `c_api/shim/posit/posit_c_api.cpp` (the C API)
- all 11 `education/number/posit/*` examples, plus tools (`float2posit`),
  numeric, playground, and several applications.

Removing posit1 would first require migrating `valid`, `posito`, the C API, the
adapters, education, and `number_systems.hpp` onto `posit` v2 and proving value
parity (regime handling has historically diverged between the two). That is a
dedicated epic, not a cut. Potential payoff once done: the ~58-target `posit1`
suite plus its maintenance burden.

### 2. Consolidate the cascade suites -- real coverage, not duplicates

`dd_cascade` (22), `qd_cascade` (21), `td_cascade` (19) = 62 targets mirror the
`dd`/`qd` category structure, but they exercise a **different backend**
(`floatcascade<>`) than the native `dd`/`qd` types. They are verifying that the
cascade backing reproduces the double/quad-double results -- deleting them loses
that coverage. Note `td_cascade` (triple-double) has **no** native equivalent,
so it is not a duplicate of anything.

Realistic option: parameterize the native and cascade suites over a shared test
body (one `.cpp` templated on the implementation) to cut file/target count while
keeping both backends covered. This reduces targets but is a test-refactor, not
a deletion, and trades target count for a more complex test harness.

### 3. cfloat config sweeps (96 targets) -- gateable, but needs a build change

cfloat is the largest single suite: arithmetic 43, conversion 25, api 10,
math 10, standard 4, complex 2, logic 1, performance 1. The 68
arithmetic+conversion targets are largely config permutations (different
`nbits`/`es`/subnormal/saturation combinations), each a separate `.cpp`. Most
only need to *build* at higher regression intensities.

Caveat that changes the whole picture (see next lever): today these all compile
in every build regardless of regression level.

### 4. REGRESSION_LEVEL does NOT reduce build-target count (build-system gap)

`compile_all()` globs `*.cpp` and calls `add_executable` unconditionally. The
`REGRESSION_LEVEL_{1..4}` macros only change what each test *does at runtime* --
they do not gate which targets are *compiled*. So "raise the default regression
level to build less" does not work as-is.

The highest-leverage structural change available is therefore a **build-system
one**: make `compile_all()` (or the per-suite CMake) gate target *inclusion* by
regression level -- e.g. a default (`LEVEL_1`) build compiles one representative
config per suite, and the full permutation sweep (the bulk of cfloat's 96, the
62 cascade targets, the deep number-system matrices) only materializes at
`LEVEL_2+`. This would cut a default `BUILD_ALL` by several hundred targets with
zero loss of coverage at the levels that run in CI's heavier tiers.

### 5. Small, clean-ish cuts

- `posito` (9 targets): a thin layer, only **1** external includer outside its
  own tests; partially built on `posit1`. Candidate to fold in or drop if not on
  a roadmap.
- `number/positional/`: header present with **0** test targets -- an orphan/
  experimental header to either wire up or remove.

## Recommendation

| Option | Target delta | Cost | Coverage risk |
|--------|-------------:|------|---------------|
| Regression-gated compilation (lever 4) | -several hundred (default build) | build-system change to `compile_all` | none (full set at LEVEL_2+) |
| Cascade test parameterization (lever 2) | ~ -40 | test-harness refactor | low (keeps both backends) |
| posit1 retirement (lever 1) | ~ -58 + upkeep | dependency-migration epic | medium (value parity) |
| posito / positional cleanup (lever 5) | ~ -9 | small | low |

The single best return-on-effort is **lever 4** (regression-gated
compilation): it is the only option that removes hundreds of targets from the
default build without touching test coverage, because it changes *when* targets
build rather than *whether* the coverage exists. The number-system suites are
large because the verification is real; the win is compiling the exhaustive
sweep on demand rather than always.

## Appendix: reproduce this inventory

```bash
# configure the full surface
cmake -DUNIVERSAL_BUILD_ALL=ON <src>

# per-area target counts (source .cpp == targets via compile_all)
for a in static elastic internal applications benchmark mixedprecision \
         education numeric playground tools c_api validation type_hierarchy; do
  printf '%-16s %s\n' "$a" "$(find "$a" -name '*.cpp' | wc -l)"
done

# posit1 external dependency surface (the retirement blocker)
grep -rlE '#include <universal/number/posit1/' --include=*.cpp --include=*.hpp . \
  | grep -vE 'number/posit1/|static/tapered/posit1/'
```
