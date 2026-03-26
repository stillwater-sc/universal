# ucalc Compute Engine Roadmap

## Vision

ucalc evolves from a bc/dc-style REPL calculator into a **compute oracle** --
a flexible compute engine that both humans and AI assistants can query for
numerical analysis, quantization preview, precision forensics, and reproducible
computation. The key insight: bc/dc were designed for humans at a terminal;
ucalc is designed for an era where AI agents need structured, machine-readable
numerical answers just as much as humans need interactive exploration.

**Epic**: GitHub issue #595

## Design Principles

1. **Dual interface**: Interactive REPL for humans, structured CLI for AI agents
2. **Machine-readable by default**: Every command supports `--json` and `--csv` output
3. **Pipe-composable**: Stdin/stdout friendly, exit codes are meaningful
4. **Introspection-first**: Bits, ULPs, components, and error are first-class concepts
5. **No compilation required**: Answers to "what does this value look like in posit<32,2>?" should take seconds, not minutes
6. **ASCII-only output**: No unicode characters (Windows console compatibility)

## Command Vocabulary Overview

```
ucalc "3.14"                                  # evaluate, print
ucalc -t posit<32,2> "3.14"                  # evaluate in specific type
ucalc sweep "sin(0.1)"                        # compare across all types
ucalc quantize mxfp4 "[0.3, -1.2, 0.007]"   # quantize a vector
ucalc ulp posit<32,2> "3.14"                 # ULP at value
ucalc bits cfloat<16,5> "3.14"               # bit decomposition
ucalc compare "a=3.14; a*a" float double dd  # side-by-side
ucalc error "sin(x)" posit<32,2> "[0,pi,100]"  # error profile
ucalc histogram cfloat<8,3> "[0,1]"           # representable value distribution
ucalc --json sweep "exp(1.0)"                 # machine-readable output
echo "type posit32; 1/3" | ucalc              # pipe mode
ucalc -f script.ucalc                         # batch mode
```

---

## Tier 1: MVP -- Interactive Calculator

**Goal**: Replace the write-compile-run cycle for quick numerical questions.

### 1.1 Type Dispatch Registry

Runtime type dispatch table mapping string names to pre-instantiated Universal
types. Build on the existing `type_list` + fold-expression dispatch pattern
from `blas/serialization/type_registry.hpp`.

**MVP type set** (~15 types):

| Alias | Type |
|-------|------|
| `float` | IEEE-754 binary32 |
| `double` | IEEE-754 binary64 |
| `posit8` | `posit<8,0>` |
| `posit16` | `posit<16,1>` |
| `posit32` | `posit<32,2>` |
| `posit64` | `posit<64,3>` |
| `cfloat8` | `cfloat<8,2>` |
| `fp16` | `cfloat<16,5>` (IEEE half) |
| `bfloat16` | `bfloat16` |
| `fp32` | `cfloat<32,8>` |
| `fp64` | `cfloat<64,11>` |
| `fixpnt16` | `fixpnt<16,8>` |
| `fixpnt32` | `fixpnt<32,16>` |
| `dd` | double-double |
| `qd` | quad-double |

Each registered type exposes: arithmetic ops, conversion from string/double,
`to_binary()`, `type_tag()`, `components()`, `numeric_limits` queries.

**File**: `tools/ucalc/type_dispatch.hpp`

### 1.2 Expression Parser and Evaluator

Recursive-descent (Pratt) parser for infix arithmetic with variables.

