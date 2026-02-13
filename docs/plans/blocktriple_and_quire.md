# Rewrite Atomic Fused Operators to blocktriple<> and Extract Quire from posit.hpp

## Context

The new `posit<nbits,es,bt>` (3-param) is fully limb-based: all arithmetic uses `blocktriple<>` and `blocksignificand<>`. However, the atomic fused operators (`fma`, `fam`, `fmma`) currently use the old bitset-based `internal::value<>`, `bitblock<>`, `module_multiply()`, and `module_add()`. This contradicts the architectural goal of the migration: the posit implementation should have zero dependency on `value<>`.

Additionally, `posit.hpp` currently bundles the quire (which legitimately depends on `value<>`) as an unconditional include. This forces the `value<>` dependency onto every posit consumer. The quire should be isolated into its own standalone header that applications explicitly opt into.

**Goals:**
1. Rewrite `atomic_fused_operators.hpp` to use `blocktriple<>` exclusively (no `value<>`)
2. Remove quire/fdp includes from `posit.hpp` so the base posit has no `value<>` dependency
3. Move all `value<>`-dependent bridge functions into `quire.hpp` (they stay there for quire's future redesign)

## Part A: Rewrite Atomic Fused Operators with blocktriple<>

### Key Architecture

Posit's own arithmetic already demonstrates the pattern:
```
posit → normalizeAddition/normalizeMultiplication → blocktriple<fbits, OP, bt>
  → blocktriple::add/mul (unrounded full-precision result)
  → convert(blocktriple, posit) (single rounding step)
```

For **fused** operations, we chain two operations with only ONE final rounding:
- FMA: `mul → add → convert` (single round at end)
- FAM: `add → mul → convert` (single round at end)
- FMMA: `mul → mul → add → convert` (single round at end)

**Challenge:** `blocktriple::add()` and `blocktriple::mul()` require both operands to have the same `fbits` template parameter. When chaining `MUL→ADD`, the MUL result has precision `2*(fbits+1)` bits, but the ADD operand (c) has only `fbits` bits. We need to widen the narrower operand to match.

### Approach: Extract-and-Reconstruct

Use blocktriple's accessor methods (`sign()`, `scale()`, `significandscale()`, `at()`) to extract the logical (sign, realScale, fraction_bits) from an intermediate result, then reconstruct a blocktriple of the target operator type using `set(sign, scale, raw)` or `setsign()/setscale()/setbits()`.

This is the same approach used by `convert(blocktriple, posit)` at `posit_impl.hpp:378-413`.

### Step A1: Write helper functions

**File:** `include/sw/universal/number/posit/atomic_fused_operators.hpp`

Remove all `value<>`, `bitblock<>`, `module_multiply`, `module_add` usage. Replace with:

#### Helper 1: `extractToAdd()` — transfer any blocktriple to ADD type

```cpp
// Transfer a blocktriple<src_fbits, src_op> result into blocktriple<tgt_fbits, ADD>
// Used when chaining MUL→ADD (FMA, FMMA)
template<unsigned src_fbits, BlockTripleOperator src_op, typename bt, unsigned tgt_fbits>
void extractToAdd(const blocktriple<src_fbits, src_op, bt>& src,
                  blocktriple<tgt_fbits, BlockTripleOperator::ADD, bt>& tgt) {
    if (src.iszero()) { tgt.setzero(); return; }
    if (src.isnan() || src.isinf()) { tgt.setnan(); return; }

    using Src = blocktriple<src_fbits, src_op, bt>;
    using Tgt = blocktriple<tgt_fbits, BlockTripleOperator::ADD, bt>;
    int sigScale = src.significandscale();
    int realScale = src.scale() + sigScale;
    int msbPos = static_cast<int>(Src::radix) + sigScale; // hidden bit position in src

    tgt.setnormal();
    tgt.setsign(src.sign());
    tgt.setscale(realScale);

    // Extract fraction bits from src and place in ADD layout:
    // ADD layout: [3 int bits | hidden bit at tgt_fbits+3 | tgt_fbits fraction | 3 rounding]
    // setbits(raw) expects: hidden bit at position tgt_fbits, fraction below, shifted by rbits
    uint64_t raw = 0;
    for (unsigned i = 0; i < tgt_fbits; ++i) {
        int srcPos = msbPos - 1 - static_cast<int>(i); // bits below MSB
        if (srcPos >= 0 && srcPos < static_cast<int>(Src::bfbits) && src.at(unsigned(srcPos)))
            raw |= (1ull << (tgt_fbits - 1 - i));
    }
    raw |= (1ull << tgt_fbits);  // hidden bit
    raw <<= Tgt::rbits;          // rounding bit shift for ADD
    tgt.setbits(raw);
}
```

#### Helper 2: `normalizeAdditionWide()` — normalize posit to wider-than-natural ADD blocktriple

```cpp
// Normalize a posit into an ADD blocktriple wider than its natural fbits.
// Used when the other add operand has higher precision (e.g., a MUL product).
template<unsigned nbits, unsigned es, typename bt, unsigned tgt_fbits>
void normalizeAdditionWide(const posit<nbits, es, bt>& p,
                           blocktriple<tgt_fbits, BlockTripleOperator::ADD, bt>& tgt) {
    constexpr unsigned pf = nbits - 3 - es;  // posit's natural fraction bits
    using Tgt = blocktriple<tgt_fbits, BlockTripleOperator::ADD, bt>;
    if (p.isnar()) { tgt.setnan(); return; }
    if (p.iszero()) { tgt.setzero(); return; }

    tgt.setnormal();
    tgt.setsign(sign(p));
    tgt.setscale(scale(p));

    // Extract fraction → place with hidden bit at tgt_fbits, shifted by rbits
    blockbinary<pf, bt> frac = extract_fraction<nbits, es, bt, pf>(p);
    uint64_t raw = frac.to_ull();
    raw |= (1ull << pf);             // hidden bit at position pf
    raw <<= (tgt_fbits - pf);        // zero-extend to tgt_fbits width
    raw <<= Tgt::rbits;              // rounding bits
    tgt.setbits(raw);
}
```

#### Helper 3: `extractToMul()` — transfer any blocktriple to MUL type (for FAM)

Same pattern as `extractToAdd()` but no rounding-bit shift.

#### Helper 4: `normalizeMultiplicationWide()` — normalize posit to wider MUL blocktriple (for FAM)

Same pattern as `normalizeAdditionWide()` but no rounding-bit shift.

### Step A2: Rewrite FMA using blocktriple

```cpp
// FMA: fused multiply-add: a*b + c (single rounding at end)
template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> fma(const posit<nbits, es, bt>& a, const posit<nbits, es, bt>& b,
                         const posit<nbits, es, bt>& c) {
    constexpr unsigned fbits = nbits - 3 - es;
    constexpr unsigned mbits = 2 * (fbits + 1);  // MUL product precision

    posit<nbits, es, bt> result;
    result.setzero();
    if (a.isnar() || b.isnar() || c.isnar()) { result.setnar(); return result; }

    // Step 1: Multiply a * b via blocktriple
    if (a.iszero() || b.iszero()) {
        // product is zero, result = c
        return c;
    }
    blocktriple<fbits, BlockTripleOperator::MUL, bt> ma, mb, product;
    a.normalizeMultiplication(ma);
    b.normalizeMultiplication(mb);
    product.mul(ma, mb);

    if (c.iszero()) {
        // result = product only (single round)
        convert(product, result);
        return result;
    }

    // Step 2: Add product + c
    // Transfer product to ADD-type at product precision
    blocktriple<mbits, BlockTripleOperator::ADD, bt> add_product, add_c, sum;
    extractToAdd(product, add_product);
    normalizeAdditionWide<nbits, es, bt, mbits>(c, add_c);
    sum.add(add_product, add_c);

    if (sum.iszero()) { result.setzero(); return result; }
    convert(sum, result);
    return result;
}
```

### Step A3: Rewrite FAM using blocktriple

```cpp
// FAM: fused add-multiply: (a + b) * c (single rounding at end)
template<unsigned nbits, unsigned es, typename bt>
posit<nbits, es, bt> fam(const posit<nbits, es, bt>& a, ..., c) {
    constexpr unsigned fbits = nbits - 3 - es;
    // ADD result effective precision: fbits (with carry possible, handled by scale)
    // For the MUL step, both operands need the same fbits, so use fbits for both

    // Step 1: Add a + b via blocktriple (same as posit::operator+=)
    // ... handle NaR, zero special cases ...
    blocktriple<fbits, BlockTripleOperator::ADD, bt> aa, ab, sum;
    a.normalizeAddition(aa);
    b.normalizeAddition(ab);
    sum.add(aa, ab);

    // Step 2: Multiply sum * c
    // Transfer sum (ADD-type) to MUL-type, normalize c to MUL-type
    blocktriple<fbits, BlockTripleOperator::MUL, bt> mul_sum, mul_c, product;
    extractToMul(sum, mul_sum);
    c.normalizeMultiplication(mul_c);
    product.mul(mul_sum, mul_c);

    convert(product, result);
    return result;
}
```

### Step A4: Rewrite FMMA using blocktriple

```cpp
// FMMA: (a*b) +/- (c*d) — two MUL products added with single rounding
// Same MUL→ADD pattern as FMA, but with two products
```

### Step A5: Remove all value<> dependencies

After rewriting, `atomic_fused_operators.hpp` will contain:
- NO `#include <universal/internal/value/value.hpp>` (direct or indirect)
- NO `internal::value<>`, `bitblock<>`, `module_multiply()`, `module_add()`
- NO `extract_fraction_as_bitblock()`, `posit_to_value_for_fused()`
- Only uses: `blocktriple<>`, `posit`, `normalizeAddition/Multiplication()`, `extract_fraction<>()`, `sign()`, `scale()`, `convert(blocktriple, posit)`

## Part B: Extract Quire from posit.hpp

### Step B1: Remove quire/fdp includes from `posit.hpp`

**File:** `include/sw/universal/number/posit/posit.hpp`

Remove these lines:
```cpp
#include <universal/number/quire/exceptions.hpp>   // line 80
#include <universal/number/posit/quire.hpp>         // line 81
#include <universal/number/posit/fdp.hpp>           // line 89
```

Keep `atomic_fused_operators.hpp` (it no longer depends on value/quire).

Move it to just after `attributes.hpp`:
```cpp
#include <universal/number/posit/manipulators.hpp>
#include <universal/number/posit/attributes.hpp>
#include <universal/number/posit/atomic_fused_operators.hpp>   // blocktriple-based fma/fam/fmma
```

### Step B2: Remove quire-related forward declarations from `posit_fwd.hpp`

**File:** `include/sw/universal/number/posit/posit_fwd.hpp`

Remove:
- `template<unsigned fbits> class value;` forward declaration (line 16)
- `template<unsigned nbits, unsigned es, unsigned capacity> class quire;` (line 32)
- `template<unsigned nbits, unsigned es, typename bt> internal::value<2*(nbits-2-es)> quire_mul(...)` (line 33)
- `template<unsigned nbits, unsigned es, typename bt, unsigned fbits> posit<nbits,es,bt>& convert(const internal::value<fbits>&, ...)` (line 36)

Keep the blocktriple convert forward declaration (line 27).

### Step B3: Make `quire.hpp` standalone

**File:** `include/sw/universal/number/posit/quire.hpp`

Add at the top (before the bridge functions):
```cpp
#include <universal/number/posit/posit.hpp>  // bring in full posit infrastructure
```

This creates the correct dependency: `quire.hpp` depends on `posit.hpp`, not the other way around. No circular dependency since `posit.hpp` no longer includes `quire.hpp`.

### Step B4: Make `fdp.hpp` include `quire.hpp`

**File:** `include/sw/universal/number/posit/fdp.hpp`

Ensure it includes `quire.hpp`:
```cpp
#include <universal/number/posit/quire.hpp>  // for quire class and quire_mul
```

### Step B5: Update consumer files

All files that use quire/fdp must explicitly include the header. Add `#include <universal/number/posit/fdp.hpp>` (or `quire.hpp`) to:

1. `static/quire/api/api.cpp`
2. `static/quire/arithmetic/arithmetic.cpp`
3. `applications/reproducibility/blas/l1_fused_dot.cpp`
4. `applications/reproducibility/blas/norms.cpp`
5. `applications/reproducibility/blas/lu.cpp`
6. `applications/reproducibility/blas/l3_fused_mm.cpp`
7. `applications/precision/numeric/residual.cpp`
8. `education/quire/quires.cpp`
9. `tools/cmd/propp.cpp`
10. BLAS headers: `blas/ext/posit_fused_blas.hpp`, `blas/ext/solvers/posit_fused_lu.hpp`, `blas/cg_fdp_solvers.hpp`
11. Benchmark gemm files (if they use fdp)

Files that only use `fma()/fam()/fmma()` need no changes — those are still in `posit.hpp`.

## Verification

1. Build `posit_fused_ops` test (gcc): `make -j4 posit_fused_ops && ./static/posit/posit_fused_ops`
2. Build `posit_fused_ops` test (clang): repeat in `build_clang/`
3. Full gcc test suite: `ctest` — confirm 935/935 pass
4. Full clang test suite: `ctest` — confirm 935/935 pass
5. Verify no `value<>` in atomic_fused_operators.hpp: `grep -n "value\|bitblock\|module_" atomic_fused_operators.hpp`
6. Verify posit.hpp has no value<> dependency: compile a file that only includes posit.hpp and does NOT link value.hpp
