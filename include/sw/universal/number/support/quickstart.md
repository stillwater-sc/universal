# Decimal Conversion Quick Start Guide

## 5-Minute Integration

### Step 1: Include the Header

```cpp
#include <universal/number/support/decimal_converter.hpp>
```

### Step 2: Use the API

```cpp
using namespace sw::universal;

// For value<>
internal::value<52> v(3.14159);
std::string s = to_decimal_string(v, std::ios_base::scientific, 10);
std::cout << s << '\n';  // Output: 3.1415900000e+00

// For blocktriple<>
blocktriple<32, BlockTripleOperator::REP, uint32_t> bt(2.71828);
std::cout << std::fixed << std::setprecision(5) << bt << '\n';  // Output: 2.71828
```

That's it! You now have accurate, arbitrary-precision decimal conversion.

## Common Use Cases

### Use Case 1: High-Precision Output

```cpp
// Works with any precision!
internal::value<256> big_pi(/* high-precision pi */);
std::cout << std::setprecision(75) << big_pi << '\n';
// Outputs all 75 digits accurately
```

### Use Case 2: Scientific Notation

```cpp
value<52> small(1.23e-100);
std::cout << std::scientific << small << '\n';  // 1.230000e-100
```

### Use Case 3: Fixed-Point Display

```cpp
value<32> money(123.456789);
std::cout << std::fixed << std::setprecision(2) << money << '\n';  // 123.46
```

### Use Case 4: Integrating with Your Type

```cpp
class MyFloatingPoint {
    internal::value<112> _value;

public:
    friend std::ostream& operator<<(std::ostream& ostr, const MyFloatingPoint& x) {
        return ostr << to_decimal_string(x._value, ostr.flags(), ostr.precision());
    }
};
```

## Testing Your Integration

```bash
# Build the test
cd build
cmake .. -DBUILD_CONVERSION=ON
make decimal_converter_test

# Run it
./conversion/decimal_converter_test
```

## What's Supported?

✅ Scientific notation (`std::scientific`)
✅ Fixed-point notation (`std::fixed`)
✅ Custom precision (`std::setprecision(n)`)
✅ Sign control (`std::showpos`)
✅ Field width and alignment (`std::setw`, `std::left/right`)
✅ Special values (zero, infinity, NaN)
✅ Arbitrary precision (no limit!)

## Need More Details?

📖 Read the full API documentation: `DECIMAL_CONVERSION_API.md`
📋 See implementation details: `IMPLEMENTATION_SUMMARY.md`
🔬 Check the test suite: `decimal_converter_test.cpp`

## Quick Reference

| Function | Purpose |
|----------|---------|
| `to_decimal_string(value<>, flags, prec)` | Convert value<> to string |
| `to_decimal_string(blocktriple<>, flags, prec)` | Convert blocktriple<> to string |
| `operator<<(ostr, value<>)` | Stream insertion for value<> |
| `operator<<(ostr, blocktriple<>)` | Stream insertion for blocktriple<> |

## Common Flags

```cpp
std::ios_base::scientific   // Use scientific notation
std::ios_base::fixed        // Use fixed-point notation
std::ios_base::showpos      // Show '+' for positive numbers
std::ios_base::uppercase    // Use 'E' instead of 'e'
```

## Example Output

```cpp
value<52> x(123.456);

to_decimal_string(x, std::ios_base::scientific, 3)  // "1.235e+02"
to_decimal_string(x, std::ios_base::fixed, 2)       // "123.46"
to_decimal_string(x, std::ios_base::showpos, 6)     // "+123.456"
```

---

**Questions?** Check the full documentation or open a GitHub issue.