- **Tokenizer**: decimal, hex (`0x`), scientific notation, identifiers, operators, parens, brackets
- **Precedence**: `+/-` < `*/ %` < unary `-` < `^` (power) < function call
- **Built-in functions**: `sqrt`, `abs`, `log`, `log2`, `log10`, `exp`, `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `pow`, `min`, `max`, `floor`, `ceil`
- **Variables**: `x = <expr>` stores result; `x` retrieves it; `ans` is implicit last result
- **Constants**: `pi`, `e`, `ln2`, `maxpos`, `minpos`, `epsilon`, `inf`, `nan`
- **Semicolons**: Chain expressions: `a = 3.14; a * a`

**File**: `tools/ucalc/expression.hpp`

### 1.3 REPL Loop and Command Processor

Interactive loop with commands and expression evaluation.

**Commands**:

| Command | Description |
|---------|-------------|
| `type <name>` | Set active type (e.g., `type posit32`) |
| `show <expr>` | Value + binary decomposition + components |
| `compare <expr>` | Evaluate across all registered types, tabular output |
| `types` | List available types with brief properties |
| `vars` | List defined variables and their values |
| `help` | Command reference |
| `quit` / `exit` | Exit |

**Modes**:
- Interactive: prompt, readline if available
- Pipe: if stdin is not a terminal, suppress prompts, read commands line-by-line
- Batch: `ucalc -f script.ucalc` executes a file of commands

**Error handling**: Parse errors, overflow, divide-by-zero displayed as messages, never crashes.

**Files**: `tools/ucalc/ucalc.cpp`, `tools/ucalc/CMakeLists.txt`

### 1.4 Structured Output Modes

From the very start, every command supports output format flags:

- `--json` : JSON object per result (or JSON array for multi-result commands)
- `--csv` : Header row + data rows
- `--plain` : Human-readable (default in interactive mode)
- `--quiet` : Value only, no decoration (useful for shell `$(ucalc ...)` substitution)

This is what makes ucalc composable with AI agents and shell pipelines from day one.

**Example JSON output**:
```json
{
  "expression": "3.14",
  "type": "posit<32,2>",
  "value": "3.14",
  "decimal": "3.13999938964843750",
  "binary": "0.01.00.0010001111010111",
  "components": {"sign": 0, "regime": 1, "exponent": 0, "fraction": "0010001111010111"},
  "ulp_error": 0.125,
  "exact": false
}
```

**Exit codes**:

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Parse error |
| 2 | Arithmetic error (overflow, div-by-zero) |
| 3 | Unknown type |
| 4 | File not found (batch mode) |

---

## Tier 2: Numerical Forensics

**Goal**: Answer "where did precision go?" without writing C++.

### 2.1 Bit-Level Dissection

`bits <expr>` and `show <expr>` display the full bit decomposition:
sign, exponent/regime, fraction/mantissa, with field boundaries marked.

```
ucalc> type posit32
ucalc> show 3.14
  value:      3.13999938964843750
  binary:     0.01.00.0010001111010111000010100
  sign:       0 (+)
  regime:     01 (k=0)
  exponent:   00
  fraction:   0010001111010111000010100
  components: (+1, scale=1, fraction=1.0010001111010111000010100)
```

`components <expr>` returns just the structured decomposition (useful with `--json`).

### 2.2 Error Propagation Tracing

`trace <expr>` evaluates an expression and shows the intermediate result,
ULP error, and rounding direction at each operation.

```
ucalc> type fp16
ucalc> trace (1.0 + 1e-4) - 1.0
  step 1: 1e-4              = 0.00009989 (exact: 0.0001, ulp_err: 0.11)
  step 2: 1.0 + 0.00009989  = 1.0        (exact: 1.0001, ulp_err: 1.00, ROUNDED DOWN)
  step 3: 1.0 - 1.0         = 0.0        (exact: 0.0001, catastrophic cancellation)
  result: 0.0
  total_ulp_error: inf (relative)
```

### 2.3 Catastrophic Cancellation Detection

`cancel <expr>` flags subtractions where operands share many leading bits.
Reports the number of cancelled significant digits.

```
ucalc> type fp32
ucalc> cancel sqrt(1000001) - sqrt(1000000)
  WARNING: catastrophic cancellation
  operand 1: 1000000.500000
  operand 2: 1000000.000000
  shared leading digits: 7 of 7.2 significant digits
  result significant digits: ~0.2
  suggestion: reformulate as 1 / (sqrt(1000001) + sqrt(1000000))
```

### 2.4 Rounding Audit Trail

`audit <expr>` shows every rounding event: direction (up/down/ties-to-even),
magnitude (in ULPs), and the exact unrounded intermediate value (computed in
quad-double reference).

---

## Tier 3: Type Exploration and Comparison

**Goal**: Rapid side-by-side comparison across number systems.

### 3.1 Sweep Command

`sweep <expr>` evaluates an expression across all registered types and
displays a comparison table.

```
ucalc> sweep sin(0.1)
  type           value                 ulp_error  bits
  float          0.0998334166...       0.00       32
  double         0.0998334166468...    0.00       64
  posit8         0.1015625             17.23      8
  posit16        0.0998535156          0.20       16
  posit32        0.0998334139          0.04       32
  fp16           0.0998535156          0.20       16
  bfloat16       0.0996093750          2.29       16
  dd             0.0998334166468...    0.00       128
  qd             0.0998334166468...    0.00       256
