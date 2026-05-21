# elreal: Exact Lazy Real Arithmetic

## Why

Some computations cannot tolerate any precision loss. Geometric predicates in
mesh generation need a definite sign for any input -- including configurations
that are arbitrarily close to degenerate. Comparisons in symbolic computation
need to terminate with a correct answer for any pair of distinct reals.
Iterative refinement loops in numerical PDE solvers need a residual that does
not accumulate roundoff. Fixed-precision arithmetic (`double`, `dd`, `qd`,
even `ereal<19>`) gives up these guarantees when the working precision is set
upfront.

`elreal` implements the exact-real-arithmetic paradigm from Ryan McCleeary's
2019 dissertation: a real value is represented as a **lazy stream of
progressively refined floating-point components**. Operations return promises;
precision is pulled on demand only when the caller asks for it. Comparison
walks the stream until the sign is determined -- the canonical hard problem
of exact-real arithmetic, made tractable by a bounded refinement budget.

## What

`elreal` is the third (and most precise) multi-component arithmetic tier in
Universal, joining the fixed-precision Bailey/Hida types (`dd`, `qd`) and the
adaptive Shewchuk-style `ereal<N>`:

| Property | Value |
|----------|-------|
| Class form | `class elreal` (non-templated) |
| Storage | `std::vector<double>` + `std::function<double(std::size_t)>` generator |
| Internal element | IEEE-754 `double` per stream component |
| Refinement on demand | Yes, via `at(k)` and `refine_to(precision_bits)` |
| Comparison | Budgeted sign-determination (`sign`, `compare`) |
| Special values | `+/-inf`, `NaN`, signed zero -- all preserved through arithmetic |
| Mathematical functions | `sqrt`, `hypot`, `exp` family, `log` family, `pow`, hyperbolic, inverse trig, forward trig |
| Geometric predicates | `orient2d`, `orient3d`, `incircle`, `insphere` |
| Cross-validation oracle | `check_against_elreal_oracle` for other multi-component types |

### Key properties

- **Lazy refinement.** Each operation returns an `elreal` whose `at(k)`
  component is computed only when called. A typical comparison resolves at
  depth 0; near-degenerate inputs may consume the refinement budget.
- **Non-templated.** A single class -- no `maxlimbs` parameter to set
  upfront. Refinement depth is per-call.
- **Bounded budget.** Sign determination has a per-call budget
  (`elreal_default_budget = 8`) that caps worst-case work on the
  algorithmically-undecidable equality case.
- **No external dependencies.** Pure header-only C++20, like the rest of
  Universal.

### Internal representation

```cpp
class elreal {
private:
    mutable std::vector<double>                _components;       // materialised stream
    mutable std::size_t                        _computed_depth;   // high-water mark
    mutable std::function<double(std::size_t)> _generator;        // produces depth k on demand

    // ... constructors, operators, math functions
};
```

The mathematical value is the sum of all stream components -- materialised
plus any the generator would produce. `at(k)` returns the k-th component,
materialising it via the generator if necessary. `operator double()` returns
the sum of *currently-materialised* components (it does not call the
generator -- callers spend a refinement step explicitly via `at(k)` or
`refine_to`).

## How to use

### Include

```cpp
#include <universal/number/elreal/elreal.hpp>
using namespace sw::universal;
```

### Construction

```cpp
elreal a(3.14);                        // from double
elreal b(42);                          // from int (and other native integer types)
elreal one_third(1LL, 3LL);            // from rational p/q (the canonical exact case)
elreal pi_string("3.14159265358979");  // from decimal / scientific / rational string
elreal r(SpecificValue::infpos);       // from SpecificValue enum
elreal raw = elreal::from_expansion({3.14, 1.0e-17});  // from a pre-computed expansion
```

The rational constructor is the distinguishing feature: `elreal(1LL, 3LL)`
represents 1/3 *exactly*, not the IEEE-rounded double approximation. Phase D's
comparison resolves `elreal(1LL, 3LL) > elreal(1.0/3.0)` as true, because the
stream's depth-1 correction reveals the residual bit pattern that the rounded
double dropped.

