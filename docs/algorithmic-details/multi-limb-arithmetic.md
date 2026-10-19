# Multi-limb arithmetic

Universal's fixed-size number systems keep their integers and significands in arrays of
blocks, and do their arithmetic on those arrays: `blocksignificand` and `blocktriple` under
`cfloat`, `areal`, `valid` and `poxel`; `blockbinary` and `integer` for the binary integers,
and `integer<N>` again as the big integer under every decimal parser. This page describes how
those arrays are laid out and the algorithms that add, multiply and divide them.

Until October 2026 multiplication and division were bit-serial: one full-width add or
subtract for every bit of an operand or quotient. Three changes replaced them with word-level
algorithms and shared kernels:

- #1742 / PR #1746 -- `blocksignificand::div`, long division a word at a time
- #1743 / PR #1747 -- `blocksignificand::mul`, schoolbook multiplication
- #1744 / PR #1748 -- `integer` and `blockbinary` division

The component-level overview of `blockbinary`, `blocksignificand` and `blocktriple` -- their
template parameters, encodings and how each number system uses them -- is
[Multi-Limb Arithmetic Types](../multi-limb-arithmetic.md) in Design. A note on words: that
page calls the storage units of these types limbs. Here they are **blocks** (of the block type
`bt`), and a **limb** is the 32-bit word the arithmetic kernels compute with; the two coincide
only for `uint32_t` blocks.

Related implementation reading:

- `include/sw/universal/internal/blocktype/limb_division.hpp` -- `limb::divide`, `limb::gather`, `limb::scatter`
- `include/sw/universal/internal/blocktype/limb_multiplication.hpp` -- `limb::multiply_low`
- `include/sw/universal/internal/blocktype/carry.hpp` -- `addcarry()` for 64-bit blocks
- `include/sw/universal/internal/blocksignificand/blocksignificand.hpp` -- `add`, `sub`, `mul`, `div`, `to_limbs`, `from_limbs`
- `include/sw/universal/internal/blocktriple/blocktriple.hpp` -- operator sizing, `add`, `mul`, `div`, `roundingDecision`
- `include/sw/universal/number/integer/integer_impl.hpp` -- `divide_magnitudes`, `idiv`, `reduce`
- `include/sw/universal/internal/blockbinary/blockbinary.hpp` -- `longdivision`

## 1. Architecture

### Blocks and limbs

A number stores `nrBlocks` blocks of its block type `bt` -- `uint8_t`, `uint16_t`, `uint32_t`
or `uint64_t` -- least significant block first. The bits above `nbits` in the top block are
always zero; every setter enforces it with `MSU_MASK`, and the relational operators rely on it,
because they compare raw blocks.

The arithmetic kernels do not work on blocks. They work on **32-bit limbs with 64-bit
intermediates**, whatever the block type: a 32 x 32 product, a 64-bit sum with its carry and a
64-by-32 division are native operations on every platform Universal builds for, including MSVC,
which has no `unsigned __int128`. Two pairs of functions convert:

- `blocksignificand::to_limbs()` / `from_limbs()` (members, `blocksignificand.hpp`)
- `limb::gather()` / `limb::scatter()` (free functions over block accessors, `limb_division.hpp`)

```
  blocks of uint8_t, nbits = 20             blocks of uint64_t, nbits = 80

  bit  19      16 15      8 7       0       bit 127            64 63                 0
      +--------+---------+---------+            +---------------+--------------------+
      |0000xxxx|xxxxxxxx |xxxxxxxx |            |000...0xxxxxxxx|xxxxxxxxxxxxxxxxxxxx|
      +--------+---------+---------+            +---------------+--------------------+
       block 2  block 1   block 0                    block 1           block 0
          \        |        /                     /         \           |
           \       |       /                limb 3 (empty)  limb 2   limb 1 limb 0
            +------+------+                      not read  (bits 64..79)
            |  limb 0     |  bits 0..19
            +-------------+
```

Two details matter, and both caught out the first prototype of #1742:

