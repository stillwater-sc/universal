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
