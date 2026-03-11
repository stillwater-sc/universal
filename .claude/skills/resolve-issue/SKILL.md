---
name: resolve-issue
description: End-to-end GitHub issue resolution — fetches the issue, analyzes it, plans a fix, creates a TODO checklist, implements the solution with dual-compiler validation, and opens a draft PR. Use when the user wants to fully resolve a GitHub issue in one shot.
user-invocable: true
argument-hint: "<issue-number>"
allowed-tools: Agent, Bash, Read, Edit, Write, Glob, Grep, WebFetch, TaskCreate, TaskGet, TaskList, TaskUpdate
---

# Resolve a GitHub Issue End-to-End

Given a GitHub issue number, drive the full lifecycle: analyze, plan, implement, test, commit, and open a draft PR.

## Arguments

`$ARGUMENTS` — the GitHub issue number (e.g., `509`). If not provided, ask the user.

---

## Phase 1: Fetch and Understand the Issue

Retrieve the issue details:

```bash
ISSUE_NUMBER="${ARGUMENTS:-}"
if ! printf '%s\n' "$ISSUE_NUMBER" | grep -Eq '^[0-9]+$'; then
  echo "Error: issue number must be a numeric ID (got: '$ISSUE_NUMBER')." >&2
  exit 1
fi

gh issue view "$ISSUE_NUMBER" --repo stillwater-sc/universal --json title,body,labels,state,comments,assignees,createdAt,updatedAt
```

Extract and summarize:
- **Title and core problem** — what's broken or missing
- **Affected number type(s)** — posit, cfloat, lns, fixpnt, dd, qd, etc.
- **Affected subsystem** — arithmetic, conversion, math, io, exceptions, etc.
- **Issue type** — bug, feature request, enhancement, documentation, Epic
- **Severity** — crash, wrong results, missing feature, performance
- **Reproduction steps** — any code snippets or test cases in the issue
- **Comments** — additional context from discussion

Present a concise summary to the user before proceeding.

---

## Phase 2: Analyze Related Code

Launch an `issue-analyzer` agent to do deep codebase exploration:

```
Agent(subagent_type="issue-analyzer", prompt="Analyze issue $ARGUMENTS for the Universal Numbers Library...")
```

The agent will:
1. Read the issue via `gh`
2. Locate affected source files and tests
3. Check for related PRs and issues
4. Produce a structured analysis report

Review the agent's output and synthesize the key findings.

---

## Phase 3: Plan the Resolution

Based on the analysis, classify the issue complexity:

| Complexity | Criteria | Approach |
|-----------|----------|----------|
| **Trivial** | Typo, missing include, one-line fix | Implement directly |
| **Moderate** | New function, algorithm fix, test addition (1-3 files) | Implement with user confirmation |
| **Significant** | Cross-type change, new subsystem (4+ files) | Present detailed plan, ask before implementing |
| **Epic** | Major feature, multi-PR effort | Present roadmap, suggest breakdown into sub-issues |

Create a structured resolution plan:

```markdown
## Resolution Plan for Issue #NNN

### Root Cause / Design Goal
[What's causing the bug or what the feature needs to achieve]

### Approach
[High-level strategy]

### Steps
1. [Step 1 — specific file and change]
2. [Step 2 — specific file and change]
3. [Step 3 — build and test]
...

### Files to Modify
- `path/to/file.hpp` — [what changes]
- `path/to/test.cpp` — [what tests to add/modify]

### Risk Assessment
- [What could go wrong]
- [What to verify doesn't break]
```

Present this plan to the user. For **Significant** or **Epic** issues, wait for explicit approval before proceeding. For **Trivial** or **Moderate** issues, proceed unless the user objects.

---

## Phase 4: Create TODO Checklist

Create tasks to track progress through the implementation:

Use `TaskCreate` to create a task for each step in the plan. The tasks should cover:

1. **Create feature branch** — `fix/issue-NNN-short-description` or `feat/issue-NNN-short-description`
2. **Implement changes** — one task per file or logical group of changes
3. **Build with gcc** — compile affected targets
4. **Test with gcc** — run affected tests
5. **Build with clang** — compile affected targets
6. **Test with clang** — run affected tests
7. **Commit changes** — with proper message referencing the issue
8. **Create draft PR** — push and open PR

Update each task's status as you work through them.

---

## Phase 5: Implement the Fix

### Step 5a: Create the Feature Branch

```bash
git checkout -b fix/issue-$ARGUMENTS-short-description main
```

Use `fix/` prefix for bugs, `feat/` for features/enhancements.

### Step 5b: Make Code Changes

