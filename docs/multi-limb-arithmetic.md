# Multi-Limb Arithmetic Types in Universal

The Universal library implements high-performance multi-limb arithmetic types that serve as the foundation for all custom number systems. These types enable arbitrary precision arithmetic while maintaining efficiency through block-based storage and operation-specific optimizations.

## Overview

Multi-limb arithmetic in Universal is built around several core components located in `include/sw/universal/internal/`:

- **`blockbinary`** - General-purpose multi-limb integer arithmetic
- **`blockfraction`** - Floating-point fraction management
- **`blocksignificand`** - Floating-point significand with encoding optimizations
- **`blocktriple`** - Complete floating-point representation for arithmetic

These components work together to provide a flexible, efficient foundation for implementing various number systems.

### A note on the word "limb"

"Limb" is used in two different senses in this library, and they are not the same thing:

- **Here**, a limb is a *block of bits* in an integer-like container -- `blockbinary`,
  `blockfraction`, `blocksignificand`. The limbs of one value are contiguous pieces of a
  single fixed-point or significand field, and the block type (`uint8_t` ... `uint64_t`) is
  a storage and performance choice.
- **In `ereal` and the expansion arithmetic it is built on**, a limb is a *whole
  floating-point number*. A value is the exact unevaluated sum of its limbs, which are
  non-overlapping and descending in magnitude (Shewchuk/Priest expansions). The limb type
  is `float`, `double`, or a `long double` that is x87 extended or binary128, and it
  determines both the precision per limb and, through the exponent range, how many limbs
  the type may have at all.

The second sense is derived in
[ereal_limb_limit_derivation.md](ereal_limb_limit_derivation.md), which also covers how
many limbs an arithmetic *result* is allowed to occupy.

## Core Components

### 1. `blockbinary<nbits, BlockType, NumberType>`

A parameterized blocked binary number system representing integers in 2's complement or unsigned format.

**Template Parameters:**
- `nbits` - Total number of bits in the representation
- `BlockType` - Underlying storage type (uint8_t, uint16_t, uint32_t, etc.)
- `NumberType` - Either `BinaryNumberType::Signed` or `BinaryNumberType::Unsigned`

