# bit_cast dependency analysis (issue #710)

## Problem

`#include <sw/universal/number/integer/integer.hpp>` fails on MSVC in
C++20 mode because `utility/bit_cast.hpp` attempts `using std::bit_cast`
without ensuring `<bit>` was actually included.

```
sw\universal\utility\bit_cast.hpp(92,1): error C2039: 'bit_cast': is not a member of 'std'
sw\universal\native\extract_fields.hpp(18,22): error C2039: 'bit_cast': is not a member of 'std'
sw\universal\native\set_fields.hpp(18,23): error C2039: 'bit_cast': is not a member of 'std'
sw\universal\native\attributes.hpp(78,22): error C2039: 'bit_cast': is not a member of 'std'
```

Linux GCC, Linux Clang, macOS Apple Clang, and RISC-V cross-compile all
pass.  Only MSVC on Windows CI (GitHub Actions) fails.

## Root cause

`utility/bit_cast.hpp` line 31 guards `#include <bit>` with two
conditions -- `__has_include(<bit>)` AND `__cplusplus >= 202002L`:

```cpp
#if defined __has_include && __cplusplus >= 202002L
#  if __has_include (<bit>)
#    include <bit>
#  endif
#endif
```

MSVC without the `/Zc:__cplusplus` compiler flag reports
`__cplusplus == 199711L` even in C++20 mode.  Universal's own
CMakeLists.txt sets `/Zc:__cplusplus` (line 557), so Universal's
own CI passes.  But downstream consumers that include Universal
headers with a vanilla MSVC C++20 build (no `/Zc:__cplusplus`)
hit the bug.

The failure sequence:

1. MSVC C++20 mode, no `/Zc:__cplusplus` -- `__cplusplus == 199711L`
2. `<bit>` is NOT included (the `__cplusplus >= 202002L` guard fails)
3. `#include <type_traits>` (line 38) transitively pulls in MSVC's
   `<version>` header which defines `__cpp_lib_bit_cast = 201806L`
4. `#if __cpp_lib_bit_cast` (line 40) is now truthy
5. `BIT_CAST` becomes `using std::bit_cast`
6. Line 92: `BIT_CAST;` expands to `using std::bit_cast;` -- but
   `std::bit_cast` was never declared because `<bit>` was not included
7. Compilation error

The `native/extract_fields.hpp`, `native/set_fields.hpp`, and
`native/attributes.hpp` headers cascade-fail because they use
`std::bit_cast` directly inside `#if BIT_CAST_IS_CONSTEXPR` blocks,
and `BIT_CAST_IS_CONSTEXPR` is `true` (set by the same
`__cpp_lib_bit_cast` branch).

## Bit-cast usage inventory

### Central wrapper: `utility/bit_cast.hpp`

Three-tier fallback:

| Tier | Condition | Mechanism | Constexpr |
|------|-----------|-----------|-----------|
| 1 | `__cpp_lib_bit_cast` | `using std::bit_cast` from `<bit>` | yes |
| 2 | `__has_builtin(__builtin_bit_cast)` | custom template wrapping the builtin | yes |
| 3 | fallback | `non_builtin::bit_cast` via `std::memcpy` | no |

Exposes:

- `sw::bit_cast<To>(from)` -- single entry point, always available
- `BIT_CAST_IS_CONSTEXPR` -- `true` or `false`
- `BIT_CAST_CONSTEXPR` -- expands to `constexpr` or empty

### Direct `std::bit_cast` usage (the fragile pattern)

These use `std::bit_cast` explicitly instead of `sw::bit_cast`,
creating a dependency on `<bit>` being included upstream:

| File | Lines | Calls | Guard |
|------|-------|-------|-------|
| `native/extract_fields.hpp` | 18, 27, 42 | 3 | `#if BIT_CAST_IS_CONSTEXPR` |
| `native/set_fields.hpp` | 18, 21, 25, 28, 39, 50, 72, 85, 91 | 9 | `#if BIT_CAST_IS_CONSTEXPR` |
| `native/attributes.hpp` | 78, 83 | 2 | `#if BIT_CAST_IS_CONSTEXPR` |
| `static/.../float_conversion.cpp` | 45 | 1 | `#if BIT_CAST_IS_CONSTEXPR` |
| `static/.../bit_manipulation.cpp` | 129 | 1 | inside long double block |

### Correct `sw::bit_cast` usage (the robust pattern)

| File | Lines | Calls |
|------|-------|-------|
| `number/interval/manipulators.hpp` | 39, 47 | 2 |

### Independent implementation (unaffected)

| File | Mechanism |
|------|-----------|
| `native/nonconst_bitcast.hpp` | `BitCast<To>(from)` via `std::memmove` |
| `native/manipulators.hpp` | uses `BitCast` from above |

## Fix

### Fix 1 -- `utility/bit_cast.hpp` include guard

Drop the `__cplusplus >= 202002L` pre-check.  `__has_include(<bit>)`
already returns false when the header does not exist:

```cpp
// Before (broken on MSVC without /Zc:__cplusplus):
#if defined __has_include && __cplusplus >= 202002L

// After:
#if defined __has_include
```

### Fix 2 -- replace `std::bit_cast` with `sw::bit_cast`

Replace all 14 direct `std::bit_cast<To, From>(v)` calls in the 3
native/ headers with `sw::bit_cast<To>(v)`.  `sw::bit_cast` is
constexpr when `BIT_CAST_IS_CONSTEXPR` is true, so no behavioral
change.  Accessible from `sw::universal` via enclosing namespace lookup.

Note the template argument change: `std::bit_cast<To, From>(v)` has two
explicit template arguments; `sw::bit_cast<To>(v)` deduces `From`.

### Fix 3 -- test files (consistency)

Replace `std::bit_cast` with `sw::bit_cast` in the 2 test .cpp files.
These build under Universal's own CMake (which sets `/Zc:__cplusplus`),
so they are not broken today, but consistency reduces future risk.

### Not touched

- `nonconst_bitcast.hpp` -- independent `memmove`-based, unaffected
- `interval/manipulators.hpp` -- already uses `sw::bit_cast` (correct)
- The `BIT_CAST` / `BIT_CAST_IS_CONSTEXPR` macro framework -- sound

## Risk

Low.  The `bit_cast.hpp` fallback tiers are correct; only the
`<bit>` include guard is wrong.  Replacing `std::bit_cast` with
`sw::bit_cast` changes nothing on platforms where it currently
works and fixes the MSVC-without-`/Zc:__cplusplus` path.
