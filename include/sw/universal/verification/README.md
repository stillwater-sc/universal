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

Posit and cfloat have an exact reference. Each has its own decoder over the encoding
and rounds at the (nbits+1)-bit midpoint pattern, on the shared exact arithmetic in
`rounding_oracle_core.hpp`:
- posit: `posit_rounding_oracle.hpp`;
- cfloat: `cfloat_rounding_oracle.hpp`, covering every combination of subnormals,
  max-exponent values and saturation, and IEEE-754 special values (#2123).

The generic `test_suite_randoms.hpp` includes both oracles, and
`posit_test_suite_randoms.hpp` the posit one, so no test has to include an oracle itself. Random operands fill every bit of the encoding,
also past 64 bits, for a type with an exact reference; dd and the other multi-component
types keep `setbits`, as a random pattern of their limbs is not a normalized value.

These types have no exact reference yet and still check against double, also past
53 digits: dd, qd and the cascades (#2124), and lns (#2125). Each moves to the exact
reference by specializing `exact_rounding`.
