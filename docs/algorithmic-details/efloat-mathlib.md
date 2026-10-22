# efloat: correctly rounded arithmetic and math library

`efloat<nlimbs>` is Universal's adaptive-precision binary float: a sign, an `int64_t` exponent
and a significand of any number of 32-bit limbs. This page is the design record of its
arithmetic and its math library: what each function computes and how, why it computes it that
way, what was tried and dropped, how the results are verified, how fast they are next to MPFR,
and what is still open. It is written for whoever works on the speed or the accuracy next.

The code carries the local detail: every header states its algorithm, the error bound of each
unrounded evaluation and the issue that introduced it. This page ties those together, and keeps
the history and the measurements, which the code does not.

- Headers: `include/sw/universal/number/efloat/` (`efloat_impl.hpp`, `efloat_mpn.hpp`, `math/*.hpp`)
- Regression tests: `elastic/efloat/{api,arithmetic,conversion,math}/`
- Accuracy against MPFR: `benchmark/accuracy/efloat/mpfr_check.cpp`
- Speed against MPFR: `benchmark/performance/arithmetic/efloat/mpfr/mpfr_gap.cpp`
- Speed against mpmath and gmpy2: `benchmark/performance/arithmetic/efloat/mpmath_gap.{cpp,py}`
- Tracking: Epic #1763 (beat MPFR at 128-8192 bits), #1782 (this page), #1783 (cbrt, hypot,
  fma, fmod and remainder made correctly rounded)

## 1. The contract

Every function below is designed to be **correctly rounded**:
the result is the exact mathematical value rounded once to the result's precision, in whichever
of the four IEEE rounding modes `efloat_rounding_mode` selects (nearest-even, toward zero,
toward +inf, toward -inf). What backs that is not a proof. The arithmetic, `sqrt`, `hypot`,
`fma` and the conversions round exact results, and `fmod` and `remainder` return them, so they
are correct by construction. The other functions
rest on two things that are measured, not proven: the error bound of each unrounded evaluation
(section 2) and a retry limit that has not been shown to suffice (section 2, item 3). What
is established is agreement with MPFR on every case checked, 32-8192 bits, in all four modes
(section 6). Signed zeros, infinities, NaN and the exception flags follow C and IEEE 754.

**The result's precision** (#1757, PR #1758). A value constructed from a native `int`, `float`
or `double` carries no precision of its own. An operation takes the wider precision of the
operands that carry one; when none does, the result rounds at the default precision
(`set_default_precision()`, else `32 nlimbs` bits) and carries none either. A math function's
result has its argument's precision (`result_precision()` in `math/exponent.hpp`). So
`x * 3 / 7` with `x` at 640 bits runs at 640 bits, not at the type's width.

Before #1758, `efloat<100>(3)` carried 3200 bits, and `x op literal` silently ran at 3200 bits
(#1714 measured `exp` at 2048 bits running 9x slower than the 600 it was asked for). Of the four
options weighed in #1757 (exact literals sized to their bits; an exact flag that does not vote;
documentation only; mixed-type operators), the chosen design combines the second and the
fourth.

## 2. How a function is correctly rounded

All of the math library uses one scheme, in `math/exponent.hpp`.

1. **An unrounded evaluation with a stated error bound.** Each function has a
   `*_working(x, target)` that evaluates at `target + exp_log_guard_bits` (64) bits and returns
   a value within `2^(scale - target - 64 + error_bits)` of the exact result. Each function's
   `error_bits` is a named constant next to its code (table in section 4).
2. **Ziv's test.** `can_round(y, precision, correct)` reads `y`'s bits from just past the
   rounding position down to the error bound. If they are neither all 0 nor all 1, no rounding
   boundary lies within the error, and `y` rounds to the same value as the exact result in
   every mode.
