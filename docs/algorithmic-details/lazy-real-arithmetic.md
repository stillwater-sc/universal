# Lazy real arithmetic: the `elreal` algorithm

A deep dive into the algorithmic design of Universal's `elreal` type. This
document is the companion to `docs/number-systems/elreal.md` (the API
reference) and `docs/algorithmic-details/multi-component-arithmetic.md`
(the broader multi-component context).

For the original treatment of the algorithm, see Ryan McCleeary's
dissertation:

> McCleeary, R. (2019). *Lazy Exact Real Arithmetic Using Floating Point
> Operations*. PhD dissertation, University of Iowa.

## 1. The problem: undecidable equality in finite precision

Every fixed-precision real-number type -- IEEE `double`, `dd`, `qd`,
`ereal<N>` -- commits to a working precision at type-instantiation time.
That commitment is the source of three persistent pain points:

- **Geometric predicates** in mesh generation need a definite sign for any
  configuration of points, including near-degenerate ones. Fixed precision
  cannot resolve `orient2d(a, b, c)` when the three points are barely
  collinear; the determinant evaluates to a value smaller than the working
  precision's smallest representable difference.
- **Comparison in symbolic systems.** If `a` and `b` are computed by
  different algebraic paths but represent the same mathematical real, a
  fixed-precision comparison may report either equality or inequality
  depending on accumulated roundoff. A correct answer requires arbitrary
  precision *on demand*.
- **Iterative refinement.** In numerical linear algebra, the residual of a
  computed solution is needed at higher precision than the solution itself.
  Fixed-precision residuals saturate at the working precision and stop
  improving the answer.

`elreal` addresses all three by *not committing* to a precision upfront.
A real value is represented as a stream that the caller can pull from
arbitrarily deep -- or, in practice, until a per-call refinement budget is
exhausted.

## 2. The representation

Each `elreal` value carries:

- A vector of materialised components `c_0, c_1, ..., c_{N-1}`, each an
  IEEE-754 `double`, satisfying the Shewchuk **non-overlapping property**
  `|c_{i+1}| <= ulp(c_i) / 2`.
- A *generator* function that, given an index `k >= N`, produces the next
  component `c_k`. The generator embodies the operation that produced this
  `elreal` (e.g., the rational `p/q` to be expanded, or the `exp` of an
  operand).

The mathematical value of the `elreal` is the sum
`c_0 + c_1 + c_2 + ...`, taken over as many components as the caller
chooses to materialise.

```text
                  generator (closure capturing operands)
                       |
                       v
   _components: [c_0, c_1, c_2, ...]
   _computed_depth: N                   <- water mark for materialised limbs
```

The non-overlapping property guarantees that `|sum(c_i for i >= N)|` is
bounded by the magnitude of the *next* unmaterialised component, which is
in turn bounded by `ulp(c_{N-1})`. This is what makes sign determination
tractable: once a non-zero component is found, the tail cannot flip the
sign.

## 3. The refinement protocol

Two entry points:

```cpp
double at(std::size_t k) const;             // primitive
void   refine_to(std::size_t bits) const;   // wrapper
```

`at(k)` walks the generator: if `k >= computed_depth`, the generator is
called for each successive index until the depth reaches `k`, with each
produced component appended to `_components`. Once materialised, components
are cached. `at(k)` for `k < computed_depth` is an O(1) lookup.

`refine_to(bits)` is the user-facing variant: it computes the number of
~53-bit components needed for the requested precision and calls `at` for
the deepest required index, materialising the entire prefix.

Both methods are `const` and use `mutable` storage for the materialised
components -- the *logical* value of the `elreal` does not change with
refinement; only the *representation precision* deepens.

### `operator double()` is intentionally not a refinement step

`operator double()` returns the sum of *currently-materialised* components.
It does **not** invoke the generator. The user must explicitly call `at(k)`
or `refine_to` to spend a refinement step.

This split matters: in tight code (e.g., a comparison sweep over an array)
the caller wants the cheap "best estimate at current depth" path. Refinement
is a deliberate cost the caller opts into.

## 4. Sign determination (Phase D)

Comparison is the signature operation of any exact-real type. McCleeary's
dissertation section 4.2 gives the correctness theorem in the
non-overlapping-expansion setting:

> For a normalized `elreal` with components `c_0, c_1, ..., c_{N-1}`
> satisfying `|c_{i+1}| <= ulp(c_i)/2` and decreasing in magnitude, the
> sign of the true value equals the sign of the first non-zero component.

