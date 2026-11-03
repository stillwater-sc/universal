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

### Division by zero

By default a division by zero throws `erational_divide_by_zero`. A caller that would rather
get a value opts out before the include, with `ERATIONAL_THROW_DIVIDE_BY_ZERO 0`, or with
`ERATIONAL_THROW_ARITHMETIC_EXCEPTION 0` when the first is not set:

```cpp
#define ERATIONAL_THROW_DIVIDE_BY_ZERO 0
#include <universal/number/erational/erational.hpp>
```

With the exception off, `x / 0` is NaN: the 0/0 state that `isnan()` reports. Arithmetic
carries a NaN along, and a NaN divisor gives a NaN without throwing, in either mode. Any
operation that leaves a zero denominator, such as `setdenominator(0)` followed by
arithmetic, settles on 0/0 as well. Nothing is printed (#1894).

## Related

- [`rational`](/universal/number-systems/rational/) -- fixed-width rational, when the ratio is bounded
- [`einteger`](/universal/number-systems/einteger/) -- the integer it is built from
- [`ereal`](/universal/number-systems/ereal/) -- when the value is not rational but you still want many digits
