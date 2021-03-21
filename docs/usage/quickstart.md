## Quick start

If you just want to experiment with the number system tools and test suites, and don't want to bother cloning and building the source code, there is a Docker container to get started:

```text
> docker pull stillwater/universal
> docker run -it --rm stillwater/universal bash
stillwater@b3e6708fd732:~/universal/build$ ls
CMakeCache.txt       Makefile      cmake-uninstall.cmake  playground  universal-config-version.cmake
CMakeFiles           applications  cmake_install.cmake    tests       universal-config.cmake
CTestTestfile.cmake  c_api         education              tools       universal-targets.cmake
```

From the build directory, it is convenient to run any of the regression test suites:

```text
stillwater@b3e6708fd732:~/universal/build$ tests/posit/specialized/fast_posit_8_0
Fast specialization posit<8,0> configuration tests
 posit<  8,0> useed scale     1     minpos scale         -6     maxpos scale          6
Logic operator tests
 posit<8,0>     ==         (native)   PASS
 posit<8,0>     !=         (native)   PASS
 posit<8,0>     <          (native)   PASS
 posit<8,0>     <=         (native)   PASS
 posit<8,0>     >          (native)   PASS
 posit<8,0>     >=         (native)   PASS
Assignment/conversion tests
 posit<8,0> integer assign (native)   PASS
 posit<8,0> float assign   (native)   PASS
Arithmetic tests
 posit<8,0> add            (native)   PASS
 posit<8,0> +=             (native)   PASS
 posit<8,0> subtract       (native)   PASS
 posit<8,0> -=             (native)   PASS
 posit<8,0> multiply       (native)   PASS
 posit<8,0> *=             (native)   PASS
 posit<8,0> divide         (native)   PASS
 posit<8,0> /=             (native)   PASS
 posit<8,0> negate         (native)   PASS
 posit<8,0> reciprocate    (native)   PASS
Elementary function tests
 posit<8,0> sqrt           (native)   PASS
 posit<8,0> exp                       PASS
 posit<8,0> exp2                      PASS
 posit<8,0> log                       PASS
 posit<8,0> log2                      PASS
 posit<8,0> log10                     PASS
 posit<8,0> sin                       PASS
 posit<8,0> cos                       PASS
 posit<8,0> tan                       PASS
 posit<8,0> atan                      PASS
 posit<8,0> asin                      PASS
 posit<8,0> acos                      PASS
 posit<8,0> sinh                      PASS
 posit<8,0> cosh                      PASS
 posit<8,0> tanh                      PASS
 posit<8,0> atanh                     PASS
 posit<8,0> acosh                     PASS
 posit<8,0> asinh                     PASS
ValidatePower has been truncated
 posit<8,0> pow                       PASS
```

In /usr/local/bin there are a set of command line utilities to inspect floating point encodings.

```text
stillwater@b3e6708fd732:~/universal/build$ ls /usr/local/bin
compd  compf  compfp  compieee  compld  complns  compp  compsi  compui  float2posit  propenv  propp

stillwater@b3e6708fd732:~/universal$ compieee 1.2345678901234567890123
compiler              : 7.5.0
float precision       : 23 bits
double precision      : 52 bits
long double precision : 63 bits

Representable?        : maybe

Decimal representations
input value:  1.2345678901234567890123
      float:                1.23456788
     double:        1.2345678901234567
long double:    1.23456789012345678899

Hex representations
input value:  1.2345678901234567890123
      float:                1.23456788    hex: 0.7f.1e0652
     double:        1.2345678901234567    hex: 0.3ff.3c0ca428c59fb
long double:    1.23456789012345678899    hex: 0.3fff.1e06521462cfdb8d

Binary representations:
      float:                1.23456788    bin: 0.01111111.00111100000011001010010
     double:        1.2345678901234567    bin: 0.01111111111.00111100000011001010010000101000110001011001                        11111011
long double:    1.23456789012345678899    bin: 0.011111111111111.0011110000001100101001000010100011000101                        10011111101101110001101

Native triple representations (sign, scale, fraction):
      float:                1.23456788    triple: (+,0,00111100000011001010010)
     double:        1.2345678901234567    triple: (+,0,00111100000011001010010000101000110001011001111110                        11)
long double:    1.23456789012345678899    triple: (+,0,00111100000011001010010000101000110001011001111110                        1101110001101)

Scientific triple representation (sign, scale, fraction):
input value:  1.2345678901234567890123
      float:                1.23456788    triple: (+,0,00111100000011001010010)
     double:        1.2345678901234567    triple: (+,0,00111100000011001010010000101000110001011001111110                        11)
long double:    1.23456789012345678899    triple: (+,0,00111100000011001010010000101000110001011001111110                        1101110001101)
      exact: TBD
```

Or posit encodings:

