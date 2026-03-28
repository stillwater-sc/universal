# ucalc -- Universal Mixed-Precision Compute Engine

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
Universal's number types.  It also serves as a **compute oracle** for AI
agents, supporting structured JSON/CSV output for automated workflows.

## What

`ucalc` is a REPL calculator and compute engine that:

- Supports **42+ number types** out of the box: IEEE float/double, posit (8-64),
  takum (8-64), cfloat (8-64), bfloat16, FP8 variants, fixed-point, decimal
  fixed-point, LNS, integer, hexadecimal float, decimal float, rational,
  double-double, quad-double, and cascaded variants.
- Parses **infix arithmetic** with standard operator precedence, parentheses,
  variables, constants, and math functions.
- Provides **20+ analysis commands** organized in three categories:
  - **Introspection**: trace, cancel, audit, diverge, numberline, heatmap
  - **Quantization**: quantize, block, dot, clip
  - **Inspection**: show, compare, bits, range, precision, ulp, sweep, faithful
- Works in four modes: interactive REPL, one-shot CLI, pipe/script, and batch file.
- Produces **structured output** (`--json`, `--csv`, `--quiet`) for AI agent consumption.
- Optionally integrates GNU Readline for tab completion and persistent history.

### Architecture

```
tools/ucalc/
  type_dispatch.hpp   -- TypeRegistry + SFINAE math dispatch (42+ types)
  expression.hpp      -- Tokenizer + recursive-descent parser/evaluator + tracing
  output_format.hpp   -- JSON escape, CSV quoting, output format utilities
  data_loader.hpp     -- CSV file reader and vector literal parser
  registry.hpp        -- Default type registry (shared with regression tests)
  ucalc.cpp           -- REPL loop, 20+ commands, CLI flag parsing
  CMakeLists.txt      -- Build config with optional readline detection
  scripts/            -- Example scripts for humans and AI agents
```

## How

### Build

```bash
# Standalone
cmake -DUNIVERSAL_BUILD_TOOLS_UCALC=ON ..
make ucalc

# Or as part of the full build
cmake -DUNIVERSAL_BUILD_ALL=ON ..
make ucalc
```

With readline (optional):
```bash
sudo apt install libreadline-dev   # Debian/Ubuntu
cmake -DUNIVERSAL_BUILD_TOOLS_UCALC=ON ..
make ucalc
# readline auto-detected; tab completion and history enabled
```

### Usage Modes

```bash
ucalc                                  # interactive REPL
ucalc "type posit32; 1/3 + 1/3"       # one-shot from command line
ucalc -t posit32 "1/3 + 1/3"          # set type via flag
ucalc -f script.ucalc                  # batch mode (execute script file)
echo "compare sqrt(2)" | ucalc        # pipe mode
ucalc --json "show 3.14"              # JSON output for AI agents
ucalc --csv "compare pi"              # CSV output
ucalc --quiet -t posit32 "sin(0.1)"   # value-only for shell scripts
```

### CLI Flags

| Flag | Description |
|------|-------------|
| `--json` | JSON output for all commands |
| `--csv` | CSV output for tabular commands |
| `--quiet` | Value only, no decoration |
| `-t <type>` | Set active type from command line |
| `-f <file>` | Execute a script file (batch mode) |
| `--help` | Usage information |
| `--version` | Version string |

## Command Reference

### Expression Evaluation

| Feature | Syntax |
|---------|--------|
| Arithmetic | `+  -  *  /  ^  (parentheses)` |
| Functions | `sqrt, abs, log, exp, sin, cos, tan, asin, acos, atan, pow` |
| Constants | `pi, e, phi, ln2, ln10, sqrt2, sqrt3, sqrt5` (quad-double precision) |
| Variables | `x = 1/3` (then use `x` in expressions) |
| Semicolons | `type posit32; 1/3 + 1/3 + 1/3` |

### Type Inspection Commands

