---
name: new-number-type
description: Scaffold a new number system type with all required files, CMake wiring, exception hierarchy, traits, tests, and numeric_limits. Use when adding a new arithmetic type to the Universal library.
user-invocable: true
argument-hint: "<type-name> [template-params...]"
allowed-tools: Read, Edit, Write, Glob, Grep, Bash
---

# Scaffold a New Number System Type

Create all required files, CMake wiring, and test structure for a new number type in the Universal library.

## Arguments

`$ARGUMENTS` — the type name (e.g., `takum`) and optionally a description of the template parameters. If not provided, ask the user.

## Before You Start

1. Ask the user:
   - How many template parameters? (1-param like `integer<nbits>` or 2-param like `posit<nbits, es>`)
   - Is this a **static** (fixed-size) or **elastic** (adaptive) type?
   - What category? (integer, fixed-point, float, tapered, logarithmic, block format)
   - What internal building blocks does it use? (blockbinary, blocksignificand, blocktriple, etc.)

2. Read the matching skeleton template to use as the structural reference:
   - 1-param: `include/sw/universal/number/skeleton_1param/`
   - 2-param: `include/sw/universal/number/skeleton_2params/`

3. Read an existing similar type for behavioral reference (e.g., posit for tapered, cfloat for float, fixpnt for fixed-point).

## CRITICAL Rules

These are hard-won lessons from past incidents. Violating them causes build failures or CI rejections.

### Triviality
- Number types MUST be trivially constructible
- **NO in-class member initializers**: use `uint8_t _bits;` NOT `uint8_t _bits{ 0 };`
- `ReportTrivialityOfType<T>()` will `static_assert` fail if the type isn't trivial
- Default constructor must have empty body or be `= default`

### Constexpr
- Do NOT mark constructors/assignment operators `constexpr` if they call `std::frexp`, `std::ldexp`, `std::log2`, etc.
- Only use `constexpr` if the conversion path uses only `std::memcpy` or integer arithmetic

### Exception Hierarchy
- Number systems inherit from `universal_arithmetic_exception` / `universal_internal_exception`
- Internal building blocks inherit from `std::runtime_error` (NEVER from `universal_*`)
- Dependencies flow strictly **downward**: number system -> internal block -> never upward
- Each type has its OWN exception guard macro (e.g., `TYPENAME_THROW_ARITHMETIC_EXCEPTION`)
- The umbrella header forwards its exception config to building blocks:
  ```cpp
  #if !defined(TYPENAME_THROW_ARITHMETIC_EXCEPTION)
  #define TYPENAME_THROW_ARITHMETIC_EXCEPTION 0
  #if !defined(BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION)
  #define BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION 0
  #endif
  #else
  #if !defined(BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION)
  #define BLOCKBINARY_THROW_ARITHMETIC_EXCEPTION TYPENAME_THROW_ARITHMETIC_EXCEPTION
  #endif
  #endif
  ```

