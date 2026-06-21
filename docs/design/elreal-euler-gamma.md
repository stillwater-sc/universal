# elreal: computing Euler's gamma (Brent-McMillan B1 and the Abel reduction)

## TL;DR

Every other named constant in the `elreal` math suite (pi, e, ln2, ln10, phi,
the sqrt radicals) has a rapidly-converging *elementary* series -- a Machin atan
sum, an artanh sum, a Taylor exp, a Newton sqrt -- so each is just a streaming
`infsum` over a significance-windowed term co-list (see
`elreal-online-convergence.md`).

The Euler-Mascheroni constant `gamma = 0.5772156649...` has **no** such series.
It needs a real algorithm. `euler_gamma_zbcl` uses **Brent-McMillan B1**, whose
cost is dominated by an `A(n) = sum_k w_k H_k` accumulation. This note records
that algorithm and the **Abel reduction** (#1061 Phase 3b, PR #1088) that removed
the harmonic numbers and the per-term full multiply from that accumulation,
making it ~2.3-3.1x faster while staying value-identical to 305 decimal digits.
It also records why a *true* asymptotic speedup (binary splitting) was deferred.

## Why gamma is special

`gamma = lim_{m->inf} (H_m - ln m)`, where `H_m = sum_{j=1}^m 1/j`. That defining
limit converges like `1/m` -- useless for high precision. There is no known
elementary fast series for gamma, unlike pi or e. The standard high-precision
route is Brent-McMillan (1980), derived from the modified Bessel functions
`I_0`, `K_0`.

## Brent-McMillan B1

For an integer parameter `n`,

```
gamma = A(n) / B(n) - ln(n) + O(pi * exp(-4n))
```

with

```
w_k  = (n^k / k!)^2                      (the common weight)
B(n) = sum_{k>=0} w_k                     (~ I_0(2n)-like)
A(n) = sum_{k>=0} w_k * H_k,   H_0 = 0,   H_k = H_{k-1} + 1/k
```

The truncation error is `~ pi * exp(-4n)`, so for `D` correct decimal digits pick
`n >= D * ln(10) / 4`. In `euler_gamma_zbcl`, `n` is derived from the requested
working depth (`n ~ targetDigits * ln(10)/4 + slack`).

### Host-range constraint

The intermediate `w_k = (n^k/k!)^2` peaks near `k = n` at magnitude `~ exp(2n)`.
`n` grows with the requested precision, so the block significand has to carry that
magnitude. A NARROW-exponent host (float, bfloat16; max `~10^38`) overflows once
`n` exceeds `~44`. **High-precision gamma is therefore a double-host generator;**
on narrow hosts keep the depth shallow (`n` below the overflow). This is the same
constraint noted for the eager generator (#1053).

### The peak, analytically

The windowing (below) needs the peak exponent of `w_k`. Rather than run a whole
recurrence just to find it (a redundant pass, removed in #1087), it is known by
Stirling: at `k = n`, `n^n/n! ~ e^n / sqrt(2*pi*n)`, so

```
log2(w_n) ~ 2n*log2(e) - log2(2*pi*n)
```

accurate to ~1 bit -- far inside the windowing slack (see below).

## Computing A and B in the ZBCL framework

`w_k` follows a cheap scalar recurrence: `w_k = w_{k-1} * n^2 / k^2`. Each step is
a single-block scalar multiply (`* n^2`) and a single-block scalar divide
(`/ k^2`) on the ZBCL.

`B(n)` is a plain streaming sum of the `w_k`, accumulated with `priestRenorm` and
capped to the working depth (`cap = wdepth + 2` blocks): terms more than `cap`
limbs below the running peak are negligible for the `O(1)` ratio `A/B`.

**Windowing.** Only the `~sqrt(n)` terms near the peak contribute to the leading
blocks; the tiny early terms and the decaying tail are skipped by pulling each
contribution only down to a significance floor

```
aFloor = peakExp - cap * k    (k = block bit-width)
```

(`take_while_above`). This keeps the per-term work bounded.

### The A-sum was the cost

The naive `A(n) = sum_k w_k H_k` requires, per term:

- maintaining the harmonic number `H_k` as a multi-block ZBCL (`H_k = H_{k-1} + 1/k`), and
- a **full ZBCL x ZBCL multiply** `mul_online(w_k, H_k)`.

That full multiply, over `~n` terms each at working depth, is the dominant cost of
the whole constant (e.g. ~11 s at depth 16 before this change).

## The Abel reduction

Abel summation (summation by parts) rewrites the weighted sum exactly. With
`B_k = sum_{j<=k} w_j` the partial sums and `tail_k = sum_{j>k} w_j = B - B_k`:

```
A = sum_{k>=0} w_k H_k = sum_{k>=0} tail_k / (k+1)
```

### Derivation

`H_k = sum_{j=1}^{k} 1/j`, so by swapping the order of summation

```
A = sum_{k} w_k sum_{j=1}^{k} 1/j
  = sum_{j>=1} (1/j) sum_{k>=j} w_k
  = sum_{j>=1} (1/j) * tail_{j-1}
  = sum_{m>=0} tail_m / (m+1)          (m = j-1)
```

Spot check (`K = 2`): `A = w_1 H_1 + w_2 H_2 = w_1 + 1.5 w_2`. The RHS is
`tail_0/1 + tail_1/2 + tail_2/3 = (w_1+w_2) + w_2/2 + 0 = w_1 + 1.5 w_2`. Match.

### Why it is faster

The harmonic numbers vanish entirely, and the per-term **full multiply** becomes a
per-term **single-block scalar division** `tail_k / (k+1)` -- the fast
`twoDivZBCL` path. Concretely:

- **Pass A** runs the `w_k` recurrence and sums `B = sum w_k`.
- **Pass B** walks the *same* recurrence keeping a running
  `tail = B - B_k` (forward subtraction `tail_k = tail_{k-1} - w_k`) and folds the
  scalar quotient `tail_k/(k+1)` into `A`.

A full multiply (`O(M(wdepth))`) per term is replaced by an add and a scalar
divide (`O(wdepth)`) per term.

### The one numerical subtlety

`A = H_K*B - sum_k B_k/(k+1)` is the *naive* Abel form and is unusable: both
terms grow like `B * ln K` and catastrophically cancel. The **convergent** form
above (`sum tail_k/(k+1)`, all terms positive) avoids that.

It has one residual cancellation: forming `tail_k = tail_{k-1} - w_k` to the right
of the peak, where `w_k` dominates the remaining tail, loses bits. But those
`tail_k` are themselves small contributors there, so the absolute error is bounded
by roughly one ULP of the peak -- comfortably inside the `cap`-block guard
(`cap*k - wbits` spare bits). This is not assumed; it is **verified** against the
oracle.

## Validation

The exact-dyadic oracle (`verification/elreal_reference_digits.hpp`) compares the
generated constant against a 320-digit mpmath reference. With the Abel reduction,
`euler_gamma_zbcl<double>(20)` agrees to **305 digits** (threshold `>= 300`) on
both gcc and clang -- identical to the prior implementation.

> Gotcha: that 320-digit check lives in
> `elastic/elreal/math/constants_highprecision.cpp` behind `#if REGRESSION_LEVEL_4`.
> A default (`LEVEL_1`) build compiles it out and the test passes as a no-op. To
> actually exercise it, build with `-DREGRESSION_LEVEL_4=1` (or the
> `UNIVERSAL_BUILD_REGRESSION_LEVEL_4` CMake option).

## What is NOT done: binary splitting

The Abel reduction is a constant-factor win; Brent-McMillan is still inherently
`O(n)` term-products, and `n ~ D`, so the cost is still `~O(D * M(D))`.

The asymptotically-best approach is **binary splitting**: the `B` and `A` sums are
holonomic, so a balanced `P/Q/B/T` recursion over the index range -- accumulating
exact big-integer numerators/denominators and dividing once at the end -- computes
gamma in `~O(M(D) log^2 D)`. That was deliberately deferred: it requires pulling
arbitrary-precision integers (`einteger`) into the elreal math layer (which has no
such dependency today), a new `einteger -> ZBCL` block-decomposition bridge, and a
harmonic-aware splitting tuple, with no existing binary-splitting precedent in the
repository to model on. It is tracked as a separate, larger effort.

## References

- R. P. Brent and E. M. McMillan, "Some new algorithms for high-precision
  computation of Euler's constant", Math. Comp. 34 (1980), 305-312.
- Abel summation (summation by parts), standard.
- `elreal-online-convergence.md` -- the streaming `infsum` series framework the
  other constants use.
- `include/sw/universal/number/elreal/math/constants.hpp` -- `euler_gamma_zbcl`.
