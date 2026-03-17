# Run Sanitizers for early detection

## AddressSanitizer

AddressSanitizer will flag any code that might trigger buffer overflows and use-after-free errors.

```bash
# Test ASan locally
mkdir build-asan && cd build-asan
cmake -DCMAKE_BUILD_TYPE=Debug -DUNIVERSAL_ENABLE_ASAN=ON -DUNIVERSAL_BUILD_CI=ON ..
make -j$(nproc)
ASAN_OPTIONS=detect_leaks=0 ctest --output-on-failure
```

## UndefinedBehaviorSanitizer

The UndefinedBehaviorSanitizer will flag any code that triggers potentially undefined behavior. This is particularly important
for libraries like Universal, that do a lot of type conversions, reinterpretations of bits, and bit flipping.

```bash
# Test UBSan locally
mkdir build-ubsan && cd build-ubsan
# cmake -DCMAKE_BUILD_TYPE=Debug -DUNIVERSAL_ENABLE_UBSAN=ON -DUNIVERSAL_BUILD_CI=ON ..
# you need the -fsanitize=undefined flag set
cmake .. -DCMAKE_BUILD_TYPE=Debug -DUNIVERSAL_BUILD_CI=ON \
  -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all" \
  -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all" 
make -j$(nproc)
UBSAN_OPTIONS=print_stacktrace=1 ctest --output-on-failure
```

## Coverage

Coverage is a separate workflow from sanitizers and is opt-in.
To generate a meaningful report, binaries must be configured and built with coverage instrumentation enabled (`UNIVERSAL_ENABLE_COVERAGE=ON`) before running the report target.
- GCC uses `lcov` plus `genhtml`.
- Clang and AppleClang use `llvm-profdata` plus `llvm-cov`.

```bash
cmake -S . -B build-cov -DUNIVERSAL_ENABLE_COVERAGE=ON
cmake --build build-cov --target coverage
```

The HTML report is written to `build-cov/coverage-html/index.html`.

## Root Makefile wrapper (convenience)

CMake is the canonical build system for Universal. The root `Makefile` is a convenience wrapper for common local workflows.

Common commands from repository root:

```bash
make
make test
make sanitize
make coverage
make clean
make help
```

Workflow split:
- normal build/test: `make`, `make test`
- sanitizer build/test: `make sanitize`
- coverage-instrumented build/report: `make coverage`

The wrapper uses out-of-tree build directories under `build/<...>` and the default Makefile `CONFIG` is `Release` unless overridden.

Practical coverage backend behavior:
- Clang/AppleClang uses `llvm-profdata` + `llvm-cov`.
- GCC uses `lcov` + `genhtml`.
- MSVC coverage is not supported by this pipeline (configure-time failure when coverage is enabled).

Required tools are `cmake`, `ctest`, and a C/C++ compiler toolchain, plus the coverage tools listed above when running coverage.

Maintainer note: keep the root `Makefile` as a thin wrapper around CMake, and avoid turning it into a second build system.
