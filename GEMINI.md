# GEMINI.md - Universal C++ Library

## Project Overview

This project, **Universal**, is a header-only C++20 template library that provides a collection of custom arithmetic types designed for mixed-precision algorithm development and optimization. It offers plug-in replacements for native C++ types (like `float`, `double`, `int`) with specialized number systems such as:

*   **Posits**: A tapered precision floating-point format.
*   **Classic and Custom Floats**: `cfloat`, `bfloat16`, `half-precision`, etc.
*   **Fixed-Point Numbers**: `fixpnt`.
*   **Elastic/Arbitrary Precision Types**: `einteger`, `erational`, `efloat` for computations that require precision to grow dynamically.
*   And many others (logarithmic, intervals, etc.).

The library is intended for high-performance and numerically-intensive domains like Artificial Intelligence (AI), Digital Signal Processing (DSP), High-Performance Computing (HPC), and scientific research, where tailoring arithmetic precision and dynamic range can lead to significant performance and efficiency gains.

The project uses **CMake** as its primary build system and is structured with extensive tests, examples, and command-line tools.

## Building and Running

The project can be built using the standard CMake workflow or a convenience `Makefile` wrapper in the root directory.

### Prerequisites

*   A C++20 compatible compiler (GCC, Clang, or MSVC).
*   CMake (version 3.22+).
*   `git` (for versioning).

### Quick Start (Makefile Wrapper)

The root `Makefile` provides simple targets for common development workflows. Build artifacts are placed in the `build/` directory.

*   **Build demonstration components**:
    ```bash
    make
    ```
*   **Run all tests**:
    ```bash
    make test
    ```
*   **Build everything (library, tests, examples, benchmarks)**:
    ```bash
    make all
    ```
*   **Clean build artifacts**:
    ```bash
    make clean
    ```

### Standard CMake Workflow

For more control, you can use CMake directly.

1.  **Configure the build** (creates a `build` directory):
    ```bash
    mkdir build && cd build
    cmake ..
    ```
    *   By default, this configures a `Release` build and enables a demonstration set of targets (tools, educational examples, applications).

2.  **Build the project**:
    ```bash
    make -j$(nproc)
    ```

3.  **Run tests**:
    ```bash
    make test
    # or
    ctest --output-on-failure
    ```

### Build Configurations

The build is highly modular. You can enable different components using CMake variables.

*   **Build all components**:
    ```bash
    cmake -DUNIVERSAL_BUILD_ALL=ON ..
    ```
*   **Build for Continuous Integration (a broad subset of tests)**:
    ```bash
    cmake -DUNIVERSAL_BUILD_CI=ON ..
    ```
*   **Enable specific number systems** (e.g., posits and cfloats):
    ```bash
    cmake -DUNIVERSAL_BUILD_NUMBER_POSITS=ON -DUNIVERSAL_BUILD_NUMBER_CFLOATS=ON ..
    ```
    (A full list of `UNIVERSAL_BUILD_*` options is in the root `CMakeLists.txt`).

## Development Conventions

### Code Style

The project uses `.clang-format` to enforce a consistent coding style, based on the **LLVM** style with some key modifications:

*   **Indentation**: **Tabs** are used for indentation (Tab Width: 4).
*   **Column Limit**: `120` characters.
*   **Braces**: Attached on the same line (K&R style), e.g., `if (condition) {`.
*   **Pointers/References**: Aligned to the left, e.g., `int* p = nullptr;`.

To format your code, use `clang-format`.

### Testing

*   **Unit & Regression Tests**: The project has an extensive test suite. Tests for each number system are located in subdirectories under `static/` and `elastic/`.
*   **Running Tests**: Use `make test` or `ctest`.
*   **Sanitizers**: The build system supports AddressSanitizer (ASan) and UndefinedBehaviorSanitizer (UBSan) for detecting memory errors and undefined behavior.
    *   **Convenience Target**: `make sanitize` will build and run both ASan and UBSan tests.
    *   **Manual Setup (ASan)**:
        ```bash
        cmake -DUNIVERSAL_ENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug ..
        make && ctest
        ```
*   **Code Coverage**: Coverage reports can be generated via `make coverage`. This requires `lcov` (for GCC) or `llvm-profdata`/`llvm-cov` (for Clang).

### Key Directories

*   `include/sw/universal/`: The core of the header-only library.
*   `static/`, `elastic/`: Source for test suites for static-sized and elastic-sized number systems.
*   `internal/`: Tests for the low-level building blocks of the number systems.
*   `applications/`: Practical example applications demonstrating use cases.
*   `benchmark/`: Performance, accuracy, and energy benchmarks.
*   `education/`: Educational code and tutorials.
*   `tools/`: Source for command-line utilities (`ieee`, `posit`, `ucalc`, etc.).
*   `docs/`: Project documentation, design documents, and papers.
