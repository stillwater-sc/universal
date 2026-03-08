# BLAS Modernization Assessment

Date: 2026-03-07

## Overview

Analysis of the `include/sw/blas/` subsystem for modern C++ (C++20) modernization opportunities.
Files examined: `blas_l1.hpp`, `blas_l2.hpp`, `blas_l3.hpp`, `operators.hpp`, `inverse.hpp`,
`lu.hpp`, `cg.hpp`, `statistics.hpp`.

---

## High-Priority Issues (Correctness/Safety)

### 1. Bug: `scale()` never increments `cnt` — `blas_l1.hpp:109`

```cpp
// BEFORE (bug: infinite loop when incx > 1)
for (cnt = 0, ix = 0; cnt < n && ix < size(x); ix += incx) {
    x[ix] *= alpha;
}

// AFTER
for (cnt = 0, ix = 0; cnt < n && ix < size(x); ++cnt, ix += incx) {
    x[ix] *= alpha;
}
```

### 2. Bug: `solve()` returns `int` from a `vector<Scalar>` function — `lu.hpp:282,301`

```cpp
// BEFORE (line 282, 301) — returns int literal from vector<Scalar> function
return 1;  // implicit conversion, should be an error/exception
return 2;

// Should return empty vector or throw
return vector<Scalar>{};
```

### 3. Unscoped enum `NormalizationMethod` — `blas_l3.hpp:64`

```cpp
// BEFORE
enum NormalizationMethod { Norm2, Center, Zscore, Scale, Range };

// AFTER
enum class NormalizationMethod { Norm2, Center, Zscore, Scale, Range };
```

Note: This enum is declared but never used in the codebase — could be removed entirely.

### 4. Copy constructor should be `const&` — `statistics.hpp:17`

```cpp
// BEFORE
Quantiles(Quantiles&) = default;

// AFTER
Quantiles(const Quantiles&) = default;
```

### 5. C-style array in `Quantiles` — `statistics.hpp:28`

```cpp
// BEFORE
Scalar q[5];

// AFTER
std::array<Scalar, 5> q;
```

---

## Medium-Priority (Modern C++ Idioms)

### 6. Namespace style — nested namespaces (C++17/C++20)

Every file uses `namespace sw { namespace blas {` (and some use triple nesting). C++20 supports:

```cpp
// BEFORE
namespace sw { namespace blas { namespace solvers {
}}}

// AFTER
namespace sw::blas::solvers {
}
```

### 7. Use `std::swap` instead of manual temp — `blas_l1.hpp:119-122`

```cpp
// BEFORE
typename Vector::value_type tmp = x[ix];
x[ix] = y[iy];
y[iy] = tmp;

// AFTER
std::swap(x[ix], y[iy]);
```

### 8. Verbose iterator types in `minValue`/`maxValue` — `blas_l1.hpp:274-284`

```cpp
// BEFORE
typename std::vector<Ty>::const_iterator it = min_element(samples.begin(), samples.end());
return *it;

// AFTER
return *std::min_element(samples.begin(), samples.end());
```

Also: these are outside any namespace (global scope pollution), missing `std::` on
`min_element`/`max_element`.

### 9. Manual abs instead of `std::abs` in `asum` — `blas_l1.hpp:20`

```cpp
// BEFORE
sum += (x[ix] < 0 ? -x[ix] : x[ix]);

// AFTER
using std::abs;
sum += abs(x[ix]);
```

### 10. Magic number `0.` and `0.0` for generic types — `lu.hpp:74,79,94,100`

```cpp
// BEFORE
value_type sum = 0.;     // double literal assigned to generic type

// AFTER
value_type sum{0};       // value-initialization, works with any type
```

This matters for Universal types where implicit conversion from `double` may not exist
or may lose precision.

---

## Low-Priority (Style/Cleanup)

### 11. Commented-out code blocks

- `lu.hpp:174-210` has an entire commented-out `lubksb` function
- `cg.hpp:31-32` has commented-out variables
- Multiple files have `//cout << ...` debug traces throughout

### 12. `using namespace` directives in headers

- `blas_l1.hpp:12`: `using namespace sw::numeric::containers;`
- `lu.hpp:24,114,216,276`: `using namespace std;`

These pollute the namespace for anyone including the header.

### 13. Comparison with `0.0` double literal — `inverse.hpp:85`

```cpp
// BEFORE
if (B(icol, icol) == 0.0)

// AFTER
if (B(icol, icol) == Scalar(0))
```

---

## Implementation Plan

| Phase | Scope | Files | Risk |
|-------|-------|-------|------|
| 1. Bug fixes | `scale()` loop, `solve()` return type, `Quantiles` copy ctor | 3 | Low — clear bugs |
| 2. Type safety | Scoped enum, C-array to `std::array`, `0.` to `{0}` | 4 | Low — localized |
| 3. Idiom updates | `std::swap`, `auto`, nested namespaces, remove `using namespace` | 8+ | Medium — touches many files |
| 4. Cleanup | Remove dead code, debug traces | 3+ | Low — cosmetic |

### Notes

- Phase 1 should be done first as it fixes actual bugs
- Phase 3 has the widest blast radius and should be tested with both gcc and clang
- The `using namespace` removal in Phase 3 may require qualifying types throughout the
  affected files, which increases the diff size significantly
- The nested namespace change is mechanical but touches every file
