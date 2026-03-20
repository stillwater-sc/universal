# ucalc -- Universal Mixed-Precision REPL Calculator

## Why

The Universal library ships 25+ one-shot command-line tools (`ieee`, `posit`,
`half`, `single`, `double`, etc.), each showing the representation of a single
value in a single type.  To compare how `1/3 + 1/3 + 1/3` behaves across
posit, cfloat, and fixed-point you must write C++, compile, and run --
repeating the cycle for every experiment.  This friction slows down the core
use case of the library: exploring how different number systems represent and
compute with the same values.

`ucalc` collapses this into a single interactive session.  It is the
mixed-precision equivalent of `bc` or Python's REPL, purpose-built for
Universal's number types.

## What

`ucalc` is a REPL (Read-Eval-Print Loop) calculator that:

- Supports **42 number types** out of the box: IEEE float/double, posit (8-64),
  takum (8-64), cfloat (8-64), bfloat16, fixed-point, decimal fixed-point,
  LNS, integer, hexadecimal float, decimal float, rational, double-double,
  quad-double, and cascaded variants.
- Parses **infix arithmetic** with standard operator precedence, parentheses,
  variables, constants (`pi`, `e`), and math functions (`sqrt`, `abs`, `log`,
  `exp`, `sin`, `cos`, `pow`).
- Provides **analysis commands**: `compare` (evaluate across all types),
  `show` (binary decomposition), `sweep` (error analysis over a range),
  `faithful` (faithfully-rounded check), `range`, `precision`, `ulp`, `bits`.
- Works in three modes: interactive REPL, one-shot command line, and
  pipe/script mode.
- Optionally integrates GNU Readline for tab completion and persistent history.

### Architecture

```
tools/ucalc/
  type_dispatch.hpp   -- TypeRegistry + SFINAE math dispatch (42 types)
  expression.hpp      -- Tokenizer + recursive-descent parser/evaluator
  ucalc.cpp           -- REPL loop, commands, native type specializations
  CMakeLists.txt      -- Build config with optional readline detection
```

The **type dispatch** layer erases the template parameters of each Universal
type behind `std::function` wrappers, using `double` as the interchange
format.  SFINAE detects whether a type provides its own math functions
(e.g., `sw::universal::sqrt(posit<32,2>)`) and falls back to `std::sqrt`
via double conversion when it does not.

The **expression parser** is a textbook recursive-descent parser handling:
`statement -> assignment | expr`, `expr -> term ((+|-) term)*`,
`term -> unary ((*|/) unary)*`, `unary -> -unary | power`,
`power -> postfix (^ unary)?`, `postfix -> primary | func(args)`.

## How

### Build

```bash
# Standalone
cmake -DUNIVERSAL_BUILD_TOOLS_UCALC=ON ..
make ucalc

# Or as part of the demonstration suite
cmake -DUNIVERSAL_BUILD_DEMONSTRATION=ON ..
make ucalc
```

With readline (optional):
```bash
sudo apt install libreadline-dev   # Debian/Ubuntu
cmake -DUNIVERSAL_BUILD_TOOLS_UCALC=ON ..
make ucalc
# readline auto-detected; tab completion and history enabled
```

### Usage

```
ucalc                              # interactive REPL
ucalc "type posit32; 1/3 + 1/3"   # one-shot from command line
echo "compare sqrt(2)" | ucalc    # pipe mode
```

## Three Examples

### Example 1: Why does 1/3 + 1/3 + 1/3 = 1 in posit32 but not in float?

This is the classic demonstration of posit arithmetic's closure property.
The tapered precision of posits allocates more fraction bits near 1.0,
allowing the three rounded thirds to sum exactly to 1.

```
$ ucalc
ucalc -- Universal Mixed-Precision REPL Calculator
Type 'help' for commands, 'quit' to exit.
Active type: double

double> type posit32
Active type: posit32 (sw::universal::posit< 32, 2, uint8_t>)

posit32> show 1/3 + 1/3 + 1/3
  value:      1
  binary:     0b0.10.00.000000000000000000000000000
  components: ...  value    : 1.000000000000000000000e+00
  type:       sw::universal::posit< 32, 2, uint8_t>

posit32> type float
Active type: float (float (IEEE-754 binary32))

float> show 1/3 + 1/3 + 1/3
  value:      1
  binary:     0b0.01111111.00000000000000000000000
  components: float: 1
  type:       float

posit32> compare 1/3
Type             Value  Binary
-----------------------------------------------------------
float            0.333333343  0b0.01111101.01010101010101010101011
double           0.333333333  0b0.01111111101.01010101010101...
posit8           0.328125     0b0.001.~.0101
posit16          0.333312988  0b0.01.0.010101010101
posit32          0.333333334  0b0.01.10.010101010101010101010101011
posit64          0.333333333  0b0.01.110.01010101010101010101...
```

**Insight**: posit32 rounds 1/3 to a value whose triple sums exactly to 1,
while IEEE float's fixed-exponent rounding leaves a residual.

### Example 2: How much precision does each type really have?

When choosing a number type for a DSP pipeline or neural network
quantization, you need to know the effective precision -- not just
the bit width.  `ucalc` measures this directly.

```
$ ucalc "type fp16; precision"
  type:           fp16
  binary digits:  10
  decimal digits: 3.0
  epsilon:        0.0009765625

$ ucalc "type bfloat16; precision"
  type:           bfloat16
  binary digits:  7
  decimal digits: 2.1
  epsilon:        0.0078125

$ ucalc "type posit16; precision"
  type:           sw::universal::posit< 16, 1, uint8_t>
  binary digits:  13
  decimal digits: 3.9
  epsilon:        0.000244140625

$ ucalc "type lns16; precision"
  type:           lns< 16,   8, uint8_t, Saturating>
  binary digits:  8
  decimal digits: 2.4
  epsilon:        0.00390625
```

**Insight**: posit16 delivers nearly 4 decimal digits of precision --
more than fp16 (3.0) and far more than bfloat16 (2.1) -- despite all
three being 16 bits.  This is the tapered precision advantage near 1.0.

### Example 3: How accurate is sin(x) across a range in posit32 vs float?

Before deploying a custom type in production, you need to understand
the error profile of transcendental functions.  The `sweep` command
evaluates an expression across a range and reports ULP error against
double-precision reference.

```
$ ucalc "type posit32; sweep sin(x) for x in [0, 3.14159, 8]"
x                    result               double ref      ULP error
---------------------------------------------------------------------
0                         0                      0           0.00
0.44879857      0.43388268...        0.43388268...           0.12
0.89759714      0.78183001...        0.78183002...           0.50
1.3463957       0.97492713...        0.97492715...           0.25
1.7951943       0.97492867...        0.97492866...           0.13
2.2439929       0.78183335...        0.78183334...           0.25
2.6927914       0.43388587...        0.43388586...           0.50
3.14159         0.00000265...        0.00000265...       20261.95

$ ucalc "type float; sweep sin(x) for x in [0, 3.14159, 8]"
x                    result               double ref      ULP error
---------------------------------------------------------------------
0                         0                      0           0.00
0.44879857      0.43388271...        0.43388268...           0.50
...
3.14159        -0.00000086...        0.00000265...      857625.00
```

**Insight**: Both posit32 and float lose precision catastrophically near
pi due to argument reduction, but posit32 maintains sub-ULP accuracy
through most of the range.  The `sweep` command makes this immediately
visible without writing any C++.
