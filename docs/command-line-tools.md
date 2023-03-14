# Universal number system Command Line Tools

The universal library contains a collection of command line tools that help investigate 
bit-level attributes of the number systems and the values they encode. These command line 
tools get installed with the `make install` build target.

The following segments presents short description of their use.

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

## quarter

Show the sign/scale/fraction components of an IEEE-754 quarter-precision floating-point.

```text
$ quarter
quarter : components of an IEEE-754 quarter-precision float : 8 bits with 2 exponent bits
Show the sign/scale/fraction components of a quarter-precision IEEE-754 floating-point.
Usage: quarter value
Example: quarter 0.03124999
scientific   : 0.031
triple form  : (+,  -5, 0b01.0'0000)
binary form  : 0b0.00.0'0001
color coded  : 00000001

Number Traits of quarter-precision IEEE-754 floating-point
std::numeric_limits< N2sw9universal6cfloatILj8ELj2EhLb1ELb0ELb0EEE >
min exponent                                             -2
max exponent                                              2
radix                                                     2
radix digits                                              6
min                                                       1
max                                                  3.9375
lowest                                              -3.9375
epsilon (1+1ULP-1)                                  0.03125
round_error                                             0.5
smallest value                                      0.03125
infinity                                                inf
quiet_NAN                                               nan
signaling_NAN                                           nan

smallest normal number
0b0.01.00000 : 1
smallest denormalized number
0b0.00.00001 : 0.03125
```

## half

Show the sign/scale/fraction components of an IEEE-754 half-precision floating-point.

```text
$ half
half : components of an IEEE-754 half-precision floating-point: 16 bits with 5 exponent bits
Show the sign/scale/fraction components of a half-precision IEEE-754 floating-point.
Usage: half value
Example: half 0.03124999
scientific   : 0.03125
triple form  : (+,  -5, 0b01.00'0000'0000)
binary form  : 0b0.0'1010.00'0000'0000
color coded  : 0010100000000000

Number Traits of half-precision IEEE-754 floating-point
std::numeric_limits< N2sw9universal6cfloatILj16ELj5EtLb1ELb0ELb0EEE >
min exponent                                            -16
max exponent                                             16
radix                                                     2
radix digits                                             11
min                                             6.10352e-05
max                                                   65504
lowest                                               -65504
epsilon (1+1ULP-1)                              0.000976562
round_error                                             0.5
smallest value                                  5.96046e-08
infinity                                                inf
quiet_NAN                                               nan
signaling_NAN                                           nan

smallest normal number
0b0.00001.0000000000 : 6.10352e-05
smallest denormalized number
0b0.00000.0000000001 : 5.96046e-08
```

## single

Show the sign/scale/fraction components of an IEEE-754 single-precision floating-point.

```text
$ single
single : components of an IEEE-754 single-precision floating_point: 32 bits with 8 exponent bits
Show the sign/scale/fraction components of a single-precision IEEE-754 floating-point.
Usage: single value
Example: single 0.03124999
scientific   : 0.0312499907
triple form  : (+,-6,0b11111111111111111111011)
binary form  : 0b0.0111'1001.111'1111'1111'1111'1111'1011
color coded  : 0b0.0111'1001.111'1111'1111'1111'1111'1011

Number Traits of IEEE-754 float
std::numeric_limits< f >
min exponent                                           -125
max exponent                                            128
radix                                                     2
radix digits                                             24
min                                             1.17549e-38
max                                             3.40282e+38
lowest                                         -3.40282e+38
epsilon (1+1ULP-1)                              1.19209e-07
round_error                                             0.5
smallest value                                   1.4013e-45
infinity                                                inf
quiet_NAN                                               nan
signaling_NAN                                           nan

smallest normal number
0b0.00000001.00000000000000000000000 : 1.17549e-38
smallest denormalized number
0b0.00000000.00000000000000000000001 : 1.4013e-45

Universal parameterization of IEEE-754 fields
Total number of bits        : 32
number of exponent bits     : 8
number of fraction bits     : 23
exponent bias               : 127
sign field mask             : 0b1000'0000'0000'0000'0000'0000'0000'0000
exponent field mask         : 0b0111'1111'1000'0000'0000'0000'0000'0000
mask of exponent value      : 0b1111'1111
mask of hidden bit          : 0b0000'0000'1000'0000'0000'0000'0000'0000
fraction field mask         : 0b0000'0000'0111'1111'1111'1111'1111'1111
significant field mask      : 0b0000'0000'1111'1111'1111'1111'1111'1111
MSB fraction bit mask       : 0b0000'0000'0100'0000'0000'0000'0000'0000
qNaN pattern                : 0b0111'1111'1100'0000'0000'0000'0000'0000
sNaN pattern                : 0b0111'1111'1010'0000'0000'0000'0000'0000
smallest normal value       : 1.17549e-38
                            : 0b0.00000001.00000000000000000000000
smallest subnormal value    : 1.4013e-45
                            : 0b0.00000000.00000000000000000000001
exponent smallest normal    : -126
exponent smallest subnormal : -149
```

