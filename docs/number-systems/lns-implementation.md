# LNS Implementation in Universal

This page covers the concrete implementation of the Logarithmic Number
System in Universal -- the `lns<>` template, the storage layout, the
value formula, and the operations. For the conceptual background see
[Introduction](../lns/); for the swappable add/sub algorithms see
[Add/Sub Algorithms](../lns-addsub-algorithms/).

## The `lns<>` template

```cpp
#include <universal/number/lns/lns.hpp>

template<unsigned nbits, unsigned rbits, typename bt = uint8_t, auto... xtra>
class lns;
```

| Parameter | Type        | Default     | Meaning                                              |
|-----------|-------------|-------------|------------------------------------------------------|
| `nbits`   | `unsigned`  | (required)  | Total bits in the encoding (sign + exponent)         |
| `rbits`   | `unsigned`  | (required)  | Fractional bits in the exponent (precision)          |
| `bt`      | typename    | `uint8_t`   | Storage block type for the exponent bits             |
| `xtra...` | parameter pack | -- | Optional `Behavior` flag (Saturating or Modulo)      |

Constraints (enforced via `static_assert` at instantiation time):

- `nbits > rbits`: there must be at least one integer bit and the sign bit
- `nbits - rbits < 66`: the integer part of the exponent fits in 64 bits
- `rbits < 64`: the scaling factor `2^rbits` is representable in 64 bits
- `sizeof...(xtra) <= 1`: at most one optional `Behavior` argument

Common instantiations:

```cpp
using sw::universal::lns;

lns<8, 3, std::uint8_t>            // small embedded LNS, 8 bits total, 3 fractional
lns<16, 8, std::uint16_t>          // 16-bit, 8 fractional bits
lns<32, 16, std::uint32_t>         // 32-bit, 16 fractional bits
lns<64, 32, std::uint64_t>         // 64-bit, full half/half split
```

## Storage layout

The `nbits` bits of the encoding are laid out as a sign-magnitude
fixed-point exponent:

```text
  bit nbits-1                                            bit 0
  +----+--------------------+-----------+
  |  s |        integer     | fraction  |
  +----+--------------------+-----------+
       |<-- nbits-1-rbits ->|<- rbits ->|
```

- **Sign bit** (`s`): one bit at the most-significant position. Encodes
  the sign of the *value*, not the sign of the exponent.
- **Exponent field**: the remaining `nbits - 1` bits, interpreted as a
  signed two's-complement integer with `rbits` fractional bits. This is
  a `blockbinary<nbits, bt, Signed>` underneath.
- **Block storage**: the exponent bits are packed into one or more
  blocks of type `bt` (typically `std::uint8_t`, `std::uint16_t`,
  `std::uint32_t`, or `std::uint64_t` depending on the desired hardware
  word width).

Why fixpnt rather than floating-point for the exponent? Three reasons:

1. **Predictable dynamic range.** With a fixed number of integer and
   fractional bits, the dynamic range is exactly `2^(2^integer_bits)`
   and the relative spacing between adjacent values is exactly
   `2^(2^-rbits)`. No subnormal mode, no exponent-of-the-exponent
   overflow, no special floating-point handling for the exponent.

2. **Hardware mappability.** Fixpnt add / subtract maps directly to a
   single integer ALU instruction; there's no significand alignment
   stage, no exponent-of-the-exponent normalization. The whole point
   of LNS is to make multiply-as-add cheap; using fixpnt for the
   exponent keeps it cheap.

3. **Saturation semantics.** The `Saturating` `Behavior` mode (the
   default) clamps overflow to maxpos / maxneg rather than wrapping or
   producing infinity, which is the natural choice for signal-processing
   workloads where graceful degradation matters more than IEEE-style
   exception flags.

## Value formula

```text
value(s, e) = (-1)^s * 2^(e / 2^rbits)
```

where `e` is the integer interpretation of the `nbits-1` exponent bits.
With `rbits` fractional bits, the actual exponent is `e / 2^rbits` -- i.e.,
the encoded integer divided by `2^rbits` to recover the fractional
exponent.

For example, `lns<8, 3>` with exponent bits `0b001.100`:

```text
encoded integer e = 0b1100 = 12
actual exponent   = 12 / 2^3 = 1.5
magnitude         = 2^1.5 ~= 2.828
```

## Compile-time properties

The class exposes a number of useful `static constexpr` members that
downstream generic code can query:

| Member            | Meaning                                                     |
|-------------------|-------------------------------------------------------------|
| `nbits`           | Total bits (template parameter)                             |
| `rbits`           | Fractional bits in the exponent (template parameter)        |
| `behavior`        | `Saturating` or `Modulo` (from `xtra...`)                   |
| `scaling`         | `2^rbits` -- the divisor that converts encoded e -> exponent |
| `min_exponent`    | Smallest representable exponent (most-negative integer)     |
| `max_exponent`    | Largest representable exponent (most-positive integer)      |

