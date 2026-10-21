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
- Tracking: Epic #1763 (beat MPFR at 128-8192 bits), #1782 (this page), #1783 (the functions
  not yet correctly rounded)

## 1. The contract

Every function below, except the five in section 9, is designed to be **correctly rounded**:
the result is the exact mathematical value rounded once to the result's precision, in whichever
of the four IEEE rounding modes `efloat_rounding_mode` selects (nearest-even, toward zero,
toward +inf, toward -inf). What backs that is not a proof. The arithmetic, `sqrt` and the
conversions round exact results, so they are correct by construction. The other functions
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
| `exp` | `x = k ln2 + r`, `r` halved up to `sqrt(P)` times, Taylor series at a precision that falls with the terms, squared back; `abs(k) < 2^62` | 16 | exponent.hpp |
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

"Error bits" is the function's `*_error_bits`: its unrounded value lies within
`2^(scale - target - 64 + error_bits)` of the exact result.

### 4.1 exp family

- **Reduction.** `k = round(x / ln2)` from a double estimate, corrected once from `r` when `x`
  passes `2^52`; `ln2` at the working precision plus 64 bits, so `k ln2` keeps its accuracy for
  any `|k| < 2^62` (#1777). Before #1777 the range stopped at `|x| < 2^17`. Per-`k` widths of
  `ln2` were rejected: they re-grew the cache and cost 9-12% at 8192 bits.
- **Series.** `r` is halved until it is below `2^-h`, `h <= sqrt(P)` (adaptive since #1774),
  the Taylor terms are carried at a precision that falls with their size (#1754), with exact
  small-integer divisors (#1758), and the sum is squared back `h` times.
- **History.** Before #1739: `ln2` was a 59-digit literal (results stopped at about 2^-200) and
  the series stopped at 100 terms (about 680 bits). At 608 bits `exp` took 957 us against
  mpmath's 11 (#1714); #1741 brought that to 20 us and #1754 to 15.6 us.
- **Open.** Rectangular or binary splitting (MPFR's `exp_2`/`exp_3`), fixed-point series
  kernels with no normalize-and-round per term.

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

### 4.8 Constants

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

`mpfr_check`, full run (scale 1, up to 8192 bits), main at 2076edb7, MPFR 4.2.2:

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
| `cbrt` | 3234 | 889 | not yet correctly rounded (#1783) |
| `hypot` | 3234 | 1656 | not yet correctly rounded (#1783) |
| `fma` | 3234 | 926 | not yet correctly rounded (#1783) |
| `fmod` | 3234 | 1546 | not yet correctly rounded (#1783) |
| `remainder` | 3234 | 2063 | not yet correctly rounded (#1783) |
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
Measured 2026-10-21 on main at 2076edb7, Intel Core i7-12700K, g++ 15.2 `-O3 -march=native`,
MPFR 4.2.2, GMP 6.3.0; `a = 0.718...`, `b = 1.414...`, `x = 1 + a`. Each cell is
efloat / MPFR in microseconds, then the ratio.

| operation | 128 bits | | 512 bits | | 2048 bits | | 8192 bits | |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| `a+b` | 0.028 / 0.013 | 2.2x | 0.035 / 0.020 | 1.7x | 0.075 / 0.048 | 1.6x | 0.236 / 0.167 | 1.4x |
| `a*b` | 0.027 / 0.018 | 1.5x | 0.064 / 0.043 | 1.5x | 0.503 / 0.266 | 1.9x | 4.741 / 2.157 | 2.2x |
| `b/a` | 0.051 / 0.029 | 1.8x | 0.134 / 0.081 | 1.7x | 0.900 / 0.482 | 1.9x | 8.340 / 4.299 | 1.9x |
| `a/7` | 0.042 / 0.024 | 1.7x | 0.077 / 0.040 | 1.9x | 0.223 / 0.104 | 2.1x | 0.811 / 0.371 | 2.2x |
| `exp(a)` | 1.916 / 0.719 | 2.7x | 5.061 / 2.199 | 2.3x | 38.51 / 15.75 | 2.4x | 567.3 / 175.1 | 3.2x |
| `log(x)` | 1.226 / 1.122 | 1.1x | 3.809 / 3.250 | 1.2x | 34.84 / 14.79 | 2.4x | 280.2 / 123.8 | 2.3x |
| `sin(a)` | 2.011 / 0.735 | 2.7x | 4.689 / 2.126 | 2.2x | 28.25 / 14.18 | 2.0x | 297.3 / 193.5 | 1.5x |
| `sqrt(b)` | 0.128 / 0.059 | 2.2x | 0.265 / 0.135 | 2.0x | 0.983 / 0.435 | 2.3x | 6.363 / 3.081 | 2.1x |
| `fma(a,b,a)` | 0.045 / 0.029 | 1.5x | 0.087 / 0.050 | 1.8x | 0.551 / 0.327 | 1.7x | 4.902 / 3.146 | 1.6x |
| `hypot(a,b)` | 0.388 / 0.170 | 2.3x | 0.719 / 0.327 | 2.2x | 3.081 / 1.012 | 3.0x | 24.09 / 8.032 | 3.0x |
| `cbrt(b)` | 1.720 / 0.287 | 6.0x | 4.012 / 0.468 | 8.6x | 25.03 / 1.303 | 19.2x | 248.6 / 7.500 | 33.1x |
| `exp2(a)` | 2.124 / 1.024 | 2.1x | 5.495 / 2.696 | 2.0x | 39.80 / 17.20 | 2.3x | 575.7 / 190.9 | 3.0x |
| `exp10(a)` | 2.230 / 2.313 | 1.0x | 5.738 / 6.076 | 0.9x | 41.66 / 31.30 | 1.3x | 593.9 / 313.4 | 1.9x |
| `expm1(a)` | 1.935 / 0.815 | 2.4x | 5.311 / 2.398 | 2.2x | 38.51 / 15.58 | 2.5x | 569.0 / 178.3 | 3.2x |
| `log2(x)` | 1.342 / 1.270 | 1.1x | 4.122 / 3.513 | 1.2x | 35.85 / 15.33 | 2.3x | 289.4 / 130.8 | 2.2x |
| `log10(x)` | 1.411 / 2.441 | 0.6x | 4.183 / 6.962 | 0.6x | 35.96 / 30.34 | 1.2x | 290.5 / 254.3 | 1.1x |
| `log1p(a)` | 1.366 / 1.208 | 1.1x | 4.068 / 3.343 | 1.2x | 35.06 / 14.86 | 2.4x | 279.6 / 124.2 | 2.3x |
| `cos(a)` | 1.803 / 0.527 | 3.4x | 4.329 / 1.726 | 2.5x | 26.51 / 13.00 | 2.0x | 286.0 / 188.4 | 1.5x |
| `tan(a)` | 2.175 / 0.994 | 2.2x | 5.032 / 2.368 | 2.1x | 29.47 / 15.31 | 1.9x | 306.0 / 201.6 | 1.5x |
| `asin(a)` | 4.325 / 2.845 | 1.5x | 11.52 / 11.06 | 1.0x | 75.81 / 43.59 | 1.7x | 1049 / 337.6 | 3.1x |
| `acos(a)` | 3.757 / 2.850 | 1.3x | 10.69 / 11.10 | 1.0x | 71.66 / 43.26 | 1.7x | 1010 / 338.5 | 3.0x |
| `atan(a)` | 3.614 / 2.358 | 1.5x | 10.31 / 8.988 | 1.1x | 71.56 / 41.69 | 1.7x | 1009 / 331.1 | 3.0x |
| `atan2(a,b)` | 3.437 / 2.360 | 1.5x | 9.961 / 11.04 | 0.9x | 69.84 / 44.70 | 1.6x | 1004 / 324.1 | 3.1x |
| `sinh(a)` | 2.140 / 0.856 | 2.5x | 5.511 / 2.485 | 2.2x | 39.73 / 16.22 | 2.4x | 579.1 / 184.2 | 3.1x |
| `cosh(a)` | 2.117 / 0.851 | 2.5x | 5.457 / 2.486 | 2.2x | 39.68 / 16.08 | 2.5x | 579.1 / 181.3 | 3.2x |
| `tanh(a)` | 2.132 / 0.942 | 2.3x | 5.529 / 2.563 | 2.2x | 40.17 / 16.54 | 2.4x | 597.6 / 185.0 | 3.2x |
| `asinh(a)` | 1.759 / 1.354 | 1.3x | 4.646 / 3.696 | 1.3x | 38.51 / 15.52 | 2.5x | 313.4 / 128.8 | 2.4x |
| `acosh(b)` | 1.627 / 1.381 | 1.2x | 4.301 / 3.732 | 1.2x | 37.44 / 15.52 | 2.4x | 299.8 / 128.8 | 2.3x |
| `atanh(a)` | 1.585 / 1.379 | 1.1x | 4.676 / 3.568 | 1.3x | 36.14 / 15.38 | 2.4x | 291.1 / 128.7 | 2.3x |
| `pow(b,a)` | 3.493 / 2.211 | 1.6x | 9.245 / 6.055 | 1.5x | 76.20 / 31.66 | 2.4x | 882.6 / 313.8 | 2.8x |
| `pow(b,7)` | 0.424 / 0.105 | 4.0x | 0.614 / 0.196 | 3.1x | 2.430 / 1.259 | 1.9x | 18.29 / 11.00 | 1.7x |
| `tgamma(b)` | 31.08 / 7.188 | 4.3x | 118.0 / 33.34 | 3.5x | 1817 / 363.0 | 5.0x | 47401 / 7360 | 6.4x |
| `lgamma(b)` | 40.08 / 7.474 | 5.4x | 224.4 / 34.09 | 6.6x | 4499 / 757.7 | 5.9x | 133510 / 7238 | 18.4x |
| `erf(a)` | 6.382 / 2.922 | 2.2x | 20.32 / 16.08 | 1.3x | 180.7 / 171.3 | 1.1x | 3127 / 3030 | 1.0x |
| `erfc(a)` | 6.562 / 3.234 | 2.0x | 20.93 / 16.55 | 1.3x | 181.9 / 172.7 | 1.1x | 3138 / 3018 | 1.0x |

Read the ratio column as the work left on Epic #1763.

- **Ahead or level with MPFR:** `log10` (0.6x at 128 and 512 bits), `exp10` up to 512 bits,
  `erf` and `erfc` at 2048 and 8192 bits, the inverse trigonometric functions at 512 bits.
- **The primitives set the floor:** `a*b`, `b/a` and `sqrt` run 1.5-2.3x MPFR, and every
  function built on them inherits that. exp-based functions are 2-3x, trig 1.5-3x, log-based
  1.1-2.4x.
- **Largest gaps:** `cbrt` (6-33x: Newton in efloat operations, #1783), `lgamma(1.414...)` at
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

## 9. Open work

**Not correctly rounded yet** (#1783; `mpfr_check` lists them as `known`):

| function | now | fix |
|---|---|---|
| `cbrt` | Newton with 64 guard bits, rounded unchecked | exact integer cube root with remainder, as `sqrt` |
| `hypot` | `sqrt` of a rounded `x^2 + y^2` | exact squares and sum, one correctly rounded `sqrt` |
| `fma` | `x * y + z`, product rounded first | exact product, one rounding of the sum |
| `fmod`, `remainder` | `x - trunc(x/y) y` with a rounded quotient | exact, by long division of the significands |

**Speed** (Epic #1763): short (high-half) products and MPFR's can-round on them; Toom-3; short
division (`divhigh`); fixed-point series kernels; a cheap first Ziv try at
`p + log2(p)` guard bits instead of 64; rectangular or binary splitting for exp and sin/cos;
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
