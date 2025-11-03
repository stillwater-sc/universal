# Renormalization Improvement Plan for floatcascade<N>

**Date:** 2025-11-01
**Objective:** Fix non-overlapping property violation in floatcascade renormalization
**Problem:** Current renormalize() violates Priest's invariant by 3.24x, causing 60-70% precision loss in iterative algorithms
**Approach:** Research-driven, first-principles design of generalized renormalization algorithm

---

## Phase 1: Literature Research

### 1.1 Primary Sources

**Priest's Original Work:**
- **"Algorithms for Arbitrary Precision Floating Point Arithmetic" (1991)**
  - IEEE Symposium on Computer Arithmetic
  - Defines error-free transformations (two_sum, two_prod)
  - Establishes non-overlapping property requirements
  - Describes renormalization algorithms for double-double

**Hida-Li-Bailey QD Library:**
- **"Library for Double-Double and Quad-Double Arithmetic" (2000)**
  - NERSC Technical Report LBNL-46996
  - Extends Priest's work to quad-double (4 components)
  - Documents practical renormalization strategies
  - Discusses performance vs. precision tradeoffs

**Bailey's Follow-up Work:**
- **"High-Precision Floating-Point Arithmetic in Scientific Computation" (2005)**
  - Computing in Science & Engineering
  - Real-world applications of multi-component arithmetic
  - Numerical stability analysis

### 1.2 Search Strategy

**Online Resources:**
1. Google Scholar search: "Priest arbitrary precision floating point"
2. IEEE Xplore: Papers citing Priest (1991)
3. LBNL Technical Reports: QD library documentation
4. arXiv.org: Recent developments in extended precision arithmetic

**Key Questions to Answer:**
- What are the **exact** requirements for the non-overlapping property?
- How many renormalization passes are theoretically necessary?
- What is the algorithmic complexity of correct renormalization?
- Are there different strategies for different N (double-double vs quad-double vs octo-double)?
- What error bounds can we prove for the renormalization?

### 1.3 Documentation Deliverable

Create: `renormalization_theory.md`

**Contents:**
- Summary of Priest's invariant definition
- Theoretical requirements for renormalization
- Comparison of known algorithms (Priest, Hida-Li-Bailey, others)
- Complexity analysis: time vs. precision tradeoffs
- Proof sketch of correctness for our proposed algorithm
- References and citations

---

## Phase 2: Algorithm Analysis

### 2.1 Compare Existing Implementations

**Implementations to Study:**

1. **floatcascade<N>** (current)
   - File: `include/sw/universal/internal/floatcascade/floatcascade.hpp:532-546`
   - Algorithm: Single-pass accumulation
   - Pros: Simple, generalizes to any N
   - Cons: Violates non-overlapping by 3.24x

2. **Classic qd renorm()** (4 components)
   - File: `include/sw/universal/numerics/error_free_ops.hpp:312-342`
   - Algorithm: Two-phase compression + conditional refinement
   - Pros: Battle-tested, appears to work
   - Cons: Hand-coded for qd, complex control flow

3. **Classic dd renorm()** (if exists - 2 components)
   - Check: `include/sw/universal/number/dd/` directory
   - Simpler case - may reveal pattern

4. **QD Library Source** (reference implementation)
   - URL: https://www.davidhbailey.com/dhbsoftware/
   - Download original QD library source
   - Study `qd_real::renormalize()` implementation
   - Check for comments explaining design decisions

### 2.2 Identify Patterns

**Questions:**
- Does complexity scale with N? (linear, quadratic, other?)
- Is there a "compression phase" followed by "refinement phase" pattern?
- How do conditional branches help? (zero detection, early termination?)
- What role does `quick_two_sum` vs `two_sum` play?
- Are there symmetries we can exploit?

### 2.3 Verification Strategy

**How do we know renormalization is correct?**

1. **Property Testing:**
   ```cpp
   bool verify_non_overlapping(floatcascade<N>& fc) {
       for (i = 0; i < N-1; i++) {
           double ulp_i = ldexp(1.0, ilogb(fc[i]) - 52);
           if (abs(fc[i+1]) > ulp_i / 2.0) return false;
       }
       return true;
   }
   ```

2. **Identity Testing:**
   ```cpp
   // (a + b) - a should equal b to full precision
   floatcascade<N> sum = a + b;
   floatcascade<N> diff = sum - a;
   assert(diff == b);  // All N components must match
   ```