The `min_exponent` and `max_exponent` members are particularly useful
for benchmarks and tolerance computations that need to span the actual
representable range of the type rather than a fixed `[-rbits, +rbits]`
window.

## Special values

LNS cannot represent zero with the value formula `value = (-1)^s *
2^exponent` because `log2(0) = -infinity`. Universal's `lns<>` reserves
specific bit patterns for zero, NaN, and infinity:

- **Zero** (`SpecificValue::zero`): encoded as the most-negative
  exponent with sign bit clear. Returned by `iszero()`.
- **Negative zero** (`SpecificValue::minneg` / `maxneg`): same magnitude
  encoding with sign bit set.
- **NaN** (`SpecificValue::qnan` / `snan`): a reserved bit pattern
  (sign bit set + most-significant exponent bit set, when the special
  bits are co-located in the same block). Returned by `isnan()`.
- **Infinity**: `setinf(neg)` produces `+infinity` or `-infinity`. Most
  arithmetic paths produce NaN rather than infinity for out-of-range
  results when `Saturating` mode is active.

The set of representable values is therefore *almost* a power-of-two
geometric progression, with a small fixed set of special-value
exceptions encoded out-of-band.

## Construction and conversion

```cpp
lns<16, 8> a;                  // trivially default-constructed (uninitialized)
lns<16, 8> b(SpecificValue::zero);  // explicit zero
lns<16, 8> c(2.5);             // from double via convert_ieee754
lns<16, 8> d(1);               // from int via convert_signed
lns<16, 8> e("3.14");          // from string via assign

double da = double(c);          // back to double
float  fa = float(c);
```

The IEEE-754 conversion paths use `sw::math::constexpr_math::log2` and
`cm::exp2` (Epic #763) so they are `constexpr`-friendly. The integer
conversion paths use `frexp`-style scaling to compute the closest
representable exponent.

## Multiplication and division: the cheap operations

```cpp
lns<16, 8> a(2.0);
lns<16, 8> b(4.0);

lns<16, 8> c = a * b;   // 8.0
lns<16, 8> d = a / b;   // 0.5
```

Internally these reduce to integer arithmetic on the exponent field plus
a single XOR on the sign bit:

```text
a * b: sign  = sign_a XOR sign_b
       e_out = e_a + e_b   (fixpnt add on the exponent bits)

a / b: sign  = sign_a XOR sign_b
       e_out = e_a - e_b   (fixpnt subtract)
```

In `Saturating` mode, overflow on the exponent add/subtract clamps to
the most-positive / most-negative representable exponent, preserving
the sign. In `Modulo` mode, the exponent wraps -- which is rarely what
you want and exists mostly for hardware-emulation scenarios.

## Powers and roots: even cheaper

```cpp
lns<16, 8> x(3.0);

lns<16, 8> sq = x * x;          // exponent left-shift by 1, ~9.0
// sqrt(x): exponent right-shift by 1 (for non-negative x)
// pow(x, n): exponent multiplied by integer n
```

Integer powers and integer roots are exact in LNS up to the precision
of the exponent fixpnt -- no iterative refinement, no
significand-doubling, just a shift / multiply on the exponent.

## Addition and subtraction: the hard operations

```cpp
lns<16, 8> a(2.0);
lns<16, 8> b(3.0);
lns<16, 8> c = a + b;   // 5.0 -- but how?
```

Addition cannot be done as integer arithmetic on the exponent. It
requires evaluating the Gauss log-add correction
`sb_add(d) = log2(1 + 2^d)` (see [Introduction](../lns/) for the
derivation). That correction is what the [Add/Sub
Algorithms](../lns-addsub-algorithms/) framework computes; the
implementation is selected per-instantiation via the
`lns_addsub_traits<>` customization point.

The default trait selects `DoubleTripAddSub` -- the historical
placeholder which casts to `double`, does native arithmetic, and casts
back. This is correct but slow (one full encode + decode per add).

## Closure and accuracy

The `docs/closure_plots/` directory contains visualizations of the
add / sub / mul / div closure of `lns<8, k>` for `k in {2, 4, 5, 6}`.
These plots show, for every pair of representable lns values, where
the result lands in the representable space -- and where the LNS
encoding rounds an out-of-grid result to the nearest representable
value.

## See also

- [Add/Sub Algorithms](../lns-addsub-algorithms/): the swappable policy
  framework for selecting an `sb_add` / `sb_sub` implementation per
  `lns<>` instantiation
- [Algorithm Tolerance](../lns-tolerance-traits/): the per-algorithm
  log-domain error bound trait and the rbits-aware comparison helper
  used by the regression suite
- The header `include/sw/universal/number/lns/lns_impl.hpp` for the
  full class body
- The regression suite at `static/logarithmic/lns/` for usage examples
  and exhaustive correctness tests
