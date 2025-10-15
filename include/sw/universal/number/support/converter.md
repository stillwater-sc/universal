# How to Build and Run the Decimal Conversion Test

## Summary

There is a unified decimal conversion facility for Universal that works with the internal `value<>` representation.

## Quick Start

```bash
cd /home/stillwater/dev/stillwater/clones/universal/build

# Build the test
make value_decimal

# Run it
./internal/value/value_decimal
```

## What Was Created

### 1. Core Files
- **`dragon.hpp`** - Dragon algorithm implementation
- **`decimal_converter.hpp`** - Unified API for decimal conversion
- **`decimal.cpp`** - Test suite in `internal/value/`

### 2. File Locations
```
include/sw/universal/number/support/
├── dragon.hpp                 (Dragon algorithm core)
└── decimal_converter.hpp      (Unified conversion API)

internal/value/
└── decimal.cpp                (Test suite - gets built as value_decimal)
```

## Building

The test follows Universal's standard structure:

1. **Test Location**: `internal/value/decimal.cpp`
2. **CMake Integration**: Automatic via `compile_all()` macro in `internal/value/CMakeLists.txt`
3. **Build Target**: `value_decimal` (prefix `value_` + filename without extension)
4. **Folder**: "Internal/bit-level/value"

### Build Commands

```bash
# From Universal root
cd build

# Configure (if not already done)
cmake .. -DUNIVERSAL_BUILD_NUMBER_INTERNALS=ON

# Build just the decimal test
make value_decimal

# Or build all value tests
make -j$(nproc)

# Run
./internal/value/value_decimal
```

## Test Output

```
Decimal Converter Test Suite: report test cases
Testing Dragon algorithm basic functions...
1 * 2^3 = 8 (expected 8)
2 * 5^2 = 50 (expected 50)
...

Testing value<> to decimal conversion...
value<52>(1.0):
  Default:    ...
  Scientific: ...
  Fixed:      ...
...

Decimal Converter Test Suite: PASS
```

## Current Status

✅ **Working:**
- CMake integration
- Test compiles successfully
- Test runs without crashing
- Basic Dragon algorithm functions (power-of-2, power-of-5)
- API structure in place
- Unified interface for value<> types

⚠️ **Not Yet Implemented:**
- Full Dragon digit extraction algorithm (currently using simplified conversion)
- Accurate decimal representation (showing placeholder values)
- blocktriple<> support (removed to avoid circular dependencies)

## Next Steps

### To Complete the Implementation:

1. **Implement Full Dragon Algorithm** in `dragon.hpp`:
   - Proper scaling using integer arithmetic
   - Correct digit extraction
   - Rounding logic

2. **Add blocktriple Support**:
   - Create separate `decimal_converter_blocktriple.hpp`
   - Add test in `internal/blocktriple/conversion/decimal.cpp`

3. **Integrate with Number Types**:
   - Update posit, cfloat, fixpnt, etc. to use this facility
   - Replace their to_string() implementations

4. **Add Regression Tests**:
   - Known good values for comparison
   - Edge cases
   - All ioflags combinations

## Design Decisions Made

1. **Removed stream operators from decimal_converter.hpp** to avoid conflicts with existing operator<< in value.hpp
2. **Removed blocktriple support** from initial implementation to avoid circular header dependencies
3. **Used Universal's compile_all() pattern** for automatic test discovery
4. **Placed test in internal/value/** following Universal's structure

## API Usage (Once Completed)

```cpp
#include <universal/number/support/decimal_converter.hpp>
#include <universal/internal/value/value.hpp>

using namespace sw::universal;

internal::value<112> v(3.14159265358979323846);

// Convert to string
std::string s = to_decimal_string(v, std::ios_base::scientific, 30);

// Or use formatting helper
std::stringstream ss;
ss << std::setprecision(30) << std::scientific;
decimal_format_inserter(ss, to_decimal_string(v, ss.flags(), ss.precision()));
```

## Files Modified

- ✅ Created: `include/sw/universal/number/support/dragon.hpp`
- ✅ Created: `include/sw/universal/number/support/decimal_converter.hpp`
- ✅ Created: `internal/value/decimal.cpp`
- ✅ Updated: `conversion/README.md` (already existed)
- ✅ Created: Documentation files in `conversion/`

## Cleanup Done

- Removed conflicting test file from `conversion/` directory
- Fixed namespace issues
- Removed circular dependencies
- Aligned with Universal's patterns

---

**Status**: ✅ Test infrastructure complete and working. Dragon algorithm implementation is a skeleton that needs completion.
