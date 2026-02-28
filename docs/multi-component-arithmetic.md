# Multi-component arithmetic

Douglas Priest’s multi-digit arithmetic is a high-precision floating-point. It is not an infinite precision in the sense of symbolic or algebraic representations like continued fractions or symbolic series. Instead, it’s a **floating-point expansion** method: a way to represent real numbers as a sum of multiple non-overlapping floating-point components, each capturing a portion of the number’s precision. This is sometimes called a *floating-point expansion* or *unevaluated sum*.

### How Priest Handles Infinite Series Like 1.0 / 3.0

When you compute `1.0 / 3.0`, the exact result is the repeating decimal `0.333...`, which would require infinite digits to represent exactly. Priest’s method doesn’t attempt to represent this as an infinite series. Instead, it:

- **Approximates the result to a finite precision**, using a sequence of floating-point numbers that together sum to the best possible approximation.

- **Controls the error** by ensuring that each component in the expansion is non-overlapping and ordered by magnitude.

- **Terminates the expansion** once the desired precision is reached—either by a fixed number of components or by bounding the residual error.

So in practice, the expansion of `1.0 / 3.0` might look like:

```text
x ≈ x₀ + x₁ + x₂ + ... + xₙ
```

Where each `xᵢ` is a floating-point number, and the sum approximates `1/3` to within machine epsilon or a user-defined tolerance.

### Controlling Iteration

Priest controls the iteration through:

- **Error bounds**: Each step reduces the residual error, and the process stops when the error is below a threshold.

- **Component limits**: You can cap the number of components (e.g., 3 or 4 doubles) to balance precision and performance.

- **Non-overlapping property**: Ensures that each component adds new information, avoiding redundant digits.

This is fundamentally different from symbolic infinite series or arbitrary base representations. It’s a **numerical method**, not an algebraic one.

In the context of the Universal Number Library, implementing Priest-style arithmetic means:

- Define a new type that wraps a vector of floating-point components.

- Arithmetic operations will propagate and refine these expansions.

- Design the multi-component twosum/twoprod operators to support the correct rounding and error tracking to maintain the non-overlapping invariant.

## Where it fits in the library

Universal already has a handrolled double-double and quad-double implementations. A triple-double would be a useful new type. However, we want to end up with a proper adaptive version as we are refining this number system. The machinery for digit expansion and deletion is going to be reusable among dd, td, qd, and full priest. Let’s sketch a roadmap that starts with `td` (triple-double) and gracefully scales toward full adaptive Priest-style arithmetic.

---

## Phase 1: Implement `td` (Triple-Double)

### Core Design

- **Type Name**: `td` (consistent with `dd`, `qd`)
- **Representation**: `std::array<double, 3>` or a custom struct with named components `hi`, `mid`, `lo`
- **Invariant**: Non-overlapping components, ordered by magnitude:  
  $$ |x_0| \geq |x_1| \geq |x_2| $$
- **Operations**:
  - Addition, subtraction, multiplication, division
  - Normalize after each operation to maintain the invariant
  - Use error-free transforms (e.g. `TwoSum`, `TwoProd`, `QuickTwoSum`) to propagate residuals

### Testing & Benchmarking

- Compare against `dd` and `qd` for accuracy and performance
- Validate against high-precision libraries (e.g. MPFR, Boost.Multiprecision)

---

## Phase 2: Shared Machinery for Expansion Arithmetic

### Reusable Components

Create a precision-agnostic backend for:

- **Expansion normalization**: Merge and re-sort components
- **Error tracking**: Residual propagation and bounding
- **Digit deletion**: Truncate least significant components based on precision goals
- **Component insertion**: Add new digits while preserving non-overlap

### Suggested API Sketch

```cpp
template<size_t N>
struct expansion {
    std::array<double, N> components;
    void normalize();       // enforce non-overlap
    void truncate(size_t k); // reduce to k components
    double estimate();      // quick approximation
    size_t effective_precision(); // bits of precision
};
```

This could be the shared base for `dd`, `td`, `qd`, and eventually adaptive `priest`.

---

## Phase 3: Adaptive Precision Arithmetic (Full Priest)

