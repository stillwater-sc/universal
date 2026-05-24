# elastic/ereal/api/

This directory mixes two kinds of programs (audited in issue #953). The
distinction matters for CI: a regression test's exit code is meaningful, a
demo's is not (a demo "passes" if it compiles and runs without crashing).

## Regression tests (framework-based; exit code is meaningful)

These assert specific numerical/behavioral properties, count failures, and
return `EXIT_FAILURE` on mismatch. They follow the standard test-suite
reporting convention (`ReportTestSuiteHeader` / `ReportTestSuiteResults`).

| File | Exercises |
|------|-----------|
| `adaptive_simple.cpp` | adaptive-precision growth, basic invariants |
| `adaptive_threshold.cpp` | maxlimbs/threshold behavior |
| `constexpr.cpp` | constexpr construction + `static_assert` checks |
| `ostream_formatting.cpp` | `operator<<` formatting properties |
| `phase2.cpp` | truncate / numerics / fractional comprehensive checks |
| `phase3.cpp` | sqrt / cbrt / hypot root-function checks |
| `progressive_precision.cpp` | precision scaling vs maxlimbs (WIP; marked WILL_FAIL in CMake) |
| `string_parsing.cpp` | `parse()` / `operator>>` over many literal forms |

## Demos (informational; not assertion-driven)

These walk through usage patterns and print results for human inspection.
They intentionally do NOT follow the assertion framework; pass/fail is just
"compiled and ran". Left as-is by design.

| File | Demonstrates |
|------|--------------|
| `accurate_summation.cpp` | accurate summation of many terms |
| `adaptive_check.cpp` | quick adaptive-precision sanity printout |
| `catastrophic_cancellation.cpp` | ereal vs double under cancellation |
| `dot_product.cpp` | exact/accurate dot products |
| `test_negation.cpp` | negation usage walkthrough |

## Excluded

- `api.cpp` -- the canonical API walkthrough; excluded from the #953 audit per
  the parent epic (#944).

All files in this directory are ASCII-only (no Unicode), per the project's
console-portability rule.
