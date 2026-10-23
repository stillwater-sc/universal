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
| `exp` | `x = k ln2 + r`, `r` halved below `2^-T`, `T = 0.25 sqrt(F)`; series in fixed point by tapered rectangular splitting, squared back; `abs(k) < 2^62` | 16 | exponent.hpp |
| `exp2`, `exp10`, `expm1` | through `exp`; `expm1` sums without the leading 1 and doubles back with `u(u+2)`; exact `2^k`, `10^k` | 20 | exponent.hpp |
| `log` | below the switch (about 45000 bits): table of `log(j/512)` plus `2 atanh(v)`, `abs(v) < 2^-10`, the series in fixed point by rectangular splitting; above it: AGM, `log s = pi s / (2 AGM(s, 4))` | 16 | logarithm.hpp |
| `log2`, `log10`, `log1p` | through `log`; `log1p` sums its own series for small `abs(x)` | 20 | logarithm.hpp |
| `sin`, `cos`, `tan` | reduce by `pi/2` at widened precision; versine `v = 1 - cos r` from a halved argument, `v / y^2` summed in fixed point by the series engine of exp, doubled back in scaled fixed point | 32 | trigonometry.hpp |
| `asin`, `acos`, `atan`, `atan2` | one `atan` kernel: reflection, argument halved below `2^-T`, `T = 0.08 sqrt(W)`; `atan(y) / y` summed in fixed point by rectangular splitting | 24 | trigonometry.hpp |
| `sinh` ... `atanh` | `expm1` and `log1p` forms that do not cancel | 24 | hyperbolic.hpp |
| `pow(x, y)` | `exp(y log x)`, `log x` widened by the magnitude of `y log x`; exact powers outright | 24 | pow.hpp |
| `pow(x, n)` | binary powering with `bit_width(n)` extra bits | 8 | pow.hpp |
| `tgamma`, `lgamma` | Stirling series after shifting `x` up past `0.15 W`, by a tapered Horner scheme; the shift's factors multiplied in groups; reflection below 0; working precision predicted; exact `(n-1)!` | 22 | error_and_gamma.hpp |
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
  is summed in fixed point (`fixed_series` in `exponent.hpp`).
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
- **Tapering** (#1790). Block `b` enters the sum scaled by `T_(bm) < 2^-G_b`, so it is formed
  on the top `n - q_b` limbs, with `64 q_b <= G_b - 8`. Its truncation, scaled by `T_(bm)`,
  stays below `2^-(F + 8)`. On its way down a block, the Horner value gains the limbs the next
  block keeps. This took `exp` from 1.1x to 0.8x MPFR at 2048 bits and from 1.6x to 1.1x at 8192.
- **The halving target.** `T = 0.25 sqrt(F)` is the optimum measured with tapering, to within
  the noise of a flat curve, from 128 to 8192 bits: 4 at 128 bits and 23 at 8192. Before
  tapering it was `0.4 sqrt(F)` (#1788). A cost model that priced a scalar step at a tenth of a
  product chose 34 at 128 bits and 46 at 8192, and ran 1.8x and 1.05x slower: it underrated
  what a squaring of a few limbs costs.
- **History.** Before #1739: `ln2` was a 59-digit literal (results stopped at about 2^-200) and
  the series stopped at 100 terms (about 680 bits). At 608 bits `exp` took 957 us against
  mpmath's 11 (#1714); #1741 brought that to 20 us and #1754 to 15.6 us.
- **History.** Before #1788 the series ran in efloat arithmetic: one rounded multiply,
  divide and add per term, after up to `sqrt(P)` halvings. That was 2.3-3.3x MPFR. The fixed-point
  kernel (#1788) ran at 0.7-0.9x MPFR up to 512 bits, 1.1x at 2048 bits and 1.5-1.75x at 8192,
  over arguments with `|r|` from 0 to `ln2 / 2` of either sign. Tapering (#1790) brought that to
  0.7-0.8x up to 2048 bits and 1.1x at 8192.
- **Work per call** (`|r| ~ 1/4`, #1790 targets): at 128 bits, `N = 34` terms in blocks of
  `m = 6`, 10 products and 3 squarings on 5 limbs. At 8192 bits, 281 terms with `m = 7` (capped
  so the divisor fits in a limb), 46 products, most of them tapered, and 22 squarings on 131
  limbs.
- **Open.** Binary splitting (MPFR's `exp_3`) for the widest precisions. At 8192 bits the
  products are most of the time, and efloat's 8192-bit product is itself 2.2x MPFR's, so
  Toom-3 would help there first. A two-limb block divisor would allow `m ~ sqrt(N) = 14`. The same kernel would also serve the series of `sin`/`cos` and
  `atanh`, and `expm1` below 1/2 still sums in efloat arithmetic. A cheaper first Ziv try is
  not needed for exp, which is already below MPFR's time at small precisions.

### 4.2 log family

- **Table and series** (#1754, mpmath's method below 2500 bits): `x = 2^e m`,
  `m` in `[sqrt(1/2), sqrt(2))`, `c = j/512` the nearest grid point, and
  `log x = e ln2 + log c + 2 atanh((m - c)/(m + c))`, gaining about 21 bits a term. Keeping `m`
  around 1 means `x` near 1 needs only the series, with no cancellation against `e ln2`. The
  363 table entries are computed on first use and cached per thread.
- **The series in fixed point** (#1795): `atanh(v) = v S(z)`, `S(z) = 1 + z/3 + z^2/5 + ...`,
  `z = v^2` (`atanh_working_small`), on atan's engine (`fixed_series::arctangent_series`, #1792)
  without the alternation. `S` lies in `[1, 1.1)` for `abs(v) < 1/2`, so `v S` keeps its
  relative precision as `x -> 1`. Block length `0.7 sqrt(N)`, as for atan, measured again:
  `0.5` and `1.0` ran 3-7% slower at 8192 bits, `1.4` 20% slower.
- **Table entries** (#1795) come from the same routine, `log(j/512) = 2 atanh(u/w)` with
  `abs(u/w) < 0.18`: 7-10x cheaper than the efloat series they replaced (2048 bits: 16-55 us
  an entry, from 108-456; 8192 bits: 0.2-0.36 ms, from 1.4-3.3 ms). That matters now that the
  table runs at high precision, where each new `j` fills its entry once: over `mpfr_check`'s
  cold-cache arguments `log` averages 40 us at 2048 bits (main 83) and 1.4 ms at 8192 (main 2.3).
- **AGM** (#1771; Brent 1976, Brent and Zimmermann *Modern Computer Arithmetic* 4.8.2, as MPFR
  does it): `s = x 2^m > 2^(W/2)`, `log s = pi s / (2 AGM(s, 4))`, `log x = log s - m ln2`.
  About `2 log2 W` steps, each a multiply, an exact root and a halving.
- **The switch.** AGM once `W > 4500 max(10, k)` for `x` within `2^-k` of 1
  (`log_agm_bits_per_series_bit`, measured: the series costs `W M(W) / k`, the AGM
  `log(W) M(W)`). From the table the two cross near 45000 working bits; for `x = 1 + 2^-k` near
  `5000 k` (k = 20-40, at 65536 and 131072 bits). With the efloat series the constant was 144
  (#1771) and the AGM ran from 1440 bits: at 8192 bits it takes 276 us, the fixed-point series
  140, and at 2048 bits 34 against 9.6.
- **Error** (#1795): `log_working` lies 14.6-40 bits inside `log_error_bits = 16` at 24-8192
  bits, measured against MPFR over random `x`, `x` next to 1, at and half-way between grid points
  and either side of `sqrt(2)`. Main's series lay 14.1-40 bits inside, and the AGM it ran from
  1440 bits up lay 65-70 inside, with its own guard bits. The minimum is the series' final
  roundings at the working precision (the doc comment's `2^-(W - 3)`), not the series.
  `elastic/efloat/math/log_kernel.cpp` checks the bound against four exact roots and the plain
  series at `2P + 256` bits.
- **History.** Newton's iteration on `exp` (about 4.5 `exp` calls per `log`) until #1754 below
  8192 bits and #1771 above. At 8192 bits that Newton had an absolute, not relative, error: for
  `x = 1 - 2^-7481 r` it was off by `2^711` ulps. #1771 made `log` 5x faster at 8192 bits
  (1441 to 289 us).
- **Open.** The Sasaki-Kanada theta-function start (about a fifth fewer AGM steps). A second
  table step to `abs(v) < 2^-20` (#1795 left it out: the targets were met without it, and a
  2^10-entry table would hold about 1 MB a thread at 8192 bits). `ln2` and `ln10` still sum
  `2 atanh(1/3)` and `2 atanh(1/9)` term by term on first use at each precision.

### 4.3 Trigonometric

- **Reduction** (`reduce_half_pi`): `x = k pi/2 + r`, `|r| <= pi/4`, with `pi` computed by
  Machin's formula at the needed width and cached. The subtraction cancels about
  `scale(x) - scale(r)` bits, and `x` can lie as close to `k pi/2` as its own precision allows,
  so the reduction retries wider until `r` keeps `bits + 64` correct bits, up to three times
  `2(P + scale(x))`. This is a division at widened precision, not Payne-Hanek: its cost grows
  with `scale(x)`. `|x| < 0.785` skips the division (#1766).
  - **Below `2^40`** (#1790), `k = round(x 2/pi)` comes from `double`. It lies within `2^-12` of
    `x 2/pi`, so `k` is the nearest integer, or its neighbour next to a half-integer, which
    leaves `|r|` at most `2^-12 pi/2` past `pi/4`; that is harmless to the versine. `r = x - k
    pi/2` is still formed, and widened, at full precision. This replaced the division at the
    working precision, a fifth of `sin` at 128 bits.
- **Tiny arguments** (#1790). `sin`, `cos` and `tan` return `x` or `1`, moved toward the next
  term, through `round_near` when that term lies below `2^-(P+4)` of the value. The cuts were
  2 bits more conservative than the series allow. They are now `2 scale(x) < -P - 3` for `sin`
  (`x^2/6 < 2^-(P+4)`) and `< -P - 4` for `cos` and `tan`. At 128 bits that takes `sin(1e-20)`
  from 0.51 to 0.125 us; MPFR takes 0.017. `mpfr_check` draws its trig special arguments on
  both sides of the cuts. A `sin` cut loosened 5 bits past the bound fails it.
- **Versine kernel** (#1760): `v = 1 - cos r`, then `cos r = 1 - v` and
  `|sin r| = sqrt(v (2 - v))`, neither of which cancels.
- **In fixed point** (#1790). `r` is halved `h` times, to `y = |r| / 2^h` below `2^-T`. With
  `z = y^2`, `v(y) = y^2 W(z) / 2`, where
  `W(z) = 2 sum_k (-1)^k z^k / (2k + 2)! = 1 - z/12 + z^2/360 - ...`
  lies in `(0.9, 1]` whatever the size of `y`.
  - **The series.** `W` is summed on exp's engine (`fixed_series`) with the divisors
    `d(t) = (2t + 1)(2t + 2)`, alternating and tapered. Fixed point loses nothing as `r -> 0`.
  - **Forming `v`.** `v = y^2 W / 2` comes from a `z` scaled to its own leading bit, as
    `V = v 2^(F + E)`.
  - **Doubling back.** `v(2y) = 4v - 2v^2` becomes `V' = V - V^2 / 2^(F + E + 1)` with `E`
    falling by 2: one square and a subtraction per doubling.
- **Error of the fixed-point versine.**
  - A doubling multiplies the relative error by `(1 - v) / (1 - v / 2) < 1` and adds one
    truncation, so the error does not grow with `h`.
  - With `F >= working + 24`, the kernel is within `2^-(working + 13)`. It returns `v` rounded
    to the working precision (`r`'s precision plus `h`), as the efloat kernel did.
  - Measured against MPFR from 24 to 8192 bits, `trig_working` lies 30.6-55 bits inside its
    declared bound for `sin`, `cos` and `tan`, and the versine within one ulp at `r`'s
    precision.
  - `elastic/efloat/math/trig_kernel.cpp` checks both bounds against plain efloat Taylor series.
    13 of 15 mutations of the versine and of the tapering fail it. Of the other two, turning
    tapering off changes only the speed, and dropping one limb too many in the taper is caught
    by `exp_kernel.cpp`.
- **The halving target.** `T = 0.22 sqrt(F)` was measured best, the optimum flat: 3 at 128 bits,
  10 at 2048, 20 at 8192. Each halving gains the series in `z` two bits a term, so the
  optimum lies below exp's.
- **Work per call:** at 128 bits, 19 terms in blocks of 5, 8 products and 2 doublings on 4
  limbs. At 8192 bits, 155 terms in blocks of only 3, because the divisors are about `4 t^2`,
  54 products and 19 doublings.
- **Measured speed:** `sin`, `cos` and `tan` run 0.9-1.1x MPFR up to 2048 bits and 1.1-1.2x at
  8192 on the benchmark argument. Over arguments from `1e-20` to 100, inside and outside
  `pi/4` and near `pi/2`, they run 0.3-1.4x at 512 bits and up; the worst is `cos(1.2)`, at
  1.4x at 512 bits. At 128 bits they run 0.4-1.4x, with three exceptions:
  - `cos(1.2)` at 2.1x;
  - `cos(-10.3)` at 1.6x;
  - tiny `x`, at 3.6-4.3x against MPFR's 0.017 us fast path.
- **History.** The efloat kernel before #1790 (`h ~ 0.25 sqrt(P) + 6` halvings, `u ~ 0.21
  P^0.35` partial sums, `(h, u) = (11, 2)` at 512 bits, `(17, 3)` at 2048, `(28, 5)` at 8192;
  #1766) ran every term and doubling as a rounded efloat operation: `sin` at 2.7x / 2.2x / 1.9x
  / 1.5x MPFR at 128 / 512 / 2048 / 8192 bits.
- **Rejected** (#1760): tapering the plain sin/cos series alone reached only 2.2x mpmath;
  halving with sin/cos double-angle formulas costs two multiplies a step and did not pay.
- **atan** (#1774): one kernel for `atan`, `asin`, `acos`, `atan2`: reflect `|x| > 1`, halve by
  `a / (sqrt(a^2 + 1) + 1)`, then the series. It replaced a series that gained 0.6 bits a term
  near `|x| = 0.8` (asin at 8192 bits: 13.8 ms to 0.5 ms).
- **atan in fixed point** (#1792). With `y` the halved argument and `z = y^2`,
  `atan(y) = y S(z)`, `S(z) = 1 - z/3 + z^2/5 - ...`, and `S` lies in `(0.9, 1]`: summed with
  `F >= working + 24` fraction bits it keeps its relative precision for any `y`
  (`fixed_series::arctangent_series`). Its divisors `2k + 1` do not chain from term to term as
  exp's do, so a block is `sum_j (-1)^j z^j / (2(i m + j) + 1)` and the Horner step multiplies
  by `z^m` alone. A block is summed in groups whose divisors' product fits in a limb: one
  multiplication by a limb per term and one division per group. Tapering as in exp, with
  `G_b = b m a`.
- **Halving target** (#1792): `y` below `2^-T`, `T = 0.08 sqrt(W)` for the working precision
  `W`, at least 2, measured best
  over a flat optimum: `T = 2` up to 512 bits, 4 at 2048, 7 at 8192. A halving costs a root
  and a quotient, four or five products, where a term costs one multiplication by a limb, so
  far fewer halvings pay than for exp (`0.25 sqrt(F)`) or the versine (`0.22 sqrt(F)`). The
  efloat series before #1792 halved `sqrt(W/10) + 1` times, 29 at 8192 bits, and the halvings
  were 55% of its time. Block length `0.7 sqrt(N)`: `1.0 sqrt(N)` ran 2-6% slower from 512 to
  8192 bits, `2 sqrt(N)` 30% slower at 8192.
- **Error** (#1792): `atan_working` lies 21-46 bits inside its declared `inverse_trig_error_bits
  = 24` at 24-8192 bits, measured against MPFR over `atan`, `asin`, `acos` and `atan2`; the
  kernel it replaced lay 19.5-27 bits inside. `elastic/efloat/math/atan_kernel.cpp` checks the
  bound against Euler's series at `3P + 256` bits.
- **Open.**
  - Payne-Hanek for huge arguments.
  - Binary splitting at high precision (MPFR's `sin_cos`).
  - Past `2^40` the reduction still divides.
  - Odd quadrants form `sin r` as `sqrt(v (2 - v))`: `cos(1.2)` at 128 bits is 2.1x MPFR, the worst
    measured case. A sine series of its own in fixed point would remove the root.
  - A two-limb block divisor. The versine's divisors are about `4 t^2`, which caps its blocks at
    3-5 terms.
  - mpmath caches a sin/cos table below 400 bits.

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

### 4.6 tgamma, lgamma (#1779, #1797)

- Stirling's series (DLMF 5.11.1) after shifting `x` up to `z >= 0.15 W`
  (`gamma_shift_per_bit`): `lgamma x = lgamma(x + N) - log(x (x+1) ... (x+N-1))`. The remainder
  is bounded by the first omitted term (DLMF 5.11(ii)). The series' smallest term lies about
  `9.06 z` bits down, so `z0 = 0.11 W` already reaches `2^-W`; 0.15 was measured by sweep
  (#1797): 0.3 spends shift factors where Stirling terms are now cheaper, and 0.12 adds terms.
- The coefficients `B_2k / (2k(2k-1))` come from the Brent-Harvey tangent-number recurrence
  (*Modern Computer Arithmetic* 4.7.2): positive additions and small-integer products, cached
  per thread.
- The series is summed by Horner's scheme in `1/z^2` (`stirling_sum`, #1797), one product a
  term. Term `k` lies `d_k` bits below the first, so its partial sum is formed at
  `W - d_k + 16` bits, tapered as exp's series is (#1754).
- The shift's product (`shift_product`, #1797) multiplies its factors in groups of up to 8:
  `(x + j0) ... (x + j0 + g - 1)` is the polynomial `sum e_i x^i` with natural coefficients
  below `2^62`, from `x^2 .. x^8` computed once, so a group costs `g` products by a 64-bit
  integer and one full product. Groups shorten from about 2048 bits, where the coefficients
  reach `2^62`.
- Below 0, reflection with `n = round(x)` and `f = x - n`, which is exact.
- The working precision is predicted before the first pass (`lgamma_precision`, #1797): `E`,
  the scale of the largest part (`(z - 1/2) log z`, the log of the shift's product, the log of
  the reflection's sine), less the scale of `lgamma x`, both estimated in double. Near the
  zeros at 1 and 2 the estimate comes from `|x - 1|` and `|x - 2|`, which a double would lose.
  The shift alone cancels about `log2 z0` bits, 11 at 8192 bits, more than the 8 spare bits
  #1779 started with, so every argument used to take a second pass at twice the width. A pass
  that still falls short, next to the zeros below 0 where `std::lgamma` is no guide, is
  repeated with the shortfall added rather than at twice the width.
- `tgamma = sign * exp(lgamma)`, with lgamma to absolute accuracy, at `needed + E` bits, also
  predicted (`tgamma_precision`); `(n-1)!` is exact while it fits twice the result's width.
  The coarse lgamma pass that judges exp's range runs only for `|x| >= 2^50` or tiny `x`.
- Before #1779: `std::tgamma(double(x))`, 53 bits at most, inf past 171.6.

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
Measured 2026-10-22 with #1795 (the fixed-point log series) on top of main at 4a4e692b, and the `tgamma` and `lgamma` rows
with #1797 on top of main at cbeb5b46, Intel Core i7-12700K, g++ 15.2 `-O3 -march=native`,
MPFR 4.2.2, GMP 6.3.0; `a = 0.718...`, `b = 1.414...`, `x = 1 + a`. Each cell is
efloat / MPFR in microseconds, then the ratio.

| operation | 128 bits | | 512 bits | | 2048 bits | | 8192 bits | |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `a+b` | 0.029 / 0.013 | 2.2x | 0.035 / 0.020 | 1.7x | 0.072 / 0.048 | 1.5x | 0.235 / 0.167 | 1.4x |
| `a*b` | 0.027 / 0.018 | 1.5x | 0.062 / 0.042 | 1.5x | 0.506 / 0.266 | 1.9x | 4.739 / 2.156 | 2.2x |
| `b/a` | 0.052 / 0.028 | 1.8x | 0.135 / 0.079 | 1.7x | 0.909 / 0.483 | 1.9x | 8.358 / 4.293 | 1.9x |
| `a/7` | 0.044 / 0.024 | 1.8x | 0.077 / 0.040 | 1.9x | 0.222 / 0.104 | 2.1x | 0.813 / 0.374 | 2.2x |
| `exp(a)` | 0.584 / 0.721 | 0.8x | 1.485 / 2.138 | 0.7x | 12.28 / 15.70 | 0.8x | 191.7 / 175.2 | 1.1x |
| `log(x)` | 0.778 / 1.108 | 0.7x | 1.459 / 3.184 | 0.5x | 9.938 / 14.77 | 0.7x | 139.7 / 123.5 | 1.1x |
| `sin(a)` | 0.834 / 0.732 | 1.1x | 1.907 / 2.084 | 0.9x | 14.60 / 14.14 | 1.0x | 226.1 / 193.6 | 1.2x |
| `sqrt(b)` | 0.125 / 0.059 | 2.1x | 0.257 / 0.133 | 1.9x | 0.964 / 0.437 | 2.2x | 6.380 / 3.071 | 2.1x |
| `fma(a,b,a)` | 0.120 / 0.029 | 4.2x | 0.169 / 0.049 | 3.4x | 0.657 / 0.328 | 2.0x | 5.292 / 3.137 | 1.7x |
| `hypot(a,b)` | 0.376 / 0.170 | 2.2x | 0.650 / 0.321 | 2.0x | 2.535 / 1.017 | 2.5x | 17.65 / 8.049 | 2.2x |
| `cbrt(b)` | 0.744 / 0.288 | 2.6x | 1.197 / 0.456 | 2.6x | 3.965 / 1.304 | 3.0x | 25.05 / 7.533 | 3.3x |
| `exp2(a)` | 0.736 / 1.020 | 0.7x | 1.795 / 2.671 | 0.7x | 14.05 / 17.18 | 0.8x | 207.5 / 191.0 | 1.1x |
| `exp10(a)` | 0.763 / 2.319 | 0.3x | 1.890 / 5.989 | 0.3x | 14.51 / 30.99 | 0.5x | 211.5 / 312.7 | 0.7x |
| `expm1(a)` | 0.618 / 0.819 | 0.8x | 1.521 / 2.353 | 0.6x | 12.29 / 15.54 | 0.8x | 191.4 / 178.0 | 1.1x |
| `log2(x)` | 0.893 / 1.286 | 0.7x | 1.680 / 3.449 | 0.5x | 10.89 / 15.31 | 0.7x | 149.1 / 128.7 | 1.2x |
| `log10(x)` | 0.954 / 2.450 | 0.4x | 1.732 / 6.837 | 0.3x | 10.97 / 30.37 | 0.4x | 149.3 / 253.9 | 0.6x |
| `log1p(a)` | 0.902 / 1.205 | 0.7x | 1.604 / 3.282 | 0.5x | 9.954 / 14.86 | 0.7x | 140.6 / 123.5 | 1.1x |
| `cos(a)` | 0.623 / 0.529 | 1.2x | 1.505 / 1.680 | 0.9x | 12.73 / 12.97 | 1.0x | 215.2 / 188.0 | 1.1x |
| `tan(a)` | 1.006 / 0.994 | 1.0x | 2.170 / 2.335 | 0.9x | 15.53 / 15.33 | 1.0x | 235.5 / 201.3 | 1.2x |
| `asin(a)` | 2.212 / 2.827 | 0.8x | 5.159 / 10.86 | 0.5x | 33.18 / 43.47 | 0.8x | 367.5 / 338.8 | 1.1x |
| `acos(a)` | 1.661 / 2.850 | 0.6x | 4.218 / 10.83 | 0.4x | 28.93 / 43.16 | 0.7x | 331.2 / 338.8 | 1.0x |
| `atan(a)` | 1.544 / 2.350 | 0.7x | 3.668 / 8.817 | 0.4x | 28.05 / 41.88 | 0.7x | 325.3 / 329.4 | 1.0x |
| `atan2(a,b)` | 1.326 / 2.354 | 0.6x | 3.737 / 10.88 | 0.3x | 27.57 / 44.60 | 0.6x | 322.8 / 323.2 | 1.0x |
| `sinh(a)` | 0.803 / 0.858 | 0.9x | 1.815 / 2.467 | 0.7x | 13.50 / 16.00 | 0.8x | 201.3 / 182.6 | 1.1x |
| `cosh(a)` | 0.782 / 0.850 | 0.9x | 1.811 / 2.492 | 0.7x | 13.49 / 16.00 | 0.8x | 200.7 / 181.3 | 1.1x |
| `tanh(a)` | 0.759 / 0.943 | 0.8x | 1.824 / 2.514 | 0.7x | 13.82 / 16.45 | 0.8x | 204.5 / 184.7 | 1.1x |
| `asinh(a)` | 1.282 / 1.357 | 0.9x | 2.310 / 3.649 | 0.6x | 12.50 / 15.57 | 0.8x | 155.2 / 128.5 | 1.2x |
| `acosh(b)` | 1.196 / 1.382 | 0.9x | 2.122 / 3.664 | 0.6x | 11.32 / 15.52 | 0.7x | 142.1 / 128.6 | 1.1x |
| `atanh(a)` | 1.073 / 1.393 | 0.8x | 1.903 / 3.496 | 0.5x | 11.74 / 15.37 | 0.8x | 158.6 / 128.6 | 1.2x |
| `pow(b,a)` | 1.660 / 2.210 | 0.8x | 3.355 / 5.955 | 0.6x | 23.51 / 31.63 | 0.7x | 341.0 / 316.4 | 1.1x |
| `pow(b,7)` | 0.378 / 0.104 | 3.6x | 0.522 / 0.189 | 2.8x | 2.338 / 1.259 | 1.9x | 18.15 / 11.19 | 1.6x |
| `tgamma(b)` | 10.26 / 7.183 | 1.4x | 30.30 / 32.74 | 0.9x | 250.0 / 362.7 | 0.7x | 4743 / 7354 | 0.6x |
| `lgamma(b)` | 10.00 / 7.518 | 1.3x | 28.81 / 33.61 | 0.9x | 231.6 / 757.6 | 0.3x | 4532 / 7229 | 0.6x |
| `erf(a)` | 4.985 / 2.909 | 1.7x | 15.38 / 15.71 | 1.0x | 146.9 / 171.1 | 0.9x | 2747 / 2995 | 0.9x |
| `erfc(a)` | 4.920 / 3.175 | 1.5x | 15.83 / 16.23 | 1.0x | 148.0 / 174.0 | 0.9x | 2747 / 3002 | 0.9x |

Read the ratio column as the work left on Epic #1763.

- **Ahead or level with MPFR:**
  - the exp family (#1788, #1790) up to 2048 bits: `exp`, `exp2`, `expm1`, `sinh`, `cosh` and
    `tanh` at 0.6-0.9x, `exp10` at 0.3-0.5x up to 2048 bits and 0.7x at 8192, `pow(b, a)` at
    0.9x up to 512 bits;
  - `sin`, `cos` and `tan` (#1790) at 0.9-1.1x up to 2048 bits;
  - `erf` and `erfc` from 512 bits up;
  - `atan`, `asin`, `acos` and `atan2` (#1792) at 0.4-0.8x up to 2048 bits and 1.0-1.1x at
    8192;
  - the log family (#1795) up to 2048 bits: `log`, `log2`, `log1p`, `asinh`, `acosh`, `atanh`
    and `pow(b, a)` at 0.5-0.9x, `log10` at 0.3-0.4x.
- **exp family** (#1788, #1790): 1.1-1.2x at 8192 bits, against 2.3-3.3x before #1788.
  `erf`, `erfc` gained 10-25% and `tgamma` 1-7% through their exp calls.
- **sin, cos, tan** (#1790): 1.1-1.2x at 8192 bits, against 2.1-3.3x at 128 bits and 1.5x at
  8192 before.
- **atan, asin, acos, atan2** (#1792): 2-3x faster than before, which was 0.9-1.7x up to 2048
  bits and 3.0-3.1x at 8192. Over arguments from `1e-5` to `1e6`, and `asin`, `acos` near +-1,
  they run 0.2-0.8x MPFR up to 2048 bits and 0.6-1.1x at 8192. Tiny `x` at 128 bits is the
  exception: `atan(1e-20)` rounds from `x` (11x MPFR's fast path; 14x before).
- **log family** (#1795): 1.3-3.6x faster than before, which was 1.0-1.3x up to 512 bits and
  2.2-2.5x at 2048 and 8192; now 1.1-1.2x at 8192 (`log10` 0.6x, `pow(b, a)` 1.1x). Over
  arguments from `2e-250` to `4e300`, next to 1, at both ends of the table and with `log1p`,
  `asinh`, `acosh`, `atanh` across their ranges, they run 0.1-1.0x MPFR up to 2048 bits and
  0.2-1.3x at 8192. Two exceptions at 128 bits: `log1p(1e-6)` at 2.1x (its own series, 3.1x
  before) and `log(1 + 2^-200)`, which rounds to `log 1` there, at 5.6x on both. `lgamma` and
  `tgamma` gained 5-11% below 8192 bits through their log calls.
- **tgamma, lgamma** (#1797): 2.6-29x faster, from 3.0-6.3x and 4.8-18.3x MPFR to 0.3-1.4x. The
  second pass at twice the width that every argument took was most of the time (112 of 136 ms
  at 8192 bits); the working precision is now predicted. The tapered Horner sum, the grouped
  shift product and the shorter shift (`0.15 W`) did the rest. Over `1 +- 2^-k`, `2 +- 2^-k`,
  `1.3e-6`, `1.3e6`, `1.3e100`, `-3 + 2^-30`, `-2.71`, `1000` and `30.5` they run 0.02-1.5x MPFR
  (were 0.04-18x); the one row above that, `lgamma(2 - 2^-200)` at 128 bits (2.4x, 2.3x
  before), is `lgamma 2 = 0` after rounding, under a microsecond on both.
- **cbrt, hypot, fma** (remeasured after #1783): `cbrt` 2.5-3.3x (was 6-33x), `hypot`
  2.1-2.5x, `fma` 1.7-4.1x. `fma` is slower than before at small precisions (0.12 us at 128 bits
  against 0.045): it now forms the exact product and sum, where it used to round twice and was
  wrong in 29% of the cases.
- **The primitives set the floor:** `a*b`, `b/a` and `sqrt` run 1.5-2.3x MPFR, and every
  function built on them inherits that.
- **Largest gaps:** `fma` and `pow(b, 7)` at 128 bits (4.2x, 3.6x), `cbrt` (2.6-3.3x), and the
  primitives.

**Benchmark hygiene.** Single runs of the mpmath comparison vary by 15-50%; compare branches
with best-of-5 runs on an idle machine. Warm the caches first. The first-call cost of a new
precision (filling `ln2`, `pi`, the log table, the Bernoulli numbers) is real but separate.
A change to one function can move unrelated rows by a few percent through code layout, since
`mpfr_gap` is one translation unit: #1790 moved untouched rows by up to 4% at 128 bits under
g++, and neither an isolated translation unit nor clang showed the move. #1792 moved `fma` at
512 bits by +3.5% under g++ in an isolated translation unit too, and by -2.5% under clang; its
code was byte for byte the same size, 48 bytes further along. #1795 moved `a*b` at 128 bits by
+4% in `mpfr_gap`, and by nothing in a translation unit of its own under either compiler. Check a surprising
row both ways before reading it as a cost.

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
| #1785 | 2026-10-21 | cbrt, hypot, fma correctly rounded; fmod, remainder exact |
| #1787 | 2026-10-22 | `std::numeric_limits<efloat>`; arithmetic overflows and underflows at the math library's range, scale in `[1 - 2^62, 2^62 - 1]` |
| #1789 | 2026-10-22 | exp series in fixed point by rectangular splitting: exp family 2.3-3.3x MPFR to 0.7-1.7x |
| #1791 | 2026-10-22 | sin/cos/tan versine on the fixed-point engine; tapered blocks (exp at 8192 bits 1.6x to 1.1x) |
| #1794 | 2026-10-22 | atan/asin/acos/atan2 series in fixed point, fewer halvings: 0.9-3.1x MPFR to 0.4-1.1x |
| #1796 | 2026-10-23 | log's atanh series in fixed point, AGM from 45000 bits instead of 1440: log family 1.0-2.5x MPFR to 0.3-1.2x |
| #1799 | 2026-10-23 | lgamma, tgamma: working precision predicted, tapered Stirling sum, grouped shift product: 3.0-18.3x MPFR to 0.3-1.4x |

## 9. Open work

**Library gaps.** An exact integer cube root with remainder, like `sqrtrem`, would make `cbrt`
cheaper than Newton under Ziv.

**Speed** (Epic #1763): short (high-half) products and MPFR's can-round on them; Toom-3; short
division (`divhigh`); the fixed-point series engine of exp and the versine (#1788, #1790)
carried over to expm1 (atan done in #1792, log's atanh in #1795); MPFR's bit-burst atan at 8192 bits; a cheap first Ziv try at `p + log2(p)` guard bits instead of 64; binary
splitting for exp and sin/cos;
binary-splitting constants; Sasaki-Kanada AGM start; binary splitting for lgamma's shift product;
a tapered fill of the Bernoulli table (since #1797 the first gamma call at 8192 bits fills 1439
terms in 1.1 s, against 915 in 0.4 s at the old `0.3 W` shift; the shorter shift saves 1.5 ms
a warm call, so it pays after about 460 calls in a thread);
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
