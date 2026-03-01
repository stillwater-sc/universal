# POP Precision Tuning — Design and Implementation

## Background

POP (Precision-Optimized Programs) is an automated static analysis tool for bit-level precision tuning of numerical programs, developed at the University of Perpignan (Ben Khalifa, Martel, Adje, 2021). Given a program and user-specified accuracy requirements (e.g., "variable y needs 20 significant bits"), POP determines the minimum number of bits each variable and intermediate result needs, at every control point, to meet that accuracy target.

Reference: Dorra Ben Khalifa, "Fast and Efficient Bit-Level Precision Tuning," PhD thesis, Universite de Perpignan, 2021 (https://theses.hal.science/tel-03509266).

See also: [POP Developer Guide](pop-developer-guide.md) for usage examples and API reference.

## Problem Statement

Mixed-precision programming is a manual, error-prone process. Developers must decide which variables can use lower precision (FP16, FP32) and which require higher precision (FP64, FP128) to meet an accuracy target. This is difficult because:

- Rounding errors propagate non-linearly through computations
- Trial-and-error tools (Precimonious, CRAFT) are slow and unsound
- Existing static tools (FPTuner, Rosa) handle only straight-line code

Universal provides 30+ configurable number types but no automated guidance on which type to use where. POP's static analysis fills this gap.

## POP Architecture

POP has two solver backends in the original thesis:

- **POP(SMT)**: Encodes forward + backward error analysis as propositional logic constraints solved by Z3 with binary search on a cost function.
- **POP(ILP)**: Reformulates the problem as an Integer Linear Program solved in one pass by GLPK, with optional Policy Iteration for tighter carry-bit propagation. 10-100x faster than SMT.

Universal implements the ILP approach with an embedded simplex solver (header-only, no external dependencies) and an optional GLPK binding for larger problems.

### Core Quantities

At each control point in the program, POP tracks two integer quantities:

- `ufp(x)` -- unit in the first place (weight of the most significant bit): `floor(log2(|x|))`
- `nsb(x)` -- number of significant bits: `-log2(relative_error)`

These are sufficient to determine the minimum precision needed for any arithmetic representation.

### Analysis Phases

1. **Range determination** (dynamic): Run the program on representative inputs to determine value ranges (ufp) at each control point. In Universal, this is done via `range_analyzer`.
2. **Forward analysis**: Compute how errors grow through each operation using transfer functions.
3. **Backward analysis**: Starting from the user's accuracy requirement on the output, propagate backwards to determine the minimum precision at each input and intermediate.
4. **Constraint generation**: Express the transfer functions as linear constraints.
5. **Solving**: Find the optimal (minimum total bits) assignment satisfying all constraints.
6. **Type mapping**: Convert `{ufp, nsb}` pairs to concrete Universal types via `TypeAdvisor`.

### Transfer Functions

Implemented in `include/sw/universal/mixedprecision/transfer.hpp` as `constexpr` functions.

**Forward functions** — given input precisions, compute output precision:

| Operation | Forward Transfer |
|-----------|-----------------|
| `z = x + y` | `nsb(z) = ufp(z) - min(lsb(x), lsb(y)) + 1 + carry` |
| `z = x - y` | Same as addition |
| `z = x * y` | `nsb(z) = nsb(x) + nsb(y) + carry` |
| `z = x / y` | `nsb(z) = nsb(x) + nsb(y) + carry` |
| `z = -x` | `nsb(z) = nsb(x)` |
| `z = |x|` | `nsb(z) = nsb(x)` |
| `z = sqrt(x)` | `nsb(z) = nsb(x) + carry` |

**Backward functions** — given required output precision, compute required input precision:

| Operation | Backward Transfer |
|-----------|------------------|
| `z = x + y` | `nsb(x) >= nsb(z) + ufp(z) - ufp(x) + carry` |
| `z = x * y` | `nsb(x) >= nsb(z) + carry` |
| `z = x / y` | `nsb(x) >= nsb(z) + carry` |
| `z = -x` | `nsb(x) = nsb(z)` |
| `z = sqrt(x)` | `nsb(x) >= nsb(z) + carry` |

The carry bit function is a key contribution: previous methods assumed `carry = 1` at every operation. POP uses a refined function that adds carry only when the error from the previous computation can actually propagate, reducing over-approximation significantly.

### LP Formulation

The LP-based method generates a linear number of constraints and variables proportional to program size:

```text
minimize: sum over all nodes i of nsb(i)
subject to: transfer function constraints for each operation
            nsb(i) >= nsb_required(i)  for output nodes
            nsb(i) >= 1               for all nodes
```

The LP relaxation (dropping integrality constraints) is solved in polynomial time by the embedded simplex solver. Integer solutions are obtained by ceiling the LP result. For true ILP solutions, use the optional GLPK binding.

### Key Results from the Thesis

- 38-87% reduction in total bits needed vs all-double-precision programs
- Analysis time: seconds for most benchmarks, even with loops/conditionals/arrays
- Sound static guarantees (no trial-and-error)
- Benchmarks: rotation matrices, matrix determinants, Simpson integration, PID controllers, N-body problem, pedometer/IoT algorithms, Newton-Raphson, Runge-Kutta, odometry

## Implementation

### Header Structure

All POP headers are in `include/sw/universal/mixedprecision/` and use namespace `sw::universal`:

| Header | Phase | Purpose |
|--------|-------|---------|
| `ufp.hpp` | 1 | UFP computation, bridge to `range_analyzer` |
| `transfer.hpp` | 1 | Forward and backward transfer functions (`constexpr`) |
| `expression_graph.hpp` | 2 | `ExprGraph` DAG builder and iterative fixpoint analysis |
| `simplex.hpp` | 3 | Embedded header-only Big-M simplex LP solver |
| `pop_solver.hpp` | 3 | `PopSolver`: translates graph to LP constraints and solves |
| `glpk_solver.hpp` | 3 | Optional GLPK ILP binding (`#ifdef UNIVERSAL_HAS_GLPK`) |
| `carry_analysis.hpp` | 4 | `CarryAnalyzer`: policy iteration for carry-bit refinement |
| `codegen.hpp` | 5 | `PopCodeGenerator`: emits C++ type alias headers |
| `pop.hpp` | — | Umbrella header including all of the above |

### Key Types

```cpp
namespace sw { namespace universal {

// Precision descriptor (transfer.hpp)
struct precision_info {
    int ufp;    // floor(log2(|x|))
    int nsb;    // number of significant bits
    constexpr int lsb() const;  // ufp - nsb + 1
};

// Operation kinds (expression_graph.hpp)
enum class OpKind : uint8_t {
    Constant, Variable, Add, Sub, Mul, Div, Neg, Abs, Sqrt
};

// Expression graph node (expression_graph.hpp)
struct ExprNode {
    OpKind op;
    int id;
    std::string name;
    int lhs{-1}, rhs{-1};         // input edges (-1 = none)
    double lo{0.0}, hi{0.0};      // value range
    int ufp{0};                    // floor(log2(max_abs))
    int nsb_forward{0};            // from forward pass
    int nsb_backward{0};           // from backward pass
    int nsb_final{0};              // result: max(forward, backward)
    int carry{1};                  // carry bit (1=conservative, 0=refined)
    int nsb_required{-1};          // user requirement (-1=none)
    std::vector<int> consumers;    // downstream nodes
};

// LP solver status (simplex.hpp)
enum class LPStatus { Optimal, Infeasible, Unbounded, MaxIterations };

}} // namespace sw::universal
```

### `ExprGraph` — DAG Builder and Fixpoint Analysis

The central class. Builds an expression DAG and runs iterative fixpoint analysis:

```cpp
class ExprGraph {
    // Construction: returns node ID
    int constant(double value, const std::string& name = "");
    int variable(const std::string& name, double lo, double hi);
    template<typename T> int variable(const std::string& name, const range_analyzer<T>& ra);
    int add(int lhs, int rhs);
    int sub(int lhs, int rhs);
    int mul(int lhs, int rhs);
    int div(int lhs, int rhs);
    int neg(int operand);
    int abs(int operand);
    int sqrt(int operand);

    // Requirements
    void require_nsb(int node_id, int nsb);

    // Analysis (iterative fixpoint — no LP solver needed)
    void analyze(int max_iterations = 20);

    // Results
    int get_nsb(int node_id) const;
    const ExprNode& get_node(int node_id) const;
    int size() const;
    std::string recommended_type(int node_id, const TypeAdvisor& = TypeAdvisor()) const;
    void report(std::ostream&) const;
    void report(std::ostream&, const TypeAdvisor&) const;

    // Access for LP solver / carry analysis
    std::vector<ExprNode>& nodes();
    const std::vector<ExprNode>& nodes() const;
};
```

Range estimation uses conservative interval arithmetic. The `variable()` overload accepting `range_analyzer<T>` extracts `lo`, `hi`, and `ufp` from the analyzer's observed values.

### `PopSolver` — LP-Based Optimization

Translates an `ExprGraph` into LP constraints and solves for the globally optimal bit assignment:

```cpp
class PopSolver {
    bool solve(ExprGraph& graph);         // generate constraints + solve + write nsb_final
    double total_nsb() const;             // total bits across all nodes
    LPStatus status() const;              // solver status
    void report(std::ostream&, const ExprGraph&) const;
};
```

The solver automatically selects the backend based on compile-time configuration:
- Default: `SimplexSolver` (embedded, header-only, no dependencies)
- With `-DUNIVERSAL_HAS_GLPK`: `GlpkSolver` (true ILP via GLPK)

### `CarryAnalyzer` — Policy Iteration

Refines carry bits via alternating LP solves and carry recomputation:

```cpp
class CarryAnalyzer {
    int refine(ExprGraph& graph, int max_iterations = 10);
    int iterations() const;
    void report(std::ostream&, const ExprGraph&) const;
};
```

The carry refinement criterion: `carry(z = x op y) = 0` when `lsb(operand_error) > ufp(z)`, meaning the operand's rounding error cannot affect the result.

### `PopCodeGenerator` — Mixed-Precision Code Emission

Generates C++ code using Universal types based on POP results:

```cpp
class PopCodeGenerator {
    explicit PopCodeGenerator(const ExprGraph& graph, const TypeAdvisor& = TypeAdvisor());
    std::string generateHeader(const std::string& guard = "POP_PRECISION_CONFIG_HPP") const;
    std::string generateReport() const;
    std::string generateExampleCode(const std::string& function_name = "compute") const;
};
```

### `SimplexSolver` — Embedded LP Solver

A header-only Big-M simplex solver for small LPs (< 100 variables):

```cpp
class SimplexSolver {
    void set_num_vars(int n);
    void set_objective(const std::vector<double>& coeffs);  // minimize
    void add_ge_constraint(const std::vector<double>& coeffs, double rhs);
    void add_le_constraint(const std::vector<double>& coeffs, double rhs);
    void add_eq_constraint(const std::vector<double>& coeffs, double rhs);
    LPStatus solve(int max_iterations = 10000);
    double get_value(int var) const;
    double objective_value() const;
    LPStatus status() const;
};
```

## Integration with Universal Infrastructure

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

This mapping is a unique advantage: POP can recommend arbitrary bit widths, and Universal can actually instantiate them as concrete template types. The `TypeAdvisor::recommendForNsb()` method performs this mapping automatically.

### Connection to Existing Infrastructure

| Component | Integration |
|-----------|-------------|
| `range_analyzer` | `ExprGraph::variable(name, ra)` extracts `lo`, `hi`, `ufp` from profiled ranges. `range_analyzer::ufp()` alias added. |
| `TypeAdvisor` | `TypeAdvisor::recommendForNsb(nsb, lo, hi)` maps NSB to concrete types. Used by `ExprGraph::recommended_type()` and `PopCodeGenerator`. |
| `PrecisionConfigGenerator` | `PopCodeGenerator::generateHeader()` follows the same output format. |
| Error tracker | POP's forward analysis is a formalized version of error tracking. The error tracker can validate POP's static bounds at runtime. |
| Block formats (`mxfloat`, `nvblock`) | POP's neural network perspective (thesis Section 10.3.2) maps to block-scaled quantization. |
| `fixpnt` / `dfixpnt` | The thesis identifies fixed-point code synthesis as a near-term goal. Universal's fixed-point types are ready targets. |
| `posit` | The thesis explicitly mentions posit integration as future work. Universal is the reference posit implementation. |

### Modifications to Existing Headers

| File | Change |
|------|--------|
| `include/sw/universal/utility/range_analyzer.hpp` | Added `int ufp() const { return maxScale(); }` convenience alias |
| `include/sw/universal/utility/type_advisor.hpp` | Added `TypeRecommendation recommendForNsb(int nsb, double lo, double hi) const` method |

## Build Configuration

### CMake Options

```bash
# Build POP tests
cmake -DUNIVERSAL_BUILD_MIXEDPRECISION_POP=ON ..

# Build with GLPK for ILP solving
cmake -DUNIVERSAL_BUILD_MIXEDPRECISION_POP=ON -DUNIVERSAL_USE_GLPK=ON ..

# POP is also included in the SDK cascade
cmake -DUNIVERSAL_BUILD_SDK=ON ..
```

### Test Suite

8 test executables in `mixedprecision/pop/`:

| Test | Validates |
|------|-----------|
| `test_transfer` | Forward/backward transfer functions (constexpr) |
| `test_ufp` | UFP computation and `range_analyzer` bridge |
| `test_expression_graph` | DAG construction, fixpoint analysis, type recommendations |
| `test_simplex` | Embedded LP solver correctness |
| `test_pop_solver` | End-to-end LP-based optimization |
| `test_carry_analysis` | Carry refinement convergence |
| `test_codegen` | Code generation output |
| `test_complete_workflow` | End-to-end: profile -> graph -> optimize -> codegen |

Run all tests:

```bash
ctest -R pop
```

## Unique Value Proposition

What makes Universal + POP compelling vs. POP standalone:

1. **30+ number types**: POP can recommend arbitrary bit widths, and Universal can instantiate them (vs. standard IEEE which gives only 4 choices).
2. **Compile-time parametric types**: Universal's template design means POP recommendations apply at compile time: `cfloat<nsb + es, es>`.
3. **Posit + quire**: The thesis targets posit libraries. Universal is the reference implementation.
4. **Fixed-point synthesis**: The thesis identifies this as a short-term goal. Universal's `fixpnt` and `dfixpnt` are ready.
5. **Block formats**: `mxfloat`/`nvblock` enable the neural network quantization perspective from the thesis.

## Future Work

1. **Loop handling**: Extend policy iteration to handle loops via widening/narrowing in the constraint system.
2. **Static range analysis**: Supplement dynamic `range_analyzer` profiling with abstract interpretation for ranges, removing the need for representative inputs.
3. **Expression-level tuning**: Support different precision for each subexpression, not just per variable.
4. **LLVM/Clang integration**: Source-level analysis via Clang AST visitors to automatically extract expression graphs from C++ code.
5. **Weighted objectives**: Support non-uniform cost functions (e.g., energy-weighted optimization using `TypeAdvisor` energy estimates).
