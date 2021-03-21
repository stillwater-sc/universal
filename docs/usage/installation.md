# How to build

If you do want to work with the code, the universal numbers software library is built using cmake version v3.18. 
Install the latest [cmake](https://cmake.org/download).
There are interactive installers for MacOS and Windows. 
For Linux, a portable approach downloads the shell archive and installs it at /usr/local:

```text
> wget https://github.com/Kitware/CMake/releases/download/v3.18.2/cmake-3.18.2-Linux-x86_64.sh 
> sudo sh cmake-3.18.2-Linux-x86_64.sh --prefix=/usr/local --exclude-subdir
```

For Ubuntu, snap will install the latest cmake, and would be the preferred method:

```text
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

```text
$ git clone https://github.com/stillwater-sc/universal
$ cd universal
$ mkdir build
$ cd build
$ cmake ..
-- The C compiler identification is GNU 7.5.0
-- The CXX compiler identification is GNU 7.5.0
-- The ASM compiler identification is GNU
-- Found assembler: /usr/bin/cc
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
-- No default build type specified: setting CMAKE_BUILD_TYPE=Release
-- C++17 support has been enabled by default
-- Performing Test COMPILER_HAS_SSE3_FLAG
-- Performing Test COMPILER_HAS_SSE3_FLAG - Success
-- Performing Test COMPILER_HAS_AVX_FLAG
-- Performing Test COMPILER_HAS_AVX_FLAG - Success
-- Performing Test COMPILER_HAS_AVX2_FLAG
-- Performing Test COMPILER_HAS_AVX2_FLAG - Success
-- universal -> universal
-- include_install_dir         = include
-- include_install_dir_full    = include/universal
-- config_install_dir          = share/universal
-- include_install_dir_postfix = universal
-- PROJECT_SOURCE_DIR          = /home/stillwater/dev/clones/universal
-- PROJECT_VERSION             = 2.1.41
-- CMAKE_CURRENT_SOURCE_DIR    = /home/stillwater/dev/clones/universal
-- CMAKE_CURRENT_BINARY_DIR    = /home/stillwater/dev/clones/universal/build
--
-- ******************* Universal Arithmetic Library Configuration Summary *******************
-- General:
--   Version                      :   2.1.41
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
--   BUILD_EDUCATION              :   ON
--   BUILD_APPLICATIONS           :   ON
--   BUILD_NUMERICAL              :   OFF
--   BUILD_FUNCTIONS              :   OFF
--   BUILD_PLAYGROUND             :   ON
--
--   BUILD_CONVERSION_TESTS       :   OFF
--
--   BUILD_PERFORMANCE_TESTS      :   OFF
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
-- Build files have been written to: /home/stillwater/dev/clones/universal/build
```

The build options are enabled/disabled as follows:

```text
> cmake -DBUILD_EDUCATION=OFF -DBUILD_POSITS=ON ..

```

After building, issue the command _make test_ to run the complete test suite of all the enabled components, 
as a regression capability when you are modifying the source code. This will take several minutes but will touch 
all the corners of the code.

```text
> git clone https://github.com/stillwater-sc/universal
> cd universal
> mkdir build
> cd build
> cmake ..
> make -j $(nproc)
> make test
```

For Windows and Visual Studio, there are `CMakePredefinedTargets` that accomplish the same tasks:

```text
    - ALL_BUILD will compile all the projects
    - INSTALL   will install the Universal library
    - RUN_TESTS will run all tests
```

![visual-studio-project](../img/visual-studio-project.png)