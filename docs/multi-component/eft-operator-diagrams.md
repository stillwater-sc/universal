# Error-Free Transformation (EFT) operator diagrams and tests

A testing-oriented reference for the three error-free transformations that the
multi-component (`ereal`, expansion) arithmetic is built on. It shows, for each
operator, the exact data-flow of floating-point operations, the identity it
guarantees, and how that identity is verified by the regression test
`internal/expansion/primitives/eft_exactness.cpp`.

All diagrams are ASCII. Operators are in
`include/sw/universal/internal/expansion/expansion_ops.hpp`.

## What an EFT is

For a floating-point operation `op` in {+, *}, an error-free transformation
turns the single rounded result into a pair `(x, y)` of floating-point numbers:

```
    x = fl(a op b)        the value you would get from the hardware op (rounded)
    y = (a op b) - x      the rounding error -- and y is itself representable
```

The defining guarantee is the exact identity

```
    x + y == a op b        (mathematically exact, no rounding)
```

So `x` is the part that "fits" in one double and `y` captures every bit that
was rounded away. Expansions chain these pairs to carry arbitrary precision:
each limb is an `x`, and the `y` it shed is folded into the next limb.

Notation in the diagrams:
- `(expr)` in a box is one IEEE-754 double operation (correctly rounded).
- `->` names the value produced.
- `[PRIMARY]` is the rounded result `x`; `[ERROR]` is the exact error `y`.

---

## TWO-SUM (Knuth 1969 / Dekker 1971), 6 ops, no precondition

Computes `(x, y)` with `x + y == a + b` for ALL finite `a, b`.

```
        a        b
         \      /
        +-v----v-+
        |  a + b |  ----------------------> x            (1) [PRIMARY] the sum
        +---+----+
            | x
       +----v----+
       |  x - a  |  ----------------------> b_virtual    (2) part of b that landed in x
       +----+----+
            | b_virtual
       +----v--------+
       |  x - b_virt |  ------------------> a_virtual    (3) part of a that landed in x
       +-------------+

   b, b_virtual -->  ( b - b_virtual ) ---> b_roundoff   (4) bits of b that were dropped
   a, a_virtual -->  ( a - a_virtual ) ---> a_roundoff   (5) bits of a that were dropped

   a_roundoff, b_roundoff
                -->  ( a_round + b_round ) -> y          (6) [ERROR] y == (a+b) - x  exactly
```

Why six operations: because `|a|` and `|b|` are unknown, the algorithm cannot
assume which operand dominates, so it recovers the rounded-away bits of BOTH
operands (steps 2-5) and sums them (step 6). The two "virtual" values
reconstruct what each operand contributed to `x`; the differences are exactly
what was lost.

Exactness regime: holds whenever `a + b` does not overflow to infinity
(subnormals included -- addition of subnormals is itself exact).

---

## FAST-TWO-SUM (Dekker 1971), 3 ops, requires |a| >= |b|

Same guarantee `x + y == a + b`, at half the cost, but ONLY when `|a| >= |b|`.

```
        a        b              PRECONDITION: |a| >= |b|
         \      /
        +-v----v-+
        |  a + b |  ----------------------> x            (1) [PRIMARY] the sum
        +---+----+
            | x, a
       +----v----+
       |  x - a  |  ----------------------> t            (2) the rounded value of b's contribution
       +----+----+
            | t, b
       +----v----+
       |  b - t  |  ----------------------> y            (3) [ERROR] y == (a+b) - x  exactly
       +---------+
```

The precondition is load-bearing: when `|a| >= |b|`, the quantity `x - a`
recovers `b`'s contribution exactly in one step, so a single subtraction yields
the error. If `|a| < |b|` the recovery is wrong and `y` no longer captures the
full error. Expansion algorithms keep their components magnitude-ordered
precisely so they can use this cheaper operator; in this library it is used
inside `renormalize_expansion`'s sweep, where the operands are sorted.

---

## TWO-PROD (Dekker 1971, FMA form), 2 ops

Computes `(x, y)` with `x + y == a * b` exactly, using a fused multiply-add.