| Command | Description |
|---------|-------------|
| `type <name>` | Set the active arithmetic type |
| `types` | List all 42+ available types |
| `show <expr>` | Value + decimal + binary + components |
| `compare <expr>` | Evaluate across all types in a table |
| `bits <expr>` | Raw bit pattern |
| `range` | Symmetry range: [maxneg ... minneg] 0 [minpos ... maxpos] |
| `precision` | Binary/decimal digits, epsilon, minpos, maxpos |
| `ulp <value>` | Unit in the last place at a given value |
| `sweep <expr> for <var> in [a, b, n]` | Error analysis across a range |
| `faithful <expr>` | Check faithful rounding vs qd reference |
| `increment <expr>` | Show value and next representable value |
| `decrement <expr>` | Show value and previous representable value |

### Numerical Forensics Commands

| Command | Description |
|---------|-------------|
| `trace <expr>` | Show each operation with ULP error and rounding direction |
| `cancel <expr>` | Detect catastrophic cancellation in subtractions |
| `audit <expr>` | Rounding audit trail with cumulative error drift |
| `diverge <expr> <t1> <t2> <tol> for <var> in [a, b]` | Find where two types first disagree |
| `numberline [lo, hi]` | ASCII visualization of representable value density |
| `heatmap` | Precision (sig bits) vs magnitude bar chart |

### Quantization Workbench Commands

| Command | Description |
|---------|-------------|
| `quantize <fmt> [data] \| -f <file>` | Quantize data, report RMSE/QSNR/errors |
| `block <fmt> [data] \| -f <file>` | MX/NV block decomposition (scale + elements) |
| `dot [v1] [v2] [accum=<type>]` | Mixed-precision dot product |
| `clip <type> [data] \| -f <file>` | Overflow/underflow map for a distribution |

## Examples

### Example 1: Posit Closure -- 1/3 + 1/3 + 1/3

```
posit32> show 1/3 + 1/3 + 1/3
  value:      1.000000000e+00
  binary:     0b0.10.00.000000000000000000000000000
  type:       posit< 32, 2, uint32_t>

float> show 1/3 + 1/3 + 1/3
  value:      1
  binary:     0b0.01111111.00000000000000000000000
  type:       float (IEEE-754 binary32)
```

posit32 sums three rounded thirds to exactly 1.0; IEEE float doesn't.

### Example 2: Trace Error Propagation

```
float> trace (1.0 + 1e-4) - 1.0
  step 1: 1 + 9.99999997e-05
          result:    1.00009999
          reference: 1.000100016...
          ROUNDED DOWN  0.10 ULP
  step 2: 1.00009999 - 1
          result:    9.99999997e-05  (exact)
  result: 9.99999997e-05
  reference precision: quad-double
```

Shows where precision is lost at each arithmetic step.

### Example 3: Cancellation Detection

```
float> cancel sqrt(1000001) - sqrt(1000000)
  WARNING: CATASTROPHIC cancellation (step 3)
  operand 1:       1000.00049
  operand 2:       1000
  shared digits:   6.3 of 6.9
  result digits:   ~0.6
```

Identifies subtractions where nearly all significant digits are lost.

### Example 4: Quantize a Weight Tensor

```bash
# Compare quantization quality across formats
for fmt in fp8e4m3 fp8e5m2 bfloat16 posit8 fp16; do
  echo -n "$fmt: "
  ucalc --quiet "quantize $fmt -f weights.csv"
done
```

```
fp8e4m3:  0.0131171 31.6dB 10000
fp8e5m2:  0.0267517 25.4dB 10000
bfloat16: 0.00165137 49.6dB 10000
posit8:   0.0131249 31.6dB 10000
fp16:     0.000103695 73.7dB 10000
```

Shows RMSE, QSNR (quantization signal-to-noise ratio in dB), and element count.

### Example 5: Precision Heatmap

