# Examples of the benefit of using custom number systems

This directory contains computational science examples that demonstrate and quantify the benefits
of using custom arithmetic types, such as fixed-point and posits, to solve computational problems.

## Accuracy

Ginormous posit configurations

```text
 posit< 80,2> useed scale     4     minpos scale       -312     maxpos scale        312
 posit< 80,3> useed scale     8     minpos scale       -624     maxpos scale        624
 posit< 80,4> useed scale    16     minpos scale      -1248     maxpos scale       1248
 posit< 96,2> useed scale     4     minpos scale       -376     maxpos scale        376
 posit< 96,3> useed scale     8     minpos scale       -752     maxpos scale        752
 posit< 96,4> useed scale    16     minpos scale      -1504     maxpos scale       1504
 posit<112,2> useed scale     4     minpos scale       -440     maxpos scale        440
 posit<112,3> useed scale     8     minpos scale       -880     maxpos scale        880
 posit<112,4> useed scale    16     minpos scale      -1760     maxpos scale       1760
 posit<128,2> useed scale     4     minpos scale       -504     maxpos scale        504
 posit<128,3> useed scale     8     minpos scale      -1008     maxpos scale       1008
 posit<128,4> useed scale    16     minpos scale      -2016     maxpos scale       2016
```

## Dynamic Range

Posits control their dynamic range by the number of exponent bits. The number of exponent bits directly
controls the value of useed, which is the exponential shift, 2^2^es, of the regime.

This yields the ability of very small posits to still cover a very large dynamic range:
Extended Modified Standard posit configurations

```text
 posit<  4,0> useed scale     1     minpos scale         -2     maxpos scale          2
 posit<  4,1> useed scale     2     minpos scale         -4     maxpos scale          4
 posit<  8,0> useed scale     1     minpos scale         -6     maxpos scale          6
 posit<  8,1> useed scale     2     minpos scale        -12     maxpos scale         12
 posit<  8,2> useed scale     4     minpos scale        -24     maxpos scale         24
 posit<  8,3> useed scale     8     minpos scale        -48     maxpos scale         48
 posit<  8,4> useed scale    16     minpos scale        -96     maxpos scale         96
 posit< 16,0> useed scale     1     minpos scale        -14     maxpos scale         14
 posit< 16,1> useed scale     2     minpos scale        -28     maxpos scale         28
 posit< 16,2> useed scale     4     minpos scale        -56     maxpos scale         56
 posit< 16,3> useed scale     8     minpos scale       -112     maxpos scale        112
 posit< 16,4> useed scale    16     minpos scale       -224     maxpos scale        224
 posit< 32,0> useed scale     1     minpos scale        -30     maxpos scale         30
 posit< 32,1> useed scale     2     minpos scale        -60     maxpos scale         60
 posit< 32,2> useed scale     4     minpos scale       -120     maxpos scale        120
 posit< 32,3> useed scale     8     minpos scale       -240     maxpos scale        240
 posit< 32,4> useed scale    16     minpos scale       -480     maxpos scale        480
 posit< 64,0> useed scale     1     minpos scale        -62     maxpos scale         62
 posit< 64,1> useed scale     2     minpos scale       -124     maxpos scale        124
 posit< 64,2> useed scale     4     minpos scale       -248     maxpos scale        248
 posit< 64,3> useed scale     8     minpos scale       -496     maxpos scale        496
 posit< 64,4> useed scale    16     minpos scale       -992     maxpos scale        992

IEEE float configurations from numeric_limits<Ty>
        float                       minexp scale       -125     maxexp scale        128     minimum  1.17549e-38     maximum  3.40282e+38
       double                       minexp scale      -1021     maxexp scale       1024     minimum 2.22507e-308     maximum 1.79769e+308
  long double                       minexp scale      -1021     maxexp scale       1024     minimum 2.22507e-308     maximum 1.79769e+308
```

## Reproducibility

The crown jewel of posit arithmetic is the ability to restore associative and distributive laws of algebra.

With IEEE floating point, associativity, that is

```eq
    y = (a + b) + c    =>    y = a + (b + c)
```

does not hold. The reason is that with IEEE floating point arithmetic, each operation immediately rounds
the result to the available precision. This creates two different rounding paths for the two equations, and depending on the precision and dynamic range of the results creates the possibility of rounding differences.

Similarly, distribution, that is

```eq
    y = (a + b) * c    =>    y = a*c + b*c
```

also fails for IEEE floating point, for the same reason.

### The Kulisch super-accumulator

The solution to this problem is simple: do not round intermediate steps. This requires an accumulator that is able to faithfully represent the product of the smallest number and the biggest number in the number system. And if we want to accumulate a collection of these products, we will also need some capacity bits to capture the overflow carries.

The fact that the accumulator must be able to represent the products of the smallest and largest number in the number system makes it clear that the dynamic range of the number system will have a dramatic impact on the size of this super-accumulator. For example, to provide an accumulator for IEEE double precision floats requires 4100 bits. 

With posits we can control precision and dynamic range independently, which creates more opportunities to apply a super-accumulator, which is dubbed `quire` in the posit standard. For example, the quire for a standard 64-bit posit is just 1024 bits.
