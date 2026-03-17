---
name: build-test
description: Build and test targets with both gcc and clang. Use when you need to build, compile, test, or verify code changes with dual-compiler validation.
user-invocable: true
argument-hint: "<target-name> [target-name...]"
allowed-tools: Bash, Read, Glob, Grep
---

# Build and Test with Dual-Compiler Validation

Build one or more CMake targets with both gcc and clang, run the executables, and report results.

## Arguments

`$ARGUMENTS` — one or more CMake target names (e.g., `posit_ostream_formatting`, `bt_ostream_formatting posit_api`)

If no arguments are provided, ask the user which target(s) to build.

## Build Directories

| Directory | Compiler | Purpose |
|-----------|----------|---------|
| `build_ci/` | gcc (`/usr/bin/g++`) | Primary build |
| `build_ci_clang/` | clang (`clang++`) | Portability validation |

Both directories are pre-configured with `cmake -DUNIVERSAL_BUILD_ALL=ON ..` and ready for incremental builds.

## CRITICAL Safety Rules

These rules are non-negotiable. They exist because violating them previously caused a load=400 incident that required a hard server reset.

1. **ONE build at a time** — NEVER run multiple `make` or `cmake --build` commands concurrently
2. **Max `-j4`** — NEVER use `-j$(nproc)` or `-j` values above 8. Use `-j4` as default.
3. **Check first** — Before building, run `pgrep -a make` to verify no build is already running
4. **Sequential compilers** — Build with gcc FIRST, wait for completion, THEN build with clang

## Workflow

For each target in `$ARGUMENTS`:

### Step 1: Safety Check
```bash
pgrep -a make
```
If a build is running, STOP and tell the user. Do NOT proceed.

### Step 2: Build with gcc
```bash
cd build_ci && cmake --build . --target <target> -j4
```
If the build fails, report the error and skip to the next target. Do NOT proceed to clang for this target.

### Step 3: Run gcc executable
Find the executable in `build_ci/` and run it:
```bash
find build_ci/ -name "<target>" -type f -executable
```
Then run it and capture output. Report PASS or FAIL.

### Step 4: Build with clang
```bash
cd build_ci_clang && cmake --build . --target <target> -j4
```

### Step 5: Run clang executable
Find and run the clang-built executable. Report PASS or FAIL.

### Step 6: Summary
Report a table like:

| Target | gcc build | gcc test | clang build | clang test |
|--------|-----------|----------|-------------|------------|
| target_name | OK | PASS | OK | PASS |

## Finding Target Names

If the user gives a partial name, search for matching targets:
```bash
cd build_ci && cmake --build . --target help 2>&1 | grep -i "<partial>"
```

## Common Target Patterns

Test targets follow these naming conventions:
- `posit_api`, `posit_traits`, `posit_manipulators` — posit API tests
- `posit_ostream_formatting` — posit output formatting
- `bt_ostream_formatting` — blocktriple output formatting
- `er_api_ostream_formatting` — ereal output formatting
- `<type>_<category>` — general pattern (e.g., `cfloat_arithmetic`, `lns_conversion`)

## Error Handling

- **Build failure**: Report the last 20 lines of error output. Do NOT retry automatically.
- **Test failure**: Report full test output. Look for `FAIL` lines in output.
- **Missing target**: Search for similar targets and suggest alternatives.
- **Missing build directory**: Tell the user to run cmake configuration first.
