# lindex: Level-Index Arithmetic

## Why

A floating-point format overflows. Its range ends at a fixed exponent, and any computation
that passes it, such as a product of large probabilities' reciprocals, a partition
function, or a repeated exponential, has to be rescaled by hand or it becomes inf.

Level-index (LI) arithmetic, introduced by Clenshaw and Olver (1984), removes overflow. It
represents a value by its **generalized logarithm**, a level that counts how many times
you can take a logarithm before the value drops below 1, plus an index that holds the
remainder. Lozier and Olver (1990) prove that the result is closed under + - * /: no
sum, difference, product or quotient of representable values leaves the range. The range
of an 8-bit `lindex` already reaches phi(8), a tower of exponentials past any notation.

Tracking issue: #1349. Design and verification: [docs/level-index-design.md](../level-index-design.md).

## What

`lindex<nbits, lbits = 3, bt = std::uint8_t>`

| Parameter | Meaning |
|---|---|
| `nbits` | total bits, at most 64 |
| `lbits` | level bits: levels 0 .. 2^lbits - 1. The default 3 is closed under + - * / |
| `bt` | storage block type |

`fbits = nbits - 1 - lbits` bits hold the index.

The value map is the generalized exponential and its inverse:

```text
phi(x) = x               0 <= x < 1           psi(X) = X                 0 <= X < 1
phi(x) = e^phi(x - 1)    x >= 1               psi(X) = 1 + psi(ln X)     X >= 1
```

A value X >= 0 is stored as psi(X) = l + f: level l, index f in [0, 1).

| level | X = phi(l + f) covers |
|---|---|
| 0 | [0, 1): plain fixed point |
| 1 | [1, e) |
| 2 | [e, 15.15) |
| 3 | [15.15, 3814279.1) |
| 4 | [3814279.1, 10^1656520.4) |
| 5 .. 7 | beyond any notation shorter than the value map itself |

No IEEE type reaches level 5: psi(DBL_MAX) = 4.632, and psi of the largest binary128 value
is 4.804.

## Encoding

```text
[ s : 1 ][ l + f : lbits integer bits . fbits fraction bits ]
```

- The magnitude is one unsigned fixed-point number l + f.
- A negative value is the two's complement of the whole pattern. phi is increasing, so
  comparing two encodings as signed integers orders them by value.
- Zero is all zeros. NaR is `1000...0`; it equals itself and is below every value, as in
  posit. There is no infinity.

```text
lindex<8,3>
   1:   0b0.000.0001    level 0   index 0.0625   psi 0.0625         value 0.0625
  16:   0b0.001.0000    level 1   index 0        psi 1              value 1
  32:   0b0.010.0000    level 2   index 0        psi 2              value 2.718281828
  64:   0b0.100.0000    level 4   index 0        psi 4              value 3814279.105
 127:   0b0.111.1111    level 7   index 0.9375   psi 7.9375         value phi(7.9375)
```

A value past double prints as `phi(psi)`, the level-index map itself, never as `inf`.
`education/tables/lindex.cpp` writes the full encoding tables.

## How to Use It

```cpp
#include <universal/number/lindex/lindex.hpp>
using namespace sw::universal;

lindex<32, 3> x(1.0e300), y(1.0e300);
lindex<32, 3> z = x * y;               // 1e600: past double, no overflow
std::cout << z << '\n';                // phi(4.68227)
std::cout << exp(exp(exp(z))) << '\n'; // phi(7.68227): exact steps of one level
std::cout << log(z) << '\n';           // 1381.55, exact: one level down
```

`#include <universal/number/lindex/core.hpp>` gives the type with no I/O headers in the
include graph.

## Rounding

Every operation is **correctly rounded**: rounded once, to nearest with ties to even, on the
2^-fbits grid of psi, and saturated at maxpos. This holds for conversion, + - * /, and every
function of the math library.

