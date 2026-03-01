# POP Precision Tuning — Developer Guide

This guide shows how to use the POP (Precision-Optimized Programs) facility in the Universal Numbers Library to determine the minimum number of bits each variable and intermediate in a numerical program needs to meet a specified accuracy target.

## Quick Start

Include the umbrella header and use the `sw::universal` namespace:

```cpp
#include <universal/mixedprecision/pop.hpp>
using namespace sw::universal;
```

Build with the POP test suite enabled:

```bash
mkdir build && cd build
cmake -DUNIVERSAL_BUILD_MIXEDPRECISION_POP=ON ..
make -j4
ctest -R pop
```

## Step-by-Step Workflow

### Step 1: Build an Expression Graph

An `ExprGraph` is a directed acyclic graph (DAG) representing your computation. Each node is either a leaf (constant or variable with a known value range) or an arithmetic operation.

```cpp
ExprGraph g;

// Leaf nodes: variables with observed value ranges [lo, hi]
int a = g.variable("a", 1.0, 10.0);
int b = g.variable("b", 0.5,  5.0);

// Constants
int k = g.constant(3.14159, "pi");

// Arithmetic operations return node IDs
int ab = g.mul(a, b);       // a * b
int sum = g.add(ab, k);     // a*b + pi
int diff = g.sub(a, b);     // a - b
int ratio = g.div(a, b);    // a / b

// Unary operations
int neg_a = g.neg(a);       // -a
int abs_b = g.abs(b);       // |b|
int root  = g.sqrt(ab);     // sqrt(a*b)
```

Each node automatically computes its value range and UFP (unit in the first place) from its inputs using conservative interval arithmetic.

### Step 2: Set Accuracy Requirements

Specify how many significant bits (NSB) the output(s) must have:

```cpp
g.require_nsb(sum, 24);   // require 24 significant bits at output
```

You can set requirements on multiple nodes. POP propagates backwards from all requirements simultaneously.

### Step 3: Analyze — Choose Your Solver

There are two analysis modes:

**Option A: Iterative Fixpoint (no LP solver, simpler, always available)**

```cpp
g.analyze();
```

This runs forward and backward transfer function passes until convergence. Good for straight-line code and single-output programs.

**Option B: LP-Based Optimal Solver (recommended)**

```cpp
PopSolver solver;
bool ok = solver.solve(g);
if (ok) {
    solver.report(std::cout, g);
}
```

This formulates the precision constraints as a Linear Program and finds the globally optimal (minimum total bits) assignment. Better for programs with multiple outputs and competing requirements.

### Step 4: Refine Carry Bits (Optional, 10-30% Reduction)

The default carry bit (`carry=1`) is conservative. `CarryAnalyzer` uses policy iteration to identify operations where `carry=0` is safe, reducing total bits:

```cpp
CarryAnalyzer ca;
int iters = ca.refine(g);  // alternates: solve LP -> update carries -> repeat
ca.report(std::cout, g);
```

### Step 5: Read Results

After analysis, query the precision assignment at each node:

```cpp
int nsb = g.get_nsb(node_id);            // significant bits assigned
const ExprNode& node = g.get_node(id);   // full node details
```

Get a type recommendation using `TypeAdvisor`:

```cpp
TypeAdvisor advisor;
std::string type_name = g.recommended_type(node_id, advisor);
// e.g. "half (cfloat<16,5>)", "single (cfloat<32,8>)"
```

Print a full report:

```cpp
g.report(std::cout);                // basic: ID, name, op, ufp, fwd, bwd, final, req
g.report(std::cout, advisor);       // with type recommendations
```

### Step 6: Generate Code (Optional)

`PopCodeGenerator` emits a C++ header with type aliases for each variable:

```cpp
PopCodeGenerator gen(g, advisor);

// Type alias header
std::string header = gen.generateHeader("MY_PRECISION_CONFIG_HPP");
// #pragma once
// using type_a = sw::universal::cfloat<16,5>;
// using type_b = sw::universal::cfloat<16,5>;
// ...

// Analysis report as comment block
std::string report = gen.generateReport();
// /* POP Precision Analysis Report
//  * a            nsb=17   -> half (cfloat<16,5>)
//  * ...
//  * Bit savings: 42.3%
//  */

// Example function using the type aliases
std::string code = gen.generateExampleCode("compute");
```

