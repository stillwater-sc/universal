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

`erational` is a non-template class: a sign flag, and a numerator and denominator held as
non-negative [`edecimal`](/universal/number-systems/edecimal/) magnitudes, normalised by their GCD
after every arithmetic operation.

| Property | Value |
|---|---|
| Representation | sign + adaptive numerator / adaptive denominator |
| Arithmetic | exact for `+ - * /` |
| Overflow | none |
| Normalisation | reduced by GCD after arithmetic; the sign is a separate flag, never in the digits |

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

A build with exceptions disabled (`-fno-exceptions`) gets the value without asking:
`ERATIONAL_THROW_DIVIDE_BY_ZERO` defaults to `UNIVERSAL_HAS_EXCEPTIONS`, which
`<universal/utility/exceptions.hpp>` takes from the compiler. Setting it to `1` in such a build
stops with an `#error` (#1925).

With the exception off, `x / 0` is NaN: the 0/0 state that `isnan()` reports. Arithmetic
carries a NaN along, and a NaN divisor gives a NaN without throwing, in either mode. Any
operation that leaves a zero denominator, such as `setdenominator(0)` followed by
arithmetic, settles on 0/0 as well. A NaN converts to integer `0` and to the IEEE NaN.
Nothing is printed (#1894).

### Signs, comparison and integer conversion

`erational(n, d)` takes two integers of any sign and stores `-3/2` for both `(-3, 2)` and
`(3, -2)`. It does not reduce: `erational(8, 18)` keeps `8/18`.

The comparison operators compare values. The sign orders them, `+0 == -0`, an unreduced
`2/4` equals `1/2`, and a NaN is unordered: every comparison with it is false except `!=`.

`int(x)` and the other signed conversions give the integer part, truncated toward zero, with
its sign: `int(erational(-7, 2))` is `-3`. The unsigned conversions give that same value
wrapped modulo 2^n, as a C++ signed integer converts (#1908).

### Zero, increment, non-finite values and `toPair`

- **Zero has no sign.** Every operation leaves `+0`: `erational(0) / erational(-3)`, `-erational(0)`
  and `x - x` all give `0/1` (#1990).
- **Increment and decrement.** `++` and `--` add and subtract one as a value: `1/2` becomes `3/2`,
  and `-3/2` becomes `-1/2`.
- **Converting from float, double or long double.** A finite value converts exactly. erational has
  no infinity, so NaN and +/-inf both convert to the 0/0 NaN, which converts back to the IEEE NaN.
- **`toPair()`.** It returns `{numerator, denominator}` as `int64_t`, with the sign on the numerator:
  `-3/2` is `{-3, 2}`. A part beyond the int64 range saturates to `INT64_MIN` or `INT64_MAX` instead
  of wrapping, and the NaN is `{0, 0}`.

## Related

- [`rational`](/universal/number-systems/rational/) -- fixed-width rational, when the ratio is bounded
- [`edecimal`](/universal/number-systems/edecimal/) -- the integer it is built from
- [`ereal`](/universal/number-systems/ereal/) -- when the value is not rational but you still want many digits
