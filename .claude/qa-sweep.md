# QA sweep config for Universal

Read by the `/qa-sweep` skill and the `qa-reviewer` agent, which are synced
from [stillwater-sc/claude-plugins](https://github.com/stillwater-sc/claude-plugins/tree/main/plugins/qa-sweep).
This file is Universal's own; edit it here.

## Repository

`stillwater-sc/universal`

## Project

Universal is a C++20 header-only library of custom arithmetic types: posits,
cfloats, fixed-point, logarithmic, multi-component (dd, qd, cascades), block
formats (MX, NV, ZFP), interval and elastic types, and the internal block
storage they share. `CLAUDE.md` describes the layout, the regression levels,
the ASCII-only rule and the compiler flags, and the findings should be judged
against it.

## Build and test

At most `-j4`. Build directories: `build-ci/`, and `build-asan-<n>/` and
`build-ubsan-<n>/` for area `<n>`.

```bash
cmake --fresh -S . -B build-ci -DCMAKE_BUILD_TYPE=Release -DUNIVERSAL_BUILD_CI_LITE=ON \
  -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ \
  -DUNIVERSAL_BUILD_REGRESSION_STRESS=OFF -DUNIVERSAL_BUILD_REGRESSION_LEVEL_4=OFF \
  -DUNIVERSAL_BUILD_REGRESSION_LEVEL_3=OFF -DUNIVERSAL_BUILD_REGRESSION_LEVEL_2=OFF \
  -DUNIVERSAL_BUILD_REGRESSION_LEVEL_1=ON
cmake --build build-ci -j4
ctest --test-dir build-ci -j4 --timeout 600 --output-on-failure
```

This is the fast CI tier's build (`UNIVERSAL_BUILD_CI_LITE`), at regression
level 1, with GCC. Every configure in this file uses `cmake --fresh`, which
discards the build tree's cache: CMake keeps cached settings across
reconfigures, and takes a compiler only on the first configure, so a reused
`build-ci/` that once held a full or `UNIVERSAL_BUILD_CI` build would keep
those options, its compiler and its regression level. The objects stay, so a
rebuild is still incremental. The regression flags then only restate the
defaults: `STRESS` and levels 2-4 off, level 1 on. `--fresh` needs CMake 3.24 or later,
newer than the 3.22 that `CMakeLists.txt` accepts; with an older CMake, report
that as the build step's finding and stop. A test's name is its target's:
`<type>_<stem>`, as in `posit_logarithm`. Build one type's tests by turning on
its `UNIVERSAL_BUILD_NUMBER_*` option, as the **Areas** table does.

## Docs-only paths

`docs/`, `docs-site/`, `papers/`, `joss/`, and Markdown files and images
outside `.claude/`.

## Areas

Each area's **Test selection** is the CMake options that build its tests;
they are what `<selection>` stands for in **Sanitizers**. The tests are the
directories those options add.

| # | Area | Headers | Tests | Test selection |
|---|---|---|---|---|
| 0 | posit | `include/sw/universal/number/posit/` | `static/tapered/posit/` | `-DUNIVERSAL_BUILD_NUMBER_POSITS=ON` |
| 1 | bposit, takum, valid | `number/bposit/`, `takum/`, `valid/` | `static/tapered/{bposit,takum,takum_log,valid}/` | `-DUNIVERSAL_BUILD_NUMBER_BPOSITS=ON -DUNIVERSAL_BUILD_NUMBER_TAKUMS=ON -DUNIVERSAL_BUILD_NUMBER_VALIDS=ON` |
| 2 | cfloat | `number/cfloat/` | `static/float/cfloat/` | `-DUNIVERSAL_BUILD_NUMBER_CFLOATS=ON` |
| 3 | Other floats | `number/bfloat16/`, `dfloat/`, `hfloat/`, `e8m0/` | `static/float/{bfloat16,dfloat,hfloat,e8m0}/` | `-DUNIVERSAL_BUILD_NUMBER_BFLOATS=ON -DUNIVERSAL_BUILD_NUMBER_DFLOATS=ON -DUNIVERSAL_BUILD_NUMBER_HFLOATS=ON -DUNIVERSAL_BUILD_NUMBER_E8M0=ON` |
| 4 | Block formats | `number/microfloat/`, `mxfloat/`, `nvblock/`, `zfpblock/` | `static/block/` | `-DUNIVERSAL_BUILD_NUMBER_MICROFLOATS=ON -DUNIVERSAL_BUILD_NUMBER_MXFLOATS=ON -DUNIVERSAL_BUILD_NUMBER_NVBLOCKS=ON -DUNIVERSAL_BUILD_NUMBER_ZFPBLOCKS=ON` |
| 5 | Fixed-point | `number/fixpnt/`, `dfixpnt/` | `static/fixpnt/` | `-DUNIVERSAL_BUILD_NUMBER_FIXPNTS=ON -DUNIVERSAL_BUILD_NUMBER_DFIXPNTS=ON` |
| 6 | integer and rational | `number/integer/`, `rational/` | `static/integer/`, `static/rational/` | `-DUNIVERSAL_BUILD_NUMBER_INTEGERS=ON -DUNIVERSAL_BUILD_NUMBER_RATIONALS=ON` |
| 7 | Logarithmic | `number/lns/`, `dbns/`, `lindex/` | `static/logarithmic/` | `-DUNIVERSAL_BUILD_NUMBER_LNS=ON -DUNIVERSAL_BUILD_NUMBER_DBNS=ON -DUNIVERSAL_BUILD_NUMBER_LINDEX=ON` |
| 8 | dd and qd | `number/dd/`, `qd/` | `static/highprecision/{dd,qd}/` | `-DUNIVERSAL_BUILD_NUMBER_DOUBLE_DOUBLE=ON -DUNIVERSAL_BUILD_NUMBER_QUAD_DOUBLE=ON` |
| 9 | Cascades | `number/dd_cascade/`, `td_cascade/`, `qd_cascade/`, `internal/floatcascade/` | `static/highprecision/*_cascade/` | `-DUNIVERSAL_BUILD_NUMBER_DD_CASCADE=ON -DUNIVERSAL_BUILD_NUMBER_TD_CASCADE=ON -DUNIVERSAL_BUILD_NUMBER_QD_CASCADE=ON` |
| 10 | Ranges | `number/areal/`, `interval/`, `sorn/` | `static/range/` | `-DUNIVERSAL_BUILD_NUMBER_AREALS=ON -DUNIVERSAL_BUILD_NUMBER_INTERVALS=ON -DUNIVERSAL_BUILD_NUMBER_SORNS=ON` |
| 11 | Elastic integers and rationals | `number/einteger/`, `edecimal/`, `erational/` | `elastic/{einteger,decimal,rational}/` | `-DUNIVERSAL_BUILD_NUMBER_EINTEGERS=ON -DUNIVERSAL_BUILD_NUMBER_EDECIMALS=ON -DUNIVERSAL_BUILD_NUMBER_ERATIONALS=ON` |
| 12 | Elastic reals | `number/efloat/`, `ereal/`, `elreal/` | `elastic/{efloat,ereal,elreal}/` | `-DUNIVERSAL_BUILD_NUMBER_EFLOATS=ON -DUNIVERSAL_BUILD_NUMBER_EREALS=ON -DUNIVERSAL_BUILD_NUMBER_ELREALS=ON` |
| 13 | Internals | `include/sw/universal/internal/` | `internal/` | `-DUNIVERSAL_BUILD_NUMBER_INTERNALS=ON` |
| 14 | Quire, conversions, native, utility | `number/quire/`, `number/convert/`, `include/sw/universal/native/`, `utility/` | `static/{quire,conversions,native,utility}/` | `-DUNIVERSAL_BUILD_NUMBER_QUIRES=ON -DUNIVERSAL_BUILD_NUMBER_CONVERSIONS=ON -DUNIVERSAL_BUILD_NUMBER_NATIVE_TYPES=ON -DUNIVERSAL_BUILD_UTILITY=ON` |

Header paths after row 0 are under `include/sw/universal/` unless they say
otherwise. Not in the rotation: `unum`, `unum2`, `poxel` and `bisection`,
which are experimental.

## Sanitizers

AddressSanitizer and UndefinedBehaviorSanitizer, with Clang, over the area's
tests at regression level 1, in two separate builds as
`.github/workflows/sanitizers.yml` runs them. They can't share one: ASan
compiles at `-O1`, and UBSan needs `-O0` because the optimizer can remove UB
sites before the instrumentation sees them (see the comment above
`UNIVERSAL_ENABLE_ASAN` in `CMakeLists.txt`). Run them one after the other.

```bash
# ASan
cmake --fresh -S . -B build-asan-<n> -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DUNIVERSAL_ENABLE_ASAN=ON \
  -DCMAKE_C_FLAGS_DEBUG="-gline-tables-only" -DCMAKE_CXX_FLAGS_DEBUG="-gline-tables-only" \
  -DUNIVERSAL_BUILD_REGRESSION_STRESS=OFF -DUNIVERSAL_BUILD_REGRESSION_LEVEL_4=OFF \
  -DUNIVERSAL_BUILD_REGRESSION_LEVEL_3=OFF -DUNIVERSAL_BUILD_REGRESSION_LEVEL_2=OFF \
  -DUNIVERSAL_BUILD_REGRESSION_LEVEL_1=ON \
  <selection>
cmake --build build-asan-<n> -j4
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 \
  ctest --test-dir build-asan-<n> -j4 --timeout 600 --output-on-failure

# UBSan
cmake --fresh -S . -B build-ubsan-<n> -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DUNIVERSAL_ENABLE_UBSAN=ON \
  -DCMAKE_C_FLAGS_DEBUG="-gline-tables-only" -DCMAKE_CXX_FLAGS_DEBUG="-gline-tables-only" \
  -DUNIVERSAL_BUILD_REGRESSION_STRESS=OFF -DUNIVERSAL_BUILD_REGRESSION_LEVEL_4=OFF \
  -DUNIVERSAL_BUILD_REGRESSION_LEVEL_3=OFF -DUNIVERSAL_BUILD_REGRESSION_LEVEL_2=OFF \
  -DUNIVERSAL_BUILD_REGRESSION_LEVEL_1=ON \
  <selection>
cmake --build build-ubsan-<n> -j4
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
  ctest --test-dir build-ubsan-<n> -j4 --timeout 600 --output-on-failure
```

`<n>` is the area's number. Separate build directories per area keep one
area's options from carrying over into another's when the sweep runs in a
clone that is not fresh. The first frame to report is the first one inside
`include/sw/`.

## Review focus

- **Encoding boundaries:** the largest and smallest finite values, the
  subnormal range, zero and negative zero, and the special encodings (NaR,
  NaN, infinity) through every conversion and operation. Saturating versus
  overflowing variants of the same type.
- **Rounding:** round-to-nearest-even at the tie, with and without sticky
  bits, especially when a conversion from a native type or between two
  Universal types truncates more than one limb.
- **Limbs:** configurations whose `nbits` is not a multiple of the block
  type's width, and the same configuration with `uint8_t`, `uint16_t`,
  `uint32_t` and `uint64_t` blocks. Shifts by the full width of a type, such
  as a 64-bit shift of a `uint64_t`, are undefined behaviour.
- **Class invariants:** number types must stay trivially constructible (no
  in-class member initializers), `blockbinary` and other temporaries must be
  initialized (Clang doesn't zero the stack), and constructors that call
  `std::frexp`, `std::ldexp` or other math functions must not be `constexpr`.
- **`numeric_limits` and traits:** values that disagree with what the type
  actually represents (`digits`, `digits10`, `min_exponent`, `epsilon`,
  `denorm_min`).
- **Exceptions:** each number system throws its own exception types, and the
  internal blocks throw theirs; a mix-up, or an exception thrown where the
  type should saturate or return NaR, is a bug.
- **Tests:** exhaustive tests that cover only small `nbits`, so larger
  configurations are tested by sampling or not at all; checks that run only at
  `REGRESSION_LEVEL_4`, which CI does not run; reference values computed with
  the same code as the result.
- **ASCII-only** in every in-scope file listed in `CLAUDE.md`.
