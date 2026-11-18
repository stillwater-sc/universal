# Integer: Arbitrary-Width Fixed-Size Integer Arithmetic

## Why

Native C++ integers are locked to 8, 16, 32, or 64 bits. This constraint forces painful trade-offs: you either waste memory by using a wider type than necessary, or risk silent overflow by using a narrower one. For cryptographic applications, hash computations, combinatorics, or any domain where you need 128-bit, 256-bit, or even 1024-bit integers, the language offers nothing out of the box.

The Universal `integer` type removes this limitation. It provides a fixed-size, two's complement integer of *any* bit width, with compile-time configuration of overflow behavior and domain restrictions. Unlike arbitrary-precision libraries (GMP, Boost.Multiprecision), Universal integers are trivially copyable, stack-allocated, and suitable for hardware acceleration -- they behave like native integers, just wider.

## What

`integer<nbits, BlockType, NumberType>` is a fixed-size signed integer parameterized by:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Total number of bits (8, 16, 32, 128, 256, ...) |
| `BlockType` | typename | `uint8_t` | Storage limb type (uint8_t, uint16_t, uint32_t, uint64_t) |
| `NumberType` | enum | `IntegerNumber` | Domain: `IntegerNumber`, `WholeNumber`, or `NaturalNumber` |

### Key Properties

- **Two's complement encoding** with configurable width
- **Modular arithmetic** by default (overflow wraps silently, like native types)
- **Division by zero throws** by default; optional exceptions for the domain errors
- **Domain enforcement**: `WholeNumber` rejects negatives; a `NaturalNumber` result outside its domain is NaNN, Not a Natural Number
- **Trivially copyable**: no heap allocation, suitable for GPU/FPGA memory
- **Full constexpr support** for compile-time evaluation
- **Range**: [-2^(nbits-1), 2^(nbits-1) - 1] for signed integers

### Supported Operations

- Arithmetic: `+`, `-`, `*`, `/`, `%`
- Comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
- Bitwise: `&`, `|`, `^`, `~`, `<<`, `>>`
- Increment/decrement: `++`, `--`
- Conversions: to/from `int64_t`, `float`, `double`, strings
- Bit manipulation: `setbit()`, `clearbit()`, `testbit()`, `flip()`
- String I/O: binary, decimal, hex, octal representations
- Integer power: `ipow(a, b)`, also `pow(a, b)` for an integer or `int` exponent

### Integer power

`ipow(a, b)` computes a^b exactly, by squaring in `integer`'s own arithmetic, so a power past
the range wraps the way `*` wraps. The exponent is an `integer` or a native integer, which need
not fit in `nbits`. `pow(a, b)` with an integer or `int` exponent is `ipow`; only a `double`
exponent goes through `double`. The edge cases follow `std::pow` and integer division:

- `a^0 == 1` for every `a`, `0^0` included
- a negative exponent gives `1 / a^|b|` truncated: `1` for `a == 1`, `+-1` for `a == -1` by the
  parity of `b`, `0` for `|a| > 1`, and the division by zero for `a == 0`

