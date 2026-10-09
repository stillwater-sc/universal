#!/usr/bin/env bash
#
# ci-apt-install.sh - apt-get update + install for CI, with a time limit and a retry.
#
# A healthy runner installs our toolchains in under four minutes (MinGW-w64 + Wine, the
# slowest, averages 90 s with a worst case near 200 s). A sick one -- a stalled mirror, a
# stuck dpkg lock -- hangs indefinitely: one MinGW job sat 54 minutes in apt before it was
# cancelled. So each attempt gets a time limit, and a timed-out or failed attempt is retried
# after repairing any half-finished install. A fresh attempt usually succeeds.
#
# Usage:
#   tools/scripts/ci-apt-install.sh [--i386] [--no-install-recommends] package ...
#
#   --i386                   add the i386 architecture first (Wine needs it)
#   --no-install-recommends  passed through to apt-get install
#
# Environment:
#   CI_APT_ATTEMPTS  attempts before giving up            (default 2)
#   CI_APT_LIMIT     seconds allowed per attempt          (default 300)
#
# apt waits up to 60 s for a dpkg lock held by another process instead of failing at once
# (DPkg::Lock::Timeout). Lock files are never deleted: removing a lock that a live process
# holds corrupts the package database.
#
# Worst case: two attempts and one repair, about 12.5 minutes. Pair it with a step-level
# timeout-minutes of 15 as a backstop.

set -euo pipefail

attempts="${CI_APT_ATTEMPTS:-2}"
limit="${CI_APT_LIMIT:-300}"
repair_limit=120
install_opts=()
packages=()
for arg in "$@"; do
	case "$arg" in
		--i386)                  sudo dpkg --add-architecture i386 ;;
		--no-install-recommends) install_opts+=("$arg") ;;
		*)                       packages+=("$arg") ;;
	esac
done
if [ "${#packages[@]}" -eq 0 ]; then
	echo "usage: $0 [--i386] [--no-install-recommends] package ..." >&2
	exit 2
fi

for ((attempt = 1; attempt <= attempts; ++attempt)); do
	start=$SECONDS
	if sudo timeout --kill-after=10 "$limit" bash -c \
		'apt-get -o DPkg::Lock::Timeout=60 update && DEBIAN_FRONTEND=noninteractive apt-get -o DPkg::Lock::Timeout=60 install -y "$@"' \
		_ "${install_opts[@]+"${install_opts[@]}"}" "${packages[@]}"; then
		echo "apt: installed ${packages[*]} in $((SECONDS - start)) s (attempt $attempt)"
		exit 0
	fi
	echo "::warning::apt attempt $attempt of $attempts failed or exceeded ${limit} s after $((SECONDS - start)) s"
	[ "$attempt" -lt "$attempts" ] || break
	# A killed attempt can leave its own apt or dpkg children running, and packages
	# half-configured. Stop the children; the next attempt then waits for any lock that
	# is still held (DPkg::Lock::Timeout) rather than deleting it.
	sudo killall -q apt-get dpkg 2>/dev/null || true
	sleep 5
	# finish any interrupted configuration, under its own time limit
	if ! sudo timeout --kill-after=10 "$repair_limit" dpkg --configure -a; then
		echo "::error::dpkg --configure -a failed or exceeded ${repair_limit} s: the runner's package state is broken, re-run the job"
		exit 1
	fi
done
echo "::error::apt could not install ${packages[*]} in $attempts attempts of ${limit} s: the runner environment looks unhealthy, re-run the job"
exit 1