## Integration with `range_analyzer`

If you already use `range_analyzer` to profile your program (Step 2 of the mixed-precision methodology), you can feed its results directly into the expression graph:

```cpp
range_analyzer<double> ra_x, ra_y;

// ... observe values during profiling ...
for (double val : samples) {
    ra_x.observe(val);
}

ExprGraph g;
int x = g.variable("x", ra_x);   // extracts lo, hi, ufp from analyzer
int y = g.variable("y", ra_y);
```

The `range_analyzer` provides the dynamic range data that POP's static analysis needs for the UFP values at each control point.

## Complete Example: Determinant

A 2x2 matrix determinant `det = a*d - b*c` with cancellation:

```cpp
#include <universal/mixedprecision/pop.hpp>
#include <iostream>

int main() {
    using namespace sw::universal;

    ExprGraph g;

    // Matrix elements with known ranges
    int a = g.variable("a", 1.0, 10.0);
    int b = g.variable("b", 1.0, 10.0);
    int c = g.variable("c", 1.0, 10.0);
    int d = g.variable("d", 1.0, 10.0);

    // det = a*d - b*c
    int ad = g.mul(a, d);
    int bc = g.mul(b, c);
    int det = g.sub(ad, bc);

    // Require 20 significant bits at the determinant
    g.require_nsb(det, 20);

    // Solve with LP
    PopSolver solver;
    solver.solve(g);

    // Refine carries
    CarryAnalyzer ca;
    ca.refine(g);

    // Report with type recommendations
    TypeAdvisor advisor;
    g.report(std::cout, advisor);

    // Note: a,b,c,d will need MORE than 20 bits because
    // subtraction can cancel leading bits (ufp shift).
    // POP automatically accounts for this.

    return 0;
}
```

## Complete Example: Dot Product with Profiling

```cpp
#include <universal/mixedprecision/pop.hpp>
#include <universal/utility/range_analyzer.hpp>
#include <iostream>

int main() {
    using namespace sw::universal;

    // Step 1: Profile the computation
    range_analyzer<double> ra_a, ra_b, ra_prod;

    double a[] = { 1.5, -2.3, 0.7, 3.1 };
    double b[] = { 4.2,  1.8, 5.5, -0.9 };
    for (int i = 0; i < 4; ++i) {
        ra_a.observe(a[i]);
        ra_b.observe(b[i]);
        ra_prod.observe(a[i] * b[i]);
    }

    // Step 2: Build expression graph from profiled ranges
    ExprGraph g;
    int va = g.variable("a", ra_a);
    int vb = g.variable("b", ra_b);
    int accum = g.mul(va, vb);

    for (int i = 1; i < 4; ++i) {
        int ai = g.variable("a" + std::to_string(i), ra_a);
        int bi = g.variable("b" + std::to_string(i), ra_b);
        int pi = g.mul(ai, bi);
        accum = g.add(accum, pi);
    }

    g.require_nsb(accum, 16);

    // Step 3: Optimize
    PopSolver solver;
    solver.solve(g);

    CarryAnalyzer ca;
    ca.refine(g);

    // Step 4: Generate mixed-precision code
    TypeAdvisor advisor;
    PopCodeGenerator gen(g, advisor);
    std::cout << gen.generateReport();
    std::cout << gen.generateHeader();

    return 0;
}
```

## Architecture Overview

### Headers

| Header | Purpose |
|--------|---------|
| `<universal/mixedprecision/pop.hpp>` | Umbrella header — includes everything below |
| `<universal/mixedprecision/transfer.hpp>` | Forward and backward transfer functions (`constexpr`) |
| `<universal/mixedprecision/ufp.hpp>` | UFP computation, bridge to `range_analyzer` |
| `<universal/mixedprecision/expression_graph.hpp>` | `ExprGraph` DAG builder and iterative fixpoint analysis |
| `<universal/mixedprecision/simplex.hpp>` | Embedded header-only Big-M simplex LP solver |
| `<universal/mixedprecision/pop_solver.hpp>` | `PopSolver`: translates graph to LP constraints and solves |
| `<universal/mixedprecision/glpk_solver.hpp>` | Optional GLPK ILP binding (when `UNIVERSAL_HAS_GLPK` defined) |
| `<universal/mixedprecision/carry_analysis.hpp>` | `CarryAnalyzer`: policy iteration for carry-bit refinement |
| `<universal/mixedprecision/codegen.hpp>` | `PopCodeGenerator`: emits C++ type alias headers |