```

Reference value computed in qd (or user-specified reference type).

### 3.2 Range and Precision Queries

```
ucalc> range posit32
  minpos:   7.52316e-37
  maxpos:   1.32923e+36
  epsilon:  3.72529e-09
  dynamic_range_decades: 72.2
  significant_digits: ~8.5

ucalc> precision cfloat<8,4> vs posit<8,2>
  type          min_prec  max_prec  avg_prec  dynamic_range
  cfloat<8,4>   3 bits    3 bits    3 bits    15.5 decades
  posit<8,2>    0 bits    5 bits    3.2 bits  5.9 decades
```

### 3.3 Breakpoint Finder

`diverge <expr> <type1> <type2> <tolerance> for x in [a, b]` finds the
first input value where two types disagree by more than the given tolerance.

```
ucalc> diverge "sin(x)" posit<32,2> float 1ulp for x in [0, 6.28]
  first divergence at x = 0.00024414...
  posit<32,2>: 0.000244140...
  float:       0.000244140...
  difference:  1.03 ULPs
```

### 3.4 Number Line Visualization

`numberline <type> [lo, hi]` shows an ASCII representation of where
representable values cluster.

```
ucalc> numberline cfloat<5,2> [0, 4]
  0     0.5     1       2       4
  ||||||| | | |  |  |  |   |   |
  dense near 0  ------>  sparse near maxpos
  representable values in [0,4]: 12
```

### 3.5 Precision Heatmap

`heatmap <type>` shows effective precision (significant bits) as a function
of magnitude, revealing wobble (hexfloat), taper (posit), or uniform
precision (IEEE).

```
ucalc> heatmap posit<16,1>
  magnitude    sig_bits  bar
  1e-4         2         ##
  1e-3         4         ####
  1e-2         6         ######
  1e-1         9         #########
  1e+0         12        ############
  1e+1         9         #########
  1e+2         6         ######
  1e+3         4         ####
  1e+4         2         ##
```

---

## Tier 4: AI/ML Quantization Workbench

**Goal**: Preview quantization effects without training frameworks.

### 4.1 Quantize Command

`quantize <format> <vector>` applies quantization and shows per-element
results and aggregate error.

```
ucalc> quantize mxfp4 [0.3, -1.2, 0.007, 2.5, -0.001]
  block_size: 32
  shared_scale: 2^1
  index  original   quantized   error      ulps
  0      0.3        0.25        -0.05      1
  1      -1.2       -1.0        0.2        1
  2      0.007      0.0         -0.007     clipped
  3      2.5        2.0         -0.5       1
  4      -0.001     0.0         0.001      flushed
  RMSE: 0.234
  max_abs_error: 0.5
  clipped: 1, flushed: 1
```

Supported formats: `mxfp4`, `mxfp6`, `mxfp8`, `nvfp4`, `int4`, `int8`,
`e5m2`, `e4m3`, `bfloat16`, `fp16`, and any registered Universal type.

### 4.2 Block Format Simulation

`block <format> <vector>` shows the full MX/NV block decomposition:
shared scale, per-element encoding, and reconstructed values.

```
ucalc> block nvfp4 [0.3, -1.2, 0.007, 2.5] tensor_scale=4.0
  tensor_scale: 4.0
  block_scale (e4m3): 0.75
  effective_scale: 3.0
  index  original  pre_scaled  encoded  decoded  error
  0      0.3       0.1         e2m1(+0.0)  0.0   -0.3
  1      -1.2      -0.4        e2m1(-0.5)  -1.5  -0.3
  ...
```

### 4.3 Mixed-Precision Dot Product

`dot <v1> <v2> [accum=<type>]` computes dot products with explicit control
over element type and accumulation type.

```
ucalc> type fp16
ucalc> dot [1.0, 2.0, 3.0] [4.0, 5.0, 6.0] accum=fp32
  element_type: fp16
  accum_type: fp32
  result: 32.0
  reference (qd): 32.0
  ulp_error: 0

