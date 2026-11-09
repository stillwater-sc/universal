# ereal: Adaptive-Precision Real Numbers

## Why

`double` gives you 53 bits and no way to ask for more. `dd` and `qd` give you two and four
doubles' worth, and then stop. `ereal` lets you choose the precision as a template
parameter and get it -- hundreds or thousands of bits -- while still behaving like an
ordinary value type you can copy, compare, and put in a `std::vector`.

It represents a real number as a **non-overlapping expansion**: an unevaluated sum of
ordinary floating-point limbs, each capturing the rounding error the previous one could
not hold. Arithmetic uses Shewchuk's error-free transformations, so the expansion stays
exact until it runs out of limbs.

```cpp
#include <universal/number/ereal/ereal.hpp>
using namespace sw::universal;

ereal<19> x(2);
std::cout << sqrt(x) << '\n';     // ~303 decimal digits of precision available
```

## What

```cpp
template<unsigned maxlimbs = 8, typename FpType = double>
class ereal;
```

`maxlimbs` is how many limbs the expansion may hold; `FpType` is the type of each limb.

| configuration | significand bits | decimal digits |
|---|---|---|
| `ereal<8, double>` (default) | 424 | 127 |
| `ereal<19, double>` | 1007 | 303 |
| `ereal<5, float>` | 120 | 36 |

### The limb budget is not a style choice

`maxlimbs` is bounded by the limb type, and the bound is enforced with a `static_assert`:

| limb type | `max_safe_limbs` |
|---|---|
| `float` | 5 |
| `double` | 19 |
| x87 `long double` | 255 |
| binary128 `long double` | 144 |

The rule is `-(min_exponent - 1) / digits`. Past it, the last limb falls below the
smallest normal of the limb type, which breaks the non-overlapping property Shewchuk's
arithmetic depends on -- and when that happens `two_sum` and `two_product` silently lose
bits rather than failing. The assertion exists because the failure is quiet.

That is also why the highest-precision configurations want a wide limb: `ereal<24, long
double>` reaches 953 digits of `sqrt(2)` on a host with x87 `long double`, far past what
19 `double` limbs can hold.

Arithmetic results are capped at `min(2 * maxlimbs, max_safe_limbs)` limbs. Inside that
budget the operations are error-free; past it the guarantee is the truncation contract
rather than exactness.

## Using it

```cpp
#include <universal/number/ereal/ereal.hpp>
#include <iostream>

using namespace sw::universal;

int main() {
    using Real = ereal<19>;          // ~303 decimal digits

    Real a(1), b(3);
    Real third = a / b;

    std::cout << sqrt(Real(2)) << '\n';
    std::cout << exp(Real(1))  << '\n';
    std::cout << third         << '\n';
}
```

The math library is complete to the configuration's precision: roots, `exp`/`log`, the
trigonometric and hyperbolic families and their inverses, `fma`, `frexp`/`ldexp`,
`fdim`/`modf`/`rint`/`nearbyint`, plus `std::complex<ereal>` support. Constants are
carried to the precision the type holds rather than to a fixed cap.

At NaN, the infinities and the signed zeros, every function returns what C99 Annex F
specifies. Some examples: `sqrt(-1)` is NaN, `log(+inf)` is +inf, `pow(-0, -1)` is -inf,
`atan2(+0, -0)` is pi, `sin(-0)` is -0 and `ceil(-0.5)` is -0. `fmod` and `remainder` are
exact for quotients of any size, and `fmod(x, 0)` is NaN. Only a build with
`EREAL_THROW_ARITHMETIC_EXCEPTION` set throws instead, as it does for `x / 0`. `hypot`
scales its operands, so `hypot(1e200, 1e200)` does not overflow.

`sin`, `cos` and `tan` keep the configuration's precision at any argument. They reduce it
by Payne and Hanek's method, from as many bits of 1/(2 pi) as the argument's exponent
needs. Those bits are computed once per configuration in `einteger`, so the reduction
covers the whole range of the limb type, including 2^16383 for an 80-bit or 128-bit
`long double`. `sin(ereal<4>(1e200))` is -0.6440 to 282 bits; it used to be -0.3737
(#2016).

`erf`, `erfc`, `tgamma` and `lgamma` are computed in ereal arithmetic. Until #2017 they
went through `double` and gave 53 bits at every configuration.
- **erf:** a series of positive terms. Where erfc falls below the precision, erf is
  computed as 1 - erfc, with erfc from its asymptotic series.
- **erfc:** the asymptotic series where it reaches the precision. Elsewhere it is
  1 - erf computed in a configuration twice as wide, or a continued fraction.
- **lgamma:** Stirling's series, shifted up, with the reflection formula below 0.
- **tgamma:** an exact factorial at a positive integer; otherwise exp(lgamma) with the sign.

Two limits remain:
- **Underflow:** a value near the limb type's floor keeps only the bits above it.
  `erfc(20)`, near 2^-577, has about 490 bits with double limbs.
- **erfc at `ereal<19>`:** there is no wider configuration to run 1 - erf in. For x
  between about 2 and 8, erfc loses the bits that cancellation costs, about 1.44 x^2.

## Choosing between ereal and elreal

| | `ereal` | [`elreal`](/universal/number-systems/elreal/) |
|---|---|---|
| Precision bound | limb budget, fixed at compile time | unbounded |
| Evaluation | eager | lazy |
| Precision set | template parameter | runtime, per value |
| Behaves like | an ordinary value type | an expression that refines on demand |
| Cheaper at | trigonometry | `sqrt`, `exp` |

Pick `ereal` when you know the precision you need. Pick `elreal` when the whole point is
that you do not -- when you are refining until something converges, or generating a
reference that has to out-resolve every fixed type.

## Related

- [`efloat`](/universal/number-systems/efloat/) -- adaptive multi-digit float, a different
  representation of the same idea
- [`dd`](/universal/number-systems/dd/) / [`qd`](/universal/number-systems/qd/) -- two and
  four limbs, fixed, and much faster
- [`quad`, `octo`](/universal/number-systems/standard-types/) -- IEEE binary128/binary256
  when you need a *standard* wide format rather than the most digits per cycle
