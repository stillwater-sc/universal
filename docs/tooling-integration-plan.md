# Modern Tooling Integration Implementation Plan

## Overview

Adding `.clang-format`, `.clang-tidy`, and enhanced CI workflows to ensure consistent code quality, catch bugs early, and provide automatic style enforcement across the 600+ header files in the Universal library.

## Phase 1: Code Formatting with .clang-format

### 1.1 Create .clang-format Configuration

**File**: `.clang-format` (repository root)

**Recommended Configuration for Universal**:

```yaml
---
# Based on: LLVM style with modifications for scientific computing
BasedOnStyle: LLVM

# Language
Language: Cpp
Standard: c++20

# Indentation
IndentWidth: 4
TabWidth: 4
UseTab: Always  # Universal currently uses tabs
ContinuationIndentWidth: 4
IndentCaseLabels: false
IndentPPDirectives: AfterHash
IndentWrappedFunctionNames: false

# Line length
ColumnLimit: 120  # Scientific code often needs wider lines

# Braces
BreakBeforeBraces: Custom
BraceWrapping:
  AfterClass: false
  AfterControlStatement: Never
  AfterEnum: false
  AfterFunction: false
  AfterNamespace: false
  AfterStruct: false
  AfterUnion: false
  AfterExternBlock: false
  BeforeCatch: false
  BeforeElse: false
  BeforeLambdaBody: false

# Spacing
SpaceAfterCStyleCast: true
SpaceAfterLogicalNot: false
SpaceAfterTemplateKeyword: false
SpaceBeforeAssignmentOperators: true
SpaceBeforeCpp11BracedList: false
SpaceBeforeParens: ControlStatements
SpaceInEmptyParentheses: false
SpacesInAngles: false
SpacesInCStyleCastParentheses: false
SpacesInParentheses: false
SpacesInSquareBrackets: false

# Template and function declarations
AlwaysBreakTemplateDeclarations: Yes
BreakConstructorInitializers: BeforeColon
ConstructorInitializerIndentWidth: 4

# Pointer and reference alignment
PointerAlignment: Left
ReferenceAlignment: Left

# Includes
SortIncludes: true
IncludeBlocks: Regroup
IncludeCategories:
  # Standard library headers
  - Regex:           '^<[^/]*>$'
    Priority:        1
  # C++ standard library with paths
  - Regex:           '^<.*\.h>'
    Priority:        2
  # Universal library headers
  - Regex:           '^<universal/'
    Priority:        3
  # Local headers
  - Regex:           '.*'
    Priority:        4

# Comments
ReflowComments: true
SpacesBeforeTrailingComments: 2

# Namespaces
CompactNamespaces: false
FixNamespaceComments: true
NamespaceIndentation: None

# Preprocessor
AlignConsecutiveMacros: true
AlignEscapedNewlines: Right

# Other
AllowShortBlocksOnASingleLine: Empty
AllowShortFunctionsOnASingleLine: Inline
AllowShortIfStatementsOnASingleLine: Never
AllowShortLambdasOnASingleLine: Inline
AllowShortLoopsOnASingleLine: false
AlwaysBreakAfterReturnType: None
BinPackArguments: true
BinPackParameters: true
BreakBeforeBinaryOperators: None
BreakBeforeTernaryOperators: true
BreakStringLiterals: true
KeepEmptyLinesAtTheStartOfBlocks: false
MaxEmptyLinesToKeep: 1
...
```

### 1.2 Testing the Configuration

```bash
# Format a single file to test
clang-format -i include/sw/universal/number/posit/posit.hpp

# Check formatting without modifying (dry-run)
clang-format --dry-run --Werror include/sw/universal/number/posit/posit.hpp

# Format all headers
find include -name "*.hpp" -exec clang-format -i {} \;

# Format all source files
find . -name "*.cpp" -exec clang-format -i {} \;
```

### 1.3 Git Hook for Automatic Formatting (Optional)

**File**: `.git/hooks/pre-commit`

```bash
#!/bin/bash
# Pre-commit hook to run clang-format on changed C++ files

files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp|h|cc|cxx)$')

if [ -n "$files" ]; then
    echo "Running clang-format on modified files..."
    clang-format -i $files
    git add $files
fi
```

---

## Phase 2: Static Analysis with .clang-tidy

### 2.1 Create .clang-tidy Configuration

**File**: `.clang-tidy` (repository root)

**Recommended Configuration for Universal**:

```yaml
---
# Checks to enable
Checks: >
  -*,
  bugprone-*,
  -bugprone-easily-swappable-parameters,
  -bugprone-exception-escape,
  cert-*,
  -cert-err58-cpp,
  clang-analyzer-*,
  concurrency-*,
  cppcoreguidelines-*,
  -cppcoreguidelines-avoid-magic-numbers,
  -cppcoreguidelines-pro-bounds-array-to-pointer-decay,
  -cppcoreguidelines-pro-bounds-constant-array-index,
  -cppcoreguidelines-pro-bounds-pointer-arithmetic,
  -cppcoreguidelines-pro-type-reinterpret-cast,
  -cppcoreguidelines-avoid-c-arrays,
  misc-*,
  -misc-non-private-member-variables-in-classes,
  modernize-*,
  -modernize-use-trailing-return-type,
  -modernize-avoid-c-arrays,
  performance-*,
  portability-*,
  readability-*,
  -readability-function-cognitive-complexity,
  -readability-identifier-length,
  -readability-magic-numbers,
  -readability-uppercase-literal-suffix

# Check options
CheckOptions:
  # Naming conventions
  readability-identifier-naming.NamespaceCase: lower_case
  readability-identifier-naming.ClassCase: CamelCase
  readability-identifier-naming.StructCase: CamelCase
  readability-identifier-naming.TemplateParameterCase: CamelCase
  readability-identifier-naming.FunctionCase: camelBack
  readability-identifier-naming.VariableCase: camelBack
  readability-identifier-naming.PrivateMemberSuffix: ''
  readability-identifier-naming.ProtectedMemberSuffix: ''
  readability-identifier-naming.MacroCase: UPPER_CASE
  readability-identifier-naming.EnumConstantCase: CamelCase
  readability-identifier-naming.ConstexprVariableCase: camelBack
  readability-identifier-naming.GlobalConstantCase: camelBack
  readability-identifier-naming.MemberConstantCase: camelBack
  readability-identifier-naming.StaticConstantCase: camelBack

  # Modernization
  modernize-use-nullptr.NullMacros: 'NULL'
  modernize-use-auto.MinTypeNameLength: 5
  modernize-loop-convert.MaxCopySize: 16
  modernize-loop-convert.MinConfidence: reasonable
  modernize-loop-convert.NamingStyle: CamelCase

  # Performance
  performance-move-const-arg.CheckTriviallyCopyableMove: true
  performance-unnecessary-value-param.AllowedTypes: ''

  # Readability
  readability-function-size.LineThreshold: 150
  readability-function-size.StatementThreshold: 150
  readability-function-size.BranchThreshold: 20
  readability-function-size.ParameterThreshold: 8
  readability-function-size.NestingThreshold: 5
  readability-simplify-boolean-expr.ChainedConditionalReturn: true
  readability-simplify-boolean-expr.ChainedConditionalAssignment: true

# Header filter - only check our code
HeaderFilterRegex: 'include/sw/universal/.*'

# Warning as error
WarningsAsErrors: ''

# Format style
FormatStyle: file

# System headers to ignore
SystemHeaders: false

# Use color in diagnostics
UseColor: true
...
```

### 2.2 Running clang-tidy

```bash
# Check a single file
clang-tidy include/sw/universal/number/posit/posit.hpp \
    -- -std=c++20 -Iinclude

# Check all headers with compilation database
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
clang-tidy -p build include/sw/universal/number/posit/posit.hpp

# Fix issues automatically (use with caution!)
clang-tidy -fix include/sw/universal/number/posit/posit.hpp \
    -- -std=c++20 -Iinclude

# Run on all files
find include -name "*.hpp" | xargs -P$(nproc) -I{} \
    clang-tidy {} -- -std=c++20 -Iinclude
```

### 2.3 Common Issues to Expect and Fix

Based on the codebase analysis, expect these common findings:

1. **Missing `[[nodiscard]]`** on value-returning functions
2. **Use of C-style casts** instead of `static_cast`/`reinterpret_cast`
3. **Implicit conversions** that should be explicit
4. **Magic numbers** that should be named constants
5. **Pass-by-value** for large objects instead of pass-by-const-reference
6. **Missing `const`** qualifiers on member functions
7. **Old-style `typedef`** instead of `using`
8. **Missing `override`** keywords
9. **Potential integer overflow** in arithmetic operations
10. **Unused variables** in template instantiations

---

## Phase 3: EditorConfig for Cross-Editor Consistency

### 3.1 Create .editorconfig

**File**: `.editorconfig` (repository root)

```ini
# EditorConfig is awesome: https://EditorConfig.org

# Top-most EditorConfig file
root = true

# All files
[*]
charset = utf-8
end_of_line = lf
insert_final_newline = true
trim_trailing_whitespace = true

# C++ files
[*.{cpp,hpp,h,cc,cxx,hxx}]
indent_style = tab
indent_size = 4
tab_width = 4
max_line_length = 120

# CMake files
[CMakeLists.txt,*.cmake]
indent_style = tab
indent_size = 2

# Markdown files
[*.md]
trim_trailing_whitespace = false
max_line_length = off

# YAML files
[*.{yml,yaml}]
indent_style = space
indent_size = 2

# JSON files
[*.json]
indent_style = space
indent_size = 2
```