ucalc> dot [1.0, 2.0, 3.0] [4.0, 5.0, 6.0] accum=fp16
  result: 32.0
  reference (qd): 32.0
  ulp_error: 0

ucalc> type posit32
ucalc> dot [1e10, 1.0, -1e10] [1.0, 1.0, 1.0] accum=quire
  element_type: posit<32,2>
  accum_type: quire<32,2>
  result: 1.0  (exact)
  note: quire accumulation preserves full precision
```

### 4.4 Overflow/Underflow Maps

`clip <type> <distribution>` shows which values in a distribution overflow,
underflow, or get flushed to zero.

```
ucalc> clip e5m2 normal(0, 1.0, 1000)
  total values: 1000
  representable: 972 (97.2%)
  flushed to zero: 15 (1.5%)  -- |value| < minpos
  clipped to maxpos: 8 (0.8%) -- |value| > maxpos
  clipped to inf: 5 (0.5%)    -- overflow to inf
  effective range used: [-57344, 57344]
  minpos: 6.10e-5, maxpos: 57344
```

---

## Tier 5: Education and Visualization

**Goal**: Make numerical representations tangible for students and researchers.

### 5.1 Step-by-Step Arithmetic

`steps <expr>` shows the binary arithmetic process in detail.

```
ucalc> type cfloat<8,4>
ucalc> steps 1.5 + 0.375
  operand A: 1.5     = 0 0111 100  (1.100 * 2^0)
  operand B: 0.375   = 0 0101 100  (1.100 * 2^-2)

  1. Align exponents (shift B right by 2):
     A: 1.100 * 2^0
     B: 0.011 * 2^0  (shifted, lost bits: 00)

  2. Add significands:
     1.100
   + 0.011
   -------
     1.111

  3. Normalize: already normalized
  4. Round: no rounding needed (exact)
  5. Encode: 0 0111 111

  result: 1.875 = 0 0111 111
  exact: yes
```

### 5.2 Multiplication Steps

```
ucalc> type cfloat<8,4>
ucalc> steps 1.5 * 2.5
  operand A: 1.5 = 0 0111 100 (1.100 * 2^0)
  operand B: 2.5 = 0 1000 010 (1.010 * 2^1)

  1. Add exponents: 0 + 1 = 1
  2. Multiply significands:
       1.100
     x 1.010
     ------
       0000
      1100
     0000
    1100
    --------
    10.011000

  3. Normalize: 1.0011 * 2^2 (right shift 1, exp -> 2)
  4. Round to 3 fraction bits: 1.001 * 2^2 (truncated 1, round up -> 1.010)
  5. Encode: 0 1001 010

  result: 3.75 (exact: 3.75, no rounding error)
```

### 5.3 Precision Heatmap (extended)

See Tier 3.5. In education mode, includes explanatory annotations:

```
ucalc> heatmap posit<8,0> --annotated
  Posit tapered precision: bits shift from fraction to regime at extremes

  magnitude  sig_bits  bar                   note
  1/64       0         .                     all bits used by regime
  1/16       1         #                     1 fraction bit
  1/4        2         ##
  1          4         ####                  <-- sweet spot (useed=2)
  4          2         ##
  16         1         #
  64         0         .                     all bits used by regime
```

---

## Tier 6: Reproducibility and Verification

**Goal**: Provide a reference oracle for numerical computation.

### 6.1 Reference Computation

`ref <expr>` evaluates in the highest available precision (qd by default)
and reports the result to arbitrary decimal places.

```
ucalc> ref pi * pi
  9.8696044010893586188344909998761511...
  computed in: quad-double (256-bit significand)
  decimal digits reliable: ~62
```

### 6.2 Cross-Platform Oracle

`oracle <type> <expr>` gives the canonical answer for what a specific type
should produce, useful for validating hardware or library implementations.

```
ucalc> oracle posit<32,2> "sin(0.1)"
  type: posit<32,2>
  value: 0.0998334139
  binary: 0.01.00.1100110010110...
  hex: 0x0CC2D...
  This is the correctly-rounded result for posit<32,2>.
