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


