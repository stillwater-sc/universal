# efloat: Adaptive-Precision Multi-Digit Float

## Why

[`ereal`](/universal/number-systems/ereal/) reaches high precision as an expansion of
ordinary floating-point limbs, which is fast but bounds how many limbs are safe. `efloat`
takes the other route: a single significand of arbitrary length with its own exponent, in
the style of a classical multi-precision float such as MPFR. That removes the limb-count
ceiling, so the precision you can request is limited by memory rather than by the exponent
range of a host type.

`efloat`'s arithmetic is correctly rounded at any precision in all four rounding modes, and
its math library is designed to be: checked against MPFR from 32 to 8192 bits, it returns no
incorrectly rounded result. It runs at MPFR's speed, ahead of it in every elementary function,
while staying header-only and free of dependencies. It also evaluates
at compile time: arithmetic, the math library and constants such as pi to 1024 bits can be
computed by the compiler into `constexpr` variables.

```cpp
#include <universal/number/efloat/efloat.hpp>
using namespace sw::universal;

efloat<1024> x(2);
std::cout << std::setprecision(30) << sqrt(x) << '\n';   // 1.414213562373095048801688724210e+00
```

## What

```cpp
template<unsigned nlimbs = 1024>
class efloat;
```

`nlimbs` sets the type's nominal width in 32-bit units, `nlimbs * 32` bits: the default
precision, `numeric_limits<efloat<nlimbs>>::digits`, the capacity of `efloat_constant<nlimbs>`
and the size of the inline buffer derive from it. It is not a ceiling on a value:
`set_precision()` accepts more, and the significand then grows onto the heap, so an `efloat<4>`
at `set_precision(256)` carries 256-bit results. The type carries a sign, an exponent and the
significand.

| Property | Value |
|---|---|
| Nominal width | `nlimbs * 32` bits, set at compile time; each value rounds at its own precision, which may differ |
| Exponent | `int64_t`; a finite value's scale lies in `[1 - 2^62, 2^62 - 1]` |
| Storage | 64-bit limbs, least significant first: inside the value up to `EFLOAT_INLINE_LIMBS` (8, i.e. 512 bits), past that in a block from a per-thread cache |
| Allocation | none in steady state: the cache recycles blocks, and the operators write their results in place |
| Rounding | correct rounding in all four modes (`efloat_rounding_mode`), IEEE-754 status flags |
| Special values | +-0, +-inf, quiet and signaling NaN |