```

### 6.3 Test Vector Generation

`testvec <type> <function> [a, b, n]` generates golden reference vectors
for regression tests, output in C++ initializer list or CSV.

```
ucalc> testvec posit<16,1> sin [0, 3.14159, 10] --format=cpp
  // Golden reference vectors for sin(x) in posit<16,1>
  // Generated by ucalc on 2026-03-26
  struct TestVector { double input; uint16_t expected_bits; double expected_value; };
  constexpr TestVector sin_posit16[] = {
    { 0.000000, 0x0000, 0.000000 },
    { 0.349066, 0x1580, 0.341797 },
    ...
  };

ucalc> testvec posit<16,1> sin [0, 3.14159, 10] --format=csv --json
  [produces machine-readable CSV or JSON output]
```

### 6.4 Faithfulness Check

`faithful <expr>` evaluates in both the active type and a high-precision
reference, reporting whether the result is faithfully rounded (one of the
two nearest representable values).

```
ucalc> type posit32
ucalc> faithful "exp(1.0)"
  result:    2.71828174591064453
  reference: 2.71828182845904524...
  floor:     2.71828174591064453  <-- result matches floor
  ceil:      2.71828198432922363
  faithful:  YES
```

---

## Tier 7: Statistical and Distribution Analysis

**Goal**: Understand the statistical properties of number systems.

### 7.1 Representable Value Histogram

`histogram <type> [lo, hi, bins]` shows how representable values are
distributed across a range.

```
ucalc> histogram cfloat<8,3> [0, 1, 10]
  bin         count  bar
  [0.0, 0.1)  8     ########
  [0.1, 0.2)  4     ####
  [0.2, 0.3)  2     ##
  [0.3, 0.4)  2     ##
  [0.4, 0.5)  2     ##
  [0.5, 0.6)  1     #
  [0.6, 0.7)  1     #
  [0.7, 0.8)  1     #
  [0.8, 0.9)  1     #
  [0.9, 1.0]  1     #
  total representable in [0,1]: 23
```

### 7.2 Stochastic Rounding Simulation

`stochastic <expr> N` evaluates an expression N times with stochastic
rounding and reports the distribution of results.

```
ucalc> type cfloat<8,4>
ucalc> stochastic "0.1 + 0.2" 10000
  unique results: 2
  0.296875: 7023 (70.2%)  -- round down
  0.312500: 2977 (29.8%)  -- round up
  mean: 0.30146
  exact: 0.3
  bias: +0.00146
```

### 7.3 Error Distribution Over a Range

`errordist <expr> <type> for x in [a, b, n]` evaluates an expression at
n points and shows the distribution of ULP errors.

```
ucalc> errordist "sin(x)" posit<16,1> for x in [0, 6.28, 10000]
  ulp_error   count  bar
  0           312    ###
  (0, 0.5]    4521   #############################################
  (0.5, 1]    4088   #########################################
  (1, 2]      879    #########
  (2, 4]      187    ##
  (4, 8]      13     .
  max_ulp: 6.3 at x = 3.14155
  mean_ulp: 0.72
  faithfully_rounded: 89.2%
```

### 7.4 Condition Number Estimation

`cond <matrix> [type]` computes an estimate of the condition number of a
small matrix in the given type.

```
ucalc> type fp32
ucalc> cond [[1, 2], [1.0001, 2]]
  condition number (1-norm): ~20000
  warning: ill-conditioned, expect ~4 digits of precision loss
  effective precision: ~3 decimal digits (of 7.2 available)
```

---

## Tier 8: Scripting and AI Agent Integration

**Goal**: Make ucalc a first-class tool for AI assistants and automation.

### 8.1 Non-Interactive CLI

Every command available in the REPL is also available as a CLI subcommand:

```bash
ucalc eval "3.14"
ucalc eval -t posit32 "3.14"
ucalc sweep "sin(0.1)"
ucalc compare "3.14" float double posit32
ucalc bits cfloat<16,5> "3.14"
ucalc ulp posit32 "3.14"
ucalc quantize mxfp4 "[0.3, -1.2]"
ucalc ref "pi^2"
ucalc oracle posit32 "sin(0.1)"
ucalc testvec posit16 sin "[0, 3.14, 100]"
```

### 8.2 Structured Output for All Commands

`--json` and `--csv` flags on every command. AI agents parse JSON directly;
data analysis tools consume CSV.

```bash
# AI agent workflow: "What's the best 8-bit type for this weight distribution?"
ucalc --json quantize cfloat<8,4> "[0.3, -1.2, 0.007, 2.5]"
ucalc --json quantize posit<8,2> "[0.3, -1.2, 0.007, 2.5]"
ucalc --json quantize e4m3 "[0.3, -1.2, 0.007, 2.5]"
# Agent compares RMSE from JSON, recommends type
```

### 8.3 Batch Script Execution

`ucalc -f script.ucalc` executes a file of ucalc commands. Scripts support
comments (`#`), variables, and all interactive commands.

