#!/bin/bash
# merge-cr-gate.sh -- PreToolUse hook for Bash commands
#
# Policy: the PR agent cannot approve its own PR, so CodeRabbit is the outside
# reviewer. A `gh pr merge` (including --admin) is allowed when the PR has at
# least one review by CodeRabbit; it is denied when it has none.
#
# Only a single, plain `gh pr merge ...` invocation is judged, and only when
# every argument is one `gh pr merge` documents. Shell operators
# (; & | ` > < newline), an unknown flag or a second positional get no
# decision from this hook and fall through to the normal permission flow, so a
# parsing gap can only withhold an allow, never grant one. A shell expansion
# ($ anywhere; glob, brace or tilde in the target or --repo) is denied, since
# bash would hand gh a PR the hook never saw.
#
# The merge target (number, URL or branch) and --repo are handed to
# `gh pr view` exactly as written, so the hook checks the same PR that
# `gh pr merge` would merge.
#
# Output: PreToolUse JSON with permissionDecision allow|deny, or nothing.

CODERABBIT_LOGIN='coderabbitai[bot]'   # GitHub App bot; [] cannot occur in a user login

input=$(cat)
cmd=$(printf '%s' "$input" | python3 -c "import sys,json; d=json.load(sys.stdin); print(d.get('tool_input',{}).get('command',''))" 2>/dev/null)

# not a merge: no opinion
printf '%s' "$cmd" | grep -qE '(^|[[:space:]])gh[[:space:]]+pr[[:space:]]+merge([[:space:]]|$)' || exit 0

emit() {  # $1 = allow|deny, $2 = reason
    python3 -c "import json,sys; print(json.dumps({'hookSpecificOutput': {'hookEventName': 'PreToolUse', 'permissionDecision': sys.argv[1], 'permissionDecisionReason': sys.argv[2]}}))" "$1" "$2"
    exit 0
}

# bash expands $VAR / $(...) before gh runs, so the hook cannot know which PR
# would be merged: deny, rather than leave it to the normal permission flow
case "$cmd" in
    *'$'*) emit deny "The merge command contains a shell expansion (\$), so the PR to be merged cannot be identified. Pass the PR number or URL literally, then retry." ;;
esac

# compound commands: no opinion (normal permission flow decides)
if printf '%s' "$cmd" | grep -qE '[;&|`<>]' || [ "$(printf '%s' "$cmd" | wc -l)" -gt 0 ]; then
    exit 0
fi

# Parse against gh pr merge's flag set. Prints two lines, target then repo
# (either may be empty); exits non-zero for anything it does not recognise.
parsed=$(printf '%s' "$cmd" | python3 -c "
import sys, shlex
try: toks = shlex.split(sys.stdin.read())
except ValueError: sys.exit(1)
if toks[:3] != ['gh', 'pr', 'merge']: sys.exit(1)
long_bool  = {'--admin','--auto','--delete-branch','--disable-auto','--merge','--rebase','--squash','--help'}
long_value = {'--author-email','--body','--body-file','--match-head-commit','--subject','--repo'}
short_bool  = {'d':'--delete-branch','m':'--merge','r':'--rebase','s':'--squash'}
short_value = {'A':'--author-email','b':'--body','F':'--body-file','t':'--subject','R':'--repo'}
target, repo = [], None
args = toks[3:]
i = 0
while i < len(args):
    t = args[i]; i += 1
    if t == '--':
        target += args[i:]; break
    if t.startswith('--'):
        name, eq, val = t.partition('=')
        if name in long_bool and not eq: continue
        if name not in long_value: sys.exit(1)
        if not eq:
            if i >= len(args): sys.exit(1)
            val = args[i]; i += 1
        if name == '--repo': repo = val
        continue
    if t.startswith('-') and len(t) > 1:
        # cobra short flags: a run of bool letters, optionally ending in one
        # value letter whose value is attached or is the next argument
        j = 1
        while j < len(t):
            c = t[j]; j += 1
            if c in short_bool: continue
            if c not in short_value: sys.exit(1)
            val = t[j:]
            if val.startswith('='): val = val[1:]
            if not val:
                if i >= len(args): sys.exit(1)
                val = args[i]; i += 1
            if c == 'R': repo = val
            break
        continue
    target.append(t)
if len(target) > 1: sys.exit(1)
print(target[0] if target else '')
print(repo or '')
") || exit 0

target=$(printf '%s\n' "$parsed" | sed -n 1p)
repo_flag=$(printf '%s\n' "$parsed" | sed -n 2p)

# glob, brace and tilde expansion would also change the target bash hands gh
case "$target$repo_flag" in
    *[*?[{~]*) emit deny "The merge target or --repo contains a shell glob, brace or tilde character, so the PR to be merged cannot be identified. Pass the PR number or URL literally, then retry." ;;
esac

args=()
[ -n "$target" ] && args+=("$target")
[ -n "$repo_flag" ] && args+=(--repo "$repo_flag")

# resolve the target exactly as gh pr merge would; without a target gh uses
# the current branch's PR
url=$(gh pr view "${args[@]}" --json url -q .url 2>/dev/null)
case "$url" in
    https://github.com/*/*/pull/*) ;;
    *) exit 0 ;;   # could not resolve (or not github.com): no opinion
esac
path=${url#https://github.com/}       # OWNER/REPO/pull/N
owner=${path%%/*}; rest=${path#*/}
name=${rest%%/*}; num=${rest##*/}
case "$num" in ''|*[!0-9]*) exit 0 ;; esac

# one count per page with --paginate; sum them
counts=$(gh api --paginate "repos/$owner/$name/pulls/$num/reviews" \
    -q "[.[] | select(.user.login == \"$CODERABBIT_LOGIN\" and .user.type == \"Bot\")] | length" 2>/dev/null) || exit 0
[ -n "$counts" ] || exit 0
count=0
for c in $counts; do
    case "$c" in ''|*[!0-9]*) exit 0 ;; esac
    count=$((count + c))
done

if [ "$count" -ge 1 ]; then
    emit allow "PR $owner/$name#$num has $count CodeRabbit review(s): CodeRabbit is the outside reviewer for agent-authored PRs, so merging (including --admin) is permitted by project policy."
else
    emit deny "PR $owner/$name#$num has no CodeRabbit review yet. Project policy: an agent-authored PR merges only after at least one CodeRabbit review cycle. Wait for CodeRabbit to review, then retry."
fi
