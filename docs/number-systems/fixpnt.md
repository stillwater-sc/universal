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

### Division by zero

`fixpnt` has no NaN or infinity to carry a failed division in the value, so by
default a division by zero throws `fixpnt_divide_by_zero`. A caller that would rather get a value
opts out before the include, with `FIXPNT_THROW_DIVIDE_BY_ZERO 0`, or with
`FIXPNT_THROW_ARITHMETIC_EXCEPTION 0` when the first is not set:

```cpp
#define FIXPNT_THROW_DIVIDE_BY_ZERO 0
#include <universal/number/fixpnt/fixpnt.hpp>
```

A build with exceptions disabled (`-fno-exceptions`) gets the value without asking:
`FIXPNT_THROW_DIVIDE_BY_ZERO` defaults to `UNIVERSAL_HAS_EXCEPTIONS`, which
`<universal/utility/exceptions.hpp>` takes from the compiler. Setting it to `1` in such a build
stops with an `#error` (#1925).

With the exception off, `x / 0` is `maxpos` or `maxneg` by the sign of the dividend for
`Saturate`, and `0` for `Modulo`. Nothing is printed. Before #1894 the opt-out was the
default and it wrote a line to stderr, which generic code could not detect.

### Trigonometry: the double path and the CORDIC engine

`fixpnt` has two kinds of trigonometry, because there are two kinds of experiment.

- **Software experiments** use the ordinary `sin`, `cos`, `tan`, `atan`, `atan2` and `hypot`. They
  compute through `double`: `sin(x)` is `fixpnt(std::sin(double(x)))`. This is fast and convenient. A
  `double` carries 53 significant bits, though, so for a format with more than about 52 fraction bits,
  such as `fixpnt<64,60>` or `fixpnt<128,120>`, the result is correct to at most 53 bits.
- **Hardware experiments** use the circular CORDIC engine in namespace `cordic` (#2015). It is a
  shift-and-add model of a CORDIC unit, computed in the type's own arithmetic. It is correct to the
  full width of the type, and it has an iteration count and a guard width that a designer can set.

```cpp
#include <universal/number/fixpnt/fixpnt.hpp>
using namespace sw::universal;

using F = fixpnt<32, 28, Saturate, std::uint32_t>;
F theta(0.5), s, c;
cordic::sincos(theta, s, c);                    // rotation mode: sin and cos together
F a = cordic::atan2(F(1), F(-1));               // vectoring mode: 3 pi / 4
F r, t;
cordic::polar(F(3), F(4), r, t);                // r = 5, t = atan2(4, 3)

// a unit with 4 guard bits that stops after 16 iterations
F s16 = cordic::sin<4>(theta, 16);

// linear CORDIC: multiply and divide by shift and add (#2040)
F p = cordic::mul(F(3.25), F(-1.5));            // -4.875
F q = cordic::div(F(1), F(3));                  // 1/3, rounded to 28 fraction bits
```

The circular functions are `sincos`, `sin` and `cos` (rotation mode), and `atan2`, `atan`, `hypot`
and `polar` (vectoring mode). The linear functions are `mul` (rotation mode) and `div` (vectoring
mode). Each takes the iteration count as an optional last argument, and the guard width as an
optional first template argument. Asking for more iterations than the datapath has fraction bits
gives that maximum. All of them run the same iteration, Walther's unified CORDIC, with the geometry
`m` (circular, linear or hyperbolic) as a template parameter. `docs/algorithmic-details/cordic.md`
explains the algorithm and its history.

- **Datapath.** The iterations run in a wider `fixpnt`, with `g` guard fraction bits and enough integer
  bits that nothing wraps. Each shift truncates, as an arithmetic shift does in hardware. The result is
  rounded to nearest once, at the end, and then follows the type's arithmetic: a result the type
  cannot hold, such as `cos(0) = 1` in `fixpnt<8,7>`, saturates under `Saturate` and wraps under
  `Modulo`, as a conversion from `double` does.
- **Defaults.** The guard width is `g = ceil(log2(rbits + 8)) + 2`, and the iteration count is
  `rbits + g`. With them, every result is within 1 ulp of the exact value. The measured worst case is
  0.62 ulp, over every encoding of the formats up to 16 bits and samples of `fixpnt<128,120>`.
- **Accuracy against the iteration count.** The angle left after `n` iterations is at most
  `atan(2^-(n-1))`, so the error halves with each iteration until the guard bits run out. In
  `fixpnt<16,12>` the worst `sin` error is 509 ulp at 4 iterations, 32 ulp at 8 and 2.4 ulp at 12.
  At the default 19 iterations, it is 7.9 ulp with no guard bits, 3.1 with 2, 1.1 with 4 and 0.57
  with the default 7.
- **Range reduction.** Rotation converges for `|theta| <= 1.7432866...` radians. A format that holds
  only `[-1, 1)` or less needs no reduction, and that choice is made at compile time. Every other
  format reduces by `pi/2`, with a `pi/2` that carries the integer bits of the quotient in addition to
  the datapath's fraction bits, so the reduced angle is accurate whatever the size of `theta`.
- **Vectoring.** `atan2` and `hypot` first scale both inputs by a power of two, so the larger lies in
  `[1/2, 1)`. Then a small input keeps its angle accuracy. Next they rotate a left half-plane vector
  by `-pi/2` or `+pi/2`. `atan2(0, 0)` and `hypot(0, 0)` are 0.
- **Constants.** The `atan(2^-i)` table, `1/K_n` for every iteration count `n`, and `pi/2` are computed
  once per datapath type, by series in `fixpnt` arithmetic 16 bits wider than the datapath, and are
  then rounded to it. Nothing goes through `double`.

- **Linear mode.** `mul` and `div` scale both operands by powers of two into `[1/2, 1)`, as a
  floating-point unit aligns significands, so a small operand keeps its accuracy. The datapath carries
  as many fraction bits as the result can need, plus guard bits: under `Modulo` that includes every
  bit of a wrapped product or quotient. Division by zero follows `operator/`. The measured worst case
  is 0.53 ulp for `mul` and 0.50 ulp for `div`, over every pair of the 8-bit formats and samples up to
  `fixpnt<128,120>`, against exact integer references.

The tests are in `static/fixpnt/binary/math/cordic.cpp` (circular) and `cordic_linear.cpp` (linear).
The lns add/sub has a hyperbolic CORDIC for the same purpose (`CORDICAddSub`, #783).

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