```bash
# script.ucalc -- quantization comparison report
type posit<8,2>
quantize [0.3, -1.2, 0.007, 2.5, -0.001, 1.8, -3.2, 0.5]
type cfloat<8,4>
quantize [0.3, -1.2, 0.007, 2.5, -0.001, 1.8, -3.2, 0.5]
compare "sin(0.1)" posit<8,2> cfloat<8,4> e4m3
```

### 8.4 Shell Integration

`--quiet` mode outputs just the value, for shell variable capture:

```bash
# In a bash script
RESULT=$(ucalc --quiet -t posit32 "sin(0.1)")
ULP=$(ucalc --quiet ulp posit32 "sin(0.1)")

# Loop over types
for TYPE in posit32 float cfloat16 bfloat16; do
  echo "$TYPE: $(ucalc --quiet -t $TYPE 'sin(0.1)')"
done
```

### 8.5 MCP Server Mode (Future)

`ucalc --mcp` starts ucalc as a Model Context Protocol server, exposing
all commands as MCP tools that AI assistants can call directly over stdio
or SSE transport.

```json
{
  "tool": "ucalc.sweep",
  "arguments": {
    "expression": "sin(0.1)",
    "output_format": "json"
  }
}
```

This is the ultimate AI integration: ucalc becomes a tool in the AI's
toolbox, callable without shell intermediation.

---

## Implementation Phasing

### Phase 1: Foundation (Tier 1)
**Scope**: MVP REPL + structured output

| Component | Dependency | Effort |
|-----------|------------|--------|
| 1.1 Type dispatch registry | None | Medium |
| 1.2 Expression parser | 1.1 | Medium |
| 1.3 REPL loop + commands | 1.1, 1.2 | Medium |
| 1.4 Structured output (--json, --csv, --quiet) | 1.3 | Small |

**Deliverable**: Working REPL that evaluates expressions in ~15 types,
with `type`, `show`, `compare`, `types`, `vars`, `help` commands.
AI agents can query via CLI with `--json`.

### Phase 2: Introspection (Tiers 2 + 3)
**Scope**: Precision forensics + type comparison

| Component | Dependency | Effort |
|-----------|------------|--------|
| 2.1 Bit-level dissection (`bits`, `components`) | Phase 1 | Small |
| 2.2 Error propagation tracing (`trace`) | Phase 1 | Large |
| 2.3 Cancellation detection (`cancel`) | Phase 1 | Medium |
| 2.4 Rounding audit (`audit`) | Phase 1 | Large |
| 3.1 Sweep command | Phase 1 | Small |
| 3.2 Range/precision queries | Phase 1 | Small |
| 3.3 Breakpoint finder (`diverge`) | Phase 1 | Medium |
| 3.4 Number line visualization | Phase 1 | Small |
| 3.5 Precision heatmap | Phase 1 | Small |
| Readline + history | Phase 1 | Small |
| Extended type set (~30+ types) | Phase 1 | Medium |

**Deliverable**: Full numerical forensics toolkit. Users and AI agents can
diagnose precision issues interactively or via structured queries.

### Phase 3: Quantization (Tier 4)
**Scope**: AI/ML quantization workbench

| Component | Dependency | Effort |
|-----------|------------|--------|
| 4.1 Quantize command | Phase 1 | Medium |
| 4.2 Block format simulation | Phase 1 | Medium |
| 4.3 Mixed-precision dot product | Phase 1 | Medium |
| 4.4 Overflow/underflow maps | Phase 1 | Small |
| Vector literal support in parser | Phase 1 | Small |

