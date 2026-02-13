# Port Quire, FDP, and Fused BLAS to New Posit

## Context

The posit→posit1, posit2→posit rename is complete (934/934 tests green). The new `posit<nbits,es,bt>` (3-param) is now the default at `include/sw/universal/number/posit/`. However, quire, FDP (fused dot product), and fused BLAS features are still only available through posit1. This blocks migration of ~19 consumer files that depend on these features.

**Problem**: The quire/fdp infrastructure uses `internal::value<fbits>` and `bitblock<>` internally, while the new posit uses `blocktriple<>` and `blocksignificand<>`. We need bridge functions at the interface points.

**Approach**: Copy quire.hpp and fdp.hpp from posit1 into the new posit, keeping the quire's `value<>`/`bitblock<>` internals unchanged. Add bridge functions to convert between the two type systems at the posit↔quire boundary. Make posit-facing methods templated on `bt` so the quire works with `posit<nbits,es,bt>` for any block type.

## Step 1: Add `convert(value<>, posit<>)` bridge to `posit_impl.hpp`

**File**: `include/sw/universal/number/posit/posit_impl.hpp`

Add `#include <universal/internal/value/value.hpp>` near the existing blockbinary/blocktriple includes.

Add after the existing `convert(blocktriple<>...)` function (~line 430):
```cpp
// Bridge: convert internal::value<fbits> to posit (needed by quire output path)
template<unsigned nbits, unsigned es, typename bt, unsigned fbits>
inline posit<nbits, es, bt>& convert(const internal::value<fbits>& v, posit<nbits, es, bt>& p) {
    if (v.iszero()) { p.setzero(); return p; }
    if (v.isinf() || v.isnan()) { p.setnar(); return p; }
    // Copy bitblock fraction → blocksignificand
    blocksignificand<fbits, bt> sig;
    sig.clear();
    bitblock<fbits> frac = v.fraction();
    for (unsigned i = 0; i < fbits; ++i) sig.setbit(i, frac[i]);
    return convert_(v.sign(), v.scale(), sig, p);
}
```

## Step 2: Add `posit_to_value()` bridge to `posit_impl.hpp`

Add after the convert bridge (needed by quire input path):
```cpp
// Bridge: extract internal::value<fbits> from a posit (needed by quire input path)
template<unsigned nbits, unsigned es, typename bt>
internal::value<nbits - 3 - es> posit_to_value(const posit<nbits, es, bt>& p) {
    constexpr unsigned fbits = nbits - 3 - es;
    internal::value<fbits> v;
    if (p.iszero()) return v;
    if (p.isnar()) { v.setinf(); return v; }
    // Extract fraction as blockbinary, convert to bitblock
    blockbinary<fbits, bt> frac_bb = extract_fraction<nbits, es, bt, fbits>(p);
    bitblock<fbits> frac_bits;
    for (unsigned i = 0; i < fbits; ++i) frac_bits[i] = frac_bb.test(i);
    v.set(sign(p), scale(p), frac_bits, false, false);
    return v;
}
```

## Step 3: Copy `posit1/quire.hpp` → `posit/quire.hpp`

**Source**: `include/sw/universal/number/posit1/quire.hpp`
**Dest**: `include/sw/universal/number/posit/quire.hpp`

Modifications to the copy:

### 3a. Add value.hpp include
After the `#include <universal/number/quire/exceptions.hpp>` line, add:
```cpp
#include <universal/internal/value/value.hpp>
```
This brings in `internal::value<>`, `bitblock<>`, `module_multiply()`, `module_add()`.

### 3b. Update posit-facing methods to be templated on `bt`