3. **Stress Testing:**
   - Random values across many orders of magnitude
   - Pathological cases (numbers near zero, near overflow, denormals)
   - Repeated operations: `renorm(renorm(renorm(x)))` should be idempotent

---

## Phase 3: Algorithm Design

### 3.1 Design Principles

**Must-Have Properties:**
1. **Correctness:** Strictly enforce `|c[i+1]| ≤ ulp(c[i])/2` for all i
2. **Generality:** Work for any N ≥ 2 (dd, td, qd, od, ...)
3. **Efficiency:** O(N) or O(N²) time complexity (avoid O(N³) or worse)
4. **Robustness:** Handle edge cases (zeros, infinities, denormals)
5. **Provability:** Algorithm correctness should be verifiable

**Nice-to-Have Properties:**
1. **Simplicity:** Easier to understand and maintain than qd's conditional maze
2. **Idempotency:** `renorm(renorm(x)) == renorm(x)`
3. **Stability:** Small input changes → small output changes
4. **Compiler-Friendly:** Volatile modifiers don't prevent all optimizations

### 3.2 Proposed Algorithm Structure

**Hypothesis:** Generalized two-phase algorithm

**Phase 1: Compression**
- Combine components from least to most significant
- Use quick_two_sum for ordered operands
- Generate N temporary components

**Phase 2: Refinement**
- Iteratively tighten non-overlapping bounds
- Detect and redistribute excess magnitude
- Continue until all components satisfy invariant

**Pseudocode (Draft):**
```cpp
template<size_t N>
floatcascade<N> renormalize(const floatcascade<N>& e) {
    floatcascade<N> result = e;

    // Phase 1: Initial compression (bottom-up accumulation)
    for (int pass = 0; pass < MAX_PASSES; ++pass) {
        volatile double s = result[N-1];

        for (int i = N-2; i >= 0; --i) {
            volatile double hi, lo;
            quick_two_sum(s, result[i], hi, lo);
            result[i+1] = lo;
            s = hi;
        }
        result[0] = s;

        // Phase 2: Verification and refinement
        if (verify_non_overlapping(result)) {
            return result;  // Done - all components satisfy invariant
        }

        // Not yet converged - refine further
        // (Specific refinement strategy TBD based on research)
    }

    // If we get here, renormalization didn't converge
    // This should never happen with correct algorithm
    return result;
}
```

**Open Questions for Research:**
- How many passes (MAX_PASSES) are theoretically sufficient?
- Should refinement be different from compression?
- Can we prove convergence?
- What's the best verification strategy during iteration?

### 3.3 Alternative Approaches

If two-phase doesn't work well:

**Option A: Adaptive Algorithm**
- Detect which components violate invariant
- Apply targeted refinement only where needed
- May be faster for nearly-normalized inputs

**Option B: Sorting-Based**
- Sort all components by magnitude
- Merge adjacent components that overlap
- Re-sort and repeat until converged

**Option C: Iterative Improvement**
- Start with current simple algorithm
- Measure violation factors
- Apply correction proportional to violation
- Iterate until below threshold

---

## Phase 4: Implementation

### 4.1 Test Infrastructure First

**Before changing renormalize(), create comprehensive tests:**

File: `internal/floatcascade/arithmetic/renormalize_verification.cpp`

**Tests:**
1. **Property Test:** Verify non-overlapping for N ∈ {2, 3, 4, 8}
2. **Identity Test:** (a+b)-a == b for all components
3. **Idempotency Test:** renorm(renorm(x)) == renorm(x)
4. **Stress Test:** Random values, edge cases, denormals
5. **Regression Test:** Ensure multiplication precision stays 212+ bits

### 4.2 Octo-Double Implementation

**Why N=8?**
- Tests scalability beyond hand-coded dd/qd
- 8 components = 424 bits = ~128 decimal digits
- Useful for ultra-high-precision applications
- If algorithm works for N=8, it works for any N

**Files to Create:**

1. **`include/sw/universal/number/od_cascade/od_cascade.hpp`**
   ```cpp
   namespace sw { namespace universal {
       // Octo-double using floatcascade<8>
       using od_cascade = floatcascade<8>;

       // Constants
       constexpr int odc_max_precision = 424;  // 8 * 53 bits
       constexpr double odc_eps = /* 2^-420 */;
       // ... other constants
   }}
   ```

2. **`static/od_cascade/arithmetic/od_arith_add.cpp`**
   - Basic addition tests
   - Verify renormalization with 8 components