The proof is straightforward: each subsequent component is at most half a
ULP of the previous, so their *sum* is bounded by `|c_k|` for any `k`
where `c_k != 0`. The tail therefore cannot flip the sign of the prefix.

The algorithm:

```cpp
int sign(const elreal& v, std::size_t budget = elreal_default_budget) {
    if (v.isnan()) return 0;
    if (v.isinf()) return v.isneg() ? -1 : +1;
    for (std::size_t k = 0; k < budget; ++k) {
        double c = v.at(k);
        if (c > 0.0) return +1;
        if (c < 0.0) return -1;
    }
    return 0;   // exhausted budget; treat as zero within precision
}
```

Two cases drive the budget choice:

- **General position.** The first non-zero component is `c_0`; the sign
  resolves in one step. Cheap.
- **Cancellation.** A subtraction like `a - b` for `a ~= b` produces an
  elreal whose `c_0` is exactly zero (the EFT residual of `a.at(0) - b.at(0)`
  vanishes by construction). Sign determination proceeds to `c_1`, which
  carries the operand depth-1 contributions. If those also cancel, the
  algorithm continues until the first non-zero component is found or the
  budget is exhausted.

The budget defaults to 8 components (~424 bits cumulative). Callers that
need more pass a larger value: `sign(v, 32)`. The budget is per-call --
no global state.

## 5. The refinement-budget contract for `==`

Equality is the algorithmically *undecidable* case. Given two `elreal`
values that are mathematically equal (by some path that built them
differently), the comparison must walk arbitrarily deep before declaring
agreement -- and in finite precision, "arbitrarily deep" must be a number.

`elreal`'s contract:

- `a == b` iff `compare(a, b, default_budget) == 0`. That is,
  *indistinguishable within the budget*.
- The user can tighten the budget per call.
- Mathematically-distinct values whose first divergent component lies *past*
  the budget will be reported as equal. That is a known limitation, not a
  bug, and is the inevitable consequence of requiring finite-cost
  comparison.

This matches the standard contract for IEEE-754 floating-point: equality
is approximate. `elreal` simply makes the precision *explicit* and
*per-call* rather than implicit and global.

## 6. Refinement depth in arithmetic operators (Phase C)

Each ring operation `(+, -, *, /)` produces a new `elreal` whose generator
captures the operand streams by value. The depth-1 corrections use the
existing EFT primitives in `include/sw/universal/numerics/error_free_ops.hpp`:

| Op | depth-0 | depth-1 generator |
|---|---|---|
| `a + b` | `two_sum(a0, b0)` leading | `sum_err + a.at(1) + b.at(1)` |
| `a - b` | `two_diff(a0, b0)` leading | `diff_err + a.at(1) - b.at(1)` |
| `a * b` | `two_prod(a0, b0)` leading | `prod_err + a0*b.at(1) + a.at(1)*b0` |
| `a / b` | `a0 / b0` | `(ieee_residual + a.at(1) - c0 * b.at(1)) / b0` |