- **Blocks narrower than a limb** land at an offset inside it. Block `i` of `b` bits occupies
  bits `i*b ..` of the number, so it goes to limb `(i*b)/32` shifted left by `(i*b) % 32`, and
  comes back shifted right by the same amount. Forgetting the offset in either direction
  corrupts every 8- and 16-bit block configuration.
- **A 64-bit top block can reach past the last limb.** With `nbits = 80`, `nrLimbs = 3` but
  block 1 spans bits 64..127; its upper half maps to limb 3, which does not exist. Those bits are
  above `nbits` and zero, so the conversion simply skips any limb index `>= nrLimbs`.

### The shared kernels

Both kernels are `constexpr` and allocation-free: the caller passes scratch arrays it sizes at
compile time, so a fixed-size type can use them in constant evaluation and on the stack.

| kernel | header | computes |
|---|---|---|
| `limb::multiply_low(u, m, v, n, w, k)` | `limb_multiplication.hpp` | `w = (u * v) mod 2^(32k)` |
| `limb::divide(u, m, v, n, q, un, vn)` | `limb_division.hpp` | `q = floor(u / v)`; returns remainder != 0 |
| `limb::divide(u, m, v, n, q, r, un, vn)` | `limb_division.hpp` | the same, and `r = u - q v` |

### How blocktriple sizes each operator

A `blocktriple<fbits, op, bt>` is a sign, a scale and a significand `blocksignificand<bfbits,
bt>` whose width and radix point depend on the operator it will carry out. From
`blocktriple.hpp`, with `fhbits = fbits + 1`:

| operator | significand bits `bfbits` | output radix | why |
|---|---|---|---|
| ADD | `3 + abits`, `abits = fbits + 3` | `abits` | 3 integer bits for two's complement and carry; 3 rounding bits |
| MUL | `2 + 2 fbits` | `2 fbits` | the whole product of two `fhbits` significands |
| DIV | `2 + divbits`, `divbits = 3 fbits + 4` | `divbits` | `2 fbits + 4` bits below the fraction, see section 4 |
| SQRT | `2 fhbits` | `2 fhbits` | |

A number system prepares its operands into the right shape and calls the operator:
`cfloat::normalizeAddition` / `normalizeMultiplication` / `normalizeDivision` fill the
significand (for division, `1.f` shifted up by `divshift = divbits - fbits`), then
`blocktriple::add` / `mul` / `div` produce an **unrounded** result, and `convert()` rounds it
to the target with `roundingDecision()`, which reads the lsb, guard and round bits and folds
every bit below them into a sticky bit (`blocksignificand::roundingDirection`).

## 2. Addition and subtraction

Addition was already block-wise. `blocksignificand::add` adds block by block with a carry:

- blocks narrower than 64 bits are summed in a `uint64_t`, and the carry is `s > ALL_ONES`;
- `uint64_t` blocks have no wider native type, so they go through `addcarry()` (`carry.hpp`):
  `unsigned __int128` on gcc and clang, `_addcarry_u64` on MSVC. In constant evaluation, where
  the intrinsics are not `constexpr`, `add` itself takes a portable compare-based carry path
  (`std::is_constant_evaluated()`).

`carry.hpp` documents an MSVC trap worth knowing before writing any carry chain:
`_addcarry_u64`'s carry-in is a single bit, while `addcarry()`'s contract is a full 64-bit
carry-in -- passing a multi-bit carry through the intrinsic silently drops it.

Subtraction is addition of the two's complement: `sub(lhs, rhs)` is
`add(lhs, twosComplementFree(rhs))`.

`blocktriple::add` aligns before it adds. The operand with the smaller scale is shifted right
by the scale difference, and the bits shifted out are not lost: `any()` reports whether any of
them was set, and that becomes the shifted operand's bit 0, a sticky bit. Negative operands are
put in two's complement, the significands are added, a negative sum is negated back, and a sum
that lost its leading bits (`000.ffff`) is normalized by a left shift. An overflowing sum
(`01#.ffff`) is left to the rounding stage, which can shift it right without losing rounding
information.

