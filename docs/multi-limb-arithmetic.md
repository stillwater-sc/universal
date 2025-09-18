# Multi-Limb Arithmetic Types in Universal

The Universal library implements high-performance multi-limb arithmetic types that serve as the foundation for all custom number systems. These types enable arbitrary precision arithmetic while maintaining efficiency through block-based storage and operation-specific optimizations.

## Overview

Multi-limb arithmetic in Universal is built around several core components located in `include/sw/universal/internal/`:

- **`blockbinary`** - General-purpose multi-limb integer arithmetic
- **`blockfraction`** - Floating-point fraction management
- **`blocksignificant`** - Floating-point significand with encoding optimizations
- **`blocktriple`** - Complete floating-point representation for arithmetic

These components work together to provide a flexible, efficient foundation for implementing various number systems.

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

// 256-bit signed integer using 32-bit blocks
blockbinary<256, uint32_t, BinaryNumberType::Signed> big_int;

// 128-bit unsigned integer using 64-bit blocks
blockbinary<128, uint64_t, BinaryNumberType::Unsigned> unsigned_int;
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

### 3. `blocksignificant<nbits, BlockType, BitEncoding>`

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

### 4. `blocktriple<fbits, BlockType, BehaviorTag>`

Complete floating-point representation combining sign, exponent, and significand.

**Components:**
- Sign bit
- Exponent (with bias handling)
- Significand (using `blocksignificant` internally)
- Scale factor for denormalized arithmetic results

**Key Features:**
- Intermediate representation for floating-point arithmetic
- Handles denormalized forms (ii.ffffff format)
- Comprehensive rounding support
- Conversion to/from various floating-point formats

## Integration with Number Systems

The multi-limb types are used extensively throughout Universal's number systems:

### cfloat (Classic Floating-Point)
```cpp
// cfloat uses blockbinary for exponent and blocktriple for arithmetic
template<unsigned nbits, unsigned es, typename bt,
         bool hasSubnormals, bool hasSupernormals, bool isSaturating>
class cfloat {
    // Internal storage and arithmetic using block types
    blockbinary<es, bt> _exponent;
    blockbinary<nbits-es-1, bt> _fraction;
    // Arithmetic operations use blocktriple
};
```

### areal (Adaptive Floating-Point)
Uses similar block-based approach with adaptive precision based on operand requirements.

### integer (Multi-Precision Integer)
Direct use of `blockbinary` for arbitrary precision integer arithmetic.

### lns (Logarithmic Number System)
Uses block types for efficient logarithmic arithmetic operations.

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
Block types use contiguous storage with little-endian bit ordering within blocks:
```
Block 0: [bits 0-31]   Block 1: [bits 32-63]   Block 2: [bits 64-95]
```

### Operation-Specific Optimizations
- Addition: Uses 2's complement `blocksignificant` for efficient alignment
- Multiplication: Uses 1's complement `blocksignificant` for simpler partial products
- Division: Uses specialized algorithms in `blockfraction` for quotient/remainder

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

Potential enhancements include:
- SIMD-optimized implementations
- Hardware-specific assembly optimizations
- GPU acceleration for parallel operations
- Enhanced compile-time optimizations
- Extended precision support for quantum computing applications

This multi-limb arithmetic foundation enables Universal to provide efficient, accurate implementations of diverse number systems while maintaining a clean, extensible architecture.