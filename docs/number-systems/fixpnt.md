# Fixed-Point (fixpnt): Deterministic Fractional Arithmetic

## Why

Floating-point arithmetic is non-associative, non-distributive, and introduces rounding errors that vary across platforms and compiler settings. For DSP pipelines, control systems, financial calculations, and embedded processors without an FPU, these properties are unacceptable. You need fractional arithmetic with *deterministic* behavior: the same input always produces the same output, bit-for-bit, on every platform.

Fixed-point arithmetic achieves this by representing numbers as scaled integers. The radix point is fixed at compile time, so every operation reduces to integer arithmetic with known, bounded precision. The Universal `fixpnt` type makes this explicit and configurable: you choose exactly how many bits go to the integer part and how many to the fractional part, and whether overflow wraps or saturates.

## What

`fixpnt<nbits, rbits, arithmetic, bt>` is a signed binary fixed-point number:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Total number of bits |
| `rbits` | `unsigned` | -- | Fraction bits (bits after the radix point) |
| `arithmetic` | `bool` | `Modulo` | `Modulo` (wrap on overflow) or `Saturate` (clamp to range) |
| `bt` | typename | `uint8_t` | Storage block type |

### Value Representation

A `fixpnt<nbits, rbits>` stores a signed two's complement integer internally. The represented value is:

```
value = stored_integer / 2^rbits
```

For example, `fixpnt<8, 4>` uses 4 integer bits and 4 fraction bits:
- Stored bits `01000100` (decimal 68) represents 68 / 16 = **4.25**
- Resolution: 1/16 = 0.0625
- Range: [-8.0, 7.9375]

### Key Properties

- **Deterministic**: integer arithmetic under the hood, no FPU required
- **Configurable precision**: independent control of integer and fraction bits
- **Two overflow modes**: `Modulo` wraps silently; `Saturate` clamps to maxpos/maxneg
- **Trivially copyable**: no heap allocation, suitable for embedded and hardware targets
- **No denormals, no NaN, no infinity**: every bit pattern is a valid number
- **Uniform resolution**: precision is constant across the entire range (unlike floating-point)

### Supported Operations

- Arithmetic: `+`, `-`, `*`, `/`
- Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Bit shift: `<<`, `>>` (for scaling by powers of 2)
- Conversions: to/from `int`, `float`, `double`
- Range queries: `minpos()`, `maxpos()`, `minneg()`, `maxneg()`

## How It Works

Internally, `fixpnt` stores its value in a `blockbinary<nbits, bt>` -- the same limb-based storage used by `integer`. All arithmetic is performed as scaled integer operations:

- **Addition/subtraction**: direct binary add/subtract (same as integer)
- **Multiplication**: produces a 2×nbits intermediate, then rounds/truncates back to nbits
- **Division**: long division on the binary representation

The `Saturate` mode detects overflow after each operation and clamps the result to the maximum or minimum representable value, which is essential for control systems where wraparound could be catastrophic.

## How to Use It

### Include

```cpp
#include <universal/number/fixpnt/fixpnt.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
// 16-bit fixed-point with 8 fraction bits
// Range: [-128, 127.99609375], Resolution: 1/256
fixpnt<16, 8> temperature(23.5);
fixpnt<16, 8> offset(0.125);
auto adjusted = temperature + offset;  // 23.625 exactly

// High-precision fractional (no integer part except sign)
fixpnt<16, 15> fraction(0.33333);  // ~15 bits of fractional precision
```

### Saturating Arithmetic for Control Systems

```cpp
// Motor control: saturate instead of wrapping on overflow
fixpnt<16, 8, Saturate> duty_cycle(0.0);
fixpnt<16, 8, Saturate> increment(0.1);

for (int i = 0; i < 200; ++i) {
    duty_cycle += increment;
    // Saturates at maxpos (~127.996) instead of wrapping to negative
}
```

### DSP Filter Implementation

```cpp
template<typename Fixed>
Fixed iir_lowpass(Fixed input, Fixed prev_output, Fixed alpha) {
    // y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
    return alpha * input + (Fixed(1) - alpha) * prev_output;
}

using Q15 = fixpnt<16, 15>;  // Q1.15 format: [-1, 0.999969...]
Q15 alpha(0.1);
Q15 output(0.0);
Q15 input(0.5);
output = iir_lowpass(input, output, alpha);
```

### Plug-in Replacement

```cpp
template<typename Real>
Real compute_pid(Real error, Real integral, Real derivative,
                 Real Kp, Real Ki, Real Kd) {
    return Kp * error + Ki * integral + Kd * derivative;
}

// Use with floating-point during development
auto result_f = compute_pid(0.5f, 0.1f, 0.01f, 1.0f, 0.1f, 0.05f);

// Switch to fixed-point for deployment on embedded target
using F16 = fixpnt<16, 8, Saturate>;
auto result_fx = compute_pid(F16(0.5), F16(0.1), F16(0.01),
                              F16(1.0), F16(0.1), F16(0.05));
```

## Problems It Solves

| Problem | How fixpnt Solves It |
|---------|-----------------------|
| Floating-point non-determinism across platforms | Integer arithmetic: bit-exact on every platform |
| No FPU on embedded processor | Pure integer ops, no hardware FPU needed |
| Overflow causes catastrophic wraparound in control loops | `Saturate` mode clamps to safe bounds |
| Need exact representation of fractional constants | Configurable fraction bits, no IEEE rounding |
| DSP pipeline requires known, fixed latency | No variable-time denormal handling |
| Financial arithmetic needs exact decimal fractions | Choose `rbits` to match required precision (e.g., rbits=20 for ~6 decimal digits) |