### Key Features

- **Dynamic expansion size**: `std::vector<double>` instead of fixed array
- **Precision control**: Stop when residual < ε or max components reached
- **Arithmetic operations**: Use iterative refinement to reach target precision

### Integration Strategy

- Define a new type: `priest`
- Internally use the same expansion machinery, but with dynamic sizing
- Allow conversion between `dd`, `td`, `qd`, and `priest` for interoperability

---

## Architectural Vision

| Layer        | Purpose                              | Type Examples         |
|--------------|--------------------------------------|-----------------------|
| `float`      | Native IEEE                          | `float`               |
| `double`     | Native IEEE                          | `double`              |
| Fixed        | Fixed-size expansions                | `dd`, `td`, `qd`      |
| Adaptive     | Arbitrary precision via expansions   | `priest`              |
| Symbolic     | (Future?) Continued fractions, etc.  | `symbolic_real`       |

Each layer builds on the previous, with shared normalization and arithmetic logic. We could even expose a `precision_policy` trait to guide adaptive behavior.

---

# Implementation 

How we can architect `td` to reuse and extend the `expansion` machinery, while also preparing for adaptive Priest-style arithmetic.

---

## Reuse Strategy: Building `td` on `dd`/`qd` Foundations

### What’s Already Reusable

- **Error-free transforms**: `two_sum`, `two_prod`, `three_sum`, `quick_two_sum`—these are already abstracted and can be reused directly.
- **Arithmetic scaffolding**: The layered approach to addition, multiplication, division, and normalization is modular and extensible.
- **Traits and manipulators**: `dd_traits`, `numeric_limits`, and `manipulators` can be templated or extended for `td`.
- **Conversion and formatting logic**: The `to_string`, `to_pair`, and `to_binary` functions can be adapted with minimal changes.

---

## Proposed `td` Type Design

### Struct Definition

```cpp
class td {
public:
    double hi, mid, lo;

    // Constructors
    td() = default;
    constexpr td(double h, double m, double l) noexcept : hi(h), mid(m), lo(l) {}

    // Conversion from native types
    constexpr td(double val) noexcept : hi(val), mid(0.0), lo(0.0) {}

    // Arithmetic operators
    td& operator+=(const td& rhs);
    td& operator*=(const td& rhs);
    td& operator/=(const td& rhs);

    // ... and so on
};
```

### Arithmetic Implementation

We can follow the same pattern as `dd`, but extend the residual propagation:

- Addition: `hi + hi`, propagate to `mid`, then to `lo`
- Multiplication: use `two_prod` and `three_sum` to accumulate partial products
- Division: iterative refinement with `fma` and residual tracking

---

## Toward Adaptive Precision: Shared Expansion Machinery

Let’s abstract the common logic into a reusable expansion class:

### Expansion Template

```cpp
template<size_t N>
class expansion {
public:
    std::array<double, N> limbs;

    void normalize();       // enforce non-overlap
    void renorm();          // optional aggressive renormalization
    double estimate();      // quick approximation
    size_t precision_bits(); // effective precision
};
```

This would allow `dd`, `td`, `qd`, and eventually `priest` to share normalization, rounding, and error propagation logic.

---

## Future-Proofing for `priest`

When we move to adaptive precision:

- Replace `std::array<double, N>` with `std::vector<double>`
- Use the same normalization and arithmetic routines
- Add precision control: terminate expansion when residual < ε

---

## Suggested File Layout

| File                          | Purpose                              |
|------------------------------|--------------------------------------|
| `td.hpp` / `td_impl.hpp`     | Triple-double type and operations    |
| `expansion.hpp`              | Shared expansion logic (templated)   |
| `priest.hpp` / `priest_impl.hpp` | Adaptive precision arithmetic       |
| `traits/td_traits.hpp`       | Type traits for `td`                 |
| `mathlib_td.hpp`             | Elementary math functions for `td`   |

---

## Shared 'expansion' framework

Starting with the shared `expansion` template framework. This will gives us a clean, reusable foundation for `dd`, `td`, `qd`, and eventually adaptive `priest`. Let’s build this in a modular way that fits naturally into Universal’s architecture and testing framework.