**Deliverable**: Quantization preview tool. AI agents can evaluate
quantization strategies without Python/PyTorch.

### Phase 4: Education + Verification (Tiers 5 + 6)
**Scope**: Step-by-step arithmetic + reference oracle

| Component | Dependency | Effort |
|-----------|------------|--------|
| 5.1 Step-by-step addition/subtraction | Phase 1 | Medium |
| 5.2 Step-by-step multiplication | Phase 1 | Medium |
| 6.1 Reference computation (`ref`) | Phase 1 | Small |
| 6.2 Cross-platform oracle | Phase 1 | Small |
| 6.3 Test vector generation | Phase 1 | Medium |
| 6.4 Faithfulness check | Phase 1 | Small |

**Deliverable**: Educational tool for teaching and a reference oracle
for validating implementations.

### Phase 5: Statistics + Automation (Tiers 7 + 8)
**Scope**: Distribution analysis + full AI integration

| Component | Dependency | Effort |
|-----------|------------|--------|
| 7.1 Representable value histogram | Phase 1 | Small |
| 7.2 Stochastic rounding simulation | Phase 1 | Medium |
| 7.3 Error distribution over range | Phase 2 | Medium |
| 7.4 Condition number estimation | Phase 1 | Medium |
| 8.1 Full CLI subcommand interface | Phase 1 | Medium |
| 8.3 Batch script execution | Phase 1 | Small |
| 8.4 Shell integration helpers | Phase 1 | Small |
| 8.5 MCP server mode | Phase 1 | Large |

**Deliverable**: Complete compute oracle with statistical analysis
and native AI assistant integration via MCP.

---

## Dependency Graph

```
Phase 1: Foundation
  1.1 Type Registry
    |
    +---> 1.2 Expression Parser
    |       |
    |       +---> 1.3 REPL + Commands ---> 1.4 Structured Output
    |
    +---> Extended Type Set (Phase 2)

Phase 2: Introspection
  Phase 1 ---> bits, components, show (2.1)
  Phase 1 ---> trace (2.2) ---> cancel (2.3) ---> audit (2.4)
  Phase 1 ---> sweep (3.1), range (3.2), numberline (3.4), heatmap (3.5)
  Phase 1 ---> diverge (3.3)
  Phase 1 ---> readline + history

Phase 3: Quantization
  Phase 1 ---> quantize (4.1), block (4.2), dot (4.3), clip (4.4)
  1.2 Parser ---> vector literal support

Phase 4: Education + Verification
  Phase 1 ---> steps (5.1, 5.2)
  Phase 1 ---> ref (6.1), oracle (6.2), testvec (6.3), faithful (6.4)

Phase 5: Statistics + Automation
  Phase 2 ---> errordist (7.3)
  Phase 1 ---> histogram (7.1), stochastic (7.2), cond (7.4)
  Phase 1 ---> CLI (8.1), batch (8.3), shell (8.4), MCP (8.5)
```

---

## Competitive Positioning

| Feature | bc/dc | Python/NumPy | MATLAB | Mathematica | ucalc |
|---------|-------|-------------|--------|-------------|-------|
| Arbitrary precision arithmetic | Yes | mpmath | VPA | Yes | Yes (qd, ereal) |
| Custom number types | No | No | No | No | **30+ types** |
| Bit-level inspection | No | No | No | Limited | **Native** |
| Quantization preview | No | Manual | No | No | **Built-in** |
| MX/NV block format simulation | No | No | No | No | **Built-in** |
| Machine-readable output | No | Yes | Limited | Yes | **JSON/CSV** |
| No install / header-only deps | Yes | No | No | No | Yes |
| AI agent integration | No | Manual | No | No | **MCP native** |
| Posit/LNS/DBNS support | No | Limited | No | No | **Native** |

---

## Success Criteria

1. **5-second answers**: Any "what does X look like in type Y?" question answerable in under 5 seconds, no compilation
2. **AI-parseable**: Every command produces valid JSON with `--json`
3. **Pipeline-composable**: `ucalc` works in shell pipelines with exit codes
4. **Self-documenting**: `help <command>` explains every command with examples
5. **Zero external dependencies**: Builds with just a C++20 compiler (readline optional)
6. **Accurate**: Results match or reference qd/rational arithmetic; deviations documented
