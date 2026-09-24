# elreal: Lazy, Unbounded-Precision Real Numbers

## Why

Every other floating-point type in this library answers the question "how many bits shall
I keep?" before the computation starts. Sometimes that is the wrong question. If you are
computing a constant to publish, checking whether an algorithm converges, or building a
reference answer to score a faster type against, what you actually want is: *keep going
until the answer stops changing.*

`elreal` is that type. It carries a real number as an unevaluated expression over a stream
of blocks, and it computes only as many of them as you ask for. Ask for more digits and it
refines the same value further -- it does not recompute from scratch, and it does not run
out of precision at a fixed width.

```cpp
#include <universal/number/elreal/elreal.hpp>
using namespace sw::universal;

elreal<double> a(1), b(7);
elreal<double> q = a / b;       // nothing has been computed yet
double d = double(q);           // now it evaluates, to the current precision
```

The practical consequence is that a textbook algorithm written against a generic `Real`
converges on `elreal` where it stalls on `double`. Newton's iteration for `sqrt(2)` run on
a native `double` stops improving at 16 digits however many times you iterate it; the same
loop on `elreal` passes 1150 digits. Nothing in the kernel changes -- only the type.

## What

```cpp
template<typename FpType = double>
class elreal;
```

`FpType` is the *host* type: the ordinary floating-point type each block is stored in. It
sets how many bits a block carries, not how many the number can reach.

| host | bits per block | typical digits per block |
|---|---|---|
| `elreal<double>` (default) | 53 | ~16.1 |
| `elreal<float>` | 24 | ~6.4 |
| `elreal<bfloat16>` | 7 | ~2.8 |
| `elreal<half>` | 11 | ~3.6 |

Every host reaches arbitrary precision. A narrow host simply needs more blocks for the
same digits -- `float` needs about 2.2x as many as `double`. This was not always true:
until v4.9.0 each host stalled at a ceiling set by its *exponent* range rather than its
precision, and `float` stopped dead at 37 digits. That ceiling is gone.

### Precision is a runtime property

Precision is measured in blocks and carried per value, not per type. The default is 32
blocks (about 510 decimal digits on a `double` host):

```cpp
std::cout << elreal_default_precision();   // 32

elreal<double> x(2);
std::cout << x.precision();                // 32
x.precision(64);                           // this value now refines further
```

To change the default for a scope, use the guard rather than setting it by hand -- it
restores the previous value on exit, including when an exception unwinds:

```cpp
{
    elreal_precision_guard g(64);
    // every elreal constructed here defaults to 64 blocks
    elreal<double> y(2);
    auto r = sqrt(y);
}   // default restored to 32
```

### Laziness, and where it ends

Arithmetic on finite values builds an expression; it does not evaluate. Evaluation is
forced at a *boundary*: converting to a host type, comparing, or printing.

```cpp
elreal<double> a(1), b(3);
auto q = a / b;              // lazy
double d = double(q);        // boundary: evaluates to q.precision() blocks
```

One consequence worth knowing: `a + (-a)` is zero in value but is still a lazy expression,
not a literal zero. Comparison is depth-bounded, so two values that agree to the compared
depth compare equal even if refining further would separate them. That is the correct
behaviour for a type whose precision is a runtime choice, but it is not the behaviour of a
fixed-width type, and it will surprise you once.

Non-finite values are held in a dedicated state that follows IEEE rules, so `inf` and `nan`
propagate as you expect rather than being represented as an expression.

## Using it

The math library is the usual one, over a generic `Real`:

```cpp
#include <universal/number/elreal/elreal.hpp>
#include <iostream>

using namespace sw::universal;

template<typename Real>
Real newton_sqrt(Real a, int iterations) {
    Real x = a;
    const Real half(0.5);
    for (int i = 0; i < iterations; ++i) x = (x + a / x) * half;
    return x;
}

int main() {
    elreal_precision_guard g(16);          // ~256 digits on a double host
    elreal<double> two(2);
    std::cout << double(newton_sqrt(two, 10)) << '\n';
}
```

Newton doubles the correct digits each step, so the iteration count you need is
logarithmic in the target -- about 11 steps for a thousand digits, not 40. Iterating well
past convergence is pure cost, because every step still pays the full precision.

**Ramp the precision with the iterate.** Paying the final precision from the first step,
when the iterate is only accurate to a handful of digits, wastes most of the work. Raising
`precision()` as the iterate earns it is several times faster for the same answer -- in
the thousand-digit demo, 4s and 25s instead of 25s and 206s on the two hosts.

`sqrt`, `exp`, `log`, the trigonometric and hyperbolic functions and their inverses are all
available, as are the constants. Large-argument `sin`/`cos`/`tan` use Payne-Hanek argument
reduction, so they stay accurate out to very large `|x|` rather than losing digits to the
reduction itself.

### Printing more digits than a double holds

Streaming an `elreal` or converting it to `double` necessarily collapses it to the host's
precision -- that is what the boundary does. To render the digits the value actually
carries, go through the exact dyadic rendering helper:

```cpp
#include <universal/verification/elreal_reference_digits.hpp>   // zbcl_to_dyadic
```

`applications/precision/elreal/thousand_digit_sqrt.cpp` is the worked example: it renders
1000+ digits of `sqrt(2)` on both a `float` and a `double` host and scores them against a
1200-digit reference computed in exact integer arithmetic outside the library.

## Choosing between elreal and ereal

Both are adaptive-precision reals, and they make opposite trades.

| | [`ereal`](/universal/number-systems/ereal/) | `elreal` |
|---|---|---|
| Precision bound | capped by a limb budget | unbounded |
| Evaluation | eager | lazy |
| Precision set | template parameter | runtime, per value |
| Cheaper at | trigonometry | `sqrt`, `exp` |

Reach for `ereal` when you know the precision you need and want it to behave like an
ordinary value type. Reach for `elreal` when you do not know the precision you need --
when the whole point is to keep refining until something converges, or when you are
generating a reference that has to be right to more digits than any fixed type offers.

## Cost

`elreal` is the most expensive type in the library, and deliberately so: it buys
correctness at arbitrary depth, not speed. At precisions where `dd` and `qd` can deliver
the answer, they beat it on throughput by a wide margin. Use it where the digits matter
more than the clock -- reference generation, convergence studies, and scoring other types.
