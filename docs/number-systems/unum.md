# unum: Universal Numbers, Type I

## Why

John Gustafson's Type I unum is the format the whole unum programme started from, and it
is the one that takes the boldest position: *a number should know how precise it is.*

Every other floating-point format has a fixed layout, so a value carries no record of how
much of it is meaningful. Type I attaches a **utag** to each value recording how many
exponent and fraction bits it is actually using, plus a **ubit** saying whether the value
is exact or lies strictly between two representable neighbours. Precision becomes a
property of the value rather than of the type, and inexactness is represented rather than
silently assumed.

```cpp
#include <universal/number/unum/unum.hpp>
using namespace sw::universal;

unum<3, 4> a(1.0);
std::cout << a << " uses " << a.nbits_used() << " bits\n";   // 1 uses 11 bits
std::cout << to_binary(a) << '\n';                           // 0b0.01.~.001.0000.0
```

`1.0` occupies 11 bits of a format whose maximum is 32 -- the encoding shrinks to fit the
value. The `~` in the rendering is the ubit.

## What

```cpp
template<unsigned esizesize, unsigned fsizesize, typename bt = std::uint8_t>
class unum;
```

The parameters are *sizes of size fields*, which is the part that trips up every first
reader. `esizesize` is how many bits encode the exponent width, so the exponent may be up
to `2^esizesize` bits; `fsizesize` is the same for the fraction.

For `unum<3, 4>`:

| quantity | value |
|---|---|
| `maxesize` (largest exponent field) | 8 |
| `maxfsize` (largest fraction field) | 15 |
| `utagsize` (ubit + the two size fields) | 8 |
| `maxbits` (widest possible encoding) | 32 |

A value's actual width is whatever it needs, reported by `nbits_used()` -- 11 bits for
`1.0` above, 9 bits for `0.5` in a `unum<2,2>`.

### The ubit

The ubit is the idea the later unum types kept. Set, it means the value is not one of the
representable numbers but the open interval between this one and the next -- so the format
distinguishes "exactly 1.0" from "something between 1.0 and its successor". Arithmetic
propagates it, which is what makes the type honest about inexactness rather than rounding
and moving on.

## Where it fits

| type | what it keeps from Type I |
|---|---|
| `unum` (Type I) | variable width, utag, ubit -- the full idea |
| [`unum2`](/universal/number-systems/unum2/) (Type II) | the interval semantics, on a projective-circle lattice |
| [`posit`](/universal/number-systems/posit/) (Type III) | fixed width, tapered precision; the ubit is dropped |
| [`areal`](/universal/number-systems/areal/) | fixed width, but keeps the ubit |

Type I is the most expressive and the least hardware-friendly: variable-width encodings
are awkward to pipeline, which is exactly the pressure that produced posits. It remains
the right type for exploring what self-describing precision buys you, and
[`areal`](/universal/number-systems/areal/) is the practical middle ground -- a fixed
layout that still carries the ubit.

## Implementation status

Core type, conversions, comparison, arithmetic, `parse()`, `numeric_limits`, the math
library, and `ubound` interval arithmetic with `next_exact`/`prev_exact` lattice
navigation. Arithmetic goes through a `double` intermediate with ubit propagation rather
than operating on the variable-width encoding directly. `unum<2,2>` and `unum<2,3>` are
validated exhaustively.

## Related

- [`areal`](/universal/number-systems/areal/) -- fixed-width faithful float with a ubit
- [`posit`](/universal/number-systems/posit/) -- the hardware-oriented descendant
- [`interval`](/universal/number-systems/interval/) -- interval arithmetic over any type
