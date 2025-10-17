# Multi-Component Arithmetic Resources

This directory contains reference materials for multi-component arbitrary precision floating-point arithmetic, which forms the foundation for Universal's dd, qd, td, and priest implementations.

## Contents

### Documentation

- **[comparison-priest-bailey-shewchuk.md](comparison-priest-bailey-shewchuk.md)** - Comprehensive comparison of the three major approaches to multi-component arithmetic

### Papers (PDFs to download)

The following papers are foundational to multi-component arithmetic. Download them from the provided URLs:

#### Douglas Priest (Theoretical Foundation)

1. **"On Properties of Floating Point Arithmetics: Numerical Stability and the Cost of Accurate Computations"**
   - Douglas M. Priest, Ph.D. Dissertation, UC Berkeley, 1991
   - Mathematics Genealogy: https://www.mathgenealogy.org/id.php?id=32602
   - *Note: Full dissertation may require library access*

#### David Bailey / Yozo Hida (Fixed-Precision Implementation)

2. **"Algorithms for Quad-Double Precision Floating Point Arithmetic"**
   - Yozo Hida, Xiaoye S. Li, David H. Bailey
   - Proceedings of ARITH-15, 2000
   - Download: https://www.davidhbailey.com/dhbpapers/quad-double.pdf
   - Alternative: https://www.researchgate.net/publication/3900709_Algorithms_for_quad-double_precision_floating_point_arithmetic

3. **"Library for Double-Double and Quad-Double Arithmetic"**
   - David H. Bailey, LBNL Technical Report, 2008
   - Download: https://www.davidhbailey.com/dhbpapers/qd.pdf
   - Alternative: https://www.researchgate.net/publication/228570156_Library_for_Double-Double_and_Quad-Double_Arithmetic

#### Jonathan Shewchuk (Adaptive Precision)

4. **"Robust Adaptive Floating-Point Geometric Predicates"**
   - Jonathan Richard Shewchuk
   - ACM Symposium on Computational Geometry, 1996
   - Download: https://dl.acm.org/doi/10.1145/237218.237337

5. **"Adaptive Precision Floating-Point Arithmetic and Fast Robust Geometric Predicates"**
   - Jonathan Richard Shewchuk
   - Discrete & Computational Geometry 18:305-363, 1997
   - Download: https://people.eecs.berkeley.edu/~jrs/papers/robustr.pdf
   - Publisher: https://link.springer.com/article/10.1007/PL00009321

## Quick Summary

### Three Approaches

1. **Priest (1991)**: Theoretical foundation
   - Defined error-free transformations (two_sum, two_prod)
   - Proved error bounds and stability properties
   - Established multi-component representation theory

2. **Bailey/Hida (2000-2008)**: Fixed precision implementation
   - Created dd (double-double): 2 components, ~31 decimal digits
   - Created qd (quad-double): 4 components, ~62 decimal digits
   - Production C++/Fortran library for scientific computing
   - **Universal's dd and qd implementations are based on this work**

3. **Shewchuk (1996-1997)**: Adaptive precision
   - Dynamic precision growth based on computational need
   - Optimized for computational geometry predicates
   - Fast for typical cases, correct for pathological cases
   - **Universal's planned priest implementation follows this approach**

### Universal Library Implementation Status

- ✅ **dd (double-double)**: Complete, production-ready
- ✅ **qd (quad-double)**: Complete, production-ready
- 🟡 **td (triple-double)**: Header structure exists, arithmetic incomplete
- ❌ **priest (adaptive)**: Design complete, implementation not started

See `comparison-priest-bailey-shewchuk.md` for detailed comparison.

## Related Universal Documentation

- `../priest.md` - Design document for adaptive priest implementation
- `../multi-component-arithmetic.md` - Overview of multi-component systems
- `../floatcascade-design.md` - Design for expansion framework
- `../../include/sw/universal/number/dd/` - dd implementation
- `../../include/sw/universal/number/qd/` - qd implementation
- `../../include/sw/universal/number/td/` - td implementation (in progress)

## Software Implementations

### QD Library (Bailey/Hida)
- Language: C++ and Fortran-90
- URL: https://www.davidhbailey.com/dhbsoftware/
- License: Modified BSD
- Features: Complete math library, drop-in replacement for double

### Shewchuk's Predicates
- Language: C
- URL: https://www.cs.cmu.edu/~quake/robust.html
- License: Public domain
- Features: 2D/3D orientation and incircle tests, robust Delaunay triangulation

### Universal Numbers Library
- Language: C++20
- URL: https://github.com/stillwater-sc/universal
- License: MIT
- Features: Header-only, multiple number systems including dd/qd

## Additional Resources

### Background on Floating-Point Arithmetic

- **IEEE-754 Standard**: IEEE Standard for Floating-Point Arithmetic
- **Kahan**: Various papers on floating-point arithmetic by William Kahan (IEEE-754 architect)
- **Dekker** (1971): "A Floating-Point Technique for Extending the Available Precision"
- **Knuth**: "The Art of Computer Programming, Vol. 2: Seminumerical Algorithms", Section 4.2.2

### Applications

- **Scientific Computing**: High-precision physics simulations, climate modeling
- **Computational Geometry**: Robust mesh generation, Delaunay triangulation
- **Numerical Analysis**: Accurate summation, reproducible linear algebra
- **Computer Graphics**: Robust geometric predicates for rendering

## Contributing

To add papers to this collection:

1. Download PDF from the URLs listed above
2. Save with descriptive filename: `author-year-title.pdf`
3. Update this README with the filename
4. Update comparison document if the paper provides new insights

## Notes

- PDFs are not included in the repository due to copyright
- All papers listed are publicly available from the provided URLs
- Some papers may require institutional access (ACM, Springer)
- Open access versions are preferred and linked when available

---

**Last Updated**: 2025-10-17
**Maintainer**: Universal Numbers Library Team
