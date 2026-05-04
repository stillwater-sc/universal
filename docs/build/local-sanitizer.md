# Local UBSan Workflow                                                                                                                                                          
The repo has CMake-level support for both ASan and UBSan via -DUNIVERSAL_ENABLE_UBSAN=ON / -DUNIVERSAL_ENABLE_ASAN=ON. 
Mirrors the CI config in .github/workflows/sanitizers.yml on GitHub.

1. Configure a UBSan build directory                                                                                                                                   
  From the repo root:                                                                                                                                                           
```bash
  mkdir -p build_ubsan                                                                                                                                                          
  cd build_ubsan                                                               
  cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DUNIVERSAL_BUILD_CI=ON \
    -DUNIVERSAL_ENABLE_UBSAN=ON
```                                                                                                                                                                          
The `-DUNIVERSAL_BUILD_CI=ON` enables the same component set CI builds. To match all 4 sanitizer dimensions, 
you can also create build_asan with `-DUNIVERSAL_ENABLE_ASAN=ON` instead.                                                                                                                                                                      
2. Build a target

  Standard -j4 rule (load-safety from CLAUDE.md):
```bash
  cd build_ubsan
  pgrep -a make                # safety check: no other build in flight        
  cmake --build . --target qd_api_api -j4
```
  Or build everything CI builds:

```bash
  cmake --build . -j4          # ~10-20 min, full CI sweep
```

3. Run with UBSan options matching CI

  CI uses print_stacktrace=1:halt_on_error=1. 
  The first prints a stack trace at each runtime error; the second aborts on the first one (default is to keep going and just log). 
  UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 ./static/highprecision/qd/qd_api_api                                                                                         
  To check all the high-precision targets (the cluster that flagged on PR #808):
```bash
  for tgt in qd_api_api dd_api_api ddc_api_api tdc_api_api qdc_api_api; do
    bin=$(find . -name "$tgt" -executable | head -1)
    printf "%-30s " "$tgt"
    UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 ./$bin > /dev/null 2> /tmp/ubsan_${tgt}.err
    ec=$?
    rt_errs=$(grep -c "runtime error" /tmp/ubsan_${tgt}.err)
    echo "exit=$ec, runtime_errors=$rt_errs"
  done
```
  Run the full ctest suite under UBSan (matches CI)

```bash
  cd build_ubsan
  UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 ctest --output-on-failure

  `--output-on-failure` shows captured stdout/stderr only for failing tests.                                                                                                      
  To re-run only what failed (matches the "Rerun failed tests" step in CI):
  UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 ctest --rerun-failed --output-on-failure

4. Reading a UBSan failure                                                                                                                                                       
  Output looks like:

```bash
  file.hpp:1229:48: runtime error: -nan is outside the range of representable values of type 'int'                                                                              
      #0 0x... in sw::universal::qd::to_digits(...) at qd_impl.hpp:1229:48
      #1 0x... in sw::universal::qd::to_string(...) at qd_impl.hpp:945:7
      ...
      #N 0x... in main at static/highprecision/qd/api/api.cpp:266:73
```
  
  Key cues:

  - :1229:48 is <line>:<column> - column tells you which expression in the line fired
  - The bottom-most main frame tells you which test case triggered it
  - `-fsanitize=undefined` covers a broad class: signed overflow, shift overflow, NaN->int casts ([conv.fpint]), null deref, OOB array access, etc.
  
  Useful one-shot commands                                                                                                                                                      
  Confirm UBSan is actually wired in the build:
```bash
  grep -E "fsanitize" CMakeCache.txt
  # Should show: ...-fsanitize=undefined -fno-omit-frame-pointer
```
  Skip leak detection if ASan complains about libraries you don't control:                                                                                                      
  ASAN_OPTIONS=detect_leaks=0:halt_on_error=1
