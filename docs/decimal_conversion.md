# How to Build and Test Decimal Conversion

## Quick Start

The decimal conversion test is integrated into the Universal build system as part of the `value<>` internal tests.

### Building the Test

```bash
# From the Universal root directory
cd <UNIVERSAL_ROOT>

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake - enable internal number tests
cmake .. -DUNIVERSAL_BUILD_NUMBER_INTERNALS=ON

# Build the specific decimal test
make value_decimal

# Or build all value tests
make -j$(nproc)
```

### Running the Test

```bash
# From the build directory
./internal/value/value_decimal
```

### Alternative: Build Everything

```bash
# From Universal root
mkdir -p build && cd build

# Build all internal tests (includes decimal conversion)
cmake .. -DUNIVERSAL_BUILD_NUMBER_INTERNALS=ON
make -j$(nproc)

# Run all value tests
ctest -R value

# Or run just the decimal test
./internal/value/value_decimal
```

### What Gets Built

When you enable `UNIVERSAL_BUILD_NUMBER_INTERNALS=ON`, CMake will build:
- `value_decimal` - Decimal conversion test (NEW!)
- `value_api` - API tests
- `value_arithmetic_add` - Arithmetic tests
- `value_experiment` - Experimental tests
- `value_formatting` - Formatting tests
- `value_performance` - Performance tests

### Test Location

- **Source**: `./build/internal/value/decimal.cpp`
- **Headers**:
  - `<ROOT>/include/sw/universal/number/support/dragon.hpp`
  - `<ROOT>/include/sw/universal/number/support/decimal_converter.hpp`
- **Built executable**: `./build/internal/value/value_decimal`

### Expected Output

When you run the test, you should see output like:

```
Testing Dragon algorithm basic functions...
1 * 2^3 = 8 (expected 8)
2 * 5^2 = 50 (expected 50)
...

Testing value<> to decimal conversion...
value<52>(1.0):
  Default:    1.000000
  Scientific: 1.0000000000e+00
  Fixed:      1.0000
...

Testing ioflags variations...
value<52>(123.456) with different flags:
  default:           123.456000
  showpos:           +123.456000
  scientific:        1.234560e+02
...
```

### Troubleshooting

**Problem**: `make value_decimal` fails with "No rule to make target"

**Solution**: Make sure you ran cmake with the correct flag:
```bash
cmake .. -DUNIVERSAL_BUILD_NUMBER_INTERNALS=ON
```

**Problem**: Compilation errors about missing headers

**Solution**: The headers are already in place. Make sure you're building from the Universal root directory.

**Problem**: Want to enable just this test

**Solution**: The CMake configuration automatically picks up all `.cpp` files in `internal/value/`. The test is included when you build value tests.

### Integration with CI

To add this test to your continuous integration:

```yaml
# In your .github/workflows or CI config
- name: Build and Test Decimal Conversion
  run: |
    cd build
    cmake .. -DUNIVERSAL_BUILD_NUMBER_INTERNALS=ON
    make value_decimal
    ./internal/value/value_decimal
```

### Next Steps

After verifying the test works:

1. Create a similar test for `blocktriple<>` in `/internal/blocktriple/conversion/decimal.cpp`
2. Integrate the decimal_converter into actual number types (posit, cfloat, etc.)
3. Add regression tests with specific expected values
4. Add to CI pipeline

---

**Note**: This follows Universal's standard testing pattern where each `.cpp` file in a test directory becomes an executable via the `compile_all()` macro in CMakeLists.txt.