- Arithmetic never forms an operand value. It runs the Clenshaw-Olver (1987) sequences in
  the psi domain, where every quantity lies in [0, 2]. Multiplication and division go one
  level down: psi(XY) = 1 + psi(phi(x - 1) + phi(y - 1)).
- The working type is double up to 32 bits and dd_cascade above. A result that lands near
  a rounding midpoint is evaluated again one stage wider: double, then dd_cascade, then
  qd_cascade. Products of level-0 operands, which can sit exactly on a midpoint, are exact
  integer arithmetic.

**Error is measured in psi.** One unit in the last place is 2^-fbits in psi at every level.
The value-domain gap it spans grows violently with the level: near the top of level 4 of
`lindex<16,3>` one psi-ulp is a factor of 10^16568. A test that measures value-domain error
reports that as a catastrophic failure; it is the format working as designed.

## Math Library

| function | evaluation |
|---|---|
| `exp` (X >= 0), `log` (X >= 1) | exact: one level up or down on the encoding |
| `sqrt`, `cbrt` | psi(X^(1/n)) = 1 + psi(phi(x - 1) / n) |
| `pow`, `exp2`, `exp10`, `log2`, `log10` | one psi-domain product or quotient one level down |
| `expm1`, `log1p`, `hypot` | compositions on psi points, rounded once |
| `floor`, `ceil`, `trunc`, `round`, `fpclassify`, `min`/`max`, `nextafter` | on the encoding |

The trigonometric, hyperbolic, error and gamma functions follow in #1715. sin, cos and tan of
a value past the double range will be NaR: X mod 2 pi is not computable for X = phi(5).

## Limitations

- **LI underflows.** Level 0 is plain fixed point, so the smallest positive value is
  2^-fbits, 2.4e-4 for `lindex<16,3>`. Anything below half of that rounds to 0, and 1/X
  is 0 for 52.65% of the positive `lindex<16,3>` encodings. The symmetric level-index
  system, SLI (#1350), adds a reciprocal bit to fix this.
- **`lbits = 2` is not closed.** Its last level is 3, and products past it saturate.
- **The only integers a lindex holds exactly are 0 and +-1.** Every other encoding is
  transcendental, so `lindex(2.0)` is the nearest encoding to 2, and pow of a negative
  base is defined only for exponents 0 and +-1.
- Conversion to float or double gives +-inf past that type's range, the honest IEEE answer.
  Integer conversion saturates.

## Verification

Every test compares encodings for equality against an mpmath oracle at 600 bits, never
through double and never by round trip:

| generator | checks |
|---|---|
| `tools/generators/level_index_gen.py` | the shared kernel: 592 pairs per width, every level, psi-ulp error |
| `tools/generators/lindex_conversion_gen.py` | conversion both ways in six configurations |
| `tools/generators/lindex_arithmetic_gen.py` | + - * /: every signed pair of `lindex<8,3>`, sampled 12 to 64 bits |
| `tools/generators/lindex_math_gen.py` | the math library: every `lindex<8,3>` encoding and pair, sampled 16 to 64 bits |

The regression tests are in `static/logarithmic/lindex/`. ucalc registers `lindex8`,
`lindex16` and `lindex32`.

## References

- C. W. Clenshaw, F. W. J. Olver, "Beyond floating point", *J. ACM* 31(2), 1984.
- C. W. Clenshaw, F. W. J. Olver, "Level-index arithmetic operations", *SIAM J. Numer. Anal.* 24(2), 1987.
- D. W. Lozier, F. W. J. Olver, "Closure and precision in level-index arithmetic", *SIAM J. Numer. Anal.* 27(5), 1990.
- C. W. Clenshaw, P. R. Turner, "The symmetric level-index system", *IMA J. Numer. Anal.* 8(4), 1988.

Full citations, with DOIs: [docs/level-index-design.md](../level-index-design.md#sources-and-what-was-verified).