## double

Show the sign/scale/fraction components of an IEEE-754 double-precision floating-point.

```text
$ double
double : components of an IEEE double-precision float
Show the sign/scale/fraction components of an IEEE double.
Usage: double double_value
Example: double 0.03124999
scientific   : 0.031249989999999998
triple form  : (+,-6,1111111111111111111101010100001100111000100011101110)
binary form  : 0b0.011'1111'1001.1111'1111'1111'1111'1111'0101'0100'0011'0011'1000'1000'1110'1110
color coded  : 0b0.011'1111'1001.1111'1111'1111'1111'1111'0101'0100'0011'0011'1000'1000'1110'1110
Number Traits of IEEE-754 double
std::numeric_limits< d >
min exponent                                          -1021
max exponent                                           1024
radix                                                     2
radix digits                                             53
min                                            2.22507e-308
max                                            1.79769e+308
lowest                                        -1.79769e+308
epsilon (1+1ULP-1)                              2.22045e-16
round_error                                             0.5
smallest value                                 4.94066e-324
infinity                                                inf
quiet_NAN                                               nan
signaling_NAN                                           nan

smallest normal number
0b0.00000000001.0000000000000000000000000000000000000000000000000000
smallest denormalized number
0b0.00000000000.0000000000000000000000000000000000000000000000000001

Universal parameterization of IEEE-754 fields
Total number of bits        : 64
number of exponent bits     : 11
number of fraction bits     : 52
exponent bias               : 1023
sign field mask             : 0b1000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
exponent field mask         : 0b0111'1111'1111'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
mask of exponent value      : 0b111'1111'1111
mask of hidden bit          : 0b0000'0000'0001'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
fraction field mask         : 0b0000'0000'0000'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111
significant field mask      : 0b0000'0000'0001'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111'1111
MSB fraction bit mask       : 0b0000'0000'0000'1000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
qNaN pattern                : 0b0111'1111'1111'1000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
sNaN pattern                : 0b0111'1111'1111'0100'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
smallest normal value       : 2.22507e-308
                            : 0b0.00000000001.0000000000000000000000000000000000000000000000000000
smallest subnormal value    : 4.94066e-324
                            : 0b0.00000000000.0000000000000000000000000000000000000000000000000001
exponent smallest normal    : -1022
exponent smallest subnormal : -1074
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
sign field mask             : 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
exponent field mask         : 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
mask of exponent value      : 0b111'1111'1111'1111
mask of hidden bit          : 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
fraction field mask         : 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
significant field mask      : 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
MSB fraction bit mask       : 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
qNaN pattern                : 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
sNaN pattern                : 0b0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
smallest normal value       : 3.3621e-4932
                            : 0b0.000000000000001.1000000000000000000000000000000000000000000000000000000000000000
smallest subnormal value    : 3.6452e-4951
                            : 0b0.000000000000000.0000000000000000000000000000000000000000000000000000000000000001
exponent smallest normal    : -16382
exponent smallest subnormal : -16445
```

## quad

Show the sign/scale/fraction components of an IEEE-754 quad-precision floating-point.