3. **Ziv's strategy.** `round_ziv(evaluate, precision, error_bits)` evaluates, tests, and if the
   test fails, evaluates again at twice the target. What it is given is a finite approximation;
   the premise is about the exact result, which must differ from every rounding boundary, so
   that some precision separates the two and an unbounded loop would terminate. The boundaries
   are binary fractions. Every caller returns the arguments whose exact result is a binary
   fraction before reaching `round_ziv` (item 4): `exp2` at an integer, `log2` of a power of 2,
   `pow` at an exact power, and so on. What remains is irrational (transcendental, or algebraic
   like `exp2(1/2) = sqrt(2)`) or rational but not a binary fraction, so never a boundary.
   The loop is bounded, though: after **eight retries**, at 256 times the target, it rounds the
   last value whether or not the test passed. So the guarantee is correct rounding unless the
   exact result lies within about `2^-(256 P)` of a rounding boundary, relative to its size;
   past that, the result is the last evaluation rounded once, which is faithful but not proven
   correct. There is no proof that eight suffice for any function here. The evidence is that
   every MPFR check has agreed, and the heuristic of the table-maker's-dilemma literature
   (Lefevre and Muller's searches of the hardest cases for double precision): a P-bit
   argument's result is expected to come no closer to a boundary than about `2^-(2P + c)`,
   for a small c, which a few doublings already resolve. The retry count is not reported,
   so a run cannot tell whether it reached the limit.
4. **Exact results are formed, not approximated.** A boundary is a binary fraction, so only an
   exact result can sit on one: `exp2(k)`, `log10(10^k)`, `pow(x, n)` with a narrow enough
   result, `sqrt` of a square, `acos(1) = +0`, `tgamma(n) = (n-1)!` and so on are computed
   outright and rounded once (#1774). They keep the argument's precision (`round_result`).
5. **Leading term plus a signed correction.** When the result is `L + c` with `L` exact and
   `|c| < 2^-(P+4) |L|` of known sign (sin of a tiny x, erf near +-1, exp of a tiny x),
   `round_near(L, direction, P)` rounds `L` moved `2^-(P+32)` of itself toward `c`, as MPFR's
   `mpfr_round_near_x` does.

The retry rate is low because the guard is wide: about `2^-29` per call for sin/cos/tan and
`2^-45` for exp/log (#1772); `exp_log_guard_bits = 64` is larger than a first try needs.

**The error bounds are measured, not proven.** Each `*_error_bits` was set from evaluations at
`P` against evaluations at `2P + 128`, in all four modes (the intermediate operations round in
the current mode), across 32-8192 bits, and then given slack. The measured worst cases sit at
least 17-18 bits inside the declared bounds (#1772, #1774, #1779, #1781).

**Ziv hides a missing guard.** A subtraction that cancels exactly leaves trailing zeros, which
`can_round` reads as "too close to call", so it retries at twice the precision and the final
result is still right. A deleted cancellation guard therefore passes every comparison of
results; only a direct check of the `*_working` error contract catches it (#1779). Tests that
cover a new function must check the contract, not just the rounded results.

## 3. Arithmetic

Storage is a `std::vector<uint32_t>`, most significant limb first. The kernels in
`efloat_mpn.hpp` pack operands into 64-bit limbs, least significant first, work on those, and
round the exact result once into the destination (`assign_rounded`), with no allocation up to
2048 bits.

| operation | algorithm | thresholds | PR |
|---|---|---|---|
| `+` `-` | aligned sum in a stack frame, two guard limbs plus a sticky bit; in-place `+=` | | #1758, #1766, #1767 |
| `*` | 64x64->128 schoolbook, dedicated squaring (`x * x`), subtractive Karatsuba, blocks for unbalanced operands | `karatsuba_threshold = 24`, `karatsuba_sqr_threshold = 32` limbs | #1767 |
| `/` | exact quotient: schoolbook with the Moller-Granlund 3-by-2 reciprocal, Burnikel-Ziegler divide and conquer (GMP's `dcpi1_div_qr`) above the threshold; remainder becomes the sticky bit | `dc_div_threshold = 16` limbs | #1741, #1769 |
| `/ int` | one-limb divisor, Moller-Granlund 2-by-1 reciprocal, in place | | #1766 |
| `sqrt` | exact integer root and remainder by Zimmermann's Karatsuba square root (GMP's `mpn_sqrtrem`), remainder as sticky bit | | #1770 |
| to `double`/`float` | round once to the format, subnormals included | | #1778 |
| from decimal text | convert at the target plus 64 guard bits, retry wider only when the guard bits read `0111...1` | widths 8192, 32768 | #1740 |

**History and rejected alternatives**

- Division was bit-serial until #1741 (one compare/subtract/shift per quotient bit, about 37
  multiplies' worth at 608 bits). Knuth's Algorithm D replaced it, then #1769 moved it to
  64-bit limbs and divide and conquer. **Newton reciprocal division was rejected** (#1769): at
  8192 bits a multiply costs 4.87 us and DC division 8.4 us, about 1.7 multiplies, while Newton
  needs three or more multiplies even with short products. GMP itself only switches to it past
  about 1000 limbs.
- `sqrt` was Newton's iteration from a one-bit seed with a fixed 7 steps, which capped it at
  about 97 digits (#1140, fixed in #1143). It then misrounded near halfway points and set
  Inexact on exact roots, because it rounded from 64 guard bits without knowing the remainder
  (73,500-case MPFR check on the old code: 409 wrong results and 1,151 wrong Inexact flags).
  **A Newton reciprocal square root was built and rejected** (#1770): written in efloat
  operations it was slower than the old code below 8192 bits (0.686 us against 0.346 at 128
  bits, while the exact `sqrtrem` took 0.126). The lesson recorded there: estimate the overhead
  of efloat operations before building an algorithm out of them; small sizes need limb-level
  code.
- Before #1767 a multiply made about six allocations. 64-bit limbs and Karatsuba made `a*b`
  2.7x faster at 512 bits and 5.8x at 8192.
- Storage stays 32-bit and most significant first: `bits()`, `setlimb()`, the math library and
  ereal read `_limb` directly, so #1767 changed only the kernels (#1762 steps 2-3 not done).
- Zero signs were wrong in `*`, `/` and exact cancellation until #1773 (#1768).

## 4. The functions

| function | algorithm | error bits | file |
|---|---|---|---|
| `exp` | `x = k ln2 + r`, `r` halved below `2^-T`, `T = 0.4 sqrt(F)`; series in fixed point by rectangular splitting, squared back; `abs(k) < 2^62` | 16 | exponent.hpp |
| `exp2`, `exp10`, `expm1` | through `exp`; `expm1` sums without the leading 1 and doubles back with `u(u+2)`; exact `2^k`, `10^k` | 20 | exponent.hpp |
| `log` | below the switch: table of `log(j/512)` plus `2 atanh(v)`, `abs(v) < 2^-10`; above it: AGM, `log s = pi s / (2 AGM(s, 4))` | 16 | logarithm.hpp |
| `log2`, `log10`, `log1p` | through `log`; `log1p` sums its own series for small `abs(x)` | 20 | logarithm.hpp |
| `sin`, `cos`, `tan` | reduce by `pi/2` at widened precision; versine `v = 1 - cos r` from a halved argument, series by rectangular splitting, doubled back | 32 | trigonometry.hpp |
| `asin`, `acos`, `atan`, `atan2` | one `atan` kernel: reflection, argument halving, series | 24 | trigonometry.hpp |
| `sinh` ... `atanh` | `expm1` and `log1p` forms that do not cancel | 24 | hyperbolic.hpp |
| `pow(x, y)` | `exp(y log x)`, `log x` widened by the magnitude of `y log x`; exact powers outright | 24 | pow.hpp |
| `pow(x, n)` | binary powering with `bit_width(n)` extra bits | 8 | pow.hpp |
| `tgamma`, `lgamma` | Stirling series after shifting `x` up past `0.3 W`; reflection below 0; exact `(n-1)!` | 22 | error_and_gamma.hpp |
| `erf`, `erfc` | positive-term series `2/sqrt(pi) e^{-x^2} sum t_n`; asymptotic series for large `x`; `1 - erf` with the cancelled bits added | 0 (32 guard bits) | error_and_gamma.hpp |
| `cbrt` | Newton from a double seed, precision doubling each step; exact cubes found and rounded once | 8 | sqrt.hpp |
| `hypot` | exact squares, exact sum (or a proxy that rounds alike), one exact root | exact | hypot.hpp |
| `fma` | exact product, exact sum (or a proxy), one rounding | exact | numerics.hpp |
| `fmod`, `remainder` | exact integer remainder, `2^d mod y` by binary powering | exact | fractional.hpp |

"Error bits" is the function's `*_error_bits`: its unrounded value lies within
`2^(scale - target - 64 + error_bits)` of the exact result.

### 4.1 exp family

- **Reduction.** `k = round(x / ln2)` from a double estimate, corrected once from `r` when `x`
  passes `2^52`; `ln2` at the working precision plus 64 bits, so `k ln2` keeps its accuracy for
  any `|k| < 2^62` (#1777). Before #1777 the range stopped at `|x| < 2^17`. Per-`k` widths of
  `ln2` were rejected: they re-grew the cache and cost 9-12% at 8192 bits.
- **Series** (#1788). `r` is halved `h` times, to `s = |r| / 2^h` below `2^-T`, and `exp(+-s)`
  is summed in fixed point (`exp_fixed` in `exponent.hpp`).
  - **Fixed point.** Every value is a natural number `V = floor(v 2^F)` in `n + 1` 64-bit limbs,
    with `F = 64 n` fraction bits and `F >= P + 64 + T + 16`. The binary point sits on a limb
    boundary, so a product drops whole limbs. Inside the series nothing normalizes, merges
    precisions or rounds; each step truncates.
  - **Rectangular splitting** (Smith 1989), with blocks of `m ~ sqrt(N)` terms. With
    `T_k = A_i P_j / D_ij` for `k = i m + j`, where `P_j = s^j` and `D_ij = (i m + 1) ...
    (i m + j)`, the series is a Horner scheme in `P_m / D_im` over the blocks
    `B_i = sum_j P_j / D_ij`. Each block is formed over its common denominator with one-limb
    multipliers (`addmul_1`) and one one-limb division. Only the `m` powers and the `N / m`
    Horner steps are full products. `m` is capped so that a block's divisor fits in a limb.
  - **Negative arguments.** The terms alternate. Every partial value stays positive, so the
    kernel needs no signed arithmetic and no final reciprocal.
  - **Squaring back.** The `h` squarings are fixed-point squares, `n + 1` limbs each.
- **Error.** There are about `N + 3N/m + m` truncations of `2^-F` each, under `2^10` of them at
  8192 bits. The tail of the series lies below `2^-(F + 3)`, and the `h` squarings multiply
  the relative error by `2^h`. That leaves `2^(h + 11 - F) <= 2^-(P + 69)`, inside
  `exp_error_bits`. Measured against MPFR from 24 to 8192 bits, the worst case lies 33-73 bits
  inside the declared bound. `elastic/efloat/math/exp_kernel.cpp` checks the bound against a
  plain efloat Taylor series at `2P + 256` bits. 11 of 12 mutations of the kernel fail that
  test. The twelfth halves one time fewer, which changes only the speed, because the term count
  comes from the actual `s`.
- **The halving target.** `T = 0.4 sqrt(F)` is the measured optimum, to within the noise of a
  flat curve, from 64 to 8192 bits: at 128 bits `T` is 6, and at 8192 bits it is 34-37. A cost
  model that priced a scalar step at a tenth of a product chose 34 at 128 bits and 46 at 8192,
  which ran 1.8x and 1.05x slower: it underrated what a squaring of a few limbs costs.
- **History.** Before #1739: `ln2` was a 59-digit literal (results stopped at about 2^-200) and
  the series stopped at 100 terms (about 680 bits). At 608 bits `exp` took 957 us against
  mpmath's 11 (#1714); #1741 brought that to 20 us and #1754 to 15.6 us.
- **History.** Before #1788 the series ran in efloat arithmetic: one rounded multiply,
  divide and add per term, after up to `sqrt(P)` halvings. That was 2.3-3.3x MPFR. The fixed-point
  kernel runs at 0.7-0.9x MPFR up to 512 bits, 1.1x at 2048 bits and 1.5-1.75x at 8192 bits,
  over arguments with `|r|` from 0 to `ln2 / 2` of either sign.
- **Work per call** (`|r| ~ 1/4`): at 128 bits, `N = 28` terms in blocks of `m = 6`, 9 full
  products and 5 squarings on 5 limbs. At 8192 bits, 198 terms with `m = 8` (capped so the
  divisor fits in a limb), 31 products and 35 squarings on 131 limbs.
- **Open.** Binary splitting (MPFR's `exp_3`) for the widest precisions. At 8192 bits the 66
  full products are most of the time, and efloat's 8192-bit product is itself 2.2x MPFR's, so
  Toom-3 would help there first. A two-limb block divisor would allow `m ~ sqrt(N) = 14`. The same kernel would also serve the series of `sin`/`cos` and
  `atanh`, and `expm1` below 1/2 still sums in efloat arithmetic. A cheaper first Ziv try is
  not needed for exp, which is already below MPFR's time at small precisions.

### 4.2 log family

- **Table and series** (#1754, mpmath's method below 2500 bits): `x = 2^e m`,
  `m` in `[sqrt(1/2), sqrt(2))`, `c = j/512` the nearest grid point, and
  `log x = e ln2 + log c + 2 atanh((m - c)/(m + c))`, gaining about 21 bits a term. Keeping `m`
  around 1 means `x` near 1 needs only the series, with no cancellation against `e ln2`. The
  363 table entries are computed on first use and cached per thread.
- **AGM** (#1771; Brent 1976, Brent and Zimmermann *Modern Computer Arithmetic* 4.8.2, as MPFR
  does it): `s = x 2^m > 2^(W/2)`, `log s = pi s / (2 AGM(s, 4))`, `log x = log s - m ln2`.
  About `2 log2 W` steps, each a multiply, an exact root and a halving.
- **The switch.** AGM once `W > 144 max(10, k)` for `x` within `2^-k` of 1
  (`log_agm_bits_per_series_bit`, measured: the series costs `W M(W) / k`, the AGM
  `log(W) M(W)`). Without the near-1 case the crossover is about 1450 bits.
- **History.** Newton's iteration on `exp` (about 4.5 `exp` calls per `log`) until #1754 below
  8192 bits and #1771 above. At 8192 bits that Newton had an absolute, not relative, error: for
  `x = 1 - 2^-7481 r` it was off by `2^711` ulps. #1771 made `log` 5x faster at 8192 bits
  (1441 to 289 us).
- **Open.** The Sasaki-Kanada theta-function start (about a fifth fewer AGM steps).

### 4.3 Trigonometric

- **Reduction** (`reduce_half_pi`): `x = k pi/2 + r`, `|r| <= pi/4`, with `pi` computed by
  Machin's formula at the needed width and cached. The subtraction cancels about
  `scale(x) - scale(r)` bits, and `x` can lie as close to `k pi/2` as its own precision allows,
  so the reduction retries wider until `r` keeps `bits + 64` correct bits, up to three times
  `2(P + scale(x))`. This is a division at widened precision, not Payne-Hanek: its cost grows
  with `scale(x)`. `|x| < 0.785` skips the division (#1766).
- **Versine kernel** (#1760): `v = 1 - cos r`, then `cos r = 1 - v` and
  `|sin r| = sqrt(v (2 - v))`, neither of which cancels. `r` is halved `h ~ 0.25 sqrt(P) + 6`
  times, the series in `r^2` is summed by rectangular splitting over `u ~ 0.21 P^0.35` partial
  sums (grid-searched, the optimum is flat: `(h, u) = (11, 2)` at 512 bits, `(17, 3)` at 2048,
  `(28, 5)` at 8192; #1766), and doubled back by `v(2y) = 2(2v - v^2)`, one squaring each.
- **Rejected** (#1760): tapering the plain sin/cos series alone reached only 2.2x mpmath;
  halving with sin/cos double-angle formulas costs two multiplies a step and did not pay.
- **atan** (#1774): one kernel for `atan`, `asin`, `acos`, `atan2`: reflect `|x| > 1`, halve by
  `a / (sqrt(a^2 + 1) + 1)` about `sqrt(W/10)` times, then the series. It replaced a series
  that gained 0.6 bits a term near `|x| = 0.8` (asin at 8192 bits: 13.8 ms to 0.5 ms).
- **Open.** Payne-Hanek for huge arguments; binary splitting at high precision (MPFR's
  `sin_cos`); mpmath caches a sin/cos table below 400 bits, which is why efloat trails it at
  128 bits.

### 4.4 Hyperbolic

`sinh`, `cosh`, `tanh` from `expm1`, as `u(u+2)/(2(u+1))` and `w/(w+2)`, so nothing cancels as
`x -> 0`; `asinh`, `acosh`, `atanh` as `log1p` of an argument that does not cancel
(`atanh` works on `|x|`; before #1774 `atanh` near -1 was off by up to `2^900` ulps).

### 4.5 pow

`pow(x, y) = exp(y log x)` with `log x` carried at the target plus the magnitude of `y log x`,
so its error is not multiplied by `y` (#1739). Exact powers are found first and rounded once:
`x = 2^e` with `e y` an integer; `x^n` whose `n (bits(x) - 1)` fits the precision; `y = m/2^k`
with `|x|` a perfect `2^k`-th power (a mod-8 filter rejects most non-squares). `pow(x, int)`
uses binary powering with guard bits; before #1774 it was off by up to 32 ulps from repeated
rounded products.

### 4.6 tgamma, lgamma (#1779)

- Stirling's series (DLMF 5.11.1) after shifting `x` up to `z >= 0.3 W`
  (`gamma_shift_per_bit`): `lgamma x = lgamma(x + N) - log(x (x+1) ... (x+N-1))`. The remainder
  is bounded by the first omitted term (DLMF 5.11(ii)).
- The coefficients `B_2k / (2k(2k-1))` come from the Brent-Harvey tangent-number recurrence
  (*Modern Computer Arithmetic* 4.7.2): positive additions and small-integer products, cached
  per thread.
- Below 0, reflection with `n = round(x)` and `f = x - n`, which is exact. Near the zeros of
  lgamma at 1 and 2, and near the poles, the working precision is raised by the cancellation.
- `tgamma = sign * exp(lgamma)`, with lgamma to absolute accuracy; `(n-1)!` is exact while it
  fits twice the result's width.
- Before: `std::tgamma(double(x))`, 53 bits at most, inf past 171.6.
- **Open.** `lgamma` near 1 at 8192 bits is 7.5x slower than MPFR, because the cancellation is
  only discovered by a first pass. Estimating it up front from `|x - 1|` and `|x - 2|` is the
  fix.

### 4.7 erf, erfc (#1781)

- `erf x = 2/sqrt(pi) e^{-x^2} sum t_n`, `t_n = t_{n-1} 2x^2/(2n+1)` (A&S 7.1.6, MPFR's
  `mpfr_erf_0`): all terms positive, so nothing cancels; `x^2` exact; tapered precision.
- `erfc` for large `x`: the asymptotic series (DLMF 7.12.1), used when its smallest term lies
  below the working precision (`x^2 log2 e >= W + 8`); otherwise `1 - erf x` with the cancelled
  bits added (bounded by A&S 7.1.13); for `x < 0`, `1 + erf |x|`.
- Results within `2^-(P+5)` of +-1 (or of 2, for `erfc` of a negative x) round through
  `round_near`. `erfc` returns +0 with Underflow only past efloat's exponent range.
- Before: `std::erf(double(x))`; `erfc` was 0 from `x = 27.2` on, and all four modes returned
  the same value.

### 4.8 cbrt, hypot, fma, fmod, remainder (#1783)

Until #1783 these five were computed at the result's precision but rounded more than once, and
`mpfr_check` found 889 to 2,063 wrong results each in 3,234 cases.

- **Exact sums.** `hypot` and `fma` add two exact values: `x^2 + y^2`, or the exact product
  `x y` (`exact_product`: `a + b` bits for operands of `a` and `b`) plus `z`. `exact_sum` in
  `exponent.hpp` adds them at the width that holds the sum exactly, except when the smaller
  lies wholly below the last bit `2^L` of the larger carried at `pad` bits. Then it adds a
  proxy of the same sign, `2^(L-2)`, instead. The true and the proxied sums lie strictly
  between the larger and its neighbour `2^(L-1)` away, so they round alike to any precision up
  to `pad - 2` bits, and their square roots alike up to `(pad - 4) / 2`. That keeps the width
  bounded whatever the exponent gap. `fma` passes `pad = P + 2` and rounds the sum once;
  `hypot` passes `2P + 4` and takes the exact root (`sqrt_to`, which rounds once).
- **fma at the range's ends** (#1786). When `scale(x) + scale(y)` leaves `[min_scale,
  max_scale)`, the exact product would overflow or underflow, though `x y + z` may not. Both
  terms are then scaled by `2^-t`, where `t` is the larger term's scale. A term more than
  `2^62` binades below the larger one is raised to `2^(min_scale + 2)`. It still lies far below
  the larger term's last bit, so `exact_sum` proxies it and the rounding is unchanged. The sum
  rounds at `P`, and multiplying by `2^t` brings it back, overflowing or underflowing as a product
  would. For a finite `x y` and an infinite `z`, `fma` returns `z`; computing `x * y` first could
  overflow and give `inf - inf`.
- **hypot** scales both arguments by `2^-k`, `k = scale(max)`, so the squares stay inside the
  exponent range, and multiplies the root back. When the arguments are more than
  `max(bits(max), P + 2) + 8` binades apart, the result is `max` plus a positive amount below
  its last bit, and a proxy sum gives it directly.
- **cbrt.** `a = m 8^k`, `m` in `[1, 8)`; Newton's iteration `y <- y + (m / y^2 - y) / 3` from
  `std::cbrt` of `double(m)`, at a precision that doubles with each step to 16 bits past the
  working precision, then once more there. Measured within `2^-(target + 64 + 12)`, declared
  `cbrt_error_bits = 8`; the regression test checks that bound directly. Ziv's strategy rounds
  it. A boundary is a binary fraction, so only an exact root can be one. Before Ziv, the first
  evaluation is rounded to nearest at `P + 32` bits and cubed: if that gives `a` exactly, the
  root is exact and rounds once. Before #1783, Newton ran at full precision from a one-bit
  seed and rounded unchecked: 6-33x MPFR's time and wrong in a quarter of the cases.
- **fmod** is exact: its result always fits the wider operand precision. With `2^lo` the
  smaller of the operands' last bits, `X = |x| 2^-lo = mx 2^d` and `Y = |y| 2^-lo` are
  integers, and `X mod Y = (mx mod Y)(2^d mod Y) mod Y`. `2^d mod Y` takes `log2 d` squarings
  modulo `Y`, so a quotient of `2^(2^40)` costs about 40 modular squarings, not `2^40`
  subtraction steps. Each `mod` is an exact division, truncated and corrected by one `Y`.
- **remainder** takes `|x| mod 2|y|`: whether that is at least `|y|` is the quotient's parity,
  which breaks the tie when the remainder is exactly `|y| / 2`.
- Special values as IEEE 754: `fmod(x, inf) = x` (before, NaN), `fma(inf, 0, z)` NaN, zero
  results signed as `x` for `fmod` and `remainder` and by the rounding mode for `fma`'s exact
  cancellation.

### 4.9 Constants

The library computes its own constants at the working precision and caches the widest value
per thread: `ln2 = 2 atanh(1/3)`, `ln10 = 3 ln2 + 2 atanh(1/9)`, `pi` by Machin's formula,
`atan(1/2) = atan(1/3) + atan(1/7)`. The public `efloat_pi/e/phi<nlimbs>()` are 1000-digit
literals that carry the type's width; #1739 stopped using them inside functions, because a
value widened past the type's width was reduced with a narrower `pi`. The first call at a new
precision pays to fill the caches, which matters for benchmarks.

## 5. Verification

Four kinds of check, each catching what the others miss.

1. **Regression tests** (`elastic/efloat/`, run in CI): exact oracles where one exists
   (`arithmetic/correct_rounding.cpp`, `conversion/subnormal_rounding.cpp`,
   `math/sqrt_precision.cpp` with Hensel-lifted radicands that reach the sticky-decisive case),
   independent second methods (`math/error_and_gamma.cpp` checks erf against the alternating
   Maclaurin series carried with the cancelled bits), mpmath-generated vectors
   (`tools/generators/efloat_exp_log_gen.py` -> `verification/efloat_exp_log_vectors.hpp`,
   `efloat_parse_gen.py` -> `efloat_parse_vectors.hpp`), and the `*_working` error contracts.
   Each test added with a fix fails on the code before it (the PR states how many cases).
2. **MPFR, bit for bit** (`benchmark/accuracy/efloat/mpfr_check.cpp`, built when MPFR is
   found): every function in all four modes at 32-8192 bits, five argument styles including
   each function's special arguments. The ctest runs a quick sweep up to 512 bits; with no
   arguments it runs the full one in about 25 s.
3. **Bit identity** for speed work: a change that should not alter results is run over a
   sweep of several hundred thousand results against the previous headers (`git show main:` or
   `git archive` of every changed header into a scratch include tree) and must differ nowhere.
4. **Mutation testing**: each PR since #1767 broke its own fix in named ways (a guard set to
   0, a sign flipped, a branch removed) and recorded which mutants the tests catch. Survivors
   are listed in the PRs; most are slack absorbed by another guard.

## 6. Measured accuracy

`mpfr_check`, full run (scale 1, up to 8192 bits), MPFR 4.2.2, after #1783. On main at 2076edb7,
before it, the same run found `cbrt` 889 wrong, `hypot` 1656, `fma` 926, `fmod` 1546 and
`remainder` 2063, and 0 everywhere else.

Per function: 1000 cases at each of 32, 64 and 128 bits, 200 at 512, 30 at 2048 and 4 at
8192, a quarter in each rounding mode. A case is wrong unless value, sign and precision all
match MPFR. `double` and `float` are the conversions, half of them in or below the subnormal
range.

| function | cases | wrong | |
|---|---:|---:|---|
| `add` | 3234 | 0 | |
| `sub` | 3234 | 0 | |
| `mul` | 3234 | 0 | |
| `div` | 3234 | 0 | |
| `sqrt` | 3234 | 0 | |
| `cbrt` | 3234 | 0 | |
| `hypot` | 3234 | 0 | |
| `fma` | 3234 | 0 | |
| `fmod` | 3234 | 0 | |
| `remainder` | 3234 | 0 | |
| `exp` | 3234 | 0 | |
| `exp2` | 3234 | 0 | |
| `exp10` | 3234 | 0 | |
| `expm1` | 3234 | 0 | |
| `log` | 3234 | 0 | |
| `log2` | 3234 | 0 | |
| `log10` | 3234 | 0 | |
| `log1p` | 3234 | 0 | |
| `sin` | 3234 | 0 | |
| `cos` | 3234 | 0 | |
| `tan` | 3234 | 0 | |
| `asin` | 3234 | 0 | |
| `acos` | 3234 | 0 | |
| `atan` | 3234 | 0 | |
| `atan2` | 3234 | 0 | |
| `sinh` | 3234 | 0 | |
| `cosh` | 3234 | 0 | |
| `tanh` | 3234 | 0 | |
| `asinh` | 3234 | 0 | |
| `acosh` | 3234 | 0 | |
| `atanh` | 3234 | 0 | |
| `pow` | 3234 | 0 | |
| `powi` | 3234 | 0 | |
| `tgamma` | 3234 | 0 | |
| `lgamma` | 3234 | 0 | |
| `erf` | 3234 | 0 | |
| `erfc` | 3183 | 0 | |
| `double` | 3234 | 0 | |
| `float` | 3234 | 0 | |

The sweep takes about 25 s; `mpfr_check 10` takes ten times the cases. The checks run for each
PR were larger and are recorded there: 42,520 cases for log (#1771), 53,728 for exp, sin, cos
and tan (#1772), 228,000 for the functions of #1774, 73,500 for sqrt (#1770), 321,600
conversions (#1778) and 19,338 each for gamma (#1779) and erf (#1781), all with 0 wrong after
the fix. 51 `erfc` cases are not compared: their results lie below MPFR's smallest exponent, where
efloat returns +0 with Underflow.

## 7. Measured speed

`mpfr_gap`, best of three, warm caches, microseconds per call, round to nearest.
Measured 2026-10-21 with #1788 (the exp kernel) on top of main at 4def52f7, Intel Core i7-12700K, g++ 15.2 `-O3 -march=native`,
MPFR 4.2.2, GMP 6.3.0; `a = 0.718...`, `b = 1.414...`, `x = 1 + a`. Each cell is
efloat / MPFR in microseconds, then the ratio.

| operation | 128 bits | | 512 bits | | 2048 bits | | 8192 bits | |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `a+b` | 0.028 / 0.013 | 2.1x | 0.035 / 0.020 | 1.8x | 0.074 / 0.048 | 1.5x | 0.232 / 0.167 | 1.4x |
| `a*b` | 0.026 / 0.018 | 1.5x | 0.062 / 0.042 | 1.5x | 0.485 / 0.266 | 1.8x | 4.727 / 2.159 | 2.2x |
| `b/a` | 0.053 / 0.028 | 1.9x | 0.139 / 0.079 | 1.8x | 0.885 / 0.481 | 1.8x | 8.314 / 4.305 | 1.9x |
| `a/7` | 0.043 / 0.024 | 1.8x | 0.076 / 0.040 | 1.9x | 0.220 / 0.104 | 2.1x | 0.811 / 0.371 | 2.2x |
| `exp(a)` | 0.592 / 0.730 | 0.8x | 1.878 / 2.152 | 0.9x | 17.24 / 15.71 | 1.1x | 285.6 / 175.1 | 1.6x |
| `log(x)` | 1.194 / 1.118 | 1.1x | 3.738 / 3.178 | 1.2x | 34.72 / 14.74 | 2.4x | 277.4 / 123.5 | 2.2x |
| `sin(a)` | 1.983 / 0.734 | 2.7x | 4.593 / 2.078 | 2.2x | 27.48 / 14.13 | 1.9x | 295.9 / 193.5 | 1.5x |
| `sqrt(b)` | 0.129 / 0.059 | 2.2x | 0.271 / 0.133 | 2.0x | 0.993 / 0.435 | 2.3x | 6.379 / 3.067 | 2.1x |
| `fma(a,b,a)` | 0.120 / 0.029 | 4.1x | 0.166 / 0.049 | 3.4x | 0.657 / 0.328 | 2.0x | 5.295 / 3.166 | 1.7x |
| `hypot(a,b)` | 0.363 / 0.170 | 2.1x | 0.657 / 0.321 | 2.0x | 2.525 / 1.014 | 2.5x | 17.65 / 8.102 | 2.2x |
| `cbrt(b)` | 0.735 / 0.288 | 2.6x | 1.203 / 0.466 | 2.6x | 3.940 / 1.303 | 3.0x | 24.77 / 7.537 | 3.3x |
| `exp2(a)` | 0.738 / 1.018 | 0.7x | 2.173 / 2.648 | 0.8x | 19.03 / 17.18 | 1.1x | 301.3 / 191.0 | 1.6x |
| `exp10(a)` | 0.759 / 2.307 | 0.3x | 2.260 / 5.987 | 0.4x | 19.39 / 31.00 | 0.6x | 305.4 / 314.1 | 1.0x |
| `expm1(a)` | 0.619 / 0.815 | 0.8x | 1.896 / 2.359 | 0.8x | 17.27 / 15.56 | 1.1x | 285.5 / 178.3 | 1.6x |
| `log2(x)` | 1.299 / 1.274 | 1.0x | 3.952 / 3.445 | 1.1x | 35.44 / 15.35 | 2.3x | 285.4 / 129.0 | 2.2x |
| `log10(x)` | 1.368 / 2.438 | 0.6x | 3.979 / 6.820 | 0.6x | 35.62 / 30.42 | 1.2x | 285.2 / 253.5 | 1.1x |
| `log1p(a)` | 1.333 / 1.203 | 1.1x | 3.913 / 3.280 | 1.2x | 34.48 / 14.85 | 2.3x | 279.8 / 126.3 | 2.2x |
| `cos(a)` | 1.745 / 0.526 | 3.3x | 4.164 / 1.680 | 2.5x | 25.80 / 13.02 | 2.0x | 284.7 / 188.2 | 1.5x |
| `tan(a)` | 2.140 / 1.010 | 2.1x | 4.893 / 2.331 | 2.1x | 28.69 / 15.33 | 1.9x | 308.5 / 201.7 | 1.5x |
| `asin(a)` | 4.267 / 2.830 | 1.5x | 11.19 / 10.90 | 1.0x | 74.78 / 43.59 | 1.7x | 1049 / 344.0 | 3.0x |
| `acos(a)` | 3.724 / 2.882 | 1.3x | 10.28 / 10.83 | 0.9x | 70.45 / 43.17 | 1.6x | 1011 / 344.0 | 2.9x |
| `atan(a)` | 3.648 / 2.357 | 1.5x | 10.03 / 8.824 | 1.1x | 70.44 / 41.67 | 1.7x | 1007 / 332.7 | 3.0x |
| `atan2(a,b)` | 3.489 / 2.359 | 1.5x | 9.645 / 10.82 | 0.9x | 68.60 / 44.40 | 1.5x | 995.7 / 323.5 | 3.1x |
| `sinh(a)` | 0.797 / 0.849 | 0.9x | 2.190 / 2.445 | 0.9x | 18.50 / 16.02 | 1.2x | 296.1 / 182.8 | 1.6x |
| `cosh(a)` | 0.772 / 0.849 | 0.9x | 2.185 / 2.441 | 0.9x | 18.47 / 16.04 | 1.2x | 295.8 / 181.8 | 1.6x |
| `tanh(a)` | 0.771 / 0.943 | 0.8x | 2.223 / 2.508 | 0.9x | 18.81 / 16.48 | 1.1x | 299.4 / 185.6 | 1.6x |
| `asinh(a)` | 1.717 / 1.353 | 1.3x | 4.525 / 3.623 | 1.2x | 38.18 / 15.49 | 2.5x | 305.2 / 129.1 | 2.4x |
| `acosh(b)` | 1.602 / 1.385 | 1.2x | 4.167 / 3.663 | 1.1x | 37.32 / 15.54 | 2.4x | 297.6 / 128.7 | 2.3x |
| `atanh(a)` | 1.582 / 1.379 | 1.1x | 4.545 / 3.495 | 1.3x | 35.84 / 15.37 | 2.3x | 288.0 / 129.1 | 2.2x |
| `pow(b,a)` | 2.079 / 2.204 | 0.9x | 5.843 / 5.948 | 1.0x | 54.26 / 31.62 | 1.7x | 583.4 / 313.8 | 1.9x |
| `pow(b,7)` | 0.368 / 0.103 | 3.6x | 0.562 / 0.189 | 3.0x | 2.335 / 1.262 | 1.8x | 18.11 / 11.00 | 1.6x |
| `tgamma(b)` | 29.23 / 7.245 | 4.0x | 108.7 / 32.71 | 3.3x | 1745 / 361.6 | 4.8x | 47037 / 7355 | 6.4x |
| `lgamma(b)` | 39.69 / 7.535 | 5.3x | 214.3 / 33.56 | 6.4x | 4394 / 758.3 | 5.8x | 133818 / 7236 | 18.5x |
| `erf(a)` | 4.799 / 2.930 | 1.6x | 15.66 / 15.78 | 1.0x | 151.8 / 171.1 | 0.9x | 2835 / 2990 | 0.9x |
| `erfc(a)` | 4.933 / 3.188 | 1.5x | 16.32 / 16.22 | 1.0x | 152.9 / 172.7 | 0.9x | 2838 / 3004 | 0.9x |

Read the ratio column as the work left on Epic #1763.

- **Ahead or level with MPFR:**
  - since #1788, the exp family up to 512 bits: `exp`, `exp2`, `expm1`, `sinh`, `cosh` and
    `tanh` at 0.7-0.9x, `exp10` at 0.3-0.6x up to 2048 bits and 1.0x at 8192, `pow(b, a)` at
    0.9-1.0x up to 512 bits;
  - `erf` and `erfc` from 512 bits up;
  - `log10` (0.6x at 128 and 512 bits), and the inverse trigonometric functions at 512 bits.
- **exp family** (#1788): 1.1-1.2x at 2048 bits and 1.6x at 8192, against 2.3-3.3x before.
  `erf`, `erfc` gained 10-25% and `tgamma` 1-7% through their exp calls. `tgamma`'s time is
  mostly `lgamma`'s series, so it stays at 3-6x.
- **cbrt, hypot, fma** (remeasured after #1783): `cbrt` 2.5-3.3x (was 6-33x), `hypot`
  2.1-2.5x, `fma` 1.7-4.1x. `fma` is slower than before at small precisions (0.12 us at 128 bits
  against 0.045): it now forms the exact product and sum, where it used to round twice and was
  wrong in 29% of the cases.
- **The primitives set the floor:** `a*b`, `b/a` and `sqrt` run 1.5-2.3x MPFR, and every
  function built on them inherits that. trig 1.5-3x, log-based 1.1-2.4x.
- **Largest gaps:** `lgamma(1.414...)` at
  8192 bits (18x; on #1779's random arguments it was 2.1-7.5x, and the cost #1779 identified
  is the second, wider pass that cancellation near the zeros at 1 and 2 forces; this argument
  has not been profiled), `tgamma` (4-6x), and the inverse trig family at 8192 bits (3x).

**Benchmark hygiene.** Single runs of the mpmath comparison vary by 15-50%; compare branches
with best-of-5 runs on an idle machine. Warm the caches first. The first-call cost of a new
precision (filling `ln2`, `pi`, the log table, the Bernoulli numbers) is real but separate.

## 8. History

| PR | merged | change |
|---|---|---|
| #1143 | 2026-07-18 | sqrt/cbrt Newton iteration count grows with precision (was capped at ~97 digits) |
| #1739 | 2026-10-18 | computed ln2, ln10, pi, atan(1/2); 64 guard bits; series to convergence (exp/log/trig/pow stopped at ~200 bits) |
| #1740 | 2026-10-18 | decimal parsing at the value's precision (long literals were truncated or 0) |
| #1741 | 2026-10-18 | Knuth division; small-integer divisors; normalize fix |
| #1754 | 2026-10-19 | log table + series; tapered exp series |
| #1758 | 2026-10-19 | precision semantics of literals; `+` `-` `/` rounding fixes; guard bits in hyperbolic, sqrt, cbrt |
| #1760 | 2026-10-19 | versine kernel for sin/cos |
| #1764, #1765 | 2026-10-19 | mpmath_gap at 128-8192 bits; mpfr_gap |
| #1766 | 2026-10-20 | in-place `+=`, squaring, rectangular splitting, one-limb division |
| #1767 | 2026-10-20 | 64-bit limb kernels, Karatsuba |
| #1769 | 2026-10-20 | divide and conquer division |
| #1770 | 2026-10-20 | exact sqrt (sqrtrem) |
| #1771 | 2026-10-20 | log by AGM above the switch; first Ziv retry |
| #1772 | 2026-10-20 | Ziv for exp, sin, cos, tan; `round_near` |
| #1773 | 2026-10-20 | zero and infinity signs |
| #1774 | 2026-10-21 | Ziv for every remaining function; exact results; atan kernel |
| #1777 | 2026-10-21 | exp range to `2^(2^62)` |
| #1778 | 2026-10-21 | correctly rounded conversion to float/double below the smallest normal |
| #1779 | 2026-10-21 | tgamma, lgamma at any precision (were through `double`) |
| #1781 | 2026-10-21 | erf, erfc at any precision (were through `double`) |
| #1783 | 2026-10-21 | cbrt, hypot, fma correctly rounded; fmod, remainder exact |
| #1786 | 2026-10-21 | `std::numeric_limits<efloat>`; arithmetic overflows and underflows at the math library's range, scale in `[1 - 2^62, 2^62 - 1]` |
| #1788 | 2026-10-21 | exp series in fixed point by rectangular splitting: exp family 2.3-3.3x MPFR to 0.7-1.7x |

## 9. Open work

**Library gaps.** An exact integer cube root with remainder, like `sqrtrem`, would make `cbrt`
cheaper than Newton under Ziv.

**Speed** (Epic #1763): short (high-half) products and MPFR's can-round on them; Toom-3; short
division (`divhigh`); the fixed-point series kernel of exp (#1788) carried over to sin/cos,
atanh and expm1; a cheap first Ziv try at `p + log2(p)` guard bits instead of 64; binary
splitting for exp and sin/cos;
binary-splitting constants; Sasaki-Kanada AGM start; up-front cancellation estimate for lgamma;
heap-free, least-significant-first storage; Payne-Hanek reduction for huge trig arguments;
ratio guards in CI beyond the existing division-to-multiply one.

## 10. Working on it

For a speed change:

1. Measure first: `mpfr_gap name1,name2` before and after, best of several runs.
2. Prove the results did not move: a bit-identity sweep against the old headers.
3. Run `mpfr_check` in full.
4. If the change touches a guard or an error bound, re-measure the `*_working` contract at
   32-8192 bits in all four modes, and keep the declared bound at least 16 bits clear.

For an accuracy change:

1. Reproduce against MPFR (`mpfr_check 4 fname`), or against an exact oracle.
2. Add a regression test that fails on the old code, including the `*_working` contract if
   there is one, and check that each part of the fix, reverted alone, fails it.
3. Mark the commit `BEHAVIOUR-CHANGE:` (see CLAUDE.md), since results move.
