# Assessment: Maximizing Claude Code's Agentic Power for Universal

## How It All Works — The Mental Model

Think of it as layers of "intelligence scaffolding":

| Layer | What it is | Analogy |
|-------|-----------|---------|
| **CLAUDE.md** | Persistent instructions loaded every session | Your team's onboarding doc |
| **Memory** | Learned patterns accumulated across sessions | An engineer's personal notes |
| **Skills** | Reusable prompt templates invoked by `/name` or auto-triggered | Runbooks / SOPs |
| **Agents** | Autonomous workers with their own context, tools, and memory | Junior engineers you delegate to |
| **Hooks** | Event-driven automation that fires on tool use | Git hooks / CI triggers |
| **MCP Servers** | External tool integrations (GitHub, databases, etc.) | IDE plugins |

The "intelligence" isn't magic — it's **context engineering**. Each markdown file pre-loads domain-specific knowledge so Claude doesn't start from zero. A skill that says "when adding a new number type, follow these 4 CMake steps and this file layout" turns a 30-minute exploration into a 2-minute execution.

## What You Have Today

| Feature | Status | Coverage |
|---------|--------|----------|
| CLAUDE.md | Good | Build commands, architecture, testing patterns |
| Memory | Good | 142 lines of hard-won lessons (build safety, portability, CMake wiring) |
| Skills | Minimal | 1 generic C++ skill (cpp-model-invariant) |
| Agents | None | No custom agents defined |
| Hooks | None | No automation triggers |
| MCP Servers | None | No external integrations |
| Commands | Basic | 4 generic C++ commands (review, modernize, optimize, wrapup) |

## What Would Make a Real Difference

Ranked by impact-to-effort ratio.

---

### 1. SKILLS — High Impact, Low Effort

These are the fastest wins. Each is a markdown file that turns a multi-step workflow into a one-command operation.

#### `/new-number-type` — Scaffold a new number system

This is the most repeated workflow. The skill encodes the exact file organization from MEMORY.md, the CMake wiring pattern (4 insertion points), the exception hierarchy architecture, and triviality requirements. Instead of remembering all the gotchas, `/new-number-type e2m1` generates the complete scaffold correctly every time.

**Key knowledge to encode:**
- File layout: `_fwd.hpp`, `exceptions.hpp`, `_impl.hpp`, `numeric_limits.hpp`, `manipulators.hpp`, `attributes.hpp`, umbrella `.hpp`
- CMake wiring: 4 insertion points (option, cascade, add_subdirectory, CI_LITE)
- Triviality: no in-class member initializers (`uint8_t _bits;` not `uint8_t _bits{0};`)
- Exception hierarchy: internal blocks have their own, never reference consuming type's
- Test structure: api/, conversion/, logic/, arithmetic/, math/
- Skeleton templates in `include/sw/universal/number/skeleton_1param/` and `skeleton_2params/`

#### `/build-test` — Safe build and test

Encodes the critical build safety rules from the load=400 incident: check for running builds, max -j4, test with both gcc AND clang.

Usage: `/build-test posit_ostream_formatting` builds the target in both `build_ci/` and `build_ci_clang/`, runs both, and reports results.

**Key knowledge to encode:**
- Check for running builds first (`pgrep -a make`)
- Max `-j4` parallelism
- Build in `build_ci/` (gcc) AND `build_ci_clang/` (clang)
- Run both executables and compare results
- Report pass/fail summary

#### `/fix-issue` — Analyze and fix a GitHub issue

Usage: `/fix-issue 509` reads the issue, explores related code, and either proposes a fix or implements one.

**Workflow:**
1. `gh issue view <number>` to read the issue
2. Identify affected number type and subsystem
3. Search for related code, tests, and similar patterns
4. Propose approach (or implement if straightforward)
5. Build and test with both compilers
6. Create branch, commit, push, open PR referencing the issue

#### `/port-feature` — Replicate a feature across number types

For when a feature is implemented for one number type and needs replication to others (like the operator<< formatting work).

Usage: `/port-feature operator<< ereal posit` looks at how ereal implements operator<<, then implements the same for posit following the same pattern.

**Key knowledge to encode:**
- Reference implementations to examine
- Adaptation points (NaN vs NaR, template parameters, exception types)
- Test structure to replicate
- CMake auto-discovery via `file(GLOB ...)`

#### `/ci-status` — Check and analyze CI

