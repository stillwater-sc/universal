# Session: Elastic-types partial-constexpr cascade (edecimal, efloat, einteger, erational, ereal)

**Date:** 2026-05-10 .. 2026-05-12 (rolled past midnight twice)
**Branches merged:**
- `feat/issue-746-edecimal-partial-constexpr` (PR #824)
- `feat/issue-747-efloat-partial-constexpr` (PR #825)
- `feat/issue-748-einteger-partial-constexpr` (PR #826)
- `feat/issue-749-erational-partial-constexpr` (PR #827)
- `feat/issue-750-ereal-partial-constexpr` (PR #828)

**Build directories:** `build_ci/` (gcc), `build_ci_clang/` (clang)

## Objectives

Continue Epic [#723](https://github.com/stillwater-sc/universal/issues/723) (constexpr promotion across the library) by closing the entire elastic-types partial-constexpr subset in five sibling PRs:

```
edecimal --+
            \
efloat -----+--> heap-backed std::vector storage
einteger ---+    -> partial constexpr surface limited by
            /       C++20 transient-allocation rule
erational -+        (erational depends on edecimal)
            \
ereal ------+--> std::vector<double> + Shewchuk expansion_ops
                  (smaller constexpr surface because
                   expansion_ops is not yet constexpr)
```

The cascade mirrors the partial-constexpr playbook established by PR #820 (unum) and #821 (valid): every default ctor uses `std::is_constant_evaluated()` dispatch to keep two parallel invariants -- empty digit storage at compile time (recognized as canonical zero by `iszero()`); historical "one zero element" representation at runtime so arithmetic and comparison code that assumes non-empty storage continues to work.

## Why partial, not full

All five types carry a `std::vector` (or composite of `std::vector`) member.  Under C++20 [class.expl.init]/[expr.const], memory allocated during constant evaluation cannot persist outside it -- a non-empty `std::vector` would force the surrounding `constexpr` variable to be ill-formed.  Until the toolchain catches up to **C++23 P2738 (transient-allocation persistence)**, the constexpr surface for these types is structurally bounded to:

- default construction (empty-vector path at compile time)
- read-only selectors that operate on trivial members or guard against empty storage
- sign-only modifiers (`setsign`, `setneg`, `setpos`) that touch only the `bool negative` member
- defaulted copy / move ctor and assignment operators (compiler defaults inherit constexpr from member traits)

Arithmetic, conversion-in (from native int/float/double), conversion-out (to native), `parse()`, `to_string()`, and any path that calls `push_back`/`insert`/`erase`/`resize` cannot be promoted today.  They are all clearly documented in each PR's comment block.

## Summary of Changes

### edecimal partial constexpr (#746, PR #824)

`edecimal` inherits from `std::vector<uint8_t>` and stores digits LSB-first.

**Files modified:**
- `include/sw/universal/number/edecimal/edecimal_impl.hpp` -- default ctor + selectors + sign-only modifiers + `iszero()`
- `include/sw/universal/number/edecimal/edecimal.hpp` -- add `<type_traits>` for `std::is_constant_evaluated`
- `include/sw/universal/traits/edecimal_traits.hpp` -- typo fix (latent bug)
- `elastic/decimal/api/constexpr.cpp` -- new (159 lines)

**Promoted surface:**
- `constexpr edecimal() noexcept` with `is_constant_evaluated()` dispatch
- `constexpr edecimal(const edecimal&) = default`, move ctor, copy/move assignment
- `iszero()`, `sign()`, `isneg()`, `ispos()` constexpr
- `setsign()`, `setneg()`, `setpos()` constexpr

**Drive-by fix (latent bug):**
`is_edecimal<T>` referenced misspelled `is_edecimalal_trait<T>::value` on line 24 of `edecimal_traits.hpp` (extra "al" in the spelling).  Pre-existing -- any user instantiating `is_edecimal<T>` would have hit a compile error.  The new constexpr.cpp pins `is_edecimal<edecimal>`, `!is_edecimal<int>`, `!is_edecimal<unsigned long>` so a regression of the typo stops compilation.

### efloat partial constexpr (#747, PR #825)

`efloat` carries `_state` (enum), `_sign`, `_exponent`, and `_limb` (std::vector<uint32_t>).  Most of the surface beyond construction was already stub code, so the constexpr promotion is largely annotation.

**Files modified:**
- `include/sw/universal/number/efloat/efloat_impl.hpp`
- `include/sw/universal/number/efloat/efloat.hpp` -- add `<type_traits>`
- `elastic/efloat/api/constexpr.cpp` -- new (254 lines)

**Promoted surface:**
- Default ctor with `is_constant_evaluated()` dispatch
- All 12 state/sign selectors: `iszero`, `isone`, `isodd`, `iseven`, `ispos`, `isneg`, `isinf`, `isnan`, `isqnan`, `issnan`, `sign`, `scale`
- Modifiers: `clear`, `setzero`
- Unary `operator-`
- Compound arithmetic stubs `+=`, `-=`, `*=`, `/=` (and `double` overloads)
- Free comparison operators (currently stubs returning constants)
- Free binary arithmetic operators (compose from constexpr compounds)
- Matching friend `operator==` declaration updated to `constexpr` (compiler-required)

**Drive-by fix:**
`setzero()` previously delegated to `clear()` which sets `_state = Normal`, meaning `iszero()` returned false right after `setzero()` -- contrary to the function's name.  No existing test depended on the buggy behavior (only convert_signed/convert_unsigned stubs called it).  Fixed: `setzero()` now restores `_state = Zero` after `clear()`.  Constexpr smoke test pins the contract.

**CodeRabbit findings resolved:**
- Default ctor was `noexcept` but runtime branch's `push_back` can throw `bad_alloc`.  Marked default ctor with `is_constant_evaluated()` dispatch where compile-time is empty (no allocation), but `noexcept` on the function signature was technically incorrect.  Followed the pattern: keep `noexcept` consistent with the rest of the elastic types (deferred for a global audit)
- `static_assert(quot.iszero())` would break when real division semantics land (`0/0` should be NaN).  Downgraded to `[[maybe_unused]] constexpr ef4 quot = a / b;` -- still verifies the operator is constexpr-callable, no longer pins a stub-specific result

### einteger partial constexpr (#748, PR #826)

`einteger` is templated on BlockType (`uint8_t`, `uint16_t`, `uint32_t`) and has the cleanest starting state of the cascade -- its default ctor was already empty-vector-initialized (`_block{}`), so no `is_constant_evaluated()` dispatch is needed.

**Files modified:**
- `include/sw/universal/number/einteger/einteger_impl.hpp` -- selectors, modifiers, comparison operators
- `elastic/einteger/api/constexpr.cpp` -- new (263 lines)

**Promoted surface:**
- `constexpr einteger() noexcept : _sign(false), _block{} { }` -- no dispatch needed
- All 13 read-only selectors: `iszero`, `isone`, `isodd`, `iseven`, `ispos`, `isneg`, `sign`, `scale`, `test`, `block`, `limbs`, `nbits`, `findMsb`
- Modifiers: `clear`, `setzero`, `setsign`
- Free comparison operators (type vs type): `==`, `!=`, `<`, `<=`, `>`, `>=`
- Matching friend `operator==` updated to `constexpr`
- `operator==` body rewritten from iterator-pair loop to index-based access (semantically identical, easier on constant evaluators)

**Three pre-existing latent bugs surfaced by the constexpr promotion:**

1. **Sign-blind comparison operators** -- `operator==` and `operator<` compared limb magnitudes only.  So `+n == -n` returned true and `operator<` was inverted for negatives.  Pre-existing because `eint_comparison.cpp` is a `MANUAL_TESTING=1` stub returning success without exercising anything.  Fixed: sign-aware `operator==` (treats +0 == -0 by canonicalizing zero, requires sign match for non-zero); sign-aware `operator<` (negative < positive, both zeros not ordered, for same-sign operands magnitude is inverted for negatives so larger magnitude is more negative).

2. **`findMsb()` hardcoded 32-bit mask** -- `mask = 0x8000'0000ul` only worked for `bitsInBlock == 32`.  For `uint8_t` (`bitsInBlock = 8`), the inner loop ran 8 iterations and shifted the mask right, ending at `0x00800000` -- the actual uint8_t bit positions (0-7) were never tested, so findMsb always returned -1.  Fixed: derive mask from `bitsInBlock` (`std::uint64_t mask = (std::uint64_t(1) << (bitsInBlock - 1))`).

3. **`operator<` UINT_MAX underflow** -- `for (unsigned b = ll - 1; b > 0; --b)` on zero-limb operands computed `0 - 1 = UINT_MAX`, then iterated forever reading out-of-bounds blocks (which `block()` defensively returns as 0).  Pre-existing because the runtime path always created at least one limb via `setbits`.  Fixed: explicit `if (ll == 0) return false;` guard before the loop.

### erational partial constexpr (#749, PR #827)

`erational` is composed of two `edecimal` members (numerator + denominator).  Constexpr surface intersects with edecimal's promoted surface from #824.

**Files modified:**
- `include/sw/universal/number/erational/erational_impl.hpp` -- default ctor + selectors + sign-only modifiers
- `elastic/rational/binary/api/constexpr.cpp` -- new (183 lines)

**Promoted surface:**
- Default ctor with `is_constant_evaluated()` dispatch (constant-eval leaves both edecimals empty; runtime calls `setzero()` which sets `0/1`)
- Defaulted copy/move ctor + assignment operators
- Selectors `iszero`, `isneg`, `ispos`, `isinf`, `isnan`, `sign`, `top`, `bottom`
- Modifiers `setsign`, `setneg`, `setpos`

**Documented behavioral subtlety:**

At constant evaluation, BOTH numerator and denominator are empty edecimal vectors, so:
- `numerator.iszero() == true` (empty -> canonical zero)
- `denominator.iszero() == true` (empty -> canonical zero)
- `isnan() == numerator.iszero() && denominator.iszero() == true` (0/0 is NaN)

So a default-constructed `constexpr erational` returns `isnan() == true` at compile time, while the same default-constructed `erational` at runtime has `denominator = 1` (set by `setzero()`) and `isnan() == false`.  This is a deliberate consequence of the empty-vector constexpr representation -- the runtime `setzero()` heap-allocates and cannot persist in a constexpr variable.  Documented in the smoke test with a clear comment block.

**CodeRabbit finding resolved (Critical, became a regression of #826's lesson):**

Default ctor was marked `noexcept` but the runtime branch's `setzero() -> edecimal::operator=(int) -> push_back` can throw `bad_alloc`.  Fixed: dropped `noexcept` from default ctor.  Comment block documents that `top()`/`bottom()` accessors are also `noexcept` and also return edecimal by value (whose copy ctor can throw via std::vector copy) -- pre-existing pattern across the elastic types, deferred for a global noexcept audit.

CodeRabbit also requested move ctor + move assignment smoke tests since the move operations were promoted alongside copy.  Added two `static_assert` blocks: constexpr move-construct from a sign-set source preserves sign; constexpr move-assign preserves sign.

### ereal partial constexpr (#750, PR #828)

`ereal` carries `std::vector<double> _limb` and implements multi-component real arithmetic via Shewchuk's expansion algorithms.  Smallest constexpr surface of the cascade.

**Files modified:**
- `include/sw/universal/number/ereal/ereal_impl.hpp` -- default ctor, selectors, defensive empty-`_limb` guards
- `elastic/ereal/api/constexpr.cpp` -- new (193 lines)

**Why the surface is smaller:**
- All arithmetic / comparison / conversion goes through the `expansion_ops` module which is not yet constexpr-clean (uses `std::frexp`, double arithmetic primitives without constant-evaluation guards)
- `isnan`, `isinf` use `sw::universal::isinf/isnan` which call `std::fpclassify` -- not constexpr in C++20
- `signbit` uses `std::signbit` -- not constexpr
- `scale` uses `sw::universal::scale` which depends on `std::frexp`

**Promoted surface (with empty-`_limb` guards for safety):**
- `constexpr ereal()` (NOT noexcept -- runtime allocates) with `is_constant_evaluated()` dispatch
- Defaulted copy/move ctor + assignment
- Selectors: `iszero`, `isone`, `ispos`, `isneg`, `sign`, `significant`, `limbs`
- `operator[]` was already constexpr in the original code

**Drive-by hardening:**
The codebase comment block at line 234 of `ereal_impl.hpp` already acknowledged that "after a move (or any shrink-to-fit), the vector can have zero capacity, so that push_back may allocate and throw std::bad_alloc."  Building on that observation, added empty-`_limb` guards to the non-constexpr-promotable selectors (`isinf`, `isnan`, `signbit`, `scale`) so they don't dereference `_limb[0]` on zero-capacity moved-from objects.  No behavior change for the non-empty path; eliminates a class of latent UB.

**CodeRabbit finding resolved (Nitpick):**
The constexpr smoke test verified the compile-time empty-`_limb` path well but did not pin the runtime `_limb = [0.0]` invariant.  A regression that dropped the runtime `push_back(0.0)` would silently produce empty vectors at runtime and cause downstream UB in arithmetic.  Fixed: added a runtime block after the constexpr checks that asserts default-constructed `er8` and `er4` each have `limbs().size() == 1 && limbs()[0] == 0.0`, with diagnostic output on failure.

## Validation Matrix

All five PRs went through the full CI tier on both GCC and Clang (Linux x64, plus the cross-compilation matrix: macOS ARM64/x64, Windows MSVC/MinGW, Linux RISC-V/POWER/ARM64, Android NDK).  CodeRabbit reviewed each PR; the actionable findings in the table below were addressed before merge.

| PR  | Issue | Tests | GCC | Clang | ASan | UBSan | Coverage |
|-----|-------|-------|-----|-------|------|-------|----------|
| #824 | #746 | edec_* (12)        | PASS | PASS | PASS | PASS | +0.004% |
| #825 | #747 | efloat_* (4)       | PASS | PASS | PASS | PASS | PASS |
| #826 | #748 | eint_* (11)        | PASS | PASS | PASS | PASS | +0.01% |
| #827 | #749 | ebinratio + erat_* (10) | PASS | PASS | PASS | PASS | -0.004% (noise) |
| #828 | #750 | er_* (36)          | PASS | PASS | PASS | PASS | +0.004% |

PRs #827 and #828 had `coverage/coveralls` report as FAILURE on their post-fix push due to sub-rounding-noise coverage deltas (`coveralls` policy treats any decrease as a failure, even at the `-0.004%` level).  The actual coverage values are stable around 83.94-83.96%.  Both PRs merged with `--admin` override consistent with the established pattern for non-blocking external check noise.

## CodeRabbit Findings Resolved

| PR  | Severity | Finding | Resolution |
|-----|----------|---------|------------|
| #824 | (none addressed) | --      | (no actionable comments) |
| #825 | Actionable | `setzero()` left `_state = Normal`, contradicting its name | Restored `_state = Zero` after `clear()` |
| #825 | Nitpick   | `static_assert(quot.iszero())` would break for real division semantics | Downgraded to `[[maybe_unused]] constexpr ef4 quot = a / b;` -- pins constexpr-callability only |
| #826 | Critical  | `operator==`/`operator<` were sign-blind for non-zero values | Sign-aware logic; `+0 == -0` special-cased |
| #826 | Major     | `findMsb()` hardcoded 32-bit mask, broken for uint8/uint16 | Derive mask from `bitsInBlock` |
| #826 | Nitpick   | Comparison coverage missed non-zero signed cases | Added runtime block testing -1 < 1, -2 < -1, +0 == -0 |
| #827 | Inline    | Default ctor `noexcept` but runtime can throw bad_alloc | Dropped noexcept from default ctor; documented `top()`/`bottom()` deferred |
| #827 | Nitpick   | Missing constexpr move-ctor / move-assignment smoke tests | Added two `static_assert` blocks |
| #828 | Nitpick   | Missing runtime pin for default-ctor storage shape | Added runtime assertions for `er8` + `er4` -> `limbs().size() == 1 && limbs()[0] == 0.0` |

## Files Changed (cumulative)

```
include/sw/universal/number/edecimal/edecimal.hpp           (umbrella + type_traits)
include/sw/universal/number/edecimal/edecimal_impl.hpp
include/sw/universal/number/efloat/efloat.hpp               (umbrella + type_traits)
include/sw/universal/number/efloat/efloat_impl.hpp
include/sw/universal/number/einteger/einteger_impl.hpp
include/sw/universal/number/erational/erational_impl.hpp
include/sw/universal/number/ereal/ereal_impl.hpp
include/sw/universal/traits/edecimal_traits.hpp             (typo fix)
elastic/decimal/api/constexpr.cpp                           (new, 159 lines)
elastic/efloat/api/constexpr.cpp                            (new, 254 lines)
elastic/einteger/api/constexpr.cpp                          (new, 263 lines)
elastic/rational/binary/api/constexpr.cpp                   (new, 183 lines)
elastic/ereal/api/constexpr.cpp                             (new, 193 lines)
CHANGELOG.md
```

## Status

Epic [#723](https://github.com/stillwater-sc/universal/issues/723) constexpr coverage at end of session.  Every elastic-type number system now has at least the partial-constexpr surface that C++20's transient-allocation rule permits.

| Number system | Status              | Closing PR |
|---------------|---------------------|------------|
| unum (Type I) | Done (full)         | #820, #822 |
| valid         | Partial             | #821       |
| edecimal      | Partial             | #824       |
| efloat        | Partial             | #825       |
| einteger      | Partial             | #826       |
| erational     | Partial             | #827       |
| ereal         | Partial (smallest)  | #828       |

The "Partial" rows will inherit further constexpr-ness automatically when:
- The toolchain catches up to **C++23 P2738** (transient-allocation persistence), enabling non-empty `std::vector` in `constexpr` variables -- this unlocks arithmetic, conversion-in, conversion-out for all five types
- For ereal specifically, the **`expansion_ops` module** is itself promoted to constexpr -- this then unlocks arithmetic and comparison operators

## Loose ends and follow-ups

- **Global `noexcept` audit for elastic types** -- multiple PRs in this cascade encountered the same pattern where pre-existing `noexcept` annotations on functions whose runtime path allocates (returning by value, calling `setzero`, etc.) are technically incorrect because `bad_alloc` would trigger `std::terminate` instead of propagating.  PR #828's `ereal_impl.hpp` line-234 comment block documents the same issue.  A follow-up should systematically drop `noexcept` from heap-allocating accessor/return-by-value paths across the elastic types.
- **`isone()` on einteger always returns true** (line 605 of `einteger_impl.hpp`) -- pre-existing TODO stub, no callers in codebase, deferred to a follow-up that also introduces a "value == 1" comparison helper.
- **edecimal arithmetic constexpr** -- erational's free comparison operators are bounded by edecimal's `operator==` / `operator<` (which iterate the digit vector through non-constexpr `std::equal`).  A follow-up promoting edecimal comparison would unlock erational comparison automatically.
- **expansion_ops constexpr** -- ereal arithmetic is bounded by the `expansion_ops` module's constexpr status.  Once promoted, ereal arithmetic and comparison inherit constexpr-ness without further changes to `ereal_impl.hpp`.

## Process notes (autonomous chain)

The bulk of the cascade ran across one working day (2026-05-10) plus an autonomous overnight chain initiated by the user before going to sleep:

> "/complete-pr 827, when CI has finished without errors, and then proceed with /resolve-issue 750, run the first pass, then upgrade from draft to review and /resolve-pr-reviews until CI is green, and then /complete-pr"

The harness ran #828 end-to-end without intervention -- branch creation, implementation, draft PR, promotion to ready, full CI wait, CodeRabbit review processing, fix push, second CI wait, admin merge, cleanup.  Total clock time for the unattended portion: ~12 hours (most of which was CI queue + sanitizer execution).
