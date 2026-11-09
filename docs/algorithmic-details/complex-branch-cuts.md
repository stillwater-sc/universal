# Complex functions: branch cuts, signed zeros and C99 Annex G

The inverse functions of complex analysis, `log`, `sqrt`, `asin`, `acos`, `atan` and their
hyperbolic counterparts, have more than one value. A library has to pick one, the *principal
value*. Wherever that choice jumps from one sheet of the function to another, the function is
discontinuous; that line is called the *branch cut*. This page explains where the cuts lie, why
IEEE 754's signed zero decides which side of a cut a point on it belongs to, and what C99's
Annex G says about zeros, infinities and NaNs. It then records how Universal's complex library
implements all of this in an arbitrary element type `T`, and how that is verified.

- Headers: `include/sw/universal/math/complex/complex_functions.hpp` (the kernels and the dispatch),
  `complex_native_real.hpp` (Annex F `hypot` and `atan2`, pi in `T`, zero-sign-safe helpers),
  `complex_operators.hpp` (`abs`, `arg`, `polar`, classification)
- Regression tests: `static/complex/math/precision.cpp`, `static/complex/math/special_values.cpp`
- API page: [complex](../number-systems/complex.md)
- Tracking: #2026 (PR #2030)

## History

**Kahan, 1987.** William Kahan's paper "Branch Cuts for Complex Elementary Functions, or Much
Ado About Nothing's Sign Bit" (in *The State of the Art in Numerical Analysis*, eds. A. Iserles
and M. J. D. Powell, Clarendon Press, Oxford, 1987, pp. 165-211) set out the cuts that are used
today and, with them, the argument for signed zero. IEEE 754-1985 had given floating point a
`-0` distinct from `+0`. Kahan showed what it buys complex arithmetic. A cut is a line in the
plane. A point exactly on it has two plausible values, one from each side. If the component that
is zero along the cut carries a sign, `+0` and `-0` can name the two sides. Then every function
is continuous up to its cut from either side, and identities such as
`sqrt(conj(z)) == conj(sqrt(z))` hold everywhere, the cut included. Without the sign, one side of
every cut gets the "wrong" value, and such identities fail exactly on the cut. The paper also gave
formulas that deliver these values accurately in floating point; those formulas are what this
library implements (see [Choosing the formulas](#choosing-the-formulas)).

**C99, Annex G.** ISO C99 added `<complex.h>` and, in Annex G, "IEC 60559-compatible complex
arithmetic": the cuts of Kahan's paper, the ranges of the principal values, and a table of
results for every function at zeros, infinities and NaNs. An implementation that defines
`__STDC_IEC_559_COMPLEX__` conforms to it. Annex G works with Annex F, which specifies the same
things for the real functions (`atan2(+0, -0) == pi`, `hypot(inf, NaN) == inf`, and so on). Annex G
states each function's values for one quadrant only. The rest follow from two symmetries every
function obeys:

- `f(conj(z)) == conj(f(z))` for all of them;
- `f(-z) == -f(z)` for the odd ones (`sin`, `tan`, `asin`, `atan`, `sinh`, `tanh`, `asinh`, `atanh`),
  and `cos(-z) == cos(z)`, `cosh(-z) == cosh(z)` for the even ones.

The trigonometric functions are defined through the hyperbolic ones: `sin(z) = -i sinh(iz)`,
`cos(z) = cosh(iz)`, `tan(z) = -i tanh(iz)`, `asin(z) = -i asinh(iz)`, `atan(z) = -i atanh(iz)`.
In a handful of cases Annex G deliberately leaves the sign of a result component unspecified:
`exp(-inf + i inf)` is `+-0 +- i0`, for example.

**C++.** `std::complex<T>` is defined only for `float`, `double` and `long double`. Since C++11 its
inverse functions are specified to behave as the corresponding C functions, cuts included. How
closely a C++ library follows Annex G at the special values varies. glibc's complex functions
implement it, and Universal's test compares against them.

## The cuts and the ranges

Each cut runs along an axis, where one component is zero, so the sign of that zero picks the side.

| Function | Branch cut(s) | Range of the principal value |
|----------|---------------|------------------------------|
| `log`    | real axis, `(-inf, 0]` | `-pi <= Im <= pi` |
| `sqrt`   | real axis, `(-inf, 0)` | `Re >= 0` |
| `pow(z, w) = exp(w log z)` | as `log` | |
| `asin`   | real axis, `(-inf, -1)` and `(1, inf)` | `-pi/2 <= Re <= pi/2` |
| `acos`   | real axis, `(-inf, -1)` and `(1, inf)` | `0 <= Re <= pi` |
| `atan`   | imaginary axis, `(-i inf, -i)` and `(i, i inf)` | `-pi/2 <= Re <= pi/2` |
| `asinh`  | imaginary axis, `(-i inf, -i)` and `(i, i inf)` | `-pi/2 <= Im <= pi/2` |
| `acosh`  | real axis, `(-inf, 1)` | `Re >= 0`, `-pi <= Im <= pi` |
| `atanh`  | real axis, `(-inf, -1)` and `(1, inf)` | `-pi/2 <= Im <= pi/2` |

```text
     log, sqrt               asin, acos, atanh             atan, asinh
                                (acosh: (-inf, 1))

         Im                          Im                          Im
         |                           |                           #
         |                           |                           #
  =======+------ Re        =====+----+----+===== Re      --------+-------- Re
         0                     -1    0    1                      #
                                                                 #
   cut: (-inf, 0]          cuts: (-inf, -1), (1, inf)    cuts: |Im| > 1 on the
                                                         imaginary axis
```

On a cut, `+0` means the point is approached from the upper half-plane (or, for the cuts on
the imaginary axis, from the right), and `-0` from the other side:

| Point on the cut | Value |
|------------------|-------|
| `sqrt(-4 + 0i)`  | `0 + 2i` |
| `sqrt(-4 - 0i)`  | `0 - 2i` |
| `log(-2.5 + 0i)` | `log 2.5 + i pi` |
| `log(-2.5 - 0i)` | `log 2.5 - i pi` |
| `asin(2.5 + 0i)` | `pi/2 + i acosh(2.5)` |
| `asin(2.5 - 0i)` | `pi/2 - i acosh(2.5)` |
| `atanh(2.5 + 0i)` | `atanh(0.4) + i pi/2` |
| `atanh(2.5 - 0i)` | `atanh(0.4) - i pi/2` |
| `atan(+0 + 2.5i)` | `+pi/2 + i atanh(0.4)` |
| `atan(-0 + 2.5i)` | `-pi/2 + i atanh(0.4)` |

The sign of a zero is therefore part of the input. An arithmetic that loses it, for instance by
computing `0 * -1` as `+0`, moves a point to the other side of the cut, and the result flips
from one value to the other.

## Special values

These are representative cases from Annex G (the full table is in the standard, G.6). In the
table, `x` stands for a finite nonzero value and `y` for a finite value. Like the standard, the table
gives each case for a positive-signed imaginary part. `f(conj(z)) == conj(f(z))` gives the other
half-plane, so `sqrt(-inf - iy)` is `+0 - i inf`.

| Function | Input | Result |
|----------|-------|--------|
| `exp`  | `+-0 + i0` | `1 + i0` |
| `exp`  | `+inf + i0` | `+inf + i0` |
| `exp`  | `-inf + iy` | `+0 cis(y)`, for finite `y` |
| `exp`  | `x + i inf` | `NaN + iNaN` (invalid) |
| `exp`  | `+inf + iNaN` | `+-inf + iNaN` (sign of the real part unspecified) |
| `log`  | `-0 + i0` | `-inf + i pi` (divide-by-zero) |
| `log`  | `+0 + i0` | `-inf + i0` |
| `log`  | `-inf + i inf` | `+inf + i 3pi/4` |
| `log`  | `NaN + i inf` | `+inf + iNaN` |
| `sqrt` | `x + i inf` | `+inf + i inf`, also for `x` NaN |
| `sqrt` | `-inf + iy`, `y` positive-signed | `+0 + i inf` |
| `acos` | `+-0 + iNaN` | `pi/2 + iNaN` |
| `acosh`| `+-0 + i0` | `+0 + i pi/2` |
| `atanh`| `+1 + i0` | `+inf + i0` (divide-by-zero) |
| `atanh`| `+inf + iy`, `y` positive-signed | `+0 + i pi/2` |
| `tanh` | `+inf + iy` | `1 + i0 sin(2y)` |
| `cosh` | `+inf + iy` | `+inf cis(y)`, for nonzero `y` |

Two patterns explain most of the table. An infinity whose direction is known produces an
infinity or a zero with that direction: `exp(-inf + iy)` is a zero in the direction of `cis(y)`.
A NaN in one component poisons the other, unless the result is the same for every value the NaN
could stand for: `sqrt(NaN + i inf)` is `+inf + i inf` whatever the real part was.

## Choosing the formulas

The textbook definitions are exact in real arithmetic. In floating point, several of them lose
accuracy, or pick the wrong side of a cut. Kahan's paper gives replacements, and the library
uses them.

**`asin`, `acos`, `asinh`, `acosh`.** The textbook `asin(z) = -i log(iz + sqrt(1 - z^2))` forms
`1 - z^2`. That cancels catastrophically near `z = +-1`. Squaring also loses the sign of a zero
imaginary part, which sends points on the cut to the wrong side. Kahan keeps the two factors
apart, `s1 = sqrt(1 - z)` and `s2 = sqrt(1 + z)`, so each principal square root takes its own
cut correctly:

```text
asin(z):  Re = atan(x / Re(s1 s2)),              Im = asinh(Im(conj(s1) s2))
acos(z):  Re = 2 atan(Re s1 / Re s2),            Im = asinh(Im(conj(s2) s1))
acosh(z): Re = asinh(Re(conj(sqrt(z-1)) sqrt(z+1))),  Im = 2 atan(Im sqrt(z-1) / Re sqrt(z+1))
asinh(z) = -i asin(iz)
```

`acos(z) = pi/2 - asin(z)` would be shorter, but as `z` approaches 1 the result goes to zero,
and the subtraction discards its leading bits. The library still uses that identity, but only
for non-finite inputs, where it yields the exact Annex G values.

**`sqrt`.** `sqrt(z) = sqrt((|z| + x)/2) + i sign(y) sqrt((|z| - x)/2)` cancels in `|z| - x` when
`y` is small and `x > 0`. Instead the library computes `t = sqrt((|x| + |z|)/2)`, which never
cancels, and obtains the other component as `y / (2t)`. Which component gets `t` depends on the
sign of `x`.

**`tanh`, and through it `tan`.** `sinh(z) / cosh(z)` overflows for large `|x|` and loses accuracy
next to the poles. For `|x| <= 20` the library uses Kahan's form:

```text
t = tan y, s = sinh x, beta = 1 + t^2, rho = sqrt(1 + s^2)
tanh z = (beta rho s + i t) / (1 + beta s^2)
```

For `|x| > 20`, `cosh 2x` dwarfs `cos 2y`, and the library uses
`Re = sign(x) (1 - (exp(-2|x|) + cos 2y) / (cosh 2|x| + cos 2y))` and
`Im = sin 2y / (cosh 2x + cos 2y)`. Past `|x| = 350` the real part is `+-1` to every precision
carried, and the imaginary part is a signed zero.

**`log`.** `log|z|` keeps only absolute accuracy when `|z|` is close to 1, because the result is
small. For `0.5 < |z| < 2` the library computes `log1p((x - 1)(x + 1) + y^2) / 2` instead. With
`ereal`, whose products and sums are nearly exact, this recovers the full relative accuracy. With
`dd` and `qd` it helps, but the sum still rounds, so very close to `|z| = 1` they keep only the
absolute accuracy.

**`atanh`, and through it `atan`.**

```text
Re = log1p(4x / ((1 - x)^2 + y^2)) / 4,    Im = atan2(2y, (1 - x)(1 + x) - y^2) / 2
```

The squares overflow long before `|z|` reaches the end of the range. With `float` limbs that
happens past about 1.8e19. The library therefore computes `(1 - x)^2 + y^2` as
`hypot(1 - x, y)^2`, dividing by `hypot` one factor at a time, and divides both `atan2` arguments
by `max(|x|, |y|)` first. Nothing is squared at full size, and nothing is approximated. A first
version switched to the asymptotic `1/z +- i pi/2` past `|z| = 1e150`. That cutoff is already
infinite with `float` limbs, and at wide `float`-limb configurations the asymptotic series is not
accurate enough.

**`abs`.** `sqrt(x^2 + y^2)` overflows for components past the square root of the largest
value. `hypot` factors out the larger magnitude, `a sqrt(1 + (b/a)^2)`. The double path uses
`std::hypot` for the same reason.

## How Universal implements it

`sw::universal::complex<T>` serves every element type through one header set, and the trait
`is_high_precision<T>` chooses one of two paths:

- **In `T`** (ereal, efloat, dd, qd). Every function is computed from `T`'s own real functions,
  and so carries `T`'s precision.
- **Through `std::complex<double>`** (every other type). Each component is converted to `double`,
  and the platform's function is called. For types of 53 bits or fewer, this is as precise as
  `T` or more.

The path in `T` has to work for element types whose real math libraries differ in how carefully
they treat the edge cases. Three rules make it independent of them:

1. **Non-finite inputs are dispatched case by case**, following Annex G, before any real function
   is called. `T`'s `exp`, `sinh`, `atan2` and the rest only ever see finite arguments. `hypot`
   and `atan2` themselves come from `complex_native_real.hpp`, which implements their Annex F
   special values and calls `T`'s `atan2` only for finite, nonzero arguments.
2. **Signed zeros are formed explicitly.** `dd` and `qd` used to drop the sign of a zero in
   their arithmetic: `0 * -1` gave `+0`, and so did `sin(-0)`. Before this was handled,
   `atanh(2.5 - 0i)` in `dd` came out on the upper side of its cut. The kernels form every
   product, sum, quotient and odd function that can meet a zero through helpers (`mul`, `add`,
   `sub`, `div`, `odd_sin`, `odd_tan`, `odd_sinh`, `odd_asinh`, `odd_log1p`). These give the
   IEEE sign of a zero result, whatever `T`'s arithmetic does with it. #2032 then fixed `dd` and
   `qd` themselves, so for them the helpers now agree with the arithmetic; they stay for any
   other element type.
3. **Extreme arguments are scaled before `T`'s `atan2`.** `atan2` is scale invariant, but `dd`'s
   and `qd`'s implementations used to square their arguments, so `atan2(2e100, -1e200)`
   overflowed and returned -8e15 (fixed in #2032). `atan2_annexf` divides both arguments by the
   larger magnitude when it lies above 1e100 or below 1e-100, so a type's `atan2` never needs to
   cope with extreme arguments.

Pi and its fractions are computed once per type, in `T`, as `4 atan(1)`. As a result, the angles
in the special values, such as `3pi/4` for `log(-inf + i inf)`, carry `T`'s precision too.

A type opts into the path in `T` with one specialization, provided its real math library has
`sqrt`, `exp`, `log`, `log1p`, `pow`, `sin`, `cos`, `tan`, `sinh`, `cosh`, `asinh`, `atan`,
`atan2`, `signbit`, `isnan`, `isinf`, and `std::numeric_limits` infinity and NaN:

```cpp
namespace sw { namespace universal {
template<> struct is_high_precision<MyType> : std::true_type {};
}}
```

## Verification

`static/complex/math/precision.cpp` compares each function with mpmath references at
`0.75 + 1.25i` and `-2.5 - 0.375i`. Both points lie off every cut, where mpmath's principal values
are C99's. There are 330 digits per value, generated by `generate_reference_constants.py --complex`.
A `dd`, `qd` or `ereal` result is converted to an `ereal` without rounding and compared as a dyadic
rational. The worst agreement over all 16 functions, `abs`, `arg`, `pow` and the round trips is:

| Type | Holds (bits) | Worst agreement (bits) |
|------|-------------:|-----------------------:|
| `ereal<8>` | 424 | 475 |
| `qd` | 212 | 212 |
| `dd` | 106 | 102 |
| `efloat<8>` | 256 | 252 |
| `cfloat<64,11>`, double path | 53 | 49 |

The same test checks `atanh(x)` and `atan(x + i)` at `x` = 2^65, 1e100, 1e160 and 1e300 (2^65,
1e30 and 1e37 for `ereal<4, float>`), where the squares would overflow.

`static/complex/math/special_values.cpp` runs two checks, for `ereal<4>`, `ereal<4, float>`, `dd`,
`qd` and `efloat<4>`:

- A table of Annex G values and the cut sides above. It runs on every platform.
- Where the C library is glibc, all 16 functions on a 10 x 10 grid of `{-inf, -2.5, -1, -0, +0,
  0.5, 1, 2.5, +inf, NaN}`, against `std::complex<double>`. The signs of zeros, infinities and
  NaNs are compared too, read in `T` itself rather than through `double` (a `dd` holding `-0`
  converted to `+0` before #2032). Seventeen cases are skipped, the ones where Annex G leaves a sign unspecified and
  glibc picks the other sign (for example `exp(-inf - i inf)`, `sinh(-inf + i inf)`). All
  other cases match.

One case worth noting: `acosh(+-0 + iNaN)` is `NaN + i pi/2` in glibc and here. That follows
from `acosh(z) = +-i acos(z)` and the Annex G value `acos(+-0 + iNaN) = pi/2 + iNaN`.

## Open

- Very close to `|z| = 1`, `log` in `dd` and `qd` keeps only absolute accuracy (see above).
- The double path inherits the platform's `std::complex<double>`, and with it the platform's
  conformance to Annex G.
