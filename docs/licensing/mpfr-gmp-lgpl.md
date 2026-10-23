# MPFR, GMP and Universal's MIT license

A record of how Universal uses the GNU MPFR and GNU MP (GMP) libraries, what their licenses
require, and why that leaves Universal's MIT license intact. Written 2026-10-23 at main
053fd703, after efloat's work against MPFR (Epic #1763). This is an engineering record, not
legal advice.

## The licenses

| | version checked | license |
|---|---|---|
| Universal | this repository | MIT (`LICENSE`) |
| GNU MPFR | 4.2.2 (Ubuntu `libmpfr6` 4.2.2-3) | LGPL-3.0-or-later |
| GNU MP | 6.3.0 (Ubuntu `libgmp10` 2:6.3.0+dfsg-5ubuntu2) | LGPL-3.0-or-later or GPL-2.0-or-later, at the user's choice |
| gmpy2 (Python, optional) | | LGPL-3.0-or-later |
| mpmath (Python, optional) | | BSD-3-Clause |

MPFR is under the *Lesser* GPL, not the GPL. The LGPL's conditions apply when someone conveys
a work that contains the library, or a binary that links it. A program that only calls the
library's public interface is a "work that uses the Library": its own source can be under any
license. Conveying a binary combined with the library requires the license notice and a way for
the recipient to use a modified version of the library. Dynamic linking against the system's
shared library satisfies the second condition.

## How Universal uses them

**The library, everything under `include/`, uses neither.** No header includes `mpfr.h` or
`gmp.h`, and nothing in the library links either. A program that includes Universal's headers
has no MPFR or GMP dependency.

**Two optional programs call MPFR to measure efloat against it:**

| program | purpose | built when |
|---|---|---|
| `benchmark/accuracy/efloat/mpfr_check.cpp` | efloat's results against MPFR's, bit for bit, in every rounding mode | `mpfr.h`, libmpfr and libgmp are found on a native build |
| `benchmark/performance/arithmetic/efloat/mpfr/mpfr_gap.cpp` | efloat's speed against MPFR's | the same |

Both sources are MIT, like the rest of the repository, and call only MPFR's public interface.
The CMake files find MPFR and GMP on the build machine and skip both programs when they are
absent. The MPFR and GMP sources are not part of this repository.

**One optional script uses gmpy2:** `benchmark/performance/arithmetic/efloat/mpmath_gap.py`
imports gmpy2, MPFR's Python binding, when it is installed, to print a reference table. It is
not installed with Universal and is not needed to build or use it.

**No binary that links MPFR or GMP is distributed:**
- The CI workflows do not install MPFR, so CI never builds the two programs.
- The release Docker image (`docker/Dockerfile.release`) builds only the command-line tools and
  ucalc, and installs no MPFR.
- `cmake --install` installs the headers, the CMake package files and the tools under `tools/`;
  no benchmark has an install rule.

Someone who builds the two programs themselves links the shared libmpfr and libgmp of their own
system. If such a binary were ever distributed, the LGPL notice would have to go with it, and
the dynamic linking already provides for a modified library.

**Constants and reference values.** `ereal_constants.hpp` says its constants were generated
offline with MPFR; its generator, `tools/generators/ereal_reference_gen.py`, uses mpmath. Either
way, the numbers a program prints are not covered by that program's license: the LGPL and the
GPL do not claim a program's output.

## efloat's algorithms

efloat's kernels implement published algorithms, cited where they are used:

| efloat | algorithm | source |
|---|---|---|
| `efloat_mpn::invert_limb`, `udiv_qr_3by2` | division by a precomputed reciprocal | Moller and Granlund, *Improved division by invariant integers*, IEEE Trans. Computers 60(2), 2011 |
| `efloat_mpn::dcpi1_div_qr_n` | divide-and-conquer division | Burnikel and Ziegler, *Fast recursive division*, MPI-I-98-1-022, 1998 |
| `efloat_mpn::sqrtrem`, `cbrtrem` | Karatsuba square root, and the same recursion carried to the cube | Zimmermann, *Karatsuba Square Root*, INRIA RR-3805, 1999 |
| `efloat_mpn::mul_n` | Karatsuba multiplication | Karatsuba and Ofman, 1962 |
| correct rounding, `can_round` | Ziv's strategy | Ziv, *Fast evaluation of elementary mathematical functions with correctly rounded last bit*, ACM TOMS 17(3), 1991 |
| log by the AGM, erf, lgamma | as MPFR evaluates them | Brent and Zimmermann, *Modern Computer Arithmetic*, 2010; MPFR's `algorithms.pdf` |

Copyright covers code, not the algorithms it carries out. Implementing an algorithm described in
a paper, a book or MPFR's own algorithm documentation creates no obligation under the author's
license.

`efloat_mpn.hpp` follows GMP's mpn conventions on purpose: an operand is a pointer and a limb
count, least significant limb first, and several functions carry the names of their GMP
counterparts (`sqrtrem`, `dcpi1_div_qr`, `sbpi1_div_qr`, `invert_pi1`, `udiv_qr_3by2`). The
names and the calling convention say which algorithm a function is; they do not copy GMP's code.
The kernels were written from the papers above. GMP's and MPFR's sources were not consulted for
them, and only the two libraries' installed headers and shared libraries were present on the
machine where they were written.

## Rules that keep it this way

- Do not copy, translate or adapt code from GMP's or MPFR's sources into Universal. Work from
  the published algorithm, and cite it.
- Do not add MPFR, GMP or gmpy2 to the library: no `#include <mpfr.h>` or `<gmp.h>` under
  `include/`, and no link dependency in Universal's CMake package.
- Keep programs that call MPFR optional, built only when MPFR is found, and outside the
  installed and released artifacts.
- If a binary that links MPFR or GMP is ever distributed, ship the LGPL-3.0 notice with it and
  link the libraries dynamically.
