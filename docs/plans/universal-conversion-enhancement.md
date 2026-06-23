# Design Plan & Tracking Issue: Enhancing Universal Number System Conversions

## Epic: Universal Cross-Type Conversion Layer Redesign

### Motivation

The Universal Numbers Library provides a wealth of custom number systems (e.g., posits, fixed-points, arbitrary-precision integers, decimals, rationals, and various specialized floats). However, the current cross-type conversion layer (`universal_cast`, `universal_assign`, and mixed-type arithmetic operators) relies on a fundamental architectural bottleneck: **marshalling all conversions and mixed-type operations through native floating-point types (`double` or `long double`).**

This approach has severe, critical shortcomings:
1. **Precision Loss**: Converting between high-precision formats (e.g., 128-bit classic floats or 64-bit posits) loses precision when forced through a `double` (52-bit significand) or `long double` (typically 63-bit significand).
2. **Semantic Corruption of Non-Float Types**: Converting integers, fixed-points, decimals, or rationals through a binary floating-point representation results in rounding errors, base-conversion artifacts, and precision truncation (e.g., casting a large `integer<128>` or exact `rational` to a `double`).
3. **Dynamic Range Mismatch**: Intermediate casts can prematurely overflow or underflow even if the source and target formats are fully capable of representing the value.
4. **Brittle Special Value Handling**: Special values (like `NaR` in posits or `NaN` in floats) are mapped implicitly and loosely via native compiler semantics instead of explicit mapping policies.

Resolving this requires rearchitecting the conversion layer to support **specialized direct conversion paths** for compatible type families, and introducing **arbitrary-precision intermediates** (such as `ereal` or `efloat`) to serve as a lossless or near-lossless fallback.

---

### Code Quality Assessment of Current Implementation

The current conversion layer, implemented in `include/sw/universal/number/convert/`, fails to meet the core requirements of an arbitrary-precision mathematical library:

#### 1. Precision Truncation on High-Precision Floats
In `universal_convert.hpp`, compile-time dispatch selects an intermediate based on the maximum precision:
```cpp
if constexpr (maxPrec <= 52u) {
    return detail::cast_via<double, Target>(src);
} else {
    return detail::cast_via<long double, Target>(src);
}
```
*   **The Bottleneck**: For types wider than `long double`'s significand (63 bits on x86, or 52 bits on MSVC/ARM where `long double` is alias to `double`), this is lossy.
*   **Result**: Converting a `cfloat<128,15>` to a `posit<64,3>` needlessly discards accuracy.

#### 2. Non-Floating-Point Type Mismatch
*   **`integer<N>`**: If a 128-bit integer exceeds $2^{53}$, converting it to a target type through `double` yields an approximation, discarding the lower bits of the integer.
*   **`fixpnt<N, R>`**: High-precision fixed-point numbers have their fractional bits truncated to fit the `double` significand, corrupting exact representation.
*   **`rational`**: A rational number like $1/3$ is exact. Marshalling it through `double` forces binary rounding (`0.3333333333333333`), causing subsequent conversions to other exact formats (like decimals or other rationals) to be based on an already approximated value.

#### 3. Premature Overflow/Underflow
Some Universal types support dynamic ranges far exceeding native `double` ($10^{\pm 308}$). 
*   **Result**: If a type representing a huge or tiny value is cast to another capable type, marshalling through `double` will prematurely snap the value to $\pm \infty$ or $0$.

#### 4. Lack of Explicit Special Value Mapping
Except for a zero check (`if (src.iszero()) return Target{};`), mapping policies for `NaR` (Not a Real), `NaN`, `Inf`, and subnormals are handled implicitly by native compiler casts, leading to undefined or platform-dependent behavior.

---

### Requirements for a Universal Converter

To satisfy the goals of the library, a universal conversion layer must adhere to these requirements:

1.  **Direct Lossless Conversion Paths**: If two types belong to the same family or have a natural mathematical mapping (e.g., `posit<nbits, es>` to `posit<nbits2, es2>`), the conversion must be done directly and bit-exactly, without intermediates.
2.  **No-Loss fallback Intermediates**: For conversions where direct paths are not yet implemented, the fallback intermediate must possess enough precision and dynamic range to represent both source and target types exactly (e.g., using adaptive-precision `ereal` or `efloat` instead of native `double`).
3.  **Explicit Rounding and Saturation Policies**: Any conversion that is mathematically lossy (e.g., narrowing precision) must have explicit, configurable rounding modes (e.g., Round-to-Nearest, Truncate) and overflow handling (e.g., saturate to `maxpos` or map to infinity).
4.  **Semantic Type Preservation**: Integers and fixed-point numbers must be converted via exact integer arithmetic; rationals and decimals must convert via exact fractional/decimal arithmetic.
5.  **Robust Special Value Policies**: Explicit mapping tables for non-numerical values (e.g., `NaR -> NaN`, `Inf -> maxpos` or `Inf`, etc.) based on target system capabilities.

