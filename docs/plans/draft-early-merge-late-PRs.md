# Plan: Tiered CI Workflow — Draft PR Early, Merge PR Late

## Context

The current CI runs the full 11-platform matrix on every push and every PR (including drafts), burning ~2000+ billable minutes for a typical 5-push development cycle. This discourages the "Draft PR early" workflow that enables GitHub's excellent design review process. The fix: add draft-PR detection, path filtering, and split the primary workflow into fast/full tiers.

**Goal**: Draft PRs get fast feedback (~15 min, 2 Linux jobs). Ready-for-review PRs and merges to main get the full matrix (~45 min, 11 platforms + sanitizers + coverage + clang-tidy).

## Files to Modify

| File | Change |
|------|--------|
| `.github/workflows/cmake.yml` | Split `build` job into `fast` + `full`, add `paths-ignore`, add draft detection |
| `.github/workflows/sanitizers.yml` | Add draft-skip condition + `paths-ignore` |
| `.github/workflows/coverage.yml` | Add draft-skip condition + `paths-ignore` |
| `.github/workflows/clang-tidy.yml` | Add draft-skip condition + `paths-ignore` |

**Unchanged**: `conventional-commits.yml` (cheap, should run on drafts), `docs.yml` (properly scoped), `full-regression.yml` (manual-only), `release.yml` (tag-only).

## Behavior Matrix

| Scenario | fast | full | sanitizers | coverage | clang-tidy | commits | docs |
|----------|:----:|:----:|:----------:|:--------:|:----------:|:-------:|:----:|
| Push to feature branch (code) | YES | NO | — | — | — | — | — |
| Push to feature branch (docs only) | NO | NO | — | — | — | — | — |
| Draft PR (code) | YES | NO | NO | NO | NO | YES | YES |
| Draft PR (docs only) | NO | NO | NO | NO | NO | YES | YES |
| PR marked ready / non-draft PR (code) | YES | YES | YES | YES | YES | YES | YES |
| Push to main (code) | YES | YES | — | YES | — | — | YES |
| Push to main (docs only) | NO | NO | — | NO | — | — | YES |

## Implementation Steps

### Step 1: `cmake.yml` — Split into fast + full tiers

**1a. Triggers** — add `paths-ignore` and `ready_for_review` event type:

```yaml
on:
  push:
    paths-ignore:
      - '**.md'
      - 'docs/**'
      - 'docs-site/**'
      - '.github/FUNDING.yml'
      - 'CITATION.cff'
      - 'LICENSE'
      - '.clang-format'
      - '.clang-tidy'
      - '.gitignore'
  pull_request:
    branches: [ main ]
    types: [ opened, synchronize, reopened, ready_for_review ]
    paths-ignore:
      - '**.md'
      - 'docs/**'
      - 'docs-site/**'
      - '.github/FUNDING.yml'
      - 'CITATION.cff'
      - 'LICENSE'
      - '.clang-format'
      - '.clang-tidy'
      - '.gitignore'
```

**1b. `fast` job** — Linux GCC + Clang with `CI_LITE`, runs on every trigger (including drafts):

- 2-entry matrix: `ubuntu-latest` with GCC and Clang
- Both use `-DUNIVERSAL_BUILD_CI_LITE=ON` (~15 min)
- Concurrency group: `cmake-fast-<artifact>-<ref>` (separate from full)
- Steps: checkout → install clang (if needed) → ccache → configure → build → test → rerun failed → upload logs
- Same step structure as current `build` job, minus cross-compilation installs and Windows/macOS specifics

**1c. `full` job** — current 11-platform matrix, skipped for drafts and feature-branch pushes:

```yaml
if: |
  (github.event_name == 'push' && github.ref == 'refs/heads/main') ||
  (github.event_name == 'pull_request' && github.event.pull_request.draft == false)
```

- Exact same matrix and steps as the current `build` job (all 11 entries)
- Concurrency group: `cmake-<artifact>-<ref>` (unchanged)
- `fast` and `full` run in parallel (no `needs:` dependency) — when the full matrix is needed, total wall time stays the same

### Step 2: `sanitizers.yml` — Skip for drafts

Add to triggers:
```yaml
pull_request:
  branches: [ main ]
  types: [ opened, synchronize, reopened, ready_for_review ]
  paths-ignore:
    - '**.md'
    - 'docs/**'
    - 'docs-site/**'
    - '.github/FUNDING.yml'
    - 'CITATION.cff'
    - 'LICENSE'
```

Add to job:
```yaml
if: |
  github.event_name != 'pull_request' ||
  github.event.pull_request.draft == false
```

This preserves schedule and manual triggers while skipping draft PRs.

### Step 3: `coverage.yml` — Skip for drafts

Same pattern as sanitizers. Add `types` + `paths-ignore` to `pull_request` trigger. Add `paths-ignore` to `push` trigger. Add draft-skip `if` condition to the `coverage` job.

### Step 4: `clang-tidy.yml` — Skip for drafts

Add `types` + `paths-ignore` to trigger. Add to job:
```yaml
if: github.event.pull_request.draft == false
```

Simpler condition since this workflow only fires on `pull_request`.

## Key Design Decisions

1. **CI_LITE for fast tier, not CI**: `CI_LITE` builds a representative subset (internals, one of each number system category, MX block types) in ~15 min. Full CI builds everything in ~45 min. CI_LITE is sufficient to catch compilation errors during development.

2. **No `needs:` dependency between fast and full**: Running them in parallel means faster wall-clock time for ready PRs. The minor duplication (fast also runs when full runs) costs ~$0.02 and saves ~15 min of latency.

3. **`paths-ignore` on both push and pull_request**: Pure-docs commits skip CI entirely. Mixed docs+code commits still trigger CI (GitHub triggers if ANY file outside the ignore list changes).

4. **`ready_for_review` event type**: Without this, transitioning draft→ready would NOT re-trigger CI. This is the mechanism that makes the workflow work.

## Edge Cases

- **Draft→ready transition**: Fires `ready_for_review` event with `draft == false` → full matrix runs. No extra push needed.
- **Ready→draft conversion**: GitHub has no `converted_to_draft` event. Existing jobs continue. Concurrency groups cancel them on next push.
- **Pure-docs PR marked ready**: `paths-ignore` suppresses the workflow. Correct — nothing to build.
- **`paths-ignore` evaluates HEAD commit diff**, not full PR diff. If the last push only touches docs, CI is skipped even if earlier commits touched code. Acceptable: the earlier commits already triggered CI.

## Verification

1. Create a draft PR with a code change → verify only `fast` jobs + `conventional-commits` run
2. Push a docs-only commit to the draft → verify no cmake/sanitizer/coverage/clang-tidy jobs run
3. Mark the draft PR ready → verify full matrix + sanitizers + coverage + clang-tidy all trigger
4. Push to main (merge) → verify `fast` + `full` + coverage run
5. Check GitHub Actions UI: draft PR should show 2 green checks (fast tier), not 15+

## Estimated Savings

Typical development cycle (5 pushes to draft PR, then mark ready):
- **Before**: 5 × 15 jobs = 75 job-runs (~2075 billable minutes)
- **After**: 5 × 2 jobs + 1 × 17 jobs = 27 job-runs (~595 billable minutes)
- **Reduction**: ~71% fewer CI minutes
