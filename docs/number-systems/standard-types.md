# Standard IEEE-754 Types: quarter, half, single, duble, quad, octo

## Why

`cfloat` can express any floating-point format you can describe. That power is also the
problem: a reader who wants "just an IEEE single" has to know that it spells as

```cpp
cfloat<32, 8, std::uint32_t, true, false, false>
```

and has to be confident about all six parameters -- that `true` is subnormals, that the
first `false` is supernormals, that the second `false` is saturating arithmetic. Get one
wrong and you have silently built a format that is *not* the one the hardware implements,
and your comparison against native `float` is measuring your typo.

Universal therefore predefines the IEEE-754 interchange formats under their ordinary
names. These are the formats a compiler, a GPU, or an FPGA soft core is most likely to
implement, so they are the ones worth reaching for first:

```cpp
#include <universal/number/cfloat/cfloat.hpp>
using namespace sw::universal;

single x = 1.0f;   // exactly IEEE-754 binary32
duble  y = 1.0;    // exactly IEEE-754 binary64
quad   z = 1.0;    // exactly IEEE-754 binary128, no hardware required
```

Each type has two spellings: a word name (`single`) and an `fpNN` name (`fp32`). They are
the same type -- `using fp32 = single;` -- so pick whichever reads better in your code.
The `fpNN` form is usually clearer in mixed-precision work where the bit width is the
point.

## What

| type | alias | nbits | es | fbits | exponent range | maxpos | epsilon |
|---|---|---|---|---|---|---|---|
| `quarter` | `fp8` | 8 | 2 | 5 | 1 .. 2 | 3.938e+00 | 3.125e-02 |
| `half` | `fp16` | 16 | 5 | 10 | -13 .. 16 | 6.550e+04 | 9.766e-04 |
| `single` | `fp32` | 32 | 8 | 23 | -125 .. 128 | 3.403e+38 | 1.192e-07 |
| `duble` | `fp64` | 64 | 11 | 52 | -1021 .. 1024 | 1.798e+308 | 2.220e-16 |
| `xtndd` | `fp80` | 80 | 11 | 68 | -1021 .. 1024 | 1.798e+308 | 3.388e-21 |
| `quad` | `fp128` | 128 | 15 | 112 | -16381 .. 16384 | 1.190e+4932 | 1.926e-34 |
| `octo` | `fp256` | 256 | 19 | 236 | -262141 .. 262144 | 3.648e+78841 | 9.056e-72 |

All values measured from the types themselves, not from a `double` round-trip -- `quad`
and `octo` exceed `double`'s range, so printing them through one reports `inf`.

`half`, `single`, `duble`, `quad` and `octo` match the IEEE-754 interchange formats
binary16, binary32, binary64, binary128 and binary256 exactly, field for field. All six
enable subnormals and disable supernormals and saturating arithmetic, which is what
IEEE-754 specifies.

### Why `duble` and not `double`

`double` is a C++ keyword. The library needs a name for "the Universal type that is
IEEE-754 binary64" that is distinct from the built-in, and `duble` is that name. If the
spelling bothers you, `fp64` is the same type.

### Two names that need a caveat

**`quarter` is not the 8-bit format your ML framework means.** It is the IEEE-754
interchange rule extended down to 8 bits: 2 exponent bits, 5 fraction bits, dynamic range
topping out below 4. That makes it a faithful small IEEE format and a poor neural-network
format. The formats deep-learning toolchains actually use are in
[`microfloat`](/universal/number-systems/microfloat/):

```cpp
#include <universal/number/microfloat/microfloat.hpp>
e4m3 a;   // OCP OFP8 E4M3 -- NaN on overflow
e5m2 b;   // OCP OFP8 E5M2 -- wider range, less precision
```

