# einteger: Adaptive-Precision Integer

## Why

`integer<N>` fixes its width at compile time, which is what you want when the number has
to fit a register or an interchange format. When it does not -- factorials, combinatorics,
exact rational arithmetic, cryptographic sizes, or the exact oracles this library scores
its own floating-point types against -- you want an integer that simply grows.

`einteger` allocates limbs as the value needs them. There is no overflow, only memory.

```cpp
#include <universal/number/einteger/einteger.hpp>
using namespace sw::universal;

einteger<std::uint32_t> two(2), p(1);
for (int i = 0; i < 200; ++i) p = p * two;
std::cout << p << '\n';
// 1606938044258990275541962092341162602522202993782792835301376
```

## What

```cpp
template<typename BlockType = std::uint32_t>
class einteger;
```

`BlockType` is the limb type the magnitude is stored in. The value is sign-magnitude, so
the limb array holds the magnitude and the sign rides alongside -- there is no two's
complement wrap, and no most-negative-value asymmetry.

| Property | Value |
|---|---|
| Width | grows with the value |
| Overflow | none |
| Representation | sign + magnitude limb array |
| Default limb | `std::uint32_t` |

Full arithmetic, comparison, and decimal/hex parsing and printing are provided.
`showLimbs()` prints the limb encoding when you need to see the representation.

## Where it is used

`einteger` backs the exact-arithmetic oracles used to validate the floating-point types.
When a test needs to decide whether a `qd` or `ereal` result is correct, it computes the
same quantity as an exact dyadic rational over `einteger` and compares -- so no
floating-point sits between an implementation and its own score.

## Related

- [`integer`](/universal/number-systems/integer/) -- fixed-width, for hardware-shaped values
- [`edecimal`](/universal/number-systems/edecimal/) -- adaptive integer in base 10
- [`erational`](/universal/number-systems/erational/) -- exact ratios of adaptive integers
