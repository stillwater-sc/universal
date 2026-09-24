# positional: Multi-Radix Positional Integer

## Why

[`integer`](/universal/number-systems/integer/) is binary and
[`edecimal`](/universal/number-systems/edecimal/) is decimal. Both are the same idea at a
different radix, and hard-coding the radix into each type means the arithmetic gets
written twice.

`positional` parameterises it: a sign-magnitude integer in *any* radix, with one
implementation of the carry-propagating algorithms serving all of them. That makes it the
natural type for teaching how positional arithmetic works -- showing the same addition in
base 2, base 10 and base 16 -- and for checking radix-specific code against an
independent implementation.

```cpp
#include <universal/number/positional/positional.hpp>
using namespace sw::universal;

positional<10, 10> a;   // 10 digits, radix 10
positional<16,  2> b;   // 16 digits, radix 2
```

## What

```cpp
template<unsigned ndigits, unsigned radix>
class positional;
```

`ndigits` is the number of digit positions; `radix` is the base each digit counts in. The
value is sign-magnitude, so there is no two's-complement wrap and no most-negative-value
asymmetry -- the sign is carried separately from the digits.

| Property | Value |
|---|---|
| Representation | sign + `ndigits` digits in `radix` |
| Width | fixed at compile time |
| Radix | any, set by template parameter |

It is built on `blockdigit`, the internal digit-vector engine that also backs the decimal
types, so a bug fixed in one is fixed in all of them.

### Behavioural switches

Like the other fixed-size integer types, `positional` takes its switches before the header:

- literals in logic and arithmetic operators -- enabled by default
- throwing arithmetic exceptions -- **disabled** by default; the default behaviour is a
  diagnostic on `stderr` for divide and modulo by zero

Both default inside the core header rather than the umbrella, so a translation unit that
includes `core.hpp` directly gets the same behaviour.

## Related

- [`integer`](/universal/number-systems/integer/) -- the binary case, optimised
- [`edecimal`](/universal/number-systems/edecimal/) -- adaptive-width decimal
- [`dfixpnt`](/universal/number-systems/dfixpnt/) -- decimal fixed-point