```text
$ quad
quad : components of an IEEE-754 quad-precision float : 128 bits total with 15 exponent bits
Show the sign/scale/fraction components of a quad-precision IEEE-754 floating-point.
Usage: quad value
Example: quad 0.03124999
scientific   : 0.03124999068677425384521484375
triple form  : (+,  -6, 0b01'1111'1111'1111'1111'1111'0110'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000)
binary form  : 0b0.011'1111'1111'1001.1111'1111'1111'1111'1111'0110'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000'0000
color coded  : 00111111111110011111111111111111111101100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000

Number Traits of quad-precision IEEE-754 floating-point
std::numeric_limits< N2sw9universal6cfloatILj128ELj15EjLb1ELb0ELb0EEE >
min exponent                                         -16384
max exponent                                          16384
radix                                                     2
radix digits                                            113
min                                                       0
max                                                     inf
lowest                                                 -inf
epsilon (1+1ULP-1)                              1.92593e-34
round_error                                             0.5
smallest value                                            0
infinity                                                inf
quiet_NAN                                               nan
signaling_NAN                                           nan

smallest normal number
0b0.000000000000001.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000 : 0
smallest denormalized number
0b0.000000000000000.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001 : 0
```

## fixpnt

Show the sign/scale/fraction components of a fixed-point value.

```text
$ fixpnt
fixpnt : components of a fixed-point value
Show the sign/scale/fraction components of a fixed-point value.
Usage: fixpnt float_value
Example: fixpnt 1.0625

                                                              class sw::universal::fixpnt < 8, 4, 1, unsigned char>
1.0625
b0001.0001
                                                              class sw::universal::fixpnt < 12, 4, 1, unsigned char>
1.0625
b0000'0001.0001
                                                              class sw::universal::fixpnt < 16, 8, 1, unsigned char>
1.06250000
b0000'0001.0001'0000
                                                              class sw::universal::fixpnt < 32, 16, 1, unsigned char>
1.0625000000000000
b0000'0000'0000'0001.0001'0000'0000'0000
                                                              class sw::universal::fixpnt < 64, 32, 1, unsigned char>
1.06250000000000000000000000000000
b0000'0000'0000'0000'0000'0000'0000'0001.0001'0000'0000'0000'0000'0000'0000'0000
```

## signedint

Show the sign/scale/fraction components of a signed integer.

```text
$ signedint
signedint : components of a signed integer
Show the sign/scale/fraction components of a signed integer.
Usage: signedint value
Example: signedint 1234567890123456789012345
class sw::universal::integer<128,unsigned int>         : 1234567890123456789012345 (+,80,00000101011011100000111100110110101001100100010000111101111000101101111101111001)
```

## unsignedint

Show the sign/scale/fraction components of an unsigned integer.

```text
$ unsignedint

```

## posit

Show the sign/scale/regime/exponent/fraction components of a posit.

```text
$ posit
posit : posit components
Show the sign/scale/regime/exponent/fraction components of a posit.
Usage: posit value

Example: posit 3.1415926535897932384626433832795028841971
posit< 8,0>  = 01101001 : 3.125
posit< 8, 1> = 01011001 : 3.125
posit< 8, 2> = 01001101 : 3.25
posit< 8, 3> = 01000110 : 3
posit<16, 1> = 0101100100100010 : 3.1416
posit<16, 2> = 0100110010010001 : 3.1416
posit<16, 3> = 0100011001001000 : 3.1406
posit<24, 1> = 010110010010000111111011 : 3.141592
posit<24, 2> = 010011001001000011111110 : 3.141594
posit<24, 3> = 010001100100100001111111 : 3.141594
posit<32, 1> = 01011001001000011111101101010100 : 3.14159265
posit<32, 2> = 01001100100100001111110110101010 : 3.14159265
posit<32, 3> = 01000110010010000111111011010101 : 3.14159265
posit<48, 1> = 010110010010000111111011010101000100010000101101 : 3.1415926535898
posit<48, 2> = 010011001001000011111101101010100010001000010111 : 3.1415926535899
posit<48, 3> = 010001100100100001111110110101010001000100001011 : 3.1415926535897
posit<64, 1> = 0101100100100001111110110101010001000100001011010001100000000000 : 3.14159265358979312
posit<64, 2> = 0100110010010000111111011010101000100010000101101000110000000000 : 3.14159265358979312
posit<64, 3> = 0100011001001000011111101101010100010001000010110100011000000000 : 3.14159265358979312
posit<64, 4> = 0100001100100100001111110110101010001000100001011010001100000000 : 3.14159265358979312

```

