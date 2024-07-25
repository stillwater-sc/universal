# Computational Mathematics Examples

This directory contains computational Mathematics examples that demonstrate and quantify the benefits
of using custom arithmetic to solve difficult numerical calculations using approximated real numbers.

## Accuracy

Posits control their precision by the difference between `nbits`and `es`. The number of fraction bits in the posit
is given by the equation: 

`fraction_bits = nbits - sign_bit - regime_bits - exponent_bits`

Around `1` and `-1`, the regime field is encoded in its minimum form of 2 bits. 
At the extreme regimes, the posit encoding will have no fraction bits, and effectively be an exponential.

## Dynamic Range

Posits control their dynamic range by the number of exponent bits. The number of exponent bits directly
controls the value of useed, which is the exponential shift, 2^2^es, of the regime.

```text
IEEE float configurations from numeric_limits<Ty>
        float                       minexp scale       -125     maxexp scale        128     minimum  1.17549e-38     maximum  3.40282e+38
       double                       minexp scale      -1021     maxexp scale       1024     minimum 2.22507e-308     maximum 1.79769e+308
  long double                       minexp scale      -1021     maxexp scale       1024     minimum 2.22507e-308     maximum 1.79769e+308
```

## Reproducibility

The crown jewel of posit arithmetic is the ability to restore associative and distributive laws of algebra.

With IEEE floating point, associativity, that is

```eq
    y = (a + b) + c  -> y = a + (b + c)
```

does not hold. The reason is that with IEEE floating point arithmetic, each operation immediately rounds
the result to the available precision. This caused two different rounding paths for the two equations.

Similarly, distribution, that is

```eq
    y = (a + b) * c  -> y = a*c + b*c
```

also does not hold, for the same reason.