The real-valued types have no `ipow`: their integer power is `pow(a, int)` (#1875).

### Division by zero

`integer` has no NaN or infinity to carry a failed division in the value, so by
default a division by zero throws `integer_divide_by_zero`. A caller that would rather get a value
opts out before the include, with `INTEGER_THROW_DIVIDE_BY_ZERO 0`, or with
`INTEGER_THROW_ARITHMETIC_EXCEPTION 0` when the first is not set:

```cpp
#define INTEGER_THROW_DIVIDE_BY_ZERO 0
#include <universal/number/integer/integer.hpp>
```

A build with exceptions disabled (`-fno-exceptions`) gets the value without asking:
`INTEGER_THROW_DIVIDE_BY_ZERO` defaults to `UNIVERSAL_HAS_EXCEPTIONS`, which
`<universal/utility/exceptions.hpp>` takes from the compiler. Setting it to `1` in such a build
stops with an `#error` (#1925).

With the exception off, `x / 0` and `x % 0` are `0`, as are both outputs of `reduce()`
and `idiv()`. Nothing is printed. `INTEGER_THROW_ARITHMETIC_EXCEPTION` still defaults to `0`,
so the `WholeNumber` and `NaturalNumber` domain errors keep their default; only division by
zero changed (#1894).

### Conversions with native floating-point and unsigned types

These rules apply at every width (#2131).

- **Native float to integer:** truncates toward zero, exactly: `integer<128>(1e30)` is
  1000000000000000019884624838656. NaN, infinity and values past the range follow the cast rule
  the real types use when they convert to a native integer (#2104):
  - an `IntegerNumber` saturates to `[-max, max]`, and NaN gives `min()`, the 100...0 pattern no
    finite value maps to. So `integer<16>(-32768.0)` is -32767;
  - `WholeNumber` saturates to `[0, max - 1]`, and NaN gives `max()`, all ones. A negative value, or one that truncates to 0, clamps to 0.
  - `NaturalNumber` saturates to `[1, max]`. NaN, a negative value, or one that truncates to 0 gives NaNN (see below), so all ones is one of its values.
- **Integer to float, double or long double:** correctly rounded, rounded once from the integer's
  top bits.
- **Integer to a native unsigned type:** modular, as in C++. A negative value sign-extends, so
  `(unsigned)integer<8>(-1)` is `0xFFFFFFFF`.
- **`>>`:** an arithmetic shift. Shifting a negative value by `nbits` or more gives -1, not 0.
- **A failed string parse** (`integer<16>("xyz")`) leaves 0.

`std::numeric_limits` is specialized for all three kinds:
- **`IntegerNumber`:** `min()` and `lowest()` are the most negative value.
- **The unsigned kinds:** `WholeNumber` is {0, 1, 2, ...} and `NaturalNumber` is {1, 2, 3, ...}, so their `min()` is 0 and 1. The operators used to enforce the reverse (a `WholeNumber` result of 0 was the error, a `NaturalNumber` 0 was valid); they now check the documented domains (#2131). `NaturalNumber` has a quiet NaN, NaNN (#2137).
- **Every kind:** `epsilon()`, `round_error()`, `denorm_min()` and `infinity()` return the integer
  type's zero.

## How It Works

The integer is stored as an array of `BlockType` limbs, with the number of blocks computed at compile time as `ceil(nbits / bits_per_block)`. Arithmetic operations are performed limb-by-limb with carry propagation, exactly as in hardware ALU design but generalized to arbitrary widths.

The `NumberType` parameter enforces mathematical domain constraints at the type level:
- `IntegerNumber`: full signed range {..., -2, -1, 0, 1, 2, ...}
- `WholeNumber`: non-negative {0, 1, 2, ...}, throws on negative assignment
- `NaturalNumber`: strictly positive {1, 2, 3, ...}, with NaNN for any result outside that domain

### NaNN, Not a Natural Number

A `NaturalNumber` has no 0, so its all-zeros encoding is free. It is **NaNN**, Not a Natural Number, a quiet NaN (#2137):

- **Results.** Every result outside {1, 2, 3, ...} is NaNN, and nothing throws, whatever `INTEGER_THROW_ARITHMETIC_EXCEPTION` says.
  - Some results are NaNN by their bits alone: `10 % 5`, `7 * 0`, `1 & 2`, `5 ^ 5`, `1 << nbits`, and a product that wraps to 0.
  - A difference below 1 is NaNN, as are `1 / 2`, `--1`, and the negation of any value.
- **Propagation.** NaNN propagates through `+ - * / % & | ^ << >> ++ --`.
  - `x / NaNN` and `x % NaNN` are NaNN, not a division by zero: the divisor is not a number, rather than zero.
  - A literal 0 converts to NaNN, so `n + 0` and `n / 0` are NaNN.
- **Comparisons** are posit-style: `NaNN == NaNN`, and NaNN orders below every value.
- **Conversions.**
  - 0, a negative value, a NaN, and a float below 1 convert to NaNN.
  - NaNN converts to a quiet NaN, and to the integer NaR of the #2104 cast rule: `min()` for a signed target, `max()` for an unsigned one.
- **Text.** NaNN prints as `NaNN`, and `parse` reads `"NaNN"`, `"0"` and any negative value as NaNN.
- **API.**
  - `isnan()` is true for NaNN. `iszero()` is the same all-zeros test, since a `NaturalNumber` has no zero.
  - `numeric_limits<>::has_quiet_NaN` is true and `quiet_NaN()` is NaNN; `min()` is 1 and `max()` is all ones.

`WholeNumber` keeps 0 as a value. Its all-ones NaN exists only in the native cast rule.

## How to Use It

### Include

```cpp
#include <universal/number/integer/integer.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
// 128-bit integer for cryptographic operations
integer<128, uint64_t> a("123456789012345678901234567890");
integer<128, uint64_t> b("987654321098765432109876543210");
auto product = a * b;  // No overflow up to 128-bit range

// 256-bit integer with an efficient 64-bit limb
integer<256, uint64_t> big_num(1);
big_num <<= 200;  // 2^200 -- not possible with native types
```

### Domain-Restricted Integers

```cpp
// Counter that must be positive
integer<32, uint32_t, NaturalNumber> counter(1);
counter = 0;      // NaNN: 0 is not a natural number
counter.isnan();  // true; counter + 1 stays NaNN

// Array index that must be non-negative
integer<16, uint16_t, WholeNumber> index(0);
// index = -1;  // throws: negative is not a whole number
```

### Plug-in Replacement in Generic Code

```cpp
template<typename Integer>
Integer factorial(Integer n) {
    Integer result(1);
    for (Integer i(2); i <= n; ++i) result *= i;
    return result;
}

// Native int overflows at 13!
int native = factorial(13);  // undefined behavior

// Universal integer handles 100! easily
integer<512> result = factorial(integer<512>(100));
```

## Problems It Solves

| Problem | How Integer Solves It |
|---------|-----------------------|
| Need 128-bit or 256-bit integers | Template parameter sets exact width |
| Silent integer overflow in financial systems | Domain restrictions + optional exceptions |
| Cryptographic big-number arithmetic | Arbitrary width with efficient block storage |
| Hash computations wider than 64 bits | Native-like API with any bit width |
| Compile-time integer computation | Full constexpr support |
| Hardware-accelerator compatibility | Trivially copyable, fixed layout |
