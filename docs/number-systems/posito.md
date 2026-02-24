# Posito: Experimental Posit Validation Variant

## Why

When developing and validating a new number system, it is essential to have independent implementations that can be cross-checked against each other. The `posito` type is an alternative implementation of posit arithmetic used for experimental validation and research. It shares the same mathematical specification as `posit` and `posit1` but follows a separate implementation path, making it useful for detecting implementation bugs through differential testing.

## What

`posito<nbits, es>` implements the same posit encoding as posit1:

| Parameter | Type | Description |
|-----------|------|-------------|
| `nbits` | `unsigned` | Total bits |
| `es` | `unsigned` | Maximum exponent bits |

### Key Properties

- Same encoding as posit/posit1: sign, regime, exponent, fraction
- Same value semantics: tapered precision, NaR, reciprocal symmetry
- Independent implementation path for validation
- Uses posit1 internal classes (positRegime, positExponent, positFraction)
- Two-parameter template (like posit1, no BlockType parameter)

## How to Use It

### Include

```cpp
#include <universal/number/posito/posito.hpp>
using namespace sw::universal;
```

### Differential Testing

```cpp
// Cross-check posit implementations
posit<32, 2> p(3.14159);
posito<32, 2> po(3.14159);

// Both should produce identical results
auto p_result = p * posit<32, 2>(2.71828);
auto po_result = po * posito<32, 2>(2.71828);

assert(double(p_result) == double(po_result));
```

## When to Use

- **Research**: validating posit arithmetic properties
- **Testing**: differential testing against posit/posit1
- **Legacy**: compatibility with code that depends on posito
- **For new code**: prefer `posit<nbits, es, bt>` (the current v2 implementation)