### Arithmetic

```cpp
elreal a("1/3"), b("1/2");
elreal sum = a + b;
elreal diff = a - b;
elreal prod = a * b;
elreal quot = a / b;
elreal neg  = -a;
elreal r    = abs(a);
elreal s    = sqrt(a);
elreal h    = hypot(a, b);
```

All operators produce a new `elreal` whose generator captures the operand
streams. Compound assignment (`+=`, `-=`, `*=`, `/=`) is also available.

Division by zero throws `elreal_divide_by_zero` when
`ELREAL_THROW_ARITHMETIC_EXCEPTION` is defined; otherwise IEEE-754 `+/-inf`
or `NaN` propagates.

### Comparison with refinement budget

```cpp
elreal a(1LL, 3LL);
elreal b(1.0 / 3.0);

if (a > b)      { /* true: the rational a has more bits than the double b */ }
if (a == b)     { /* false: same reason */ }

int s = sign(a - b);          // returns +1
int c = compare(a, b);        // returns +1; equivalent to sign(a - b)
int s2 = sign(a - b, 16);     // wider refinement budget (default is 8)
```

Comparison is *budgeted*. The default budget of 8 components is enough for
any practical signed-comparison query. For algorithmically-undecidable cases
(asking whether two distinct mathematical reals happen to be equal),
the comparison returns 0 (treats as equal) once the budget is exhausted.

### Math functions

All shipped at depth-0 (matching `std::cmath`) with depth-1 derivative-based
refinement:

```cpp
elreal y;

y = exp(a);    y = exp2(a);   y = expm1(a);
y = log(a);    y = log2(a);   y = log10(a);   y = log1p(a);
y = pow(a, b);

y = sinh(a);   y = cosh(a);   y = tanh(a);
y = asinh(a);  y = acosh(a);  y = atanh(a);

y = sin(a);    y = cos(a);    y = tan(a);
y = asin(a);   y = acos(a);   y = atan(a);
y = atan2(a, b);
```

Math constants are available as factory functions returning multi-component
expansions:

```cpp
elreal pi   = elreal_pi();      // ~212-bit expansion of pi
elreal e    = elreal_e();       // Euler's number
elreal ln2  = elreal_ln2();
elreal ln10 = elreal_ln10();
elreal phi  = elreal_phi();     // golden ratio
elreal sqrt2 = elreal_sqrt2();
```

### Geometric predicates

The canonical showcase for exact-real arithmetic:

```cpp
#include <universal/number/elreal/geometry/predicates.hpp>

Point2D<elreal> a(elreal(0.0), elreal(0.0));
Point2D<elreal> b(elreal(1.0), elreal(0.0));
Point2D<elreal> c(elreal(0.5), elreal(0.5));

elreal r = orient2d(a, b, c);
int s = sign(r);  // +1 for left turn, -1 right, 0 collinear
```

`orient3d`, `incircle`, `insphere` follow the same pattern.

### Cross-validation oracle

`elreal` can serve as the cross-implementation oracle for other multi-component
types in Universal's validation pipeline:

```cpp
#include <universal/verification/test_suite_elreal_oracle.hpp>

dd_cascade target = exp(dd_cascade(0.5));
elreal     oracle = exp(elreal(0.5));

bool ok = check_against_elreal_oracle(target, oracle);
```

See `docs/algorithmic-details/multi-component-arithmetic.md` section 8.6 for
details on what this catches today (cross-implementation gross-error
detection) and the current precision ceiling.

## Refinement protocol

`at(k)` is the primitive; `refine_to(bits)` is the user-facing wrapper:

```cpp
elreal v = exp(elreal(0.5));

double c0 = v.at(0);           // leading double (calls generator if needed)
double c1 = v.at(1);           // depth-1 correction
v.refine_to(200);              // ask for ~200 bits of precision

std::size_t depth = v.computed_depth();   // high-water materialised
const std::vector<double>& comps = v.components();  // read-only access
```

