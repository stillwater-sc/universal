# efloat: Adaptive-Precision Multi-Digit Float

## Why

[`ereal`](/universal/number-systems/ereal/) reaches high precision as an expansion of
ordinary floating-point limbs, which is fast but bounds how many limbs are safe. `efloat`
takes the other route: a single significand of arbitrary length with its own exponent, in
the style of a classical multi-precision float. That removes the limb-count ceiling, so
the precision you can request is limited by memory rather than by the exponent range of a
host type.

It is the type to reach for when you want a thousand digits of a constant and do not care
what it costs.

```cpp
#include <universal/number/efloat/efloat.hpp>
using namespace sw::universal;

efloat<1024> x(2);
std::cout << sqrt(x) << '\n';
```

## What

```cpp
template<unsigned nlimbs = 1024>
class efloat;
```

`nlimbs` sizes the significand. The type carries a sign, an exponent, and a limb vector.

| Property | Value |
|---|---|
| Significand | `nlimbs` limbs, set at compile time |
| Exponent | independent, wide |
| Precision ceiling | memory, not a host exponent range |

### Precision

Every efloat value rounds at a precision, which can differ from the type's `nlimbs * 32`
bits. `set_precision(bits)` sets it on a value, and `set_default_precision(bits)` sets the
one a default-constructed efloat starts with.

An operation runs at the wider of its operands' precisions. A value constructed from a
native integer or float carries no precision of its own: it is exact, and it takes its
partner's. Mixed expressions therefore stay at the precision the caller chose:

```cpp
efloat<100> x(1);
x /= 3;
x.set_precision(640);              // 640 of the type's 3200 bits
auto y = x * 3 / efloat<100>(7);   // computed and rounded at 640 bits
auto z = exp(x);                   // so is the math library
```

When neither operand carries a precision, as in `efloat<100>(1) / efloat<100>(3)`, the result
rounds at the default precision: `set_default_precision()` if set, else the type's
`nlimbs * 32` bits, and carries none either; so does `sqrt`, `exp`, `sin` and the rest of the
exp/log, trigonometric and hyperbolic families of such a value. `get_precision()` reports the
precision a value rounds at, and `carries_precision()` whether it has one of its own. An integer
operand of a mixed operation converts exactly, all 64 bits, rather than through `double`.

`+`, `-`, `*` and `/` are correctly rounded at the result's precision. The exp/log,
trigonometric and hyperbolic families, `sqrt` and `cbrt` compute with guard bits past it
and round once.

### Mathematical library

The mathematical library is oracle-grade: `sqrt` and `cbrt` use an adaptive Newton
iteration whose step count follows the requested precision rather than a fixed budget, and
the trigonometric family, `exp`/`log`, and the constants `pi`, `e` and `phi` are all
carried past a thousand digits.

### Known limitations

- `std::complex<efloat>` is not supported.
- `operator<<` is a stub; use the string conversions to render a value.

## Related

- [`ereal`](/universal/number-systems/ereal/) -- expansion-based, faster, limb-bounded
- [`elreal`](/universal/number-systems/elreal/) -- lazy and unbounded, refines on demand
- `applications/precision/adaptive/` -- worked demonstrations
