# Classic Floating-Point (cfloat): Configurable IEEE-754 Compatible Arithmetic

## Why

IEEE-754 defines a handful of fixed formats: half (16-bit), single (32-bit), double (64-bit), and quad (128-bit). But modern workloads -- especially deep learning, DSP, and mixed-precision HPC -- need floating-point formats that don't exist in the standard: 8-bit floats for inference, 24-bit floats for AMD GPUs, formats with max-exponent value numbers for gradual overflow, or saturating arithmetic instead of infinity.

The Universal `cfloat` type is a fully parameterized floating-point that can emulate *any* IEEE-754 format and extend beyond it. You configure the total bit width, exponent size, subnormal behavior, max-exponent value behavior, and overflow semantics at compile time. The result is a type that behaves exactly like hardware floating-point but with the precision and range *you* choose.

## What

`cfloat<nbits, es, bt, hasSubnormals, hasMaxExpValues, isSaturating>` is a configurable floating-point:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `nbits` | `unsigned` | -- | Total bits (4 to 256+) |
| `es` | `unsigned` | -- | Exponent bits |
| `bt` | typename | `uint8_t` | Storage block type |
| `hasSubnormals` | `bool` | -- | Enable gradual underflow |
| `hasMaxExpValues` | `bool` | -- | Reclaim max-exponent encodings as numeric values |
| `isSaturating` | `bool` | -- | Overflow saturates instead of producing infinity |

### Encoding

Standard sign-exponent-fraction layout:

```
[sign : 1 bit] [exponent : es bits] [fraction : nbits - 1 - es bits]
```

