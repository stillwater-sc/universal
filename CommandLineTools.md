# Universal number system Command Line Tools

The universal library contains a collection of command line tools that help investigate bit-level attributes of the number systems and the values they encode. These command line tools get installed with the `make install` build target.

The following segments presents short description of their use.

## compieee

Compare the three IEEE formats on a given real number value:

```text
λ ./compieee.exe
Show the truncated value and (sign/scale/fraction) components of different floating point types.
Usage: compieee floating_point_value
Example: compieee 0.03124999
input value:                0.03124999
      float:              0.0312499907 (+,-6,11111111111111111111011)
     double:      0.031249989999999998 (+,-6,1111111111111111111101010100001100111000100011101110)
long double:  0.0312499899999999983247 (+,-6,111111111111111111101001011110100011111111111110001111111001111)
```

## compf

Show the sign/scale/fraction components of an IEEE float.

```text
λ ./compf.exe
compf : components of an IEEE single-precision float
Show the sign/scale/fraction components of an IEEE float.
Usage: compf float_value
Example: compf 0.03124999
float: 0.031249990686774254 (+,-6,11111111111111111111011)
```

## compd

Show the sign/scale/fraction components of an IEEE double.

```text
λ ./compd.exe
compd : components of an IEEE double-precision float
Show the sign/scale/fraction components of an IEEE double.
Usage: compf double_value
Example: compd 0.03124999
double: 0.031249989999999998 (+,-6,1111111111111111111101010100001100111000100011101110)
```

## compld

Show the sign/scale/fraction components of an IEEE long double. On Windows using the Microsoft Visual Studio environment, the `long double` is aliased to `double`.

```text
λ ./compld.exe
compld: components of an IEEE long-double (compiler dependent, 80-bit extended precision on x86 and ARM, 128-bit on RISC-V
Show the sign/scale/fraction components of an IEEE long double.
Usage: compld long_double_value
Example: compld 0.03124999
long double: 0.0312499899999999983247 (+,-6,000000000000000000000000000000000011111111111110000000000000000)
```

## compfp

Show the sign/scale/fraction components of a fixed-point value.

```text
λ ./compfp.exe
compfp : components of a fixed-point value
Show the sign/scale/fraction components of a fixed-point value.
Usage: compfp float_value
Example: compfp 1.0625
class sw::unum::fixpnt<32,16,1,unsigned char>: 1.0625000000000000 b0000000000000001.0001000000000000
```

## propp

Show the arithmetic properties of a posit environment, including its quire.

```text
λ ./propp.exe
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

## compsi

Show the sign/scale/fraction components of a signed integer.

```text
λ ./compsi.exe
compsi : components of a signed integer
Show the sign/scale/fraction components of a signed integer.
Usage: compsi integer_value
Example: compsi 1234567890123456789012345
class sw::unum::integer<128,unsigned int>         : 1234567890123456789012345 (+,80,00000101011011100000111100110110101001100100010000111101111000101101111101111001)
```

## compui

Show the sign/scale/fraction components of an unsigned integer.

```text
λ ./compui.exe
compui : components of an unsigned integer
Show the sign/scale/fraction components of an unsigned integer.
Usage: compui integer_value
Example: compui 123456789012345670
TBD:
```

## compp

Show the sign/scale/regime/exponent/fraction components of a posit.

```text
λ ./compp.exe
pc : posit components
Show the sign/scale/regime/exponent/fraction components of a posit.
Usage: pc float_value
Example: pc -1.123456789e17
posit< 8, 0> = s1 r1111111 e f qNW v-64
posit< 8, 1> = s1 r1111111 e f qNW v-4096
posit< 8, 2> = s1 r1111111 e f qNW v-16777216
posit< 8, 3> = s1 r1111111 e f qNW v-281474976710656
posit<16, 1> = s1 r111111111111111 e f qNW v-268435456
posit<16, 2> = s1 r111111111111111 e f qNW v-72057594037927936
posit<16, 3> = s1 r111111110 e000 f100 qNW v-1.080863910568919e+17
posit<32, 1> = s1 r111111111111111111111111111110 e1 f qNW v-1.4411518807585587e+17
posit<32, 2> = s1 r1111111111111110 e00 f1000111100100 qNW v-1.1234370007964058e+17
posit<32, 3> = s1 r111111110 e000 f1000111100100001110 qNW v-1.1234562422498918e+17
posit<48, 1> = s1 r111111111111111111111111111110 e0 f1000111100100010 qNW v-1.1234589910289613e+17
posit<48, 2> = s1 r1111111111111110 e00 f10001111001000011100110010111 qNW v-1.1234567885160448e+17
posit<48, 3> = s1 r111111110 e000 f10001111001000011100110010111010111 qNW v-1.1234567889983898e+17
posit<64, 1> = s1 r111111111111111111111111111110 e0 f10001111001000011100110010111011 qNW v-1.1234567890193613e+17
posit<64, 2> = s1 r1111111111111110 e00 f100011110010000111001100101110101110001001111 qNW v-1.1234567890000077e+17
posit<64, 3> = s1 r111111110 e000 f100011110010000111001100101110101110001001110101000 qNW v-1.123456789e+17
posit<64, 4> = s1 r11110 e1000 f100011110010000111001100101110101110001001110101000000 qNW v-1.123456789e+17
```

## float2posit

Show the conversion of a float to a posit step-by-step.

```text
λ ./float2posit.exe
Show the conversion of a float to a posit step-by-step.
Usage: float2posit floating_point_value posit_size_in_bits[one of 8|16|32|48|64|80|96|128|256]
Example: convert -1.123456789e17 32
$ ./float2posit.exe 1.234567890 32
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
λ ./propenv.exe
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

