# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

Universal is a header-only C++ template library for custom arithmetic plug-in types. It provides alternatives to native integer and floating-point arithmetic, enabling mixed-precision algorithm development and optimization. The library is particularly valuable for Deep Learning, DSP, HPC, and embedded applications where tailored arithmetic types can yield significant performance and energy efficiency gains.

## Building and Testing

### Quick Build Commands

```bash
# Standard build workflow
mkdir build && cd build
cmake ..
make -j $(nproc)
make test
```

### Build Configuration

The library uses CMake with extensive build options. Key options:

- **Build all components**: `cmake -DUNIVERSAL_BUILD_ALL=ON ..`
- **Specific number system**: `cmake -DUNIVERSAL_BUILD_NUMBER_POSITS=ON ..`
- **Enable optimizations**: `cmake -DUSE_AVX2=ON ..` (or SSE3, AVX)
- **Regression levels**:
  - `-DUNIVERSAL_BUILD_REGRESSION_SANITY=ON` (level 1, default)
  - `-DUNIVERSAL_BUILD_REGRESSION_LEVEL_2=ON` (level 2, increased intensity over level 1)
  - `-DUNIVERSAL_BUILD_REGRESSION_LEVEL_3=ON` (level 3, increased intensity over level 2)
  - `-DUNIVERSAL_BUILD_REGRESSION_LEVEL_4=ON` (level 4, stress testing)
  - `-DUNIVERSAL_BUILD_REGRESSION_STRESS=ON` (level 1,2,3,4, comprehensive)

### Running Specific Tests

Individual test executables can be built:

```bash
# Build and run a specific test
make posit_logarithm
./static/posit/posit_logarithm

# Run test suite
ctest
```

### Common Development Tasks

- **Build educational examples**: `cmake -DUNIVERSAL_BUILD_EDUCATION=ON ..`
- **Build playground**: `cmake -DUNIVERSAL_BUILD_PLAYGROUND=ON ..`
- **Build internal number systems**: `cmake -DUNIVERSAL_BUILD_NUMBER_INTERNALS=ON ..`
- **Build numerical stress cases**: `cmake -DUNIVERSAL_BUILD_NUMERICS=ON ..`


## Library Architecture

### Header-Only Design

Universal is completely header-only with no external dependencies. The main include structure:

```cpp
#include <universal/number/[type]/[type].hpp>
```

Each number system has a single include file that brings in the complete implementation.

Use '-I<UNIVERSAL_ROOT>/include/sw' to find the include files for compilation.

### Number System Organization

Number systems are categorized as:

1. **Static** (fixed-size): Can be shared with hardware accelerators
   - `integer` - arbitrary configuration fixed-size integer
   - `fixpnt` - fixed-point numbers
   - `cfloat` - classic floating-point
   - `posit` - tapered floating-point (unum Type III)
   - `valid` - interval arithmetic with posit encoding
   - `lns` - logarithmic number system
   - `areal` - faithful floating-point with uncertainty bit
   - `bfloat16` - brain float
   - `dd`, `qd` - double/quad-double precision
   - `dd_cascade`, `td_cascade`, `qd_cascade` - double/triple/quad-double precision using floatcascade<>

2. **Elastic** (adaptive-precision): Can grow/shrink during computation
   - `einteger` - adaptive-precision integer
   - `edecimal` - adaptive-precision decimal
   - `erational` - adaptive-precision rational
   - `efloat` - adaptive-precision multi-digit float
   - `ereal` - adaptive-precision multi-component real

### Directory Structure

```
include/sw/
  ├── blas/               # Basic dense linear algebra
      ├── blas/            # BLAS L1, L2, L3
      ├── generators/      # Matrix generators
      ├── solvers/         # Constraint solvers
      └── vmath/             # vector math operators
  ├── math/               # Constants, functions, complex
  ├── numeric/            # Numerical containers and functions
  └── universal/          # Number system implementations
      └── number/           # Number system implementations
          ├── cfloat/         # Classic floating-point
          ├── posit/          # Posit (tapered floating-point)
          ├── fixpnt/         # Fixed-point
          └── [other types]/

Regression tests
static/                   # Fixed-size number system tests
  ├── posit/              # Organized by: api, conversion, logic, arithmetic, math
  ├── cfloat/
  └── [other types]/

elastic/                  # Adaptive-precision tests
  ├── einteger/
  ├── efloat/
  ├── ereal/
  └── [other types]/

Utilities, demos, educational examples, benchmarks
tools/cmd/                # Command-line inspection tools
applications/             # Example applications by use case
education/                # Educational examples
playground/               # Experimentation sandbox
benchmark/                # Performance and accuracy benchmarks

Mixed-precision algorithm design and optimzation SDK
mixedprecision/           # Mixed-precision algorithm SDK
```