## float2posit

Show the conversion of a float to a posit step-by-step.

```text
$ float2posit
Show the conversion of a float to a posit step-by-step.
Usage: float2posit floating_point_value posit_size_in_bits[one of 8|16|32|48|64|80|96|128|256]
Example: convert -1.123456789e17 32

$ float2posit 1.234567890 32
1.23456789   input value
Test for ZERO
(+, 0, 0011110000001100101001000010100000111101111000011011) input value is NOT zero
Test for NaR
(+, 0, 0011110000001100101001000010100000111101111000011011) input value is NOT NaR
construct the posit
0011'1100'0000'1100'1010'0100'0010'1000'0011'1101'1110'0001'1011  full fraction bits
0000'0000'0000'0000'0000'0000'0000'0111'1111'1111'1111'1111'1111  mask of remainder bits
0'0000'0000'0000'0000'0000'0000'0000'0000'0000  unconstrained posit : length = nbits(32) + es(2) + 3 guard bits : 37
0'0001'0000'0000'0000'0000'0000'0000'0000'0000  runlength = 1
0'0000'0000'0000'0000'0000'0000'0000'0000'0000  exponent value = 0
0'0000'0000'0111'1000'0001'1001'0100'1000'0100  most significant 28 fraction bits(nbits - 1 - run - es)
0'0000'0000'0000'0000'0000'0000'0000'0000'0001  sticky bit representing the truncated fraction bits
0'0001'0000'0111'1000'0001'1001'0100'1000'0101  unconstrained posit bits  length = 34
0'0000'0000'0000'0000'0000'0000'0000'0000'0100  last bit mask
0'0000'0000'0000'0000'0000'0000'0000'0000'0010  bit after last bit mask
0'0000'0000'0000'0000'0000'0000'0000'0000'0001  sticky bit mask
rounding decision(blast & bafter) | (bafter & bsticky) : round down
0'1000'0011'1100'0000'1100'1010'0100'0010'1000  shifted posit
0100'0001'1110'0000'0110'0101'0010'0001  truncated posit
0100'0001'1110'0000'0110'0101'0010'0001  rounded posit
0100'0001'1110'0000'0110'0101'0010'0001  final posit

```

## propenv

Show the storage properties of the native, standard, and extended posits.

