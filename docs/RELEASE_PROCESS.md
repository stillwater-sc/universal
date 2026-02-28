# Universal Library Release Process

This document describes the release process for the Universal Numbers Library (v4+). The process uses **conventional commits**, **git tags**, and **git-cliff** for changelog generation.

## Overview

Development and releasing are fully decoupled. No files on `main` are modified during a release -- you simply push an annotated tag and CI handles the rest. This eliminates rebase cascades for in-flight branches.

```
feature branch → PR (squash merge) → main → git tag → GitHub Release + Docker
```

## Conventional Commits

All PR titles must follow the [Conventional Commits](https://www.conventionalcommits.org/) format. This is enforced by CI on every pull request.

### Format

```
<type>(<optional scope>): <description>
```

### Types

| Type | Purpose | Changelog Section |
|------|---------|-------------------|
| `feat` | New feature | Added |
| `fix` | Bug fix | Fixed |
| `perf` | Performance improvement | Performance |
| `refactor` | Code change (no new feature/fix) | Changed |
| `docs` | Documentation only | Documentation |
| `test` | Adding or fixing tests | Testing |
| `build` | Build system changes | Build System |
| `ci` | CI/CD changes | CI/CD |
| `style` | Formatting, no logic change | Styling |
| `chore` | Maintenance, deps, etc. | Miscellaneous |

### Scopes (optional)

**Number systems**: `posit`, `cfloat`, `fixpnt`, `lns`, `integer`, `areal`, `valid`, `bfloat16`, `dfloat`, `hfloat`, `dd`, `qd`, `dd_cascade`, `td_cascade`, `qd_cascade`, `einteger`, `edecimal`, `erational`, `efloat`, `ereal`

**Building blocks**: `blockbinary`, `blockdecimal`, `blocksignificant`, `blocktriple`, `microfloat`, `e8m0`, `mxblock`, `nvblock`

**Infrastructure**: `blas`, `math`, `numeric`, `cmake`, `ci`, `docker`, `deps`

### Breaking Changes

Append `!` after the type to signal a breaking change:

```
feat!: redesign number system API
feat(posit)!: change template parameter order
```

### Examples

```
feat(dfloat): add decimal256 significand handling
fix(posit): correct rounding for subnormal conversion
docs: update installation instructions
test(cfloat): add exhaustive 8-bit verification
ci: add RISC-V cross-compilation target
refactor(blockbinary): simplify division algorithm
perf(blas): vectorize dot product inner loop
chore: update fallback version to 4.1.0
```

## Day-to-Day Development

### 1. Create a Feature Branch

```bash
git checkout main && git pull
git checkout -b feat/decimal256
```

Branch naming is flexible -- use descriptive names like `feat/decimal256`, `fix/posit-rounding`, `docs/api-guide`.

### 2. Develop and Commit

Use conventional commit messages:

```bash
git commit -m 'feat(dfloat): add decimal256 significand handling'
git commit -m 'test(dfloat): add decimal256 regression tests'
git commit -m 'fix(dfloat): correct overflow in wide multiply'
```

### 3. Create a Pull Request

```bash
git push -u origin feat/decimal256
gh pr create --title 'feat(dfloat): add decimal256 support' --body '...'
```

The PR title must follow conventional commit format -- CI will reject it otherwise.

### 4. Merge

Use **squash merge** on GitHub. The PR title becomes the single commit message on `main`, which feeds directly into the changelog.

### 5. Clean Up

```bash
git checkout main && git pull
git branch -d feat/decimal256
```

## Creating a Release

### 1. Ensure main is ready

```bash
git checkout main && git pull
```

Verify CI is green on the latest commit.

### 2. Preview the changelog (optional)

```bash
git cliff --unreleased
```

### 3. Choose a version

Follow [Semantic Versioning](https://semver.org/):

| Change Type | Version Bump | Example |
|-------------|-------------|---------|
| Bug fixes only | Patch | `v4.0.0` → `v4.0.1` |
| New features (backward compatible) | Minor | `v4.0.1` → `v4.1.0` |
| Breaking changes | Major | `v4.1.0` → `v5.0.0` |

You decide the version when creating the tag. Conventional commit types are a guide, not automated enforcement.

### 4. Tag and push

```bash
git tag -a v4.1.0 -m 'feat: mixed-precision SDK and block format benchmarks'
git push origin v4.1.0
```

### 5. CI handles the rest

The `release.yml` workflow triggers on the tag push and automatically:

1. Generates release notes from commit history using **git-cliff**
2. Creates a **GitHub Release** with the changelog
3. Builds and pushes a **Docker image** to Docker Hub (`stillwater/universal`)

## Version Resolution

The library version is determined at CMake configure time:

| Context | Source | Example |
|---------|--------|---------|
| Git checkout | `git describe --tags --abbrev=0` | `v4.1.0` → 4.1.0 |
| Source tarball (no `.git`) | Fallback in `CMakeLists.txt` | 4.0.0 |
| Manual override | `-DUNIVERSAL_VERSION_MAJOR=X ...` | Any version |

The fallback version in `CMakeLists.txt` should be updated after major releases.

## CI Workflows

| Workflow | Trigger | Purpose |
|----------|---------|---------|
| `cmake.yml` | Push to main, PRs | Build matrix (GCC, Clang, MSVC, macOS, ARM64, RISC-V, MinGW) |
| `conventional-commits.yml` | PR open/edit | Validates PR title format |
| `release.yml` | Tag push (`v*`) | Creates GitHub Release + Docker image |
| `full-regression.yml` | Manual | Complete test suite before release |
| `sanitizers.yml` | Weekly, PRs | ASan/UBSan testing |

## Changelog Generation

Changelogs are generated by [git-cliff](https://git-cliff.org/) using the configuration in `cliff.toml`. The output groups commits by type and includes scope, PR number, and author:

```markdown
## [4.1.0] - 2026-02-28

### Added
- **dfloat**: portable blockbinary storage and string I/O (#524) by @Ravenwater

### Fixed
- **posit**: correct subnormal rounding edge case (#525) by @contributor

### Documentation
- update installation guide (#526) by @contributor
```

Useful local commands:

```bash
git cliff --unreleased              # Preview unreleased changes
git cliff --latest                  # Show latest release notes
git cliff v4.0.0..v4.1.0           # Between two tags
git cliff -o CHANGELOG.md          # Generate full changelog file
```

Install git-cliff via `cargo install git-cliff` or `brew install git-cliff`.

## Troubleshooting

### Release not created after tag push

1. Check the Actions tab for the `release.yml` workflow run
2. Verify the tag matches the pattern `v[0-9]*`
3. Ensure the tag is annotated (`git tag -a`), not lightweight

### Tag already exists

```bash
# Delete and recreate
git tag -d v4.1.0
git push origin :refs/tags/v4.1.0
git tag -a v4.1.0 -m 'feat: release description'
git push origin v4.1.0
```

### Manual release (fallback)

```bash
gh release create v4.1.0 --title "Universal v4.1.0" --generate-notes
```

### PR title rejected by CI

Check that the title follows `<type>(<scope>): <description>` format. The type must be one of the 10 allowed types. The scope (if used) must be one of the allowed scopes listed above.