```
  lhs   0 0 1 . f f f f r r r          scale s
  rhs   0 0 1 . f f f f r r r          scale s - 3, shifted right by 3:
  rhs'  0 0 0 . 0 0 1 f f f f   |rrr   the three bits shifted out OR into bit 0
                              ^
                              sticky
```

## 3. Multiplication

`blocksignificand::mul` (#1743) forms the low `nbits` of the product with
`limb::multiply_low`, schoolbook multiplication on 32-bit limbs:

```
for i in 0 .. m-1:
    carry = 0
    for j in 0 .. n-1 while i + j < k:
        t        = u[i] * v[j] + w[i+j] + carry      // 64-bit
        w[i+j]   = low 32 bits of t
        carry    = high 32 bits of t
    if i + n < k: w[i+n] = carry                     // that limb is still zero here
```

The 64-bit intermediate cannot overflow: with every term at most `2^32 - 1`,
`(2^32 - 1)^2 + 2 (2^32 - 1) = 2^64 - 1`. And `w[i+n]` is still zero when row `i` writes its
carry there, because row `i - 1` wrote at most up to `w[i-1+n]`.

**The low-nbits contract.** The shift-and-add loop this replaced nulled the bits above `nbits`
after every add, so it produced the product modulo `2^nbits`; `multiply_low` produces the same,
by never forming partial products at or above limb `k = nrLimbs`, and `from_limbs()` masks the
top block. In practice nothing is cut off: a `blocktriple<MUL>` sizes its significand to
`2 + 2 fbits` bits, enough for the whole product of two `fbits + 1`-bit significands.

**Cost.** O(nrLimbs^2) word multiply-adds, against O(nbits x nrBlocks) block adds and shifts
for the loop it replaced. Karatsuba's O(n^1.585) overtakes schoolbook only somewhere past a few
dozen limbs; the widest significands Universal builds (a 1024-bit cfloat's product is 2018
bits, 64 limbs) are at the edge of that, and none of the common ones are near it.

## 4. Division

### Knuth's Algorithm D

`limb::divide` is Algorithm D (Knuth, TAOCP Vol. 2, 4.3.1), in the form of Hacker's Delight's
`divmnu`. It divides an `m`-limb `u` by an `n`-limb `v`, producing one 32-bit quotient digit
per step instead of one bit:

1. **Normalize.** Shift `v` left until its top bit is set, and `u` by the same `s` bits (into
   an extra limb). This bounds the error of the digit estimate below.
2. **Estimate.** For each quotient digit, from the most significant down, divide the top two
   limbs of the running remainder by the top limb of `v`:
   `qhat = (un[j+n] * 2^32 + un[j+n-1]) / vn[n-1]`, `rhat` the remainder.
3. **Correct.** While `qhat >= 2^32` or `qhat * vn[n-2] > rhat * 2^32 + un[j+n-2]`, lower
   `qhat` and raise `rhat` by `vn[n-1]`, stopping once `rhat >= 2^32`. With `v` normalized this
   runs at most twice, and leaves `qhat` exact or one too large.
4. **Multiply-subtract.** `un[j .. j+n] -= qhat * vn`, with a running borrow.
5. **Add back.** If that went negative, `qhat` was one too large: add `vn` back once and
   decrement `qhat`. This is rare -- about `2/2^32` of digits.
6. **Remainder.** After the last digit, `un[0 .. n-1]` holds the remainder shifted left by
   `s`; shift it back (the remainder-returning overload of `limb::divide`).

A single-limb divisor takes a short path instead: one 64-by-32 division per limb of `u`.

```
  one step, n = 2                     un:  [ u3 | u2 | u1 | u0 ]     vn: [ v1 | v0 ]
                                                ^^^^^^^^^
                                     qhat = (u3:u2) / v1, corrected against v0 and u1
                                     u3:u2:u1 -= qhat * (v1:v0)    -> next remainder
```

### The fraction quotient in blocktriple

`blocksignificand::div` (#1742) uses the kernel for a **fixed-point** quotient. Both operands
are read at the dividend's radix `R`, so their values are `A / 2^R` and `B / 2^R`, and the
quotient bits are

```
Q = floor(A * 2^R / B), with bit 0 set when the division was inexact
```

kept modulo `2^nbits`. For a `blocktriple<DIV>`, `R = divbits = 3 fbits + 4` and both
operands carry `1.f` shifted up by `divshift = 2 fbits + 4`, so `Q` holds the quotient of two
`fbits + 1`-bit significands to `2 fbits + 4` bits below its fraction. `blocktriple::div` then
shifts `Q` left once, lowering the scale, when the quotient is below 1.

Why `2 fbits + 4` extra bits round correctly. A quotient of two `p`-bit significands is either
exact or differs from every rounding boundary of `p + 1` bits (the midpoints between
representable values) by more than about `2^-2p` relative: if `a/b` came that close to a
midpoint `m`, then `a - m b` would be a nonzero multiple of `2^-(2p+1)` smaller in magnitude
than that, which it cannot be. So the bits below the round bit of a truncated quotient carried
to `2p + 2` places are nonzero exactly when the quotient is inexact, and they serve as the
sticky bit. The old bit-serial division relied on this alone. The limb division also ORs the
remainder's being nonzero into bit 0, which lies below the rounding bits, where
`roundingDirection()` folds it into the sticky bit like any other: the rounded results are
identical, and the unrounded quotient now says exactly whether it is exact.

Two guards in `blocksignificand::div`:

- **Aliasing.** `clear()` resets the receiver, radix included, so everything is read from the
  operands first: `x.div(x, y)` and `y.div(x, y)` work.
- **Radix range.** A radix outside `[0, nbits)` has no quotient bits and would index past the
  scratch arrays; `div` returns zero instead. Every radix a `blocktriple` uses is in range.

### Integer and blockbinary division

`integer`'s `idiv` (behind `/` and `%`; `divide_magnitudes` in `integer_impl.hpp`) and
`blockbinary`'s `longdivision` (#1744) divide **magnitudes** and then restore the signs, as
the shift-and-subtract loops they replaced did:

- the quotient truncates toward zero, negated when the signs differ;
- the remainder takes the dividend's sign;
- the magnitudes are formed one bit wider than `nbits`, so that the magnitude of maxneg,
  `2^(nbits-1)`, does not wrap; the quotient of two magnitudes is below `2^nbits`, so `nbits`
  bits hold all of it, and `maxneg / -1` gives `2^(nbits-1)`, which wraps back to maxneg.

`integer<>::reduce()` divides through `idiv` too since #1750; it used to carry a block-digit
Knuth division of its own that was wrong for every divisor wider than one block.

efloat keeps its own copy of Algorithm D (`divide_limbs` in `efloat_impl.hpp`, #1714 / PR
#1741), on MSB-first `std::vector<uint32_t>` limbs with the divisor's zero tail limbs dropped,
so that a small-integer divisor takes the single-limb path. It could move onto `limb::divide`.

## 5. Verification methodology

Each change was required to be bit-identical, and shown so without trusting the code under
test. The recipe:

1. **Exhaustive old-against-new.** Build the same harness twice: once against the branch, once
   against the previous implementation, by putting `git show main:<header>` copies first on the
   include path (`-I overlay -I include/sw`). Enumerate every operand pair of small
   configurations -- 8 to 12 bits, every block type that fits, subnormals and max-exponent
   values on and off, saturating or not -- and compare the outputs, or a hash of them when they
   run to gigabytes. 74.8M pairs each for cfloat division and multiplication; 45M
   quotient-remainder pairs for integer and blockbinary.
2. **Wide random old-against-new**, for the widths enumeration cannot reach (24 to 1000 bits,
   every block type), with the edge operands that break long division: maxneg, -1, 1, equal
   operands, one-limb divisors. Also under clang ASan and UBSan.
3. **Native oracles.** `cfloat<32,8>` and `cfloat<64,11>` with subnormals are IEEE single and
   double, so native `float` / `double` arithmetic is an exact oracle, for every block type;
   `int32_t` / `int64_t` / `uint64_t` are for `integer`.
4. **Identity oracles** for widths no native type holds: `a == q * b + r`, `|r| < |b|`, `r`
   zero or of `a`'s sign, with the product formed twice as wide.
5. **Mutants.** Break the code on purpose -- drop the remainder, drop the sticky bit, store a
   result before reading the operands -- and check the tests fail.

The regression tests that keep this:

- `static/float/cfloat/arithmetic/division_native.cpp`, `multiplication_native.cpp`
- `static/integer/binary/arithmetic/long_division.cpp`, `reduce.cpp`
- `internal/blocktriple/arithmetic/division.cpp`, `internal/blocksignificand/arithmetic/division.cpp`
  (both rewritten in #1745 to the contract of section 4)

Pitfalls met along the way, each of which looked like a bug in the code under test:

- **8- and 16-bit block packing.** The first #1742 prototype forgot the in-limb offset on the
  way back and failed ~77% of native single and double divisions for those block types.
- **Uninitialized operands.** `cfloat` and `integer` are trivially constructible: `C a;` leaves
  the blocks uninitialized, and a harness that then sets only bits `0 .. nbits-1` leaves garbage
  above `nbits`, which `scale()` reads. A harness must `clear()` or `setzero()` first.
- **Out-of-range radix.** A radix set directly, outside `[0, nbits)`, overflowed the limb
  shift's scratch array (found in review of PR #1746).

## 6. Performance

All figures are gcc -O2 on one machine, from benchmarks in the tree.

cfloat division and multiplication, `benchmark/performance/arithmetic/cfloat/performance.cpp`:

| type | division before | division after | multiplication before | multiplication after |
|---|---|---|---|---|
| `cfloat<8,2,uint16_t>` | 12 Mops/s | 38 Mops/s | 285 Mops/s | 308 Mops/s |
| `cfloat<16,5,uint16_t>` | 2.7 Mops/s | 31 Mops/s | 46 Mops/s | 81 Mops/s |
| `cfloat<32,8,uint32_t>` | 0.94 Mops/s | 10 Mops/s | 26 Mops/s | 64 Mops/s |
| `cfloat<64,11,uint64_t>` | 226 Kops/s | 4.9 Mops/s | 4.3 Mops/s | 13 Mops/s |
| `cfloat<128,15,uint64_t>` | 106 Kops/s | 2.2 Mops/s | 792 Kops/s | 6.6 Mops/s |
| `cfloat<256,15,uint64_t>` | 31 Kops/s | 887 Kops/s | 308 Kops/s | 3.3 Mops/s |
| `cfloat<512,15,uint64_t>` | 10 Kops/s | 340 Kops/s | 116 Kops/s | 1.4 Mops/s |
| `cfloat<1024,15,uint64_t>` | 2.4 Kops/s | 113 Kops/s | 38 Kops/s | 559 Kops/s |

Integer and blockbinary division, random full-width dividend over a half-width divisor,
`benchmark/performance/arithmetic/integer/division.cpp`:

| type | before | after | speedup |
|---|---|---|---|
| `integer<64,uint32_t>` | 0.39 us | 0.037 us | 10x |
| `integer<128,uint32_t>` | 0.67 us | 0.040 us | 17x |
| `integer<256,uint32_t>` | 1.77 us | 0.075 us | 24x |
| `integer<1024,uint32_t>` | 17.9 us | 0.57 us | 32x |
| `integer<4096,uint32_t>` | 305 us | 4.1 us | 74x |
| `blockbinary<1024,uint32_t>` | 36.0 us | 0.51 us | 71x |

efloat division at 608 bits, `benchmark/performance/arithmetic/efloat/mpmath_gap.cpp`: 9.0 us
before, 0.49 us after -- 2.8x faster than mpmath 1.3.0 on its pure-Python backend, against 7x
slower before.

**Where the time goes now.** At half and single precision a cfloat multiply costs about one
add, and a division less than two: the significand arithmetic is no longer the bottleneck.
What remains is per-operation overhead around it -- unpacking the encoding into a
`blocktriple`, the special-value checks, and `convert()`'s rounding and re-encoding -- which
is the same for every operator, and is where the next gains at small widths would come from.
