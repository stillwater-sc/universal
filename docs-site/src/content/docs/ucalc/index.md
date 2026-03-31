---
title: "ucalc: Universal Mixed-Precision Calculator"
description: "Interactive calculator and compute oracle for exploring arithmetic across 42+ number types"
---


`ucalc` is an interactive calculator and compute oracle for exploring,
comparing, and analyzing arithmetic across 42+ number types from the
Universal library. Instead of writing, compiling, and running C++ for
each experiment, you can compare representations, measure precision,
trace errors, and visualize arithmetic behavior interactively.

ucalc serves two audiences:

- **Humans** use the interactive REPL to explore numerical behavior,
  debug precision issues, and teach how different number systems work.
- **AI agents** use the [MCP server](../ucalc/mcp-server/) to call ucalc tools
  programmatically via JSON-RPC, making ucalc a compute oracle for
  mixed-precision analysis.

## Quick Start

```bash
# Build
cmake -DUNIVERSAL_BUILD_TOOLS_UCALC=ON ..
make ucalc

# Interactive
ucalc

# One-shot
ucalc "type posit32; show 1/3"

# Pipe mode
echo "compare sqrt(2)" | ucalc

# AI agent mode
ucalc --mcp
```

## CLI Flags

| Flag | Description |
|------|-------------|
| `--json` | JSON output for all commands |
| `--csv` | CSV output for tabular commands |
| `--quiet` | Value only, no decoration |
| `--mcp` | Run as [MCP server](../ucalc/mcp-server/) (JSON-RPC over stdio) |
| `-t <type>` | Set active type from command line |
| `-f <file>` | Execute a script file (batch mode) |

## Commands Reference

### Expression Features

Expressions support standard arithmetic (`+`, `-`, `*`, `/`, `^`), parentheses,
variables (`x = expr`), constants (`pi`, `e`, `phi`, `ln2`, `ln10`, `sqrt2`,
`sqrt3`, `sqrt5`), and functions (`sqrt`, `abs`, `log`, `exp`, `sin`, `cos`,
`tan`, `asin`, `acos`, `atan`, `pow`).

Constants are sourced at quad-double precision (~64 decimal digits) and
converted to the active type at its native precision.

### Type Inspection

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
| `oracle <type> <expr>` | Canonical result with rounding verification |
| `increment <expr>` | Show value and next representable value |
| `decrement <expr>` | Show value and previous representable value |
| `color on/off` | Toggle ANSI color-coded bit fields |
| `vars` | List defined variables |

### Numerical Forensics

| Command | Description |
|---------|-------------|
| `steps <expr>` | [Step-by-step arithmetic](../ucalc/step-by-step/) (align, add, normalize, round) |
| `trace <expr>` | Show each operation with ULP error and rounding direction |
| `cancel <expr>` | Detect catastrophic cancellation in subtractions |
| `audit <expr>` | Rounding audit trail with signed ULP drift and ties-to-even detection |
| `diverge <expr> <t1> <t2> <tol> for <var> in [a, b]` | Find where two types first disagree |
| `suggest <expr>` | Find unstable patterns and suggest rewrites |
| `rewrites` | List available numerical rewrite patterns |
| `ast <expr>` | Show expression tree structure with provenance tags |
| `numberline [lo, hi]` | ASCII visualization of representable value density |
| `heatmap` | Precision (significant bits) vs magnitude bar chart |

### Quantization Workbench

| Command | Description |
|---------|-------------|
| `quantize <fmt> [data] \| -f <file>` | Quantize data, report RMSE/QSNR/errors |
| `block <fmt> [data] \| -f <file>` | MX/NV block decomposition (scale + elements) |
| `dot [v1] [v2] [accum=<type>]` | Mixed-precision dot product with configurable accumulation |
| `clip <type> [data] \| -f <file>` | Overflow/underflow map for a distribution |

### Statistics and Verification

| Command | Description |
|---------|-------------|
| `testvec <type> <func> [a, b, n]` | Generate golden test vectors (C++/JSON/CSV) |
| `errordist <expr> for <var> in [a, b, n]` | ULP error distribution histogram |
| `stochastic <expr> N` | Simulate stochastic rounding N times |
| `histogram [lo, hi, bins]` | Representable value distribution |
| `cond [[a,b],[c,d]]` | Condition number estimation (2x2 or 3x3) |

## Available Types

ucalc registers 42 types spanning the major number system families:

| Family | Types |
|--------|-------|
| Integer | int8, int16, int32, int64 |
| Fixed-point | fixpnt16, fixpnt32 |
| Decimal fixed-point | dfixpnt8_4, dfixpnt16_8 |
| Native IEEE | float, double |
| Classic float | fp16, fp32, fp64, fp128 |
| Google Brain Float | bfloat16 |
| FP8 (Deep Learning) | fp8e2m5, fp8e3m4, fp8e4m3, fp8e5m2 |
| Logarithmic | lns8, lns16, lns32 |
| Double-base | dbns8, dbns16 |
| Posit | posit8, posit16, posit32, posit64 |
| Takum | takum8, takum16, takum32, takum64 |
| Decimal float | decimal32, decimal64 |
| Hexadecimal float | hfloat32, hfloat64 |
| Rational | rational8, rational16, rational32 |
| Multi-component | dd, dd_cascade, td_cascade, qd, qd_cascade |

## Further Reading

- [Worked Examples](../ucalc/examples/) -- 22 examples exploring precision, cancellation, quantization, and more
- [Step-by-Step Arithmetic](../ucalc/step-by-step/) -- how each number system family performs arithmetic internally
- [MCP Server](../ucalc/mcp-server/) -- AI agent integration via the Model Context Protocol