```text
$ propenv
Bit sizes for native types
unsigned char           8 bits
unsigned short         16 bits
unsigned int           32 bits
unsigned long long     64 bits
  signed char           7 bits
  signed short         15 bits
  signed int           31 bits
  signed long long     63 bits
         float         24 bits
         double        53 bits
         long double   53 bits

Min-Max range for floats and posit<32,2> comparison
                         float min   1.17549e-38     max   3.40282e+38
   class sw::unum::posit<32,2> min   7.52316e-37     max   1.32923e+36

Bit sizes for standard posit configurations
posit<8,0>              8 bits
posit<16,1>            16 bits
posit<32,2>            32 bits
posit<64,3>            64 bits
posit<128,4>          128 bits
posit<256,5>          256 bits


Bit sizes for extended posit configurations
posit<4,0>              8 bits
posit<8,0>              8 bits
posit<16,1>            16 bits
posit<20,1>            32 bits
posit<24,1>            32 bits
posit<28,1>            32 bits
posit<32,2>            32 bits
posit<40,2>            64 bits
posit<48,2>            64 bits
posit<56,2>            64 bits
posit<64,3>            64 bits
posit<80,3>           128 bits
posit<96,3>           128 bits
posit<112,3>          128 bits
posit<128,4>          128 bits
posit<256,5>          256 bits

Long double properties
value    1.2345678901234567
hex      00 00 00 00 00 00 00 04 3f f3 c0 ca 42 8c 59 fb
sign     +
scale    1
fraction 1056399862553083
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
numeric_limits< sw::universal::posit<16, 1> >::is_signed         : 1
numeric_limits< sw::universal::posit<16, 1> >::is_integer        : 0
numeric_limits< sw::universal::posit<16, 1> >::is_exact          : 0
numeric_limits< sw::universal::posit<16, 1> >::min_exponent      : -28
numeric_limits< sw::universal::posit<16, 1> >::min_exponent10    : -8
numeric_limits< sw::universal::posit<16, 1> >::max_exponent      : 28
numeric_limits< sw::universal::posit<16, 1> >::max_exponent10    : 8
numeric_limits< sw::universal::posit<16, 1> >::has_infinity      : 1
numeric_limits< sw::universal::posit<16, 1> >::has_quiet_NaN     : 1
numeric_limits< sw::universal::posit<16, 1> >::has_signaling_NaN : 1
numeric_limits< sw::universal::posit<16, 1> >::has_denorm        : 0
numeric_limits< sw::universal::posit<16, 1> >::has_denorm_loss   : 0
numeric_limits< sw::universal::posit<16, 1> >::is_iec559         : 0
numeric_limits< sw::universal::posit<16, 1> >::is_bounded        : 0
numeric_limits< sw::universal::posit<16, 1> >::is_modulo         : 0
numeric_limits< sw::universal::posit<16, 1> >::traps             : 0
numeric_limits< sw::universal::posit<16, 1> >::tinyness_before   : 0
numeric_limits< sw::universal::posit<16, 1> >::round_style       : 1
Numeric limits for posit< 32, 2>
numeric_limits< sw::universal::posit<32, 2> >::min()             : 7.52316e-37
numeric_limits< sw::universal::posit<32, 2> >::max()             : 1.32923e+36
numeric_limits< sw::universal::posit<32, 2> >::lowest()          : -1.32923e+36
numeric_limits< sw::universal::posit<32, 2> >::epsilon()         : 7.45058e-09
numeric_limits< sw::universal::posit<32, 2> >::digits            : 28
numeric_limits< sw::universal::posit<32, 2> >::digits10          : 8
numeric_limits< sw::universal::posit<32, 2> >::max_digits10      : 9
numeric_limits< sw::universal::posit<32, 2> >::is_signed         : 1
numeric_limits< sw::universal::posit<32, 2> >::is_integer        : 0
numeric_limits< sw::universal::posit<32, 2> >::is_exact          : 0
numeric_limits< sw::universal::posit<32, 2> >::min_exponent      : -120
numeric_limits< sw::universal::posit<32, 2> >::min_exponent10    : -36
numeric_limits< sw::universal::posit<32, 2> >::max_exponent      : 120
numeric_limits< sw::universal::posit<32, 2> >::max_exponent10    : 36
numeric_limits< sw::universal::posit<32, 2> >::has_infinity      : 1
numeric_limits< sw::universal::posit<32, 2> >::has_quiet_NaN     : 1
numeric_limits< sw::universal::posit<32, 2> >::has_signaling_NaN : 1
numeric_limits< sw::universal::posit<32, 2> >::has_denorm        : 0
numeric_limits< sw::universal::posit<32, 2> >::has_denorm_loss   : 0
numeric_limits< sw::universal::posit<32, 2> >::is_iec559         : 0
numeric_limits< sw::universal::posit<32, 2> >::is_bounded        : 0
numeric_limits< sw::universal::posit<32, 2> >::is_modulo         : 0
numeric_limits< sw::universal::posit<32, 2> >::traps             : 0
numeric_limits< sw::universal::posit<32, 2> >::tinyness_before   : 0
numeric_limits< sw::universal::posit<32, 2> >::round_style       : 1
Numeric limits for posit< 64, 3>
numeric_limits< sw::universal::posit<64, 3> >::min()             : 4.8879e-150
numeric_limits< sw::universal::posit<64, 3> >::max()             : 2.04587e+149
numeric_limits< sw::universal::posit<64, 3> >::lowest()          : -2.04587e+149
numeric_limits< sw::universal::posit<64, 3> >::epsilon()         : 3.46945e-18
numeric_limits< sw::universal::posit<64, 3> >::digits            : 59
numeric_limits< sw::universal::posit<64, 3> >::digits10          : 17
numeric_limits< sw::universal::posit<64, 3> >::max_digits10      : 18
numeric_limits< sw::universal::posit<64, 3> >::is_signed         : 1
numeric_limits< sw::universal::posit<64, 3> >::is_integer        : 0
numeric_limits< sw::universal::posit<64, 3> >::is_exact          : 0
numeric_limits< sw::universal::posit<64, 3> >::min_exponent      : -496
numeric_limits< sw::universal::posit<64, 3> >::min_exponent10    : -150
numeric_limits< sw::universal::posit<64, 3> >::max_exponent      : 496
numeric_limits< sw::universal::posit<64, 3> >::max_exponent10    : 150
numeric_limits< sw::universal::posit<64, 3> >::has_infinity      : 1
numeric_limits< sw::universal::posit<64, 3> >::has_quiet_NaN     : 1
numeric_limits< sw::universal::posit<64, 3> >::has_signaling_NaN : 1
numeric_limits< sw::universal::posit<64, 3> >::has_denorm        : 0
numeric_limits< sw::universal::posit<64, 3> >::has_denorm_loss   : 0
numeric_limits< sw::universal::posit<64, 3> >::is_iec559         : 0
numeric_limits< sw::universal::posit<64, 3> >::is_bounded        : 0
numeric_limits< sw::universal::posit<64, 3> >::is_modulo         : 0
numeric_limits< sw::universal::posit<64, 3> >::traps             : 0
numeric_limits< sw::universal::posit<64, 3> >::tinyness_before   : 0
numeric_limits< sw::universal::posit<64, 3> >::round_style       : 1
>>>>>>>>>>>>>>>>>> posit<128,4> does not render correctly due to limits of native floating point types
Numeric limits for posit< 128, 4>
numeric_limits< sw::universal::posit<128, 4> >::min()             : 1.32901e-607
numeric_limits< sw::universal::posit<128, 4> >::max()             : 7.52439e+606
numeric_limits< sw::universal::posit<128, 4> >::lowest()          : -7.52439e+606
numeric_limits< sw::universal::posit<128, 4> >::epsilon()         : 3.76158e-37
numeric_limits< sw::universal::posit<128, 4> >::digits            : 122
numeric_limits< sw::universal::posit<128, 4> >::digits10          : 36
numeric_limits< sw::universal::posit<128, 4> >::max_digits10      : 37
numeric_limits< sw::universal::posit<128, 4> >::is_signed         : 1
numeric_limits< sw::universal::posit<128, 4> >::is_integer        : 0
numeric_limits< sw::universal::posit<128, 4> >::is_exact          : 0
numeric_limits< sw::universal::posit<128, 4> >::min_exponent      : -2016
numeric_limits< sw::universal::posit<128, 4> >::min_exponent10    : -610
numeric_limits< sw::universal::posit<128, 4> >::max_exponent      : 2016
numeric_limits< sw::universal::posit<128, 4> >::max_exponent10    : 610
numeric_limits< sw::universal::posit<128, 4> >::has_infinity      : 1
numeric_limits< sw::universal::posit<128, 4> >::has_quiet_NaN     : 1
numeric_limits< sw::universal::posit<128, 4> >::has_signaling_NaN : 1
numeric_limits< sw::universal::posit<128, 4> >::has_denorm        : 0
numeric_limits< sw::universal::posit<128, 4> >::has_denorm_loss   : 0
numeric_limits< sw::universal::posit<128, 4> >::is_iec559         : 0
numeric_limits< sw::universal::posit<128, 4> >::is_bounded        : 0
numeric_limits< sw::universal::posit<128, 4> >::is_modulo         : 0
numeric_limits< sw::universal::posit<128, 4> >::traps             : 0
numeric_limits< sw::universal::posit<128, 4> >::tinyness_before   : 0
numeric_limits< sw::universal::posit<128, 4> >::round_style       : 1
>>>>>>>>>>>>>>>>>> posit<256,5> does not render correctly due to limits of native floating point types
Numeric limits for posit< 256, 5>
numeric_limits< sw::universal::posit<256, 5> >::min()             : 1.6912e-2447
numeric_limits< sw::universal::posit<256, 5> >::max()             : 5.91296e+2446
numeric_limits< sw::universal::posit<256, 5> >::lowest()          : -5.91296e+2446
numeric_limits< sw::universal::posit<256, 5> >::epsilon()         : 2.21086e-75
numeric_limits< sw::universal::posit<256, 5> >::digits            : 249
numeric_limits< sw::universal::posit<256, 5> >::digits10          : 75
numeric_limits< sw::universal::posit<256, 5> >::max_digits10      : 76
numeric_limits< sw::universal::posit<256, 5> >::is_signed         : 1
numeric_limits< sw::universal::posit<256, 5> >::is_integer        : 0
numeric_limits< sw::universal::posit<256, 5> >::is_exact          : 0
numeric_limits< sw::universal::posit<256, 5> >::min_exponent      : -8128
numeric_limits< sw::universal::posit<256, 5> >::min_exponent10    : -2463
numeric_limits< sw::universal::posit<256, 5> >::max_exponent      : 8128
numeric_limits< sw::universal::posit<256, 5> >::max_exponent10    : 2463
numeric_limits< sw::universal::posit<256, 5> >::has_infinity      : 1
numeric_limits< sw::universal::posit<256, 5> >::has_quiet_NaN     : 1
numeric_limits< sw::universal::posit<256, 5> >::has_signaling_NaN : 1
numeric_limits< sw::universal::posit<256, 5> >::has_denorm        : 0
numeric_limits< sw::universal::posit<256, 5> >::has_denorm_loss   : 0
numeric_limits< sw::universal::posit<256, 5> >::is_iec559         : 0
numeric_limits< sw::universal::posit<256, 5> >::is_bounded        : 0
numeric_limits< sw::universal::posit<256, 5> >::is_modulo         : 0
numeric_limits< sw::universal::posit<256, 5> >::traps             : 0
numeric_limits< sw::universal::posit<256, 5> >::tinyness_before   : 0
numeric_limits< sw::universal::posit<256, 5> >::round_style       : 1
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

Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
       8      34      58     106     202     394     778    1546    3082    6154   12298
       9      38      66     122     234     458     906    1802    3594    7178   14346
      10      42      74     138     266     522    1034    2058    4106    8202   16394
      11      46      82     154     298     586    1162    2314    4618    9226   18442
      12      50      90     170     330     650    1290    2570    5130   10250   20490
      13      54      98     186     362     714    1418    2826    5642   11274   22538
      14      58     106     202     394     778    1546    3082    6154   12298   24586
      15      62     114     218     426     842    1674    3338    6666   13322   26634
      16      66     122     234     458     906    1802    3594    7178   14346   28682

Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
      16      66     122     234     458     906    1802    3594    7178   14346   28682
      17      70     130     250     490     970    1930    3850    7690   15370   30730
      18      74     138     266     522    1034    2058    4106    8202   16394   32778
      19      78     146     282     554    1098    2186    4362    8714   17418   34826
      20      82     154     298     586    1162    2314    4618    9226   18442   36874
      21      86     162     314     618    1226    2442    4874    9738   19466   38922
      22      90     170     330     650    1290    2570    5130   10250   20490   40970
      23      94     178     346     682    1354    2698    5386   10762   21514   43018
      24      98     186     362     714    1418    2826    5642   11274   22538   45066

Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
      24      98     186     362     714    1418    2826    5642   11274   22538   45066
      25     102     194     378     746    1482    2954    5898   11786   23562   47114
      26     106     202     394     778    1546    3082    6154   12298   24586   49162
      27     110     210     410     810    1610    3210    6410   12810   25610   51210
      28     114     218     426     842    1674    3338    6666   13322   26634   53258
      29     118     226     442     874    1738    3466    6922   13834   27658   55306
      30     122     234     458     906    1802    3594    7178   14346   28682   57354
      31     126     242     474     938    1866    3722    7434   14858   29706   59402
      32     130     250     490     970    1930    3850    7690   15370   30730   61450

Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
      32     130     250     490     970    1930    3850    7690   15370   30730   61450
      33     134     258     506    1002    1994    3978    7946   15882   31754   63498
      34     138     266     522    1034    2058    4106    8202   16394   32778   65546
      35     142     274     538    1066    2122    4234    8458   16906   33802   67594
      36     146     282     554    1098    2186    4362    8714   17418   34826   69642
      37     150     290     570    1130    2250    4490    8970   17930   35850   71690
      38     154     298     586    1162    2314    4618    9226   18442   36874   73738
      39     158     306     602    1194    2378    4746    9482   18954   37898   75786
      40     162     314     618    1226    2442    4874    9738   19466   38922   77834

Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
      40     162     314     618    1226    2442    4874    9738   19466   38922   77834
      41     166     322     634    1258    2506    5002    9994   19978   39946   79882
      42     170     330     650    1290    2570    5130   10250   20490   40970   81930
      43     174     338     666    1322    2634    5258   10506   21002   41994   83978
      44     178     346     682    1354    2698    5386   10762   21514   43018   86026
      45     182     354     698    1386    2762    5514   11018   22026   44042   88074
      46     186     362     714    1418    2826    5642   11274   22538   45066   90122
      47     190     370     730    1450    2890    5770   11530   23050   46090   92170
      48     194     378     746    1482    2954    5898   11786   23562   47114   94218

Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
      48     194     378     746    1482    2954    5898   11786   23562   47114   94218
      49     198     386     762    1514    3018    6026   12042   24074   48138   96266
      50     202     394     778    1546    3082    6154   12298   24586   49162   98314
      51     206     402     794    1578    3146    6282   12554   25098   50186  100362
      52     210     410     810    1610    3210    6410   12810   25610   51210  102410
      53     214     418     826    1642    3274    6538   13066   26122   52234  104458
      54     218     426     842    1674    3338    6666   13322   26634   53258  106506
      55     222     434     858    1706    3402    6794   13578   27146   54282  108554
      56     226     442     874    1738    3466    6922   13834   27658   55306  110602

Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
      56     226     442     874    1738    3466    6922   13834   27658   55306  110602
      57     230     450     890    1770    3530    7050   14090   28170   56330  112650
      58     234     458     906    1802    3594    7178   14346   28682   57354  114698
      59     238     466     922    1834    3658    7306   14602   29194   58378  116746
      60     242     474     938    1866    3722    7434   14858   29706   59402  118794
      61     246     482     954    1898    3786    7562   15114   30218   60426  120842
      62     250     490     970    1930    3850    7690   15370   30730   61450  122890
      63     254     498     986    1962    3914    7818   15626   31242   62474  124938
      64     258     506    1002    1994    3978    7946   15882   31754   63498  126986

Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
      64     258     506    1002    1994    3978    7946   15882   31754   63498  126986
      65     262     514    1018    2026    4042    8074   16138   32266   64522  129034
      66     266     522    1034    2058    4106    8202   16394   32778   65546  131082
      67     270     530    1050    2090    4170    8330   16650   33290   66570  133130
      68     274     538    1066    2122    4234    8458   16906   33802   67594  135178
      69     278     546    1082    2154    4298    8586   17162   34314   68618  137226
      70     282     554    1098    2186    4362    8714   17418   34826   69642  139274
      71     286     562    1114    2218    4426    8842   17674   35338   70666  141322
      72     290     570    1130    2250    4490    8970   17930   35850   71690  143370

Quire size table as a function of <nbits, es, capacity = 10>
Capacity is 2^10 accumulations of max_pos^2
   nbits                               es value
       +       0       1       2       3       4       5       6       7       8       9
      80     322     634    1258    2506    5002    9994   19978   39946   79882  159754
      81     326     642    1274    2538    5066   10122   20234   40458   80906  161802
      82     330     650    1290    2570    5130   10250   20490   40970   81930  163850
      83     334     658    1306    2602    5194   10378   20746   41482   82954  165898
      84     338     666    1322    2634    5258   10506   21002   41994   83978  167946
      85     342     674    1338    2666    5322   10634   21258   42506   85002  169994
      86     346     682    1354    2698    5386   10762   21514   43018   86026  172042
      87     350     690    1370    2730    5450   10890   21770   43530   87050  174090
      88     354     698    1386    2762    5514   11018   22026   44042   88074  176138
```
