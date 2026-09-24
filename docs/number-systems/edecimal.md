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

## Related

- [`einteger`](/universal/number-systems/einteger/) -- the binary equivalent, faster to compute with
- [`dfloat`](/universal/number-systems/dfloat/) -- fixed-size IEEE 754-2008 decimal floating-point
- [`dfixpnt`](/universal/number-systems/dfixpnt/) -- fixed-size decimal fixed-point
