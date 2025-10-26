# Changelog

All notable changes to the Universal Numbers Library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added - 2025-01-26

#### Expansion Operations Infrastructure (Milestone 1)
- Added Shewchuk's adaptive precision floating-point expansion algorithms
- Created `include/sw/universal/internal/expansion/expansion_ops.hpp` with core algorithms:
  - `two_sum()` - Error-free transformation for addition (Knuth/Dekker)
  - `fast_two_sum()` - Optimized EFT when |a| >= |b| (Dekker)
  - `two_prod()` - Error-free multiplication using FMA
  - `grow_expansion()` - Add single component to expansion (Shewchuk Figure 6)
  - `fast_expansion_sum()` - Merge two nonoverlapping expansions (Shewchuk Figure 8)
  - `linear_expansion_sum()` - Robust expansion merging (Shewchuk Figure 7)
  - Utility functions: `estimate()`, `is_decreasing_magnitude()`, `is_nonoverlapping()`, `is_strongly_nonoverlapping()`
- Created test infrastructure for expansion operations:
  - `internal/expansion/api/api.cpp` - API usage examples
  - `internal/expansion/api/expansion_ops.cpp` - Comprehensive unit tests
  - `internal/expansion/CMakeLists.txt` - Build configuration
- Integrated expansion tests into main build system
- All tests passing (7/7 test suites)

#### Documentation
- Created `CHANGELOG.md` to track repository changes
- Created `docs/sessions/` directory for development session records
- Added session documentation for expansion operations implementation

### Technical Details

**Expansion Operations (Shewchuk 1997)**

The expansion operations implement adaptive precision floating-point arithmetic based on
Jonathan Richard Shewchuk's seminal paper "Adaptive Precision Floating-Point Arithmetic
and Fast Robust Geometric Predicates" (Discrete & Computational Geometry 18:305-363, 1997).

Key differences from Priest's fixed-precision algorithms (used in `floatcascade`):
- **Variable component count**: Expansions can grow/shrink dynamically
- **Adaptive algorithms**: Only examine as many components as needed
- **Strongly nonoverlapping**: Stricter invariant than Priest's nonoverlapping
- **Geometric predicates**: Optimized for orientation tests, incircle tests, etc.

**References:**
- Shewchuk, J.R. (1997). "Adaptive Precision Floating-Point Arithmetic and Fast Robust
  Geometric Predicates." Available at: https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf
- Priest, D.M. (1991). "Algorithms for Arbitrary Precision Floating Point Arithmetic."
- Hida, Li, Bailey (2000). "Library for Double-Double and Quad-Double Arithmetic."

### Roadmap

**Completed:**
- ✅ Milestone 1: Core expansion operations (GROW, FAST-SUM, LINEAR-SUM)

**In Progress:**
- 🔄 Milestone 2: Scalar operations & compression (SCALE, COMPRESS, adaptive comparison)

**Planned:**
- ⏳ Milestone 3: Enhanced `ereal` arithmetic (integrate expansion_ops into ereal)
- ⏳ Milestone 4: Adaptive comparison & geometric predicates
- ⏳ Milestone 5: Conversion & interoperability with dd/td/qd_cascade
- ⏳ Milestone 6: Optimization & production hardening

## [3.87] - Current Development Branch

### Existing Features
- Fixed multi-component floating-point: `dd_cascade`, `td_cascade`, `qd_cascade`
- Priest's algorithms in `floatcascade<N>` template
- Comprehensive test suites for cascade types
- Interactive educational tutorial on expansion algebra

---

## Notes

### Version Numbering
Universal uses semantic versioning: MAJOR.MINOR.PATCH
- MAJOR: Breaking API changes
- MINOR: New features, backward compatible
- PATCH: Bug fixes, backward compatible

### Contributing
When adding entries to this changelog:
1. Add new entries under `[Unreleased]` section
2. Use subsections: Added, Changed, Deprecated, Removed, Fixed, Security
3. Include relevant issue/PR numbers
4. Move entries to versioned section on release
5. Update roadmap status as milestones complete

### Session Documentation
Detailed development sessions are documented in `docs/sessions/` with:
- Session date and focus
- Design decisions and rationale
- Implementation details
- Test results
- Next steps

---

**Legend:**
- ✅ Completed
- 🔄 In Progress
- ⏳ Planned
- ⚠️ Blocked
- 🐛 Bug Fix
- 💡 Enhancement
- 🔧 Maintenance
