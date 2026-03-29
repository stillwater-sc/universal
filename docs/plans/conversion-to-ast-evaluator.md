# Conversion to AST-Based Expression Evaluator

## Epic #642: Rewrite Suggestion Database for Cancel Command

### Motivation

The `cancel` command detects catastrophic cancellation but cannot suggest
reformulations because:

1. Syntax-based pattern matching fires on the reformulated expression too
2. The conjugate form only helps when operands are exact inputs
3. Syntax matching cannot distinguish computed intermediates from exact inputs

Solving this requires exposing the expression tree, tracking value provenance,
and matching against known unstable patterns structurally.

### Phase Design

Each phase delivers standalone value, not just a stepping stone.

#### Phase 1: AST Node Types and Tree Builder

Add `ASTNode` variant type (Literal, Variable, Constant, BinaryOp, UnaryOp,
FunctionCall). Modify `ExpressionEvaluator` to optionally build and return the
tree. Add `ast <expr>` command that prints the expression tree.

**Standalone value**: `steps` command can show expression structure. Future
commands (simplify, differentiate) become possible.

**Scope**: expression.hpp + ucalc.cpp

**Deliverable**: `ast 1/3 + sin(x)` shows the tree structure

#### Phase 2: Provenance Tags on AST Nodes

Tag each node: `exact` (literal, variable, constant) or `computed` (operation
result). Extend `trace` to show provenance per step. Extend `cancel` to report
whether cancelled operands are exact inputs.

**Standalone value**: `cancel` can distinguish actionable cancellation (exact
inputs, fixable by algebraic rewrite) from inherent cancellation (computed
intermediates, may not be fixable).

**Scope**: expression.hpp + ucalc.cpp (trace/cancel commands)

**Deliverable**: `cancel sqrt(a) - sqrt(b)` says "operands are computed from
exact inputs a, b"

#### Phase 3: Rewrite Pattern Table

Define `RewritePattern` struct with name, description, AST template pairs
(unstable/stable), and preconditions. Populate with 7 canonical patterns:

| Unstable pattern | Stable alternative | Condition |
|---|---|---|
| `sqrt(a) - sqrt(b)` | `(a - b) / (sqrt(a) + sqrt(b))` | `a`, `b` exact inputs, `a ~= b > 0` |
| `(-b + sqrt(b^2 - 4ac)) / (2a)` | `2c / (-b - sqrt(b^2 - 4ac))` | When `-b` and `sqrt(...)` nearly cancel |
| `log(1 + x)` | `log1p(x)` | `\|x\| << 1` |
| `exp(x) - 1` | `expm1(x)` | `\|x\| << 1` |
| `1 - cos(x)` | `2 * sin(x/2)^2` | `\|x\| << 1` |
| `sin(a) - sin(b)` | `2 * cos((a+b)/2) * sin((a-b)/2)` | `a ~= b` |
| `(1 - cos(x)) / x^2` | `(sin(x/2) / (x/2))^2 / 2` | `\|x\| << 1` |

Add `rewrites` command that lists all available patterns.

**Standalone value**: Educational reference. Queryable pattern database.

**Scope**: new rewrite_patterns.hpp + ucalc.cpp

**Deliverable**: `rewrites` command lists patterns with descriptions

#### Phase 4: AST Subtree Matching

Implement structural matching: given an AST and a pattern template, find
matching subtrees. Pattern variables bind to arbitrary subtrees. Check
preconditions (provenance, magnitude). Populate `cancel`'s `suggestion` field.
Add `suggest <expr>` command.

**Standalone value**: `cancel` finally suggests rewrites. `suggest` proactively
finds unstable patterns.

**Scope**: expression.hpp (matcher) + ucalc.cpp

**Deliverable**: `cancel sqrt(a) - sqrt(b)` suggests
`(a-b)/(sqrt(a)+sqrt(b))`; `suggest` finds unstable patterns

#### Phase 5: Suggestion Verification

Evaluate both original and suggested expression in the active type. Compare
cancellation metrics (shared digits, QSNR, ULP error). Only emit suggestion
if the alternative is measurably better.

**Standalone value**: Prevents false positive suggestions. Shows side-by-side
comparison.

**Scope**: ucalc.cpp (cancel/suggest commands)

**Deliverable**: `suggest` shows "original: 6.3 shared digits, alternative:
0.2 shared digits"

### Dependency Graph

```
Phase 1 (AST builder)
    |
    v
Phase 2 (Provenance tags)
    |
    v
Phase 3 (Pattern table) --------+
    |                            |
    v                            v
Phase 4 (AST matching) <--------+
    |
    v
Phase 5 (Verification)
```

Phase 3 (pattern table) is independent of Phase 2 (provenance) and can be
done in parallel, but Phase 4 (matching) needs both.

### Risk Assessment

- **Phase 1 is the riskiest**: changing the parser from immediate-evaluation
  to AST-building could break the existing expression evaluator. Mitigation:
  keep the current evaluator working, add AST building as an optional mode.

- **Pattern matching complexity**: AST structural matching with variable
  binding is well-understood but has edge cases (commutativity, associativity).
  Start simple -- exact structural match -- then add algebraic equivalences.

- **False positives**: the verification step (Phase 5) is essential to prevent
  suggesting rewrites that don't help.

### Current State

- The `suggestion` field already exists in CancelInfo and the JSON/CSV schema
- The `operand_a_rep` / `operand_b_rep` fields in TraceStep provide lossless
  operand representations
- The expression parser is a recursive-descent Pratt parser (~400 lines)
- The trace infrastructure records each operation with operands and results