```text
stillwater@b3e6708fd732:~/universal/build$ compp 1.2345678901234567890123
posit< 8,0> = s0 r10 e f01000 qNE v1.25
posit< 8,1> = s0 r10 e0 f0100 qNE v1.25
posit< 8,2> = s0 r10 e00 f010 qNE v1.25
posit< 8,3> = s0 r10 e000 f01 qNE v1.25
posit<16,1> = s0 r10 e0 f001111000001 qNE v1.234619140625
posit<16,2> = s0 r10 e00 f00111100000 qNE v1.234375
posit<16,3> = s0 r10 e000 f0011110000 qNE v1.234375
posit<32,1> = s0 r10 e0 f0011110000001100101001000011 qNE v1.2345678918063641
posit<32,2> = s0 r10 e00 f001111000000110010100100001 qNE v1.2345678880810738
posit<32,3> = s0 r10 e000 f00111100000011001010010001 qNE v1.2345678955316544
posit<48,1> = s0 r10 e0 f00111100000011001010010000101000110001011010 qNE v1.2345678901234578
posit<48,2> = s0 r10 e00 f0011110000001100101001000010100011000101101 qNE v1.2345678901234578
posit<48,3> = s0 r10 e000 f001111000000110010100100001010001100010110 qNE v1.2345678901233441
posit<64,1> = s0 r10 e0 f001111000000110010100100001010001100010110011111101100000000 qNE v1.2345678901234567
posit<64,2> = s0 r10 e00 f00111100000011001010010000101000110001011001111110110000000 qNE v1.2345678901234567
posit<64,3> = s0 r10 e000 f0011110000001100101001000010100011000101100111111011000000 qNE v1.2345678901234567
posit<64,4> = s0 r10 e0000 f001111000000110010100100001010001100010110011111101100000 qNE v1.2345678901234567
```

The following two educational examples are pretty informative when you are just starting out learning about posits: edu_scales and edu_tables.

```text
stillwater@b3e6708fd732:~/universal/build$ education/posit/edu_scales
Experiments with the scale of posit numbers
Posit specificiation examples and their ranges:
Scales are represented as the binary scale of the number: i.e. 2^scale

Small, specialized posit configurations
nbits = 3
 posit<  3,0> useed scale     1     minpos scale         -1     maxpos scale          1
 posit<  3,1> useed scale     2     minpos scale         -2     maxpos scale          2
 posit<  3,2> useed scale     4     minpos scale         -4     maxpos scale          4
 posit<  3,3> useed scale     8     minpos scale         -8     maxpos scale          8
 posit<  3,4> useed scale    16     minpos scale        -16     maxpos scale         16
nbits = 4
 posit<  4,0> useed scale     1     minpos scale         -2     maxpos scale          2
 posit<  4,1> useed scale     2     minpos scale         -4     maxpos scale          4
 posit<  4,2> useed scale     4     minpos scale         -8     maxpos scale          8
 posit<  4,3> useed scale     8     minpos scale        -16     maxpos scale         16
 posit<  4,4> useed scale    16     minpos scale        -32     maxpos scale         32
nbits = 5
 posit<  5,0> useed scale     1     minpos scale         -3     maxpos scale          3
 posit<  5,1> useed scale     2     minpos scale         -6     maxpos scale          6
 posit<  5,2> useed scale     4     minpos scale        -12     maxpos scale         12
 posit<  5,3> useed scale     8     minpos scale        -24     maxpos scale         24
 posit<  5,4> useed scale    16     minpos scale        -48     maxpos scale         48
...
```

The command `edu_tables` generates tables of full posit encodings and their constituent parts:

```text
stillwater@b3e6708fd732:~/universal/build$ education/posit/edu_tables | more
Generate posit configurations
Generate Posit Lookup table for a POSIT<2,0> in TXT format
   #           Binary         Decoded       k    sign   scale          regime        exponent        fraction
     value    posit_format
   0:               00              00      -1       0       0               0               ~               ~
          0           2.0x0p
   1:               01              01       0       0       0               1               ~               ~
          1           2.0x1p
   2:               10              10       1       1       0               0               ~               ~
        nar           2.0x2p
   3:               11              11       0       1       0               1               ~               ~
         -1           2.0x3p
Generate Posit Lookup table for a POSIT<3,0> in TXT format
   #           Binary         Decoded       k    sign   scale          regime        exponent        fraction
     value    posit_format
   0:              000             000      -2       0      -1              00               ~               ~
          0           3.0x0p
   1:              001             001      -1       0      -1              01               ~               ~
        0.5           3.0x1p
   2:              010             010       0       0       0              10               ~               ~
          1           3.0x2p
   3:              011             011       1       0       1              11               ~               ~
          2           3.0x3p
   4:              100             100       2       1      -1              00               ~               ~
        nar           3.0x4p
   5:              101             111       1       1       1              11               ~               ~
         -2           3.0x5p
   6:              110             110       0       1       0              10               ~               ~
         -1           3.0x6p
   7:              111             101      -1       1      -1              01               ~               ~
       -0.5           3.0x7p
...
```