---

### Implementation Phases

This tracking issue suggests a 5-phase redesign to incrementally replace the `double` bottleneck with a correct, robust, and extensible conversion system.

```text
Phase 1: Specialized Overloads (Direct Paths)
    |
    v
Phase 2: High-Precision Fallback Intermediates (ereal / efloat)
    |
    v
Phase 3: Explicit Rounding & Special Value Mapping Policies
    |
    v
Phase 4: Rearchitecting Mixed-Type Arithmetic Operators
    |
    v
Phase 5: Exhaustive Cross-Type Conversion Test Suite
```

#### Phase 1: Specialized Overloads (Direct Paths)
Introduce specialized overloads of `universal_cast` and `universal_assign` for highly-used type families to bypass intermediates entirely.
*   **Tasks**:
    *   Add direct conversions for `posit<N1, ES1> -> posit<N2, ES2>`.
    *   Add direct conversions for `integer<N1> -> integer<N2>`.
    *   Add direct conversions for classic float variations (`cfloat -> cfloat`).
*   **Value**: Immediate performance and precision gains for the most common workflows.

#### Phase 2: High-Precision Fallback Intermediates (`ereal` / `efloat`)
Instead of fallback to `double` or `long double`, use the library's own adaptive-precision types as intermediates when a specialized direct path is missing.
*   **Tasks**:
    *   Define compile-time traits to check if Source/Target require arbitrary precision.
    *   Integrate `sw::universal::ereal` (multi-component adaptive float) or `sw::universal::efloat` as the fallback intermediate for types wider than 52 bits.
*   **Value**: Bypasses the $52$-bit significand limit for all custom formats, ensuring lossless fallback.

#### Phase 3: Explicit Rounding & Special Value Mapping Policies
Formalize how conversions handle precision loss, overflow, and special values.
*   **Tasks**:
    *   Provide explicit rounding mode parameters to `universal_cast` (defaulting to the target type's default rounding).
    *   Add explicit handlers for exceptional values (`NaN`, `Inf`, `NaR`) to ensure correct state transitions between differing number systems.
*   **Value**: Extinguish platform-dependent bugs and guarantee deterministic arithmetic results.

#### Phase 4: Rearchitecting Mixed-Type Arithmetic Operators
Update `mixed_arithmetic.hpp` to prevent silent degradation of mixed expressions.
*   **Tasks**:
    *   Compute the optimum intermediate evaluation type for mixed expressions at compile time.
    *   Perform mixed operations (e.g., `posit<64,3> + cfloat<128,15>`) in the higher-precision type (or a lossless intermediate) rather than down-converting both to `double` before addition.
*   **Value**: Resolves the precision loss bottleneck in mixed-precision scientific calculations.

#### Phase 5: Exhaustive Cross-Type Conversion Test Suite
Establish a rigorous verification environment for cross-type conversions.
*   **Tasks**:
    *   Create `tests/conversions/cross_type_conversion.cpp`.
    *   Add tests verifying precision limits, large integers, fixed-points, rationals, and overflow/underflow boundaries.
*   **Value**: Ensures regression safety and validates the correctness of the conversion redesign.

---

### Risk Assessment

*   **Circular Dependency**: Using `ereal` or `efloat` as fallback intermediates in `universal_convert.hpp` might introduce circular header dependencies, because `ereal` and `efloat` headers themselves might include conversion headers.
    *   *Mitigation*: Carefully separate forward declarations of conversion templates from their concrete implementations, or keep `universal_convert.hpp` as a pure interface header with implementations deferred until all types are defined.
*   **Performance Overhead**: Adaptive-precision fallback (`ereal` / `efloat`) will be slower than native `double` CPU instructions.
    *   *Mitigation*: This is acceptable for a fallback, but underscores the importance of Phase 1 (writing specialized direct overloads for common types like `posit` and `cfloat` to keep hot paths fast).
*   **MSVC/ARM Compatibility**: Ensure compile-time dispatch behaves correctly on platforms where `long double` is identical to `double`.

---

### Reference GitHub Issues & Notes
*   See `TODO` comments in `include/sw/universal/number/convert/universal_convert.hpp`.
*   Related to GitHub Issue #197 (AST and expression-template evaluation).
