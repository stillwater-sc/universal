# Development Session: Header Layering Rollout, and the Bugs It Found

**Date:** 2026-08-27 (into 2026-08-28)
**Branches:** per-issue branches off `main`, all merged: PRs #1416, #1417, #1418, #1419, #1420, #1421, #1423, #1425, #1426, #1427, #1429, #1430, #1431, #1432, #1433, #1435, #1437.
**Focus:** Roll #1334's core/manipulators/iostream layering across the number systems, and fix what the rollout exposed.
**Status:** Epic #1334 at **22 of 38 types layered**; issues #1422 and #1424 closed; #1434 and #1436 filed.

## Session Overview

Epic #1334 gives each number system a `core.hpp` that pulls **zero** of the five I/O-family
headers (`<iostream>`, `<sstream>`, `<iomanip>`, `<ostream>`, `<istream>`), so a compute kernel
does not pay for text formatting it never calls. The session took the rollout from 8 types to 22
and completed Phase 2 groups 1-4 plus most of group 5.

The layering itself was mechanical. What made the session worth writing down is that **the cost
was almost never in the type being split** -- it was in a shared header underneath it -- and that
each group surfaced a class of defect that had nothing to do with layering and that the existing
test suite structurally could not see.

| Group | Types | The thing underneath it |
|---|---|---|
| cfloat | `cfloat` | -- |
| logarithmic | `lns`, `takum`, `takum_log` | -- |
| static + elastic | `fixpnt`, `edecimal`, `rational`, `erational` | `native/manipulators_core.hpp`, `blockdigit` |
| multi-component | `dd`, `qd`, 3 cascades | `numerics/error_free_ops.hpp`, `floatcascade` |
| block formats | `microfloat`, `e8m0`, `mxfloat`, `nvblock`, `zfpblock` | element types must be layered first |
| wrappers | `quire`, `interval`, `faithful` | substrates already clean from Phase N |

## The rule: split the substrate before the type

A type's core cannot reach zero while it includes another type's *umbrella*. Four shared headers
were the binding constraint, and each one paid out across many types at once:

- **`native/manipulators_core.hpp`** (new). `sign()`, `scale()`, `exponent()`, `fraction()` and the
  `fieldBits()` accessors are mask-and-shift on the IEEE encoding, but they shared a header with
  `to_triple()`, `to_hex()` and `color_print()`, which pull four stream headers. `scale()` is called
  from `rational`, `dd`, `qd`, `ereal`, `efloat`, `faithful` and `floatcascade` -- **seven** number
  systems dragging four stream headers in to reach a function that does nothing but mask and shift.
  Same split as `ieee754_core.hpp` out of `ieee754.hpp`. 25,964 lines, 0 I/O.
- **`numerics/error_free_ops.hpp`** included `<iomanip>`, `<string>` and `<sstream>` and used **none
  of them** -- 532 lines of pure error-free-transformation arithmetic carrying three stream headers,
  billed to `dd`, `qd`, all three cascades, `ereal` and `elreal`. 59,835 lines / 4 I/O to 25,240 / 0.
- **`internal/blockdigit`** and **`internal/floatcascade`** both defined `operator<<` as an
  **in-class friend definition**, which pins `<iostream>` into every consumer. Made a declaration
  (`<iosfwd>` suffices) with the definition out-of-line in a new `iostream.hpp`. blockdigit backs
  blockoctal/blockdecimal/blockhexadecimal/positional/rational; floatcascade backs all three cascades.
- **Element types before block formats.** `mxfloat` and `nvblock` included the *umbrellas* of
  `microfloat` and `e8m0`. Their cores now take `microfloat/core.hpp` and `e8m0/core.hpp`.

## The layer contract, as it settled

**`to_string()` and `parse()` stay in the CORE.** This was the main judgement call, made when `dd`
became the first type whose text layer was *member* functions. `parse()` is what
`assign(const std::string&)` calls, so it is arithmetic surface. `to_string()` concatenates a
`std::string` directly -- it never opens a stream; it only *takes* `std::streamsize` and
`std::ios_base` flags, which is what `<ios>` is for. Only free stream operators and `sstream`-based
builders move out.

