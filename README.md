# Universal: a header-only C++ template library for universal number arithmetic

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/25452f0319d34bc2a553cd1857d7dfae)](https://app.codacy.com/gh/stillwater-sc/universal?utm_source=github.com&utm_medium=referral&utm_content=stillwater-sc/universal&utm_campaign=Badge_Grade_Dashboard)
[![Awesome Cpp](https://awesome.re/mentioned-badge.svg)](https://github.com/fffaraz/awesome-cpp#math)
[ ![Codeship Status for stillwater-sc/universal](https://app.codeship.com/projects/22533f00-252a-0136-2ba6-6657a5454f61/status?branch=master)](https://app.codeship.com/projects/286490)
[![Coverage Status](https://coveralls.io/repos/github/stillwater-sc/universal/badge.svg?branch=master)](https://coveralls.io/github/stillwater-sc/universal?branch=master)

The goal of Universal Numbers, or unums, is to replace IEEE floating-point with a number system that is more efficient and mathematically consistent in concurrent execution environments.

The motivation to replace IEEE floating-point had been brewing in the HPC community since the late 90's as most algorithms became memory bound. The inefficiency of IEEE floating-point has been measured and agreed upon, but it was the AI Deep Learning community that moved first and replaced IEEE with number systems that are tailored to the application to yield speed-ups of two to three orders of magnitude.

The Universal library is a ready-to-use header-only library that provides plug-in replacement for native types, and provides a low-friction environment to start exploring alternatives to IEEE floating-point in your own algorithms. 

The basic use pattern is as simple as:
```
#include <universal/posit/posit>

template<typename Real>
Real MyKernel(const Real& a, const Real& b) {
    return a * b;  // replace this with your kernel computation
}

constexpr double pi = 3.14159265358979323846;

int main() {
    using Real = sw::unum::posit<32,2>;  

    Real a = sqrt(2);
    Real b = pi;
    std::cout << "Result: " << MyKernel(a, b) << std::endl;
  
}
```

The library contains integers, decimals, fixed-points, rationals, linear floats, tapered floats, logarithmic, interval and several multi-precision integers and floats. There are example skeletons to get you started quickly if you desire to add your own number system, which is highly encouraged.

## Quick start

If you just want to experiment with the number system tools and test suites, and don't want to bother cloning and building the source code, there is a Docker container to get started:

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

The following two educational examples are pretty informative when you are just starting out learning about posits:
bash-4.3# education/posit/edu_scales
bash-4.3# education/posit/edu_tables
```

## How to build

If you do want to work with the code, the universal numbers software library is built using cmake version v3.18. 
Install the latest [cmake](https://cmake.org/download).
There are interactive installers for MacOS and Windows. 
For Linux, a portable approach downloads the shell archive and installs it at /usr/local:
```
> wget https://github.com/Kitware/CMake/releases/download/v3.18.2/cmake-3.18.2-Linux-x86_64.sh 
> sudo sh cmake-3.18.2-Linux-x86_64.sh --prefix=/usr/local --exclude-subdir
```
For Ubuntu, snap will install the latest cmake, and would be the preferred method:
```
> sudo snap install cmake --classic
```

The Universal library is a pure C++ template library without any further dependencies, even for the regression test suites,
to enable hassle-free installation and use.

Simply clone the github repo, and you are ready to build the different components of the Universal library. 
The library contains tools to work with integers, decimals, fixed-points, floats, posits, valids, and logarithmic
number systems. It contains educational programs that showcase simple use cases to familiarize yourself with
different number systems, and application examples to highlight the use of different number systems to gain performance
or numerical accuracy. Finally, each number system offers its own verification suite. 

The easiest way to become familiar with all the options in the build process is to fire up the CMake GUI 
(or ccmake if you are on a headless server). The cmake output will summarize which options have been set. 
The output will looks something like this:

```
$ cmake ..
-- C++14 support has been enabled by default
-- universal -> universal
-- include_install_dir         = include
-- include_install_dir_full    = include/universal
-- config_install_dir          = share/universal
-- include_install_dir_postfix = universal
-- PROJECT_SOURCE_DIR          = /home/stillwater/dev/clones/universal
-- PROJECT_VERSION             = 2.1.0
-- CMAKE_CURRENT_SOURCE_DIR    = /home/stillwater/dev/clones/universal
-- CMAKE_CURRENT_BINARY_DIR    = /home/stillwater/dev/clones/universal/build
...
--
-- ******************* Universal Arithmetic Library Configuration Summary *******************
-- General:
--   Version                      :   2.1.0
--   System                       :   Linux
--   C compiler                   :   /usr/bin/cc
--   Release C flags              :   -O3 -DNDEBUG -Wall -Wpedantic -Wno-narrowing -Wno-deprecated
--   Debug C flags                :   -g -Wall -Wpedantic -Wno-narrowing -Wno-deprecated
--   C++ compiler                 :   /usr/bin/c++
--   Release CXX flags            :   -O3 -DNDEBUG -std=c++14  -Wall -Wpedantic -Wno-narrowing -Wno-deprecated -std=c++14  -Wall -Wpedantic -Wno-narrowing -Wno-deprecated
--   Debug CXX flags              :   -g -std=c++14  -Wall -Wpedantic -Wno-narrowing -Wno-deprecated -std=c++14  -Wall -Wpedantic -Wno-narrowing -Wno-deprecated
--   Build type                   :   Release
--
--   BUILD_CI_CHECK               :   OFF
--
--   BUILD_STORAGE_CLASSES        :   OFF
--   BUILD_NATIVE_TYPES           :   OFF
--   BUILD_INTEGERS               :   OFF
--   BUILD_DECIMALS               :   OFF
--   BUILD_FIXPNTS                :   OFF
--   BUILD_LNS                    :   OFF
--   BUILD_UNUM_TYPE_1            :   OFF
--   BUILD_UNUM_TYPE_2            :   OFF
--   BUILD_POSITS                 :   OFF
--   BUILD_VALIDS                 :   OFF
--   BUILD_REALS                  :   OFF
--
--   BUILD_C_API_PURE_LIB         :   OFF
--   BUILD_C_API_SHIM_LIB         :   OFF
--   BUILD_C_API_LIB_PIC          :   OFF
--
--   BUILD_CMD_LINE_TOOLS         :   ON
--   BUILD_EXAMPLES_EDUCATION     :   ON
--   BUILD_EXAMPLES_APPLICATIONS  :   ON
--   BUILD_NUMERICAL              :   OFF
--   BUILD_FUNCTIONS              :   OFF
--   BUILD_PLAYGROUND             :   ON
--
--   BUILD_CONVERSION_TESTS       :   OFF
--
--   BUILD_PERF_TESTS             :   OFF
--
--   BUILD_IEEE_FLOAT_QUIRES      :   OFF
--
--   BUILD_DOCS                   :   OFF
--
-- Dependencies:
--   SSE3                         :   NO
--   AVX                          :   NO
--   AVX2                         :   NO
--   Pthread                      :   NO
--   TBB                          :   NO
--   OMP                          :   NO
--
-- Utilities:
--   Serializer                   :   NO
--
-- Install:
--   Install path                 :   /usr/local
--
-- Configuring done
-- Generating done
```
The build options are enabled/disabled as follows:
```
> cmake -DBUILD_EXAMPLES_EDUCATION=OFF -DBUILD_POSITS=ON ..
```
After building, issue the command _make test_ to run the complete test suite of all the enabled components, 
as a regression capability when you are modifying the source code. This will take several minutes but will touch 
all the corners of the code.

```
> git clone https://github.com/stillwater-sc/universal
> cd universal
> mkdir build
> cd build
> cmake ..
> make -j $(nproc)
> make test
```
For Windows and Visual Studio, there are `CMakePredefinedTargets` that accomplish the same tasks:

    - ALL_BUILD will compile all the projects
    - INSTALL   will install the Universal library
    - RUN_TESTS will run all tests
    
![visual-studio-project](background/img/visual-studio-project.png)

# Installation and usage

After cloning the library, building and testing it in your environment, you can install it via:
```
> cd universal/build
> cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/your/installation/path
> cmake --build . --config Release --target install -- -j $(nproc)
```

or manually via the Makefile target in the build directory:
```
> make -j ${nproc) install
```

The default install directory is `/usr/local` under Linux.  There is also an uninstall
```
> make uninstall
```

If you want to use the number systems provided by Universal in your own project, 
you can use the following CMakeLists.txt structure:
```
project("my-numerical-experiment")

find_package(UNIVERSAL CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/mymain.cpp)
target_link_libraries(${PROJECT_NAME} UNIVERSAL::UNIVERSAL)
```

## Controlling the build to include different components

The default build configuration will build the command line tools, a playground, educational and application examples.
If you want to build the full regression suite across all the number systems, use the following cmake command:
```
cmake -DBUILD_CI_CHECK=ON ..
```

For performance, the build configuration can enable specific x86 instruction sets (SSE/AVX/AVX2). For example, if your processor supports the AVX2 instruction set, you can build the test suites and educational examples with the AVX2 flag turned on. This typically yields a 20% performance boost.
```
cmake -DBUILD_CI_CHECK=on -DUSE_AVX2=ON ..
```

The library builds a set of useful command utilities to inspect native IEEE float/double/long double numbers as well as
the custom number systems provided by Universal. Assuming you have build and installed the library, the commands are

```
    compsi         -- show the components (sign, scale, fraction) of a signed integer value
    compui         -- show the components (sign, scale, fraction) of a signed integer value
    compf          -- show the components (sign, scale, fraction) of a float value
    compd          -- show the components (sign, scale, fraction) of a double value
    compld         -- show the components (sign, scale, fraction) of a long double value
    compfp         -- show the components (sign, scale, fraction) of a fixed-point value
    compp          -- show the components (sign, scale, fraction) of a posit value
    complns        -- show the components (sign, scale, fraction) of an lns value

    convert        -- show the conversion process of a Real value to a posit

    propenv        -- show the properties of the execution (==compiler) environment that built the library
    propp          -- show numerical properties of a posit environment including the associated quire
```
For example:
```
>$ compfp 1.234567890123456789012
compiler              : 7.5.0
float precision       : 23 bits
double precision      : 52 bits
long double precision : 63 bits

Decimal representations
input value:             1.23456789012
      float:                1.23456788
     double:        1.2345678901199999
long double:    1.23456789011999999999

Hex representations
input value:             1.23456789012
      float:                1.23456788    hex: 0.7f.1e0652
     double:        1.2345678901199999    hex: 0.3ff.3c0ca428c1d2b
long double:    1.23456789011999999999    hex: 0.3fff.1e06521460e95b9a

Binary representations:
      float:                1.23456788    bin: 0.01111111.00111100000011001010010
     double:        1.2345678901199999    bin: 0.01111111111.0011110000001100101001000010100011000001110100101011
long double:    1.23456789011999999999    bin: 0.011111111111111.001111000000110010100100001010001100000111010010101101110011010

Native triple representations (sign, scale, fraction):
      float:                1.23456788    triple: (+,0,00111100000011001010010)
     double:        1.2345678901199999    triple: (+,0,0011110000001100101001000010100011000001110100101011)
long double:    1.23456789011999999999    triple: (+,0,001111000000110010100100001010001100000111010010101101110011010)

Universal triple representation (sign, scale, fraction):
input value:             1.23456789012
      float:                1.23456788    triple: (+,0,00111100000011001010010)
     double:        1.2345678901199999    triple: (+,0,0011110000001100101001000010100011000001110100101011)
long double:    1.23456789011999999999    triple: (+,0,001111000000110010100100001010001100000111010010101101110011010)
      exact: TBD
```
This _compfp_ command is very handy to quickly determine how your development environment represents (truncates) a specific value. 

The specific commands _compf_, _compd_, and _compld_ focus on float, double, and long double representations respectively.

There is also a command _compp_ to help you visualize and compare the posit component fields for a given value:
```
>$ compp 1.234567890123456789012
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

## Motivation

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
3. **Tapered Accuracy** - Tapered accuracy is when values with small exponent have more digits of accuracy and values with large exponents have fewer digits of accuracy. This concept was first introduced by Morris (1971) in his paper ”Tapered Floating Point: A New Floating-Point Representation”.
4. **Parameterized precision and dynamic range** -- posits are defined by a size, _nbits_, and the number of exponent bits, _es_. This enables system designers the freedom to pick the right precision and dynamic range required for the application. For example, for AI applications we may pick 5 or 6 bit posits without any exponent bits to improve performance. For embedded DSP applications, such as 5G base stations, we may select a 16 bit posit with 1 exponent bit to improve performance per Watt.
5. **Simpler Circuitry** - There are only two special cases, Not a Real and Zero. No denormalized numbers, overflow, or underflow. 

## Goals of the library

This library is a bit-level arithmetic reference implementation of the evolving unum III (posit and valid) standard.
The goal is to provide a faithful posit arithmetic layer for any C/C++/Python environment.

As a reference library, there is extensive test infrastructure to validate the arithmetic, and there is a host
of utilities to become familiar with the internal workings of posits and valids.

We want to provide a complete unum library, and we are looking for contributors to finish the Type I and II unum implementations.

## Contributing to universal

We are happy to accept pull requests via GitHub. The only requirement that we would like PR's to adhere to
is that all the test cases pass, so that we know the new code isn't breaking any functionality.

[![Stargazers over time](https://starchart.cc/stillwater-sc/universal.svg)](https://starchart.cc/stillwater-sc/universal)

## Verification Suite

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

## Structure of the tree

The universal library contains a set of functional groups to deal with different number systems. In the examples shown above, we have seen the ".../universal/include/universal/posit" group and its test suite, ".../universal/tests/posit". 

Here is a complete list:

- *universal/unum* - flexible configuration unum number system
- *universal/integer* - arbitrary configuration integers
- *universal/fixpnt* - arbitrary configuration fixed-point number systems
- *universal/areal* - arbitrary configuration linear floating-point number systems
- *universal/posit* - arbitrary configuration posit number systems
- *universal/valid* - arbitrary configuration valid number systems
- *universal/decimal* - multi-precision decimal
- *universal/mpfloat* - multi-precision linear floating-point
- *universal/rational* - multi-precision rational number system
- *universal/float* - contains the implementation of the IEEE floating point augmentations for reproducible computation
- *universal/lns* - logarithmic number system

And each of these functionality groups have an associated test suite located in ".../universal/tests/..."

## Background information

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

## Public Domain and community resources

The unum format is a public domain specification, and there are a collection of web resources that
manage information and discussions around the use of unums.

[Posit Hub](https://posithub.org)

[Unum-computing Google Group](https://groups.google.com/forum/#!forum/unum-computing)

## Projects that leverage posits

[Matrix Template Library](http://simunova.com/#en-mtl4-index-html)

The Matrix Template Library incorporates modern C++ programming techniques to provide an easy and intuitive interface to users while enabling optimal performance. The natural mathematical notation in MTL4 empowers all engineers and scientists to implement their algorithms and models in minimal time. All technical aspects are encapsulated in the library. Think of it as MATLAB for applications.

[G+SMO](http://gs.jku.at/gismo) 

G+Smo (Geometry + Simulation Modules, pronounced "gismo") is a new open-source C++ library that brings together mathematical tools for geometric design and numerical simulation. It is developed mainly by researchers and PhD students. It implements the relatively new paradigm of isogeometric analysis, which suggests the use of a unified framework in the design and analysis pipeline. G+Smo is an object-oriented, cross-platform, template C++ library and follows the generic programming principle, with a focus on both efficiency and ease of use. The library is partitioned into smaller entities, called modules. Examples of available modules include the dimension-independent NURBS module, the data fitting and solid segmentation module, the PDE discretization module and the adaptive spline module, based on hierarchical splines of arbitrary dimension and polynomial degree. 

[FEniCS](https://fenicsproject.org/)

FEniCS is a popular open-source (LGPLv3) computing platform for solving partial differential equations (PDEs). FEniCS enables users to quickly translate scientific models into efficient finite element code. With the high-level Python and C++ interfaces to FEniCS, it is easy to get started, but FEniCS offers also powerful capabilities for more experienced programmers. FEniCS runs on a multitude of platforms ranging from laptops to high-performance clusters.


[ODEINT-v2](http://headmyshoulder.github.io/odeint-v2/)

Odeint is a modern C++ library for numerically solving Ordinary Differential Equations. It is developed in a generic way using Template Metaprogramming which leads to extraordinary high flexibility at top performance. The numerical algorithms are implemented independently of the underlying arithmetics. This results in an incredible applicability of the library, especially in non-standard environments. For example, odeint supports matrix types, arbitrary precision arithmetics and even can be easily run on CUDA GPUs.

Several AI and Deep Learning libraries are being reengineered to enable the use of posits for both training and inference. They will be announced as they are released.
