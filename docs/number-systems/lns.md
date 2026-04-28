# LNS: Logarithmic Number System

This page is the conceptual introduction to the Logarithmic Number System
(LNS): the history, the design intent, the value proposition, and the
fundamental cost structure. The Universal-specific implementation details
live on the [Implementation](../lns-implementation/) page; the swappable
add/sub policies on the [Add/Sub Algorithms](../lns-addsub-algorithms/)
page; and the per-algorithm tolerance contract for regression testing on
the [Algorithm Tolerance](../lns-tolerance-traits/) page.

## History

The logarithmic number system has roots that go back further than digital
computers themselves -- the slide rule (~1620) is fundamentally an LNS
multiplication device, with logarithmic scales that turn multiplication
into physical addition.

The modern computer-arithmetic treatment starts with:

- **Mitchell, J. N. (1962).** "Computer multiplication and division using
  binary logarithms." *IRE Transactions on Electronic Computers*,
  EC-11(4), 512-517.

  Mitchell observed that multiplication / division on power-of-two
  binary values reduces to integer add / subtract on the exponent, and
  proposed a piecewise-linear approximation for the log-add correction
  needed to handle addition. The two-line formula `log2(1 + u) ~= u`
  for `u in [0, 1]` is now known as the *Mitchell approximation* and
  remains the cheapest closed-form sb_add available -- at the cost of
  ~9% worst-case relative error.

The 1980s and 1990s saw a wave of LNS hardware research aimed at making
the log-domain add tractable for production hardware:

- **Arnold, M. G., Bailey, T. A., Cowles, J. R., Cuthbertson, K. (1990).**
  "An Improved Logarithmic Number System Architecture." *Journal of VLSI
  Signal Processing*, 1(1), 13-20.

  A series of follow-on Arnold and Bailey papers explored multi-knot
  piecewise approximations and hardware-friendly closed forms for the
  log-add correction. The "Arnold-Bailey style" piecewise-linear with
  knots at integer-d values is a representative member of that family.

- **Coleman, J. N., Chester, E. I., Softley, C. I., Kadlec, J. (2000).**
  "Arithmetic on the European Logarithmic Microprocessor." *IEEE
  Transactions on Computers*, 49(7), 702-715.

  The European Logarithmic Microprocessor (ELM) project demonstrated a
  full LNS-based microprocessor competitive with floating-point on
  multiply-heavy DSP workloads. ELM remains the highest-profile public
  LNS hardware and a reference point for production-class accuracy
  bounds.

- **Coleman, J. N. (2008).** "An (n+2)-bit multiplicative inverse and
  square root algorithm with optimum approximation." A representative
  example of the LNS lineage: when your number system makes multiply
  cheap, the operations *adjacent* to multiply (powers, roots, log)
  collapse to nearly-free integer manipulations on the exponent.

LNS has had successful niche deployments in defence (signal processing,
HARM), embedded DSP, and more recently in low-power neural network
accelerators where multiply-heavy convolutions dominate the compute and
the system can tolerate the moderate add/sub accuracy hit.

## Why

In many signal processing and scientific computing workflows,
multiplication and division dominate the computation. Think FFTs, power
spectra, convolutions, geometric transformations. In IEEE-754
floating-point, multiplication requires a full significand multiply -- a
hardware-expensive operation that consumes area, power, and latency.

In the Logarithmic Number System (LNS), every value is stored as the
*logarithm* of its magnitude (with a separate sign bit). The arithmetic
identities

```text
log2(x * y) = log2(x) + log2(y)
log2(x / y) = log2(x) - log2(y)
log2(x^2)   = 2 * log2(x)
log2(sqrt(x)) = (1/2) * log2(x)
```

mean that the operations that are most expensive in IEEE-754
floating-point hardware (significand multiply, significand divide,
iterative square root) collapse to integer add / subtract / shift on the
exponent field. If your workload is multiply-heavy, LNS can provide
dramatic savings in hardware cost, power consumption, and latency.

The trade-off is symmetric: addition and subtraction, which are cheap in
standard floating-point, become expensive in LNS -- they require either
table lookups or polynomial / piecewise approximations of a
transcendental correction term. LNS is not a universal replacement for
floating-point; it is a specialized tool for workloads where the
operation mix favors multiplication, and where the loss of bit-exact
add/sub accuracy is an acceptable trade for the gain in multiply
throughput / energy.

## What

A Logarithmic Number System represents a real number `x` as a `(sign,
log2(|x|))` pair, with the logarithm typically stored as a fixed-point
or signed-integer value. Special values (zero, infinity, NaN) are
encoded via reserved bit patterns; in particular, *zero is not
representable as a value of `log2(0) = -infinity`* and must be
represented out-of-band.

Conceptually:

```text
value = (-1)^sign * 2^exponent

  sign:     1 bit
  exponent: signed fixed-point number (integer bits + fractional bits)
```

The number of fractional bits in the exponent controls the *precision*
of the representation; the number of integer bits controls the *dynamic
range*. Doubling the integer bits squares the dynamic range; doubling
the fractional bits halves the relative spacing between adjacent
representable values. The trade-off is identical in spirit to
IEEE-754's exponent-vs-mantissa split, but lives entirely in the log
domain rather than half in the linear domain (mantissa) and half in the
log domain (exponent).

