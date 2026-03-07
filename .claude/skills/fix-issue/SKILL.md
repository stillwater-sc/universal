---
name: fix-issue
description: Analyze a GitHub issue, explore related code, and propose or implement a fix. Use when the user asks to investigate, fix, or resolve a GitHub issue.
user-invocable: true
argument-hint: "<issue-number>"
allowed-tools: Bash, Read, Edit, Write, Glob, Grep, WebFetch, Task
---

# Analyze and Fix a GitHub Issue

Given a GitHub issue number, investigate the problem, find the relevant code, and either propose or implement a fix.

## Arguments

`$ARGUMENTS` — the GitHub issue number (e.g., `509`). If not provided, ask the user.

## Phase 1: Understand the Issue

Read the issue and extract key information:

```bash
gh issue view $ARGUMENTS --repo stillwater-sc/universal --json title,body,labels,state,comments
```

From the issue, identify:
- **What's broken or missing** — the core problem statement
- **Which number type(s)** are affected (posit, cfloat, lns, fixpnt, dd, qd, ereal, etc.)
- **Which subsystem** — api, conversion, arithmetic, math, operator<<, exceptions, etc.
- **Severity** — crash, wrong results, missing feature, documentation, performance
- **Labels** — enhancement, bug, help wanted, Epic, etc.
- **Any code snippets or reproduction steps** in the issue body or comments

## Phase 2: Locate Related Code

Based on the issue analysis, search the codebase:

1. **Find the relevant type's implementation:**
   ```text
   include/sw/universal/number/TYPE/TYPE_impl.hpp
   ```

2. **Search for keywords from the issue:**
   - Function names, error messages, type names mentioned in the issue
   - Use Grep and Glob to find related files

3. **Check existing tests** for the affected area:
   ```text
   static/{category}/TYPE/api/
   static/{category}/TYPE/arithmetic/
   static/{category}/TYPE/conversion/
   static/{category}/TYPE/math/
   ```
   Examples: `static/fixpnt/binary/api/`, `static/tapered/posit/api/`, `static/block/microfloat/api/`

4. **Check if there's a related PR or branch:**
   ```bash
   gh pr list --repo stillwater-sc/universal --state all --search "related keywords"
   ```

## Phase 3: Assess Complexity

Classify the fix:

| Complexity | Criteria | Action |
|-----------|----------|--------|
| **Trivial** | Typo, missing include, simple one-line fix | Implement directly |
| **Moderate** | New function, algorithm fix, test addition (1-3 files) | Implement with user confirmation |
| **Significant** | Cross-type change, new subsystem, architectural (4+ files) | Present plan, ask user before implementing |
| **Epic** | Major feature, multi-PR effort | Present roadmap, suggest breakdown into sub-issues |

## Phase 4: Implement the Fix

### For Trivial/Moderate fixes:

1. **Create a feature branch:**
   ```bash
   git checkout -b fix/issue-NNN-short-description
   ```

2. **Make the code changes** — follow existing patterns in the codebase

3. **Build and test with BOTH compilers:**
   - Build with gcc: `cmake --build --preset gcc-debug --target TARGET`
   - Run gcc test
   - Build with clang: `cmake --build --preset clang-debug --target TARGET`
   - Run clang test
   - NEVER skip the clang build — CI uses clang and it catches different issues

4. **Run related regression tests** to check for regressions

5. **Commit with a descriptive message** referencing the issue:
   ```text
   fix(TYPE): description of what was fixed

   Resolves #NNN
   ```

6. **Push and create a draft PR:**
   ```bash
   git push -u origin fix/issue-NNN-short-description
   gh pr create --draft --title "fix(TYPE): short description" --body "..."
   ```
   Always create PRs as **draft** — this skips the expensive CI jobs (sanitizers, coverage,
   full 11-platform matrix) and only runs the fast tier (gcc + clang CI_LITE, ~8 min).
   When the user is satisfied, they promote with `gh pr ready NNN`.

### For Significant/Epic issues:

1. Present a structured analysis:
   ```markdown
   ## Issue #NNN: Title

   ### Root Cause
   [explanation]

   ### Affected Files
   - file1.hpp:line — what needs to change
   - file2.cpp:line — what needs to change

   ### Proposed Approach
   [step-by-step plan]

   ### Estimated Scope
   [number of files, complexity assessment]

   ### Risks
   [what could go wrong, what to watch for]
   ```

2. Ask the user how to proceed

## Critical Rules

### Build Safety
- ONE build at a time, max `-j4`
- Check `pgrep -a 'make|cmake|ninja'` before building
- Test with BOTH gcc AND clang

### Code Quality
- Follow existing patterns — read similar implementations before writing
- Number types must be trivially constructible (no in-class initializers)
- Exception hierarchy: number types inherit `universal_*`, internal blocks inherit `std::runtime_error`
- Don't use `constexpr` on constructors that call math functions

### Portability Pitfalls
- Don't use `long double` bit-shift division — use `std::ldexp()`
- Always initialize `blockbinary` temporaries (clang doesn't zero stack)
- MSVC has no `operator long double()` on `blocksignificand`
- MinGW has IPA ICF bugs with multiple template instantiations

### Issue Communication
- **If the user explicitly asks** to notify the issue thread when starting work:
  ```bash
  gh issue comment NNN --repo stillwater-sc/universal --body "Working on this. Plan: [brief description of approach]"
  ```
- **If the user explicitly asks** to notify the issue after opening a PR, post a follow-up summary:
  ```bash
  gh issue comment NNN --repo stillwater-sc/universal --body "Implemented [summary] in PR #PPP."
  ```
- Do not comment on issues automatically; always ask for confirmation first.
- Reference the issue number in commits: `Resolves #NNN` or `Fixes #NNN`
- If the fix is partial, use `Relates to #NNN` instead
- Don't close Epic issues with a single PR — they track multi-step efforts

## Common Issue Patterns

| Issue Pattern | Where to Look | Typical Fix |
|--------------|---------------|-------------|
| Wrong arithmetic results | `TYPE_impl.hpp` operators, `blockbinary` | Algorithm fix, edge case handling |
| Missing math function | `math/TYPE_math.hpp` or `mathlib.hpp` | Implement using existing patterns |
| Precision loss in output | `operator<<`, `to_string()` | Use `support::decimal` for exact conversion |
| Conversion failure | `TYPE_impl.hpp` assignment operators | Fix the conversion path |
| Build failure on platform X | Compiler-specific code paths | Portability fix with `#ifdef` or alternative |
| Missing istream support | `operator>>` in `TYPE_impl.hpp` | Implement parsing |
| Performance issue | Hot path in arithmetic operators | Profile, optimize inner loop |
| CI test failure | Test expectations vs actual behavior | Fix test or fix implementation |