### Portability
- Never use `long double` manual bit-shift division — use `std::ldexp(1.0l, exponent)` instead
- Always initialize `blockbinary` temporaries (clang doesn't zero stack like gcc)
- Test with BOTH gcc AND clang before committing

## File Creation Order

Create files in this exact order (dependencies flow top-to-bottom):

### Step 1: Header files in `include/sw/universal/number/TYPE/`

| File | Purpose | Key contents |
|------|---------|-------------|
| `TYPE_fwd.hpp` | Forward declarations + type aliases | `template<params> class TYPE;` + convenience aliases |
| `exceptions.hpp` | Exception hierarchy | `TYPE_arithmetic_exception`, `TYPE_divide_by_zero`, `TYPE_internal_exception` |
| `TYPE_impl.hpp` | Main class implementation | Full class with constructors, operators, conversions |
| `numeric_limits.hpp` | `std::numeric_limits` specialization | All required constants and static functions |
| `manipulators.hpp` | `type_tag()`, `to_binary()`, `color_print()`, `range()` | Use `enable_if_t<is_TYPE<T>>` pattern |
| `attributes.hpp` | Free functions for type properties | `sign()`, `scale()`, `TYPE_range()` |
| `TYPE.hpp` | Umbrella header | Includes everything in correct order (see below) |

**Umbrella header include order** (MUST follow this sequence):
```
1. Compiler directives (compiler.hpp, architecture.hpp, bit_cast.hpp, long_double.hpp)
2. Required stdlib (<iostream>, <iomanip>)
3. Behavioral compilation switches (TYPENAME_THROW_ARITHMETIC_EXCEPTION, etc.)
4. Exception config forwarding to building blocks
5. Trait function headers (number_traits.hpp, arithmetic_traits.hpp)
6. exceptions.hpp
7. TYPE_fwd.hpp
8. TYPE_impl.hpp
9. TYPE_traits.hpp (from traits/ directory)
10. numeric_limits.hpp
11. attributes.hpp
12. manipulators.hpp
13. mathlib.hpp (if applicable)
```

### Step 2: Traits file in `include/sw/universal/traits/`

| File | Purpose |
|------|---------|
| `TYPE_traits.hpp` | `is_TYPE` trait, `is_TYPE_trait` struct, `enable_if_TYPE` alias |

The trait MUST match the exact template parameters of the class.

### Step 3: Test directory structure

Create the test directory under the appropriate category. The repo uses
`static/<CATEGORY>/<TYPE>/` where CATEGORY groups related types:

| Category | Types |
|----------|-------|
| `tapered/` | posit, takum, unum2 |
| `float/` | cfloat, bfloat16, dfloat, hfloat, e8m0 |
| `logarithmic/` | lns, dbns |
| `fixpnt/` | binary, decimal |
| `integer/` | binary, decimal, octal, hexadecimal |
| `block/` | microfloat, mxblock, nvblock |

```
static/<CATEGORY>/TYPE/         (or elastic/TYPE/ for adaptive types)
  CMakeLists.txt
  api/
    api.cpp                     # Primary API test — start here
  conversion/
    (empty initially)
  logic/
    (empty initially)
  arithmetic/
    (empty initially)
  math/
    (empty initially)
  complex/
    (empty initially, only populated when BUILD_COMPLEX=ON)
```

### Step 4: Test CMakeLists.txt

Use the standard pattern. The `compile_all` label path uses the full hierarchy
(check an existing sibling type for the exact prefix):
```cmake
file(GLOB API_SRC        "api/*.cpp")
file(GLOB CONVERSION_SRC "conversion/*.cpp")
file(GLOB LOGIC_SRC      "logic/*.cpp")
file(GLOB ARITHMETIC_SRC "arithmetic/*.cpp")
file(GLOB MATH_SRC       "math/*.cpp")
file(GLOB COMPLEX_SRC    "complex/*.cpp")

# Label path pattern: "Number Systems/static/floating-point/<CATEGORY>/TYPE/<testdir>"
# Example for lns:    "Number Systems/static/floating-point/logarithmic/lns/api"
# Example for posit:  "Number Systems/static/floating-point/tapered/posit/api"
compile_all("true" "TYPE" "Number Systems/static/floating-point/<CATEGORY>/TYPE/api" "${API_SRC}")
compile_all("true" "TYPE" "Number Systems/static/floating-point/<CATEGORY>/TYPE/conversion" "${CONVERSION_SRC}")
compile_all("true" "TYPE" "Number Systems/static/floating-point/<CATEGORY>/TYPE/logic" "${LOGIC_SRC}")
compile_all("true" "TYPE" "Number Systems/static/floating-point/<CATEGORY>/TYPE/arithmetic" "${ARITHMETIC_SRC}")
compile_all("true" "TYPE" "Number Systems/static/floating-point/<CATEGORY>/TYPE/math" "${MATH_SRC}")
compile_all("true" "TYPE" "Number Systems/static/floating-point/<CATEGORY>/TYPE/complex" "${COMPLEX_SRC}")
```

### Step 5: CMake wiring in root CMakeLists.txt

**4 insertion points** (find the right alphabetical position among existing types):

1. **Option definition** (~line 160):
   ```cmake
   option(UNIVERSAL_BUILD_NUMBER_TYPE    "Set to ON to build TYPE tests"    OFF)
   ```

2. **UNIVERSAL_BUILD_NUMBER_STATICS cascade** (~line 831):
   ```cmake
   set(UNIVERSAL_BUILD_NUMBER_TYPE ON)
   ```

3. **add_subdirectory block** (~line 974):
   ```cmake
   if(UNIVERSAL_BUILD_NUMBER_TYPE)
     add_subdirectory("static/<CATEGORY>/TYPE")
   endif(UNIVERSAL_BUILD_NUMBER_TYPE)
   ```

4. **CI_LITE cascade** (~line 756, optional — only if portability-critical):
   ```cmake
   set(UNIVERSAL_BUILD_NUMBER_TYPE ON)
   ```

### Step 6: Initial api.cpp test

Create a minimal test that:
- Includes the umbrella header
- Sets `TYPENAME_THROW_ARITHMETIC_EXCEPTION 1`
- Tests default construction
- Tests construction from native types (int, float, double)
- Tests `type_tag()` output
- Tests `ReportTrivialityOfType<TYPE<config>>()`
- Uses `ReportTestSuiteHeader()` / `ReportTestSuiteResults()` pattern
- Has full exception catch blocks

## Template Parameter Naming Conventions

| Parameter | Name | Notes |
|-----------|------|-------|
| Total bits | `nbits` | NOT `N` or `bits` |
| Exponent bits | `es` | NOT `E` or `exponent_bits` |
| Fraction bits | `rbits` or `fbits` | Depends on type |
| Block type | `bt` | Default to `uint8_t` |
| In friend declarations | `nnbits`, `nes`, `nbt` | Prefix with `n` |

## Verification Checklist

After creating all files:

1. Build with gcc: `cmake --build --preset gcc-debug --target TYPE_api`
2. Run the test: `build/gcc-debug/static/TYPE/TYPE_api`
3. Build with clang: `cmake --build --preset clang-debug --target TYPE_api`
4. Run the clang test: `build/clang-debug/static/TYPE/TYPE_api`
5. Verify triviality passes (no `static_assert` failures)
6. Verify `type_tag()` produces expected output

## Reference Implementations

For behavioral reference, read an existing type that's similar:

| If your type is... | Study this implementation |
|--------------------|--------------------------|
| Tapered floating-point | `posit/posit_impl.hpp` |
| Classic floating-point | `cfloat/cfloat_impl.hpp` |
| Fixed-point | `fixpnt/fixpnt_impl.hpp` |
| Logarithmic | `lns/lns_impl.hpp` |
| Integer | `integer/integer_impl.hpp` |
| Block format | `mxblock/mxblock_impl.hpp` |
| Double-double | `dd/dd_impl.hpp` |
