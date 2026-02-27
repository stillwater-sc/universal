# Conventional Commit Workflow

This document describes the development and release workflow for the Universal Numbers Library using conventional commits, git-cliff, and tag-based releases.

## Overview

The workflow separates **development** (branches + conventional commits) from **releasing** (git tags). No files on main are modified during a release, so in-flight branches never need to rebase due to version bumps.

## Conventional Commit Format

Every commit message (and PR title) follows this format:

```
<type>(<optional scope>): <description>
```

### Types

| Type       | Meaning                          | Example                                           |
|------------|----------------------------------|----------------------------------------------------|
| `feat`     | New feature                      | `feat(dfloat): add decimal256 support`             |
| `fix`      | Bug fix                          | `fix(posit): correct subnormal rounding`           |
| `docs`     | Documentation only               | `docs: update build instructions`                  |
| `style`    | Formatting, no logic change      | `style(cfloat): fix indentation`                   |
| `refactor` | Code change, no new feature/fix  | `refactor(lns): simplify encoding logic`           |
| `perf`     | Performance improvement          | `perf(blas): vectorize dot product`                |
| `test`     | Adding or fixing tests           | `test(fixpnt): add overflow edge cases`            |
| `build`    | Build system changes             | `build(cmake): add AVX-512 detection`              |
| `ci`       | CI/CD changes                    | `ci: add clang-18 to test matrix`                  |
| `chore`    | Maintenance, deps, etc.          | `chore: update docker base image`                  |

### Scopes

Scopes are optional but recommended. Use the number system name or infrastructure area:

`posit`, `cfloat`, `fixpnt`, `lns`, `integer`, `dfloat`, `hfloat`, `dd`, `qd`,
`bfloat16`, `areal`, `valid`, `einteger`, `edecimal`, `erational`, `efloat`, `ereal`,
`blockbinary`, `microfloat`, `e8m0`, `mxblock`, `nvblock`,
`blas`, `math`, `numeric`, `cmake`, `ci`, `docker`, `deps`

### Breaking Changes

Append `!` after the type to signal a breaking change:

```
feat!: redesign number system API
feat(posit)!: change template parameter order
```

This indicates a major version bump when releasing.

## Day-to-Day Development

### 1. Create a feature branch

```bash
git checkout main && git pull
git checkout -b feat/decimal256
```

Branch naming convention: `feat/`, `fix/`, `refactor/`, `docs/`, etc.

### 2. Work and commit

Use conventional commit messages for your commits:

```bash
git commit -m 'feat(dfloat): add decimal256 significand handling'
git commit -m 'test(dfloat): add decimal256 regression tests'
git commit -m 'fix(dfloat): correct overflow in wide multiply'
```

### 3. Push and create a PR

```bash
git push -u origin feat/decimal256
gh pr create --title 'feat(dfloat): add decimal256 support'
```

The PR title must follow conventional commit format. The `conventional-commits.yml` workflow validates this automatically.

### 4. Merge

Use **squash merge** on GitHub. The PR title becomes the commit message on main. This keeps main's history clean with one conventional commit per PR.

### 5. Clean up

```bash
git checkout main && git pull
git branch -d feat/decimal256
git push origin --delete feat/decimal256
```

## Releasing

Releases are **decoupled** from development. You decide when to release. Commits accumulate on main and are grouped into a release when you push a tag.

### 1. Ensure main is ready

```bash
git checkout main && git pull
```

Verify CI is green on the latest main commit.

### 2. Preview the changelog (optional)

```bash
# See what will be in the release notes
git cliff --unreleased
```

### 3. Tag the release

Use **single quotes** (bash interprets `!` in double quotes as history expansion):

```bash
# Patch release (bug fixes only)
git tag -a v4.0.1 -m 'fix: patch release description'

# Minor release (new features, backward compatible)
git tag -a v4.1.0 -m 'feat: minor release description'

# Major release (breaking changes)
git tag -a v5.0.0 -m 'feat!: major release description'
```

### 4. Push the tag

```bash
git push origin v4.1.0
```

### 5. CI does the rest

The `release.yml` workflow triggers on the tag push and:

1. Generates release notes with git-cliff from all conventional commits since the previous tag
2. Creates a GitHub Release with the structured changelog
3. Builds and pushes the Docker image to Docker Hub

No files on main are modified. No rebase cascade for in-flight branches.

## Version Resolution

CMakeLists.txt derives the version from the most recent git tag using `git describe --tags`. This means:

- **In a git checkout**: version comes from the tag automatically
- **From a source tarball** (no `.git`): falls back to the hardcoded `UNIVERSAL_FALLBACK_VERSION_*` variables in CMakeLists.txt
- **Override**: pass `-DUNIVERSAL_VERSION_MAJOR=X -DUNIVERSAL_VERSION_MINOR=Y -DUNIVERSAL_VERSION_PATCH=Z` to cmake

The fallback version should be updated periodically in a normal commit (e.g., after a major release).

## Versioning Policy

The project follows [Semantic Versioning](https://semver.org/):

- **MAJOR** (`feat!:` or `BREAKING CHANGE:` footer): incompatible API changes
- **MINOR** (`feat:`): new functionality, backward compatible
- **PATCH** (`fix:`): bug fixes, backward compatible

You decide the version number when you create the tag. The conventional commit types are a guide, not an automated enforcement.

## Tools

| Tool | Purpose | Config File |
|------|---------|-------------|
| [git-cliff](https://git-cliff.org/) | Changelog generation from conventional commits | `cliff.toml` |
| [amannn/action-semantic-pull-request](https://github.com/amannn/action-semantic-pull-request) | PR title linting | `.github/workflows/conventional-commits.yml` |

### Installing git-cliff locally

```bash
# Via cargo
cargo install git-cliff

# Via brew (macOS/Linux)
brew install git-cliff
```

### Useful git-cliff commands

```bash
# Preview unreleased changes (since last tag)
git cliff --unreleased

# Generate full changelog
git cliff -o CHANGELOG.md

# Show changes between two tags
git cliff v3.105.2..v4.0.0

# Show latest release only
git cliff --latest
```

## Quick Reference

```
Development:
  git checkout -b feat/thing        # branch
  git commit -m 'feat(x): ...'      # conventional commits
  gh pr create --title 'feat(x): ..' # PR with conventional title
  squash merge                       # clean commit on main

Releasing:
  git cliff --unreleased             # preview changelog
  git tag -a v4.1.0 -m 'description' # tag (use single quotes!)
  git push origin v4.1.0             # push tag -> CI creates release

Branch cleanup:
  git branch -d feat/thing           # delete local
  git push origin --delete feat/thing # delete remote
```