### Test Suite Organization

Each number system has a comprehensive regression suite organized into:
- `api/` - API usage examples and invocation patterns
- `conversion/` - Type conversion tests
- `logic/` - Comparison and logic operations
- `arithmetic/` - Arithmetic operations
- `math/` - Mathematical functions
- `complex/` - Complex number support (when BUILD_COMPLEX=ON)

The `api/api.cpp` file in each number system is the best starting point for understanding usage patterns.

## Key Concepts

### Plug-in Replacement Pattern

Number types are designed as drop-in replacements for native types:

```cpp
template<typename Real>
Real MyKernel(const Real& a, const Real& b) {
    return a * b;
}

// Use with any Universal type
using Real = sw::universal::posit<32,2>;  // 32-bit posit, 2 exponent bits
using Real = sw::universal::cfloat<32,8,23>;  // IEEE-754 single precision
```

### Type Parameterization

Most number systems use template parameters for configuration:
- Size in bits
- Exponent bits (for floating-point types)
- Fraction bits (for fixed-point and lns types)
- Memory alignment (8-, 16-, 32-, or 64-bit)
- Other type-specific parameters, such as, Arithmetic behavior, Rouding, etc.

Example: `posit<nbits, es>` where `nbits` is total bits and `es` is exponent bits.

### Quire Support

The library includes quire (super-accumulator) support for exact dot products and reproducible linear algebra, particularly for posit arithmetic.

## Command-Line Tools

The library builds inspection utilities (in `tools/cmd/`):

- `ieee` - analyze IEEE floating-point representations
- `half`, `quarter`, `single`, `double`, `quad` - inspect specific IEEE-754 formats
- `posit`, `fixpnt`, `lns` - inspect other number systems
- `float2posit` - show conversion process
- `prop*` - show numerical properties

These tools are invaluable for understanding how values are represented.

## Development Workflow

### Adding a New Number System

Use the skeleton templates as starting points:
- `include/universal/number/skeleton_1param/` - for single-parameter types
- `include/universal/number/skeleton_2params/` - for two-parameter types

These contain all constituent pieces needed for a Universal-style number system.

### Regression Testing

The library uses GitHub Actions CI. To enable CI for your branch, add it to `.github/workflows/cmake.yml`.

Regression levels are controlled via CMake defines:
- `REGRESSION_LEVEL_1` - Basic sanity (default)
- `REGRESSION_LEVEL_2` - Moderate coverage
- `REGRESSION_LEVEL_3` - Comprehensive coverage
- `REGRESSION_LEVEL_4` - Exhaustive stress testing

### Using Universal in Your Project

```cmake
project("my-numerical-experiment")

find_package(UNIVERSAL CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/mymain.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_link_libraries(${PROJECT_NAME} universal::universal)
```

## Important Notes

### C++ Standard

The library requires **C++20** (configured by default in CMakeLists.txt).

### Compiler Flags

- GCC/Clang: Uses `-Wall -Wpedantic -Wno-narrowing -Wno-deprecated`
- MSVC: Uses `/MP` for parallel compilation, `/Zc:__cplusplus` for correct macro values
- Release builds: Aggressive optimization flags enabled

### AVX/SIMD Support

When available, enable for ~20% performance boost:
```bash
cmake -DUSE_AVX2=ON ..
```

### Installation

```bash
cmake -DCMAKE_INSTALL_PREFIX=/your/path ..
make install
# To uninstall:
make uninstall
```

Default install location is `/usr/local` on Linux.

## Testing Philosophy

- Each number system provides exhaustive validation of assignment, conversion, arithmetic, logic, exceptions, number traits, and special cases
- The `api.cpp` files chronicle all usage patterns and serve as executable documentation
- Educational examples in `education/` demonstrate fundamental concepts
- Application examples in `applications/` show real-world usage organized by use case (accuracy, performance, precision, reproducibility)

## Git Workflow

Current branch: `v3.91`
Main branch: `main`

The CMakeLists.txt embeds git commit hash in the version string for traceability.
