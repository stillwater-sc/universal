# POP Precision Tuning Integration Design

## Background

POP (Precision-Optimized Programs) is an automated static analysis tool for bit-level precision tuning of numerical programs, developed at the University of Perpignan (Ben Khalifa, Martel, Adje, 2021). Given a program and user-specified accuracy requirements (e.g., "variable y needs 20 significant bits"), POP determines the minimum number of bits each variable and intermediate result needs, at every control point, to meet that accuracy target.

Reference: Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning," PhD thesis, Universite de Perpignan, 2021 (https://theses.hal.science/tel-03509266).

## Problem Statement

Mixed-precision programming is a manual, error-prone process. Developers must decide which variables can use lower precision (FP16, FP32) and which require higher precision (FP64, FP128) to meet an accuracy target. This is difficult because:

- Rounding errors propagate non-linearly through computations
- Trial-and-error tools (Precimonious, CRAFT) are slow and unsound
- Existing static tools (FPTuner, Rosa) handle only straight-line code

Universal provides 30+ configurable number types but no automated guidance on which type to use where. POP's static analysis can fill this gap.

## POP Architecture

POP has two solver backends:

- **POP(SMT)**: Encodes forward + backward error analysis as propositional logic constraints solved by Z3 with binary search on a cost function.
- **POP(ILP)**: Reformulates the problem as an Integer Linear Program solved in one pass by GLPK, with optional Policy Iteration for tighter carry-bit propagation. 10-100x faster than SMT.

### Core Quantities

At each control point in the program, POP tracks two integer quantities:

- `ufp(x)` -- unit in the first place (weight of the most significant bit)
- `nsb(x)` -- number of significant bits

These are sufficient to determine the minimum precision needed for any arithmetic representation.

### Analysis Phases

1. **Range determination** (dynamic): Run the program on representative inputs to determine value ranges (ufp) at each control point.
2. **Forward analysis**: Compute how errors grow through each operation. For `z = x + y`, the forward transfer function yields `nsb_F(z)` from `nsb(x)`, `nsb(y)`, and the ufp values.
3. **Backward analysis**: Starting from the user's accuracy requirement on the output, propagate backwards to determine the minimum precision at each input and intermediate.
4. **Constraint generation**: Express the transfer functions as integer linear constraints.
5. **Solving**: Find the optimal (minimum total bits) assignment satisfying all constraints.
6. **Type mapping**: Convert `{ufp, nsb}` pairs to concrete data types.

### Transfer Functions

The forward transfer functions for the four elementary operations (from Chapter 4 of the thesis):

**Addition** `z = x + y`:
```
nsb_F(z) >= min(nsb(x) + ufp(x), nsb(y) + ufp(y)) - ufp(z) - carry(+)
```

**Subtraction** `z = x - y`:
```
nsb_F(z) >= min(nsb(x) + ufp(x), nsb(y) + ufp(y)) - ufp(z) - carry(-)
```

**Multiplication** `z = x * y`:
```
nsb_F(z) >= min(nsb(x), nsb(y)) - carry(*)
```

**Division** `z = x / y`:
```
nsb_F(z) >= min(nsb(x), nsb(y)) - carry(/)
```

The carry bit function is a key contribution: previous methods assumed carry = 1 at every operation. POP uses a refined function that adds carry only when the error from the previous computation can actually propagate, reducing over-approximation significantly.

The backward transfer functions propagate accuracy requirements from outputs to inputs:

**Addition** backward: Given `nsb(z)` required, compute minimum `nsb(x)` and `nsb(y)`:
```
nsb_B(x) >= nsb(z) + ufp(z) - ufp(x) + carry(+)
nsb_B(y) >= nsb(z) + ufp(z) - ufp(y) + carry(+)
```

### ILP Formulation

