# Development Session: The Manipulator Placeholders, and What Hid Them

**Date:** 2026-09-20 (into 2026-09-22)
**Branches:** `fix/issue-1556-info-print-stubs` (PR #1581), `fix/issue-1582-manipulator-placeholders` (PR #1584) -- both merged to `main` as `08222937` and `54b97cff`.
**Focus:** Replace the `"TBD"`/`"tbd"` manipulator stubs across the number systems, and fix the wrong values and dead code the work uncovered.
**Status:** Issues #1556 and #1582 closed. 1,929 insertions across 22 files.

## Session Overview

Two issues, one shape. `info_print(x, printPrecision)` returned the literal string `"TBD"` in
nine number systems, ignoring both of its arguments. Fixing that surfaced a second layer --
`to_hex`, `hex_print`, `pretty_print` and `color_print` were the same stub in `efloat` and
`ereal` -- and, underneath both, a set of defects that were not placeholders at all: a decoded
field that was wrong for 224 of 256 encodings, a constant printed as if it were data, and a
reporting helper that did not compile for 34 of the library's own 39 types.

Every one of these compiled. That is the thread running through the session: **the defects were
all things a build could not see, and in three cases things the existing tests actively
certified as working.**

| | #1556 / PR #1581 | #1582 / PR #1584 |
|---|---|---|
| Placeholders replaced | 10 (`info_print` x9 + native) | 8 (`to_hex`/`hex_print`/`pretty_print`/`color_print` x2) |
| Wrong values fixed | -- | `dbns::scale()`, `components(dbns)`, `to_triple(dbns)` |
| Latent defects repaired | `components(efloat)`, `to_binary(efloat)`, `to_binary(ereal)` | `ReportFormats`, `elastic/ereal/api/api.cpp` |
| New test suites | 1 | 4 |

## The assertion that does not work, and the one that does

The manipulator surface suites added in #1551 for `bfloat16` and `areal` instantiate every
renderer and assert the result is non-empty:

```cpp
for (const Named& r : rendered) {
    fails += expect_true(!r.text.empty(), r.what, reportTestCases);
}
```

`areal`'s suite listed `info_print` in that array and passed, continuously, while `info_print`
returned `"TBD"`. **`"TBD"` is not empty.** The suite proved the function compiled and proved
nothing else, and the issue says so in as many words: *"`"TBD"` is non-empty, which is exactly
how these went unnoticed."*

The replacement assertion is that **different values must render differently**:

```cpp
fails += expect_true(info_print(a) != info_print(b),
    label + ": info_print distinguishes different values", reportTestCases);
```

No constant can satisfy it. It requires no per-type knowledge of what any given renderer is
supposed to emit, so one helper covers ten types that share no code. It is also the check that
catches the *next* stub, whatever shape it takes, because a placeholder is by definition
value-independent.

Two supporting assertions carry the rest: the text is not a placeholder (`"TBD"`/`"tbd"`
anywhere is a failure), and it has the shape the implemented types established
(`raw: ... : value ...`). And the suite was verified to *fail*: reverting `areal`'s
implementation to its stub produces

```text
    FAIL areal<16,5>: info_print is not a placeholder
    FAIL areal<16,5>: info_print reports the raw encoding
    FAIL areal<16,5>: info_print reports a value
    FAIL areal<16,5>: info_print distinguishes different values
         areal<16,5> -> TBD
```

A test that passes whether or not the bug is present is worth nothing; this was checked rather
than assumed, for `areal`'s stub and again for `dbns::scale()`.

## The layering constraint shapes every implementation

posit's `info_print` lives in `posit/iostream.hpp`, not in `manipulators.hpp`, and the comment
above it says why: *"pretty_print and info_print stream the posit itself, so they need
`operator<<`."* Under #1334 the manipulator layer must not depend on the stream layer.

All nine stubs live in `manipulators.hpp`, so none of them may render by streaming the value.
Each takes its value from whatever exact-enough converter is reachable **in its own layer**:

| type | value source | why |
|---|---|---|
| `cfloat` | `blocktriple::to_string(printPrecision, ...)` | what `operator<<` itself does; exact past a double's range |
| `integer` | `convert_to_decimal_string()` | lives in `manipulators.hpp`; exact at any width |
| `efloat` | core's `to_string(v, precision, ...)` | core, so no stream dependency |
| `ereal` | `s << p` | this layer already includes `iostream.hpp`, as `dd`/`qd` do |
| `fixpnt` | `double(v)` | `convert_to_decimal_string` is in `iostream.hpp`; documented as a rounding |
| others | `double(v)` | what `operator<<` renders anyway (`dbns/manipulators.hpp:57-60` says so) |

`fixpnt` is the one that loses information, and the comment records that rather than hiding it:
a fixpnt wider than a double's significand reports a rounded value field, which is why the exact
encoding is on the same line.

## dbns::scale(): the type contradicted itself

`dbns::scale()` computed `e0 + e1*log2of3`. A dbns encodes `(-1)^s * base0^e0 * base1^e1` with
`base0 = 0.5`, so the base-0.5 exponent contributes **negatively**.

What made this decisive was not the measurement but that **the rest of the type already had it
right**. `operator<` carries the semantics in a comment and in code:

```cpp
// Total ordering on dbns sign-magnitude (-1)^s * 0.5^a * 3^b without
// going through to_ieee754 ...
// Compare in the log domain: M(x) = -a + b * log2of3 strictly orders
const double ma = -static_cast<double>(lhs.extractExponent(0))
                + static_cast<double>(lhs.extractExponent(1)) * log2of3;
```

and `convert_ieee754` states it outright -- *"in our representation we have `0.5^a * 3^b`, which
would be equivalent to `a` being negative"* -- and only accepts `a <= 0`, storing its magnitude.
Three independent statements of the encoding, and `scale()` disagreed with all of them.

Two further defects lived in the same four lines. Zero returned `7`. And `static_cast<int>`
truncates toward zero where a binary scale must **floor**, so every value in `(0.5, 1)` reported
`0` instead of `-1`. The floor is done by hand rather than with `std::floor`, which is not
`constexpr` before C++23.

The blast radius was far wider than the issue's four sample values suggested. Sweeping all 256
encodings of `dbns<8,3>` against an independent `floor(log2(|v|))` oracle, computed from the
decoded double rather than from the implementation:

```text
dbns manipulators   scale vs floor(log2) dbns<8,3> FAIL 224 failed test cases
```

**224 of 256.** Writing an issue from a handful of hand-picked values understates it; the
exhaustive sweep is what turned "wrong for 1.5 and 0.5" into a number.

### And then the review found the overflow

CodeRabbit flagged that `dbns<34,1,uint64_t>` gives the second base a 32-bit exponent field, so
`e1` reaches `UINT32_MAX` and the scale lands near `6.81e9` -- past `INT_MAX`, where a
float-to-int conversion is undefined behaviour. Reproduced before changing anything:

```text
nbits=34 fbbits=1 sbbits=32
e0=0  e1=4294967295
log2v=6.80736e+09  INT_MAX=2147483647  exceeds=1
scale()=-2147483648        <- now 6807362104
```

Correct, and predating the branch -- the original `scale()` had the same narrowing cast. Fixed
here because this PR rewrote the function. `int64_t` is *sufficient* rather than merely larger:
both exponents come back from `extractExponent` as `uint32_t`, so the result is bounded by about
`+/-6.81e9` for any configuration. It also brings `dbns` in line with `takum::scale()` and
`efloat::scale()`, which already return `int64_t` for the same reason.

## ReportFormats: the issue's own framing was wrong

#1582 recorded the last bullet as "roughly two dozen number systems have no `info_print` at
all", implying the fix was to add the missing ones. Surveying the whole of
`include/sw/universal/number/` against the eight renderers `ReportFormats` calls says otherwise:

| renderer | number systems missing it (of 39) |
|---|---|
| `type_field` | 30 |
| `pretty_print` | 25 |
| `info_print` | 25 |
| `to_triple` | 22 |
| `to_hex` | 19 |

**Exactly five types provide all eight**: `cfloat`, `dbns`, `fixpnt`, `integer`, `lns`. Even
`posit` fails, on two counts:

```text
error: no matching function for call to 'type_field(sw::universal::posit<16, 1>&)'
error: no matching function for call to 'to_hex(sw::universal::posit<16, 1>&)'
```

So adding `info_print` everywhere would not have made `ReportFormats` compile; filling the
surface means well over a hundred functions. Detecting each renderer costs nothing and makes it
work for all 39 today:

```cpp
if constexpr (requires { type_field(a); }) std::cout << type_field(a) << '\n';
```

A second claim in the issue also needed retracting: `ReportFormats` was described as dead code.
The **template** was never instantiated, which is how a helper broken for 34 types survived --
but `test_suite.hpp` includes the header, so the rewrite is compiled by **every test in the
tree**. That is why the change was validated against the full CI_LITE suite (496 tests) rather
than only the types it names.

## Challenges & Solutions

### MSVC rejects a literal divide-by-zero

The first PR's new suite built the infinity case as `1.0 / 0.0`. gcc and clang fold that to
infinity under IEEE semantics; **MSVC rejects it at compile time**:

```text
test_info_print_surface.cpp(201): error C2124: divide or mod by zero
```

Ten platforms built it clean, including both local compilers. Only the full CI tier caught it,
which is the argument for the draft-to-ready gate in one line. Fixed by taking infinity and NaN
from `std::numeric_limits`, and the two adjacent `cfloat` cases were moved off the `<cmath>`
`NAN`/`INFINITY` macros so the block reads one way.

### A vacuous assertion, caught in review

The `printPrecision` check compared only the *lengths* of the precision-3 and precision-17
renderings:

```cpp
expect_true(terse.size() <= verbose.size(), ...)
```

That holds trivially when the two are identical -- which is exactly what an implementation that
ignores `printPrecision` produces. The assertion could not fail for the bug it existed to catch.
Now the strings must differ, with the length comparison retained as a second invariant. Before
tightening it, all six subjects were checked to genuinely differ at 3 versus 17, because a strict
assertion that some type satisfies only by luck is a false failure waiting to happen.

### An empty-limb guard for a state that could not be reached

Review flagged that `to_binary(ereal)` computes `limbs().size() - 1` without checking for an
empty vector. Probing every cancellation path -- `a-a`, `a+(-a)` on a genuine two-limb
expansion, `a*0`, default construction -- each canonicalised to a single zero limb, so the state
was **not reproducible through the public API**, and the reply said so.

The guard went in anyway, on evidence rather than on the claim:
`renormalize_expansion({3.0, -3.0})` measurably returns an empty vector, and
`expansion_sum_normalized` returns its result *without* the canonical-zero push that
`expansion_product` has, so nothing enforces the invariant at the boundary into `_limb`. And the
type's own code guards `_limb.empty()` in nine accessors, with `to_string` documenting it
outright: *"renormalize_expansion can prune all components to empty."* `to_binary` was the only
one without the guard. No regression test accompanies it -- fabricating a state the API cannot
produce would assert a shape nobody can reach.

### A style nit that looked wrong and was right

Review flagged two spaces before `const` on `scale()`. The surrounding block uses deliberate
column alignment, so the instinct was to dismiss it. Measuring settled it:

```text
col=26   constexpr bool isneg()  const noexcept
col=26   constexpr bool sign()   const noexcept
col=29   constexpr int64_t scale()  const noexcept     <- mine
```

The neighbours align `const` at column 26; the line lands at 29 because `int64_t` is wider than
`bool`. **Widening the return type had already broken the alignment the padding existed to
serve.** Single space is correct. The prescribed remedy -- run clang-format on the header -- was
declined with reasons: the header is not clang-format-clean, formatting it would collapse the
one-line accessor bodies and strip the manual alignment throughout, and clang-format is not
enforced in CI (the `.clang-format` entries in the workflows are `paths:` filters that trigger
runs, not a format gate).

## Testing & Validation

| suite | what it pins |
|---|---|
| `static/utility/test_info_print_surface.cpp` | all ten `info_print` implementations against the anti-placeholder contract, special encodings, `printPrecision` |
| `static/utility/test_report_formats.cpp` | `ReportFormats` instantiated over 12 number systems + natives; `ReportFormatSurface` reports the gap |
| `static/logarithmic/dbns/api/manipulators.cpp` | `scale()` vs an exhaustive `floor(log2(abs(v)))` oracle; the wide-exponent overflow; no phantom fraction |
| `elastic/efloat/api/manipulators.cpp` | efloat's four repaired renderers |
| `elastic/ereal/api/manipulators.cpp` | ereal's four repaired renderers |
| `static/range/areal/api/manipulators.cpp` | tightened: rejects placeholders, requires value-dependence |
| `static/float/bfloat16/api/manipulators.cpp` | same tightening |
| `static/highprecision/dd_cascade/api/manipulators.cpp` | same tightening (guard, not fix -- its `info_print` was real) |

Both PRs passed the full tier: 11 platforms (Linux x64 GCC/Clang, ARM64, POWER64LE, RISC-V64,
macOS x64/ARM64, Android NDK, MinGW, MSVC), ASan, UBSan, Coverage, Clang-Tidy. Locally,
CI_LITE 496/496 under gcc and 214/214 under clang, clean under `-Wall -Wextra -Wpedantic`, with
the dbns overflow case additionally clean under `-fsanitize=undefined,float-cast-overflow`.

## Next Steps

- **`dbns::fraction()`** still exists and still returns `0` for every value. It is no longer
  *printed* as a decoded field, and a comment records why it stays -- generic code that probes
  for `.fraction()` on an arbitrary number type. Whether a double-base encoding should offer the
  accessor at all is unresolved.
- **The `ReportFormats` surface** is now legible rather than fatal, but it is still a surface
  with 30 types missing `type_field` and 25 missing `pretty_print`. `ReportFormatSurface` exists
  to make deciding where to spend that effort possible; no decision has been made.
- **`efloat` and `ereal` remain the least complete manipulator layers.** Their eight renderers
  are real now, but `efloat`'s `components()` was repaired only because a test needed to trust
  it, and neither type has the breadth of `cfloat`'s surface.

## References

- Issues: [#1556](https://github.com/stillwater-sc/universal/issues/1556),
  [#1582](https://github.com/stillwater-sc/universal/issues/1582)
- PRs: [#1581](https://github.com/stillwater-sc/universal/pull/1581),
  [#1584](https://github.com/stillwater-sc/universal/pull/1584)
- Prior art this builds on: [#1551](https://github.com/stillwater-sc/universal/pull/1551)
  (the manipulator surface suite pattern, and the non-empty assertion that let `"TBD"` through),
  [#1554](https://github.com/stillwater-sc/universal/issues/1554) (`to_string(areal)`),
  [#1453](https://github.com/stillwater-sc/universal/issues/1453) (the never-instantiated
  template body), [#1334](https://github.com/stillwater-sc/universal/issues/1334) (the
  core/manipulators/iostream layering that constrains every implementation here)
- Merge commits: `08222937`, `54b97cff`

## Appendix

Reproducing the dbns sweep against its oracle:

```bash
cmake -S . -B build_dbns -DCMAKE_BUILD_TYPE=Release -DUNIVERSAL_BUILD_NUMBER_DBNS=ON
cmake --build build_dbns --target dbns_manipulators -j4
./build_dbns/static/logarithmic/dbns/dbns_manipulators
```

Reverting `scale()` to `e0 + e1*log2of3` makes it report 224 failures by encoding and value.

Showing what every type reports, side by side -- both new utility suites have a
`MANUAL_TESTING 1` walk-through:

```bash
# static/utility/test_info_print_surface.cpp   -> one info_print line per number system
# static/utility/test_report_formats.cpp       -> ReportFormats output + per-type surface
```
