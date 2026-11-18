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
| `exact_bound_reference` | exact dyadic rationals; the result is checked against TestType's error bound for the operator, pass or fail with the measured error, no golden value (#2124) | types guaranteed to a bound, not correctly rounded: dd, qd, the cascades; needs an `exact_bound<TestType>` specialization |

```cpp
VerifyBinaryOperatorThroughRandoms<posit<64, 2>>(report, OPCODE_DIV, 1000);                   // default: exact
VerifyBinaryOperatorThroughRandoms<posit<32, 2>, exact_reference>(report, OPCODE_DIV, 1000);  // exact on request
VerifyBinaryOperatorThroughRandoms<posit<64, 2>, double>(report, OPCODE_DIV, 1000);           // compile error
```

`default_reference_t<TestType>` is `double` where double holds the results, and
otherwise `exact_reference` or `exact_bound_reference`, whichever the type has. The exact reference covers `+ - * /` and `sqrt`; the
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

dd, dd_cascade, td_cascade, qd and qd_cascade have the exact bound reference
(`multidouble_bound_oracle.hpp`). It checks each result against a bound per operator
in units of epsilon; each bound is recorded with its source in that header, a
published double-word bound where one applies (Joldes, Muller, Popescu 2017). The
random operands are normalized values (every limb below half an ulp of the one above
it), with cancellation cases for + and -. A bound check catches an error larger than
the bound, not every error: one unit in the last place of the last limb lies inside
every bound.

lns has an exact reference (`lns_rounding_oracle.hpp`, #2125). lns rounds in the log
domain: the exponent is the integer nearest to `2^rbits * log2|X|`, ties to even. `*`,
`/` and sqrt are exact integer arithmetic on the exponent. `+`, `-`, exp and exp2
are evaluated in efloat with Ziv's retry to a 2048-bit cap; a result still within the
error bound of a tie there is undecided: the verifiers count and report it, neither
pass nor fail. The reference covers both `Behavior`s, Saturating and Wrapping, and
`VerifyElementaryFunctionThroughRandoms` takes the reference parameter too (exp, exp2).
It shows lns `+ - exp exp2` not correctly rounded past about 45 fraction bits (#2128).