3. **`static/od_cascade/math/od_math_sqrt.cpp`**
   - Test mathematical function with 8 components
   - Verify precision doesn't degrade

### 4.3 Implementation Strategy

**Incremental Development:**

**Step 1:** Implement renormalize() v2 with verification logging
```cpp
template<size_t N>
floatcascade<N> renormalize_v2(const floatcascade<N>& e, bool verbose = false) {
    // ... new algorithm ...

    if (verbose) {
        std::cout << "Pass " << pass << ": max_violation = " << max_violation << "\n";
    }
}
```

**Step 2:** Run side-by-side comparison
```cpp
auto result_old = renormalize(x);     // Current implementation
auto result_new = renormalize_v2(x);  // New implementation

verify_non_overlapping(result_old);   // Expected to fail
verify_non_overlapping(result_new);   // Expected to pass
```

**Step 3:** Gradual rollout
- Test with dd_cascade (N=2) first - simplest case
- Then td_cascade (N=3)
- Then qd_cascade (N=4) - the problem case
- Finally od_cascade (N=8) - the stress test

**Step 4:** Replace old renormalize()
- Only after all tests pass
- Keep old version commented out for reference
- Document the change in floatcascade_volatile_hardening.md style

---

## Phase 5: Verification and Testing

### 5.1 Unit Tests

**Test Matrix:**

| N | Test | Expected Result |
|---|------|-----------------|
| 2 | Non-overlapping | PASS (violation < 1.01x) |
| 3 | Non-overlapping | PASS (violation < 1.01x) |
| 4 | Non-overlapping | PASS (violation < 1.01x) |
| 8 | Non-overlapping | PASS (violation < 1.01x) |
| 2 | Identity (a+b)-a=b | PASS (all components) |
| 3 | Identity (a+b)-a=b | PASS (all components) |
| 4 | Identity (a+b)-a=b | PASS (all components) |
| 8 | Identity (a+b)-a=b | PASS (all components) |
| 2 | Idempotency | PASS |
| 3 | Idempotency | PASS |
| 4 | Idempotency | PASS |
| 8 | Idempotency | PASS |

### 5.2 Integration Tests

**Multiplication Precision:**
- Re-run `multiplication_precision.cpp`
- Expected: Test 3 now PASSES (non-overlapping satisfied)
- Expected: Tests 1, 2, 4 still PASS (no regression)

**pow() Precision:**
- Re-enable qd_cascade pow() tests in CI
- Expected: Precision improves from 77-92 bits to 200+ bits
- Expected: All pow() tests PASS with 85-bit threshold

### 5.3 Performance Benchmarks

**Measure:**
- Time for single renormalize() call
- Time for 1000 multiplication operations (renorm after each)
- Compare old vs new implementation

**Acceptance Criteria:**
- New algorithm may be slower, but not > 2x slower
- If significantly slower, consider optimizations or hybrid approach

---

## Phase 6: Documentation

### 6.1 Technical Documentation

**Update Files:**

1. **`renormalization_theory.md`** (new)
   - Literature review summary
   - Algorithm description
   - Proof of correctness
   - Complexity analysis

2. **`multiplication_precision_rca.md`** (update)
   - Add section: "Resolution: Improved Renormalization"
   - Before/after comparison
   - Performance impact analysis

3. **`floatcascade.hpp`** (inline comments)
   - Detailed algorithm explanation
   - Why each phase is necessary
   - Reference to theory document

### 6.2 User-Facing Documentation

**CHANGELOG Entry:**
```markdown
## [Version X.XX] - 2025-11-XX

### Fixed
- **floatcascade renormalization:** Fixed non-overlapping property violation
  that caused 60-70% precision loss in iterative algorithms (exp, log, pow).
  - Issue: Old renormalize() violated Priest's invariant by 3.24x
  - Fix: Implemented generalized two-phase renormalization algorithm
  - Impact: qd_cascade pow() precision improved from 77-92 bits to 200+ bits
  - See: `internal/floatcascade/arithmetic/multiplication_precision_rca.md`
```

---

## Success Criteria

### Must-Have (Required for Success)

✅ **Correctness:**
- [ ] Non-overlapping property holds for all N ∈ {2, 3, 4, 8}
- [ ] Violation factor < 1.01x (vs current 3.24x)
- [ ] All identity tests pass to full precision

✅ **Precision Improvement:**
- [ ] qd_cascade multiplication: Non-overlapping test PASSES
- [ ] qd_cascade pow(): Achieves 200+ bits (vs current 77-92)
- [ ] All cascade math functions improve proportionally