## plimits

Show the numeric_limits<> of the standard posits.

```text
λ ./plimits.exe
C:\Users\tomtz\Documents\dev\clones\universal\build\tools\cmd\Release\plimits.exe: numeric_limits<> of standard posits
Numeric limits for posit< 8, 0>
numeric_limits< sw::unum::posit<8, 0> >::min()             : 0.015625
numeric_limits< sw::unum::posit<8, 0> >::max()             : 64
numeric_limits< sw::unum::posit<8, 0> >::lowest()          : -64
numeric_limits< sw::unum::posit<8, 0> >::epsilon()         : 0.03125
numeric_limits< sw::unum::posit<8, 0> >::digits            : 6
numeric_limits< sw::unum::posit<8, 0> >::digits10          : 1
numeric_limits< sw::unum::posit<8, 0> >::max_digits10      : 2
numeric_limits< sw::unum::posit<8, 0> >::is_signed         : 1
numeric_limits< sw::unum::posit<8, 0> >::is_integer        : 0
numeric_limits< sw::unum::posit<8, 0> >::is_exact          : 0
numeric_limits< sw::unum::posit<8, 0> >::min_exponent      : -6
numeric_limits< sw::unum::posit<8, 0> >::min_exponent10    : -1
numeric_limits< sw::unum::posit<8, 0> >::max_exponent      : 6
numeric_limits< sw::unum::posit<8, 0> >::max_exponent10    : 1
numeric_limits< sw::unum::posit<8, 0> >::has_infinity      : 1
numeric_limits< sw::unum::posit<8, 0> >::has_quiet_NaN     : 1
numeric_limits< sw::unum::posit<8, 0> >::has_signaling_NaN : 1
numeric_limits< sw::unum::posit<8, 0> >::has_denorm        : 0
numeric_limits< sw::unum::posit<8, 0> >::has_denorm_loss   : 0
numeric_limits< sw::unum::posit<8, 0> >::is_iec559         : 0
numeric_limits< sw::unum::posit<8, 0> >::is_bounded        : 0
numeric_limits< sw::unum::posit<8, 0> >::is_modulo         : 0
numeric_limits< sw::unum::posit<8, 0> >::traps             : 0
numeric_limits< sw::unum::posit<8, 0> >::tinyness_before   : 0
numeric_limits< sw::unum::posit<8, 0> >::round_style       : 1
Numeric limits for posit< 16, 1>
numeric_limits< sw::unum::posit<16, 1> >::min()             : 3.72529e-09
numeric_limits< sw::unum::posit<16, 1> >::max()             : 2.68435e+08
numeric_limits< sw::unum::posit<16, 1> >::lowest()          : -2.68435e+08
numeric_limits< sw::unum::posit<16, 1> >::epsilon()         : 0.000244141
numeric_limits< sw::unum::posit<16, 1> >::digits            : 13
numeric_limits< sw::unum::posit<16, 1> >::digits10          : 3
numeric_limits< sw::unum::posit<16, 1> >::max_digits10      : 4
numeric_limits< sw::unum::posit<16, 1> >::is_signed         : 1
numeric_limits< sw::unum::posit<16, 1> >::is_integer        : 0
numeric_limits< sw::unum::posit<16, 1> >::is_exact          : 0
numeric_limits< sw::unum::posit<16, 1> >::min_exponent      : -28
numeric_limits< sw::unum::posit<16, 1> >::min_exponent10    : -8
numeric_limits< sw::unum::posit<16, 1> >::max_exponent      : 28
numeric_limits< sw::unum::posit<16, 1> >::max_exponent10    : 8
numeric_limits< sw::unum::posit<16, 1> >::has_infinity      : 1
numeric_limits< sw::unum::posit<16, 1> >::has_quiet_NaN     : 1
numeric_limits< sw::unum::posit<16, 1> >::has_signaling_NaN : 1
numeric_limits< sw::unum::posit<16, 1> >::has_denorm        : 0
numeric_limits< sw::unum::posit<16, 1> >::has_denorm_loss   : 0
numeric_limits< sw::unum::posit<16, 1> >::is_iec559         : 0
numeric_limits< sw::unum::posit<16, 1> >::is_bounded        : 0
numeric_limits< sw::unum::posit<16, 1> >::is_modulo         : 0
numeric_limits< sw::unum::posit<16, 1> >::traps             : 0
numeric_limits< sw::unum::posit<16, 1> >::tinyness_before   : 0
numeric_limits< sw::unum::posit<16, 1> >::round_style       : 1
Numeric limits for posit< 32, 2>
numeric_limits< sw::unum::posit<32, 2> >::min()             : 7.52316e-37
numeric_limits< sw::unum::posit<32, 2> >::max()             : 1.32923e+36
numeric_limits< sw::unum::posit<32, 2> >::lowest()          : -1.32923e+36
numeric_limits< sw::unum::posit<32, 2> >::epsilon()         : 7.45058e-09
numeric_limits< sw::unum::posit<32, 2> >::digits            : 28
numeric_limits< sw::unum::posit<32, 2> >::digits10          : 8
numeric_limits< sw::unum::posit<32, 2> >::max_digits10      : 9
numeric_limits< sw::unum::posit<32, 2> >::is_signed         : 1
numeric_limits< sw::unum::posit<32, 2> >::is_integer        : 0
numeric_limits< sw::unum::posit<32, 2> >::is_exact          : 0
numeric_limits< sw::unum::posit<32, 2> >::min_exponent      : -120
numeric_limits< sw::unum::posit<32, 2> >::min_exponent10    : -36
numeric_limits< sw::unum::posit<32, 2> >::max_exponent      : 120
numeric_limits< sw::unum::posit<32, 2> >::max_exponent10    : 36
numeric_limits< sw::unum::posit<32, 2> >::has_infinity      : 1
numeric_limits< sw::unum::posit<32, 2> >::has_quiet_NaN     : 1
numeric_limits< sw::unum::posit<32, 2> >::has_signaling_NaN : 1
numeric_limits< sw::unum::posit<32, 2> >::has_denorm        : 0
numeric_limits< sw::unum::posit<32, 2> >::has_denorm_loss   : 0
numeric_limits< sw::unum::posit<32, 2> >::is_iec559         : 0
numeric_limits< sw::unum::posit<32, 2> >::is_bounded        : 0
numeric_limits< sw::unum::posit<32, 2> >::is_modulo         : 0
numeric_limits< sw::unum::posit<32, 2> >::traps             : 0
numeric_limits< sw::unum::posit<32, 2> >::tinyness_before   : 0
numeric_limits< sw::unum::posit<32, 2> >::round_style       : 1
Numeric limits for posit< 64, 3>
numeric_limits< sw::unum::posit<64, 3> >::min()             : 4.8879e-150
numeric_limits< sw::unum::posit<64, 3> >::max()             : 2.04587e+149
numeric_limits< sw::unum::posit<64, 3> >::lowest()          : -2.04587e+149
numeric_limits< sw::unum::posit<64, 3> >::epsilon()         : 3.46945e-18
numeric_limits< sw::unum::posit<64, 3> >::digits            : 59
numeric_limits< sw::unum::posit<64, 3> >::digits10          : 17
numeric_limits< sw::unum::posit<64, 3> >::max_digits10      : 18
numeric_limits< sw::unum::posit<64, 3> >::is_signed         : 1
numeric_limits< sw::unum::posit<64, 3> >::is_integer        : 0
numeric_limits< sw::unum::posit<64, 3> >::is_exact          : 0
numeric_limits< sw::unum::posit<64, 3> >::min_exponent      : -496
numeric_limits< sw::unum::posit<64, 3> >::min_exponent10    : -150
numeric_limits< sw::unum::posit<64, 3> >::max_exponent      : 496
numeric_limits< sw::unum::posit<64, 3> >::max_exponent10    : 150
numeric_limits< sw::unum::posit<64, 3> >::has_infinity      : 1
numeric_limits< sw::unum::posit<64, 3> >::has_quiet_NaN     : 1
numeric_limits< sw::unum::posit<64, 3> >::has_signaling_NaN : 1
numeric_limits< sw::unum::posit<64, 3> >::has_denorm        : 0
numeric_limits< sw::unum::posit<64, 3> >::has_denorm_loss   : 0
numeric_limits< sw::unum::posit<64, 3> >::is_iec559         : 0
numeric_limits< sw::unum::posit<64, 3> >::is_bounded        : 0
numeric_limits< sw::unum::posit<64, 3> >::is_modulo         : 0
numeric_limits< sw::unum::posit<64, 3> >::traps             : 0
numeric_limits< sw::unum::posit<64, 3> >::tinyness_before   : 0
numeric_limits< sw::unum::posit<64, 3> >::round_style       : 1
>>>>>>>>>>>>>>>>>> posit<128,4> does not render correctly due to limits of native floating point types
Numeric limits for posit< 128, 4>
numeric_limits< sw::unum::posit<128, 4> >::min()             : 0
numeric_limits< sw::unum::posit<128, 4> >::max()             : inf
numeric_limits< sw::unum::posit<128, 4> >::lowest()          : -inf
numeric_limits< sw::unum::posit<128, 4> >::epsilon()         : 3.76158e-37
numeric_limits< sw::unum::posit<128, 4> >::digits            : 122
numeric_limits< sw::unum::posit<128, 4> >::digits10          : 36
numeric_limits< sw::unum::posit<128, 4> >::max_digits10      : 37
numeric_limits< sw::unum::posit<128, 4> >::is_signed         : 1
numeric_limits< sw::unum::posit<128, 4> >::is_integer        : 0
numeric_limits< sw::unum::posit<128, 4> >::is_exact          : 0
numeric_limits< sw::unum::posit<128, 4> >::min_exponent      : -2016
numeric_limits< sw::unum::posit<128, 4> >::min_exponent10    : -610
numeric_limits< sw::unum::posit<128, 4> >::max_exponent      : 2016
numeric_limits< sw::unum::posit<128, 4> >::max_exponent10    : 610
numeric_limits< sw::unum::posit<128, 4> >::has_infinity      : 1
numeric_limits< sw::unum::posit<128, 4> >::has_quiet_NaN     : 1
numeric_limits< sw::unum::posit<128, 4> >::has_signaling_NaN : 1
numeric_limits< sw::unum::posit<128, 4> >::has_denorm        : 0
numeric_limits< sw::unum::posit<128, 4> >::has_denorm_loss   : 0
numeric_limits< sw::unum::posit<128, 4> >::is_iec559         : 0
numeric_limits< sw::unum::posit<128, 4> >::is_bounded        : 0
numeric_limits< sw::unum::posit<128, 4> >::is_modulo         : 0
numeric_limits< sw::unum::posit<128, 4> >::traps             : 0
numeric_limits< sw::unum::posit<128, 4> >::tinyness_before   : 0
numeric_limits< sw::unum::posit<128, 4> >::round_style       : 1
>>>>>>>>>>>>>>>>>> posit<256,5> does not render correctly due to limits of native floating point types
Numeric limits for posit< 256, 5>
numeric_limits< sw::unum::posit<256, 5> >::min()             : 0
numeric_limits< sw::unum::posit<256, 5> >::max()             : inf
numeric_limits< sw::unum::posit<256, 5> >::lowest()          : -inf
numeric_limits< sw::unum::posit<256, 5> >::epsilon()         : 2.21086e-75
numeric_limits< sw::unum::posit<256, 5> >::digits            : 249
numeric_limits< sw::unum::posit<256, 5> >::digits10          : 75
numeric_limits< sw::unum::posit<256, 5> >::max_digits10      : 76
numeric_limits< sw::unum::posit<256, 5> >::is_signed         : 1
numeric_limits< sw::unum::posit<256, 5> >::is_integer        : 0
numeric_limits< sw::unum::posit<256, 5> >::is_exact          : 0
numeric_limits< sw::unum::posit<256, 5> >::min_exponent      : -8128
numeric_limits< sw::unum::posit<256, 5> >::min_exponent10    : -2463
numeric_limits< sw::unum::posit<256, 5> >::max_exponent      : 8128
numeric_limits< sw::unum::posit<256, 5> >::max_exponent10    : 2463
numeric_limits< sw::unum::posit<256, 5> >::has_infinity      : 1
numeric_limits< sw::unum::posit<256, 5> >::has_quiet_NaN     : 1
numeric_limits< sw::unum::posit<256, 5> >::has_signaling_NaN : 1
numeric_limits< sw::unum::posit<256, 5> >::has_denorm        : 0
numeric_limits< sw::unum::posit<256, 5> >::has_denorm_loss   : 0
numeric_limits< sw::unum::posit<256, 5> >::is_iec559         : 0
numeric_limits< sw::unum::posit<256, 5> >::is_bounded        : 0
numeric_limits< sw::unum::posit<256, 5> >::is_modulo         : 0
numeric_limits< sw::unum::posit<256, 5> >::traps             : 0
numeric_limits< sw::unum::posit<256, 5> >::tinyness_before   : 0
numeric_limits< sw::unum::posit<256, 5> >::round_style       : 1
```

## propq

Show size tables of quires.