### Key Types

| Type | Description |
|------|-------------|
| `precision_info` | `{int ufp, int nsb}` pair with `lsb()` method |
| `ExprNode` | DAG node: op, range, precision fields, carry, consumers |
| `OpKind` | Enum: `Constant`, `Variable`, `Add`, `Sub`, `Mul`, `Div`, `Neg`, `Abs`, `Sqrt` |
| `ExprGraph` | DAG builder with `variable()`, `constant()`, arithmetic ops, `analyze()`, `report()` |
| `PopSolver` | LP-based solver: `solve(graph)`, `total_nsb()`, `report()` |
| `CarryAnalyzer` | Carry refinement: `refine(graph)`, `iterations()`, `report()` |
| `PopCodeGenerator` | Code gen: `generateHeader()`, `generateReport()`, `generateExampleCode()` |
| `SimplexSolver` | Embedded LP solver: `set_num_vars()`, `add_ge_constraint()`, `solve()` |
| `LPStatus` | Enum: `Optimal`, `Infeasible`, `Unbounded`, `MaxIterations` |

### Transfer Functions

The `constexpr` transfer functions in `transfer.hpp` can also be used standalone for manual precision reasoning:

```cpp
// Forward: given input precisions, what is the output precision?
precision_info x{3, 24};  // ufp=3, nsb=24
precision_info y{1, 16};  // ufp=1, nsb=16
auto z = forward_add(x, y, /*ufp_z=*/3, /*carry=*/1);
// z.nsb tells you the output precision

// Backward: given required output precision, what do inputs need?
int nsb_x_needed = backward_add_lhs(/*nsb_z=*/20, /*ufp_z=*/3, /*ufp_x=*/3, /*carry=*/1);
int nsb_y_needed = backward_add_rhs(/*nsb_z=*/20, /*ufp_z=*/3, /*ufp_y=*/1, /*carry=*/1);
```

### Using GLPK for Larger Problems

For problems with more than ~100 variables, the embedded simplex solver may be slow. Link against GLPK for true integer solutions and better performance:

```bash
cmake -DUNIVERSAL_BUILD_MIXEDPRECISION_POP=ON -DUNIVERSAL_USE_GLPK=ON ..
```

The `PopSolver` automatically uses `GlpkSolver` when `UNIVERSAL_HAS_GLPK` is defined; no code changes are needed.

## Concepts

### UFP (Unit in the First Place)

The UFP of a value `x` is `floor(log2(|x|))`, the exponent of the most significant bit. For a range `[lo, hi]`, the UFP is computed from the maximum absolute value. UFP determines how many bits of the representation are used for magnitude vs. precision.

### NSB (Number of Significant Bits)

The NSB of a value is `-log2(relative_error)`, measuring how many bits carry useful information. A 32-bit IEEE float has 23-24 significant bits; a 64-bit double has 52-53.

### Transfer Functions

Transfer functions describe how precision propagates through arithmetic. For addition `z = x + y`:
- **Forward**: `nsb(z) = ufp(z) - min(lsb(x), lsb(y)) + 1 + carry`
- **Backward**: `nsb(x) >= nsb(z) + ufp(z) - ufp(x) + carry`

Key insight: subtraction of nearly equal values (cancellation) causes `ufp(z) << ufp(x)`, demanding much higher input precision — POP automatically detects and handles this.

### Carry Bits

The carry bit is 0 or 1, accounting for potential rounding error propagation at each operation. The conservative default is `carry=1`. Policy iteration can prove `carry=0` when the operand error cannot affect the result, reducing total bits by 10-30%.

## Reference

Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning," PhD thesis, Universite de Perpignan, 2021. Available at https://theses.hal.science/tel-03509266.