Follow existing patterns in the codebase:
- Read similar implementations before writing new code
- Number types must be trivially constructible (no in-class initializers)
- Exception hierarchy: number types have their own; internal blocks have their own
- Don't use `constexpr` on constructors that call math functions (`std::frexp`, `std::ldexp`, etc.)
- Use `std::ldexp()` instead of long double bit-shift division
- Always initialize `blockbinary` temporaries (clang doesn't zero stack)

### Step 5c: Build and Test with Both Compilers

**CRITICAL Safety Rules** (violating these previously caused a load=400 server incident (CPU load average)):
- **ONE build at a time** — NEVER run concurrent builds
- **Max `-j4`** — NEVER use `-j$(nproc)`
- **Check first** — run `pgrep -a make` before every build
- **Sequential compilers** — gcc FIRST, then clang

Use the pre-configured build directories:

| Directory | Compiler | Purpose |
|-----------|----------|---------|
| `build_ci/` | gcc | Primary build |
| `build_ci_clang/` | clang | Portability validation |

Build sequence for each affected target:

```bash
# 1. Safety check
pgrep -a make

# 2. Build with gcc
cd /home/stillwater/dev/stillwater/clones/universal/build_ci && cmake --build . --target TARGET -j4

# 3. Run gcc test
./path/to/TARGET

# 4. Build with clang
cd /home/stillwater/dev/stillwater/clones/universal/build_ci_clang && cmake --build . --target TARGET -j4

# 5. Run clang test
./path/to/TARGET
```

If a build or test fails:
- Report the error clearly
- Diagnose the root cause
- Fix and retry (do NOT brute-force past failures)
- NEVER skip the clang build — CI uses clang

### Step 5d: Run Related Regression Tests

Beyond the specific target, run any closely related tests to check for regressions.

---

## Phase 6: Commit and Create Draft PR

### Step 6a: Stage and Commit

Stage only the files you changed (never `git add -A`):

```bash
git add path/to/changed/files...
```

Commit with a descriptive message:

```bash
git commit -m "$(cat <<'EOF'
fix(TYPE): description of what was fixed

Detailed explanation of the change and why it resolves the issue.

Resolves #NNN

Co-Authored-By: Claude Opus 4.6 <noreply@anthropic.com>
EOF
)"
```

Use `fix(TYPE):` for bugs, `feat(TYPE):` for features, `refactor(TYPE):` for refactoring.
For partial fixes, use `Relates to #NNN` instead of `Resolves #NNN`.
For Epic issues, never use `Resolves` — they track multi-step efforts.

### Step 6b: Push and Create Draft PR

```bash
git push -u origin fix/issue-$ARGUMENTS-short-description
```

Create the PR as **draft** to only trigger the fast CI tier (~8 min):

```bash
gh pr create --draft --title "fix(TYPE): short description" --body "$(cat <<'EOF'
## Summary
- [What this PR does]
- [Root cause / design rationale]

## Changes
- `path/to/file.hpp` — [what changed]
- `path/to/test.cpp` — [what tests were added/modified]

## Test Results
| Target | gcc build | gcc test | clang build | clang test |
|--------|-----------|----------|-------------|------------|
| target_name | OK | PASS | OK | PASS |

## Test plan
- [ ] Fast CI passes (gcc + clang CI_LITE)
- [ ] Promote to ready when satisfied: `gh pr ready NNN`

Resolves #NNN

Generated with [Claude Code](https://claude.com/claude-code)
EOF
)"
```

### Step 6c: Report the Result

Present the final summary:

```markdown
## Issue #NNN Resolved

**Branch**: `fix/issue-$ARGUMENTS-short-description`
**PR**: #PPP (draft)
**Status**: All local tests passing (gcc + clang)

### What was done
- [Summary of changes]

### Next steps
- Wait for fast CI (~8 min)
- Review CodeRabbit feedback
- When satisfied: `gh pr ready PPP`
```

---

## Critical Rules

### When to Stop and Ask
- If the issue is labeled **Epic** — present a roadmap, don't try to solve everything
- If the fix requires changes to 5+ files — present the plan first
- If the analysis reveals the issue is a duplicate or already fixed — report this
- If the issue is unclear or ambiguous — ask the user for clarification
- If a build or test fails repeatedly — report the issue, don't loop

### Code Quality Invariants
- Follow existing patterns — read similar implementations before writing
- Number types must be trivially constructible
- Exception hierarchy: number system exceptions are separate from internal block exceptions
- Test with BOTH gcc AND clang — clang catches different issues
- Reference the issue number in the commit message

### Communication
- Do NOT comment on the GitHub issue automatically
- If the user wants to notify the issue thread, they will ask explicitly
- Always present the PR URL when done so the user can review it