```
posit16> heatmap
  posit< 16, 2, uint16_t>

  magnitude     sig_bits  bar
  1e-12              2.0  ######
  1e-11              3.0  ##########
  ...
  1e-1              12.0  ########################################
  1e+0              11.0  ####################################
  1e+1              12.0  ########################################
  ...
  1e+11              3.0  ##########
  1e+12              3.0  ##########

  tapered precision: peaks near 1, falls off at extremes
```

Visualizes how precision varies with magnitude -- tapered for posit, uniform for IEEE.

### Example 6: Number Line Density

```
posit8> numberline [0, 4]
  posit<  8, 2, uint8_t> in [0, 4]
  representable values: 81

  0                                  2                                   4
  |||||||||| ||||||| |  | | | |  | | |   |    |   |    |   |    |   |    |
  dense near 0  ------>  sparse near 4
```

Shows where representable values cluster -- dense near 0 for floating-point types.

### Example 7: Mixed-Precision Dot Product

```
float> dot [1e10, 1, -1e10] [1, 1, 1]
  result:       0                     <- catastrophic cancellation
  abs error:    1, rel error: 1

float> dot [1e10, 1, -1e10] [1, 1, 1] accum=dd
  result:       1.0                   <- exact with dd accumulation
  error:        exact
```

Shows how accumulation precision prevents catastrophic cancellation in dot products.

### Example 8: MX Block Decomposition

```
ucalc> block mxfp4 [0.3, -1.2, 0.007, 2.5, -0.001, 1.8, -3.2, 0.5]
  format:    MX FP4 (e2m1, block=32, e8m0 scale)
  block 0  scale: 0.5 (0.5)
  idx       original  element        decoded       error
  0              0.3  0.5               0.25        0.05
  1             -1.2  -2                  -1        -0.2
  ...
  RMSE:  0.21579794
  QSNR:  17.6 dB
```

Shows how MX block quantization works: shared scale and per-element encoding.

### Example 9: Increment/Decrement (Next Representable Value)

```
decimal32> increment 1.0
  +0000001e+0  1.0
  +1000001e-6  1.000001

float> increment 1.0
  0b0.01111111.00000000000000000000000  1
  0b0.01111111.00000000000000000000001  1.00000012
```

Shows the encoding and next representable value side by side, using the type's
native radix (decimal digits for dfloat, binary bits for cfloat/posit, hex for hfloat).

### Example 10: Overflow/Underflow Map

```bash
# Which FP8 format best fits this weight distribution?
for t in fp8e4m3 fp8e5m2 posit8; do
  echo -n "$t: "
  ucalc --quiet "clip $t -f weights.csv"
done
```

```
fp8e4m3:  99.8% 0clip 19flush
fp8e5m2:  100.0% 0clip 1flush
posit8:   100.0% 0clip 0flush
```

Shows what fraction of values are representable, clipped (overflow), or flushed (underflow).

## Script Examples

The `scripts/` directory contains ready-to-use example scripts:

**Human-facing** (plain output):
- `01_precision_comparison.ucalc` -- Compare 1/3 closure across types
- `02_trig_accuracy_sweep.ucalc` -- sin(x) ULP error over [0, pi]
- `03_numerical_constants.ucalc` -- Constants at every precision level
- `04_catastrophic_cancellation.ucalc` -- Quadratic formula failure
- `05_fp8_deep_learning.ucalc` -- FP8 format exploration

**AI-agent-facing** (JSON/CSV output):
- `06_agent_type_selection.ucalc` -- Weight quantization format comparison
- `07_agent_precision_audit.ucalc` -- Faithfulness audit across types
- `08_agent_sweep_analysis.ucalc` -- Error threshold detection
- `09_agent_type_properties.ucalc` -- Type property database
- `10_agent_golden_vectors.ucalc` -- Reference values for test validation
- `11_agent_trace_analysis.ucalc` -- Error propagation across types
- `12_agent_quantize_comparison.ucalc` -- Format QSNR comparison
