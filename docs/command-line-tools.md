# Universal number system Command Line Tools

The universal library contains a collection of command line tools that help investigate
bit-level attributes of the number systems and the values they encode. These command line
tools get installed with the `make install` build target.

Most type-specific inspection tools (quarter, half, single, double, quad, fixpnt,
signedint, unsignedint, posit, float2posit) have been consolidated into
[ucalc](ucalc/README.md), the interactive mixed-precision calculator. ucalc provides
the same inspection capabilities via `show`, `compare`, `precision`, and `range` commands
across all 42+ number types. See the [ucalc documentation](ucalc/README.md) for details.

The remaining standalone tools are described below.

## ieee

Compare the three IEEE formats on a given real number value:

```text
$ ieee
Show the truncated value and (sign/scale/fraction) components of different floating point types.
Usage: ieee floating_point_value
Example: ieee 0.03124999
input value:                0.03124999
      float:              0.0312499907 (+,-6,11111111111111111111011)
     double:      0.031249989999999998 (+,-6,1111111111111111111101010100001100111000100011101110)
long double:  0.0312499899999999983247 (+,-6,111111111111111111101001011110100011111111111110001111111001111)
```

## longdouble

Show the sign/scale/fraction components of an IEEE-754 long double.

On Windows using the Microsoft Visual Studio environment, the `long double` is aliased to `double`.

```text
$ longdouble
longdouble: components of an IEEE long-double (compiler dependent, 80-bit extended precision on x86 and ARM, 128-bit on RISC-V
Show the sign/scale/fraction components of an IEEE long double.
Usage: longdouble long_double_value
Example: longdouble 0.03124999
scientific   : 0.0312499899999999983247
triple form  : (+,-6,111111111111111111110101010000110011100010001110111000000000000)
binary form  : 0b0.011'1111'1111'1001.1111'1111'1111'1111'1111'1010'1010'0001'1001'1100'0100'0111'0111'0000'0000'0000
color coded  : 0b0.0011'1111'1111'1001.111'1111'1111'1111'1111'1010'1010'0001'1001'1100'0100'0111'0111'0000'0000'0000
Number Traits of IEEE-754 long double
std::numeric_limits< e >
min exponent                                         -16381
max exponent                                          16384
radix                                                     2
radix digits                                             64
min                                            3.3621e-4932
max                                           1.18973e+4932
lowest                                       -1.18973e+4932
epsilon (1+1ULP-1)                               1.0842e-19
round_error                                             0.5
smallest value                                 3.6452e-4951
infinity                                                inf
quiet_NAN                                               nan
signaling_NAN                                           nan

smallest normal number
0b0.000000000000001.1000000000000000000000000000000000000000000000000000000000000000
smallest denormalized number
0b0.000000000000000.0000000000000000000000000000000000000000000000000000000000000001

Universal parameterization of IEEE-754 fields
Total number of bits        : 80
number of exponent bits     : 15
number of fraction bits     : 63
exponent bias               : 16383
```

## propp

Show the arithmetic properties of a posit environment, including its quire.

```text
$ propp
Show the arithmetic properties of a posit.
Usage: propp [nbits es capacity]
Example: propp 16 1 8
arithmetic properties of a posit<16, 1> environment
 posit< 16, 1> useed scale     2     minpos scale - 28     maxpos scale         28
  minpos                     : 16.1x0001p + 3.72529e-09
  maxpos                     : 16.1x7fffp + 2.68435e+08
Properties of a quire<16, 1, 8>
  dynamic range of product   : 112
  radix point of accumulator :  56
  full  quire size in bits   : 120
  lower quire size in bits   :  56
  upper quire size in bits   :  57
  capacity bits              :   8
Quire segments
+ : 00000000_000000000000000000000000000000000000000000000000000000000.00000000000000000000000000000000000000000000000000000000
```


## plimits

Show the numeric_limits<> of the standard posits.

```text
$ plimits
plimits: numeric_limits<> of standard posits
Numeric limits for posit< 8, 0>
numeric_limits< sw::universal::posit<8, 0> >::min()             : 0.015625
numeric_limits< sw::universal::posit<8, 0> >::max()             : 64
numeric_limits< sw::universal::posit<8, 0> >::lowest()          : -64
numeric_limits< sw::universal::posit<8, 0> >::epsilon()         : 0.03125
numeric_limits< sw::universal::posit<8, 0> >::digits            : 6
numeric_limits< sw::universal::posit<8, 0> >::digits10          : 1
numeric_limits< sw::universal::posit<8, 0> >::max_digits10      : 2
numeric_limits< sw::universal::posit<8, 0> >::is_signed         : 1
numeric_limits< sw::universal::posit<8, 0> >::is_integer        : 0
numeric_limits< sw::universal::posit<8, 0> >::is_exact          : 0
numeric_limits< sw::universal::posit<8, 0> >::min_exponent      : -6
numeric_limits< sw::universal::posit<8, 0> >::min_exponent10    : -1
numeric_limits< sw::universal::posit<8, 0> >::max_exponent      : 6
numeric_limits< sw::universal::posit<8, 0> >::max_exponent10    : 1
numeric_limits< sw::universal::posit<8, 0> >::has_infinity      : 1
numeric_limits< sw::universal::posit<8, 0> >::has_quiet_NaN     : 1
numeric_limits< sw::universal::posit<8, 0> >::has_signaling_NaN : 1
numeric_limits< sw::universal::posit<8, 0> >::has_denorm        : 0
numeric_limits< sw::universal::posit<8, 0> >::has_denorm_loss   : 0
numeric_limits< sw::universal::posit<8, 0> >::is_iec559         : 0
numeric_limits< sw::universal::posit<8, 0> >::is_bounded        : 0
numeric_limits< sw::universal::posit<8, 0> >::is_modulo         : 0
numeric_limits< sw::universal::posit<8, 0> >::traps             : 0
numeric_limits< sw::universal::posit<8, 0> >::tinyness_before   : 0
numeric_limits< sw::universal::posit<8, 0> >::round_style       : 1
Numeric limits for posit< 16, 1>
numeric_limits< sw::universal::posit<16, 1> >::min()             : 3.72529e-09
numeric_limits< sw::universal::posit<16, 1> >::max()             : 2.68435e+08
numeric_limits< sw::universal::posit<16, 1> >::lowest()          : -2.68435e+08
numeric_limits< sw::universal::posit<16, 1> >::epsilon()         : 0.000244141
numeric_limits< sw::universal::posit<16, 1> >::digits            : 13
numeric_limits< sw::universal::posit<16, 1> >::digits10          : 3
numeric_limits< sw::universal::posit<16, 1> >::max_digits10      : 4
...
```

## propq

Show size tables of quires.

```text
$ propq
print quire size tables
Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
       4      18      26      42      74     138     266     522    1034    2058    4106
       5      22      34      58     106     202     394     778    1546    3082    6154
       6      26      42      74     138     266     522    1034    2058    4106    8202
       7      30      50      90     170     330     650    1290    2570    5130   10250
       8      34      58     106     202     394     778    1546    3082    6154   12298
       9      38      66     122     234     458     906    1802    3594    7178   14346
      10      42      74     138     266     522    1034    2058    4106    8202   16394
      11      46      82     154     298     586    1162    2314    4618    9226   18442
      12      50      90     170     330     650    1290    2570    5130   10250   20490
```
