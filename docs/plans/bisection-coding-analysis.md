# Universal Coding of the Reals using Bisection -- Analysis and Assessment

## Paper Summary

**Title:** Universal Coding of the Reals using Bisection
**Author:** Peter Lindstrom, Lawrence Livermore National Laboratory
**Venue:** CoNGA'19 (Conference for Next Generation Arithmetic), March 2019, Singapore
**DOI:** 10.1145/3316279.3316286

## Core Insight

Any number system for the reals can be fully defined by just two functions:

1. **Generator** `g(x)`: produces a monotonic bracketing sequence `a_i = g^i(1)`
   that narrows down the scale of the number (analogous to the exponent)
2. **Refinement** `f(a, b)`: bisects a bounded interval `[a, b]` to produce a
   midpoint (analogous to decoding the fraction bits)

Every bit in the encoding is the outcome of a binary comparison: is the value
in the lower half or upper half of the current interval? Encoding is binary
search; decoding is interval bisection.

## Architecture

### Phase 1: Bootstrapping (3 special values)

The framework starts with the projective reals on `(-inf, +inf)` and
bootstraps with three special bisection rules:

```
f(-inf, +inf) = 0       -- first bit: sign
f(-inf, 0)    = -1      -- negative side
f(0, +inf)    = +1      -- positive side
```

This gives 4 values at 2-bit precision: `{+/-inf, -1, 0, +1}`.

Two symmetry rules extend to all of R:

```
f(a, b) = -f(-b, -a)        -- negation
f(0, b) = f(b^-1, +inf)^-1  -- reciprocation
```

### Phase 2: Bracketing (unbounded search via generator)

Starting from `a_0 = 1`, the generator produces an increasing sequence:

```
a_{i+1} = g(a_i)    -- e.g., g(x) = 2x for posits, g(x) = x^2 for URR
```

The bracketing phase finds the interval `[a_i, a_{i+1})` containing `|x|`.
The number of applications of `g` is encoded in unary (ones terminated by
a zero), forming the **exponent** prefix of the bit string.

The generator determines the dynamic range and the coarse structure of
the number line.

### Phase 3: Refinement (binary search via bisection)

Once `x` is bracketed in `[a_i, a_{i+1})`, refinement recursively bisects
this interval. Each bit selects the lower (0) or upper (1) half:

```
x_m = f(x_l, x_h)    -- bisection point (not necessarily the midpoint)
if x < x_m: emit 0, narrow to [x_l, x_m)
else:        emit 1, narrow to [x_m, x_h)
```

The refinement function determines the local precision, density, and
smoothness of the representation.

### Refinement as Generalized Mean

All refinement rules can be expressed as Kolmogorov means:

```
K_h(a, b) = h^-1((h(a) + h(b)) / 2)
```

| Representation | h(x)     | Mean type  | f(a,b)          |
|---------------|----------|------------|-----------------|
| IEEE/Posit    | x        | Arithmetic | (a + b) / 2     |
| LNS           | log(x)   | Geometric  | sqrt(a * b)     |
| URR           | log(x)   | Geometric  | sqrt(a * b)     |
| Elias delta   | log(x)   | Geometric  | sqrt(a * b)     |
| Harmonic      | 1/x      | Harmonic   | 2/(1/a + 1/b)   |

The choice of mean determines the **fraction map** phi(r) that maps fraction
bits to real values, and the associated probability density.

### Hyper Mean (bridging bracketing and refinement)

When the interval spans more than one binade (x_h > 2*x_l), a special
**hyper mean** H(a, b) is used that averages the exponents geometrically
before refining the fraction:

```
H(a, b) = 2^H(log_2(a), log_2(b))   when both a,b > 0
```

This handles the transition between the exponent and fraction fields
seamlessly, without needing to explicitly separate them.

## Key Properties

### Tapered Accuracy

Numbers near 1 get more fraction bits (short exponent prefix), while
extreme values consume more bits on the exponent, leaving fewer for
the fraction. This is inherent to the unary exponent encoding and
is shared by posits, Elias codes, and all representations in this
framework. IEEE is the exception -- its fixed exponent length gives
uniform accuracy within each binade but wastes bits at all scales.

### No Exponent/Fraction Boundary

Unlike IEEE 754 and posits, the framework does not explicitly separate
exponent and fraction fields. The bits naturally transition from
bracketing (exponent) to refinement (fraction) through the encoding
process. There is no arbitrary decision about how many bits to allocate
to each.

### Smooth Density via Natural Refinement

The "natural refinement" rule (Eq. 12) produces a smooth mapping from
bit strings to reals with no wobbling accuracy. This eliminates the
sawtooth-like precision variation seen in IEEE and posit representations
that use piecewise linear fraction maps.

### Number Distributions

Each coding scheme has an associated probability density. The framework
derives these analytically:

| System | Density f(x) for x >= 1 | Distribution |
|--------|------------------------|--------------|
| Unary | ln(2) * 2^(1-x) | Laplace |
| Posit(m) | 2^(-m) * x^(-2^(-m) - 1) | Type-1 Pareto |
| Elias delta | ln(2) / (x * ln(2x)^2) | Log-Cauchy |
| LNS(m) | 2^(1-m) / (ln(2) * x) | Reciprocal |

## Number Systems Expressible in This Framework

### Existing systems (Table 1 from paper)