The `a / b` depth-1 generator (Phase L.1, #906) reconstructs the IEEE
division residual `a - b * c0` exactly via `two_prod` + `two_diff`, then
combines it with the Taylor partials.

**Phase L.2.a** adds depth-2 for division: the `gen_newton_div`
variant produces

```text
c_2 = (a.at(2) - b.at(2) * c_0 - b.at(1) * c_1) / b0
```

picking up the operand depth-2 contributions and preserving the
non-overlapping property `|c_2| <= ulp(c_1) / 2`. For pure-double
inputs (`a.at(2) = b.at(1) = b.at(2) = 0`) this evaluates to 0 -- the
lazy-real interpretation: we don't invent precision the operands didn't
carry. For multi-component inputs (e.g. `elreal_pi() / elreal_e()`),
c_2 captures meaningful additional bits.

Depth-3+ for `/` and depth-2+ for the other operators (and for sqrt)
require either Newton iteration with multi-component multiplications
inside generators, or depth-2+ support across `+`/`-`/`*` so the
operand streams propagate further than depth 1. Both are Phase L.2.b
and follow-up scope.

The non-overlapping property is preserved by construction -- the EFT
residual is bounded by `ulp(leading) / 2`, and the operand depth-1 terms
are themselves bounded by `ulp(operand_0)`.

Depth 2+ for arithmetic is deferred to Phase L.2 (#906) of the follow-up
epic. The issue is that the operator generators capture only the operand
depth-0/1 components and don't propagate to depth 2 without lazy division
walking deeper. Newton iteration provides this for `/` and `sqrt`;
lazy-pi pull would provide it for `sin/cos/tan` (Phase N #908).

## 7. Math functions: derivative-based depth-1 correction (Phase E)

Math functions follow a uniform pattern:

```text
result.at(0) = std::f(a.at(0))        // depth-0 = std lib correctly rounded
result.at(1) = f'(a.at(0)) * a.at(1)  // depth-1 = derivative * operand correction
result.at(k>=2) = 0                   // deferred
```

The first-order Taylor expansion: `f(x_0 + x_1 + ...) ~= f(x_0) + f'(x_0) * (x_1 + ...)`,
truncated to depth 1.

This works for any function whose derivative is closed-form:

| Family | Function | Derivative |
|---|---|---|
| Roots | `sqrt(x)` | `1 / (2 sqrt(x))` -- EFT residual trick avoids lazy `/` |
| Exp | `exp(x)` | `exp(x)` |
| Exp | `exp2(x)` | `exp2(x) * ln(2)` |
| Exp | `expm1(x)` | `exp(x)` = `expm1(x) + 1` |
| Log | `log(x)` | `1 / x` |
| Log | `log2(x)` | `1 / (x * ln(2))` |
| Log | `log10(x)` | `1 / (x * ln(10))` |
| Log | `log1p(x)` | `1 / (1 + x)` |
| Pow | `pow(a, b)` | partials `b * a^(b-1)` and `a^b * log(a)` |
| Hyperbolic | `sinh / cosh / tanh` | `cosh / sinh / sech^2` |
| Inv hyperbolic | `asinh / acosh / atanh` | `1/sqrt(1+x^2)`, `1/sqrt(x^2-1)`, `1/(1-x^2)` |
| Inv trig | `asin / acos / atan / atan2` | `±1/sqrt(1-x^2)`, `1/(1+x^2)`, ... |
| Trig | `sin / cos / tan` | `cos / -sin / sec^2` |

For inputs that are themselves rationals (e.g., `exp(elreal(1, 3))`), the
operand carries a non-zero depth-1 component and the derivative correction
delivers meaningful refinement past the std library's depth-0 result. For
inputs constructed from a single `double`, the operand depth-1 is zero and
the derivative correction collapses -- the result is just `std::f(double)`.

This is why `sin(elreal_pi())` is observably closer to true zero than
`std::sin(M_PI)`: the multi-component `elreal_pi` has a non-zero depth-1
component carrying the bits past M_PI's 53-bit representation, and the
derivative `cos(M_PI) * pi.at(1)` cancels the std::sin rounding error.

## 8. The forward-trig limitation (Payne-Hanek)

`sin / cos / tan` for large-magnitude arguments require *exact range
reduction* modulo `2*pi`. The standard library reduces using
IEEE-754 `M_PI` (53 bits); for `|x|` near `2^53`, the reduced argument has
no precision left.

McCleeary 2019 / Payne-Hanek 1983 solve this by performing the reduction
with arbitrarily many bits of pi pulled from `elreal_pi`'s stream:

```text
1. Compute k = round(x / (pi/2))                 // integer multiple
2. Reduce r = x - k * (pi/2) using exact lazy arithmetic
3. Dispatch on k mod 4 -> sin(r) or cos(r) on the reduced argument
4. Evaluate sin(r) / cos(r) via Taylor on the reduced argument
```

The Phase E.6 implementation does **not** ship this path -- it uses
`std::sin/cos/tan` for depth 0 and a derivative correction for depth 1.
Faithful results require `|x|` within "reasonable magnitude" (roughly
`|x| < 2^25`); past that, the result is faithful in the IEEE-754 sense (it
matches `std::sin`) but loses precision as `|x|` grows.

Full Payne-Hanek with lazy pi-pull is a follow-up to Phase E.6 of the
elreal epic (#873). When it lands:

- `elreal_pi` extends past the current 4-component static expansion (BBP
  bit-extraction or a generator-based variant of `from_expansion`).
- An integer-mod-pi reduction loop pulls more bits of pi until
  `|r| < ulp(x)`.
- Taylor or Chebyshev evaluation of sin/cos on the reduced argument provides
  the lazy refinement.

## 9. Geometric predicates as the killer app

The orient2d / orient3d / incircle / insphere predicates are determinants
of small matrices, evaluated with the operand coordinates. Each predicate
reduces to a sign query, which goes through Phase D's `sign(...)`.

```cpp
elreal orient2d(Point2D<elreal> a, Point2D<elreal> b, Point2D<elreal> c) {
    elreal acx = a.x - c.x;
    elreal acy = a.y - c.y;
    elreal bcx = b.x - c.x;
    elreal bcy = b.y - c.y;
    return acx * bcy - acy * bcx;
}
```

The caller asks `sign(orient2d(a, b, c), budget)` to get +1 / 0 / -1.

For **general-position inputs**, the determinant's depth-0 is decisive and
the predicate resolves in O(1) double-precision work. For
**near-degenerate inputs**, the depth-0 is small (relative to its expected
magnitude given non-degenerate inputs) and the sign-determination walk
proceeds to depth 1, 2, ... until either a non-zero component is found or
the budget is exhausted. **Exactly-degenerate** inputs (collinear,
coplanar, cocircular, cospherical) with integer-valued coordinates produce
a stream where every materialised component is exactly zero; the predicate
correctly returns 0.

This is the canonical Shewchuk vs McCleeary distinction. Shewchuk's
expansion arithmetic eagerly builds the full expansion needed to resolve
the sign exactly; McCleeary's lazy refinement does only as much work as
the current input demands.

`docs/algorithmic-details/multi-component-arithmetic.md` section 9 has the
full ereal-vs-elreal comparison.

## 10. The validation-oracle role (Phase G)

`elreal` doubles as a *cross-implementation oracle* for validating other
multi-component types: `dd`, `qd`, `dd_cascade`, `td_cascade`, `qd_cascade`,
`ereal<N>`.

The helper at
`include/sw/universal/verification/test_suite_elreal_oracle.hpp` takes a
target-type value and an `elreal` reference computed via an entirely
independent code path, and asserts agreement within the target's
`numeric_limits::digits10` precision. Cross-implementation agreement is a
non-trivial bug signal even at matched precision -- before this pipeline,
the only references available for multi-component math-function tests
were `std::cmath`, which is a self-check (the multi-component
implementations themselves typically use std lib as the depth-0 building
block).

Today's ceiling is double precision (capped by elreal's depth-1 in
arithmetic and math). When deeper refinement lands, the oracle's
precision improves transparently -- the helper's API surface does not
change.

See `multi-component-arithmetic.md` section 8.6 for the broader
validation-strategy context.

## 11. Implementation in Universal: a phase-by-phase summary

The `elreal` type was built up across nine PRs (epic #873):

| Phase | Issue | Delivered |
|---|---|---|
| A | #874 | type skeleton, `at(k)` / `refine_to` protocol, exceptions, traits |
| B | #875 | construction from `double`, integers, rationals, decimal/scientific strings |
| C | #876 | arithmetic operators with depth-1 EFT refinement |
| D | #877 | sign / compare / ordering with per-call refinement budget |
| E.1-E.6 | #887-#892 | math constants, sqrt/hypot, exp/log/pow, hyperbolic, inverse trig, forward trig |
| F | #879 | geometric predicates (orient2d/3d, incircle, insphere) |
| G | #880 | validation-oracle helper for other multi-component types |

Each phase added a focused capability layer on top of the previous. The
foundation (Phase A) -- the storage representation and the refinement
protocol -- is what makes every subsequent phase work without ad-hoc
restructuring.

## References

- McCleeary, R. (2019). *Lazy Exact Real Arithmetic Using Floating Point
  Operations*. PhD dissertation, University of Iowa.
  <https://iro.uiowa.edu/esploro/outputs/doctoral/Lazy-exact-real-arithmetic-using-floating/9983776998202771>
- Shewchuk, J. R. (1997). "Adaptive Precision Floating-Point Arithmetic and
  Fast Robust Geometric Predicates." *Discrete & Computational Geometry*
  18(3), 305-363.
- Payne, M. and Hanek, R. (1983). "Radian Reduction for Trigonometric
  Functions." *SIGNUM Newsletter*, 18(1), 19-24.
- Priest, D. M. (1991). *On Properties of Floating Point Arithmetics:
  Numerical Stability and the Cost of Accurate Computations*. PhD
  dissertation, UC Berkeley.

## See also

- `docs/number-systems/elreal.md` -- user-facing API reference
- `docs/algorithmic-details/multi-component-arithmetic.md` -- the broader
  Priest / Bailey-Hida / Shewchuk / McCleeary landscape and validation
  strategies
- `include/sw/universal/number/elreal/elreal_impl.hpp` -- the implementation,
  with substantial inline documentation