```
        a        b
         \      /
        +-v----v-+
        |  a * b |  ----------------------> x            (1) [PRIMARY] the product
        +---+----+
            | x
   a, b, -x |
       +----v------------+
       |  fma(a, b, -x)  | ----------------> y           (2) [ERROR] y == a*b - x  exactly
       +-----------------+
```

`fma(a, b, -x)` computes `a*b - x` with a SINGLE rounding. Because `x = fl(a*b)`,
the true error `a*b - x` is exactly representable in a double, so the fma
returns it with no error -- giving the exact `y` in one instruction. (Without an
FMA this needs Dekker's 17-op Veltkamp split; the library assumes hardware FMA.)

Exactness regime: holds when `a * b` is a NORMAL double. If the product
overflows, `x` is infinity; if it underflows into the subnormal range, the
error term `y` is not representable and exactness is lost. This is a property of
double, not of the algorithm: an expansion whose value spans more than ~1022
binary exponents cannot have an exact double-limb product.

Caveat: the identity depends on `fma` being correctly rounded. A software `fma`
that is off by even 1 ULP (observed on some platforms) silently breaks every
product. The test below doubles as a platform check for this.

---

## How they compose

Expansion operations are sequences of these EFTs:
- `grow_expansion`, `linear_expansion_sum`, `renormalize_expansion` chain
  TWO-SUM / FAST-TWO-SUM to add expansions and keep components non-overlapping.
- `scale_expansion` / `expansion_product` chain TWO-PROD (then sum the partial
  products) to multiply expansions.

See `docs/bugs/ereal-priest-conformance-audit.md` for the routine-by-routine
mapping to Shewchuk/Priest.

---

## Testing: `internal/expansion/primitives/eft_exactness.cpp`

Each EFT is verified against an INDEPENDENT exact reference -- exact
dyadic-rational arithmetic (`include/sw/universal/verification/dyadic_exact.hpp`,
backed by `einteger`), which shares no code with the transformations. The check
for an operator is simply its defining identity, evaluated in exact arithmetic:

| Operator | Identity verified | Reference check (exact dyadic) | Regime exercised |
|----------|-------------------|--------------------------------|------------------|
| `two_sum`      | `x + y == a + b`, `x == fl(a+b)` | `dyadic(a)+dyadic(b) == dyadic(x)+dyadic(y)` | any finite sum; subnormal tails, ties, opposite signs, large gaps |
| `fast_two_sum` | same, with `\|a\| >= \|b\|`      | same, operands forced `\|a\|>=\|b\|`         | ordered pairs; plus a counterexample asserting the unordered case is NOT exact (precondition is load-bearing) |
| `two_prod`     | `x + y == a * b`, `x == fl(a*b)` | `dyadic(a)*dyadic(b) == dyadic(x)+dyadic(y)` | normal-product band (no overflow / subnormal underflow); also a de-facto `fma` correctness check |

Structure of the test:
- `VerifyOracleSanity` first checks the dyadic reference on known exact values,
  so any later failure is attributable to the EFT, not the oracle.
- `VerifyTwoSum`, `VerifyFastTwoSum`, `VerifyTwoProd` each run structured edge
  cases plus randomized fuzz across the exponent range (REGRESSION_LEVEL 1..4).
- A failure prints the operator, the offending `(a, b)`, and the iteration, so
  it is reproducible from the fixed seed.

This is Layer 1 of epic #987; Layer 2
(`elastic/ereal/arithmetic/exact_value_oracle.cpp`) extends the same dyadic
reference from the primitives up to whole `ereal` operations.

## References

- D. E. Knuth, "The Art of Computer Programming, Vol 2," (TwoSum).
- T. J. Dekker, "A floating-point technique for extending the available
  precision," Numerische Mathematik 18, 1971 (FastTwoSum, TwoProduct, split).
- J. R. Shewchuk, "Adaptive Precision Floating-Point Arithmetic and Fast Robust
  Geometric Predicates," Discrete & Computational Geometry 18:305-363, 1997.
- Companion docs: `docs/multi-component/comparison-priest-bailey-shewchuk.md`,
  `docs/bugs/ereal-priest-conformance-audit.md`,
  `docs/bugs/ereal-failure-mode.md`.
