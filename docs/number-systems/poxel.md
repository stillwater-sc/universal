# poxel: a Posit with an Uncertainty Bit

## Why

A posit rounds. When a result falls between two lattice values, the posit returns the nearer one and keeps no record that it rounded. Gustafson's Type I unum fixed this for floats with the **ubit**: a trailing bit that says "the value lies in the open interval between this lattice point and the next one". Universal's `areal` is that idea applied to IEEE-style floats.

`poxel` applies it to the posit lattice. A `poxel<nbits, es>` is a `posit<nbits - 1, es>` with a ubit appended. The `nbits` patterns then name every lattice point *and* every open interval between neighbouring lattice points. Together these **tiles** cover the projective real line without gaps or overlap, so every real number converts to exactly one tile, and no conversion ever rounds.

Tracking issue: #1631. Applications: #1637.

## What

`poxel<nbits, es, bt>`

| Parameter | Meaning |
|---|---|
| `nbits` | total bits, including the ubit; at most 64 |
| `es` | exponent size of the underlying posit lattice |
| `bt` | storage block type, default `std::uint8_t` |

Constraints, checked at compile time: `nbits <= 64`, `nbits >= es + 4`, `es < 16`.

| alias | type | lattice | fraction bits at 1.0 | minpos .. maxpos |
|---|---|---|---|---|
| `poxel9`  | `poxel<9, 2, uint16_t>`  | `posit<8, 2>`  | 3  | 5.96e-08 .. 1.68e07 |
| `poxel17` | `poxel<17, 2, uint32_t>` | `posit<16, 2>` | 11 | 1.39e-17 .. 7.21e16 |
| `poxel33` | `poxel<33, 2, uint64_t>` | `posit<32, 2>` | 27 | 7.52e-37 .. 1.33e36 |

## Encoding

Let `P` be the posit pattern of a lattice point, read as a signed integer. The poxel pattern is `T = 2 P + u`:

- `u = 0`: the exact point `P`
- `u = 1`: the open interval `(P, next(P))`

Integer order of `T` is the order of the tiles on the real line, so comparison is integer comparison. Negation is two's-complement negation of `T`, for points and for open intervals alike. The four end tiles are:

| pattern | tile |
|---|---|
| `2 maxpos + 1` | `(maxpos, +inf)` |
| `1` | `(0, minpos)` |
| `-1` | `(-minpos, 0)` |
| `2 NaR + 1` | `(-inf, -maxpos)` |

The pattern `2 NaR` (the sign bit alone) is NaR. Because there are open end tiles, a poxel never overflows to NaR and never underflows to zero. `numeric_limits<poxel>::has_infinity` is false; `infinity()` returns the `(maxpos, +inf)` tile.

## Conversion

Conversion from a native type **truncates** the posit encoding and records whether any bit was dropped. An exactly representable value gives the point tile. Any other value gives the unique open tile that contains it. No real is ever moved to a neighbouring value.

```cpp
poxel9 x(0.1);   // (0.09375, 0.1015625), ubit set
poxel9 y(0.125); // 0.125, exact
```

## Arithmetic: a flag, not an enclosure

