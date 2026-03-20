# Plan: ucalc — Universal Mixed-Precision REPL Calculator (Epic #595)

## Context

Issue #595 ("Improve the command line utilities") identifies that the current 25+ one-shot CLI tools are not productive. Users must write, compile, and run C++ to compare arithmetic across types. A REPL calculator (`ucalc`) collapses this to interactive commands, becoming the single most impactful usability improvement for the library.

## Sub-Issue Breakdown

The epic decomposes into **6 implementation issues**, ordered by dependency:

### Issue 1: Type dispatch registry for ucalc
**Labels**: enhancement, ucalc
**Milestone**: MVP

Define the runtime type dispatch table that maps string names to pre-instantiated Universal types. Build on the existing `type_list` + fold-expression dispatch pattern from `blas/serialization/type_registry.hpp`.

**Scope**:
- Define a `ucalc::TypeRegistry` that maps string aliases to a dispatch function
- Support ~15 MVP types: `float`, `double`, `posit8`, `posit16`, `posit32`, `posit64`, `cfloat8`, `cfloat16` (fp16), `bfloat16`, `cfloat32` (fp32), `cfloat64` (fp64), `fixpnt16`, `fixpnt32`, `dd`, `qd`
- Each registered type provides: arithmetic ops (+,-,*,/), conversion from double, `to_binary()`, `type_tag()`, `components()`
- Use `std::function` + type-erased value wrapper (double as interchange) or `std::variant` over the fixed type set
- File: `tools/ucalc/type_dispatch.hpp`

**Key files to reuse**:
- `include/sw/blas/serialization/type_registry.hpp` — dispatch pattern
- `include/sw/blas/serialization/datafile.hpp` — type ID constants
- `include/sw/universal/number/*/manipulators.hpp` — output functions

---

### Issue 2: Expression parser and evaluator
**Labels**: enhancement, ucalc
**Milestone**: MVP
**Blocked by**: Issue 1

Implement a recursive-descent expression parser for infix arithmetic with variables.

**Scope**:
- Tokenizer: numbers (decimal, hex, scientific), identifiers, operators, parens
- Parser: standard precedence (+/- < */ < unary - < functions)
- Built-in functions: `sqrt`, `abs`, `log`, `exp`, `sin`, `cos`, `pow`
- Variable binding: `x = <expr>` stores result, `x` retrieves it
- Constants: `pi`, `e`, `maxpos`, `minpos`, `maxneg`
- AST evaluation against the current active type via the type dispatch registry
- File: `tools/ucalc/expression.hpp`

---

### Issue 3: REPL loop and command processor
**Labels**: enhancement, ucalc
**Milestone**: MVP
**Blocked by**: Issues 1, 2

Implement the interactive REPL loop with commands and expression evaluation.

**Scope**:
- REPL loop: prompt, read line, dispatch to command or expression evaluator
- Commands:
  - `type <name>` — set active type (e.g., `type posit32`)
  - `show <expr>` — evaluate and display value + binary decomposition + components
  - `compare <expr>` — evaluate across all registered types, tabular output
  - `types` — list available types
  - `vars` — list defined variables
  - `help` — command reference
  - `quit` / `exit` — exit
- Error handling: parse errors, overflow, divide-by-zero displayed as messages (not crashes)
- Stdin/pipe mode: if stdin is not a terminal, read commands without prompts (script mode)
- File: `tools/ucalc/ucalc.cpp`
- CMake: `tools/ucalc/CMakeLists.txt`, wired from `tools/CMakeLists.txt`

---

### Issue 4: Readline and history support
**Labels**: enhancement, ucalc
**Milestone**: Polished

Add GNU Readline (or libedit) integration for line editing, tab completion, and persistent history.

**Scope**:
- Optional dependency: `find_package(Readline)`, fall back to basic `std::getline` if not found
- Tab completion for: type names, variable names, command names, function names
- Persistent history: `~/.ucalc_history`
- CMake: conditional linking

---

### Issue 5: Extended type set and properties commands
**Labels**: enhancement, ucalc
**Milestone**: Polished
**Blocked by**: Issue 1

Expand the type registry from ~15 to ~30+ types and add introspection commands.

**Scope**:
- Additional types: `lns<8..32>`, `dbns` variants, `integer<8..64>`, `dfloat` (decimal32/64/128), `takum<8..32>`, `hfloat`, `dd_cascade`, `td_cascade`, `qd_cascade`
- Commands:
  - `range` — show dynamic range of current type (minpos, maxpos, epsilon)
  - `precision` — show effective precision in decimal/binary digits
  - `ulp <value>` — show ULP at the given value
  - `convert <value> from <type> to <type>` — explicit conversion with rounding analysis
  - `bits <expr>` — show raw bit pattern as hex and binary

---

### Issue 6: Sweep and error analysis commands
**Labels**: enhancement, ucalc
**Milestone**: Full
**Blocked by**: Issues 1, 2, 3

Add commands for numerical analysis workflows.

**Scope**:
- `sweep <expr> for x in [a, b, n]` — evaluate expression across a range, show error vs reference (double/qd)
- `faithful <expr>` — check if result is faithfully rounded vs higher-precision reference
- `errorplot <expr> for x in [a, b, n]` — ASCII bar chart of ULP error across range
- `dot [v1] [v2]` — dot product with quire accumulation (posit types)

## Dependency Graph

```
Issue 1 (type dispatch)
  |
  +---> Issue 2 (expression parser)
  |       |
  |       +---> Issue 3 (REPL loop) ---> Issue 4 (readline)
  |
  +---> Issue 5 (extended types)

Issue 3 ---> Issue 6 (sweep/error analysis)
```

## Verification

After each issue, the corresponding functionality can be tested:
- Issue 1: Unit test that dispatches arithmetic across all registered types
- Issue 2: Unit test that parses and evaluates expressions like `1/3 + 1/3 + 1/3`
- Issue 3: Interactive smoke test + pipe mode: `echo "type posit32; 1/3 + 1/3 + 1/3" | ucalc`
- Issue 4: Interactive test with arrow keys, tab completion
- Issue 5: `types` command shows 30+ types, `range`/`precision`/`ulp` output verified
- Issue 6: `sweep sin(x) for x in [0, 3.14159, 100]` produces tabular output