`operator double()` sums only the *currently-materialised* components and
does **not** invoke the generator. To see a generator-driven correction in
the double-converted value, explicitly call `at(k)` (or `refine_to`) first:

```cpp
elreal s = sin(elreal_pi());

double naive = double(s);      // depth-0 only: ~1.22e-16 (= std::sin(M_PI))
(void)s.at(1);                 // forces depth-1 materialisation
double full = double(s);       // depth-0 + depth-1 ~= 0 (the cos(pi)*pi.at(1)
                               //   correction cancels std::sin's rounding error)
```

This is by design: `operator double()` is the cheap "best estimate at current
depth" path. Refinement is an explicit caller choice.

## Known limitations

- **Depth-1 ceiling.** Arithmetic operators and math functions currently
  refine to depth 1. Deeper refinement (depth 2+) requires Newton-style
  lazy iteration or a higher-precision internal algorithm. The infrastructure
  is in place (the generator can produce arbitrarily many components); the
  per-function generators just need to be extended. Tracked as a follow-up to
  Phase F of the elreal epic (#873).
- **Constants stop at 4 components (~212 bits).** `elreal_pi`, `elreal_e`,
  and friends use static 4-component expansions reused from `qd_constants.hpp`.
  Extending past this requires either BBP-style on-demand bit extraction (for
  pi) or Taylor truncation with lazy refinement (for e, ln2, etc.).
- **Forward trig range reduction.** `sin/cos/tan` use the std library for
  depth 0, which limits faithful results to "reasonable magnitude" inputs
  (roughly `|x| < 2^25`). Payne-Hanek range reduction using lazy pi bit-pull
  is documented in `elreal_impl.hpp` as the path forward.
- **Comparison budget is not strictly an oracle.** Equality returns 0 within
  the budget; mathematically-distinct values whose first non-zero component
  lies past the budget will compare equal. Raise the budget per-call when
  this matters.

## Comparison with `dd`, `qd`, `ereal`

| Aspect | `dd` (Bailey/Hida) | `qd` (Bailey/Hida) | `ereal<N>` (Shewchuk) | `elreal` (McCleeary) |
|---|---|---|---|---|
| Components | 2 (fixed) | 4 (fixed) | up to N (compile-time) | unbounded (per-call) |
| Precision target | committed upfront | committed upfront | committed upfront | per-call (budget) |
| Common-case cost | constant | constant | constant | depth-0 (cheapest) |
| Worst-case cost | constant | constant | up to N components | up to budget components |
| Best use case | hot loops at 106 bits | hot loops at 212 bits | adaptive ~127-303 bits | precision-on-demand, undecidable comparisons |

Pick `dd` / `qd` when the precision target is known. Pick `ereal<N>` when it
is adaptive but bounded. Pick `elreal` when the common case dominates *or*
when undecidable comparison (geometric predicates, symbolic computation) is
in scope.

## References

- McCleeary, R. (2019). *Lazy Exact Real Arithmetic Using Floating Point
  Operations*. PhD dissertation, University of Iowa.
  <https://iro.uiowa.edu/esploro/outputs/doctoral/Lazy-exact-real-arithmetic-using-floating/9983776998202771>
- Shewchuk, J. R. (1997). "Adaptive Precision Floating-Point Arithmetic and
  Fast Robust Geometric Predicates." *Discrete & Computational Geometry*
  18(3), 305-363.
- Priest, D. M. (1991). *On Properties of Floating Point Arithmetics:
  Numerical Stability and the Cost of Accurate Computations*. PhD
  dissertation, UC Berkeley.

## Files

- `include/sw/universal/number/elreal/elreal.hpp` -- umbrella header
- `include/sw/universal/number/elreal/elreal_impl.hpp` -- main class + arithmetic + math
- `include/sw/universal/number/elreal/math/constants/elreal_constants.hpp` -- pi, e, ln2, ...
- `include/sw/universal/number/elreal/geometry/predicates.hpp` -- geometric predicates
- `include/sw/universal/verification/test_suite_elreal_oracle.hpp` -- oracle helper
- `docs/algorithmic-details/multi-component-arithmetic.md` -- design + comparison study
