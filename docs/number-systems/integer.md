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
- **Optional exceptions** on overflow, underflow, and division by zero
- **Domain enforcement**: `WholeNumber` rejects negatives; `NaturalNumber` rejects zero and negatives
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

## How It Works

The integer is stored as an array of `BlockType` limbs, with the number of blocks computed at compile time as `ceil(nbits / bits_per_block)`. Arithmetic operations are performed limb-by-limb with carry propagation, exactly as in hardware ALU design but generalized to arbitrary widths.

The `NumberType` parameter enforces mathematical domain constraints at the type level:
- `IntegerNumber`: full signed range {..., -2, -1, 0, 1, 2, ...}
- `WholeNumber`: non-negative {0, 1, 2, ...}, throws on negative assignment
- `NaturalNumber`: strictly positive {1, 2, 3, ...}, throws on zero or negative

## How to Use It

### Include

```cpp
#include <universal/number/integer/integer.hpp>
using namespace sw::universal;
```

### Basic Usage

```cpp
// 128-bit integer for cryptographic operations
integer<128, uint32_t> a("123456789012345678901234567890");
integer<128, uint32_t> b("987654321098765432109876543210");
auto product = a * b;  // No overflow up to 128-bit range

// 256-bit integer 
integer<256, uint32_t> big_num(1);
big_num <<= 200;  // 2^200 -- not possible with native types
```

### Domain-Restricted Integers

```cpp
// Counter that must be positive
integer<32, uint32_t, NaturalNumber> counter(1);
// counter = 0;  // throws: 0 is not a natural number
// counter = -5; // throws: negative is not a natural number

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