- Exponent bias: `2^(es-1) - 1`
- Hidden bit: 1 for normal numbers, 0 for subnormals
- Infinity: exponent all-1s, fraction all-0s (when `hasMaxExpValues` is false), as in IEEE-754
- NaN: exponent all-1s, fraction non-zero (when `hasMaxExpValues` is false), as in IEEE-754
- When `hasMaxExpValues`: the all-1s exponent binade encodes numeric values; only 4 encodings are reserved: +-inf is `s.11...11.11...10` and NaN is `s.11...11.11...11`
- When `isSaturating`: overflow produces maxpos/maxneg instead of infinity. With `hasMaxExpValues` as well there is no infinity at all: `s.11...11.11...10` is maxpos/maxneg (the OCP E4M3FN layout)
- Quiet and signalling NaN: without `hasMaxExpValues` the fraction MSB is the quiet bit, as in IEEE-754 (#1896): set is quiet, clear is signalling, whatever the sign. `setnan` writes `0.11...11.10...00` (quiet) and `0.11...11.01...00` (signalling). With `hasMaxExpValues`, and in the corner case of a one-bit fraction, there are only two NaN patterns and the sign is the type: `0.11...11.11...11` is quiet, `1.11...11.11...11` is signalling
- Invalid operations (`inf - inf`, `0 * inf`, `0 / 0`, `sqrt(-x)`) deliver a quiet NaN
- An operation on a NaN delivers a quiet NaN, a signalling NaN operand included (IEEE-754 6.2, #1899): `+ - * /`, `fdp`, `sqrt`, `exp`, `exp2`, `trunc`, `floor`, `ceil`, `round`, `ldexp`, `frexp`. With `CFLOAT_THROW_ARITHMETIC_EXCEPTION` a signalling NaN operand of `+ - * /` throws instead. Negation, `abs` and conversions keep a signalling NaN signalling where the encoding allows it; with sign-typed NaNs (max-exponent values) negation and `abs` change the sign and with it the type. cfloat has no status flags, so with exceptions off the invalid operation leaves no other trace
- `min`/`max` are IEEE-754-2019 `minimum`/`maximum` (#1901): a NaN operand, in either position, gives a quiet NaN, and `-0` orders below `+0`. `fmin`/`fmax` are C `fmin`/`fmax` (C23 `fminimum_num`, IEEE-754-2019 `minimumNumber`): a NaN operand, quiet or signalling, is ignored and the other operand is returned; two NaN operands give a quiet NaN. All four compare cfloat values directly and return an operand unchanged, so they are exact for `quad` and wider
- `nextafter(x, t)` and `nexttoward(x, t)` return a quiet NaN when either operand is a NaN, step `-0` like `+0` (`nextafter(-0, 1)` is `+minpos`), and set `errno` to `ERANGE` when a finite `x` steps to infinity. `nexttoward` steps by one ulp of `x`'s own type; its `cfloat<128,15>` target only sets the direction. `++` and `--` step `-0` the same way as `+0`

One rule covers every configuration: infinity is the encoding right after maxpos, and every NaN sits above it. Subnormals do not affect the all-1s exponent binade, so the eight configurations have three layouts there. For `cfloat<8,4>`, positive half of the all-1s exponent binade:

| configuration | `0x78` .. `0x7D` | `0x7E` | `0x7F` | maxpos |
|---|---|---|---|---|
| `hasMaxExpValues` false | `0x78` inf, `0x79`..`0x7B` sNaN, `0x7C`..`0x7D` qNaN | qNaN | qNaN | `0x77` = 240 |
| `hasMaxExpValues` | 256 .. 416 | inf | NaN (quiet) | `0x7D` = 416 |
| `hasMaxExpValues`, `isSaturating` | 256 .. 416 | 448 | NaN (quiet) | `0x7E` = 448 |

So `half`, `single`, `duble`, `quad` and the other configurations without max-exponent values are bit-compatible with IEEE-754 for values and infinities (#1780), and classify every IEEE-754 NaN pattern as NaN of the same type, quiet or signalling (#1896). The NaN payload and sign are not carried: converting a native NaN, or setting one with `setnan`, writes the canonical pattern of its type. A cfloat quiet NaN is the native `quiet_NaN()` pattern.

### Standard IEEE-754 Aliases

```cpp
using quarter   = cfloat<8, 2, uint8_t, true, false, false>;     // 8-bit, IEEE-754 style
using half      = cfloat<16, 5, uint16_t, true, false, false>;   // IEEE-754 binary16
using single    = cfloat<32, 8, uint32_t, true, false, false>;   // IEEE-754 binary32
using duble     = cfloat<64, 11, uint64_t, true, false, false>;  // IEEE-754 binary64
using quad      = cfloat<128, 15, uint64_t, true, false, false>; // IEEE-754 binary128
using octo      = cfloat<256, 19, uint64_t, true, false, false>; // IEEE-754 binary256
```

### FP8 Aliases

```cpp
using fp8e2m5   = cfloat<8, 2, uint8_t, true, true, false>;
using fp8e3m4   = cfloat<8, 3, uint8_t, true, true, false>;
using fp8e4m3   = cfloat<8, 4, uint8_t, true, true, false>;
using fp8e5m2   = cfloat<8, 5, uint8_t, true, true, false>;
```

These enable subnormals and max-exponent values, to get the most numbers out of 256 encodings. They are **not** the OCP OFP8 formats (#1903): the all-1s exponent binade holds numbers, so maxpos is larger and inf and NaN sit elsewhere.

| alias | maxpos | +inf | OCP format | OCP maxpos | OCP +inf | OCP overflow |
|---|---|---|---|---|---|---|
| `fp8e4m3` | 416 (`0x7D`) | `0x7E` | E4M3 (`e4m3fn`) | 448 (`0x7E`) | none | NaN (NONSAT) or maxpos (SAT) |
| `fp8e5m2` | 81920 (`0x7D`) | `0x7E` | E5M2 | 57344 (`0x7B`) | `0x7C` | inf (NONSAT) or maxpos (SAT) |

The OCP specification has two overflow modes for conversion: NONSAT and SAT. The OCP encodings are available in two places:

| OCP format | encoding-identical type | overflow |
|---|---|---|
| E4M3 | `microfloat` `e4m3` | NaN (NONSAT) |
| E4M3 | `cfloat<8, 4, uint8_t, true, true, true>` | maxpos (SAT) |
| E5M2 | `microfloat` `e5m2` | inf (NONSAT) |
| E5M2 | `cfloat<8, 5, uint8_t, true, false, false>` | inf (NONSAT) |

Each of these matches the OCP encoding on all 256 patterns. The saturating cfloat also saturates arithmetic results, not only conversions.

#### How OCP spends the top binade, and why

The top binade (all-1s exponent) holds `2^m` encodings per sign: 8 for E4M3, 4 for E5M2. The two OCP formats treat it differently, and neither gives up as much of it as the IEEE layout would. The rationale is in section 3 of Micikevicius et al., [*FP8 Formats for Deep Learning*](https://arxiv.org/abs/2209.05433) (NVIDIA, Arm, Intel, 2022), the proposal the OCP OFP8 specification is based on. Its stated principle is to stay consistent with IEEE-754 unless deviating gives a significant benefit for deep-learning accuracy.

**E4M3 reclaims 7 of the 8 encodings.** It has no infinity and keeps a single NaN pattern, `S.1111.111`. The other seven become the magnitudes 256, 288, ..., 448. That extends the range from 17 to 18 binades: maxpos is 448 instead of the IEEE-style 240. With only 4 exponent bits, the paper judges the extra range much more useful than multiple special encodings. It stops at 7: a single unsigned NaN and zero would free one more value (480), but would break the +/- symmetry that lets IEEE values be compared and sorted as integers. The paper judges that 448 -> 480 is not worth that.

**E5M2 reclaims none of the 4.** It keeps IEEE infinity and NaN at `S.11111.00` and `S.11111.{01,10,11}`. The paper gives three reasons:
- **FP16 interchange.** E5M2 is IEEE half precision with fewer mantissa bits, as bfloat16 is single precision with fewer. Converting between E5M2 and FP16 is just rounding the mantissa, including the special values.
- **Little to gain.** Reclaiming the binade would add only 3 magnitudes, one binade on top of the 32 that E5M2 already spans, subnormals included. The paper's experiments found 5 exponent bits give enough range per tensor. For E4M3 the same move was one binade on 17.
- **Overflow handling.** E5M2 is the recommended gradient format. Training that mixes FP8 and FP16 relies on special values, because FP16 automatic mixed precision adjusts its loss scale by causing and detecting overflows (section 2).

So the values OCP gives up are few. E4M3 loses 2 of 256 encodings, its two NaNs. E5M2 loses 8: 2 infinities and 6 NaNs. In E4M3 the paper chose more values over IEEE conformance. In E5M2 the values on offer were worth less than FP16 compatibility and inf/NaN for overflow detection.

The cfloat FP8 aliases make a third choice. `hasMaxExpValues` reclaims every top-binade encoding except one infinity and one NaN per sign:

| | E4M3 values in the top binade | maxpos | E5M2 values in the top binade | maxpos |
|---|---|---|---|---|
| IEEE-style (`hasMaxExpValues = false`) | 0 | 240 | 0 | 57344 (OCP E5M2) |
| cfloat `fp8eXmY` (`hasMaxExpValues = true`) | 6 | 416 | 2 | 81920 |
| OCP | 7 | 448 (E4M3) | 0 | 57344 |

For E4M3 the cfloat alias has one value fewer than OCP, because it also keeps an infinity. For E5M2 it has two values more, at the cost of no longer being FP16 with a shorter mantissa.

### Deep Learning Format Aliases

```cpp
using bfloat_t = cfloat<16, 8, uint16_t, true, false, false>;    // Google Brain float
using msfp8    = cfloat<8, 2, uint8_t, false, false, false>;     // Microsoft FP8
using amd24    = cfloat<24, 8, uint32_t, true, false, false>;    // AMD 24-bit
```

## How It Works

Arithmetic is performed using a `blocktriple` intermediate representation that carries sufficient precision to compute exact products and sums before rounding. The pipeline is:

1. **Decode** operands into blocktriple (sign, scale, significant)
2. **Compute** in extended precision (the blocktriple is wider than the target)
3. **Round** using round-to-nearest-even (or configurable rounding mode)
4. **Encode** back into cfloat format

This mirrors how hardware FPUs work internally but is fully parameterized at the template level.

## How to Use It

### Include

```cpp
#include <universal/number/cfloat/cfloat.hpp>
using namespace sw::universal;
```

### Custom 8-bit Float for Deep Learning

```cpp
// FP8 with 4 exponent bits, 3 fraction bits, saturating
using fp8 = cfloat<8, 4, uint8_t, true, false, true>;

fp8 weight(0.5f);
fp8 activation(0.75f);
fp8 result = weight * activation;  // Saturates on overflow, no infinity

// Explore the encoding
std::cout << to_binary(result) << " = " << result << std::endl;
std::cout << "maxpos: " << fp8::maxpos() << std::endl;
std::cout << "minpos: " << fp8::minpos() << std::endl;
```

### Mixed-Precision Algorithm Development

```cpp
template<typename HighPrec, typename LowPrec>
HighPrec mixed_precision_dot(const std::vector<LowPrec>& a,
                              const std::vector<LowPrec>& b) {
    HighPrec sum(0);
    for (size_t i = 0; i < a.size(); ++i) {
        sum += HighPrec(a[i]) * HighPrec(b[i]);  // accumulate in high precision
    }
    return sum;
}

using FP8  = cfloat<8, 4, uint8_t, true, false, false>;
using FP32 = cfloat<32, 8, uint32_t, true, false, false>;

std::vector<FP8> weights = { FP8(0.5), FP8(0.25), FP8(0.125) };
std::vector<FP8> inputs  = { FP8(1.0), FP8(2.0), FP8(3.0) };
FP32 result = mixed_precision_dot<FP32>(weights, inputs);
```

### Full encoding efficiency with hasMaxExpValues

```cpp
// IEEE-754 configuration
constexpr bool hasSubnormals   = true;
constexpr bool hasMaxExpValues = false;
constexpr bool isSaturating    = false;
// cfloat<nbits, es, BlockType, hasSubnormals, hasMaxExpValues, isSaturating>;

// Standard: subnormals, overflow -> infinity, last binade used for special encodings: inf and NaN
using Standard = cfloat<8, 4>; // equivalent to cfloat<8, 4, uint8_t, true, false, false>;

// Expanded: subnormals, overflow -> infinity, last binade encodes values extending normal range
using Expanded = cfloat<8, 4, uint8_t, true, true, false>;

// Saturating: subnormals, no overflow, last binade encodes values extending normal range
using Saturated = cfloat<8, 4, uint8_t, true, true, true>;

Standard  s(200.0f);  // may produce infinity
Expanded  e(200.0f);  // uses max-exponent value encoding, can still produce infinite
Saturated c(200.0f);  // uses max-exponent value encoding, stays finite
```

## Problems It Solves

| Problem | How cfloat Solves It |
|---------|-----------------------|
| Need 8-bit float for ML inference but IEEE has no 8-bit format | `cfloat<8, es>` with configurable exponent width |
| Infinity corrupts neural network training | `isSaturating=true` clamps to maxpos |
| Exploring precision/range trade-offs for new hardware | Any combination of nbits and es |
| Need IEEE-754 semantics in software for testing | Exact emulation of half/single/double/quad |
| Custom float for FPGA/ASIC design exploration | Parameterized at compile time, matches hardware encoding |
| Subnormal handling is too slow on some hardware | `hasSubnormals=false` to disable gradual underflow |
| Need wider-than-quad precision (128-bit+) | `cfloat<256, 19>` for octuple precision |