**Key Features:**
- Efficient storage using blocks of the specified type
- Support for both signed (2's complement) and unsigned arithmetic
- Comprehensive set of arithmetic operations
- Overflow and carry detection
- Long division with quotient and remainder

**Usage Example:**
```cpp
#include <universal/internal/blockbinary/blockbinary.hpp>

// 48-bit signed integer using 16-bit blocks, yielding 3 limbs
blockbinary<48, uint16_t, BinaryNumberType::Signed> big_int;

// 196-bit unsigned integer using 64-bit blocks, also yielding 3 limbs
blockbinary<196, uint64_t, BinaryNumberType::Unsigned> unsigned_int;
```

### 2. `blockfraction<nbits, BlockType>`

Manages floating-point fraction bits with optimizations for different arithmetic operations.

**Key Features:**
- Fraction bits scaled appropriately for add, multiply, divide operations
- Radix point management for different operation contexts
- Efficient normalization and alignment
- Support for guard bits and sticky bits for accurate rounding

**Design Philosophy:**
The fraction bits in floating-point need different representations for optimal performance:
- Addition/subtraction: 2's complement encoding for efficient alignment
- Multiplication: 1's complement encoding for simpler partial product accumulation
- Division: Specialized encoding for long division algorithms

### 3. `blocksignificand<nbits, BlockType, BitEncoding>`

Represents floating-point significands with operation-specific bit encodings.

**Template Parameters:**
- `nbits` - Number of bits in the significand
- `BlockType` - Storage block type
- `BitEncoding` - One of `BitEncoding::Flex`, `BitEncoding::Ones`, or `BitEncoding::Twos`

**Encoding Types:**
- `Flex` - Flexible encoding for general use
- `Ones` - 1's complement encoding (optimal for multiplication)
- `Twos` - 2's complement encoding (optimal for addition/subtraction)

**Key Features:**
- Type-encoded bit representations for compile-time optimization
- Radix point management that adapts to operation requirements
- Efficient conversion between encoding types
- Support for denormalized and normalized forms

### 4. `blocktriple<fbits, BlockTripleOperator, BlockType>`

Complete floating-point representation combining sign, exponent, and significand.

**Components:**
- NaN, Inf, Zero
- Sign bit
- Exponent
- Significand (using `blocksignificand` internally)
- Scale factor for denormalized arithmetic results

**Key Features:**
- Intermediate representation for floating-point arithmetic
- Always normalized in triple form: (sign, exp, significand)
- Comprehensive rounding support (TBD: currently only nearest-tie-to-even)

**Architecture:**

The blocktriple creates an execution environment for floating-point arithmetic.
Configured by the BlockTripleOperator it sets up a blocksignificant with the following structure:
```text
   ADD        iii.ffffrrrrrrrrr          3 integer bits, f fraction bits, and 2*fhbits rounding bits
   MUL         ii.ffff'ffff              2 integer bits, 2*f fraction bits
   DIV         ii.ffff'ffff'ffff'rrrr    2 integer bits, 3*f fraction bits, and r rounding bits
```
The arithmetic operators are presented as in-place methods, that is:
- add(lhs, rhs)
- sub(lhs, rhs)
- mul(lhs, rhs)
- div(lhs, rhs)

The basic usage pattern is:
```cpp
blocktriple<fbits, BlockTripleOperator::ADD, uint32_t> a, b, c;
a = normalize(src_a);
b = normalize(src_b);
c.add(a, b);
convert(c, target);
```

## Integration with Number Systems

The multi-limb types are used extensively throughout Universal's number systems:

### cfloat (Classic Floating-Point)
```cpp
// cfloat uses blockbinary for exponent and blocktriple for arithmetic
template<unsigned nbits, unsigned es, typename bt,
         bool hasSubnormals, bool hasMaxExpValues, bool isSaturating>
class cfloat {
    // Internal storage is raw blocks
    bt _blocks[nrBlocks];
    // Arithmetic operations use blockbinary and blocktriple to
    // manipulate exponent and fraction bits
};
```

### areal (a faithful Floating-Point format with an uncertainty bit)
Uses similar block-based approach with adaptive precision based on operand requirements.

### integer (arbitrary fixed-sized Integer)
Direct use of `blockbinary` for arbitrary precision integer arithmetic.

### lns (Logarithmic Number System)
Uses `blockbinary` to encode and implement arithmetic operations.

### fixpnt (Fixed-Point)
Employs `blockbinary` with implicit decimal point positioning.

## Performance Considerations

### Block Size Selection
Choose block types based on target architecture:
- `uint8_t` - Minimal memory, good for embedded systems
- `uint16_t` - Balanced performance/memory
- `uint32_t` - Optimal for most 32/64-bit architectures
- `uint64_t` - Maximum performance on 64-bit systems (with carry bit considerations)

### Memory Layout
Block types use contiguous storage with little-endian bit ordering within blocks. Here is an example using uint32_t as BlockType:
```text
Block 0: [bits 0-31]   
Block 1: [bits 32-63]   
Block 2: [bits 64-95]
```

### Operation-Specific Optimizations
- Addition: Uses 2's complement `blocksignificand` for efficient alignment
- Multiplication: Uses 1's complement `blocksignificand` for simpler partial products
- Division: Uses specialized algorithms in `blocksignificand` for quotient/remainder

## Error Handling

Multi-limb types provide comprehensive error detection:
- Overflow/underflow detection
- Division by zero handling
- Invalid operation identification
- Exception propagation to calling number systems

## Extension Points

The block-based architecture allows for:
- Custom block types for specialized hardware
- SIMD optimizations for vector operations
- Hardware-specific carry bit handling
- Custom rounding modes and behaviors

## Best Practices

1. **Choose appropriate block sizes** for your target architecture
2. **Use type-appropriate encodings** for different operations
3. **Leverage compile-time parameterization** for optimal performance
4. **Handle exceptions appropriately** in your number system implementations
5. **Test extensively** with boundary conditions and edge cases

## Future Directions

- SIMD-optimized implementations
- Hardware-specific assembly optimizations
- KPU and NPU acceleration for parallel operations
- Enhanced compile-time optimizations

This multi-limb arithmetic foundation enables Universal to provide efficient, accurate implementations of diverse number systems while maintaining a clean, extensible architecture.