---

## Phase 4: GitHub Actions CI Integration

### 4.1 Add Formatting Check to CI

**File**: `.github/workflows/format-check.yml`

```yaml
name: Code Formatting Check

on:
  pull_request:
    branches: [ main, v3.* ]
  push:
    branches: [ main, v3.* ]

jobs:
  check-format:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v4

    - name: Install clang-format
      run: |
        sudo apt-get update
        sudo apt-get install -y clang-format-16

    - name: Check formatting
      run: |
        find include -name "*.hpp" -o -name "*.h" | \
        xargs clang-format-16 --dry-run --Werror

        find . -path ./build -prune -o -name "*.cpp" -print | \
        xargs clang-format-16 --dry-run --Werror

    - name: Show formatting diff (on failure)
      if: failure()
      run: |
        find include -name "*.hpp" -o -name "*.h" | \
        xargs clang-format-16 --dry-run --Werror -i
        git diff
```

### 4.2 Add Clang-Tidy Check to CI

**File**: `.github/workflows/static-analysis.yml`

```yaml
name: Static Analysis

on:
  pull_request:
    branches: [ main, v3.* ]
  push:
    branches: [ main, v3.* ]

jobs:
  clang-tidy:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v4

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          clang-16 \
          clang-tidy-16 \
          cmake \
          ninja-build

    - name: Configure CMake
      run: |
        cmake -B build \
          -G Ninja \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DCMAKE_CXX_COMPILER=clang++-16

    - name: Run clang-tidy on changed files
      run: |
        # Get list of changed files
        git fetch origin ${{ github.base_ref }}
        CHANGED_FILES=$(git diff --name-only origin/${{ github.base_ref }}...HEAD | \
                       grep -E '\.(cpp|hpp|h)$' || true)

        if [ -n "$CHANGED_FILES" ]; then
          echo "Running clang-tidy on changed files:"
          echo "$CHANGED_FILES"

          for file in $CHANGED_FILES; do
            if [ -f "$file" ]; then
              clang-tidy-16 -p build "$file"
            fi
          done
        else
          echo "No C++ files changed"
        fi
```

### 4.3 Add Sanitizers to CI

**File**: `.github/workflows/sanitizers.yml`

```yaml
name: Sanitizers

on:
  pull_request:
    branches: [ main ]
  schedule:
    # Run weekly on main
    - cron: '0 0 * * 0'

jobs:
  address-sanitizer:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v4

    - name: Configure with ASan
      run: |
        cmake -B build \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g" \
          -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address" \
          -DUNIVERSAL_BUILD_CI=ON

    - name: Build
      run: cmake --build build -j $(nproc)

    - name: Test
      run: |
        cd build
        ASAN_OPTIONS=detect_leaks=1:detect_stack_use_after_return=1 \
        ctest --output-on-failure

  undefined-sanitizer:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v4

    - name: Configure with UBSan
      run: |
        cmake -B build \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_CXX_FLAGS="-fsanitize=undefined -fno-omit-frame-pointer -g" \
          -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=undefined" \
          -DUNIVERSAL_BUILD_CI=ON

    - name: Build
      run: cmake --build build -j $(nproc)

    - name: Test
      run: |
        cd build
        UBSAN_OPTIONS=print_stacktrace=1 ctest --output-on-failure

  thread-sanitizer:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v4

    - name: Configure with TSan
      run: |
        cmake -B build \
          -DCMAKE_BUILD_TYPE=Debug \
          -DCMAKE_CXX_FLAGS="-fsanitize=thread -fno-omit-frame-pointer -g" \
          -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread" \
          -DUNIVERSAL_BUILD_CONCURRENCY=ON

    - name: Build
      run: cmake --build build -j $(nproc)

    - name: Test
      run: |
        cd build
        TSAN_OPTIONS=second_deadlock_stack=1 ctest --output-on-failure
```

---

## Phase 5: Pre-commit Hooks (Optional but Recommended)

### 5.1 Using pre-commit Framework

**File**: `.pre-commit-config.yaml`

```yaml
# See https://pre-commit.com for more information
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.5.0
    hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer
      - id: check-yaml
      - id: check-added-large-files
      - id: check-merge-conflict
      - id: check-case-conflict
      - id: mixed-line-ending

  - repo: https://github.com/pocc/pre-commit-hooks
    rev: v1.3.5
    hooks:
      - id: clang-format
        args: [--style=file]
      - id: clang-tidy
        args: [--checks=bugprone-*,cert-*,performance-*]

  - repo: https://github.com/codespell-project/codespell
    rev: v2.2.6
    hooks:
      - id: codespell
        args: [--ignore-words-list=hist,nd,te]
```

