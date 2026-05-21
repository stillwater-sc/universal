# elreal Oracle Sweep Results

Phase J of the elreal follow-up epic (#903). The Phase G validation
oracle (`check_against_elreal_oracle` / `report_against_elreal_oracle`,
shipped in #900) is extended across every multi-component type in
Universal that exposes a math suite: `dd`, `qd`, `dd_cascade`,
`td_cascade`, `qd_cascade`, and `ereal<N>`.

This document is the aggregated pass/fail report. The per-type sweep
sources live in `elastic/elreal/oracle/*_sweep.cpp`; the common
helper headers are `elastic/elreal/oracle/math_sweep_common.hpp` and
`include/sw/universal/verification/test_suite_elreal_oracle.hpp`.

## Precision ceiling caveat

elreal's lazy refinement currently caps at **depth 1** for arithmetic
and math (see `docs/algorithmic-details/lazy-real-arithmetic.md`). The
helper's effective comparison precision is therefore **double**
(~ 15 decimal digits), regardless of the target type's nominal
precision. The sweep at today's ceiling catches:

- Catastrophic disagreement (wrong sign, wrong magnitude, NaN where a
  finite result is expected, finite where a NaN is expected)
- Algorithmic drift visible at double precision (i.e. the target type
  diverges from std-library agreement in the leading 15 digits)

The sweep at today's ceiling does **not** catch:

- Sub-double-ULP precision drift in the target type (e.g. `qd` losing
  the bottom limb's precision while keeping ~ 32 digits of correctness)

That deeper check lights up automatically when Phase L (#906, depth-2+
arithmetic) and Phase M (#907, depth-2+ math) of the follow-up epic
land. No API changes to the helper or to these sweep tests are
required at that point -- the oracle simply gets sharper.

## Test setup

- **Hardware**: 12th Gen Intel Core i7-12700K, single thread
- **Compilers**: gcc 13.3.0 and clang 18.1.3
- **Build**: `cmake -DUNIVERSAL_BUILD_NUMBER_ELREALS=ON
  -DUNIVERSAL_BUILD_NUMBER_DOUBLE_DOUBLE=ON
  -DUNIVERSAL_BUILD_NUMBER_QUAD_DOUBLE=ON
  -DUNIVERSAL_BUILD_NUMBER_DD_CASCADE=ON
  -DUNIVERSAL_BUILD_NUMBER_TD_CASCADE=ON
  -DUNIVERSAL_BUILD_NUMBER_QD_CASCADE=ON
  -DUNIVERSAL_BUILD_NUMBER_EREALS=ON ..` (Release, `-O3 -DNDEBUG`)

## Coverage map

Each cell shows whether the target type exposes the function (and
whether the sweep exercises it). Empty cells mean the target type
does not expose the function today; "x" means the function is part
of the type's math suite and the sweep exercises it.

| Function   | dd | qd | dd_cascade | td_cascade | qd_cascade | ereal<N> |
|------------|:--:|:--:|:----------:|:----------:|:----------:|:--------:|
| `sqrt`     | x  | x  | x          | x          | x          |          |
| `exp`      | x  | x  | x          | x          | x          |          |
| `exp2`     | x  | x  | x          |            | x          |          |
| `expm1`    | x  | x  | x          |            | x          |          |
| `log`      | x  | x  | x          | x          | x          |          |
| `log2`     | x  | x  | x          |            | x          |          |
| `log10`    | x  | x  | x          |            | x          |          |
| `log1p`    | x  | x  | x          |            | x          |          |
| `sin`      | x  | x  | x          | x          | x          |          |
| `cos`      | x  | x  | x          | x          | x          |          |
| `tan`      | x  | x  | x          | x          | x          |          |
| `asin`     | x  | x  | x          | x          | x          |          |
| `acos`     | x  | x  | x          | x          | x          |          |
| `atan`     | x  | x  | x          | x          | x          |          |
| `sinh`     | x  | x  | x          | x          | x          |          |
| `cosh`     | x  | x  | x          | x          | x          |          |
| `tanh`     | x  | x  | x          | x          | x          |          |
| `asinh`    |    |    |            | x          |            |          |
| `acosh`    |    |    |            | x          |            |          |
| `atanh`    |    |    |            | x          |            |          |
| `pow`      | x  | x  | x          | x          | x          |          |
| `pown`     |    |    |            |            |            | x        |

The td_cascade column is the narrowest -- it does not yet expose
exp2 / expm1 / log2 / log10 / log1p. Worth flagging as a separate
issue if td_cascade is meant to have feature parity with the other
cascade types; out of scope for Phase J.

ereal<N> exposes only `pown` from the math suite today; the rest of
its math support is via `pown` over an integer-valued double exponent
plus user-side arithmetic.

## Results

| Sweep                            | gcc 13.3 | clang 18.1 | Reject-path check |
|----------------------------------|:--------:|:----------:|:-----------------:|
| dd math (17 functions)           | PASS     | PASS       | OK                |
| qd math (17 functions)           | PASS     | PASS       | OK                |
| dd_cascade math (17 functions)   | PASS     | PASS       | OK                |
| td_cascade math (15 functions)   | PASS     | PASS       | OK                |
| qd_cascade math (17 functions)   | PASS     | PASS       | OK                |
| ereal<2> / ereal<4> / ereal<8> pown | PASS  | PASS       | OK                |

"Reject-path check" exercises the helper with a deliberately-wrong
2x-off oracle and verifies the helper *rejects* it. This guards
against the helper silently passing on any input due to loose
tolerance.

## What this means

Cross-implementation agreement at double precision is a non-trivial
bug signal even when the comparison is at double precision rather than
at the target's nominal precision: the two paths through dd / qd / 
cascades on one side and through elreal lazy refinement on the other
share *zero code*. Any wrong-sign / wrong-magnitude / wrong-NaN bug
on either side would diverge them, and the sweep would catch it.

After this phase lands, the elreal oracle is the standard
cross-implementation reference for the math suite across every
multi-component type in Universal. When Phase L and Phase M lift the
depth-1 ceiling, the sweep will sharpen automatically -- the helpers
and the test sources do not need to change.

## Reproducing

```bash
mkdir build && cd build
cmake -DUNIVERSAL_BUILD_NUMBER_ELREALS=ON \
      -DUNIVERSAL_BUILD_NUMBER_DOUBLE_DOUBLE=ON \
      -DUNIVERSAL_BUILD_NUMBER_QUAD_DOUBLE=ON \
      -DUNIVERSAL_BUILD_NUMBER_DD_CASCADE=ON \
      -DUNIVERSAL_BUILD_NUMBER_TD_CASCADE=ON \
      -DUNIVERSAL_BUILD_NUMBER_QD_CASCADE=ON \
      -DUNIVERSAL_BUILD_NUMBER_EREALS=ON ..
make elreal_dd_math_sweep elreal_qd_math_sweep \
     elreal_dd_cascade_math_sweep elreal_td_cascade_math_sweep \
     elreal_qd_cascade_math_sweep elreal_ereal_pown_sweep -j4
for t in elreal_dd_math_sweep elreal_qd_math_sweep \
         elreal_dd_cascade_math_sweep elreal_td_cascade_math_sweep \
         elreal_qd_cascade_math_sweep elreal_ereal_pown_sweep; do
  ./elastic/elreal/$t
done
```

## References

- Phase G helper: `include/sw/universal/verification/test_suite_elreal_oracle.hpp`
- Sweep common: `elastic/elreal/oracle/math_sweep_common.hpp`
- Per-type sweeps: `elastic/elreal/oracle/*_sweep.cpp`
- Algorithmic context: `docs/algorithmic-details/lazy-real-arithmetic.md`
- Follow-up epic: #903
- Phase J issue: #904
