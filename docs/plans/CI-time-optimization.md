# Minimizing the CI test time

## The Problem

UNIVERSAL_BUILD_CI=ON sets UNIVERSAL_BUILD_NUMBERS=ON, which cascades into UNIVERSAL_BUILD_NUMBER_STATICS=ON + UNIVERSAL_BUILD_NUMBER_ELASTICS=ON +
UNIVERSAL_BUILD_NUMBER_INTERNALS=ON + UNIVERSAL_BUILD_NUMBER_CONVERSIONS=ON + UNIVERSAL_BUILD_COMPLEX=ON + the C API libraries. That's every single number
system (27+ subdirectories of regression tests) on every push across 5 platforms. The Windows MSVC build is the bottleneck because MSVC is significantly
slower at heavy template instantiation.

## Recommendations (least disruptive first)

  1. Tiered CI matrix - different build scopes per platform

  The biggest win for least effort. Windows doesn't need to build the same breadth as Linux/GCC. Run the full BUILD_NUMBERS on Linux GCC (your primary
  platform), and run a reduced "core types" build on Windows/macOS. A Windows compilation error in posit<32,2> will almost always also fail on
  cfloat<32,8,23> - you don't need exhaustive type coverage on every platform to catch portability issues.

  Concretely: create a UNIVERSAL_BUILD_CI_LITE option that builds a representative subset (e.g., internals + posits + cfloats + fixpnts + a couple of
  elastics), and use that for Windows + macOS.

  2. Add CMake build caching (ccache / sccache)

  Currently there's zero build caching. Adding ccache on Linux/macOS and sccache on Windows would make incremental CI runs dramatically faster (60-70%
  reduction when only a few files change). This is a ~10-line addition to the workflow.

  3. Parallel ctest

  The workflow runs ctest -C Release with no -j flag, so tests run sequentially. Adding ctest -j 4 (or $(nproc)) is a free speedup for the test phase.

  4. Drop one macOS variant

  You're testing both macos-latest (ARM64) and macos-15-intel (x64). Both use Apple Clang. The x64 macOS runner catches very little that ARM64 doesn't.
  Consider moving macos-15-intel to the weekly full-regression only.

  5. Windows-only: parallel MSVC build

  The build step uses cmake --build without --parallel. Adding cmake --build ... --parallel would use MSVC's /MP flag (already set in your CMakeLists.txt)
  but only if the build tool actually receives the flag.

  ---
  My recommended combo for cutting CI from ~60 min to ~20-25 min:

  ┌───────────────────────────────────────┬────────────────────────────┬────────────────────────────┐
  │                Change                 │     Estimated savings      │           Effort           │
  ├───────────────────────────────────────┼────────────────────────────┼────────────────────────────┤
  │ Tiered matrix (lite build on Windows) │ 25-30 min                  │ Medium - new CMake option  │
  ├───────────────────────────────────────┼────────────────────────────┼────────────────────────────┤
  │ Add ccache/sccache                    │ 10-15 min on repeat builds │ Low - workflow change only │
  ├───────────────────────────────────────┼────────────────────────────┼────────────────────────────┤
  │ ctest -j $(nproc)                     │ 2-5 min                    │ Trivial                    │
  ├───────────────────────────────────────┼────────────────────────────┼────────────────────────────┤
  │ Drop macOS-intel from default CI      │ 8-12 min wall clock        │ Trivial                    │
  ├───────────────────────────────────────┼────────────────────────────┼────────────────────────────┤
  │ --parallel on cmake build step        │ 3-5 min on Windows         │ Trivial                    │
  └───────────────────────────────────────┴────────────────────────────┴────────────────────────────┘

