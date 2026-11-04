---
name: qa-sweep
description: Daily QA sweep of a C++ repository. Builds and runs the tests, reviews the commits since the last sweep, takes a deep pass over one area in rotation (with sanitizers), and files the findings as one GitHub issue labelled qa-sweep. Reads the repository's .claude/qa-sweep.md for everything repo-specific. Meant for a scheduled run; read-only on the code.
disable-model-invocation: true
allowed-tools: Read Grep Glob Agent Bash(git rev-parse *) Bash(git log *) Bash(git show *) Bash(git diff *) Bash(git blame *) Bash(cmake *) Bash(ctest *) Bash(gh issue list *) Bash(gh issue view *) Bash(gh issue create *) Bash(gh issue close *) Bash(gh label list *) Bash(gh label create *) Bash(date *) Bash(nproc) Bash(mkdir *)
---

<!-- Generated from stillwater-sc/claude-plugins plugins/qa-sweep/skills/qa-sweep/SKILL.md at 1537658 by scripts/sync-qa-sweep.sh. Edit it there, not here. -->

# Daily QA sweep

You run unattended, usually from a scheduled task on a fresh clone of the
default branch. Nobody is watching, so don't stop to ask: when a step fails,
record the failure as a finding and go on to the next step.

## The repository's config

Read `.claude/qa-sweep.md` at the repository root first. It holds everything
that differs between repositories, under these headings:

- **Repository**: the `owner/name` used in every `gh` command below, written
  `<repo>` here.
- **Project**: what the code is, and which files (usually `CLAUDE.md`) state
  its rules.
- **Build and test**: the configure, build and test commands for step 2, the
  build directories, and the `-j` limit.
- **Docs-only paths**: the paths whose commits step 3 skips.
- **Areas**: the numbered list of areas for step 4, each with its headers and
  its test selection.
- **Sanitizers**: the commands for step 4's sanitizer pass, or "none".
- **Review focus**: what counts as a defect in this codebase, beyond the
  reviewer's general checklist.

If the file is missing, file no issue. Report that the repository has no
`.claude/qa-sweep.md` and stop.

## Ground rules

- **Read-only on the repository.** Don't edit tracked files, commit, push,
  open pull requests, or comment on any existing issue or PR. Your only write
  to GitHub is the one sweep issue in step 6, and creating the `qa-sweep`
  label if it is missing. The `allowed-tools` above pre-approve only the
  read-only `git` subcommands and those `gh` operations; they don't block
  anything else, so these rules still apply.
- **Builds:** only the commands and build directories the config gives, at
  its `-j` limit. Scratch programs go under `build-qa-scratch/`. Run long
  builds in the background and wait for them; a foreground command can time
  out.
- Use the `qa-reviewer` agent for the reviews in steps 3 and 4 (when this
  skill is loaded as a plugin, the agent is `qa-sweep:qa-reviewer`). Pass it
  the config's **Review focus** with every request. It knows what counts as a
  finding and what evidence to give.

## 1. Where to start

```bash
date -u +%F                         # the sweep's date
git rev-parse HEAD                  # the commit being swept
gh issue list --repo <repo> --label qa-sweep --state all \
  --limit 1 --json number,body
```

The last sweep's issue body has a line `Swept commit: <sha>`. The range for
step 3 is `<sha>..HEAD`. If there is no earlier sweep, or the sha is not in
the history, use the commits of the last 7 days (`git log --since=7.days`).

## 2. Build and run the tests

Run the config's **Build and test** commands. A compile error,
warning-as-error or failing test on the default branch is the sweep's first
finding. Give the target, the error or the failing assertion, and the last
commit that touched the code involved.

## 3. Review the commits since the last sweep

List the range with `git log --stat <range>`. Skip commits that touch only
the config's **Docs-only paths**. A commit that touches `.claude/` is never
docs-only: it can change this sweep, its reviewer or the config, so review
it. Give the rest to `qa-reviewer`, a few
related commits at a time. For each, ask it to:

- check the change for functional bugs and performance problems;
- check whether a test reaches each new or changed path, and whether a test
  added with a fix would have failed before it.

If nothing but documentation changed, say so in the issue and go on.

## 4. Deep pass over one area

Count the config's **Areas** (N), then pick today's by the day of the year:

```bash
echo $(( 10#$(date -u +%j) % N ))
```

Then:

1. **Review.** Give `qa-reviewer` the area's headers and the tests the config
   lists for it. Ask for all three kinds of finding. For a large area, split
   the headers across two or three reviewer runs.
2. **Sanitizers.** Unless the config says "none", run its **Sanitizers**
   commands with the area's test selection. Every sanitizer report is a
   functional bug. Give the test, the report's first frame inside the
   project's own sources, and the error kind.

## 5. Drop what is already reported

```bash
gh issue list --repo <repo> --label qa-sweep --state open \
  --limit 50 --json number,title,body
gh issue list --repo <repo> --state open --limit 1000 --json number,title
```

Then, for each finding, search the open issues' titles and bodies for its
file, function or symptom:

```bash
gh issue list --repo <repo> --state open --limit 20 \
  --search '<file or symbol> in:title,body' --json number,title,body
```

Leave out a finding that an open issue already reports. Mention it under
"Still open" with the issue number, and don't comment on that issue.

## 6. File the sweep issue

Create the label if it doesn't exist:

```bash
gh label list --repo <repo> --search qa-sweep --json name
gh label create qa-sweep --repo <repo> --color 5319e7 \
  --description "Findings of the daily QA sweep"
```

File one issue, titled `QA sweep YYYY-MM-DD: <area>`, labelled `qa-sweep`,
with this body:

```markdown
Swept commit: <full sha>
Range reviewed: <first sha>..<last sha> (<n> commits, <m> after skipping docs-only)
Deep pass: <area> (<k> headers)
Build and tests: <passed / which failed>
Sanitizers: <passed / which reported / none configured>

## Functional bugs
- **<title>**: `<file:line>`. <triggering input and effect>. <confirmed with a test program / unconfirmed>.

## Regression gaps
- ...

## Performance
- ...

## Still open
- #<n>: <title>

🤖 Generated with [Claude Code](https://claude.com/claude-code) by the daily QA sweep
```

Leave out a section with no entries. If the sweep found nothing, still file
the issue, saying so, and close it right away:
`gh issue close <n> --repo <repo> --reason completed`. The closed issue keeps
the `Swept commit` line for the next sweep, without adding an open issue.

If the issue can't be filed (`gh` not logged in, no network, a refused
command), print the complete title and body in your final output instead,
with the error. That output is then the only record of the sweep, and the next
run falls back to the last 7 days.

End with the issue's URL and a one-line count: bugs, gaps, performance.
