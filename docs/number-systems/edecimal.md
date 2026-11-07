# edecimal: Adaptive-Precision Decimal Integer

## Why

Binary integers are the right representation for arithmetic and the wrong one for
inspection: converting a large binary value to decimal for display costs a division per
digit. `edecimal` stores the value in base 10 to begin with, so printing is free and every
intermediate is exactly the number a human would write down.

That makes it the natural type for teaching material, for decimal-exact bookkeeping, and
for cross-checking binary arithmetic against an independent representation.

```cpp
#include <universal/number/edecimal/edecimal.hpp>
using namespace sw::universal;

edecimal a, b;
a = 123456789;
b = 987654321;
std::cout << a * b << '\n';   // 121932631112635269
```

## What

`edecimal` is a non-template class holding a sign and a vector of decimal digits that
grows with the value. There is no overflow.

| Property | Value |
|---|---|
| Base | 10 |
| Width | grows with the value |
| Overflow | none |
| Representation | sign + decimal digit vector |

Arithmetic, comparison, parsing and printing are provided, along with `sqrt` and the
number-theoretic helpers used by the educational examples.

### Division by zero

`edecimal` has no NaN or infinity to carry a failed division in the value, so by
default a division by zero throws `edecimal_integer_divide_by_zero`. A caller that would rather get a value
opts out before the include, with `EDECIMAL_THROW_DIVIDE_BY_ZERO 0`, or with
`EDECIMAL_THROW_ARITHMETIC_EXCEPTION 0` when the first is not set:

```cpp
#define EDECIMAL_THROW_DIVIDE_BY_ZERO 0
#include <universal/number/edecimal/edecimal.hpp>
```

A build with exceptions disabled (`-fno-exceptions`) gets the value without asking:
`EDECIMAL_THROW_DIVIDE_BY_ZERO` defaults to `UNIVERSAL_HAS_EXCEPTIONS`, which
`<universal/utility/exceptions.hpp>` takes from the compiler. Setting it to `1` in such a build
stops with an `#error` (#1925).

With the exception off, `x / 0` and `x % 0` are `0`, as are both fields of
`decint_divide()`. Nothing is printed, and the long division never runs with a zero divisor
(#1894).

### Zero, conversion and parsing

- **Zero has one form.** It has a single `0` digit and a positive sign. Negation, the remainder of
  an exact division (`-6 % 3`) and shifting zero (`0 << 3`) all give that zero, so they compare
  equal to `edecimal(0)` (#1989).
- **Integer semantics.** `/` and `%` truncate toward zero, and the remainder takes the sign of the
  dividend, as they do for native integers. `<<` and `>>` multiply and divide by powers of ten,
  and `>>` truncates toward zero.
- **Converting from float, double or long double.** The value is truncated toward zero. NaN and
  +/-inf convert to `0`, as they do for `einteger` and `erational` (#1989).
- **Parsing.** `parse()` accepts integers, decimal points and exponents when the value is an exact
  integer (`"3.14e2"` is 314). Any digits left after the decimal point, even zeros, are rejected:
  `"3.14"`, `"3.0"` and `"10.50e0"` all fail, so the input's digits are preserved exactly (#865).

## Related

- [`einteger`](/universal/number-systems/einteger/) -- the binary equivalent, faster to compute with
- [`dfloat`](/universal/number-systems/dfloat/) -- fixed-size IEEE 754-2008 decimal floating-point
- [`dfixpnt`](/universal/number-systems/dfixpnt/) -- fixed-size decimal fixed-point
