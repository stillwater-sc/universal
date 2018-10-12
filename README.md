# Universal: a C++ template library for universal number arithmetic

Universal Numbers, or unums, are a collection of number systems to replace floating point with more efficient and mathematically correct arithmetic properties.

# How to build

[ ![Codeship Status for stillwater-sc/universal](https://app.codeship.com/projects/22533f00-252a-0136-2ba6-6657a5454f61/status?branch=master)](https://app.codeship.com/projects/286490)
[![Coverage Status](https://coveralls.io/repos/github/stillwater-sc/universal/badge.svg?branch=master)](https://coveralls.io/github/stillwater-sc/universal?branch=master)

If just want to experiment with the number system tools and test suites, and don't want to bother cloning and building the source code, there is a Docker container at your service:

```
> docker pull stillwater/universal
> docker run -it --rm stillwater/universal /bin/bash
bash-4.3# ls tools/cmd
CTestTestfile.cmake  cmake_install.cmake  cmd_dc  cmd_fc  cmd_ieee_fp  cmd_ldc  cmd_numeric_limits  cmd_pc
bash-4.3# tools/cmd/cmd_ieee_fp 1.2345678901234567890123
input value:  1.2345678901234567890123
      float:                1.23456788 (+,0,00111100000011001010010)
     double:        1.2345678901234567 (+,0,0011110000001100101001000010100011000101100111111011)
long double:    1.23456789012345669043 (+,0,001111000000110010100100001010001100010110011111101100000000000)

or run a test suite

bash-4.3# tests/posit/posit_8bit_posit
Standard posit<8,0> configuration tests
 posit<  8,0> useed scale     1     minpos scale         -6     maxpos scale          6
 posit<8,0> add         PASS
 posit<8,0> subtract    PASS
 posit<8,0> multiply    PASS
 posit<8,0> divide      PASS
 posit<8,0> negate      PASS
 posit<8,0> reciprocate PASS

These two educational examples are pretty informative when you are just starting out learning about posits:
bash-4.3# education/posit/edu_scales
bash-4.3# education/posit/edu_tables
```

If you do want to work with the code, the universal numbers software library is built using cmake. 
Install the latest cmake [cmake](https://cmake.org/download).

The library is a pure template library without any further dependencies.

Simply clone the github repo, and you are ready to build the universal library. What you are building are tools to work with floats and posits, educational programs that highlight the use of the posit library, and the posit verification suite. Issue the command _make test_ (or better yet _ctest -j 16_ (or how many cores you have)) to run the complete posit verification suite, which can be used as a regression capability when you are modifying the source code. This will take several minutes but will touch all the corners of the posit functionality.

```
> git clone https://github.com/stillwater-sc/universal
> cd universal/build
> cmake ..
> make
> make test
```

The default build configuration will build the educational examples and utilities. If you want to build the full regression suite, use the following cmake command:
```
cmake -DBUILD_CI_CHECK=ON ..
```

For performance, the default build configuration compiles with AVX2. If your particular CPU doesn't support AVX2, you will need to build with that flag turned off:
```
cmake -DBUILD_CI_CHECK=on -DUSE_AVX2=OFF ..
```

The library builds a set of useful command utilities, which can be found in the directory ".../build/tools/cmd".

```
>:~/dev/universal/build$ make cmd_ieee_fp
Scanning dependencies of target cmd_ieee_fp
[ 50%] Building CXX object tools/cmd/CMakeFiles/cmd_ieee_fp.dir/ieee_fp.cpp.o
[100%] Linking CXX executable cmd_ieee_fp
[100%] Built target cmd_ieee_fp
>:~/dev/universal/build$ tools/cmd/cmd_ieee_fp 1.234567890123456789012
input value:   1.234567890123456789012
      float:                1.23456788 (+,0,00111100000011001010010)
     double:        1.2345678901234567 (+,0,0011110000001100101001000010100011000101100111111011)
long double:    1.23456789012345669043 (+,0,000000000000001111000000110010100100001010001100010110011111101)
```
The command _cmd_ieee_fp_ is very handy to quickly determine how your development environment represents (truncates) a specific value. There are also the specific commands _cmd_fc_, _cmd_dc_, and _cmd_ldc_, which focus on float, double, and long double representations respectively.

There is also a command _cmd_pc_ to help you visualize and compare the posit component fields for a given value:
```
>:~/dev/universal/build$ make cmd_pc
Scanning dependencies of target cmd_pc
[100%] Building CXX object tools/cmd/CMakeFiles/cmd_pc.dir/pc.cpp.o
[100%] Linking CXX executable cmd_pc
[100%] Built target cmd_pc
>:~/dev/universal/build$ tools/cmd/cmd_pc 1.234567890123456789012
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

The fields are prefixed by their first characters, for example, "posit<16,2> = s0 r10 e00 f00111100000 qNE v1.234375"
- sign     field = s0, indicating a positive number
- regime   field = r10, indicates the first positive regime, named regime 0
- exponent field = e00, indicates two bits of exponent, both 0
- fraction field = f00111100000, a full set of fraction bits

The field values are followed by a quadrant descriptor and a value representation in decimal:

- qNE            = North-East Quadrant, representing a number in the range "[1, maxpos]"
- v1.234375      = the value representation of the posit projection

The positive regime for a posit shows a very specific structure, as can be seen in the image blow:
![regime structure](background/img/positive_regimes.png)

# Motivation

Modern AI applications have demonstrated the inefficiencies of the IEEE floating point format. Both Google and Microsoft have jettisonned IEEE floating point for their AI cloud services to gain two orders of magnitude better performance. Similarly, AI applications for mobile and embedded applications are shifting away from IEEE floating point. But, AI applications are hardly the only applications that expose the limitations of floating point. Cloud scale, IoT, embedded, control, and HPC applications are also limited by the inefficiencies of the IEEE floating point format. A simple change to a new number system can improve scale and cost of these appliations by orders of magnitude.

When performance and/or power efficiency are differentiating attributes for the use case, the complexity of IEEE floats simply can't compete with number systems that are tailored to the needs of the application. 

## Advantages of posits: better, faster, cheaper, and more power efficient

The core limitations of IEEE floating point are caused by two key problems of the format: 
- inefficient representation of the reals
- irreproducibility in the context of concurrency

The complete list of issues that are holding back IEEE floating point formats:
1. **Wasted Bit Patterns** - 32-bit IEEE floating point has around eight million ways to represent NaN (Not-A-Number), while 64-bit floating point has two quadrillion, that is approximately 2.251x10^15 to be more exact. A NaN is an exception value to represent undefined or invalid results, such as the result of a division by zero.
2. **Mathematically Incorrect** - The format specifies two zeroes - a negative and positive zero - which have different behaviors. - Loss of associative and distributive law due to rounding after each operation. This loss of associative and distributive arithmetic behavior creates irreproducible result of concurrent programs that use IEEE floating point. This is particularly problematic for embedded and control applications.
3. **Overflows to ± inf and underflows to 0** - Overflowing to ± inf increases the relative error by an infinite factor, while underflowing to 0 loses sign information.
4. **Unused dynamic range** - The dynamic range of double precision floats is a whopping 2^2047, whereas most numerical software is architected to operate around 1.0.
5. **Complicated Circuitry** - Denormalized floating point numbers have a hidden bit of 0 instead of 1. This creates
a host of special handling requirements that complicate compliant hardware implementations.
6. **No Gradual Overflow and Fixed Accuracy** - If accuracy is defined as the number of significand bits, IEEE
floating point have fixed accuracy for all numbers except denormalized numbers because the number of signficand
digits is fixed. Denormalized numbers are characterized by a decreased number of significand digits when the value approaches zero as a result of having a zero hidden bit. Denormalized numbers fill the underflow gap (i.e.
the gap between zero and the least non-zero values). The counterpart for gradual underflow is gradual overflow
which does not exist in IEEE floating points.

In contrast, the _posit_ number system is designed to be efficient, symmetric, and mathematically correct in any concurrency environment.

1. **Economical** - No bit patterns are redundant. There is one representation for infinity denoted as ± inf and zero.
All other bit patterns are valid distinct non-zero real numbers. ± inf serves as a replacement for NaN.
2. **Mathematical Elegant** - There is only one representation for zero, and the encoding is symmetric around 1.0. Associative and distributive laws are supported through deferred rounding via the quire, enabling reproducible linear algebra algorithms in any concurrency environment.
3. **Tapered Accuracy** - Tapered accuracy is when values with small exponent have more digits of accuracy and values with large exponents have less digits of accuracy. This concept was first introduced by Morris (1971) in his paper ”Tapered Floating Point: A New Floating-Point Representation”.
4. **Parameterized precision and dynamic range** -- posits are defined by a size, _nbits_, and the number of exponent bits, _es_. This enables system designers the freedom to pick the right precision and dynamic range required for the application. For example, for AI applications we may pick 5 or 6 bit posits without any exponent bits to improve performance. For embedded DSP applications, such as 5G base stations, we may select a 16 bit posit with 1 exponent bit to improve performance per Watt.
5. **Simpler Circuitry** - There are only two special cases, Not a Real and Zero. No denormalized numbers, overflow, or underflow. 

# Goals of the library

This library is a bit-level arithmetic reference implementation of the evolving unum III (posit and valid) standard.
The goal is to provide a faithful posit arithmetic layer for any C/C++/Python environment.

As a reference library, there is extensive test infrastructure to validate the arithmetic, and there is a host
of utilities to become familiar with the internal workings of posits and valids.

We want to provide a complete unum library, and we are looking for contributors to finish the Type I and II unum implementations.

# Contributing to universal

We are happy to accept pull requests via GitHub. The only requirement that we would like PR's to adhere to
is that all the test cases pass, so that we know the new code isn't breaking any functionality. 

# Verification Suite

Normally, the verification suite is run as part of the _make test_ command in the build directory. However, it is possible to run specific components of the test suite, for example, to validate algorithmic changes to more complex arithmetic functions, such as square root, exponent, logarithm, and trigonometric functions.

Here is an example:
```
>:~/dev/universal/build$ make posit_32bit_posit
Scanning dependencies of target posit_32bit_posit
[100%] Building CXX object tests/posit/CMakeFiles/posit_32bit_posit.dir/32bit_posit.cpp.o
[100%] Linking CXX executable posit_32bit_posit
[100%] Built target posit_32bit_posit
>:~/dev/universal/build$ tests/posit/posit_32bit_posit
Standard posit<32,2> configuration tests
 posit< 32,2> useed scale     4     minpos scale       -120     maxpos scale        120

Arithmetic tests 200000 randoms each
 posit<32,2> addition       PASS
 posit<32,2> subtraction    PASS
 posit<32,2> multiplication PASS
 posit<32,2> division       PASS
```

# Structure of the tree

The universal library contains a set of functionality groups to deal with different number systems. In the examples shown above, we have seen the ".../universal/posit" group and its test suite, ".../universal/tests/posit". Here is a complete list:

- *universal/posit* - contains the implementation of the posit number system
- *universal/valid* - contains the implementation of the valid number system
- *universal/unum* - contains the implementation of the unum Type I number system (TBD)
- *universal/unum2* - contains the implementation of the unum Type II number system (TBD)
- *universal/float* - contains the implementation of the IEEE floating point augmentations for reproducible computation
- *universal/bitset* - contains the implementation of an abitrary integer number system

And each of these functionality groups have an associated test suite located in ".../universal/tests/..."

# Background information

Universal numbers, unums for short, are for expressing real numbers, and ranges of real numbers. 
There are two modes of operation, selectable by the programmer, _posit_ mode, and _valid_ mode.

In _posit_ mode, a unum behaves much like a floating-point number of fixed size, 
rounding to the nearest expressible value if the result of a calculation is not expressible exactly.
A posit offers more accuracy and a larger dynamic range than floats with the same number of bits.

In _valid_ mode, a unum represents a range of real numbers and can be used to rigorously bound answers 
much like interval arithmetic does.

Posit configurations have a very specific relationship to one another. When expanding a posit, 
the new value falls 'between' the old values of the smaller posit. The new value is the arithmetic mean 
of the two numbers if the expanding bit is a fraction bit, and it is the geometric mean of the two numbers 
if the expanding bit is a regime or exponent bit. 
This [page](PositRefinementViz.md) shows a visualization of the expansion of _posit<2,0>_ to _posit<7,1>_:

# Public Domain and community resources

The unum format is a public domain specification, and there are a collection of web resources that
manage information and discussions around the use of unums.

[Posit Hub](https://posithub.org)

[Unum-computing Google Group](https://groups.google.com/forum/#!forum/unum-computing)

# Projects that leverage posits

[Matrix Template Library](http://simunova.com/#en-mtl4-index-html)

The Matrix Template Library incorporates modern C++ programming techniques to provide an easy and intuitive interface to users while enabling optimal performance. The natural mathematical notation in MTL4 empowers all engineers and scientists to implement their algorithms and models in minimal time. All technical aspects are encapsulated in the library. Think of it as MATLAB for applications.

[G+SMO](http://gs.jku.at/gismo) 

G+Smo (Geometry + Simulation Modules, pronounced "gismo") is a new open-source C++ library that brings together mathematical tools for geometric design and numerical simulation. It is developed mainly by researchers and PhD students. It implements the relatively new paradigm of isogeometric analysis, which suggests the use of a unified framework in the design and analysis pipeline. G+Smo is an object-oriented, cross-platform, template C++ library and follows the generic programming principle, with a focus on both efficiency and ease of use. The library is partitioned into smaller entities, called modules. Examples of available modules include the dimension-independent NURBS module, the data fitting and solid segmentation module, the PDE discretization module and the adaptive spline module, based on hierarchical splines of arbitrary dimension and polynomial degree. 

[FEniCS](https://fenicsproject.org/)

FEniCS is a popular open-source (LGPLv3) computing platform for solving partial differential equations (PDEs). FEniCS enables users to quickly translate scientific models into efficient finite element code. With the high-level Python and C++ interfaces to FEniCS, it is easy to get started, but FEniCS offers also powerful capabilities for more experienced programmers. FEniCS runs on a multitude of platforms ranging from laptops to high-performance clusters.


[ODEINT-v2](http://headmyshoulder.github.io/odeint-v2/)

Odeint is a modern C++ library for numerically solving Ordinary Differential Equations. It is developed in a generic way using Template Metaprogramming which leads to extraordinary high flexibility at top performance. The numerical algorithms are implemented independently of the underlying arithmetics. This results in an incredible applicability of the library, especially in non-standard environments. For example, odeint supports matrix types, arbitrary precision arithmetics and even can be easily run on CUDA GPUs.

Several AI and Deep Learning libraries are being reengineered to enable the use of posits for both training and inference. They will be announced as they are released.
