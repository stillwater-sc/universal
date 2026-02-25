---
title: Contributing
description: How to contribute to the Universal Numbers Library

---

Thank you for your interest in contributing to the Universal Numbers Library.

## How to Contribute

1. **Report bugs** — open a [GitHub Issue](https://github.com/stillwater-sc/universal/issues) with a minimal reproducer
2. **Suggest features** — describe the use case and proposed solution in an issue
3. **Submit pull requests** — fork the repo, create a feature branch, and open a PR against `main`

## Development Setup

```bash
git clone https://github.com/stillwater-sc/universal
cd universal && mkdir build && cd build
cmake -DUNIVERSAL_BUILD_ALL=ON ..
make -j4
make test
```

## Guidelines

- Follow existing code style (see [Code Formatting](../build/code-formatting/))
- Add regression tests for new features
- Ensure CI passes on GCC, Clang, and MSVC before requesting review

## More Information

- [Contributors](../contributing/contributors/) — current project contributors
- [Code of Conduct](../contributing/code-of-conduct/) — community standards
- [Release Process](../contributing/release-process/) — how releases are published
