# erational: Adaptive-Precision Rational

## Why

Every floating-point type rounds. For a long chain of arithmetic on values that are
*actually* rational -- exact linear algebra, geometric predicates, continued fractions,
or a reference answer you will score a floating-point type against -- rounding is the only
error source, and it is avoidable.

`erational` holds a numerator and denominator as adaptive-precision integers and keeps
them reduced. Addition, subtraction, multiplication and division are exact. Nothing is
rounded, and nothing overflows.

```cpp
#include <universal/number/erational/erational.hpp>
using namespace sw::universal;

erational r;
r = 1;
r /= 3;
std::cout << r << '\n';   // 1/3   -- exactly, not 0.333...
```

## What

`erational` is a non-template class: a sign, an [`einteger`](/universal/number-systems/einteger/)
numerator, and an `einteger` denominator, normalised by their GCD after every operation.

| Property | Value |
|---|---|
| Representation | sign + adaptive numerator / adaptive denominator |
| Arithmetic | exact for `+ - * /` |
| Overflow | none |
| Normalisation | reduced by GCD; the sign is carried by the numerator |

The cost is the normalisation: denominators grow quickly under repeated addition, and the
GCD reduction is the dominant expense. This is a correctness type, not a throughput type.

## Related

- [`rational`](/universal/number-systems/rational/) -- fixed-width rational, when the ratio is bounded
- [`einteger`](/universal/number-systems/einteger/) -- the integer it is built from
- [`ereal`](/universal/number-systems/ereal/) -- when the value is not rational but you still want many digits