---

## Step 1: `expansion<N>` Template Design

This template will encapsulate a fixed-size expansion of `N` doubles, with normalization and utility functions.

### Header: `expansion.hpp`

```cpp
#pragma once
#include <array>
#include <cmath>
#include <algorithm>
#include <limits>

namespace sw::universal {

template<size_t N>
class expansion {
public:
    std::array<double, N> limbs{};

    // Constructors
    constexpr expansion() = default;
    constexpr expansion(const std::array<double, N>& values) : limbs(values) {}

    // Access
    double& operator[](size_t i) { return limbs[i]; }
    const double& operator[](size_t i) const { return limbs[i]; }

    // Estimate total value
    double estimate() const {
        double sum = 0.0;
        for (const auto& x : limbs) sum += x;
        return sum;
    }

    // Normalize: enforce non-overlapping and ordered magnitude
    void normalize();

    // Truncate to k components (zero out tail)
    void truncate(size_t k) {
        for (size_t i = k; i < N; ++i) limbs[i] = 0.0;
    }

    // Effective precision (rough estimate in bits)
    size_t precision_bits() const;

    // Debug print
    std::string to_string() const;
};

} // namespace sw::universal
```
---

## Implementation: `expansion_impl.hpp`

Here’s a sketch of the normalization and precision estimation logic:

```cpp
#pragma once
#include "expansion.hpp"
#include <sstream>
#include <iomanip>

namespace sw::universal {

template<size_t N>
void expansion<N>::normalize() {
    // Simple renormalization: sort by magnitude descending
    std::sort(limbs.begin(), limbs.end(), [](double a, double b) {
        return std::fabs(a) > std::fabs(b);
    });

    // Optional: enforce non-overlap using quick_two_sum or error-free transforms

    // For now, assume limbs are already non-overlapping
}

template<size_t N>
size_t expansion<N>::precision_bits() const {
    double total = std::fabs(estimate());
    if (total == 0.0) return 0;

    double residual = 0.0;
    for (const auto& x : limbs) residual += std::fabs(x);

    double relative_error = std::fabs(residual - total) / total;
    return static_cast<size_t>(-std::log2(relative_error + std::numeric_limits<double>::epsilon()));
}

template<size_t N>
std::string expansion<N>::to_string() const {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < N; ++i) {
        oss << std::setprecision(17) << limbs[i];
        if (i < N - 1) oss << ", ";
    }
    oss << "]";
    return oss.str();
}

} // namespace sw::universal

```
---

## Step 2: Regression Test Cases

### Sample Tests

```cpp
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include <sw/universal/number/expansion/expansion.hpp>

using namespace sw::universal;

void test_estimate() {
    expansion<3> e({0.25, 0.125, 0.0625});
    assert(std::fabs(e.estimate() - 0.4375) < 1e-15);
}

void test_normalize() {
    expansion<3> e({1e-10, 1.0, 1e-5});
    e.normalize();
    assert(std::fabs(e[0] - 1.0) < 1e-15);
    assert(std::fabs(e[1] - 1e-5) < 1e-15);
    assert(std::fabs(e[2] - 1e-10) < 1e-15);
}

void test_truncate() {
    expansion<4> e({1.0, 0.1, 0.01, 0.001});
    e.truncate(2);
    assert(e[2] == 0.0 && e[3] == 0.0);
}

void test_precision_bits() {
    expansion<3> e({1.0, 1e-16, 1e-32});
    size_t bits = e.precision_bits();
    assert(bits > 50); // Should exceed double precision
}

int main() {
    test_estimate();
    test_normalize();
    test_truncate();
    test_precision_bits();

    std::cout << "All expansion tests passed.\\n";
    return EXIT_SUCCESS;
}

```

---

## Next Steps

Once this is integrated and tested:

- We can use `expansion<2>` for `dd`, `expansion<3>` for `td`, `expansion<4>` for `qd`
- Then define `expansion_dynamic` for adaptive `priest`
- Build arithmetic operators (`+=`, `*=`, `/=`) using error-free transforms and expansion logic




