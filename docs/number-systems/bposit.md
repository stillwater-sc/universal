# bposit: the Bounded Posit

## Why

A posit's tapered precision is its strength: it spends bits on dynamic range only when a value needs them. At the extremes, though, the regime keeps growing until it has consumed every bit, so near maxpos and minpos a standard posit has **no fraction bits at all**. Its precision falls to zero. The variable-length regime also makes decode and encode hardware depend on the precision.

`bposit` is John Gustafson's **bounded posit (b-posit)**. It caps the regime at a maximum size `rS`. Every value then keeps a guaranteed number of fraction bits, the decoder's critical path is nearly the same at every precision, and the quire is bounded: for fixed `rS` and `eS` it grows by only two bits per bit of precision. The price is a bounded dynamic range.

Sources:

- *Closing the Gap Between Float and Posit Hardware Efficiency*, arXiv 2603.01615
- John L. Gustafson, *Every Bit Counts: Posit Computing*

## What

`bposit<nbits, rs, es, bt>`

| Parameter | Meaning |
|---|---|
| `nbits` | total bits, at most 64 |
| `rs` | maximum regime size, in bits (Gustafson's `rS`) |
| `es` | exponent size, in bits (Gustafson's `eS`); always present in full |
| `bt` | storage block type, default `std::uint8_t` |

Constraints, checked at compile time:

- `2 <= rs < nbits - 1`
- `nbits > 1 + rs + es`: at least one fraction bit at every magnitude
- `nbits <= 64`
- `rs * 2^es <= 2^28`, so every scale, and the sum of two scales in a product, fits an `int`

Standard configurations, Gustafson's `rS = 6, eS = 5`:

| alias | type | fraction bits | scale range | minpos .. maxpos |
|---|---|---|---|---|
| `bposit16` | `bposit<16, 6, 5, uint16_t>` | 4 .. 8 | [-192, 191] | 1.69e-58 .. 6.08e57 |
| `bposit32` | `bposit<32, 6, 5, uint32_t>` | 20 .. 24 | [-192, 191] | 1.59e-58 .. 6.28e57 |
| `bposit64` | `bposit<64, 6, 5, uint64_t>` | 52 .. 56 | [-192, 191] | 1.59e-58 .. 6.28e57 |

## Encoding

```
[ sign | regime: 2..rs bits | exponent: es bits | fraction: the rest ]
```

A negative value is the two's complement of its magnitude's encoding, as for posits. Fields are read from the magnitude.

**Regime.** The regime is a run of identical bits that **counts its terminating bit**:

- A run that is shorter than `rs` ends with one opposite bit. The regime is then the run plus that bit.
- A run that reaches `rs` bits ends there, with **no** terminating bit.

So the regime field is 2 .. `rs` bits long. Its value is `r = run - 1` for a run of ones and `r = -run` for a run of zeros. That gives `r` in `[-rs, rs - 1]`. A standard posit is the special case `rs = nbits - 1`.

**Exponent.** The exponent `e` is always the full `es` bits: the bounded regime never truncates it.

**Fraction.** The fraction `f` is the remaining `nbits - 1 - (regime size) - es` bits.

Value:

```
x = (-1)^sign * (1 + f) * 2^(r * 2^es + e)
```

Derived quantities:

| | formula | `<16, 6, 5>` |
|---|---|---|
| scale range | `[-rs * 2^es, rs * 2^es - 1]` | `[-192, 191]` |
| fewest fraction bits (`F_min`) | `nbits - 1 - rs - es` | 4 |
| most fraction bits | `nbits - 3 - es` | 8 |
| maxpos | `(2 - 2^-F_min) * 2^(rs * 2^es - 1)` | 6.08e57 |
| minpos | `(1 + 2^-F_min) * 2^(-rs * 2^es)` | 1.69e-58 |

minpos is not a power of two. The pattern `0...01` carries the capped all-zeros regime, a zero exponent and a fraction of `2^-F_min`. The exact power of two `2^(-rs * 2^es)` would be the all-zero pattern, which is zero.

**Special values.** As for posits: zero is `0...0`, and NaR is `10...0`. There is no infinity and no negative zero.

**Order.** Encodings are monotonic. Read as two's-complement integers, bit patterns order exactly as their values. Comparison is integer comparison, and `++` / `--` step to the adjacent value.

## Rounding and saturation

These follow the posit standard:

- **Rounding:** round to nearest, ties to the even encoding.
- **Never to 0 or NaR:** a nonzero real never rounds to 0, and a finite real never rounds to NaR.
- **Clamping:** a magnitude above maxpos becomes maxpos, and a nonzero magnitude below minpos becomes minpos. Signs are kept.

## Range and precision profile

| config | `F_min` | max fraction | scale range | minpos | maxpos | decades |
|---|---|---|---|---|---|---|
| `<8,4,0>` | 3 | 5 | [-4, 3] | 0.07031 | 15 | 2.3 |
| `<8,3,1>` | 3 | 4 | [-6, 5] | 0.01758 | 60 | 3.5 |
| `<12,4,2>` | 5 | 7 | [-16, 15] | 1.574e-05 | 6.451e+04 | 9.6 |
| `<16,4,2>` | 9 | 11 | [-16, 15] | 1.529e-05 | 6.547e+04 | 9.6 |
| `<16,6,5>` | 4 | 8 | [-192, 191] | 1.693e-58 | 6.081e+57 | 115.6 |
| `<32,6,5>` | 20 | 24 | [-192, 191] | 1.593e-58 | 6.277e+57 | 115.6 |
| `<64,6,5>` | 52 | 56 | [-192, 191] | 1.593e-58 | 6.277e+57 | 115.6 |

With `rS = 6, eS = 5` the range is the same for every `nbits > 12`. Extra bits buy only precision. For comparison, a standard `posit<32, 2>` spans about 1e-36 .. 1e36, but falls to zero fraction bits at the ends.

## numeric_limits

| member | value |
|---|---|
| `min()` | minpos |
| `max()` | maxpos |
| `lowest()` | -maxpos |
| `epsilon()` | `2^-(nbits - 3 - es)`, the gap above 1.0 |
| `digits` | `nbits - 2 - es`, the significand bits at 1.0 |
| `radix` | 2 |
| `round_style` | `round_to_nearest` |
| `has_infinity` | false |
| `quiet_NaN()` | NaR |
| `min_exponent` / `max_exponent` | `-rs * 2^es + 1` / `rs * 2^es` |

## bposit does not have the TwoSum / TwoProduct property

Bounding the taper guarantees fraction bits, but it does **not** make the roundoff of an operation representable. So TwoSum and TwoProduct are **not** exact error-free transformations for bposit.

**The 8-bit counterexample** (#1251). In `bposit<8, 4, 0>`, minpos is 9/128.

- **TwoSum.** 9/128 + 5/64 = 19/128. That is a tie between 9/64 (encoding 0x09) and 5/32 (0x0A), so it rounds to the even encoding, 5/32. The error, -1/128, is below minpos and not representable.
- **TwoProduct.** 9/128 * -15 = -135/128 rounds to -17/16. The error, 1/128, is not representable.

**Exhaustive counts**, over all pairs of nonzero operands whose exact result lies strictly between minpos and maxpos:

| config | TwoSum: unrepresentable error | TwoProduct: unrepresentable error |
|---|---|---|
| `<8, 4, 0>` | 22832 of 61884 | 43788 of 57000 |
| `<8, 3, 1>` | 11720 of 62924 | 25036 of 53004 |

**The standard configuration fails too.** In `bposit<16, 6, 5>`:

- TwoSum fails for 2872 of the 4096 pairs among the 64 smallest values. For example, (1 + 1/16) 2^-192 + (1 + 1/8) 2^-192 leaves (1/16) 2^-192, below minpos = (17/16) 2^-192.
- TwoProduct's error is unrepresentable for about 0.2% of random pairs at ordinary magnitudes, and about 12% across the full range.

Exact accumulation in bposit therefore comes from the **quire** (below), which always works; compensated summation does not. bposit provides no exact `twosum` / `twoprod`.

## Quire: exact dot products

`quire<bposit<...>>` is the generalized quire, sized by `quire_traits<bposit<nbits, rs, es, bt>>`. Every product of two bposits lands in it exactly; `fdp(x, y)` accumulates `quire_mul(x[i], y[i])` and rounds the sum once with `quire_resolve`, through bposit's own encoder, so the result saturates to +/-minpos or +/-maxpos like any other bposit result, never to 0 or NaR.

The layout puts the smallest product **bit** at bit 0 and maxpos^2 below the top:

| | formula | `<16,6,5>` | `<32,6,5>` | `<64,6,5>` |
|---|---|---|---|---|
| smallest product bit | `2^-(2 (rs 2^es + F_min))` | 2^-392 | 2^-424 | 2^-488 |
| radix point | `2 (rs 2^es + F_min)` | 392 | 424 | 488 |
| upper range | `2 rs 2^es` | 384 | 384 | 384 |
| range | `4 rs 2^es + 2 F_min` | 776 | 808 | 872 |
| with the default 30 carry bits | `range + 30` | 806 | 838 | 902 |

**The quire size depends on n.** A b-posit's smallest values still carry `F_min` fraction bits, so products of tiny values reach `2 F_min` bits below minpos^2. Sizing from minpos^2 alone, as the often-quoted "800 bits for any n > 12" does, matches the exact size only at n = 16 (776 bits, plus a 23-bit carry guard and the sign). Anchoring the quire at minpos^2 drops the lowest 2 F_min bits of products of tiny values: 8 at n = 16, 104 at n = 64. This is the trap #1202 fixed for cfloat. The ranges above were confirmed by brute force over all products of the small configurations, and `static/tapered/bposit/arithmetic/fdp.cpp` checks a dot product whose exact value lives only in those low bits.

```cpp
std::vector<bposit32> x = ..., y = ...;
bposit32 d = fdp(x, y);                       // exact sum of products, rounded once

quire<bposit32> q;                            // or accumulate by hand
for (std::size_t i = 0; i < x.size(); ++i) q += quire_mul(x[i], y[i]);
bposit32 s = quire_resolve(q);
```

## Reference encoding: `bposit<8, 4, 0>`

All 128 non-negative encodings. A negative value is the two's complement of the matching positive one, and `0x80` is NaR. The fraction field is never empty. Patterns 0x01 .. 0x07 (r = -4) and 0x78 .. 0x7F (r = 3) use the capped 4-bit regime, which has no terminating bit.

| pattern | bits after sign | regime | r | fraction | fraction bits | value |
|---|---|---|---|---|---|---|
| 0x00 | 0000000 | (zero) | | | | 0 |
| 0x01 | 0000001 | 0000 | -4 | 001 | 3 | 9/128 |
| 0x02 | 0000010 | 0000 | -4 | 010 | 3 | 5/64 |
| 0x03 | 0000011 | 0000 | -4 | 011 | 3 | 11/128 |
| 0x04 | 0000100 | 0000 | -4 | 100 | 3 | 3/32 |
| 0x05 | 0000101 | 0000 | -4 | 101 | 3 | 13/128 |
| 0x06 | 0000110 | 0000 | -4 | 110 | 3 | 7/64 |
| 0x07 | 0000111 | 0000 | -4 | 111 | 3 | 15/128 |
| 0x08 | 0001000 | 0001 | -3 | 000 | 3 | 1/8 |
| 0x09 | 0001001 | 0001 | -3 | 001 | 3 | 9/64 |
| 0x0A | 0001010 | 0001 | -3 | 010 | 3 | 5/32 |
| 0x0B | 0001011 | 0001 | -3 | 011 | 3 | 11/64 |
| 0x0C | 0001100 | 0001 | -3 | 100 | 3 | 3/16 |
| 0x0D | 0001101 | 0001 | -3 | 101 | 3 | 13/64 |
| 0x0E | 0001110 | 0001 | -3 | 110 | 3 | 7/32 |
| 0x0F | 0001111 | 0001 | -3 | 111 | 3 | 15/64 |
| 0x10 | 0010000 | 001 | -2 | 0000 | 4 | 1/4 |
| 0x11 | 0010001 | 001 | -2 | 0001 | 4 | 17/64 |
| 0x12 | 0010010 | 001 | -2 | 0010 | 4 | 9/32 |
| 0x13 | 0010011 | 001 | -2 | 0011 | 4 | 19/64 |
| 0x14 | 0010100 | 001 | -2 | 0100 | 4 | 5/16 |
| 0x15 | 0010101 | 001 | -2 | 0101 | 4 | 21/64 |
| 0x16 | 0010110 | 001 | -2 | 0110 | 4 | 11/32 |
| 0x17 | 0010111 | 001 | -2 | 0111 | 4 | 23/64 |
| 0x18 | 0011000 | 001 | -2 | 1000 | 4 | 3/8 |
| 0x19 | 0011001 | 001 | -2 | 1001 | 4 | 25/64 |
| 0x1A | 0011010 | 001 | -2 | 1010 | 4 | 13/32 |
| 0x1B | 0011011 | 001 | -2 | 1011 | 4 | 27/64 |
| 0x1C | 0011100 | 001 | -2 | 1100 | 4 | 7/16 |
| 0x1D | 0011101 | 001 | -2 | 1101 | 4 | 29/64 |
| 0x1E | 0011110 | 001 | -2 | 1110 | 4 | 15/32 |
| 0x1F | 0011111 | 001 | -2 | 1111 | 4 | 31/64 |
| 0x20 | 0100000 | 01 | -1 | 00000 | 5 | 1/2 |
| 0x21 | 0100001 | 01 | -1 | 00001 | 5 | 33/64 |
| 0x22 | 0100010 | 01 | -1 | 00010 | 5 | 17/32 |
| 0x23 | 0100011 | 01 | -1 | 00011 | 5 | 35/64 |
| 0x24 | 0100100 | 01 | -1 | 00100 | 5 | 9/16 |
| 0x25 | 0100101 | 01 | -1 | 00101 | 5 | 37/64 |
| 0x26 | 0100110 | 01 | -1 | 00110 | 5 | 19/32 |
| 0x27 | 0100111 | 01 | -1 | 00111 | 5 | 39/64 |
| 0x28 | 0101000 | 01 | -1 | 01000 | 5 | 5/8 |
| 0x29 | 0101001 | 01 | -1 | 01001 | 5 | 41/64 |
| 0x2A | 0101010 | 01 | -1 | 01010 | 5 | 21/32 |
| 0x2B | 0101011 | 01 | -1 | 01011 | 5 | 43/64 |
| 0x2C | 0101100 | 01 | -1 | 01100 | 5 | 11/16 |
| 0x2D | 0101101 | 01 | -1 | 01101 | 5 | 45/64 |
| 0x2E | 0101110 | 01 | -1 | 01110 | 5 | 23/32 |
| 0x2F | 0101111 | 01 | -1 | 01111 | 5 | 47/64 |
| 0x30 | 0110000 | 01 | -1 | 10000 | 5 | 3/4 |
| 0x31 | 0110001 | 01 | -1 | 10001 | 5 | 49/64 |
| 0x32 | 0110010 | 01 | -1 | 10010 | 5 | 25/32 |
| 0x33 | 0110011 | 01 | -1 | 10011 | 5 | 51/64 |
| 0x34 | 0110100 | 01 | -1 | 10100 | 5 | 13/16 |
| 0x35 | 0110101 | 01 | -1 | 10101 | 5 | 53/64 |
| 0x36 | 0110110 | 01 | -1 | 10110 | 5 | 27/32 |
| 0x37 | 0110111 | 01 | -1 | 10111 | 5 | 55/64 |
| 0x38 | 0111000 | 01 | -1 | 11000 | 5 | 7/8 |
| 0x39 | 0111001 | 01 | -1 | 11001 | 5 | 57/64 |
| 0x3A | 0111010 | 01 | -1 | 11010 | 5 | 29/32 |
| 0x3B | 0111011 | 01 | -1 | 11011 | 5 | 59/64 |
| 0x3C | 0111100 | 01 | -1 | 11100 | 5 | 15/16 |
| 0x3D | 0111101 | 01 | -1 | 11101 | 5 | 61/64 |
| 0x3E | 0111110 | 01 | -1 | 11110 | 5 | 31/32 |
| 0x3F | 0111111 | 01 | -1 | 11111 | 5 | 63/64 |
| 0x40 | 1000000 | 10 | 0 | 00000 | 5 | 1 |
| 0x41 | 1000001 | 10 | 0 | 00001 | 5 | 33/32 |
| 0x42 | 1000010 | 10 | 0 | 00010 | 5 | 17/16 |
| 0x43 | 1000011 | 10 | 0 | 00011 | 5 | 35/32 |
| 0x44 | 1000100 | 10 | 0 | 00100 | 5 | 9/8 |
| 0x45 | 1000101 | 10 | 0 | 00101 | 5 | 37/32 |
| 0x46 | 1000110 | 10 | 0 | 00110 | 5 | 19/16 |
| 0x47 | 1000111 | 10 | 0 | 00111 | 5 | 39/32 |
| 0x48 | 1001000 | 10 | 0 | 01000 | 5 | 5/4 |
| 0x49 | 1001001 | 10 | 0 | 01001 | 5 | 41/32 |
| 0x4A | 1001010 | 10 | 0 | 01010 | 5 | 21/16 |
| 0x4B | 1001011 | 10 | 0 | 01011 | 5 | 43/32 |
| 0x4C | 1001100 | 10 | 0 | 01100 | 5 | 11/8 |
| 0x4D | 1001101 | 10 | 0 | 01101 | 5 | 45/32 |
| 0x4E | 1001110 | 10 | 0 | 01110 | 5 | 23/16 |
| 0x4F | 1001111 | 10 | 0 | 01111 | 5 | 47/32 |
| 0x50 | 1010000 | 10 | 0 | 10000 | 5 | 3/2 |
| 0x51 | 1010001 | 10 | 0 | 10001 | 5 | 49/32 |
| 0x52 | 1010010 | 10 | 0 | 10010 | 5 | 25/16 |
| 0x53 | 1010011 | 10 | 0 | 10011 | 5 | 51/32 |
| 0x54 | 1010100 | 10 | 0 | 10100 | 5 | 13/8 |
| 0x55 | 1010101 | 10 | 0 | 10101 | 5 | 53/32 |
| 0x56 | 1010110 | 10 | 0 | 10110 | 5 | 27/16 |
| 0x57 | 1010111 | 10 | 0 | 10111 | 5 | 55/32 |
| 0x58 | 1011000 | 10 | 0 | 11000 | 5 | 7/4 |
| 0x59 | 1011001 | 10 | 0 | 11001 | 5 | 57/32 |
| 0x5A | 1011010 | 10 | 0 | 11010 | 5 | 29/16 |
| 0x5B | 1011011 | 10 | 0 | 11011 | 5 | 59/32 |
| 0x5C | 1011100 | 10 | 0 | 11100 | 5 | 15/8 |
| 0x5D | 1011101 | 10 | 0 | 11101 | 5 | 61/32 |
| 0x5E | 1011110 | 10 | 0 | 11110 | 5 | 31/16 |
| 0x5F | 1011111 | 10 | 0 | 11111 | 5 | 63/32 |
| 0x60 | 1100000 | 110 | 1 | 0000 | 4 | 2 |
| 0x61 | 1100001 | 110 | 1 | 0001 | 4 | 17/8 |
| 0x62 | 1100010 | 110 | 1 | 0010 | 4 | 9/4 |
| 0x63 | 1100011 | 110 | 1 | 0011 | 4 | 19/8 |
| 0x64 | 1100100 | 110 | 1 | 0100 | 4 | 5/2 |
| 0x65 | 1100101 | 110 | 1 | 0101 | 4 | 21/8 |
| 0x66 | 1100110 | 110 | 1 | 0110 | 4 | 11/4 |
| 0x67 | 1100111 | 110 | 1 | 0111 | 4 | 23/8 |
| 0x68 | 1101000 | 110 | 1 | 1000 | 4 | 3 |
| 0x69 | 1101001 | 110 | 1 | 1001 | 4 | 25/8 |
| 0x6A | 1101010 | 110 | 1 | 1010 | 4 | 13/4 |
| 0x6B | 1101011 | 110 | 1 | 1011 | 4 | 27/8 |
| 0x6C | 1101100 | 110 | 1 | 1100 | 4 | 7/2 |
| 0x6D | 1101101 | 110 | 1 | 1101 | 4 | 29/8 |
| 0x6E | 1101110 | 110 | 1 | 1110 | 4 | 15/4 |
| 0x6F | 1101111 | 110 | 1 | 1111 | 4 | 31/8 |
| 0x70 | 1110000 | 1110 | 2 | 000 | 3 | 4 |
| 0x71 | 1110001 | 1110 | 2 | 001 | 3 | 9/2 |
| 0x72 | 1110010 | 1110 | 2 | 010 | 3 | 5 |
| 0x73 | 1110011 | 1110 | 2 | 011 | 3 | 11/2 |
| 0x74 | 1110100 | 1110 | 2 | 100 | 3 | 6 |
| 0x75 | 1110101 | 1110 | 2 | 101 | 3 | 13/2 |
| 0x76 | 1110110 | 1110 | 2 | 110 | 3 | 7 |
| 0x77 | 1110111 | 1110 | 2 | 111 | 3 | 15/2 |
| 0x78 | 1111000 | 1111 | 3 | 000 | 3 | 8 |
| 0x79 | 1111001 | 1111 | 3 | 001 | 3 | 9 |
| 0x7A | 1111010 | 1111 | 3 | 010 | 3 | 10 |
| 0x7B | 1111011 | 1111 | 3 | 011 | 3 | 11 |
| 0x7C | 1111100 | 1111 | 3 | 100 | 3 | 12 |
| 0x7D | 1111101 | 1111 | 3 | 101 | 3 | 13 |
| 0x7E | 1111110 | 1111 | 3 | 110 | 3 | 14 |
| 0x7F | 1111111 | 1111 | 3 | 111 | 3 | 15 |

## How to use it

```cpp
#include <universal/number/bposit/bposit.hpp>
using namespace sw::universal;

bposit32 a(3.0), b(7.0);
bposit32 c = a / b;                       // round to nearest, ties to even
std::cout << c << ' ' << to_binary(c) << '\n';

using Small = bposit<8, 4, 0>;
Small tiny(1.0e-9);                       // saturates to minpos, 9/128: never zero
```