| System | Generator g(x) | Sequence a_i | Refinement f(a,b) |
|--------|---------------|-------------|-------------------|
| Unary | x + 1 | i + 1 | (a+b)/2 (arithmetic) |
| Fibonacci | round(phi*x) | F_{i+2} | (a+b)/2 |
| Posit(m) | 2^(2^m) * x | b^i | hyper mean H |
| Elias gamma | 2x | 2^i | hyper mean H |
| Elias delta | 2x^2 | 2^(2^i - 1) | hyper mean H |
| Elias omega | 2^x | 2^^i (tetration) | hyper mean H |
| FP(m) | H(x, 2^(2^(m-1))) | see paper | geometric mean |
| LNS(m) | 2^(2^(m-1)) * sqrt(x) | 2^(2^(m-1)(1-2^(-i))) | geometric mean |

### Novel systems proposed

- **NaturalPosit**: posit with natural refinement instead of linear --
  eliminates wobbling accuracy, smooth density
- **Golden ratio base**: g(x) = phi*x, represents integers as sums of
  Fibonacci numbers
- **Base phi**: uses golden ratio as exponential base
- **Exponent-less systems**: Unary, Fibonacci -- no binary exponent needed

## C++ Implementation Pattern (Appendix B)

The paper provides a remarkably compact C++ implementation:

```cpp
// Fibonacci representation
struct Fibonacci {
    real operator()(real x) const { return std::round((1 + std::sqrt(real(5))) * x / 2); }
    real operator()(real a, real b) const { return (a + b) / real(2); }
};

// Posit representation with m-bit exponent
template <int m = 0, typename real = flex::real>
struct Posit {
    real operator()(real x) const { return real(base) * x; }
    real operator()(real a, real b) const { return a >= b ? (a + b) / real(2) : std::sqrt(a * b); }
    static const int base = 1 << (1 << m);
};

// Elias delta representation
struct EliasDelta {
    real operator()(real x) const { return real(2) * x * x; }
    real operator()(real a, real b) const { return a >= b ? (a + b) / real(2) : std::sqrt(a * b); }
};
```

Each number system is just a struct with two operators:
- `operator()(x)` -- the generator g(x)
- `operator()(a, b)` -- the refinement f(a, b)

The entire encoding/decoding machinery is shared.

## Assessment for Universal Library Integration

### Feasibility: HIGH

The bisection coding framework is an excellent fit for Universal because:

1. **Minimal per-type code**: each new number system is defined by just
   `g(x)` and `f(a,b)` -- two functions. The shared bisection machinery
   handles encoding, decoding, rounding, and special values.

2. **Unifies existing types**: posits, LNS, and Elias codes are already
   in Universal. The bisection framework provides a single abstraction
   that subsumes all of them, potentially simplifying maintenance.

3. **Enables rapid prototyping**: new number systems (Fibonacci, golden
   ratio, NaturalPosit, exponent-less types) can be explored with
   minimal implementation effort.

4. **Header-only, no dependencies**: the implementation is pure arithmetic
   -- no tables, no special hardware, no external libraries.

### Architectural Approach

A `bisection<Generator, Refinement, p>` template class where:
- `Generator` provides `g(x)` (bracketing sequence)
- `Refinement` provides `f(a,b)` (interval bisection)
- `p` is the precision in bits

The class would implement the Universal number system concept:
- Trivially constructible with `blockbinary` storage
- `operator=` from native types via the bisection encoding algorithm
- `operator double()` via the bisection decoding algorithm
- Arithmetic via encode-compute-decode (initially through double)
- `to_binary()`, `type_tag()`, `numeric_limits` specialization

### Key Design Decisions

1. **Auxiliary precision type**: encoding/decoding requires an auxiliary
   real type for interval arithmetic. Use `double` for types up to ~50
   bits; `dd` or `qd` for higher precision. This is the main performance
   tradeoff noted in the paper.

2. **Storage**: fixed-size `blockbinary<p+1>` (p bits + sign), same as
   other Universal types. The bit layout is implicit -- no explicit
   exponent/fraction fields to manage.

3. **Rounding**: the framework defines rounding naturally via the last
   bisection step. Round to nearest, ties to even by checking the
   binary representation's last bit.

4. **Special values**: +/-inf and NaN are handled by the bootstrapping
   rules. Zero is the midpoint of (-inf, +inf).

### Suggested Implementation Phases

1. **Phase 1**: Core `bisection<G, F, p>` class with encode/decode via
   double, basic arithmetic, `to_binary()`, `numeric_limits`
2. **Phase 2**: Add pre-built generators for Elias gamma/delta/omega, Fibonacci, Unary
3. **Phase 3**: create a CLI, like ucalc, for interactive experimentation with these number systems
4. **Phase 4**: Performance -- direct bit manipulation for known generators
5. **Phase 5**: Validation -- exhaustive testing for small p, using inductive reasoning of 
the relationship between configurations nbits, and nbits+1.

### Risks and Limitations

- **Performance**: O(p) bisection steps per encode/decode vs O(1) for
  direct bit manipulation in native implementations like cfloat/posit. 
  Not competitive for applications, but valuable for prototyping and exploration.
- **Auxiliary type precision**: encoding accuracy depends on the auxiliary
  real type. For p > 53, need multi-precision support.
- **Arithmetic**: native arithmetic in bisection representation requires
  decode-compute-encode, which is slow. Direct arithmetic algorithms
  (like posit's regime-based add) would need separate implementation.

### Value Proposition

The bisection framework would give Universal a unique capability: a
**number system laboratory** where researchers can define new representations
with two lines of code and immediately explore their properties --
dynamic range, precision distribution, rounding behavior, density --
using ucalc and the existing test infrastructure. No other library offers
this level of generality for number system experimentation.