**For `dd`/`qd`/cascades the dependency runs `manipulators -> iostream`,** the opposite of `fixpnt`,
because their string builders format values *through* `operator<<`. `iostream.hpp` must therefore
include **only** `core.hpp`; including `manipulators.hpp` back makes a cycle that `#pragma once`
merely masks, leaving visibility dependent on include order (caught by review on #1427).

**`<string>`, `<map>` and `<cstdio>` are in-scope for a core** and `<iostream>` is not. `<cstdio>`
backs `fprintf(stderr, ...)`, the Phase 0 idiom adopted so diagnostics need no stream header.

## Four bug classes the rollout exposed

None are layering bugs. All were pre-existing; the split made them visible.

### 1. `color_print` infinite recursion -- a SegFault in CI

`color_print(const qd&)` formats each limb with `color_print(r[i])` on a `double`. The intended
target is the **non-template** `color_print(double)` in `native/ieee754_double.hpp`, which
`ieee754_core.hpp` deliberately omits. Re-pointing the impl removed it from the overload set, the
`double` converted implicitly back to `qd`, and the function called itself until the stack died --
1 test of 894, on both gcc and clang.

**The first fix was wrong and is the lesson.** Binding `color_print<double>(r[i])` explicitly stops
the recursion but selects a *different function* -- the `color_print<Real>` template -- which prints
a different format:

```
color_print(d)          0b0.0111'1111'1111.1000'...     <- non-template, ieee754_double.hpp
color_print<double>(d)  0011111111111000...             <- template, native/manipulators.hpp
```

That would have traded a crash for a silent output change, and no test asserts on `color_print`'s
text. The correct fix is the *include*, not the call site. Affected `qd`, `td_cascade`,
`qd_cascade`; `dd` and `dd_cascade` were safe because they already wrote `color_print<double>`
before the split.

### 2. Moving an `operator<<` produces link errors, not compile errors

Three separate times -- `positional_impl.hpp`, the blockdigit api test, and three floatcascade
consumers -- moving a stream operator out of a header produced `undefined reference` in exactly the
translation units that stream the type. `-fsyntax-only` cannot see this; it needs a build-and-link
pass over every consumer. The third occurrence reached CI because the sweep that would have caught
it had been killed and never flushed its log.

### 3. ODR: header-defined free functions without `inline` (#1424)

`erational` and `edecimal` could not be used from **two translation units**:

```
$ g++ -std=c++20 -Iinclude/sw a.cpp b.cpp     # both #include erational.hpp
multiple definition of `sw::universal::ltrim(std::string&)'
... 68 in total
```

| type | duplicate symbols |
|---|---:|
| `erational` | 68 -> 0 |
| `edecimal` | 12 -> 0 |
| `dfloat` | 1 -> 0 |

`dfloat` is not in the issue -- it came from sweeping all 38 number systems with a two-TU link.
Offenders were located **by symbol, not by pattern**: compile with `-g`, then
`nm -C --defined-only -l` filtered to the strong (non-weak) global text symbols, which *is* the
ODR-violating set. 69 definition sites; `inline` applied to those lines only.

**Why it survived is the important half.** `static/appenv/multifile` is the one test that compiles
several TUs and links them, and it covered eleven types -- but not `edecimal`, `erational` or
`dfloat`. Precisely the three that were broken. Adding them takes the linked set from 12 to 15 TUs.
The guard was then checked to actually guard: removing one `inline` again makes the link fail with
`multiple definition of quotient(edecimal const&, edecimal const&)`, and restoring it links clean.
A regression test nobody has watched fail proves nothing.

### 4. Headers that only worked behind a prior include (#1422)

Every `number/*/table.hpp` used its type, its `to_binary()` and its stream operator with no include
that provides them: cfloat 47 errors as the sole include, areal 8, lns 8, dbns 7, takum 6, all to 0.
Seven headers tree-wide had no include guard at all, now 0. The guards were verified to *work* --
three of the seven still error when double-included, but with counts identical to a single include
(7/7, 22/22, 73/73), so they are guarded and merely not self-contained, which is a different defect
and was left alone.

## Final state

Twenty-two types, every core measured at **0 of 5** I/O-family headers:

| type | core | umbrella | | type | core | umbrella |
|---|---:|---:|---|---|---:|---:|
| `integer` | 48,328 | 103,498 | | `qd` | 73,739 | 86,931 |
| `e8m0` | 43,857 | 61,144 | | `edecimal` | 73,040 | 77,265 |
| `microfloat` | 44,567 | 61,922 | | `dd` | 73,665 | 85,110 |
| `faithful` | 46,983 | 64,058 | | `erational` | 74,434 | 82,739 |
| `takum` | 49,429 | 70,706 | | `lns` | 74,762 | 89,944 |
| `interval` | 55,016 | 71,958 | | `td_cascade` | 75,384 | 87,240 |
| `nvblock` | 55,588 | 73,044 | | `qd_cascade` | 75,469 | 87,347 |
| `zfpblock` | 55,705 | 77,789 | | `dd_cascade` | 75,583 | 87,006 |
| `mxfloat` | 56,069 | 73,614 | | `posit` | 77,430 | 114,007 |
| `rational` | 59,633 | 78,042 | | `fixpnt` | 78,123 | 108,901 |
| `quire` | 72,139 | 81,368 | | `cfloat` | 78,330 | 114,629 |

Substrates, all 0 I/O: `native/manipulators_core` 25,964 -- `numerics/error_free_ops` 25,240 --
`native/ieee754_core` 43,486 -- `internal/blockdigit` 45,062 -- `internal/floatcascade` 66,388.

## Filed, deliberately not fixed

Both are real and both change behaviour or span merged subtrees, so neither belongs inside a
layering refactor:

- **#1434** -- `to_binary(e8m0)` ignores its `nibbleMarker` parameter: `to_binary(v,false)` and
  `to_binary(v,true)` are byte-identical, and `mxfloat`/`nvblock` both forward a flag that is
  dropped. Not a one-liner: the convention in the same subtree is that `.` is the *field* separator
  (unconditional) and `'` is the *nibble* marker, so honouring the flag means choosing both whether
  to emit and which character -- either answer changes default output.
- **#1436** -- seven merged cores (`edecimal`, `fixpnt`, `posit`, `takum`, `rational`, `erational`,
  `qd`) do not define their own `*_THROW_ARITHMETIC_EXCEPTION`. Nothing is broken: `#if` on an
  undefined identifier evaluates to 0, the intended default, and `-Wundef` is not in the project's
  flags. The costs are a `-Wundef` diagnostic and the core's configuration depending on an include
  the epic tells callers they do not need. `quire`'s fix in #1435 is the pattern to copy.

## Process notes worth keeping

**Three generated artifacts were committed by accident** -- an empty `all.tsv` (caught before push),
three table dumps that reached `main` (#1432), and two mandelbrot `.ppm` images that reached `main`
in #1435 (#1437). All three came from running tests in the repo root so `git add -A` swept the
output. `.gitignore` is the backstop; the fix is running tests from a scratch directory.

**Cascade files leaked into #1427.** `git stash push -- <paths>` stashes only *modified* files; the
new untracked cascade headers followed the branch switch and `git add -A` took them. CI passed
because nothing included them -- it was findable only by reading the merge's file list. Checking
that a diff is confined to its subtree is now routine.

**A stacked PR dies when its base branch is deleted.** #1428 was based on the qd branch to get
concurrent CI; merging #1427 with `--delete-branch` closed it, and GitHub will not reopen a PR whose
base is gone. Reopened as #1429 against `main`.

**Sweep hygiene**, all learned the hard way: run every test binary as `timeout N <bin> </dev/null`
(without `</dev/null` a REPL waits forever -- `tools/ucalc` exits 0 with stdin closed and is *not* a
bug; without `timeout`, `education/interactive/expansion_algebra` hangs the whole sweep and *is* a
real pre-existing defect, and the two are indistinguishable unless you do both); the Bash tool caps
at 600 s so longer sweeps must run in the background *and be waited for* before pushing; run one
sweep at a time, since a stale one tests stale code and misleads; never `pkill -f` a pattern that
appears in your own command line, which was every mysterious `exit 144`.

**Always re-verify after a rebase.** Mid-rebase measurements are garbage -- `dd/core.hpp` briefly
measured 6 lines. And rebasing a branch whose commits are already squashed into `main` conflicts;
cherry-picking just the new commits is the way through.

## Next

16 types remain in group 5. Largest: `elreal` 143k, `efloat` 107k, `ereal` 105k, `einteger` 94k,
`bfloat16` 93k. `convert` is already at 0 I/O and needs only confirmation.

Before starting the elastic reals: a long-lived local stash `feat/issue-928-elreal-mccleeary` holds
WIP on `ereal_impl.hpp` and will conflict.
