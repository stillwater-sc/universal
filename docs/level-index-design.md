# Level-index arithmetic -- design assessment

Status: assessment / proposal (#1349 part 1, #1692)
Date: 2026-10-15

## Question

Universal has no level-index (LI) type. Epic #1349 asks for one, built so that its
reciprocal-symmetric extension SLI (#1350) can share the kernel. Before any code is
written, this document fixes the value map, the encoding, the error metric, the
rounding contract and the arithmetic, and answers the five open decisions the epic
lists. Three of the epic's own claims turn out to be wrong, and they are corrected
here; see [Corrections to the epic](#corrections-to-the-epic).

Every number below carries a tag such as **[C5]**. The tag names the section of
`tools/generators/level_index_design_check.py` that computes it with mpmath at 400
bits. Run the script to reproduce all of them:

```text
python3 tools/generators/level_index_design_check.py
```

It also asserts the identities the design relies on, and exits non-zero if one fails.

## Sources and what was verified

All four papers the epic cites exist, and each citation is correct as written. The
bibliographic details below were checked against the Crossref record of each DOI. The
original texts are paywalled and were not read. The attribution of each algorithm to
a paper comes from Mikaitis (2024), a secondary source that restates the formulas.
Each formula used here was then re-derived and checked numerically [C9, C10], so the
design does not rest on the secondary source alone.

| Reference | Crossref record | Used here for |
|---|---|---|
| Clenshaw, Olver, "Beyond floating point", *J. ACM* 31(2), 319-328, 1984. [10.1145/62.322429](https://doi.org/10.1145/62.322429) | as cited | the LI value map, and 3 level bits being enough |
| Clenshaw, Olver, "Level-index arithmetic operations", *SIAM J. Numer. Anal.* 24(2), 470-485, 1987. [10.1137/0724034](https://doi.org/10.1137/0724034) | as cited | the a/b/c sequences of addition and subtraction; multiplication and division one level down |
| Clenshaw, Olver, Turner, "Level-index arithmetic: an introductory survey", in *Numerical Analysis and Parallel Processing*, LNM 1397, Springer, 95-168, 1989. [10.1007/BFb0085718](https://doi.org/10.1007/BFb0085718) | as cited | the survey (7 lectures, from the 1987 Lancaster summer school) |
| Lozier, Olver, "Closure and precision in level-index arithmetic", *SIAM J. Numer. Anal.* 27(5), 1295-1304, October 1990. [10.1137/0727073](https://doi.org/10.1137/0727073) | as cited | closure under + - * / (division by zero excluded) |
| Clenshaw, Turner, "The symmetric level-index system", *IMA J. Numer. Anal.* 8(4), 517-526, 1988. [10.1093/imanum/8.4.517](https://doi.org/10.1093/imanum/8.4.517) | not in the epic; added | SLI (#1350): the reciprocal sign and the modified arithmetic |
| Mikaitis, "MATLAB Simulator of Level-Index Arithmetic", [arXiv:2402.02301](https://arxiv.org/abs/2402.02301), 2024 | -- | secondary source for the formulas; it reports Olver's float crossovers, which [C6] reproduces |

#1298 is the reason for this table: a takum header cited the wrong arXiv paper for two
years. Any change to the level-index sources should update the table.

## Value map

The generalized exponential and its inverse, the generalized logarithm:

```text
phi(x) = x               0 <= x < 1           psi(X) = X                 0 <= X < 1
phi(x) = e^phi(x - 1)    x >= 1               psi(X) = 1 + psi(ln X)     X >= 1
```

A non-negative value is `X = phi(l + f)`, where the integer `l` is the **level** and
`f` in [0, 1) is the **index**. Both functions are continuous and strictly increasing,
and `psi(phi(x)) = x`.

| level l | X = phi(l + f) covers | [C1] |
|---|---|---|
| 0 | [0, 1) | phi(0) = 0, phi(1) = 1 |
| 1 | [1, e) | phi(2) = 2.71828182846 |
| 2 | [e, 15.154) | phi(3) = 15.1542622415 |
| 3 | [15.154, 3.814e6) | phi(4) = 3814279.10476 |
| 4 | [3.814e6, 10^1656520.4) | log10 phi(5) = 1656520.4 |
| 5 | up to phi(6), with log10 log10 phi(6) = 1656520.0 | |
| 6, 7 | beyond any notation shorter than the value map itself | |

### No native type reaches level 5

| largest value of | psi [C2] |
|---|---|
| binary32 (FLT_MAX) | 4.4060342 |
| binary64 (DBL_MAX) | 4.6322002 |
| x87 80-bit long double | 4.8038132 |
| binary128 | 4.8038132 |

Levels 5, 6 and 7 hold no value that any IEEE format can carry. Two consequences:

- Conversion **from** a native float only ever lands in levels 0 to 4. Conversion
  **to** a native float overflows to +-inf from level 4.63 (double) or 4.41 (float)
  up.
- The epic's plan to use `__float128` as the oracle cannot reach the top three levels.
  The oracle has to work in the psi domain and emit **encodings**, not values. mpmath
  does this: its exponents are unbounded integers. See [Oracle](#oracle).

## Closure, and the underflow floor

### LI is closed at the top

Lozier and Olver prove that LI is closed under + - * / when division by zero is
excluded: no result leaves the representable range. The same property is easy to see
numerically in a finite format. [C11] squares the largest encoding of each level of
`lindex<16,3>` and measures how far psi moves:

| x = psi(X) | psi(X^2) - psi(X), in psi-ulps |
|---|---|
| 3 + 1 - u | 10^2.92 (about 838) |
| 4 + 1 - u | 10^1.82 (about 66) |
| 5 + 1 - u | 10^-4.74 |
| 7 + 1 - u (maxpos) | 10^(-2.1 * 10^1639951) |

From level 6 up, `X * X` rounds back to `X`. Multiplication never leaves the top two
levels, so `lbits = 3`, with levels 0 to 7, is closed. This agrees with Olver's
remark, as Mikaitis reports it, that levels beyond 6 are not entered in practice.

**`lbits = 2` is not closed.** Its last level is 3. Squaring the largest
`lindex<16,2>` value moves psi from 3.99987793 to 4.204599876, past the last encoding
[C11]. A smaller `lbits` is therefore a configuration that can overflow, and the type
must saturate there (see [Special values](#special-values-and-the-encoding)). The
default stays `lbits = 3`.

### LI is not closed at the bottom

The epic says LI eliminates both overflow and underflow. It eliminates overflow only.
Level 0 is a plain fixed-point fraction, `X = f`, so the smallest positive value is one
step of the index grid [C3]:

| type | fbits | minpos = 2^-fbits | encodings per level |
|---|---|---|---|
| `lindex<8,3>` | 4 | 0.0625 | 16 |
| `lindex<16,3>` | 12 | 0.000244141 | 4096 |
| `lindex<32,3>` | 28 | 3.72529e-9 | 268435456 |
| `lindex<64,3>` | 60 | 8.67362e-19 | 2^60 |

Anything below minpos/2 rounds to 0; minpos/2 itself ties to even, which is also 0.
For a 16-bit type the floor is 2.4e-4, so ordinary small quantities underflow. This
asymmetry between huge and tiny values is exactly what SLI (#1350) fixes: it adds a
reciprocal sign, so that values below 1 get the same level structure as values above
it. LI's underflow floor is the motivation for SLI, and the LI documentation should
say so.

## Special values and the encoding

**Decisions 2 (field layout) and 5 (zero, negatives, NaR).**

```text
lindex<nbits, lbits>:   [ s : 1 ][ l + f : lbits integer bits . fbits fraction bits ]
                        fbits = nbits - 1 - lbits
```

- **The magnitude is one unsigned fixed-point number `x = l + f`.** The epic's
  "sign + level + index" is the same bit string read as one number, with the level
  bits as the integer part and the index bits as the fraction. Reading it as one
  number is what makes `exp` and `log` exact shifts by 1 (below) and makes rounding
  ordinary fixed-point rounding.
- **Negatives are the two's complement of the whole nbits pattern**, as in posit and
  takum. Since phi is strictly increasing, larger x means larger X. So for every pair
  of encodings, comparing them as signed integers gives the same order as comparing
  their values. The epic asks whether monotone integer ordering is achievable: it is,
  for free.
- **Zero is all zeros**, because phi(0) = 0. There is only one zero.
- **NaR is `1000...0`**, the pattern that is its own two's complement, as in posit.
  As a signed integer it compares below every other value, which is the posit
  convention.
- **There is no infinity.** Closure makes one unnecessary for + - * /.
- `nbits >= lbits + 2`, so that at least one index bit exists.

The alternative, sign-magnitude, was rejected: it has two zeros and needs a custom
comparison.

**Results that leave the range** happen only in operations that closure does not
cover:

| operation | result | why |
|---|---|---|
| x / 0, 0 / 0 | NaR | closure excludes division by zero |
| any op with a NaR operand | NaR | |
| `log(0)`, `log(x < 0)`, `sqrt(x < 0)` | NaR | there is no -inf |
| `exp` from level `2^lbits - 1`, which would need level `2^lbits` | maxpos, saturated | the only place where LI with lbits >= 3 meets the top; a test must cover it |
| `lbits <= 2` products past the last level | maxpos, saturated | lbits = 2 is not closed [C11] |
| convert from +-inf or NaN | NaR | posit precedent |
| convert to IEEE above psi(max of that type) | +-inf | the honest IEEE answer |

## Error metric: psi-ulps, never value-domain ulps

One unit in the last place is `u = 2^-fbits` in psi, at every level. The value-domain
gap that one psi-ulp spans grows violently with the level. At the top of each level of
`lindex<16,3>` it is [C5]:

| level | relative gap phi(x + u) / phi(x) - 1 |
|---|---|
| 0 | u / X: 2.4e-4 at X = 1, but 1.0 (100%) at X = minpos |
| 1 | 2.44e-4 |
| 2 | 6.64e-4 |
| 3 | 1.01e-2 |
| 4 | e^38150.8 (a factor of 10^16568.7) |

So a correctly rounded result near the top of level 4 can be off in value by a factor
of 10^16568. A test that measures error in value-domain ulps, or as relative value
error, would report that as a catastrophic failure. It is the format working as
designed. The takum_log work made exactly this mistake: it asserted through a
value-domain metric on a log-domain type, and got phantom failures.

**Rule for every lindex test:** the error of a result is
`|psi(computed) - psi(exact)|` in units of `2^-fbits`. `psi(computed)` is the encoding
itself, read as fixed point, so it is exact. `psi(exact)` comes from the mpmath oracle.

The same numbers locate LI against IEEE for X >= 1 [C6]. `lindex<32,3>` has a finer
grid than every binary32 value below 2^11.24, and a coarser one than every binary32
value above 2^18.21. `lindex<64,3>` against binary64 crosses at 2^43.98 and 2^70.1.
Mikaitis reports Olver's figures as 2^11 and 2^18 for 32 bits and 2^44 and 2^70 for
64 bits. The reproduction agrees, which also checks the layout and the value of fbits
used here.

## Rounding contract

- **Round to nearest, ties to even, on the psi grid**, just as LNS rounds in the log
  domain. Converting a value v means computing psi(|v|) in the working precision and
  rounding it to fbits fraction bits. At level 0, psi is the identity, so this is plain
  fixed-point rounding.
- **First target: faithful.** Every operation returns one of the two encodings that
  bracket the exact psi, that is, an error below 1 psi-ulp.
- **Final target: correctly rounded.** The working precision is accurate to about
  2^-44 or better [C12, C13], against a half-ulp of 2^-29 at fbits 28. That margin
  resolves almost every case. The near-ties that remain need a retry at higher
  precision, a Ziv-style loop, which #1695 decides once faithful rounding is measured.

## numeric_limits

**Decision 3.** Every member is defined from the extreme encodings, as takum_log does,
and none from a value-domain formula.

| member | value | reasoning |
|---|---|---|
| `radix` | 2 | the radix of the fixed-point grid of l + f. The standard requires an int, and the value map has no base at all (takum_log precedent: `radix` describes the representation, not the value map) |
| `epsilon()` | 2^-fbits | (phi(1 + u) - 1) / u = 1 + O(u): 1.03191 at fbits 4, 1.00012208 at fbits 12, 1.000000002 at fbits 28 [C4]. So 2^-fbits is the gap above 1 to first order, which is what generic code reads epsilon as |
| `digits` | fbits + 1 | keeps `epsilon == 2^(1 - digits)`, the relation generic code assumes |
| `digits10`, `max_digits10` | from digits, as for cfloat | |
| `min()`, `denorm_min()` | 2^-fbits, the minpos encoding | level 0 is evenly spaced, so there is no subnormal range; `has_denorm = denorm_absent`, and `denorm_min == min` per the standard |
| `max()` | the maxpos encoding `0111...1` | not representable as a double; `double(max())` is +inf |
| `lowest()` | -max() | the two's complement of maxpos |
| `min_exponent` | 1 - fbits | the standard defines it from min(): 2^(min_exponent - 1) = 2^-fbits |
| `max_exponent`, `max_exponent10` | INT_MAX | the true value is about log2(phi(8)), far past any int. Saturate, using the takum_exponent_cast precedent: an inverted range would be worse than a sentinel |
| `has_infinity` | false | |
| `has_quiet_NaN`, `quiet_NaN()` | true, NaR | |
| `has_signaling_NaN` | false | |
| `is_iec559`, `is_exact`, `is_modulo` | false | |
| `is_bounded` | true | |
| `round_style` | `round_to_nearest` | in the psi domain |

## Exact paths: exp and log; the awkward reciprocal

For x >= 0, `exp(phi(x)) = phi(x + 1)`. For x >= 1, `ln(phi(x)) = phi(x - 1)`. Both
follow directly from the definition of phi, and [C7] checks them to 4e-121 over 2000
points. On the encoding, which is l + f as fixed point:

- **exp X, for X >= 0, adds 1 to the magnitude bits.** It is exact, with no rounding.
  Starting from the top level it would need one more level, so it saturates (see the
  table above).
- **log X, for X >= 1, subtracts 1 from the magnitude bits.** It is exact.
- `log` for X in (0, 1) is negative with magnitude -ln f, which is not a shift. `exp`
  of a negative argument is a reciprocal (below). Both are ordinary computed
  functions.

The mathlib (#1696) should land these two shifts first. They are free, exact, and test
every other part of the encode and decode.

**The reciprocal is the awkward operation.** 1/X for a large X falls into level 0, and
level 0 has the underflow floor. For `lindex<16,3>`, 1/X rounds to 0 for every X at or
above 2^13. That covers every encoding from `0x3C9B` (psi = 3.7877464) upward:
**17253 of the 32767 positive encodings, 52.65%** [C8]. So `1/x` and `x/y` with
`|y| >> |x|` give 0 more often than not. The format does not tolerate this; it is what
SLI's reciprocal sign exists to fix. LI has no exact reciprocal path, and the tests
must expect 0 there, not flag it.

## Arithmetic

All arithmetic stays in the psi domain, and the operand values are never formed: no
intermediate overflows, whatever the level.

### Addition and subtraction (Clenshaw and Olver 1987)

Let `x = l + f >= y = m + g >= 0`, and let `z = psi(phi(x) +- phi(y))`. The algorithm
runs three sequences:

```text
a_j = 1 / phi(x - j)       a_(l-1) = e^-f,          a_(j-1) = e^(-1 / a_j)
b_j = phi(y - j) / phi(x - j)
                           b_(m-1) = a_(m-1) e^g,   b_(j-1) = e^(-(1 - b_j) / a_j)
                           (m = 0: b_0 = a_0 g)
c_j = phi(z - j) / phi(x - j)
                           c_0 = 1 +- b_0,          c_j = 1 + a_j ln c_(j-1)
```

The c sequence stops at the first j with `c_j < a_j`. The result is then at level j,
with index `c_j / a_j`. If no j stops it, `phi(z - l) = f + ln c_(l-1)`, and the
result is re-levelled from there; addition can move up a level. When l = 0, both
operands are level-0 fractions, and the sum is a plain fixed-point add.

**Correction:** the epic says the sequences "stay in [0, 1]". a and b do. c does not:
for addition, c_0 = 1 + b_0 is in [1, 2]. Measured over 6000 cases [C9]:

| sequence | range |
|---|---|
| a_j | [1.2e-13, 0.99988] |
| b_j | [2.9e-13, 0.99936] |
| c_j, addition | [1.0, 1.99936] |
| c_j, subtraction | [0.00064, 1.0] |

All of them are bounded by 2, so no value is ever formed. The sequence result agrees
with direct evaluation of psi(phi(x) +- phi(y)) to 2^-390 [C9].

**Subtraction needs a cancellation-free form.** When the two operands are close,
b_0 is near 1, and `c_0 = 1 - b_0` cancels. In double, two `lindex<32,3>` operands
that differ by one ulp (2^-28) give a result that is 9.27 ulps wrong, and at a
separation of 2^-40 it is 4410 ulps wrong [C13]. The fix is to carry `d_j = 1 - b_j`
instead of b_j, using `d_(j-1) = -expm1(-d_j / a_j)`, and to start it without
rounding:

- When the operands share a level (m = l): `d_(l-1) = -expm1(g - f)`. The difference
  g - f is exact in fixed point.
- When y is in the level just below x (m = l - 1):
  `d = -expm1((g - 1) - expm1(f))`.

With this form, the error stays near 2^-47 whatever the separation [C13]. This is the
form #1693 and #1695 implement. The level-1 minus level-0 case needs no special form:
its result lands in level 0, where error is absolute, and C13 measures it at 6e-9 ulp.

**`x - x` must be decided before the sequences.** It is exactly 0. But at levels 6 and
7, every a_j underflows to 0 in double, so the stop test `c_j < a_j` never fires, and
the sequences return x itself instead of 0. The kernel tests for equal operands first
[C13].

### Multiplication and division one level down

For x, y >= 1, `ln(XY) = phi(x - 1) + phi(y - 1)`. So
`psi(XY) = 1 + psi(phi(x - 1) + phi(y - 1))`, which is an addition one level down,
followed by moving the result up one level. Division is the same with a subtraction:
`ln(X / Y) = phi(x - 1) - phi(y - 1)`. This mirrors LNS, where multiplication is the
addition of logarithms; here it is the addition of generalized logarithms.

The mixed cases are where the implementation has to be careful:

- **One operand at level 0** (y < 1): `ln(XY) = phi(x - 1) + ln y`, with `ln y < 0`.
  While XY >= 1, this is a subtraction one level down. When XY < 1, the product falls
  into level 0, and it is `y * phi(x)` directly.
- **A quotient below 1**: `X / Y < 1` lands in level 0, where its psi is the value
  itself. It is subject to the underflow floor, as the reciprocal is.

[C10] checks the level-down product, including the level-0 cases, against direct
evaluation over 2000 pairs: the largest difference is 3e-120.

## Working precision

**Decision 4.** The kernel is a template on its working type `W`.

| nbits | W | evidence |
|---|---|---|
| <= 32 (fbits <= 28) | `double` | addition errors 2^-50.4 and subtraction errors 2^-44.0, measured over 20000 pairs [C12]; with the expm1 form, near-cancellation stays at 2^-47 [C13]. The half-ulp at fbits 28 is 2^-29, so this leaves about 15 bits of margin |
| > 32 | `dd_cascade` | double misses fbits 60 by 752 ulps in addition and 6.3e4 in subtraction [C12]. dd_cascade has the `exp`, `log` and `expm1` the kernel needs (`dd_cascade/math/functions/exponent.hpp`), and it served the same role for the takum cross-conversion (#1309) |

Two conditions apply to this table. [C12] sweeps psi only up to 5.5, where the 400-bit
reference is still cheap. At levels 6 and 7 the a_j underflow to 0 in double, which is
the correct limit, but #1693 must confirm it against psi-domain encodings at those
levels. #1693 also measures the per-level accuracy of phi and psi themselves in each
W.

### Measured: the kernel against the oracle (#1693)

`static/logarithmic/lindex/kernel/kernel.cpp` runs the kernel
(`number/shared/level_index_kernel.hpp`) over 592 pairs per format from
`tools/generators/level_index_gen.py`. The pairs cover every level, near cancellation
(1, 2, 3, 32 and 2^(fbits/2) ulps apart), operands straddling a level boundary, operands on
one, y = 0, and x - x. Errors are in psi-ulps, the largest by the level of x:

| level of x | double, fbits 28: add | sub | dd_cascade, fbits 60: add | sub |
|---|---|---|---|---|
| 0 | 6.4e-9 | 0 | 4.7e-15 | 0 |
| 1 | 4.4e-8 | 6.3e-8 | 1.6e-14 | 2.0e-14 |
| 2 | 5.1e-8 | 4.2e-8 | 3.0e-14 | 2.4e-14 |
| 3 | 5.3e-8 | 5.1e-8 | 2.6e-14 | 2.0e-14 |
| 4 | 4.2e-8 | 3.0e-7 | 2.4e-14 | 1.9e-14 |
| 5 | 3.8e-8 | 1.8e-8 | 2.8e-14 | 2.6e-14 |
| 6, 7 | 0 | 0 | 0 | 0 |

Every result is faithful and every one is correctly rounded, and x - x is exactly 0 at
every level. The test asserts 2^-16 psi-ulp for double and 2^-40 for dd_cascade, about 50
times the worst measured. This answers the two conditions above:

- **Levels 6 and 7 are exact in both W.** One psi-ulp there spans a factor that no
  operand can move, so every sum and difference is x plus a quantity below the format's
  resolution. The kernel returns x itself, which the oracle confirms.
- **double does not carry fbits 60.** The same pairs give up to 260 psi-ulps in addition
  and 9e18 in subtraction (the input index alone loses 7 bits), and 132 of 592 results are
  not even faithful. So `lindex<64,3>` needs dd_cascade, as the table above prescribes.

phi and psi themselves, on native values: psi(X) is accurate to 1.5e-16 absolute in double
and 2.7e-32 in dd_cascade at every level. phi(x) has relative error 2.4e-15 at level 3 and
1.2e-13 at level 4 in double (8.8e-30 in dd_cascade), because each level multiplies an
index error by phi'(x) / phi(x). This matters only for conversion to a native value, which
#1694 rounds once at the end; the arithmetic never forms phi.

`sw::math::constexpr_math` (#763) is not chosen. The arithmetic is runtime, and
nothing here needs to evaluate a transcendental at compile time. Constants such as the
level boundaries can come from the oracle as literals.

## Name

**Decision 1: `lindex<nbits, lbits = 3, bt = std::uint8_t>`, and `slindex<>` for
#1350.** `li` and `sli` are too short to grep for: they match unrelated identifiers
throughout the tree, and every later search would pay for that. The spelled-out names
are unique, and they read as what they are.

`lindex` and `slindex` are added to the PR-title scope allow-list in
`.github/workflows/conventional-commits.yml` in this PR, so that the PRs of parts 2 to
6 pass `lint-pr-title`.

## LI/SLI boundary and structure

The boundary follows the #1297 (takum) precedent: one shared kernel and two class
templates, and the boundary is drawn before any arithmetic is written. SLI is the same
phi and psi, plus a reciprocal bit and arithmetic modified for it (Clenshaw and Turner
1988). It is a different number system, not an LI flag. Their numeric_limits,
closure, reciprocal and mathlib all differ.

```text
include/sw/universal/number/shared/
  level_index_kernel.hpp   # phi / psi, the a/b/c sequences, the expm1 subtraction;
                           # templated on the working type W; shared by LI and SLI
include/sw/universal/number/lindex/
  lindex_fwd.hpp
  core.hpp                 # I/O-free: lindex<nbits, lbits, bt>, conversion, arithmetic
  numeric_limits.hpp
  lindex_traits.hpp        # is_lindex
  attributes.hpp
  manipulators.hpp         # type_tag, to_binary, color_print
  iostream.hpp
  mathlib.hpp, math/
  table.hpp
  lindex.hpp               # umbrella
static/logarithmic/lindex/{api,conversion,logic,arithmetic,math}/
```

This differs from the epic's layout in two ways. The kernel sits in `number/shared/`,
next to `multidouble_exact.hpp` (the #1667 precedent for code shared by several types),
not inside `lindex/`, where `slindex` would have to reach into another type's
directory. And the type uses the #1334 layering (core, manipulators, iostream,
umbrella) that the `new-number-type` skill prescribes.

## Oracle

[C2] rules out a value-domain oracle: no IEEE type reaches level 5. The oracle is
mpmath, run offline, and it emits **encodings**:

- **Conversion tables**: for selected doubles and for every encoding of
  `lindex<8,3>` and `lindex<16,3>`, the exactly rounded psi as an integer encoding.
- **Arithmetic tables**: every pair of `lindex<8,3>` encodings (65536 pairs per
  operator), with the correctly rounded result encoding for + - * /. They are
  computed in psi space by the same sequences as [C9], which never form a value.
  mpmath runs the sequences directly through level 5. At levels 6 and 7 even
  `a_j = 1/phi(x - j)` has an exponent too large to hold (it is about 10^-(10^1.6e6)),
  so the oracle replaces every a_j below 2^-(working bits) by 0. That is the same
  limit the double kernel takes, and #1693 has to bound the error it introduces.
- The generator is checked in under `tools/generators/`, beside this document's check
  script. Its outputs are committed, and it does not run during the build, following
  `qd_special_gen.py`.

Turner's 1989 Pascal SLI package is the historical reference implementation. It is not
needed: the formulas are re-derived and checked here.

## Corrections to the epic

1. **LI is overflow-free, not underflow-free.** Level 0 is fixed point, with a floor of
   2^-fbits [C3]. This is the reason SLI exists.
2. **`__float128` cannot be the oracle.** It reaches only psi = 4.80 [C2]. The oracle
   must emit encodings from mpmath.
3. **The a/b/c sequences are not all in [0, 1].** For addition, c is in [1, 2] [C9].
   They are still bounded, which is the property that matters.

Two findings go beyond the epic:

4. **Closure needs `lbits >= 3`.** With `lbits = 2`, squaring maxpos goes past the last
   level [C11].
5. **Subtraction needs the expm1 form.** The textbook `c_0 = 1 - b_0` loses about 9
   ulps at fbits 28 for operands one ulp apart [C13].

## References

See [Sources and what was verified](#sources-and-what-was-verified) for full citations
and DOIs.

- Clenshaw and Olver 1984, *J. ACM* 31(2): the value map.
- Clenshaw and Olver 1987, *SIAM J. Numer. Anal.* 24(2): the arithmetic.
- Clenshaw, Olver and Turner 1989, LNM 1397: the survey.
- Lozier and Olver 1990, *SIAM J. Numer. Anal.* 27(5): closure and precision.
- Clenshaw and Turner 1988, *IMA J. Numer. Anal.* 8(4): SLI.
- Mikaitis 2024, arXiv:2402.02301: the formulas restated; a MATLAB simulator.
- `docs/takum-design.md`: the shared-kernel, two-types precedent (#1297).