Already partially built-in, but a skill adds:
- Pull CodeRabbit comments and summarize
- Categorize failures by platform
- Suggest fixes based on known portability patterns (MSVC long double, MinGW ICF, clang uninitialized vars)

### 2. AGENTS — High Impact, Medium Effort

Agents are autonomous workers that run in their own context window. Powerful for tasks you'd otherwise supervise step-by-step.

#### `build-validator` — Dual-compiler build and test agent

Runs in background. Given a target, builds with gcc and clang, runs tests, reports results. Has build safety rules baked in. You'd never need to manually check "did I test with clang too?"

**Agent configuration:**
```yaml
name: build-validator
description: Build and test targets with both gcc and clang. Use after code changes.
tools: Bash, Read, Glob, Grep
model: haiku
background: true
```

**Key behaviors:**
- Check for running builds before starting
- Build target in `build_ci/` (gcc) first
- Build same target in `build_ci_clang/` (clang)
- Run both executables
- Report unified pass/fail summary
- Flag portability issues (different behavior between compilers)

#### `issue-analyzer` — GitHub issue triage agent

Given an issue number, reads the issue, searches the codebase for related code, checks for failing tests, and produces a structured analysis.

**Agent configuration:**
```yaml
name: issue-analyzer
description: Analyze GitHub issues and provide fix proposals. Use when investigating issues.
tools: Bash, Read, Glob, Grep, WebFetch
model: sonnet
```

**Output structure:**
- Issue summary
- Root cause hypothesis
- Affected files (with line numbers)
- Proposed approach
- Estimated complexity (trivial / moderate / significant / architectural)
- Related issues or PRs

#### `regression-guard` — Pre-commit regression checker

Before pushing, builds and runs the relevant regression tests for the files changed. If `posit_impl.hpp` was modified, it knows to run posit API, conversion, arithmetic, and math tests.

**Agent configuration:**
```yaml
name: regression-guard
description: Run regression tests for changed files before pushing. Use before git push.
tools: Bash, Read, Glob, Grep
model: haiku
background: true
```

**Key behaviors:**
- `git diff --name-only` to find changed files
- Map changed files to affected number types
- Build and run relevant test suites
- Report any regressions
- Both gcc and clang

### 3. HOOKS — Medium Impact, Low Effort

Hooks fire automatically on events. They enforce rules without you having to remember them.

#### Build safety hook (PreToolUse on Bash)

Before any `make` or `cmake --build` command, checks if another build is running. If so, blocks the command with a message. Prevents the load=400 incident from ever recurring.

**Configuration (`.claude/settings.local.json`):**
```json
{
  "hooks": {
    "PreToolUse": [
      {
        "matcher": "Bash",
        "hooks": [
          {
            "type": "command",
            "command": "cmd=$(echo $CLAUDE_TOOL_INPUT | jq -r '.command'); echo \"$cmd\" | grep -qE '\\b(make|cmake --build)\\b' && pgrep -x make > /dev/null && echo 'BLOCKED: Another make process is running. Wait for it to finish.' >&2 && exit 2 || exit 0"
          }
        ]
      }
    ]
  }
}
```

#### Desktop notification hook (Notification)

When Claude needs attention (permission prompt, build finished), sends a desktop notification so you don't have to watch the terminal.

**Configuration:**
```json
{
  "hooks": {
    "Notification": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "notify-send 'Claude Code' 'Needs your attention' --icon=dialog-information"
          }
        ]
      }
    ]
  }
}
```

#### Post-edit clang-format hook (PostToolUse)

After any file edit, automatically run `clang-format` on the changed file to maintain consistent style (if a `.clang-format` file is present).

**Configuration:**
```json
{
  "hooks": {
    "PostToolUse": [
      {
        "matcher": "Edit|Write",
        "hooks": [
          {
            "type": "command",
            "command": "file=$(echo $CLAUDE_TOOL_INPUT | jq -r '.file_path'); [ -f .clang-format ] && echo \"$file\" | grep -qE '\\.(cpp|hpp|h|c)$' && clang-format -i \"$file\" || true"
          }
        ]
      }
    ]
  }
}
```

### 4. MCP SERVERS — Medium Impact, Medium Effort

MCP servers give Claude access to external tools as first-class capabilities.

#### GitHub MCP Server

