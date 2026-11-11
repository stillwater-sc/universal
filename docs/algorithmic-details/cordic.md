# CORDIC: rotation, multiplication and the elementary functions by shift and add

CORDIC (COordinate Rotation DIgital Computer) computes trigonometric, hyperbolic, exponential and
logarithmic functions, square roots, products and quotients with nothing but additions, shifts and a
small table of constants. It is one of the few algorithms that has been in continuous use for more
than sixty years, from a 1950s bomber navigation computer to pocket calculators, floating-point
coprocessors and the FPGAs in today's radios.

This document covers:

- [Why](#why-the-problem-cordic-solves): the problem CORDIC solves.
- [What](#what-one-recurrence-three-geometries-two-modes): one recurrence, three geometries and two modes, and what the `m` in the
  `x` update means.
- [How](#how-it-works): the derivation, the gain, convergence, range reduction and the errors.
- [When](#when-to-use-cordic-and-when-not): where CORDIC is the right choice and where it is not.
- [Who](#who-a-history-of-cordic): the history, from Volder (1956) and Walther (1971) to today.
- [In Universal](#cordic-in-universal): the engines in this library, their API and their measured accuracy.

It is the educational companion to the implementation in
`include/sw/universal/number/fixpnt/math/cordic.hpp`, the fixpnt API section in
`docs/number-systems/fixpnt.md`, and the regression tests `static/fixpnt/binary/math/cordic.cpp` and
`cordic_linear.cpp`.

## Why: the problem CORDIC solves

A rotation of the vector `(x, y)` by the angle `theta` is

```text
x' = x cos(theta) - y sin(theta)
y' = x sin(theta) + y cos(theta)
```

which needs `cos(theta)`, `sin(theta)` and four multiplications. In the 1950s a multiplier was
expensive: it was slow, large and power hungry, and a navigation computer had to do this kind of
calculation continuously. CORDIC rearranges the rotation so that every multiplication is by a power of
two, which is a shift:

- Factor `cos(theta)` out: `x' = cos(theta) (x - y tan(theta))`, `y' = cos(theta) (y + x tan(theta))`.
- Build `theta` from a sum of elementary angles `alpha_i = atan(2^-i)`, each taken in the positive or the
  negative direction. For those angles `tan(alpha_i) = 2^-i`, so each partial rotation is two shifts and
  two additions.
- The `cos(alpha_i)` factors do not depend on the directions chosen, because cos is even, so their
  product is a single constant, applied once.

The result is a datapath of three adders, two shifters and a small read-only table. It produces one bit
of the result per iteration. For hardware that cannot afford a multiplier, or that has to run at a clock
rate a multiplier cannot reach, that is still the best trade.

## What: one recurrence, three geometries, two modes

### The unified iteration

In the form J. S. Walther published in 1971, every CORDIC is the same three-line recurrence. For
`i = 0, 1, 2, ...` and a direction `d_i` of `+1` or `-1`:

```text
x_{i+1} = x_i - m * d_i * y_i * 2^-i
y_{i+1} = y_i +     d_i * x_i * 2^-i
z_{i+1} = z_i -     d_i * alpha_i
```

The parameter `m` selects the geometry. It is the only place `m` appears, and it appears only in the
`x` update:

| `m` | coordinate system | `x` update | elementary angle `alpha_i` | what the iteration preserves |
|-----|-------------------|-----------|-----------------------------|------------------------------|
| `1` | circular | `x - d y 2^-i` | `atan(2^-i)` | `x^2 + y^2`, up to the gain |
| `0` | linear | `x` (unchanged) | `2^-i` | `x` |
| `-1` | hyperbolic | `x + d y 2^-i` | `atanh(2^-i)` | `x^2 - y^2`, up to the gain |

The two modes choose the directions:

- **Rotation mode** drives `z` to 0: `d_i = sign(z_i)`. It rotates the vector `(x, y)` by the angle `z`.
- **Vectoring mode** drives `y` to 0: `d_i = -sign(x_i y_i)`, or `-sign(y_i)` when `x > 0`. It rotates
  the vector onto the `x` axis and accumulates the angle it turned through in `z`.

Three geometries times two modes give six primitive operations. After `n` iterations, with
`K_m = prod_i sqrt(1 + m 2^-2i)` the gain of the geometry:

| | rotation mode (`z -> 0`) | vectoring mode (`y -> 0`) |
|---|---|---|
| circular, `m = 1` | `x_n = K (x cos z - y sin z)`, `y_n = K (y cos z + x sin z)` | `x_n = K sqrt(x^2 + y^2)`, `z_n = z + atan(y / x)` |
| linear, `m = 0` | `x_n = x`, `y_n = y + x z` | `x_n = x`, `z_n = z + y / x` |
| hyperbolic, `m = -1` | `x_n = K_h (x cosh z + y sinh z)`, `y_n = K_h (y cosh z + x sinh z)` | `x_n = K_h sqrt(x^2 - y^2)`, `z_n = z + atanh(y / x)` |

Everything else is built from these six, with the right starting values:

| function | how |
|---|---|
| `sin z`, `cos z` | circular rotation from `(1/K, 0, z)` |
| `atan2(y, x)`, `hypot(x, y)` | circular vectoring from `(x, y, 0)` |
| `x * z` | linear rotation from `(x, 0, z)` |
| `y / x` | linear vectoring from `(x, y, 0)` |
| `sinh z`, `cosh z` | hyperbolic rotation from `(1/K_h, 0, z)` |
| `exp z` | `cosh z + sinh z`, from the same rotation |
| `atanh(y / x)` | hyperbolic vectoring from `(x, y, 0)` |
| `ln w` | `2 atanh((w - 1) / (w + 1))`: hyperbolic vectoring from `(w + 1, w - 1, 0)` |
| `sqrt w` | `x_n / K_h` from hyperbolic vectoring of `(w + 1/4, w - 1/4, 0)`, since `(w + 1/4)^2 - (w - 1/4)^2 = w` |
| `tan z`, `tanh z` | a rotation, then linear vectoring of `(cos, sin)` or `(cosh, sinh)` |

### Why `m` sits in the `x` update

The three geometries are three ways of measuring the length of a vector. A rotation in the circular
geometry keeps `x^2 + y^2` constant: the vector moves along a circle. A "rotation" in the hyperbolic
geometry keeps `x^2 - y^2` constant: the vector moves along a hyperbola. Both are special cases of the
quadratic form

```text
R_m^2 = x^2 + m y^2
```

The "unit circle" of each geometry is the set where `R_m = 1`:

- `m = 1`: `x^2 + y^2 = 1`, the circle.
- `m = -1`: `x^2 - y^2 = 1`, the hyperbola.
- `m = 0`: `x^2 = 1`, the vertical line `x = 1`.

One CORDIC step multiplies the vector by the matrix

```text
[ 1             -m d 2^-i ]
[ d 2^-i         1        ]
```

and its determinant is `1 + m 4^-i`. The step therefore scales `R_m` by exactly `sqrt(1 + m 4^-i)`,
whatever the direction `d`. That is where the gain `K_m` comes from, and it is the same factor in every
geometry:

- circular: `sqrt(1 + 4^-i) > 1`, so the vector grows;
- linear: `sqrt(1 + 0) = 1`, so nothing grows;
- hyperbolic: `sqrt(1 - 4^-i) < 1`, so the vector shrinks.

The angle swept by a step, measured in the geometry's own sense, is

```text
alpha_i = (1 / sqrt(m)) atan(sqrt(m) 2^-i)
```

For `m = 1` this is `atan(2^-i)`. For `m = -1`, `sqrt(m) = j` turns `atan` into `atanh`, giving
`atanh(2^-i)`. For `m -> 0`, `atan(t)/t -> 1`, and the angle is just `2^-i`. In the linear geometry the
"angle" of a vector is its slope `y / x`, and the "rotation" is a shear that moves the point vertically
along the line `x = const`. A shear that adds `2^-i` to the slope is `y <- y + x 2^-i`, with `x` left
alone, which is why the linear `x` update vanishes.

So `m` scales the coupling from `y` back into `x`:

- In the circular geometry, `x` loses what `y` gains, as on a circle.
- In the hyperbolic geometry, `x` gains with `y`, as on a hyperbola.
- In the linear geometry, `x` is untouched, and `y` accumulates multiples of `x`. That is multiplication.

### A worked linear example: 0.75 * 0.625

Linear rotation from `(x, y, z) = (0.75, 0, 0.625)`, with `d_i = sign(z_i)`. The sign of 0 counts as
`+1`, as a hardware sign bit does:

| `i` | `d_i` | `y_{i+1} = y_i + d_i x 2^-i` | `z_{i+1} = z_i - d_i 2^-i` |
|-----|-------|-------------------------------|-----------------------------|
| 0 | +1 | 0.75 | -0.375 |
| 1 | -1 | 0.375 | 0.125 |
| 2 | +1 | 0.5625 | -0.125 |
| 3 | -1 | 0.46875 | 0 |
| 4 | +1 | 0.515625 | -0.0625 |
| 5 | -1 | 0.4921875 | -0.03125 |

The exact product is 0.46875. Each iteration consumes one signed binary digit of `z`. The remaining
angle `z_n` is at most `2^-(n-1)` in magnitude, so `y_n` is within `x 2^-(n-1)` of the product, and the
error halves with each iteration. At `i = 3`, `z` happens to reach 0 exactly. The iteration has no way to
stop there: it keeps stepping, with the residual halving around 0.

## How it works

### Convergence

Rotation mode can reach any angle `z_0` that the elementary angles can cancel. Two conditions make that
true:

- The total sum bounds the range: `|z_0| <= sum_{i=0}^{n-1} alpha_i + alpha_{n-1}`.
- Each angle must be no larger than the sum of all the later ones plus the last one:
  `alpha_i <= sum_{j>i} alpha_j + alpha_{n-1}`. Then a step that overshoots can always be undone by the
  steps after it.

The three geometries meet the second condition differently:

- **Circular.** `atan(2^-i)` is a little more than half of `atan(2^-(i-1))`, so the condition holds with
  room to spare. The range is `sum atan(2^-i) = 1.7432866...` rad, which is 99.88 degrees, a little more
  than a quadrant.
- **Linear.** `2^-i` is exactly the sum of all the later powers of two, so the condition holds with
  equality. The range is `|z| < 2` when the iteration starts at `i = 0`.
- **Hyperbolic.** `atanh(2^-i)` is a little *less* than half of `atanh(2^-(i-1))`, so the condition
  fails, and the remaining angle can outrun the steps. The fix, found by Walther, is to repeat
  iterations 4, 13, 40, 121, ... (the sequence `r_{k+1} = 3 r_k + 1`). The repeats make up the
  shortfall. Hyperbolic iteration also starts at `i = 1`, because `atanh(1)` is infinite. With the
  repeats, the range is `1.1181730...`; without them it would be `1.0554694...`.

Vectoring mode converges under the same conditions, applied to the angle of the starting vector:
`|atan(y_0 / x_0)| <= 1.743`, `|y_0 / x_0| < 2` and `|atanh(y_0 / x_0)| <= 1.118`.

### The gain

| geometry | `K_m = prod sqrt(1 + m 4^-i)` | `1 / K_m` |
|---|---|---|
| circular, `i >= 0` | 1.6467602581210656... | 0.6072529350088812... |
| linear | 1 | 1 |
| hyperbolic, `i >= 1`, with the repeats | 0.8281593609602156... | 1.2074970677630721... |

Because each step scales `R_m` by the same factor in either direction, the gain is known in advance. A
rotation-mode unit starts from `x_0 = 1/K` instead of 1, and the gain cancels at no cost. A vectoring
unit multiplies `x_n` by `1/K` once at the end, or leaves it to a later stage. The angle `z_n` needs no
correction at all. For a unit that stops after fewer iterations, `K` is the partial product up to `n`.

### Range reduction

The convergence ranges are small, so a unit for arbitrary arguments reduces them first:

- **Circular.** Write `theta = q pi/2 + r` with `|r| <= pi/4`, compute `sin r` and `cos r`, then swap
  and negate them by the quadrant `q mod 4`. The constant `pi/2` must carry as many bits beyond the
  datapath's fraction as `q` has integer bits. Otherwise the subtraction loses the low bits of `r`
  (the Payne-Hanek problem, at a small scale).
- **Hyperbolic.** For `exp z`, write `z = q ln 2 + r`: then `exp z = 2^q exp r`, and `2^q` is a shift.
  For `ln w` and `sqrt w`, write `w = 2^e m`: then `ln w = ln m + e ln 2`, and
  `sqrt w = 2^(e/2) sqrt m`, with an odd `e` moved into `m`.
- **Linear.** Scale the operands by powers of two into the convergence range, as a floating-point unit
  aligns significands, and scale the result back.
- **Vectoring.** A vector in the left half-plane is first rotated by `+-pi/2`, which is an exact swap and
  negation. Scaling both inputs by the same power of two leaves the angle unchanged, and keeps small
  inputs from losing their bits.

### Errors

A CORDIC result has three sources of error:

- **Angle error.** After `n` iterations, the angle left over is at most `alpha_{n-1}`, which is about
  `2^-(n-1)`. This error halves with each iteration: the algorithm produces one bit per iteration.
- **Truncation.** Each shift `y >> i` drops bits, as an arithmetic shift in hardware does. Each iteration
  adds up to one unit of the datapath's last place, so after `n` iterations the datapath has lost up to
  about `log2(n)` bits. Guard bits beyond the result's precision absorb this. `ceil(log2(n)) + 2` of them
  are enough to bring the total within one unit of the result.
- **Constants.** The table `alpha_i`, the gain `1/K` and the reduction constants are rounded to the
  datapath. Each one contributes half a unit of the datapath's last place.

The design trade is how many iterations and guard bits to spend. Universal's engines expose both, so the
trade can be measured before it is built (see the tables below).

### Variations

Sixty years of work have produced many refinements of the basic iteration:

- **Redundant CORDIC** (Ercegovac and Lang, 1990) represents `z` or `y` in a redundant number system, so
  choosing `d_i` does not wait for a carry to propagate across the word. Allowing `d_i = 0` then makes
  the gain depend on the data. Constant-scale-factor variants (Takagi, Asada and Yajima, 1991) repeat or
  double iterations to keep `K` fixed.
- **Radix-4 and higher radices** take two or more bits per iteration, with digits in `{-2 .. 2}` or
  wider. This halves the iteration count at the cost of small multiples of `x` and `y`.
- **Angle recoding, and skipping iterations whose angle is zero,** reduce the number of iterations for a
  known angle set, such as FFT twiddle factors.
- **Pipelined and unrolled** units lay the iterations out in space, one stage per iteration, for one
  result per clock. **Iterative** units reuse one stage `n` times, for the smallest area.
- **Decimal CORDIC**, and Meggitt's pseudo-division and pseudo-multiplication, use powers of ten, and
  repeat each digit position up to nine times. This is how calculators work.

## When to use CORDIC, and when not

CORDIC is the right choice when:

- **There is no hardware multiplier**, or one cannot be afforded: small FPGAs, ASIC blocks with a tight
  area budget, early microcontrollers and calculators.
- **The problem is a rotation in the first place.** Givens rotations for QR and SVD, the Park and Clarke
  transforms in motor control, digital down-conversion and the numerically controlled oscillators of
  software radios, FFT twiddles, and polar-rectangular conversion all compute `(x, y)` by an angle or the
  angle of `(x, y)`. CORDIC does that directly, without first computing `sin` and `cos`.
- **The clock rate matters more than the latency.** An unrolled CORDIC is a chain of adders with no
  multiplier in the critical path, and it pipelines to very high clock rates.
- **The answer must be bit-exact and reproducible.** A CORDIC is a fixed sequence of integer operations,
  with no rounding mode, no subnormals and no library dependence.

It is the wrong choice when:

- **A fast multiplier is available.** Modern CPUs and GPUs evaluate polynomial or rational approximations
  with fused multiply-adds, in a handful of cycles. A CORDIC needs one dependent iteration per bit, so it
  takes about 53 iterations for double precision.
- **The latency is critical.** Even pipelined, a CORDIC takes `n` stages to produce a result.
- **Software on a general-purpose processor.** Software CORDIC is slow per operation. Universal's engines
  exist to *model* a hardware unit, and are not meant as fast software math (see below).

## Who: a history of CORDIC

### Precursors

The digit-by-digit idea is older than electronic computers. Henry Briggs, in his *Arithmetica
Logarithmica* (1624), computed logarithms by building numbers from factors that are easy to apply, which
is the same structure as a shift-and-add recurrence. It is often cited as the ancestor of the
pseudo-division methods. The trigonometric identities CORDIC rests on are textbook material: Jack
Volder recalled finding the key formulas in the 1946 edition of the *Handbook of Chemistry and Physics*.

### 1956-1959: Jack Volder at Convair

Jack E. Volder, an engineer in the aeroelectronics department of Convair in Fort Worth, Texas, was asked
to replace the analog resolver in the navigation computer of the B-58 Hustler bomber with a digital
solution of comparable speed and accuracy. The resolver turned a heading and a distance into north and
east components, which is a rotation, continuously. A digital multiplier fast enough for that was not
practical for an airborne computer of the time.

Volder's answer was to rotate by a sequence of fixed angles whose tangents are powers of two, as
described above. He described it in an internal Convair report in 1956 ("Binary computation algorithms
for coordinate rotation and function generation"), and published it in 1959 in the *IRE Transactions on
Electronic Computers*, in the paper "The CORDIC Trigonometric Computing Technique". That paper covered
the circular rotation and vectoring modes, and it noted that the same hardware could multiply, divide and
convert between number systems. In the same issue, D. H. Daggett, a colleague at Convair, described
decimal-binary conversion with CORDIC. Convair went on to build CORDIC computers, and Volder looked back
on the work in "The Birth of CORDIC" (2000).

### 1962: Meggitt's pseudo-division and pseudo-multiplication

J. E. Meggitt, at IBM, published "Pseudo Division and Pseudo Multiplication Processes" in the *IBM
Journal of Research and Development* in 1962. It is a digit-by-digit family of methods for logarithms,
exponentials, ratios and arctangents, close in spirit to CORDIC and particularly suited to decimal
arithmetic. Meggitt's methods became the basis of the calculator algorithms that followed.

### 1968-1972: Hewlett-Packard and the calculators

Hewlett-Packard's 9100A desktop calculator (1968) computed its transcendental functions in decimal, with
digit-by-digit algorithms of this family. The HP-35 (1972), the first handheld scientific calculator, did
the same. David S. Cochran, who developed the HP-35's algorithms, described them in "Algorithms and
Accuracy in the HP-35" (*Hewlett-Packard Journal*, June 1972). Decimal shift-and-add trigonometry has been
standard in calculators ever since.

### 1971: Walther's unified CORDIC

John Stephen Walther, at Hewlett-Packard Laboratories, studied these algorithms and saw that they were
one algorithm. In "A Unified Algorithm for Elementary Functions" (AFIPS Spring Joint Computer Conference,
1971) he introduced the parameter `m` in the `x` update:

```text
x_{i+1} = x_i - m d_i y_i 2^-i
```

With `m = 1, 0, -1`, the one recurrence gives circular, linear and hyperbolic CORDIC. Volder's
trigonometry, multiplication and division, and the hyperbolic functions, exponentials, logarithms and
square roots all come from the same hardware, with a different `m`, a different table of elementary
angles and a different gain. Walther also gave the convergence analysis, including the repeated
iterations that hyperbolic mode requires. His retrospective, "The Story of Unified CORDIC" (2000), tells
how the work fed into HP's calculators and desktop computers.

The unified algorithm is the reason CORDIC outlived the B-58. One small, regular unit could compute
nearly every elementary function, and that was exactly what the calculators and coprocessors of the
1970s and 1980s needed.

### 1980s: coprocessors and VLSI signal processing

Intel's 8087 floating-point coprocessor (1980) computed its transcendental instructions with shift-and-add
algorithms of the CORDIC and pseudo-division family. R. Nave described them in 1983. Other early
floating-point coprocessors are reported to have done the same. Microcode loops of adds and shifts were
cheaper in silicon than a fast multiplier, and the accuracy was set by the number of iterations.

At the same time, CORDIC moved into signal processing. Ahmed, Delosme and Morf (1982) and others showed
that arrays of CORDIC processors compute Givens rotations, and with them QR decomposition, SVD and
adaptive filtering, in systolic arrays. Y. H. Hu's survey "CORDIC-based VLSI architectures for digital
signal processing" (*IEEE Signal Processing Magazine*, 1992) collected that work.

### 1990s-today: redundant arithmetic, FPGAs and radios

Ercegovac and Lang's redundant and on-line CORDIC (1990), and Takagi, Asada and Yajima's
constant-scale-factor redundant CORDIC (1991), removed carry propagation from the critical path. Ray
Andraka's "A survey of CORDIC algorithms for FPGA based computers" (1998) made the case for CORDIC on
FPGAs. There it remains the standard way to compute phase, magnitude and rotations, and the FPGA vendors
ship it as an IP core.

Today CORDIC runs in the numerically controlled oscillators and digital down-converters of software
radios, in GNSS receivers, in radar and sonar beamformers, in motor-control loops (resolver-to-digital
conversion and the Park transform), and in matrix-decomposition accelerators. Meher, Valls, Juang,
Sridharan and Maharatna surveyed the whole field in "50 Years of CORDIC" (2009), and J.-M. Muller's
*Elementary Functions: Algorithms and Implementation* places it among the other shift-and-add methods.

## CORDIC in Universal

### The engines

| engine | where | geometry and mode | status |
|---|---|---|---|
| `cordic::sincos`, `sin`, `cos` | fixpnt | circular rotation | #2015 |
| `cordic::atan2`, `atan`, `hypot`, `polar` | fixpnt | circular vectoring | #2015 |
| `cordic::tan` | fixpnt | circular rotation, then linear vectoring | #2041 |
| `cordic::mul` | fixpnt | linear rotation | #2040 |
| `cordic::div` | fixpnt | linear vectoring | #2040 |
| `cordic::exp`, `sinhcosh`, `sinh`, `cosh` | fixpnt | hyperbolic rotation | #2041 |
| `cordic::tanh` | fixpnt | hyperbolic rotation, then linear vectoring | #2041 |
| `cordic::atanh`, `log`, `sqrt` | fixpnt | hyperbolic vectoring | #2041 |
| decimal `sin`, `cos`, `atan2` | dfixpnt | decimal circular | planned, #2042 |
| `CORDICAddSub` | lns add/sub | hyperbolic, in `double` | #783 |

The fixpnt engines all go through one iteration, `cordic::detail::step<m>`, which is the unified
recurrence with `m` as a template parameter (`cordic::coordinates::circular`, `linear`, `hyperbolic`).
Rotation mode and vectoring mode are loops over it that choose `d_i`.

They are hardware models, not fast software math. The default fixpnt `sin`, `cos`, `tan`, `atan`,
`atan2`, `hypot`, `exp`, `log` and hyperbolic functions compute through `double` and serve software
experiments. The `cordic::` functions model a
shift-and-add unit bit for bit: each shift truncates, the datapath has a chosen number of guard bits, and
the unit stops after a chosen number of iterations. A designer can then measure the error of a unit
before building it. Both knobs are on every function:

```cpp
#include <universal/number/fixpnt/fixpnt.hpp>
using namespace sw::universal;

using F = fixpnt<32, 16, Saturate, std::uint32_t>;
F s, c;
cordic::sincos(F(0.5), s, c);         // defaults: within 1 ulp
F p = cordic::mul(F(3.25), F(-1.5));  // -4.875, by shift and add
F q = cordic::div(F(1), F(3));        // 0.33333, by shift and subtract
F q12 = cordic::div<4>(F(1), F(3), 12);   // a unit with 4 guard bits that stops after 12 iterations
F e = cordic::exp(F(1));              // 2.71828, by hyperbolic rotation
F l = cordic::log(F(10));             // 2.30259, by hyperbolic vectoring
F r = cordic::sqrt(F(2));             // 1.41421, from the vectoring gain
```

### The datapath

- The iterations run in a wider `Modulo` fixpnt, with `g` guard fraction bits and enough integer bits
  that nothing wraps. The result is rounded to nearest once, at the end. It then follows the target
  type's arithmetic: a result the type cannot hold saturates under `Saturate` and wraps under `Modulo`.
- **Circular.** Rotation has `rbits + g` fraction bits and reduces by `pi/2` when the type holds more
  than `[-1, 1)`. Vectoring has `nbits + 1 + g`, with the inputs scaled so the larger lies in `[1/2, 1)`.
  The default guard width is `g = ceil(log2(rbits + 8)) + 2`, and the default iteration count is
  `rbits + g`.
- **Linear.** Both operands are scaled into `[1/2, 1)`, so a product or quotient is as accurate for small
  operands as for large ones. The datapath has `B + g` fraction bits, where `B` is the number of bits the
  result can need:
  - `mul`: `B` is `nbits + 1` under `Saturate`, where a larger product saturates and is detected from the
    exponents, and `nbits + (nbits - rbits)` under `Modulo`, which needs every bit of a wrapped product.
  - `div`: `B` is `nbits` under `Saturate` and `nbits + rbits` under `Modulo`.

  The guard width is `ceil(log2(B + 8)) + 2` for `mul`, and one more for `div`, whose error is divided by
  `x >= 1/2`. The default iteration count is `B + g`. Division by zero follows fixpnt's `operator/`.
- **Hyperbolic.** The iterations start at shift 1 and repeat 4, 13, 40, ...; the iteration count counts
  the repeats, and the default is every iteration whose shift is at most `B + g`. One datapath serves
  the family, with `B = max(nbits, rbits + 2)` fraction bits: `exp` reaches `2^(ib-1)` and needs every
  bit of the type, and `atanh` needs `1 +- t` exactly. Its integer bits hold `ln(maxpos)` and
  `ln(minpos)`, and the guard width is `ceil(log2(B + 8)) + 3`. The arguments are reduced so the
  iterations always converge:
  - `exp`, `sinh`, `cosh`, `tanh`: `z = q ln 2 + r` with `|r| <= ln 2 / 2`, in a wider type whose
    `ln 2` carries the bits of `q`. `exp z = 2^q (cosh r + sinh r)`, and for `|z|`,
    `cosh, sinh = 2^(q-1) (e^r +- 4^-q e^-r)` with `e^(+-r) = cosh r +- sinh r`. `tanh` divides the
    two, so `2^(q-1)` cancels, by linear vectoring.
  - `log`: `w = m 2^e` with `m` in `[3/4, 3/2)`, so `ln w = e ln 2 + 2 atanh((m - 1) / (m + 1))`, and
    a `w` near 1 has `e = 0` and no cancellation.
  - `sqrt`: `w = m 4^k` with `m` in `[1/4, 1)`, from `(m + 1/4, m - 1/4, 0)`.
  - `atanh`: with `1 - |t| = 2^-e [1, 2)`, `atanh |t| = e ln 2 / 2 + atanh(y / x)` for
    `x, y = (1 + |t|) +- (1 - |t|) 2^e`, a ratio in `(-1/3, 1/3)`, for every `|t| < 1`.
- **`tan`** is circular rotation, then linear vectoring of the normalized `sin` and `cos`. Where `tan`
  still fits the type, `cos >= 2^-(ib-1)`, and the absolute error of `cos` is amplified by up to
  `1 / cos^2`, so its rotation datapath has `rbits + 2 (ib - 1) + g` fraction bits.
- **Overflow and the domain.** `exp`, `sinh`, `cosh` and `tan` are not bounded, and wrapping them
  exactly would need a datapath as wide as the result: `exp` of `fixpnt<32,0>`'s largest argument has
  about `3e9` integer bits. So a result beyond the type saturates in either arithmetic, as an overflow
  detect in hardware does. `sqrt` saturates as fixpnt's own `sqrt` does. `tanh`, `atanh` and `log` are
  bounded and follow the type's arithmetic. fixpnt has no NaN, so the domain errors follow `<cmath>`'s
  `errno`: `log(0)` and `atanh(+-1)` are poles (`ERANGE`, with `maxneg` or `maxpos`), `log` of a negative
  value and `atanh` beyond `+-1` are domain errors (`EDOM`, with 0), and `sqrt` of a negative value
  throws `fixpnt_negative_sqrt_arg` or sets `EDOM`, as fixpnt's `sqrt` does.
- **Constants.** The `atan(2^-i)` and `atanh(2^-i)` tables, `1/K_n` for every iteration count, `pi/2`
  and `ln 2 = 2 atanh(1/3)` come from series in fixpnt arithmetic 16 bits wider than the datapath. None of them goes through `double`, so the engines
  are correct at any width. The linear engine needs no table beyond the exact powers of two.

### Measured accuracy

Every result is within 1 ulp of the exact value, after the type's arithmetic. These are the worst cases
over every encoding (or every pair, for the 8-bit formats) and over samples of the wide formats:

| format | sin | cos | atan2 | hypot | mul | div |
|---|---|---|---|---|---|---|
| `fixpnt<8,4>` | 0.558 | 0.512 | 0.532 | 0.524 | 0.500 | 0.500 |
| `fixpnt<8,8>` | 0.554 | 0.537 | 0.565 | 0.524 | 0.523 | 0.496 |
| `fixpnt<12,8>` | 0.618 | 0.591 | 0.531 | 0.533 | 0.531 | 0.500 |
| `fixpnt<16,12>` | 0.565 | 0.566 | 0.515 | 0.521 | 0.524 | 0.500 |
| `fixpnt<32,16>` | 0.564 | 0.546 | 0.514 | 0.513 | 0.524 | 0.500 |
| `fixpnt<64,32>` | 0.533 | 0.516 | 0.503 | 0.503 | 0.518 | 0.503 |
| `fixpnt<128,120>` | 0.496 | 0.528 | 0.503 | 0.515 | 0.500 | 0.500 |

For the hyperbolic engine and `tan`, over every encoding up to 16 bits and samples beyond (`-` marks a
function whose every result saturates in that format, such as `cosh >= 1` in `fixpnt<8,8>`):

| format | exp | sinh | cosh | tanh | atanh | log | sqrt | tan |
|---|---|---|---|---|---|---|---|---|
| `fixpnt<8,4>` | 0.495 | 0.489 | 0.497 | 0.491 | 0.494 | 0.499 | 0.496 | 0.495 |
| `fixpnt<8,8>` | - | 0.497 | - | 0.494 | 0.495 | 0.493 | 0.490 | 0.535 |
| `fixpnt<12,8>` | 0.500 | 0.503 | 0.507 | 0.500 | 0.495 | 0.500 | 0.509 | 0.502 |
| `fixpnt<16,12>` | 0.519 | 0.511 | 0.508 | 0.502 | 0.500 | 0.502 | 0.513 | 0.512 |
| `fixpnt<16,15>` | 0.507 | 0.510 | - | 0.507 | 0.504 | 0.510 | 0.515 | 0.566 |
| `fixpnt<32,16>` | 0.501 | 0.500 | 0.502 | 0.500 | 0.500 | 0.500 | 0.500 | 0.500 |
| `fixpnt<64,32>` | 0.498 | 0.498 | 0.498 | 0.497 | 0.498 | 0.497 | 0.497 | 0.500 |
| `fixpnt<64,60>` | 0.501 | 0.500 | 0.500 | 0.500 | 0.500 | 0.500 | 0.502 | 0.500 |
| `fixpnt<128,120>` | 0.499 | 0.500 | 0.500 | 0.500 | 0.500 | 0.500 | 0.500 | 0.500 |

`sinh` near 0 and `log` near 1 lose relative accuracy to cancellation in a floating-point format, but a
fixed-point ulp is absolute, and they stay within the same bound: `sinh` of a small `z` is the `y` of the
rotation with no reduction, and `log` of a `w` near 1 has `e = 0`.

The references are independent of the engines. For the circular and hyperbolic engines they are `double`
(up to 32 bits) or `qd`. For the linear engine they are exact rational results in `integer<>` arithmetic, so the
error is measured exactly, even for wrapped 128-bit products.

### The hardware knobs

In `fixpnt<16,12,Saturate>`, the error halves with each iteration until the guard bits run out:

| iterations | `sin` | `mul` | `div` |
|---|---|---|---|
| 4 | 509 | 6140 | 4090 |
| 8 | 32.1 | 486 | 254 |
| 12 | 2.4 | 29.6 | 15.7 |
| 16 | - | 2.33 | 1.00 |
| 20 | - | 0.605 | 0.498 |
| default | 0.565 (19) | 0.513 (24) | 0.498 (24) |

The linear engine needs more iterations than the circular one for the same format. It works on
normalized operands, so its error is relative to the operands, and the result can have up to
`nbits + 1` significant bits.

The guard width sets the floor that more iterations cannot pass:

| guard bits | `sin` | `mul` | `div` |
|---|---|---|---|
| 0 | 7.92 | 4.61 | 3.17 |
| 2 | 3.08 | 1.38 | 1.02 |
| 4 | 1.09 | 0.627 | 0.565 |
| default | 0.565 (7) | 0.513 (7) | 0.498 (8) |

The hyperbolic engine in the same format, whose default is 26 iterations (the shifts 1 to 24, with 4 and
13 repeated, over 8 guard bits):

| iterations | `exp` | `log` | `sqrt` | `tanh` |
|---|---|---|---|---|
| 4 | 3280 | 866 | 46.0 | 658 |
| 8 | 252 | 64.8 | 0.838 | 61.1 |
| 12 | 16.3 | 4.80 | 0.502 | 3.88 |
| 16 | 2.44 | 0.989 | 0.508 | 0.719 |
| 20 | 0.607 | 0.530 | 0.508 | 0.511 |
| default | 0.519 (26) | 0.502 (26) | 0.513 (26) | 0.502 (26) |

| guard bits | `exp` | `log` | `sqrt` | `tanh` |
|---|---|---|---|---|
| 0 | 5.76 | 1.30 | 4.14 | 1.15 |
| 2 | 1.77 | 0.643 | 1.38 | 0.703 |
| 4 | 0.888 | 0.537 | 0.715 | 0.541 |
| default | 0.519 (8) | 0.502 (8) | 0.513 (8) | 0.502 (8) |

`exp` needs the most: its result reaches `2^(ib-1)`, so the angle error is multiplied by up to 8 in
`fixpnt<16,12>`. `sqrt` needs the fewest. It reads the length of the vector, not its angle, and an
angle `eps` left over changes the length only by `cosh(eps) - 1 ~= eps^2 / 2`: the length converges
twice as fast as the angle, and 8 iterations already give `sqrt` to within 1 ulp. A repeated iteration
(the fifth, with shift 4 again) does not halve the error as the others do.

## References

- J. E. Volder, "Binary computation algorithms for coordinate rotation and function generation", Convair
  report IAR-1.148, 1956.
- J. E. Volder, "The CORDIC Trigonometric Computing Technique", *IRE Transactions on Electronic
  Computers*, EC-8(3), 330-334, 1959.
- D. H. Daggett, "Decimal-Binary Conversions in CORDIC", *IRE Transactions on Electronic Computers*,
  EC-8(3), 335-339, 1959.
- J. E. Meggitt, "Pseudo Division and Pseudo Multiplication Processes", *IBM Journal of Research and
  Development*, 6(2), 210-226, 1962.
- J. S. Walther, "A Unified Algorithm for Elementary Functions", *AFIPS Spring Joint Computer
  Conference*, 38, 379-385, 1971.
- D. S. Cochran, "Algorithms and Accuracy in the HP-35", *Hewlett-Packard Journal*, June 1972.
- H. M. Ahmed, J.-M. Delosme and M. Morf, "Highly Concurrent Computing Structures for Matrix Arithmetic
  and Signal Processing", *IEEE Computer*, 15(1), 65-82, 1982.
- R. Nave, "Implementation of Transcendental Functions on a Numerics Processor", *Microprocessing and
  Microprogramming*, 11, 221-225, 1983.
- M. D. Ercegovac and T. Lang, "Redundant and On-Line CORDIC: Application to Matrix Triangularization",
  *IEEE Transactions on Computers*, 39(6), 725-740, 1990.
- N. Takagi, T. Asada and S. Yajima, "Redundant CORDIC Methods with a Constant Scale Factor for Sine and
  Cosine Computation", *IEEE Transactions on Computers*, 40(9), 989-995, 1991.
- Y. H. Hu, "CORDIC-Based VLSI Architectures for Digital Signal Processing", *IEEE Signal Processing
  Magazine*, 9(3), 16-35, 1992.
- R. Andraka, "A Survey of CORDIC Algorithms for FPGA Based Computers", *ACM/SIGDA FPGA '98*, 191-200,
  1998.
- J. E. Volder, "The Birth of CORDIC", *Journal of VLSI Signal Processing*, 25, 101-105, 2000.
- J. S. Walther, "The Story of Unified CORDIC", *Journal of VLSI Signal Processing*, 25, 107-112, 2000.
- P. K. Meher, J. Valls, T.-B. Juang, K. Sridharan and K. Maharatna, "50 Years of CORDIC: Algorithms,
  Architectures, and Applications", *IEEE Transactions on Circuits and Systems I*, 56(9), 1893-1907, 2009.
- J.-M. Muller, *Elementary Functions: Algorithms and Implementation*, Birkhauser, 3rd edition, 2016.
