# A real with uncertainty bit

In *The End of Error*, John Gustafson adds one bit to a floating-point number, the **uncertainty bit** or **ubit**. With the ubit clear, the value is exactly the number the other bits encode. With the ubit set, the value lies somewhere in the open interval between that number and the next one. A result that does not land on a representable value is then never silently rounded: it is placed in the interval that contains it, and the ubit says so.

Universal has two ubit number systems:

| type | lattice | the ubit marks |
|---|---|---|
| [`areal<nbits, es>`](../number-systems/areal.md) | a float, sign-magnitude | the open interval to the next float |
| [`poxel<nbits, es>`](../number-systems/poxel.md) | a posit, two's complement | the open interval to the next posit |

This tutorial shows what the ubit promises and what it does not. It separates a **flag** (one tile) from an **enclosure** (a pair of tiles, `tile_interval`). It then measures both on five classic problems where IEEE-754 gives a confident wrong answer.

---

## Tiles

The lattice points and the open intervals between them are called **tiles**. Together they cover the real line without gaps or overlap, so every real number converts to exactly one tile, and no conversion ever rounds:

```cpp
#include <universal/number/poxel/poxel.hpp>
using namespace sw::universal;

poxel9 x(0.1);     // (0.09375, 0.1015625), ubit set: 0.1 is not on the lattice
poxel9 y(0.125);   // 0.125, exact
```

Both types count the ubit in `nbits`: an `areal<16,5>` and a `poxel<16,2>` are both 16 bits wide. Overflow and underflow land on open end tiles, `(maxpos, inf)` and `(0, minpos)` and their negatives, so a ubit number never pretends a huge value is maxpos or a tiny one is zero.

## One tile is a flag

Arithmetic on single tiles uses **sticky-flag** semantics. The operation runs on the stored values, the exact result is placed in its tile, and the ubit is set if that result is inexact **or** either operand carried the ubit.

- With **exact** operands, the result tile **contains** the exact result. For both types this is checked exhaustively on small configurations.
- Once an operand is **uncertain**, the ubit stays set, but the tile no longer has to contain the true value:

```cpp
poxel9 third = poxel9(1) / poxel9(3);   // (0.3125, 0.34375), ubit set
poxel9 r     = third * poxel9(3);       // (0.9375, 1),       ubit set -- 1 itself is outside
```

The ubit is an honest *"this is not exact"*. It is not a bound on where the true value lies. In the Muller-Kahan problem below, a single `poxel` tile settles on 100 with its ubit set, while the true limit is 6.

## A pair of tiles is an enclosure

`tile_interval<Tile>` (`universal/utility/tile_interval.hpp`) carries two tiles, the run `[lo, hi]`, in the spirit of Gustafson's *valid*. It works with `poxel` and with `areal` tiles of up to 64 bits, and it is guaranteed to **contain** every result.

It relies on the property above: an operation on two *exact* tiles returns the tile that contains the exact result. Every interval operation is therefore evaluated on exact endpoints only. The one containing tile gives both directed roundings: its lower end bounds the result from below, and its upper end bounds it from above.

- **Open ends stay open.** `(0, minpos) + (0, minpos)` is strictly positive.
- **`sign()`** returns `positive`, `negative`, `zero` or `undecidable`, and never a wrong answer.
- **`cos`** is enclosed by the Taylor partial sums S_30 <= cos t <= S_28, evaluated in tile-interval arithmetic.

```cpp
#include <universal/number/poxel/poxel.hpp>
#include <universal/utility/tile_interval.hpp>
using I = tile_interval<poxel<64, 2, std::uint64_t>>;

I x(1.0e-8);
I g = I(1) - cos(x) * cos(x) - (x * x + x * x) / I(4000);
g.sign();   // positive: g is in (9.5e-17, 1.01e-16); IEEE double reports -5e-20
```

When the format lacks the precision, the honest answer is **undecidable**, and that is what an enclosure reports.

---

## Five problems, measured