The most impactful one. Instead of Claude using `gh` CLI commands (which require Bash permissions and are clunky), a GitHub MCP server provides native tools for:
- Reading and creating issues
- Reviewing PRs with inline comments
- Checking CI status
- Managing labels and milestones

**Setup:**
```bash
claude mcp add --transport http github https://api.githubcopilot.com/mcp/
```

This makes the "analyze issues and fix them" workflow much smoother — issue content, PR reviews, and CI status become native tool calls rather than Bash/gh workarounds.

### 5. CLAUDE.md IMPROVEMENTS

The current CLAUDE.md is already good. Targeted additions:

#### Add a "Common Pitfalls" section

Move the critical lessons from MEMORY.md that apply universally:
- Build safety: one build at a time, max -j4
- Always test gcc AND clang before committing
- Triviality requirements for number types
- Exception hierarchy: internal blocks have their own, never reference consuming type's

#### Add a "Workflow Patterns" section

Document the recurring workflows:
- Adding a new number type (link to `/new-number-type` skill)
- Porting a feature across types
- CI failure triage patterns

---

## Recommended Implementation Order

| Priority | Item | Effort | Impact | Why |
|----------|------|--------|--------|-----|
| 1 | Build safety hook | 5 min | Critical | Prevents most damaging failure mode |
| 2 | `/build-test` skill | 30 min | High | Most frequently needed, biggest time saver |
| 3 | Notification hook | 5 min | Medium | Quality of life for long builds |
| 4 | `/new-number-type` skill | 1 hr | High | Encodes most complex recurring workflow |
| 5 | GitHub MCP server | 15 min | Medium | Unlocks richer issue/PR workflows |
| 6 | `/fix-issue` skill | 30 min | High | The "autonomous issue fixer" |
| 7 | `build-validator` agent | 30 min | High | Background dual-compiler testing |
| 8 | `issue-analyzer` agent | 30 min | Medium | Autonomous issue triage |
| 9 | `regression-guard` agent | 30 min | Medium | Pre-push safety net |
| 10 | `/port-feature` skill | 30 min | Medium | Cross-type feature replication |

---

## The Big Picture

The key insight: **every lesson learned the hard way (load=400, MSVC long double, clang uninitialized vars, MinGW ICF) becomes a rule that prevents the mistake from ever recurring.**

- **Skills** encode "how to do X correctly"
- **Hooks** enforce "never do Y"
- **Agents** do the tedious work autonomously
- **MCP servers** give richer tool access
- **Memory** accumulates patterns across sessions
- **CLAUDE.md** sets the baseline context

The result is that over time, working in this repo gets progressively faster — not because Claude gets smarter, but because the accumulated context engineering eliminates more and more friction.

---

## Technical Reference

### File Locations

| Item | Project scope | User scope |
|------|--------------|------------|
| Skills | `.claude/skills/<name>/SKILL.md` | `~/.claude/skills/<name>/SKILL.md` |
| Agents | `.claude/agents/<name>.md` | `~/.claude/agents/<name>.md` |
| Hooks | `.claude/settings.json` or `.claude/settings.local.json` | `~/.claude/settings.json` |
| MCP | `.mcp.json` (team, checked in) | `~/.claude.json` (personal) |
| Rules | `.claude/rules/*.md` | N/A |
| Memory | N/A | `~/.claude/projects/<id>/memory/` |

### Skill Frontmatter Reference

```yaml
---
name: skill-name
description: What this skill does (used for auto-invocation matching)
disable-model-invocation: false  # true = manual only via /skill-name
user-invocable: true             # false = Claude-only (background knowledge)
allowed-tools: Read, Grep, Glob, Bash
model: sonnet                    # sonnet, opus, haiku, or inherit
---
```

### Agent Frontmatter Reference

```yaml
---
name: agent-name
description: When to use this agent (used for auto-delegation)
tools: Bash, Read, Glob, Grep
model: haiku
background: true                 # runs concurrently
isolation: worktree              # isolated git worktree
memory: user                     # persistent memory
maxTurns: 10
---
```

### Hook Event Types

| Event | Fires when | Can block? |
|-------|-----------|-----------|
| `PreToolUse` | Before tool executes | Yes (exit 2) |
| `PostToolUse` | After tool succeeds | No |
| `Notification` | Claude needs attention | No |
| `Stop` | Claude finishes responding | Yes |
| `SessionStart` | Session begins | Yes |
| `UserPromptSubmit` | Before processing prompt | Yes |
