# Atmospheric and Oceanographic Examples

This directory contains computational science examples in weather forcasting
and oceanography that demonstrate and quantify the benefits of using posit arithmetic.

## Precision

Posits control their precision by the number of bits in the encoding. Since weather is a chaotic system
precision is of limited use as the error in boundary conditions will quickly over-shadown any 
numerical error. It is shown that weather simulations can be done with single precision IEEE
and 16-bit posits with associated quire and fused-dot-product operators.

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
