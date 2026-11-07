#!/usr/bin/env bash
#
# check-changelog-version.sh - require a CHANGELOG.md section for a release tag.
#
# Releases are cut by pushing an annotated tag, and release.yml writes the GitHub
# Release from the commits. Nothing in that path touches CHANGELOG.md, so between
# v5.1.0 and v5.15.0 fourteen releases shipped without a CHANGELOG entry. The fix
# is a prepare-release PR before every tag, which renames [Unreleased] to the new
# version. This script is the check that the PR happened: it fails unless
# CHANGELOG.md has a heading for the version.
#
# A heading counts in either form the file uses:
#   ## [5.15.0](https://github.com/.../compare/v5.14.0...v5.15.0) (2026-11-06)
#   ## [5.15.0] - 2026-11-06
#
# Usage:
#   tools/scripts/check-changelog-version.sh v5.16.0              # before tagging
#   tools/scripts/check-changelog-version.sh v5.16.0 CHANGELOG.md
#
# Wired into CI by .github/workflows/release.yml, before the GitHub Release is
# created.

set -euo pipefail

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
	echo "usage: $0 <tag, e.g. v5.16.0> [changelog file]" >&2
	exit 2
fi

tag="$1"
changelog="${2:-CHANGELOG.md}"
version="${tag#v}"

if ! printf '%s\n' "$version" | grep -Eq '^[0-9]+\.[0-9]+\.[0-9]+([-+][0-9A-Za-z.-]+)?$'; then
	echo "error: '$tag' is not a version tag of the form vX.Y.Z" >&2
	exit 2
fi
if [ ! -f "$changelog" ]; then
	echo "error: $changelog not found" >&2
	exit 2
fi

# escape the dots (and any '+') so the version matches literally
escaped=$(printf '%s' "$version" | sed 's/[.+]/\\&/g')
if grep -Eq "^## \[${escaped}\]" "$changelog"; then
	echo "$changelog has a section for $version"
	exit 0
fi

cat >&2 <<EOF
error: $changelog has no '## [$version]' section.

Before tagging, merge a prepare-release PR that renames '## [Unreleased]' to
'## [$version](https://github.com/stillwater-sc/universal/compare/vPREV...$tag) (YYYY-MM-DD)'
and opens a new, empty '## [Unreleased]' above it. Then tag the merge commit.

If the tag is already pushed, move it to the commit that has the section:
  git push origin :refs/tags/$tag
  git tag -d $tag
  git tag -a $tag -F <message file> <commit>
  git push origin $tag
EOF
exit 1