✅ **Generality:**
- [ ] Algorithm works for any N (tested up to N=8)
- [ ] No hard-coded constants for specific N
- [ ] Clean, understandable code

### Nice-to-Have (Desired but not Blocking)

⭐ **Performance:**
- [ ] New renormalize() within 2x of old version
- [ ] Optimizations applied where possible
- [ ] No performance regression in math functions

⭐ **Simplicity:**
- [ ] Algorithm easier to understand than qd's conditional maze
- [ ] Provable correctness
- [ ] Well-documented with references

---

## Timeline Estimate

**Research Phase (Phase 1-2):** 2-3 hours
- Literature search and reading
- Algorithm analysis and pattern identification
- Documentation of findings

**Design Phase (Phase 3):** 1-2 hours
- Algorithm design based on research
- Pseudocode and proof sketch
- Decision on approach

**Implementation Phase (Phase 4):** 3-4 hours
- Test infrastructure creation
- Octo-double implementation
- New renormalize() implementation
- Side-by-side comparison

**Verification Phase (Phase 5):** 1-2 hours
- Run full test suite
- Performance benchmarks
- Bug fixes and refinement

**Documentation Phase (Phase 6):** 1 hour
- Update markdown files
- Code comments
- CHANGELOG entry

**Total: ~8-12 hours of focused work**

---

## Risk Analysis

### Technical Risks

**Risk 1: Algorithm Doesn't Converge**
- *Likelihood:* Low
- *Impact:* High (blocks entire fix)
- *Mitigation:* Start with proven qd approach, adapt gradually

**Risk 2: Performance Unacceptable**
- *Likelihood:* Medium
- *Impact:* Medium (may need optimizations)
- *Mitigation:* Profile early, optimize hot paths, consider hybrid approach

**Risk 3: Works for N=4 but not N=8**
- *Likelihood:* Low
- *Impact:* Medium (generalization fails)
- *Mitigation:* Test incrementally (N=2, 3, 4 before 8)

### Research Risks

**Risk 4: Can't Find Priest's Original Paper**
- *Likelihood:* Low (widely cited)
- *Impact:* Low (qd library source is available)
- *Mitigation:* Multiple sources, contact library maintainers if needed

**Risk 5: Literature Doesn't Address N>4**
- *Likelihood:* Medium
- *Impact:* Low (can extrapolate from patterns)
- *Mitigation:* Derive from first principles if needed

---

## Open Questions

1. **Convergence:** Is there a mathematical proof that two-phase renormalization converges in finite passes?

2. **Optimality:** Is the qd algorithm optimal, or can we do better?

3. **Hardware:** Do modern CPUs have instructions (FMA, etc.) that could help?

4. **Alternatives:** Are there non-iterative renormalization algorithms?

5. **Verification:** Can we prove correctness statically (e.g., with formal methods)?

---

## Next Actions

**Immediate (Today):**
1. Search for and download Priest (1991) paper
2. Search for and download Hida-Li-Bailey QD library papers
3. Download QD library source code for reference
4. Create `renormalization_theory.md` to capture findings

**Near-Term (This Week):**
1. Complete literature review
2. Document requirements and constraints
3. Design generalized algorithm
4. Implement test infrastructure

**Medium-Term (Next Week):**
1. Implement improved renormalize()
2. Test with N ∈ {2, 3, 4, 8}
3. Verify precision improvements
4. Complete documentation

---

## References

**To Be Populated During Research Phase**

1. Priest, D. M. (1991). "Algorithms for Arbitrary Precision Floating Point Arithmetic." *Proceedings of the 10th IEEE Symposium on Computer Arithmetic*, pp. 132-143.

2. Hida, Y., Li, X. S., & Bailey, D. H. (2000). "Library for Double-Double and Quad-Double Arithmetic." *NERSC Technical Report LBNL-46996*, Lawrence Berkeley National Laboratory.

3. Bailey, D. H. (2005). "High-Precision Floating-Point Arithmetic in Scientific Computation." *Computing in Science & Engineering*, 7(3), pp. 54-61.

4. QD Library Source Code: https://www.davidhbailey.com/dhbsoftware/

5. [Additional references to be added as discovered]

---

**Document Status:** Planning Complete - Ready for Execution
**Next Step:** Begin Phase 1 - Literature Research
**Owner:** Claude Code investigation team
**Last Updated:** 2025-11-01