The applications in [`applications/precision/ubit`](https://github.com/stillwater-sc/universal/tree/main/applications/precision/ubit) compare rounding formats (float, double, posit), single tiles and tile intervals, and assert the outcome. Formats are compared at **equal storage**, with the ubit counted in the width. Every expected value was checked against exact arithmetic first (#1637).

### 1. Rump's polynomial

f(a, b) = 333.75 b^6 + a^2 (11 a^2 b^2 - b^6 - 121 b^4 - 2) + 5.5 b^8 + a / (2b), at a = 77617, b = 33096.

The exact value is **-54767/66192 = -0.827396059946821**. The polynomial part cancels to exactly -2 from terms near 7.9e36 = 2^123, so more than 120 bits are needed even to get the sign.

| format | result |
|---|---|
| float | about -6e29 to -8e29, depending on how the compiler evaluates it |
| double | about -1.2e21 (-1.18e21 or -1.33e21, depending on fused multiply-add) |
| posit<32,2> | **+1.17**: the wrong sign |
| areal<32,8>, poxel<32,2> single tile | 2.5e30 and 1.17, both with the ubit set |
| tile_interval, 32 and 64 bits | contains -0.8274; sign undecidable |

IEEE double has the correct (negative) sign here; it is wrong by 21 orders of magnitude. Every tile interval contains the true value and declines to name a sign, which is correct at these widths.

### 2. The Muller-Kahan recurrence

x_{n+1} = 111 - (1130 - 3000/x_{n-1}) / x_n, with Kahan's starting values x_0 = 11/2, x_1 = 61/11. The exact solution is x_n = (6^(n+1) + 5^(n+1)) / (6^n + 5^n), which tends to **6**. But 100 is the attracting fixed point, and the smallest rounding error excites it.

| n | exact | float | double | posit<32,2> | poxel<32,2> tile | tile_interval<poxel<32,2>> |
|---|---|---|---|---|---|---|
| 4 | 5.6746 | 5.6721 | 5.6746 | 5.6744 | 5.6747, u=1 | (5.6725, 5.6762) |
| 6 | 5.7491 | 4.9412 | 5.7491 | 5.6828 | 5.7785, u=1 | (4.9699, 6.3555) |
| 8 | 5.8113 | 160.50 | 5.8113 | -19.457 | 13.887, u=1 | (-inf, inf) |
| 14 | 5.9277 | 100 | 15.413 | 100 | 99.99996, u=1 | (-inf, inf) |
| 30 | 5.9958 | 100 | 100 | 100 | 100, u=1 | (-inf, inf) |

Every rounding format settles on 100. The single tile does too: its flag warns, but it is not an enclosure. The tile interval contains the exact iterate at every step, and once it can no longer bound the iterate it widens instead of converging to the wrong answer.

The *End of Error* form of this recurrence starts at v_1 = 2, v_2 = -4 and fails the same way; it is demonstrated in `muller.cpp` in the same directory.

### 3. The sign of det(M^k)

M = [1.61803398875 1; 1 0.61803398875], the golden-ratio matrix with its entries cut to 11 decimals. With the exact golden ratio det(M) would be 0. With these decimals det(M) = +2.351265625e-13 exactly, so det(M^k) = det(M)^k is **positive for every k**, about 1e-631 at k = 50. An orientation predicate needs only the sign.

| k | double | tile_interval, 32-bit | tile_interval, 64-bit |
|---|---|---|---|
| 1 | positive | undecidable | **positive** (areal and poxel) |
| 2 | positive | undecidable | undecidable |
| 5 | 0 (fused) / positive (unfused) | undecidable | undecidable |
| 20 | 0 (fused) / **negative** (unfused) | undecidable | undecidable |
| 50 | 0 (fused) / about +1e18 (unfused) | undecidable | undecidable |

How double fails depends on whether the compiler fuses m00 m11 - m01 m10 into a multiply-add, but it fails with full confidence either way. No tile interval ever asserts a wrong sign.

### 4. The BBP tail

S = sum over k >= 15 of 16^-k (4/(8k+1) - 2/(8k+4) - 1/(8k+5) - 1/(8k+6)) = **9.1e-22**. Is S in (0, 1e-16)?

Float and double answer correctly: 16^-15 = 8.7e-19 is well inside their range. The question bites at 16 bits:

| format (16 bits) | result |
|---|---|
| half | 0: flushed, so it cannot say S > 0 |
| posit<16,2> | 1.4e-17: rounded up to minpos, too large by a factor of 15000 |
| tile_interval<areal<16,5>> | (0, 1.2e-7): proves S > 0 |
| tile_interval<poxel<16,2>> | (0, 2.2e-16): proves S > 0 |
| tile_interval<poxel<16,3>> | (6.4e-22, 1.3e-21): **proves 0 < S < 1e-16** |

With standard parameters, both ubit formats prove S > 0, which half cannot. Neither can bound S below 1e-16, because their smallest tiles end at 1.2e-7 and 2.2e-16, and they say so. The same 16 bits spent as `poxel<16,3>` reach 2^-104 and decide the question. Dynamic range decides it, not an extra bit.

### 5. The Griewank structure near its minimum

g(x, y) = 1 - cos(x) cos(y) - (x^2 + y^2)/4000 at x = y = 1e-8. Expanding cos, g = x^2 (1 - 1/2000) - x^4/3 + ... = **+9.995e-17**.

| format | result |
|---|---|
| float, double, posit<32,2> | **-5e-20**: cos(1e-8) rounds to exactly 1, so the sign is wrong |
| tile_interval, 32-bit | contains g; undecidable |
| tile_interval<areal<64,11>> | (-5e-20, 4.4e-16): undecidable, as its 51 fraction bits are coarser than double's |
| tile_interval<poxel<64,2>> | (9.5e-17, 1.01e-16): **proves g > 0** |

The posit lattice carries 58 fraction bits near 1 at 64 bits, a spacing of 3.5e-18, which is fine enough to see 1e-16.

---

## What the ubit can and cannot do

| capability | IEEE float | one ubit tile (flag) | tile_interval (enclosure) |
|---|---|---|---|
| distinguish exact from approximate | no | yes: ubit = 0 means exact | yes |
| detect precision loss | silent | yes: the ubit is set | yes |
| guarantee the true value is inside | no | no: the tile can miss it | **yes** |
| avoid a confident wrong answer | no | warns, but may still show a wrong value | yes: decides, or says undecidable |
| decide a sign | always, sometimes wrongly | no | when provable |

## Lessons

- **A flag is not an enclosure.** A set ubit means "inexact", not "the truth is in this tile". Guarantees need a pair of tiles.
- **"Undecidable" is a correct answer.** Rump needs more than 120 bits. A 32- or 64-bit enclosure that says "I cannot tell" is right, and every rounding format is wrong.
- **Compare at equal storage, ubit included.** A one-bit mismatch (a 17-bit poxel against a 16-bit areal) flipped the BBP conclusion.
- **How rounding fails depends on the compiler.** Fused multiply-add changed both the digits of Rump's double result and the way double fails det(M^k). The enclosures did not change.
- **A tight enclosure needs a sound formulation.** Summing the BBP terms one at a time near minpos widens the upper bound at every step; factoring out 16^-15 keeps it tight.

## More examples from the book

The same directory has earlier demonstrations with `areal` single tiles:

| example | what it shows |
|---|---|
| `rump.cpp` | IEEE single, double and quad against the double-double, triple-double and quad-double cascades |
| `muller.cpp` | the v_1 = 2, v_2 = -4 recurrence: IEEE converges to 100, the ubit flags uncertainty from iteration 4 |
| `chaotic_bank.cpp` | Balance_n = n * Balance_{n-1} - 1 from e - 1: double goes negative, an impossible balance |
| `quadratic.cpp` | cancellation in b^2 - 4ac flagged by the ubit |
| `thin_triangle.cpp` | Kahan's thin triangle: the cancellation in s - a flagged |
| `newton.cpp` | Newton iteration, with the ubit showing when rounding affects the division |

*The End of Error* also treats Bailey's two-equation system (chapter 14) and the pendulum and two-body problems (chapters 19 and 20), where enclosures give guaranteed bounds on an ODE solution.

## Running the applications

```bash
cmake -DUNIVERSAL_BUILD_APPLICATIONS=ON -DUNIVERSAL_BUILD_NUMBER_POXELS=ON ..
make -C applications/precision/ubit
ctest -R ubit_
```

Each application prints its comparison table and ends with its assertions and PASS or FAIL.

---

Sources:

- John L. Gustafson, *The End of Error: Unum Computing*, CRC Press, 2015
- https://rosettacode.org/wiki/Pathological_floating_point_problems
- https://www.johndcook.com/blog/2019/11/12/rump-floating-point/
- https://ubiquity.acm.org/article.cfm?id=2913029
- https://people.eecs.berkeley.edu/~wkahan/EndErErs.pdf