**`xtndd` is not the x87 80-bit format.** x87 extended precision has 15 exponent bits;
`xtndd` has 11, which gives it `double`'s dynamic range in an 80-bit container -- its extra
bits all buy precision and none buy range, so it overflows to `inf` everywhere above
1.8e308 that a real x87 `long double` is perfectly happy. Tracked in
[#1599](https://github.com/stillwater-sc/universal/issues/1599). Until it is settled, do
not reach for `fp80` expecting to model x87.

## Using them

They are ordinary Universal types, so everything the library offers applies:

```cpp
#include <universal/number/cfloat/cfloat.hpp>
#include <iostream>

using namespace sw::universal;

template<typename Real>
Real dot(const std::vector<Real>& a, const std::vector<Real>& b) {
    Real sum{0};
    for (size_t i = 0; i < a.size(); ++i) sum += a[i] * b[i];
    return sum;
}

int main() {
    // the same kernel, four precisions, no code changes
    std::cout << dot<half>  ({1.0, 2.0}, {3.0, 4.0}) << '\n';
    std::cout << dot<single>({1.0, 2.0}, {3.0, 4.0}) << '\n';
    std::cout << dot<duble> ({1.0, 2.0}, {3.0, 4.0}) << '\n';
    std::cout << dot<quad>  ({1.0, 2.0}, {3.0, 4.0}) << '\n';
}
```

Bit-level inspection works the same way across all of them, which is the usual reason to
reach for a Universal type over the native one:

```cpp
single x = 0.1f;
std::cout << to_binary(x) << '\n';     // sign|exponent|fraction, field-separated
std::cout << color_print(x) << '\n';   // the same, coloured by field
std::cout << scale(x) << '\n';         // the unbiased exponent
```

### Exact dot products

Every one of these types has quire support, so you can accumulate a dot product with a
single rounding at the end:

```cpp
#include <universal/number/cfloat/fdp.hpp>

std::vector<single> y = { 1.0f, 1.0f, 1.0f, 1.0f };

// the same four terms, three orderings
dot({1e8f, -1e8f, 0.5f, 0.25f}, y);   // 0.75  -- happens to be right
dot({1e8f, 0.5f, 0.25f, -1e8f}, y);   // 0.0   -- both small terms lost
dot({1e8f, 0.5f, -1e8f, 0.25f}, y);   // 0.25  -- one small term lost

fdp(...)                              // 0.75 for every ordering
```

That is the real hazard, and it is worse than a single wrong answer: the naive sum is
*order-dependent*, so the same data summed in a different order gives a different result,
and neither is flagged. The quire accumulates the products exactly and rounds once, so
`fdp` returns 0.75 whatever order the terms arrive in.

See [Exact Arithmetic](/universal/exact-arithmetic/) for what the quire is doing.

## Choosing a type

| If you want | reach for |
|---|---|
| A drop-in for native `float` / `double`, with bit inspection | `single` / `duble` |
| Precision beyond `double`, on any hardware | `quad`, `octo` |
| Precision beyond `double`, as fast as possible | [`dd`](/universal/number-systems/dd/), [`qd`](/universal/number-systems/qd/) |
| Precision that grows until the answer converges | [`ereal`](/universal/number-systems/ereal/), [`elreal`](/universal/number-systems/elreal/) |
| A storage format for neural network weights | [`microfloat`](/universal/number-systems/microfloat/), [`bfloat16`](/universal/number-systems/bfloat16/) |
| A format nobody has standardised yet | [`cfloat`](/universal/number-systems/cfloat/) directly |

`quad` and `octo` are software, so they are far slower than `dd` and `qd` at comparable
precision. Choose them when you need a *standard* format -- an interchange file, a
reference answer, a conformance test -- and choose `dd`/`qd` when you need the digits and
do not care about the encoding.

## Decimal digits

`digits10` and `max_digits10` answer two different questions, and mixing them up is the
usual source of a round-trip bug:

| trait | question it answers |
|---|---|
| `digits` | significand bits, including the implicit leading bit |
| `digits10` | decimal digits that survive a round trip **into** the format |
| `max_digits10` | decimal digits needed to round trip **out of** and back in without loss |

| type | `digits` | `digits10` | `max_digits10` |
|---|---|---|---|
| `half` | 11 | 3 | 5 |
| `single` | 24 | 6 | 9 |
| `duble` | 53 | 15 | 17 |
| `quad` | 113 | 33 | 36 |
| `octo` | 237 | 71 | 73 |

These match what native `float` and `double` report for the same formats, which is the
property you want when substituting `single` for `float`. If you are printing a value and
expect to read it back unchanged, use `max_digits10` -- `digits10` is the smaller number
and will silently lose the last place.

```cpp
std::cout << std::setprecision(std::numeric_limits<single>::max_digits10) << x;
```

They were computed from a `digits / 3.3` approximation until
[#1597](https://github.com/stillwater-sc/universal/issues/1597), which reported 7 digits
for a binary32 where the standard says 6.
