# Universal Library Release Process

This document describes the automated release process for the Universal Numbers Library.

## Overview

The release process is **fully automated** via GitHub Actions. When a version branch is merged into `main`, a release is automatically created with the version specified in `CMakeLists.txt`.

## Workflow

### 1. Create a Version Branch

```bash
git checkout main
git pull origin main
git checkout -b v3.93  # Use the target version number
git push -u origin v3.93
```

### 2. Development

Work on issues in the version branch:

```bash
# Make changes, fix issues
git add .
git commit -m "[#123] Fix description"
git push
```

### 3. Update Version (when ready for release)

Before merging, ensure `CMakeLists.txt` has the correct version:

```cmake
set(UNIVERSAL_VERSION_MAJOR 3)
set(UNIVERSAL_VERSION_MINOR 93)
set(UNIVERSAL_VERSION_PATCH 0)
```

The version tag will be `v3.93.0` (combining MAJOR.MINOR.PATCH).

### 4. Run Full Regression

Before creating the PR, run the full regression suite:

1. Go to Actions → "Full Regression" workflow
2. Click "Run workflow"
3. Select the version branch
4. Wait for all platforms to pass

### 5. Create Pull Request

```bash
gh pr create --base main --head v3.93 --title "V3.93" --body "Release v3.93.0

## Summary
- Feature 1
- Bug fix 2
- Improvement 3

## Testing
- Full regression passed on all platforms"
```

Or use the GitHub web interface.

### 6. Merge PR

Once the PR is approved and CI passes:

1. Merge the PR into `main`
2. **Automatic**: GitHub Actions creates the tag `v3.93.0`
3. **Automatic**: GitHub Release is created with release notes
4. Delete the version branch (optional, can be done via GitHub UI)

## What Gets Automated

When a PR from a version branch (starting with `v`) is merged into `main`:

| Step | Action |
|------|--------|
| Version extraction | Reads `UNIVERSAL_VERSION_*` from `CMakeLists.txt` |
| Tag creation | Creates annotated tag `v{MAJOR}.{MINOR}.{PATCH}` |
| Release creation | Creates GitHub Release with changelog |
| Release notes | Auto-generated from commit messages since last tag |

## Version Numbering

Follow semantic versioning:

- **MAJOR**: Breaking API changes
- **MINOR**: New features, backward compatible
- **PATCH**: Bug fixes, backward compatible

Examples:
- `v3.92.0` → First release of v3.92 series
- `v3.92.1` → Patch release with bug fixes
- `v4.0.0` → Major release with breaking changes

## Troubleshooting

### Release not created after merge

1. Check that the source branch started with `v` (e.g., `v3.93`)
2. Check the Actions tab for workflow run status
3. Verify `CMakeLists.txt` has valid version numbers

### Tag already exists

If you need to recreate a release:

```bash
# Delete the tag locally and remotely
git tag -d v3.93.0
git push origin :refs/tags/v3.93.0

# Then re-run the workflow or create manually
```

### Manual release (fallback)

If automation fails, create release manually:

```bash
# Create and push tag
git checkout main
git pull
git tag -a v3.93.0 -m "Release v3.93.0"
git push origin v3.93.0

# Create release via GitHub UI or CLI
gh release create v3.93.0 --title "Universal v3.93.0" --generate-notes
```

## CI Workflows

| Workflow | Trigger | Purpose |
|----------|---------|---------|
| `cmake.yml` | Push to main/version branches, PRs | Standard CI tests |
| `full-regression.yml` | Manual | Complete test suite before release |
| `release.yml` | PR merge to main | Automated release creation |
| `sanitizers.yml` | Weekly, PRs | ASan/UBSan testing |

## Best Practices

1. **Always run full regression** before creating the merge PR
2. **Update PATCH version** for bug-fix-only releases
3. **Update MINOR version** at branch creation for new features
4. **Write descriptive commit messages** - they become release notes
5. **Reference issue numbers** in commits (e.g., `[#496] Fix description`)
