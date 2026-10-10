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

- Supports **46+ number types** out of the box: IEEE float/double, posit (8-64),
  takum (8-64) in both the linear and the logarithmic encoding, cfloat (8-64),
  bfloat16, FP8 variants, fixed-point, decimal fixed-point, LNS, integer,
  hexadecimal float, decimal float, rational, double-double, quad-double, and
  cascaded variants.

  The two takum variants are worth a word: `takum8`..`takum64` and
  `takum_log8`..`takum_log64` share an identical bit layout but map those bits to
  different numbers -- `(1+f)*2^c` against `sqrt(e)^(c+m)` -- so the same input
  prints different encodings under each. `diverge <expr> takum32 takum_log32
  <tol> for <var> in [a, b]` finds where they first disagree.
- Includes the **ubit tile types** `areal` (float lattice) and `poxel` (posit lattice),
  as single tiles (`areal32`, `poxel32`, ...) and as guaranteed enclosures
  (`areal32i`, `poxel32i`, ...), for asking what is *known* about a result rather
  than what was rounded. See [Tile Types](#tile-types-uncertainty-and-decidability).
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
  tiles.hpp           -- areal/poxel tiles and tile intervals: exact literals, set-valued Values
  uncertainty.hpp     -- ubox and decide: box metrics, predicates, verdicts over sets
  oracle.hpp          -- the tightest box: exact dyadic evaluation, interval derivatives, branch and bound
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
| Boxes (tile interval types) | `[a, b]` or `hull(a, b)`: every value from a to b |
| ULP-wide input (tile types) | `x~` or `above(x)`: the open tile just above an exact `x` |

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

## Tile Types: Uncertainty and Decidability

A tile is an exact lattice point, or the open interval to the next lattice point; the
ubit tells the two apart. ucalc has two families of tile types (#1654):

| Family | Types | Semantics |
|--------|-------|-----------|
| Single tile | `areal8/16/32/64`, `poxel8/16/32/64` | The library's sticky-flag arithmetic: the ubit says "inexact", but once an operand is open the tile need not contain the true result. |
| Tile interval | `areal8i/16i/32i/64i`, `poxel8i/16i/32i/64i` | A run of tiles that is **guaranteed** to contain the true result. Functions without an enclosing implementation (`log`, `exp`, `tan`, the inverse trig functions, `pow` with a non-integer exponent) return the entire line rather than a rounded point. |

`areal<n,es>` uses es = 5, 8, 11 at 16, 32, 64 bits (and es = 2 at 8 bits); `poxel<n,2>`
follows the Posit Standard.

Literals are read from their decimal text, not through a double, so `0.1` is the one
open tile that contains one tenth even in `poxel64`, whose lattice is finer than double's.
Named constants are bracketed from their quad-double values.

`show` reports what is known about the set: its tile count and the **sign verdict**
(negative, zero, positive, or undecidable). The JSON output carries `tiles`, `sign`,
`ubit` and `encloses`.

A single tile knows whether it contains its value: literals, `x~` and arithmetic on
exact tiles do; arithmetic with an open operand and functions computed through double
do not. Variables keep their value across a `type` switch, and a tile interval type
reads a single tile that does not enclose its value as the entire line, never as a box
that might exclude the truth. Recompute such a value in the interval type instead.

```
poxel16> (1/3)*3                 # a single tile: 1 is outside, the ubit only says inexact
(0.99951, 1)
poxel16i> (1/3)*3                # a tile interval: contains 1
(0.99951, 1.001)
```

The reference problem is the quadratic 3x^2 + 100x + 2 = 0 from
`applications/precision/ubit/quadratic_roots.cpp` (#1649). At 16 bits the root as
written cannot decide its own sign, and the rearranged formula can:

```
poxel16i> a = 3; b = 100; c = 2
poxel16i> show (-b + sqrt(b*b - 4*a*c)) / (2*a)
  value:      (-0.083374, 0.041687)
  tiles:      16385, sign undecidable
poxel16i> show (2*c) / (-b - sqrt(b*b - 4*a*c))
  value:      (-0.020081, -0.019958)
  tiles:      7, sign negative
```

With ULP-wide coefficients, `a = 3~; b = 100~; c = 2~`, the same expressions show how
input uncertainty widens each box.

### The uncertainty box: `ubox`

`ubox [trace] <expr> [in <types>]` evaluates an expression in tile interval types
(by default the 16-, 32- and 64-bit areal and poxel pairs) and summarizes each box:
its width in tiles, the **decimals of accuracy** its midpoint is guaranteed to have
(`-log10(width / 2|midpoint|)`, the measure of `docs/tutorials/decimals-of-accuracy.md`),
and its sign verdict. Below the boxes it shows what the rounded types of the same
widths compute, and their relative error against a quad-double reference, of which they
give no sign:

```
ubox: (-b + sqrt(b*b - 4*a*c)) / (2*a)
  type                       tiles  decimals  sign        box
  areal16i                   10581       0.0  negative    (-0.041687, 0)
  poxel16i                   16385       0.0  undecidable (-0.083374, 0.041687)
  areal32i                    1365       4.2  negative    (-0.0200144462, -0.0200119019)
  poxel32i                    1365       5.1  negative    (-0.0200122199, -0.02001190186)
  areal64i                    1365      12.9  negative    (-0.020012014421638469, -0.02001201442163373)
  poxel64i                    1367      14.7  negative    (-0.02001201442163639643, -0.02001201442163632227)
  rounded types, against the qd reference -2.00120144216363534418...e-02:
  fp16       -2.08282e-02                relative error 4.1e-02
  posit16    -4.16565e-02                relative error 1.1e+00
  float      -0.0200119019               relative error 5.6e-06
  posit32    -2.0012060879e-02           relative error 2.3e-06
  double     -0.020012014421636099       relative error 1.3e-14
```

Variable definitions are **replayed in each type**: `b = 100~` is the open tile above 100
in every type's own lattice, not one type's tile carried into the others. Tile syntax
needs a tile type, so define such inputs after `type poxel32i`. Inputs written with tile
syntax have no single true value, and the rounded comparison is then left out.

### The tightest box: how much of the box is the formula's fault

Next to each box, `ubox` reports the **tightest** box the type could state, and the
**overestimation** factor: computed tiles over tightest tiles. The tightest box is
computed independently of the tile arithmetic, by the oracle in
`include/sw/universal/utility/tile_oracle.hpp`. It evaluates the expression in exact dyadic
intervals: `+ - *` are exact, and `/` and `sqrt` round outward and detect exact results.

- **Point inputs.** The value is one real number, so the tightest box is one tile. The
  oracle raises its working precision until the value's interval lies in one tile. A
  non-dyadic route to a lattice point, such as `(1/3)*3`, never separates from it. The
  count is still one tile, and a note says where it is unresolved.
- **Set inputs** (`x~`, `[a, b]`). The tightest box is the tile hull of the image of the
  inputs. An open input, such as `x~`, excludes its ends, so an extreme approached there is
  not a value the computation takes. Each value carries an interval gradient, so
  monotonicity is *proven* piece by piece, not assumed:
  - On a piece where every partial derivative has one sign, the image runs between two
    corners, which are evaluated exactly.
  - Any other piece is enclosed by the mean-value form, and either dropped or bisected
    along the input contributing most.
  - The corners and the piece centres are values the computation takes, which gives an
    inner bound; the pieces give an outer one. When the two meet the tightest box is
    proven, and otherwise it is reported as a range.

With ULP-wide coefficients, both forms of the root have the same tightest box (it belongs
to the function, not the formula). The excess is the dependency problem:

```
poxel32i> a = 3~
poxel32i> b = 100~
poxel32i> c = 2~
poxel32i> ubox (-b + sqrt(b*b - 4*a*c)) / (2*a)
  type                       tiles              tightest        over  decimals  sign        box
  areal16i                   20821                     7       2974x       0.0  undecidable (-0.0625, 0.020844)
  poxel16i                   17067                     5       3413x       0.0  undecidable (-0.10419, 0.041687)
  areal32i                    5465                     5       1093x       3.6  negative    (-0.0200169906, -0.0200068094)
  ...
poxel32i> ubox (2*c) / (-b - sqrt(b*b - 4*a*c))
  areal16i                       7                     7          1x       2.5  negative    (-0.020081, -0.019958)
  poxel16i                       9                     5        1.8x       2.4  negative    (-0.020081, -0.019928)
  areal32i                       7                     5        1.4x       6.4  negative    (-0.0200120211, -0.0200120062)
  ...
```

With `--json` each box also carries `tightest` (`inner_tiles`, `outer_tiles`, `proven`, the
box) and splits the computed width into `input_tiles`, which the inputs force, and
`formula_tiles`, which the formula adds. The oracle has no exact enclosure for `log`, `exp`
or the trigonometric functions, and reports the tightest box as unavailable for them.

`ubox trace` lists every operation with the tiles in and out, and marks where the box
grows the most. For the quadratic it is the cancellation `-b + sqrt(d)`:

```
  poxel16i                   17067       0.0  undecidable (-0.10419, 0.041687)
        6  sqrt                          3 -> 11                   (99.5, 100.25)
        7  add                      1 , 11 -> 27135                (-0.625, 0.25)   <- widest growth
```

### Decidability: `decide`

`decide <predicate> [in <types>]` answers a question about the result in each type:
**yes**, **no**, or **undecidable** when the box admits both answers. The predicate is
`sign <expr>`, or `<expr> op <expr>` with `op` one of `<  <=  >  >=  ==  !=`, decided
over every pair of points of the two boxes. It reports the narrowest type that decides it:

```
poxel32i> a = 3~
poxel32i> b = 100~
poxel32i> c = 2~
poxel32i> decide sign (-b + sqrt(b*b - 4*a*c)) / (2*a)
decide: sign (-b + sqrt(b*b - 4*a*c)) / (2*a)
  areal16i   undecidable  (-0.0625, 0.020844)
  poxel16i   undecidable  (-0.10419, 0.041687)
  areal32i   negative     (-0.0200169906, -0.0200068094)
  ...
  narrowest type that decides it: areal32i
```

Both commands take `--json` and `--csv`.

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
