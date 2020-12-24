# Chaos Examples

This directory contains computational science examples in chaos theory and experiment
that demonstrate and quantify the benefits of using posit arithmetic.

## Precision

Posits control their precision by the number of bits in the encoding. They have their highest
precision around +-1.0 in regime 0 and 1. The number of fraction bits in those regimes is

```test
fbits = nbits - signBit - 2 regime bits - es exponent bits
```

## Dynamic Range

Posits control their dynamic range by the number of exponent bits. The number of exponent bits directly
controls the value of useed, which is the exponential shift, 2^2^es, of the regime.

IEEE float configurations from numeric_limits<Ty>
        float                       minexp scale       -125     maxexp scale        128     minimum  1.17549e-38     maximum  3.40282e+38
       double                       minexp scale      -1021     maxexp scale       1024     minimum 2.22507e-308     maximum 1.79769e+308
  long double                       minexp scale      -1021     maxexp scale       1024     minimum 2.22507e-308     maximum 1.79769e+308

## Reproducibility

The crown jewel of posit arithmetic is the ability to restore associative and distributive laws of algebra.

With IEEE floating point, associativity, that is

    y = (a + b) + c  -> y = a + (b + c)

does not hold. The reason is that with IEEE floating point arithmetic, each operation immediately rounds
the result to the available precision. This caused two different rounding paths for the two equations.

Similarly, distribution, that is

    y = (a + b) * c  -> y = a*c + b*c

also does not hold, for the same reason.