### Operation cost: the fundamental trade

| Operation       | IEEE float                            | LNS                                    |
|-----------------|---------------------------------------|----------------------------------------|
| Multiplication  | Significand multiply (expensive)      | Integer add on exponent (cheap)        |
| Division        | Significand divide (very expensive)   | Integer subtract on exponent (cheap)   |
| Square          | Significand multiply                  | Exponent left-shift by 1               |
| Square root     | Iterative significand op (very expensive) | Exponent right-shift by 1          |
| Addition        | Align + significand add (cheap)       | Transcendental sb_add (expensive)      |
| Subtraction     | Align + significand subtract (cheap)  | Transcendental sb_sub (expensive)      |

The LNS column inverts the IEEE column for the multiply / divide / power
group versus the add / subtract group. Every workload-level decision
about LNS comes back to "does my operation mix tilt toward the cheap
column or the expensive column?"

## How

The arithmetic identities make multiply / divide / power trivial. The
hard problem is addition.

### Multiplication / division: integer ops on the exponent

For two LNS values `x = (sx, Lx)` and `y = (sy, Ly)`:

```text
x * y = (sx XOR sy, Lx + Ly)
x / y = (sx XOR sy, Lx - Ly)
x^2   = (0,         2 * Lx)
sqrt(x) = (0,       Lx / 2)   [requires sx == 0 to be real-valued]
```

`Lx + Ly` is just a fixed-point add on the exponent fields. No
transcendental, no significand multiply, no rounding past whatever the
exponent precision dictates. Power-of-two scaling reduces to a left- or
right-shift by an integer; integer powers reduce to a multiply on the
exponent.

### Addition: the Gauss log-add formulation

For two same-sign positive LNS values `x, y > 0` with logs `Lx =
log2(x)`, `Ly = log2(y)`, arrange `Lx >= Ly` and observe

```text
log2(x + y) = log2(2^Lx + 2^Ly)
            = log2(2^Lx * (1 + 2^(Ly - Lx)))
            = Lx + log2(1 + 2^d),  d = Ly - Lx <= 0
            = Lx + sb_add(d)
```

The correction term `sb_add(d) = log2(1 + 2^d)` is a one-dimensional,
smooth, monotonic function of `d`:

- `sb_add(0) = log2(2) = 1`
- as `d -> -infinity`, `sb_add(d) -> 0` (the smaller operand vanishes
  in the log domain when it is many orders smaller than the larger)
- always positive over `d <= 0`

The entire game in LNS addition is computing `sb_add(d)` -- the rest is
just sign routing and special-value bookkeeping. For mixed-sign
operands the analogous correction is `sb_sub(d) = log2(1 - 2^d)`, with
a fundamental difficulty: `sb_sub` has unbounded slope as `d -> 0`
(catastrophic cancellation) and goes to `-infinity` exactly at `d = 0`
(which is the `x + (-x) = 0` case).

Different LNS implementations differ almost entirely in how they
compute `sb_add` and `sb_sub`. The choices span:

- **Direct evaluation**: compute via `log2` and `exp2` library calls.
  Slow (two transcendentals per add) but accurate.
- **Lookup table**: precompute `sb_add(d)` over a finite d-grid and
  index + interpolate at runtime. Fastest, but pays in SRAM.
- **Polynomial approximation**: closed-form low-degree polynomial in a
  reduced variable (typically the (1+x)/(1-x) substitution). No table,
  one transcendental remaining.
- **Piecewise-linear**: ad-hoc closed-form with hand-chosen knot points
  (the Mitchell / Arnold-Bailey family). Cheapest of all, ~2.5%
  relative error worst-case.

Universal supports all four through a [swappable add/sub algorithm
framework](../lns-addsub-algorithms/) that lets users pick the
accuracy / SRAM / energy trade per `lns<>` instantiation.

## Problems LNS Solves

| Problem | How LNS Solves It |
|---------|-----------------------|
| Multiplication is the bottleneck (DSP, FFT, spectra) | Multiplication becomes cheap addition |
| Hardware multiplier is too expensive (area, power, latency) | LNS needs only an adder for multiply |
| Square root is expensive in hardware | Just shift the exponent right by 1 bit |
| Need wide dynamic range with small bit width | Logarithmic encoding provides exponential range |
| Geometric computations dominated by products | All products become sums of logarithms |
| Embedded DSP with limited hardware resources | Multiply-heavy DSP reduces to add-only operations |

## Where to next

- The [Implementation](../lns-implementation/) page covers Universal's
  `lns<nbits, rbits, bt, xtra...>` template, including the fixpnt
  exponent storage, special-value encoding, and how to instantiate and
  use the type.
- The [Add/Sub Algorithms](../lns-addsub-algorithms/) page covers the
  five swappable policies (DoubleTrip, DirectEvaluation, Lookup,
  Polynomial, ArnoldBailey), the customization point for selecting one
  per `lns<>` instantiation, and the decision tree for picking the
  right one for your target.
- The [Algorithm Tolerance](../lns-tolerance-traits/) page covers how
  the regression suite handles the inherent approximation in the
  non-exact algorithms via per-algorithm log-domain error bounds and a
  rbits-aware value-domain tolerance helper.