`std::numeric_limits<efloat<nlimbs>>` describes the type at its widest precision (#1786):
`digits` is `nlimbs * 32` and `epsilon()` is `2^(1 - digits)`; `min()` and `max()` sit at the
ends of the scale range above, and `infinity()`, `quiet_NaN()` and `signaling_NaN()` are the
special values. The range does not fit in an `int`, so `min_exponent` and `max_exponent`
saturate at `-INT_MAX` and `INT_MAX`. The limits that are powers of two carry no precision of
their own, so `x + epsilon()` stays at `x`'s precision.

The arithmetic keeps to that range. A result past `max()` overflows, and one below `min()`
underflows, as IEEE-754 prescribes in each rounding mode, with no subnormals. Overflow goes to
inf, or to the largest value at the result's precision when the mode rounds toward zero from
it. Underflow goes to 0, or to `min()` when the mode rounds away from zero, and under round to
nearest when the result lies above `min() / 2`. Both raise their exception flags. `fma` forms
its product exactly even when the product alone would leave the range, so
`fma(max(), 2, -max())` is `max()`.

### Precision

Every efloat value rounds at a precision, which can differ from the type's `nlimbs * 32`
bits. `set_precision(bits)` sets it on a value, and `set_default_precision(bits)` sets the
one a default-constructed efloat starts with. Precision is kept in whole 32-bit units, so a
request for 53 bits rounds at 64.

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

`+`, `-`, `*`, `/` and `sqrt` are correctly rounded at the result's precision, in all four
rounding modes, and so is conversion to and from `float`, `double` and `long double`,
subnormals included.

Size the type to the precision you work at: `efloat<4>` for 128 bits rather than
`efloat<1024>` with `set_precision(128)`. A value that carries no precision, such as the result
of `efloat<1024>(1) / efloat<1024>(3)`, rounds at the default, which is the full `nlimbs * 32`
bits unless `set_default_precision()` says otherwise.

### Mathematical library

The mathematical library is designed to be correctly rounded at any precision, in all four
rounding modes:
`exp`, `exp2`, `exp10`, `expm1`, `log`, `log2`, `log10`, `log1p`, `sin`, `cos`, `tan`, `asin`,
`acos`, `atan`, `atan2`, `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh`, `pow`, `tgamma`,
`lgamma`, `erf`, `erfc`, `cbrt`, `hypot` and `fma`; `fmod` and `remainder` are exact. Each
transcendental function evaluates with guard bits past the result's precision and
rounds by Ziv's strategy, evaluating again at a wider precision in the rare case its value lies
too close to a rounding boundary; exact results, such as `exp2(3)` or `pow(9, 0.5)`, are formed
exactly. The retry loop is bounded: after eight retries, at 256 times the target precision, it
rounds its last evaluation, which is faithful but not proven correct. That can happen only for an
exact result within about `2^-(256 P)` of a rounding boundary; no proof shows it never does, so
the guarantee rests on measured error bounds and on the MPFR check below. Results hold out to `2^(2^62)`, far past the range of `double`: `tgamma(100001.5)` is
about `8.93e+456575`. None of these functions go through `double`.

`hypot` and `fma` form their squares, products and sums exactly and round once; `cbrt` takes
an exact integer cube root with remainder; `fmod` and `remainder` divide exactly, even for
quotients of `2^(2^40)` and beyond (#1783).

The constants `efloat_pi<nlimbs>()`, `efloat_e<nlimbs>()` and `efloat_phi<nlimbs>()` are
correctly rounded at the type's precision; ln 2, ln 10 and the others the functions need are
computed once per thread at the widest precision asked for and cached.

`benchmark/accuracy/efloat/mpfr_check.cpp` checks the results against MPFR: every function and
the conversions to `float` and `double`, all four rounding modes, 32 to 8192 bits, with special
arguments per function. It finds no incorrectly rounded result. The `long double` conversions are
checked by `elastic/efloat/conversion/long_double.cpp`.

## Performance

`efloat` was brought to MPFR's speed under Epic #1763 (#1761 to #1833). The arithmetic runs on
limb kernels in `internal/limbs/limb_kernels.hpp`: schoolbook, Karatsuba and Toom-3
multiplication with dedicated squaring and short (high-half) products, Burnikel-Ziegler and
approximate-quotient division, Zimmermann's square root and an exact cube root with
remainder, and fixed-size paths for operands of one to six 64-bit limbs. On x86-64 with BMI2
and ADX, chosen at run time, the inner loops are `mulx`/`adcx`/`adox` inline assembly; elsewhere,
and with `UNIVERSAL_LIMB_KERNELS_NO_ASM` defined, they are portable C++ (`unsigned __int128`, or
`_umul128` on MSVC). The math library evaluates its series in fixed point on those kernels, with
rectangular splitting, argument reduction and AGM for very wide logarithms.

Microseconds per call against MPFR 4.2.2 (GMP 6.3.0) called from C++, Intel Core i7-12700K, one
core pinned, g++ 15.2 `-O3 -march=native`, measured 2026-10-29 by `mpfr_gap`. Each cell is the
efloat time and efloat / MPFR:

| operation | 128 bits | 512 bits | 2048 bits | 8192 bits |
|---|---:|---:|---:|---:|
| `a+b` | 0.012 (0.9x) | 0.021 (1.0x) | 0.033 (0.7x) | 0.080 (0.5x) |
| `a*b` | 0.011 (0.6x) | 0.044 (1.0x) | 0.238 (0.9x) | 2.42 (1.1x) |
| `b/a` | 0.016 (0.6x) | 0.092 (1.1x) | 0.486 (1.0x) | 4.22 (1.0x) |
| `sqrt` | 0.022 (0.4x) | 0.115 (0.9x) | 0.445 (1.0x) | 2.75 (0.9x) |
| `fma` | 0.021 (0.7x) | 0.059 (1.2x) | 0.381 (1.1x) | 3.41 (1.1x) |
| `exp` | 0.54 (0.7x) | 1.35 (0.6x) | 8.48 (0.5x) | 107 (0.6x) |
| `log` | 0.63 (0.6x) | 1.20 (0.4x) | 6.29 (0.4x) | 82 (0.6x) |
| `sin` | 0.61 (0.8x) | 1.56 (0.7x) | 9.61 (0.7x) | 126 (0.6x) |
| `atan` | 1.09 (0.5x) | 2.79 (0.3x) | 16.3 (0.4x) | 178 (0.5x) |
| `pow(b,a)` | 1.31 (0.6x) | 2.86 (0.5x) | 15.9 (0.5x) | 192 (0.6x) |
| `tgamma` | 7.0 (1.0x) | 20.9 (0.6x) | 148 (0.4x) | 2486 (0.3x) |
| `erf` | 2.97 (1.0x) | 9.91 (0.6x) | 74.3 (0.4x) | 1407 (0.5x) |

Across all 35 operations `mpfr_gap` times, efloat is 0.2x to 1.2x of MPFR, and every elementary
function is at or below 1.0x at every one of the four precisions. `mpfr_bands` times the
primitives (`+`, `-`, `*`, `/`, `a/7`, `sqrt`, `fma`) at 20 precisions from 64 to 16384 bits, and
`exp`, `log` and `sin` at the 18 of them up to 8192 bits: no cell is above 1.46x MPFR, under g++
and clang alike. Through gmpy2, the usual way to reach MPFR from Python, each call pays another
~0.04 us on top.

Two habits keep measurements honest. The first call at a new precision fills the constant
caches (ln 2, pi, the Bernoulli numbers of `tgamma`), so time warm calls. And a value
converted from a `double` holds one limb whatever the type's precision, so benchmark with
operands that fill their precision.

The MPFR comparisons build when MPFR is found (Debian/Ubuntu: `libmpfr-dev`). MPFR is a
dependency of those three targets only, never of the library, and they are not built in CI or
the release image (`docs/licensing/mpfr-gmp-lgpl.md`):

| target | what it does |
|---|---|
| `benchmark_efloat_mpfr_gap` | 35 operations at 128/512/2048/8192 bits against MPFR |
| `benchmark_efloat_mpfr_bands` | the primitives at 20 precisions (64-16384 bits), exp/log/sin to 8192; `all 1.5` fails on a ratio above 1.5 |
| `accuracy_efloat_mpfr_check` | the correct-rounding check above |
| `benchmark_efloat_mpmath_gap` | efloat alone, no MPFR, with ratio guards that hold on any runner; compare with `mpmath_gap.py` |

The full tables, per release, and the history of each speed-up are in section 7 of [efloat:
correctly rounded arithmetic and math library](../algorithmic-details/efloat-mathlib.md).

## Compile-time evaluation

Since #1819, `efloat` runs in constant expressions: construction from integers and dyadic
values, `+`, `-`, `*`, `/`, the compound operators, the comparisons, `sqrt`, the whole math
library, `parse()`, the `float`/`double` conversions, and the constants pi, e and phi. The
compiler gets the run-time result, bit for bit.

```cpp
static_assert(efloat<4>(3) + efloat<4>(4) == efloat<4>(7));
static_assert(cbrt(efloat<8>(27)) == 3);

// a value of up to 512 bits lives inside the efloat: a constexpr efloat
constexpr efloat<4> pi128 = efloat_pi<4>();
static_assert(pi128 > 3 && pi128 < efloat<4>(22) / 7);

// past 512 bits the limbs would be on the heap: freeze the value in an efloat_constant
constexpr efloat<32> sqrt_of(int v) { efloat<32> x(v); x.set_precision(1024); return sqrt(x); }
constexpr efloat_constant<32> root2 = sqrt_of(2);   // 1024 bits, computed by the compiler

efloat<32> r = root2;                                // the same efloat, at run time
```

A constant evaluation may allocate, but a `constexpr` variable cannot keep what it allocated,
so an `efloat` is a `constexpr` variable only while its limbs fit the inline buffer
(`EFLOAT_INLINE_LIMBS` 64-bit limbs, 512 bits by default). `efloat_constant<nlimbs>` holds the
state, sign, exponent, precision and limbs of an `efloat<nlimbs>` in a fixed array, so it is a
`constexpr` variable at any precision. It compares with efloats and native types, converts to
`float` and `double`, and turns back into the same `efloat`, at run time or in a constant
expression.

What differs in a constant expression:

- **Rounding mode and flags.** These are per thread, which a constant expression cannot read
  or write. A constant evaluation rounds to nearest and raises no status flags.
- **Step limits.** The default limits hold the arithmetic, `sqrt` and the elementary functions
  at moderate precisions, and pi, e and phi through 512 bits. `tgamma`/`lgamma`, and pi, e and
  phi past 512 bits, need a raised limit. Use `-fconstexpr-steps=33554432` for clang,
  `-fconstexpr-ops-limit=67108864` for gcc, or `/constexpr:steps33554432` for MSVC, as the
  `elastic/efloat` tests do.
- **Run-time cost.** None. The constant-evaluation branches fold away, and the run-time kernels,
  assembly included, compile to the same instructions as before.

The tests are `elastic/efloat/api/constexpr.cpp`, `api/constexpr_constant.cpp`,
`math/constexpr_mathlib.cpp` and `conversion/constexpr_parse.cpp`.

## Known limitations

- `std::complex<efloat>` is not supported, because the standard defines `std::complex` only for
  `float`, `double` and `long double`. Use `sw::universal::complex<efloat>`. Its arithmetic,
  `abs`, `arg` and `norm` run at the operands' precision. Its transcendental functions still go
  through `std::complex<double>`, which limits them to double accuracy.
- Values carry no subnormals: the exponent range is wide enough (`2^(2^62)`) that underflow is
  a range limit, not a precision loss.

## Related

- [efloat: correctly rounded arithmetic and math library](../algorithmic-details/efloat-mathlib.md)
  -- the design record: contract, algorithms, error bounds, verification, measured speed, history
- [`ereal`](/universal/number-systems/ereal/) -- expansion-based, limb-bounded
- [`elreal`](/universal/number-systems/elreal/) -- lazy and unbounded, refines on demand
- `applications/precision/adaptive/` -- worked demonstrations