The ILP-based method (POP's most efficient solver) generates a linear number of integer constraints and variables proportional to program size. The objective function minimizes total bits:

```
minimize: sum over all control points l of nsb(l)
subject to: transfer function constraints for each operation
            nsb(l) >= nsb_B(l) for all l  (backward requirements)
            nsb(l) >= 0 for all l
```

Solved in polynomial time by a standard LP solver (GLPK).

### Key Results from the Thesis

- 38-87% reduction in total bits needed vs all-double-precision programs
- Analysis time: seconds for most benchmarks, even with loops/conditionals/arrays
- Sound static guarantees (no trial-and-error)
- Benchmarks: rotation matrices, matrix determinants, Simpson integration, PID controllers, N-body problem, pedometer/IoT algorithms, Newton-Raphson, Runge-Kutta, odometry

## Integration with Universal

### Mapping POP Output to Universal Types

POP produces bit-level precision recommendations. Universal provides the types to realize them:

| POP output (nsb) | Universal type |
|-------------------|----------------|
| 3-4 bits | `microfloat<4,*>` (e2m1, e3m0) |
| 5-8 bits | `microfloat<8,*>` (e4m3, e5m2), `cfloat<8,*>` |
| 10-11 bits | `cfloat<16,5>` (IEEE FP16) |
| 7 bits | `bfloat16` |
| 23-24 bits | `cfloat<32,8>` (IEEE FP32) |
| 52-53 bits | `cfloat<64,11>` (IEEE FP64) |
| 31 digits | `dd` (double-double) |
| N-bit fixed | `fixpnt<N+int_bits, frac_bits>` |
| Decimal fixed | `dfixpnt<ndigits, radix>` |
| Tapered | `posit<nbits, es>` |

This mapping is a unique advantage: POP can recommend arbitrary bit widths, and Universal can actually instantiate them as concrete template types.

### Proposed Components

#### 1. Transfer Function Library

A header-only library implementing POP's transfer functions as constexpr integer arithmetic:

```cpp
// include/sw/universal/mixedprecision/transfer.hpp

namespace sw::universal::mixedprecision {

struct precision_info {
    int ufp;   // unit in the first place (MSB weight)
    int nsb;   // number of significant bits
};

// Forward transfer functions
constexpr precision_info forward_add(precision_info x, precision_info y, int carry = 1);
constexpr precision_info forward_sub(precision_info x, precision_info y, int carry = 1);
constexpr precision_info forward_mul(precision_info x, precision_info y, int carry = 1);
constexpr precision_info forward_div(precision_info x, precision_info y, int carry = 1);

// Backward transfer functions
constexpr int backward_add_lhs(int nsb_z, int ufp_z, int ufp_x, int carry = 1);
constexpr int backward_add_rhs(int nsb_z, int ufp_z, int ufp_y, int carry = 1);
constexpr int backward_mul_lhs(int nsb_z, int carry = 1);
constexpr int backward_mul_rhs(int nsb_z, int carry = 1);

} // namespace sw::universal::mixedprecision
```

#### 2. Expression Graph

A lightweight DAG representation for numerical computations:

```cpp
// include/sw/universal/mixedprecision/expression_graph.hpp

namespace sw::universal::mixedprecision {

enum class OpKind { Constant, Variable, Add, Sub, Mul, Div, Sqrt, Sin, Cos };

struct ExprNode {
    OpKind op;
    int label;             // control point ID
    precision_info range;  // ufp from range analysis
    int nsb_forward;       // computed by forward pass
    int nsb_backward;      // computed by backward pass
    int nsb_final;         // min(forward, backward)
};

class ExprGraph {
    // Build expression trees
    int constant(double value, int precision);
    int variable(const std::string& name, double lo, double hi);
    int add(int lhs, int rhs);
    int mul(int lhs, int rhs);
    // ...

    // Set accuracy requirement on a node
    void require_nsb(int node, int nsb);

    // Run the analysis
    void analyze();

    // Query results
    int get_nsb(int node) const;

    // Map to Universal types
    std::string recommended_type(int node) const;
};

} // namespace sw::universal::mixedprecision
```

#### 3. ILP Solver Interface

A thin binding to GLPK (GNU LGPL) or HiGHS (MIT) for solving the constraint system:

```cpp
// include/sw/universal/mixedprecision/ilp_solver.hpp

namespace sw::universal::mixedprecision {

class ILPSolver {
    int add_variable(const std::string& name, int lo, int hi);
    void add_constraint(/* linear constraint */);
    void set_objective(/* minimize sum of nsb variables */);
    bool solve();
    int get_value(int var_id) const;
};

} // namespace sw::universal::mixedprecision
```

#### 4. Type Recommender

Maps `{ufp, nsb}` pairs to the smallest Universal type:

```cpp
// include/sw/universal/mixedprecision/type_recommender.hpp

namespace sw::universal::mixedprecision {

enum class TypeFamily { cfloat, posit, fixpnt, lns, dd, qd };

struct TypeRecommendation {
    TypeFamily family;
    unsigned nbits;
    unsigned param2;       // es for posit/cfloat, frac bits for fixpnt
    std::string type_name; // e.g. "cfloat<16,5>"
};

TypeRecommendation recommend(
    precision_info p,
    TypeFamily preferred = TypeFamily::cfloat,
    bool allow_nonstandard_widths = true
);

} // namespace sw::universal::mixedprecision
```

### Connection to Existing Infrastructure

- **Error tracker** (`docs/design/error-tracker.md`): POP's forward analysis is a formalized version of error tracking. The two can share transfer function implementations, with the error tracker providing runtime validation of POP's static bounds.
- **Block formats** (`mxfloat`, `nvblock`): POP's neural network perspective (thesis Section 10.3.2) maps to block-scaled quantization. The ILP formulation can optimize block scale selection alongside element precision.
- **`fixpnt` / `dfixpnt`**: The thesis identifies fixed-point code synthesis as a near-term goal. Universal's fixed-point types are ready targets for POP's bit-level recommendations.
- **`posit`**: The thesis explicitly mentions posit integration as future work ("we plan to integrate POSIT libraries in the near future"). Universal is the reference posit implementation.

### Implementation Phases

#### Phase 1: Transfer Functions (Small)

Implement the 10 forward/backward equations from Chapter 4 as constexpr functions. No external dependencies. Add unit tests validating against the thesis examples (rotation matrix, determinant).

#### Phase 2: Expression Graph + Backward Analysis (Medium)

Build the DAG representation. Implement backward propagation without an LP solver (use a simple iterative fixpoint on the transfer functions). This alone can provide useful precision recommendations for straight-line code.

#### Phase 3: ILP Solver Binding (Medium)

Interface to GLPK or HiGHS. Generate constraints from the expression graph. Solve for optimal bit assignments. This enables handling of programs with multiple outputs and competing precision requirements.

#### Phase 4: Policy Iteration for Carry Bits (Medium)

Implement the refined carry-bit optimization from Chapter 5. Requires iterating between the LP solver and carry-bit updates until convergence. Produces tighter (lower) precision assignments.

#### Phase 5: Code Generator (Large)

Emit mixed-precision C++ code using Universal types. Takes the expression graph + precision map and generates template-parameterized code:

```cpp
// Input: z = x * y + w
// POP output: x needs 24 bits, y needs 16 bits, w needs 11 bits, z needs 24 bits
// Generated:
cfloat<32,8> x = ...;
cfloat<16,5> y = ...;
cfloat<16,5> w = ...;
cfloat<32,8> z = static_cast<cfloat<32,8>>(x) * static_cast<cfloat<32,8>>(y)
              + static_cast<cfloat<32,8>>(w);
```

### Unique Value Proposition

What makes Universal + POP compelling vs. POP standalone:

1. **30+ number types**: POP can recommend arbitrary bit widths, and Universal can instantiate them (vs. standard IEEE which gives only 4 choices).
2. **Compile-time parametric types**: Universal's template design means POP recommendations apply at compile time: `cfloat<nsb + es, es>`.
3. **Posit + quire**: The thesis targets posit libraries. Universal is the reference implementation.
4. **Fixed-point synthesis**: The thesis identifies this as a short-term goal. Universal's `fixpnt` and `dfixpnt` are ready.
5. **Block formats**: `mxfloat`/`nvblock` enable the neural network quantization perspective from the thesis.

### Open Questions

1. **Range analysis**: POP uses dynamic range analysis (running the program on representative inputs). Should we also support static range analysis via abstract interpretation, or is dynamic sufficient for the SDK?
2. **Loop handling**: POP handles loops via policy iteration on the constraint system. For the initial integration, should we restrict to straight-line code or implement the full loop analysis?
3. **LP solver dependency**: GLPK is GNU LGPL which may conflict with Universal's MIT license for a header-only library. HiGHS (MIT license) is an alternative. Or we could implement a simple LP solver for the small constraint systems POP generates.
4. **Granularity**: POP tunes at the variable level. Should we also support expression-level tuning (different precision for each subexpression)?
5. **Integration with LLVM/Clang**: Several tools in the thesis ecosystem (TAFFO, Precimonious) work at the LLVM IR level. Should we target source-level C++ or also support IR-level analysis?
