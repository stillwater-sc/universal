# Posit DIV bug: Issue #534

## Division Bug RCA

Two bugs found in posit2's division pipeline:

Bug 1 — Missing setradix() in normalizeDivision (systematic, the original #534 bug)

  - File: include/sw/universal/number/posit/posit_impl.hpp:1102
  - Cause: The multi-limb path (else branch) of normalizeDivision() did not call tgt.setradix() to set the blocksignificand's radix point. The if constexpr
  fast path used tgt.setbits(raw) which internally calls setradix(), but the else path for wider posits used setblock()/setbit()/bitShift() which do not.
  - Effect: blocksignificand::div() reads lhs.radix() to determine iteration count and quotient bit placement. With the default radixPoint (bfbits=87 instead
   of correct divbits=85 for posit<32,2>), quotient bits were placed 2 positions too high, causing results like 10/2=4.
  - Threshold: Manifested at nbits >= 24 (where divshift >= 64 - fbits, triggering the else branch).
  - Fix: Added tgt.setradix() in the else branch, matching the pattern used in normalizeAddition's else branch at line 1025.

Bug 2 — Missing sticky bits in convert() (rounding, causing -1 ULP errors)

  - File: include/sw/universal/number/posit/posit_impl.hpp:412
  - Cause: The convert() function extracts only nbits + 4 fraction bits from the blocktriple into a blocksignificand for rounding. For DIV, the blocktriple
  has 3*fbits + 4 fraction bits — far more than extracted. The remaining bits were silently discarded without contributing to the sticky bit in convert_().
  - Effect: Round-to-even decisions were wrong in ~0.1-0.3% of cases, producing results exactly 1 ULP below the correct value.
  - Fix: After the extraction loop, check if any blocktriple bits below the extracted range are set via v.any(), and fold them into the lowest bit of the
  extracted fraction (bit 0) as sticky information.

## Regression Test Completion

File: static/tapered/posit/arithmetic/division.cpp

  - Added #include <universal/verification/posit_test_suite_randoms.hpp>
  - Added GenerateWorstCaseDivision, EnumerateToughDivisions, and ToughDivisions2 adversarial test functions (ported from posit1, adapted for posit2's API:
  .get() → to_binary())
  - Added posit<3,2> and posit<3,3> to Level 1 (matching posit1)
  - Level 2: Added VerifyBinaryOperatorThroughRandoms for posit<16,2> and posit<24,2> (1000 randoms)
  - Level 3: Added randoms for posit<20,1>, posit<28,1>, posit<32,1>, posit<32,2>, posit<32,3>
  - Level 4: Added randoms for posit<48,2>, posit<64,2>, posit<64,3>, posit<64,4>
  - Matched posit1's complete test structure exactly

## Shared Test Suite Compatibility

File: include/sw/universal/verification/posit_test_suite_randoms.hpp

  - Wrapped Compare() and VerifyConversionThroughRandoms() with #if defined(BITBLOCK_THROW_ARITHMETIC_EXCEPTION) guard, since these functions use
  posit1-specific types (bitblock, .get(), .set(), truncate)
  - VerifyBinaryOperatorThroughRandoms (templated on TestType) works with both posit and posit1 — shared test suite achieved

## Verification

  - Both bugs fixed, tested with gcc AND clang
  - posit2 division: all exhaustive tests pass (nbits 2-10), all random tests pass through posit<48,2> (matching posit1)
  - posit<64,*> failures are expected in both posit and posit1 (double oracle precision limitation)
  - posit1 division: unaffected by changes, still passes
  - posit2 addition, subtraction, multiplication: no regressions