Following `areal` (#1631), arithmetic uses **sticky-flag semantics**. The operation runs on the operands' stored lower endpoints, and the exact result is placed in its tile. The ubit is set if that result is inexact **or** either operand carried the ubit. Subtraction is addition of the negated tile: negating the open tile `(L, next)` gives `(-next, -L)`, so `a - b` uses the upper end of an open `b`.

- With exact operands, the result tile **contains** the exact result. The tests check this exhaustively.
- Once an operand is uncertain, the ubit stays set. The tile, however, is not guaranteed to contain the true result of the uncertain computation:

```cpp
poxel9 third = poxel9(1) / poxel9(3); // (0.3125, 0.34375), ubit set
poxel9 r     = third * poxel9(3);     // (0.9375, 1),       ubit set -- 1 itself is outside
```

The ubit is therefore an honest "this is not exact" signal, but a single tile is not an enclosure. For guaranteed containment, carry a **pair** of tiles: a `tile_interval`, below.

Special cases: NaR propagates. Division by exact zero gives NaR, and so does division by the `(0, minpos)` tile, whose representative is zero. With `POXEL_THROW_ARITHMETIC_EXCEPTION` set to 1, both divisions raise `poxel_divide_by_zero`, and a NaR operand raises `poxel_operand_is_nar` or a related exception.

## Enclosures: tile_interval

`tile_interval<Tile>` (`universal/utility/tile_interval.hpp`) is the run of tiles `[lo, hi]`, in the spirit of Gustafson's valid. It works with `poxel` and with `areal` (tiles of up to 64 bits), and it is guaranteed to **contain** every result.

It relies on one property of the tile type: an operation on two *exact* tiles returns the tile that contains the exact result. For `poxel` this is tested exhaustively, and the `tile_interval` tests check it for `areal`. Every interval operation is therefore evaluated on exact endpoints only. The one containing tile then gives both directed roundings: its lower end bounds the result from below, and its upper end bounds it from above.

- Ends that are open stay open. For example, `(0, minpos) + (0, minpos)` is strictly positive.
- `sign()` returns `positive`, `negative`, `zero` or `undecidable`, and never a wrong answer.
- `cos` is enclosed by the Taylor partial sums S_30 <= cos t <= S_28, evaluated in tile-interval arithmetic.
- `sqrt` is enclosed by bisection over the lattice, verified with the same exact-operand property: the tile of t * t contains t^2 exactly. `tile_sqrt` is its single-tile, sticky-flag counterpart.

```cpp
#include <universal/number/poxel/poxel.hpp>
#include <universal/utility/tile_interval.hpp>
using I = tile_interval<poxel<64, 2, std::uint64_t>>;

I x(1.0e-8);
I g = I(1) - cos(x) * cos(x) - (x * x + x * x) / I(4000);
g.sign();   // positive: g is in (9.5e-17, 1.01e-16); double reports -5e-20
```

The applications in `applications/precision/ubit` compare rounding formats, single tiles and tile intervals on Rump's polynomial, the Muller-Kahan recurrence, the sign of det(M^k), the BBP tail and the Griewank structure (#1637), and on the roots of 3x^2 + 100x + 2, the dependency problem (#1646). The tutorial [A real with uncertainty bit](../tutorials/a-real-with-uncertainty.md) walks through them with the measured results.

## API

```cpp
#include <universal/number/poxel/poxel.hpp>
using namespace sw::universal;

poxel17 a(2.0), b = poxel17(1) / poxel17(7);
b.isexact();          // false
b.ubit();             // true
b.lower<double>();    // the tile's lower endpoint
b.upper<double>();    // its upper endpoint (equal to lower for a point)
double(b);            // the lower endpoint
to_interval(b);       // "(lo, hi)" or the exact value; exact hexfloat if long double is too narrow
to_binary(b);         // posit fields, then "|u"
++b;                  // the next tile up
```

## Tests

`static/tapered/poxel`, enabled with `-DUNIVERSAL_BUILD_NUMBER_POXELS=ON`:

- `conversion`: every tile of five small configurations against the independent `posit` lattice (endpoints, order, negation); conversion of every lattice value, its neighbours and the midpoints between them; the end tiles
- `logic`: exhaustive tile order, NaR lowest
- `arithmetic`: exhaustive `+ - * /` for `<8,0>`, `<9,2>` and `<10,1>`, covering containment and the sticky flag
- `api`: the constexpr encoder, special tiles, `numeric_limits`, and exceptions
- `tile_interval`: exhaustive enclosure and tightness over every pair of tiles of `poxel<8,0>`, `poxel<9,2>` and `areal<8,2>`; multi-tile operands; open ends; `cos` against `std::cos`
