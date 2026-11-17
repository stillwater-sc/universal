# Verification

This directory contains verification functions.

## The reference type of the random-operand verifiers

`VerifyBinaryOperatorThroughRandoms` and `VerifyUnaryOperatorThroughRandoms`
(`test_suite_randoms.hpp`, `posit_test_suite_randoms.hpp`) check an operator against
a reference rounded once to the type under test. The reference type is their second
template parameter (`exact_reference.hpp`, #2121):

| ReferenceType | computes in | holds the results of |
|---|---|---|
| `double` | double, rounded once to TestType (`correctly_rounded_reference.hpp`, #1861) | types with fewer than 53 digits and a range inside double's (`posit<32,6>`, reaching 2^1920, is not one) |
| `exact_reference` | exact dyadic rationals, rounded by TestType's own rule; a quotient or square root is compared, never computed | any width; needs an `exact_rounding<TestType>` specialization |

```cpp
VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(report, OPCODE_DIV, 1000);                   // default: exact
VerifyBinaryOperatorThroughRandoms<posit<32, 2>, exact_reference>(report, OPCODE_DIV, 1000);  // exact on request
VerifyBinaryOperatorThroughRandoms<posit<64, 2>, double>(report, OPCODE_DIV, 1000);           // compile error
```

`default_reference_t<TestType>` is `double` where double holds the results and
`exact_reference` otherwise. The exact reference covers `+ - * /` and `sqrt`; the
verifiers cancel any other operator with it.

Posit has an exact reference (`posit_rounding_oracle.hpp`: its own decoder, and the
(nbits+1)-bit midpoint patterns). Both verifier headers include it, so a posit test
that includes only the generic `test_suite_randoms.hpp` gets it too. Types without one still check against double,
also past 53 digits: dd, qd, the cascades, lns, and cfloat wider than double
(c80, c96, c128). Each moves to the exact reference by specializing `exact_rounding`.
