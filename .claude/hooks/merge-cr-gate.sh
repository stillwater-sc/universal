#!/bin/bash
# merge-cr-gate.sh -- PreToolUse hook for Bash commands
#
# Policy: the PR agent cannot approve its own PR, so CodeRabbit is the outside
# reviewer. A `gh pr merge` (including --admin) is allowed when the PR has at
# least one review by CodeRabbit; it is denied when it has none.
#
# Only a single, plain `gh pr merge ...` invocation is judged. Anything with
# shell operators (; & | ` $( > < newline) gets no decision from this hook and
# falls through to the normal permission flow, so nothing can ride along on an
# approved merge.
#
# Output: PreToolUse JSON with permissionDecision allow|deny, or nothing.

input=$(cat)
cmd=$(printf '%s' "$input" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('tool_input',{}).get('command',''))" 2>/dev/null)

# not a merge: no opinion
printf '%s' "$cmd" | grep -qE '(^|[[:space:]])gh[[:space:]]+pr[[:space:]]+merge([[:space:]]|$)' || exit 0

# compound or substituted commands: no opinion (normal permission flow decides)
if printf '%s' "$cmd" | grep -qE '[;&|`<>]|\$\(' || [ "$(printf '%s' "$cmd" | wc -l)" -gt 0 ]; then
    exit 0
fi

printf '%s' "$cmd" | grep -qE '^[[:space:]]*gh[[:space:]]+pr[[:space:]]+merge([[:space:]]|$)' || exit 0

emit() {  # $1 = allow|deny, $2 = reason
    python3 -c "import json,sys; print(json.dumps({'hookSpecificOutput': {'hookEventName': 'PreToolUse', 'permissionDecision': sys.argv[1], 'permissionDecisionReason': sys.argv[2]}}))" "$1" "$2"
    exit 0
}

# PR number: the first bare integer argument after `merge` (or a /pull/N URL);
# without one, gh merges the current branch's PR
pr=$(printf '%s' "$cmd" | python3 -c "
import sys, re, shlex
try: toks = shlex.split(sys.stdin.read())
except ValueError: sys.exit(0)
i = toks.index('merge') + 1 if 'merge' in toks else len(toks)
skip_value = {'--subject','-t','--body','-b','--body-file','-F','--match-head-commit','--author-email','-A','--repo','-R'}
j = i
while j < len(toks):
    t = toks[j]
    if t in skip_value: j += 2; continue
    m = re.fullmatch(r'#?(\d+)', t) or re.search(r'/pull/(\d+)', t)
    if m: print(m.group(1)); break
    j += 1
")
repo_flag=$(printf '%s' "$cmd" | python3 -c "
import sys, shlex
try: toks = shlex.split(sys.stdin.read())
except ValueError: sys.exit(0)
for k in ('--repo','-R'):
    if k in toks and toks.index(k)+1 < len(toks): print(toks[toks.index(k)+1]); break
")

args=()
[ -n "$pr" ] && args+=("$pr")
[ -n "$repo_flag" ] && args+=(--repo "$repo_flag")

result=$(gh pr view "${args[@]}" --json number,reviews \
    -q '"\(.number) \([.reviews[] | select(.author.login | test("^coderabbit"; "i"))] | length)"' 2>/dev/null)
if [ -z "$result" ]; then
    exit 0   # could not query GitHub: no opinion, normal permission flow decides
fi
num=${result%% *}
count=${result##* }

if [ "$count" -ge 1 ] 2>/dev/null; then
    emit allow "PR #$num has $count CodeRabbit review(s): CodeRabbit is the outside reviewer for agent-authored PRs, so merging (including --admin) is permitted by project policy."
else
    emit deny "PR #$num has no CodeRabbit review yet. Project policy: an agent-authored PR merges only after at least one CodeRabbit review cycle. Wait for CodeRabbit to review, then retry."
fi
