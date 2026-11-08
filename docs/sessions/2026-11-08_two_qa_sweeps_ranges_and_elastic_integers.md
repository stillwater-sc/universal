# Development Session: Two QA Sweeps -- Ranges, and Elastic Integers and Rationals

**Date:** 2026-11-06 to 2026-11-08
**Status:** Complete -- 18 PRs merged, 14 issues closed, both sweeps closed
**Focus:** Backfill the CHANGELOG and gate releases on it; split the QA sweeps #1962 (ranges:
areal, interval, sorn) and #1984 (elastic integers and rationals) into tracking issues and
resolve every one; bring `valid` into the ubox trace.
**Issues closed:** #1962, #1964, #1969, #1970, #1971, #1972, #1973, #1984, #1987, #1988, #1989,
#1990, #1991, #1690
**PRs merged:** #1963, #1965, #1966, #1967, #1974, #1976, #1977, #1978, #1979, #1982, #1983,
#1985, #1992, #1993, #1994, #1995, #1996, #1997
**Filed and left open:** #1986 (sorn `numeric_limits` placeholders)

## Session Overview

The session began by closing out the faithful work. The sqrt test moved its reference from
`long double` to `dd` (#1964, #1965), because `long double` is 53 bits on MSVC and macOS ARM64.
`CHANGELOG.md` had stopped at v5.1.0 while the tree was tagged v5.15.0. It was backfilled from
the release notes (#1966), the two release pages that disagreed were corrected with
`gh release edit`, and `release.yml` now refuses a tag that has no CHANGELOG section (#1967).

The rest was two QA sweeps. Each was read, split into tracking issues grouped by subsystem,
cross-linked to the sweep, and resolved one issue per PR:

| sweep | tracking issues | PRs |
|---|---|---|
| #1962 ranges | #1969 faithful, #1970 areal, #1971 interval, #1972 sorn, #1973 follow-ups | #1974, #1976-#1979, #1982, #1983, #1985 |
| #1984 elastic | #1987 einteger arithmetic, #1988 einteger parse, #1989 edecimal, #1990 erational, #1991 unconfirmed | #1992-#1996 |

Last came #1690, which was older: it asked for `valid` sqrt and the trace accessors. Most of it
had already arrived with the valid overhaul (#1686).

As in the previous session, almost nothing failed loudly:

- **Plausible values:** `double(sorn)` returned 0 and `sorn::operator!=` was always false.
  `areal<a,b>(areal<c,d>)` had an empty body. `edecimal` from a NaN gave a large garbage value.
- **Code that compiled but was wrong:** `erational` `1/2 ++` gave `2/2`, and `x /= x` read its
  own overwritten numerator.
- **Hidden behind compile errors:** einteger `a / 2` did not compile at all, so nobody had
  noticed that the scalar operators were missing.

## Key Decisions

- **The areal ubit is a flag, not an enclosure** (user, #1970). A single tile reports that a
  result is inexact; it does not promise to contain the true value. Enclosure is the job of
  `tile_interval` and `valid`. `digits` is fbits + 1, and `min()` is the smallest normal value.
  This also settled #1991's widening question: a wider target owes the source no enclosure.
- **Keep sorn** (user, #1972) as the foundation for experiments rather than a plug-in type. Its
  defects were fixed, not deprecated. The `numeric_limits` placeholders went to #1986.
- **edecimal `parse("2.0")` stays rejected** (user). #865's rule is that a decimal integer has no
  fractional digits, zeros included.
- **erational from NaN or +/-inf is the 0/0 NaN** (user). erational has no infinity, and
  `isinf()` stays false.
- **`erational::toPair()` saturates** (user). The sign rides on the numerator and the parts clamp
  to INT64_MIN/INT64_MAX, with no wrap and no throw.
- **Library conventions applied without asking.** einteger `>>=` truncates toward zero, as the
  sign-magnitude edecimal does. einteger parse accepts at most one sign, as `integer<>` does.
  Both are documented in `docs/number-systems/einteger.md`.
- **"Unconfirmed" means check first** (#1991). Of four items, one was real: a faithful
  expansion copy took stale heap entries, which was correct but slower. The dbns cast bound
  holds, and its proof is now a comment. interval has no generic `sqrt_enclose` branch. Each
  item was closed with a test or a proof, not with a code change made just in case.

## Implementation Details

- **areal** (#1982, #1983): every conversion that drops bits sets the ubit, including subnormal
  targets and integers above 2^53. The converting constructor goes through a blocktriple, because
  areal has one binade more than IEEE and `areal<64,11>` overflows double. `normalize` read a wide
  subnormal as 1.f in configurations with 61 or more fraction bits.
- **interval** (#1979): the sqrt residual `s*s - v` rounded to zero below `denorm_min`. 1955 of
  the 1999 values `k*denorm_min` lost their root. Below `min() * 2^(2 digits)` the residual is now
  formed after an exact scaling. cfloat decides by exact double promotion, and rounds outward
  where its exponent range leaves double's.
- **sorn** (#1985): comparison, midpoint `to_native`, saturating conversion, NaN as the empty
  set, `SpecificValue`, const operators and division. Multiplication kept an attained zero bound
  open.
- **edecimal** (#1992): one form of zero, NaN/inf convert to 0, and shifts in one
  `insert`/`erase`. The long-division quotient was built with `assign(rbegin, rend)`. On aarch64
  and MinGW gcc 13 flags that with `-Wstringop-overflow`, which breaks the `-Werror`
  noexceptions build. It is now built with `std::reverse`, then moved into the vector base.
- **erational** (#1993): `++`/`--` add one; `/=` reads the right-hand side first. `normalize`
  takes one remainder per Euclid step and clears the sign of zero.
- **einteger** (#1994, #1995):
  - A negative shift count shifts the other way.
  - Conversion to float rounds once: the top `digits` bits, a round bit, a sticky bit, then
    `ldexp`.
  - One constrained template per scalar operator takes any arithmetic type.
  - parse builds into a local and assigns only on success. Decimal parse takes nine digits per
    step: 673 ms to 34 ms at 40000 digits.
- **valid** (#1997): `isnan()`, `lo_key()` and `hi_key()`, a new exact sqrt oracle, and
  `valid<16,2>`/`valid<32,2>` series in `ubox_trace.cpp`.

## Challenges & Solutions

- **A full-tier failure that x86 could not reproduce** (#1992). `-Wstringop-overflow` appeared
  only on ARM64 and MinGW. It reproduced in `docker run --platform linux/arm64 ubuntu:24.04`
  with apt g++ 13.3 under qemu, and with the MinGW cross compiler using the toolchain file's
  `-fno-ipa-icf -mfma`. Both needed the exact CMake flags, `-std=gnu++20 -O3 -DNDEBUG` plus the
  REGRESSION defines.
- **A CI watcher that hung for an hour.** `gh pr checks` exits non-zero whenever any check
  fails, and Codacy always does. A loop that branched on the exit status therefore never saw
  the run finish. The fix captures the output with `|| true` and tests the tab-separated status
  column for `pending`.
- **A fixture that depended on a bug.** erational's `logic.cpp` built a negative zero as
  `0 * -3`. Once arithmetic stopped producing -0, the fixture builds it with `setneg()`.
- **Two PRs in one file.** #1995 merged first and touched the same einteger header as #1994,
  whose CI had run without it. Before merging #1994, the two were merged locally and the 19
  einteger tests run on gcc and clang.
- **Checking that an oracle can fail.** The new valid sqrt oracle ran in 0.07 s, fast enough to
  be suspicious. A copy of the header with the open-end rule broken failed 358 cases, so the
  oracle does discriminate.

## Testing & Validation

- **Each fix:** a test that fails against main's header, checked by putting main's `*_impl.hpp`
  first on the include path.
- **Before merging:** the full CI_LITE suite ran on gcc and clang (577 and 576 tests), along with
  the arm64 noexceptions build where it mattered.
- **New reference tests:** edecimal and erational arithmetic against int64; einteger IEEE
  rounding and scalar operands; interval long double subnormal roots and the Stage-1 sqrt of every
  positive cfloat<16,5> and posit<16,1> encoding.
- **valid sqrt:** checked over every interval of `valid<6,0>`, `<8,1>` and `<10,1>`, and every
  tile of `valid<16,2>`. The ubox trace now asserts that valid and `tile_interval<poxel>` give the
  same box at every step.
- **CI:** every PR iterated on CodeRabbit as a draft, then merged after the full tier passed.
  Codacy and docstring coverage are advisory.

## Next Steps

- #1986: sorn `numeric_limits` members are still placeholders.
- #1630: valid's tile model (pairs of posit+ubit tiles on the projective circle).
- **Trace palette:** the ubox trace page has six colours and six series now; another type
  needs the palette extended.
- **Release:** `[Unreleased]` holds 21 behaviour changes across 20 commits, enough for v5.16.0.

## Appendix

- Headers: `number/areal/areal_impl.hpp`, `number/interval/interval_impl.hpp`,
  `number/sorn/sorn_impl.hpp`, `number/edecimal/edecimal_impl.hpp`,
  `number/erational/erational_impl.hpp`, `number/einteger/einteger_impl.hpp`,
  `number/valid/valid_impl.hpp`, `number/faithful/math/functions/fractional.hpp`,
  `number/dbns/dbns_impl.hpp`.
- New tests: `elastic/decimal/arithmetic/reference.cpp`,
  `elastic/rational/decimal/arithmetic/reference.cpp`,
  `elastic/einteger/arithmetic/scalar_operands.cpp`,
  `static/range/interval/arithmetic/special_values.cpp`,
  `static/range/areal/conversion/ubit_oracle.cpp`, `static/tapered/valid/arithmetic/sqrt.cpp`.
- Docs: `docs/number-systems/{edecimal,erational,einteger}.md`,
  `applications/precision/ubit/README.md`.