### 5.2 Installation and Setup

```bash
# Install pre-commit
pip install pre-commit

# Install hooks
pre-commit install

# Run on all files manually
pre-commit run --all-files

# Update hooks
pre-commit autoupdate
```

---

## Phase 6: CMake Integration

### 6.1 Add Format Target to CMakeLists.txt

Add to top-level `CMakeLists.txt`:

```cmake
# Optional: Add clang-format target
find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format clang-format-16)
if(CLANG_FORMAT_EXECUTABLE)
    file(GLOB_RECURSE ALL_CXX_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/include/*.cpp
        ${CMAKE_SOURCE_DIR}/include/*.hpp
        ${CMAKE_SOURCE_DIR}/static/*.cpp
        ${CMAKE_SOURCE_DIR}/applications/*.cpp
    )

    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
        -i
        -style=file
        ${ALL_CXX_SOURCE_FILES}
        COMMENT "Running clang-format on all source files"
    )

    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
        --dry-run
        --Werror
        -style=file
        ${ALL_CXX_SOURCE_FILES}
        COMMENT "Checking code formatting"
    )
endif()

# Optional: Add clang-tidy target
find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy-16)
if(CLANG_TIDY_EXECUTABLE)
    set(CMAKE_CXX_CLANG_TIDY
        ${CLANG_TIDY_EXECUTABLE};
        -header-filter=${CMAKE_SOURCE_DIR}/include/sw/universal/.*;
    )

    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_EXECUTABLE}
        -p ${CMAKE_BINARY_DIR}
        ${ALL_CXX_SOURCE_FILES}
        COMMENT "Running clang-tidy on all source files"
    )
endif()
```

### 6.2 Usage

```bash
# Format all code
make format

# Check formatting
make format-check

# Run clang-tidy
make tidy
```

---

## Phase 7: Documentation and Developer Guide

### 7.1 Add to CONTRIBUTING.md

```markdown
## Code Style and Quality

### Automatic Formatting

We use `clang-format` to ensure consistent code formatting. Before submitting a PR:

\`\`\`bash
# Format your changes
make format

# Or check formatting
make format-check
\`\`\`

### Static Analysis

We use `clang-tidy` for static analysis:

\`\`\`bash
# Run static analysis
make tidy
\`\`\`

### Pre-commit Hooks (Recommended)

Install pre-commit hooks to automatically check your code:

\`\`\`bash
pip install pre-commit
pre-commit install
\`\`\`

### IDE Integration

#### VS Code
Install extensions:
- C/C++ (Microsoft)
- clang-format
- EditorConfig

#### CLion
Settings → Editor → Code Style → C/C++ → Set from... → .clang-format

#### Vim/Neovim
Install `vim-clang-format` plugin
```

---

## Implementation Timeline

### Week 1: Setup and Testing
- [ ] Create `.clang-format` configuration
- [ ] Test on small subset of files
- [ ] Adjust configuration based on results
- [ ] Create `.editorconfig`

### Week 2: Static Analysis
- [ ] Create `.clang-tidy` configuration
- [ ] Run on subset and categorize warnings
- [ ] Create suppression list for acceptable warnings
- [ ] Document common patterns

### Week 3: CI Integration
- [ ] Add format-check workflow
- [ ] Add clang-tidy workflow (warnings only)
- [ ] Test on development branch

### Week 4: Sanitizers
- [ ] Add ASan workflow
- [ ] Add UBSan workflow
- [ ] Add TSan workflow (if concurrent code exists)

### Week 5: Rollout
- [ ] Format entire codebase (single PR)
- [ ] Enable format-check as required CI
- [ ] Enable clang-tidy (non-blocking)
- [ ] Update documentation

### Week 6: Enforcement
- [ ] Make format-check blocking
- [ ] Address high-priority clang-tidy warnings
- [ ] Add pre-commit hook instructions

---

## Expected Benefits

1. **Consistency**: Zero debates about code style
2. **Bug Detection**: Catch issues before code review
3. **Onboarding**: New contributors follow standards automatically
4. **Code Quality**: Systematic improvement over time
5. **Review Efficiency**: Focus on logic, not formatting
6. **Continuous Improvement**: Tooling evolves with C++ standards

## Maintenance

- **Quarterly**: Review and update clang-tidy checks
- **Per Release**: Update clang-format/clang-tidy versions
- **As Needed**: Adjust configurations based on feedback

---

## Notes

- Start with **formatting only** (low friction, high value)
- Add **static analysis gradually** (phased rollout)
- Make tools **helpful, not annoying** (configure wisely)
- **Document everything** (reduce friction for contributors)
- **Lead by example** (maintainers use tools first)
