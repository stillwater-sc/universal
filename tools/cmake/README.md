# tools/cmake

CMake helpers for the Universal build. Every file here is used by the top-level `CMakeLists.txt`, at the place given. A helper that nothing references is dead and gets removed. #1944 removed five that had been unreferenced, some of them since 2019.

| File | Used by | What it does |
|---|---|---|
| `banners.cmake` | `include()` at the top of `CMakeLists.txt` | The font-art banner printed at configure time. |
| `header_guards.cmake` | `include()` in `CMakeLists.txt` | The `UNIVERSAL_*` C++ macros the headers test. Each guard is declared here with `universal_header_guard_tristate`, `_choice` or `_flag`, which creates its cache option and compile definition and registers it with the summary. |
| `summary.cmake` | `include()` at the end of `CMakeLists.txt` | The configuration summary: every `UNIVERSAL_BUILD_*` option and header guard, with its value. |
| `coverage.cmake` | `cmake -P` from the coverage targets in `CMakeLists.txt` | Collects and reports code coverage for `UNIVERSAL_ENABLE_COVERAGE` builds. |
| `ClangFormat.cmake` | `include()` near the end of `CMakeLists.txt` | The `format`, `format-check`, `format-diff`, `format-headers` and `format-tests` targets. |
| `ClangTidy.cmake` | `include()` near the end of `CMakeLists.txt` | The `tidy-check`, `tidy-fix`, `tidy-check-samples` and `tidy-info` targets. |
| `Templates/universal-config.cmake.in` | `configure_package_config_file()` in `CMakeLists.txt` | The package config that `find_package(UNIVERSAL CONFIG)` reads after `make install`. |
| `Templates/cmake-uninstall.cmake.in` | `configure_file()` for the `uninstall` target | Removes what `make install` installed. |
| `how-to-find-packages.md` | documentation | How CMake's `find_package()` searches, and how it can pick up a stale install. |

`CMakeLists.txt` adds this directory itself, not a `Modules/` subdirectory, to `CMAKE_MODULE_PATH`. A `Find<Package>.cmake` module therefore belongs here.

Adding a helper: reference it from `CMakeLists.txt` and add a row to this table.

Adding a header guard: declare it in `header_guards.cmake` with one of the `universal_header_guard_*` functions. The summary reports it from that declaration; it needs no entry in `summary.cmake`. A new `UNIVERSAL_BUILD_*` option or other `UNIVERSAL_*` cache option is not registered that way, so it does go in `summary.cmake`.