The quire class template stays `quire<nbits, es, capacity>` (no bt param — quire internals don't use bt).

Change these members from taking `posit<nbits, es>` to `posit<nbits, es, bt>`:

```cpp
// Constructor: template on bt
template<typename bt>
quire(const posit<nbits, es, bt>& rhs) { *this = posit_to_value(rhs); }

// operator= from posit: template on bt
template<typename bt>
quire& operator=(const posit<nbits, es, bt>& rhs) {
    *this = posit_to_value(rhs);
    return *this;
}

// operator+= from posit: template on bt
template<typename bt>
quire& operator+=(const posit<nbits, es, bt>& rhs) {
    return operator+=(posit_to_value(rhs));
}

// operator-= from posit: template on bt
template<typename bt>
quire& operator-=(const posit<nbits, es, bt>& rhs) {
    return operator-=(posit_to_value(rhs));
}
```

The core `operator+=(const internal::value<fbits>&)` remains unchanged — it only uses `bitblock<>`.

### 3c. Update `convert_to<>()` method

The existing `convert_to<>()` calls `convert(to_value(), v)` which returns `internal::value<qbits>`. With our Step 1 bridge, this will route through `convert(value<>, posit<>)` and work with the new posit.

No changes needed to `convert_to<>()`.

### 3d. Update `quire_mul` to 3-param posit

```cpp
template<unsigned nbits, unsigned es, typename bt>
internal::value<2 * (nbits - 2 - es)> quire_mul(const posit<nbits, es, bt>& lhs, const posit<nbits, es, bt>& rhs) {
    static constexpr unsigned fbits = nbits - 3 - es;
    static constexpr unsigned fhbits = fbits + 1;
    static constexpr unsigned mbits = 2 * fhbits;
    internal::value<mbits> product;
    internal::value<fbits> a, b;
    if (lhs.isnar() || rhs.isnar()) { product.setinf(); return product; }
    if (lhs.iszero() || rhs.iszero()) return product;
    // Bridge: blockbinary fraction → bitblock
    blockbinary<fbits, bt> lf = extract_fraction<nbits, es, bt, fbits>(lhs);
    blockbinary<fbits, bt> rf = extract_fraction<nbits, es, bt, fbits>(rhs);
    bitblock<fbits> lbb, rbb;
    for (unsigned i = 0; i < fbits; ++i) { lbb[i] = lf.test(i); rbb[i] = rf.test(i); }
    a.set(sign(lhs), scale(lhs), lbb, lhs.iszero(), lhs.isnar());
    b.set(sign(rhs), scale(rhs), rbb, rhs.iszero(), rhs.isnar());
    module_multiply(a, b, product);
    return product;
}
```

### 3e. Update `quire_add` similarly

Same pattern as quire_mul: template on `bt`, bridge `blockbinary<>` → `bitblock<>`.

## Step 4: Copy `posit1/fdp.hpp` → `posit/fdp.hpp`

**Source**: `include/sw/universal/number/posit1/fdp.hpp`
**Dest**: `include/sw/universal/number/posit/fdp.hpp`

Changes:
- `#include <universal/traits/posit1_traits.hpp>` → `#include <universal/traits/posit_traits.hpp>`
- `enable_if_posit1` → `enable_if_posit` (3 occurrences: fdp_stride, fdp MSVC, fdp non-MSVC)

No other changes needed — `quire_mul()`, `convert()`, and `quire<>` are used generically.

## Step 5: Update `posit/posit.hpp` umbrella

**File**: `include/sw/universal/number/posit/posit.hpp`

Uncomment quire/fdp includes (lines 81-86):
```cpp
#include <universal/number/quire/exceptions.hpp>
#include <universal/number/posit/quire.hpp>
#include <universal/number/posit/fdp.hpp>
```

## Step 6: Update `posit/posit_fwd.hpp` forward declarations

**File**: `include/sw/universal/number/posit/posit_fwd.hpp`

Uncomment and update quire forward declarations (lines 32-33):
```cpp
template<unsigned nbits, unsigned es, unsigned capacity> class quire;
template<unsigned nbits, unsigned es, typename bt> internal::value<2*(nbits-2-es)> quire_mul(const posit<nbits,es,bt>&, const posit<nbits,es,bt>&);
```

Add forward declaration for the convert bridge:
```cpp
template<unsigned nbits, unsigned es, typename bt, unsigned fbits> posit<nbits,es,bt>& convert(const internal::value<fbits>&, posit<nbits,es,bt>&);
```

## Step 7: Update BLAS ext headers

**File**: `include/sw/blas/ext/posit_fused_blas.hpp` (line 9)
- `#include <universal/number/posit1/posit_fwd.hpp>` → `#include <universal/number/posit/posit_fwd.hpp>`

**File**: `include/sw/blas/ext/solvers/posit_fused_lu.hpp` (line 9)
- `#include <universal/number/posit1/posit_fwd.hpp>` → `#include <universal/number/posit/posit_fwd.hpp>`

No template signature changes needed — `posit<nbits, es>` resolves to `posit<nbits, es, uint8_t>` via default bt.

## Step 8: Migrate consumer files

All files below: change `#include <universal/number/posit1/posit1.hpp>` → `#include <universal/number/posit/posit.hpp>`

### Test files
1. `static/quire/api/api.cpp`
2. `static/quire/arithmetic/arithmetic.cpp`

### Application files
3. `applications/reproducibility/blas/norms.cpp`
4. `applications/reproducibility/blas/l1_fused_dot.cpp`
5. `applications/reproducibility/blas/lu.cpp`
6. `applications/reproducibility/blas/l3_fused_mm.cpp`
7. `applications/accuracy/optimization/error_vs_cost.cpp`
8. `applications/precision/numeric/residual.cpp`

### Linear algebra files
9. `linalg/blas/vector_ops.cpp`
10. `linalg/blas/matrix_ops.cpp`

### Education files
11. `education/quire/quires.cpp`
12. `education/number/posit/signalling_nar.cpp`
13. `education/number/posit/exceptions.cpp`

### Benchmark files
14. `benchmark/accuracy/blas/gemm.cpp`
15. `benchmark/range/blas/gemm.cpp`
16. `benchmark/energy/blas/gemm.cpp`
17. `benchmark/performance/blas/gemm.cpp`
18. `benchmark/reproducibility/blas/gemm.cpp`

### Tools
19. `tools/cmd/propp.cpp`

**Note**: Files that use `normalize_to<>()` (a posit1-specific method) may need adaptation. Will check during implementation.

## Step 9: Verification

1. `cd build && cmake -DUNIVERSAL_BUILD_NUMBER_POSITS=ON ..`
2. Build quire tests: `make -j4 quire_api quire_arithmetic`
3. Run them and verify pass
4. Build a consumer: `make -j4 l1_fused_dot` (or whichever is wired into CMake)
5. Run all existing tests: `ctest` — confirm 934+ tests still green
6. Build with clang to verify portability (clang is stricter on UB)
