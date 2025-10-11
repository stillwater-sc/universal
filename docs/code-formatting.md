# Code Formatting with clang-format

The Universal library provides `clang-format` configuration as a **formatting tool** for developers. It is **not enforced** in CI, allowing developers to maintain manual formatting where it improves readability.

## Philosophy

- clang-format is a **helper tool**, not a strict enforcer
- Use it for new code to maintain basic consistency
- Manual formatting is respected when it aids comprehension (aligned tables, structured enums, etc.)
- Developers decide when to apply formatting suggestions

## Quick Start

### Installing clang-format

**Ubuntu/Debian:**
```bash
sudo apt-get install clang-format
```

**macOS (Homebrew):**
```bash
brew install clang-format
```

**Windows:**
Download from [LLVM releases](https://releases.llvm.org/) or install via chocolatey:
```powershell
choco install llvm
```

### Using the Format Targets

After installing clang-format, the CMake configuration will automatically detect it and create formatting targets:

```bash
# Configure (format targets will be created automatically)
cmake -B build

# Check what would be changed in headers (first 50 issues)
make -C build format-headers-check

# Format only header files
make -C build format-headers

# Format only test/application files
make -C build format-tests

# Show what would be changed (first 50 issues)
make -C build format-diff

# Format all source files (use with caution!)
make -C build format
```

**Recommendation**: Use `format-headers-check` or `format-diff` to **review** suggestions before applying. Apply formatting selectively to files you're actively working on.

## Configuration

The `.clang-format` file in the repository root defines our code style:

- **C++ Standard**: C++20
- **Indentation**: Tabs (width 4)
- **Line Length**: 120 characters
- **Brace Style**: Attach (K&R style)
- **Pointer Alignment**: Left (`int* ptr`)
- **Include Sorting**: Enabled, grouped by category

## IDE Integration

### VS Code

1. Install the **C/C++** extension (Microsoft)
2. Add to your `settings.json`:
```json
{
  "C_Cpp.clang_format_style": "file",
  "editor.formatOnSave": true
}
```

### CLion

1. Go to **Settings → Editor → Code Style → C/C++**
2. Click **Set from...** → **Predefined Style** → **LLVM**
3. Check **Enable ClangFormat**
4. Set **clang-format binary** path if needed

### Vim/Neovim

Install `vim-clang-format`:
```vim
" In your .vimrc
Plug 'rhysd/vim-clang-format'

" Format on save
autocmd FileType c,cpp,objc ClangFormatAutoEnable
```

### Emacs

```elisp
;; In your .emacs or init.el
(require 'clang-format)
(global-set-key (kbd "C-c f") 'clang-format-region)
(global-set-key (kbd "C-c u") 'clang-format-buffer)

;; Format on save
(add-hook 'before-save-hook 'clang-format-buffer)
```

## Pre-commit Hook (Optional)

To automatically format files before committing:

```bash
# Create pre-commit hook
cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Format changed C++ files with clang-format

files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(cpp|hpp|h|cc|cxx)$')

if [ -n "$files" ]; then
    echo "Formatting modified C++ files..."
    clang-format -i $files
    git add $files
fi
EOF

# Make it executable
chmod +x .git/hooks/pre-commit
```

## Command Line Usage

### Format specific files
```bash
# Format a single file
clang-format -i include/sw/universal/number/posit/posit.hpp

# Format multiple files
clang-format -i file1.hpp file2.cpp file3.h

# Format all headers in a directory
find include/sw/universal/number/posit -name "*.hpp" -exec clang-format -i {} \;
```

### Check formatting without modifying
```bash
# Check a file (returns exit code 1 if not formatted)
clang-format --dry-run --Werror file.hpp

# Show diff of what would change
clang-format --dry-run file.hpp | diff file.hpp -
```

### Format specific lines/regions
```bash
# Format lines 10-50
clang-format -i --lines=10:50 file.cpp

# Format changed lines (useful for partial formatting)
git diff -U0 --no-color HEAD | clang-format-diff -i -p1
```

## CI Integration

**clang-format is NOT enforced in CI.** The formatting check targets are provided for developer convenience only.

Developers are trusted to maintain reasonable code formatting. When in doubt:
- Follow the existing style in the file you're editing
- Use clang-format suggestions as guidance, not requirements
- Prioritize readability over strict formatting rules

## Formatting Guidelines

### What gets formatted
- All `.hpp` header files in `include/`
- All `.cpp` source files in tests and applications
- All `.h` C header files

### What doesn't get formatted
- Third-party code in `external/` (if any)
- Generated files in `build/`
- Binary files

### Common Patterns

**Template Declarations:**
```cpp
// Well-formatted template
template<unsigned nbits, unsigned es>
class posit {
	// ...
};
```

**Namespace Style:**
```cpp
// Compact namespace declaration
namespace sw { namespace universal {

// Code here

}}  // namespace sw::universal
```

**Include Organization:**
```cpp
// Grouped and sorted automatically
#include <iostream>      // C++ standard library
#include <vector>

#include <universal/utility/compiler.hpp>     // Universal utilities
#include <universal/traits/number_traits.hpp> // Universal traits
#include <universal/number/posit/posit.hpp>   // Universal numbers

#include "local_header.hpp"  // Local headers
```

**Long Function Signatures:**
```cpp
// Automatically wrapped
template<typename Real>
void some_very_long_function_name_that_exceeds_line_limit(
	const Real& parameter1,
	const Real& parameter2,
	int another_param) {
	// Implementation
}
```

## Troubleshooting

### clang-format not found
```bash
# Check if installed
which clang-format

# Check version (we support 14+)
clang-format --version

# If not found, install it (see Installing clang-format above)
```

### Format targets not available
```bash
# Reconfigure CMake to detect clang-format
rm -rf build
cmake -B build

# Check if clang-format was found
grep "clang-format" build/CMakeCache.txt
```

### Different formatting results
- Ensure you're using the same clang-format version as CI (16+ recommended)
- Check that `.clang-format` file is in the repository root
- Some older versions may format slightly differently

### Merge conflicts in formatted code
```bash
# After resolving conflicts, reformat
clang-format -i <conflicted-file>
git add <conflicted-file>
git commit
```

## FAQ

**Q: Do I need to format existing code I'm not changing?**
A: No. Never reformat code you're not actively modifying.

**Q: What if clang-format makes my code less readable?**
A: Don't apply it! Manual formatting that improves readability is always preferred. You can also disable formatting for specific regions:
```cpp
// clang-format off
int matrix[3][3] = {
	{1, 0, 0},
	{0, 1, 0},
	{0, 0, 1}
};
// clang-format on
```

**Q: Do I have to use clang-format?**
A: No. It's a tool to help with basic formatting consistency, not a requirement.

**Q: When should I use clang-format?**
A: Use it when:
- Writing new files from scratch
- You want help with basic indentation and spacing
- Cleaning up obviously inconsistent formatting

Don't use it when:
- Existing code has intentional manual alignment
- The suggestions make code harder to read
- You're working in files with established formatting

**Q: Why tabs instead of spaces?**
A: The Universal codebase historically uses tabs for indentation, spaces for alignment (via `UseTab: ForIndentation`). This allows developers with different editor settings to view code at their preferred indent width.

**Q: Can I use a different style for my fork?**
A: Yes, modify `.clang-format` in your fork. However, try to maintain general consistency with the main repo style for easier merging.

## See Also

- [clang-format documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [clang-format style options](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
- [Contributing Guidelines](../CONTRIBUTING.md)
- [Tooling Integration Plan](tooling-integration-plan.md)
