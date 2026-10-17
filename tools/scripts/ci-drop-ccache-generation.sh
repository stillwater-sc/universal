#!/usr/bin/env bash
# ci-drop-ccache-generation.sh: delete the existing ccache generations of one artifact on
# main, so that the ccache-action post step SAVES INTO an empty slot instead of stacking a
# second generation next to the one the job restored.
#
# usage: ci-drop-ccache-generation.sh <artifact>     (GH_TOKEN and GITHUB_REPOSITORY set)
#
# Why: hendrikmuhs/ccache-action saves a new timestamped key, ccache-<artifact>-<ISO8601>,
# on every push to main. Until the Prune workflow runs after the whole CMake workflow, the
# old and new generations of every artifact coexist, which doubles the footprint for the
# length of a run. When that crossed the cache budget the cache went read-only mid-run:
# the jobs that finish early saved, and every job that finishes late failed with "Unable
# to reserve cache". The late finishers are the slow jobs, so they never got a cache and
# stayed slow (POWER, RISC-V and macOS x64 at 0% hits).
#
# The match is anchored on the timestamp, so an artifact that is a prefix of another
# (linux-x64-gcc vs linux-x64-gcc-fast) only deletes its own entries. A failed delete is
# reported but does not fail the job: the worst case is the old doubling, which the Prune
# workflow still cleans up.
set -euo pipefail

artifact="${1:?usage: ci-drop-ccache-generation.sh <artifact>}"
repo="${GITHUB_REPOSITORY:?GITHUB_REPOSITORY is not set}"

# A failed listing is a warning too: this step runs after a good build, and failing the job
# over cache housekeeping would be worse than the doubling it prevents
if ! ids=$(gh api --paginate "repos/$repo/actions/caches?per_page=100&ref=refs/heads/main" \
        --jq ".actions_caches[] | select(.key | test(\"^ccache-${artifact}-[0-9]{4}-[0-9]{2}-[0-9]{2}T\")) | \"\\(.id) \\(.key) \\(.size_in_bytes)\""); then
  echo "warning: could not list the ccache generations of ${artifact}; the Prune workflow will"
  exit 0
fi

if [ -z "$ids" ]; then
  echo "no earlier ccache generation of ${artifact} on main"
  exit 0
fi

while read -r id key size; do
  if gh api -X DELETE "repos/$repo/actions/caches/$id" >/dev/null; then
    echo "deleted ${key} ($((size / 1048576)) MB)"
  else
    echo "warning: could not delete ${key}; the Prune workflow will"
  fi
done <<< "$ids"
