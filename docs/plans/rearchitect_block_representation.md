# Plan: Static Block Digit Components and Rational Architecture

## Context

The Universal library has an inconsistent rational number architecture. There is a mature `blockbinary` for binary storage and a mature `edecimal` for elastic decimal storage, but no static octal, decimal, or hexadecimal block types. The existing `blockdecimal` is an empty stub with ~1000 lines of dead binary code under `#ifdef LATER`. The rational types are ad hoc: `rational<nbits,bt>` uses blockbinary, `erational` uses edecimal, and there is no static decimal rational nor any octal/hex rationals. This plan creates the missing building blocks and establishes a consistent architecture with a unified rational template using Convention 2 (radix tag dispatch).

## Phase 1: Create `blockdigit<ndigits, radix>` (the unified foundation)

### New file: `include/sw/universal/internal/blockdigit/blockdigit.hpp`

A single template that generalizes edecimal's digit-by-digit arithmetic to any radix:

```cpp
template<unsigned _ndigits, unsigned _radix = 10, typename DigitType = uint8_t>
class blockdigit {
    static constexpr unsigned ndigits = _ndigits;
    static constexpr unsigned radix = _radix;
private:
    bool _negative;            // sign-magnitude (not 2's complement)
    DigitType _digit[ndigits]; // _digit[0] = least significant
};
```

**Why unified**: octal (base-8), decimal (base-10), and hexadecimal (base-16) share identical algorithms -- only the carry threshold changes. Replace edecimal's hardcoded `10` with the `radix` template parameter.

**Why sign-magnitude**: Non-binary bases don't naturally encode sign via 2's complement. Both `edecimal` and `einteger` already use sign-magnitude.

**Why NOT pack into machine words**: Unlike blockbinary which bit-packs into `uint8/16/32/64` blocks for efficiency, digit-based types store one digit per element (0 to radix-1). This matches edecimal's design and keeps arithmetic simple.

**Must be trivially constructible** per MEMORY.md: `blockdigit() = default;` with no in-class member initializers.

### Algorithms (generalized from edecimal_impl.hpp)

| Operation | Source Reference | Generalization |
|-----------|-----------------|---------------|
| Addition | edecimal lines 76-112 | `carry = sum / radix; digit = sum % radix` (was `/ 10`, `% 10`) |
| Subtraction | edecimal lines 113-166 | borrow logic with `radix` boundary |
| Multiplication | edecimal lines 167-228 | schoolbook: `digit % radix`, `carry / radix` |
| Division/Modulo | edecimal lines 770-865 | long division, `findLargestMultiple` iterates 0..radix-1 |
| Comparison | edecimal lines 682-726 | sign, then digit count, then digit-by-digit from MSD |

**Key difference from edecimal**: blockdigit is fixed-size (no `push_back`). Overflow silently truncates (modular arithmetic), matching blockbinary behavior.

### Full interface (matching blockbinary patterns)

- Trivial default constructor, constructors from all native int/float types
- Assignment operators from all native types (repeated `value % radix` / `value /= radix`)
- Explicit conversion operators to all native types (accumulate `digit * radix^i`)
- In-place arithmetic: `+=, -=, *=, /=, %=`
- Free binary operators: `+, -, *, /, %`
- Comparison: `==, !=, <, >, <=, >=`
- Digit-level shift: `<<=, >>=` (insert/remove zeros at LSD end)
- Modifiers: `clear(), setzero(), setdigit(), setsign(), setneg(), setpos(), setbits()`
- Selectors: `iszero(), sign(), isneg(), ispos(), digit()`
- `to_string()` with radix-appropriate characters (0-7 for octal, 0-9 for decimal, 0-F for hex)
- Stream I/O, `to_binary()` (shows internal digit storage), `type_tag()`

### Type aliases (in the same header)

```cpp
template<unsigned ndigits> using blockoctal = blockdigit<ndigits, 8>;
template<unsigned ndigits> using blockdecimal = blockdigit<ndigits, 10>;
template<unsigned ndigits> using blockhexadecimal = blockdigit<ndigits, 16>;
```

## Phase 2: Replace the blockdecimal stub

### File: `include/sw/universal/internal/blockdecimal/blockdecimal.hpp`

Replace the entire file (empty stub + dead `#ifdef LATER` binary code) with a redirect:

```cpp
#pragma once
#include <universal/internal/blockdigit/blockdigit.hpp>

namespace sw { namespace universal {
template<unsigned ndigits>
using blockdecimal = blockdigit<ndigits, 10>;
}} // namespace sw::universal
```

## Phase 3: Convenience headers for blockoctal and blockhexadecimal

### New: `include/sw/universal/internal/blockoctal/blockoctal.hpp`
### New: `include/sw/universal/internal/blockhexadecimal/blockhexadecimal.hpp`

Same pattern as the blockdecimal redirect -- include blockdigit.hpp and provide the alias.

## Phase 4: Test suite for blockdigit

### New directory: `static/blockdigit/`

```
static/blockdigit/
    CMakeLists.txt
    api/api.cpp              -- triviality, construction, type_tag for all three radixes
    conversion/conversion.cpp -- native type round-trip for octal, decimal, hex
    arithmetic/arithmetic.cpp -- add, sub, mul, div, mod for all three radixes
    logic/logic.cpp           -- comparison operators
```

Tests should cross-validate against `edecimal` results for decimal cases.

### CMake wiring

1. Add option: `option(UNIVERSAL_BUILD_NUMBER_BLOCKDIGIT "..." OFF)` (~line 160)
2. Add to STATICS cascade: `set(UNIVERSAL_BUILD_NUMBER_BLOCKDIGIT ON)` (~line 797)
3. Add subdirectory: `add_subdirectory("static/blockdigit")` (~line 940)

## Phase 5: Unified `rational` template with radix tag dispatch (Convention 2)

### Chosen naming convention

The user selected Convention 2: a single `rational` template with radix tag types, breaking the existing `rational<nbits, bt>` API for a cleaner architecture.

### Radix tag types

```cpp
// in include/sw/universal/number/rational/rational_fwd.hpp or a new shared header
struct base2  { static constexpr unsigned radix = 2;  };
struct base8  { static constexpr unsigned radix = 8;  };
struct base10 { static constexpr unsigned radix = 10; };
struct base16 { static constexpr unsigned radix = 16; };
```

### Unified rational template

```cpp
template<unsigned _ndigits, typename Base = base2, typename bt = uint8_t>
class rational;
```

The `_ndigits` parameter means different things per base:
- `base2`: `_ndigits` = total bits (matching current `nbits` semantics), `bt` = block type for blockbinary
- `base8/10/16`: `_ndigits` = number of digits in that base, `bt` ignored (DigitType is always uint8_t)

### Implementation via partial specialization

```cpp
// Binary specialization -- wraps blockbinary (existing behavior)
template<unsigned nbits, typename bt>
class rational<nbits, base2, bt> {
    using Component = blockbinary<nbits, bt, BinaryNumberType::Signed>;
    Component n; // numerator
    Component d; // denominator
    // ... existing rational_impl.hpp logic ...
};

// Digit-based specialization -- wraps blockdigit
template<unsigned ndigits, unsigned radix, typename bt>
class rational<ndigits, BaseTag, bt> {  // for base8, base10, base16
    using Component = blockdigit<ndigits, BaseTag::radix>;
    Component n; // numerator
    Component d; // denominator
    // ... same rational arithmetic (GCD normalization, +, -, *, /) ...
};
```

The rational arithmetic (addition via cross-multiply, GCD normalization) is identical regardless of base -- only the component type changes. We can factor this into a CRTP base or simply duplicate the small amount of rational-level logic.

### Aliases

```cpp
// Binary rationals (new syntax, replacing old rational<nbits, bt>)
using rb8   = rational<8,  base2, uint8_t>;
using rb16  = rational<16, base2, uint16_t>;
using rb32  = rational<32, base2, uint32_t>;
using rb64  = rational<64, base2, uint64_t>;
using rb128 = rational<128, base2, uint32_t>;

// Decimal rationals
using rd8  = rational<8,  base10>;
using rd16 = rational<16, base10>;
using rd32 = rational<32, base10>;
using rd64 = rational<64, base10>;

// Octal rationals
using ro8  = rational<8,  base8>;
using ro16 = rational<16, base8>;

// Hexadecimal rationals
using rh8  = rational<8,  base16>;
using rh16 = rational<16, base16>;
```

### Migration of existing rational code

The existing `rational_impl.hpp` becomes the `base2` specialization:
1. Rename `rational<nbits, bt>` to `rational<nbits, base2, bt>`
2. Move the existing implementation into the specialization
3. Update `rational_fwd.hpp` with the new primary template + base tags
4. Update `rational.hpp` umbrella header with new aliases
5. Update `static/rational/` test files to use new aliases (mostly just `rb16` etc. which stay the same)

### Files to modify for rational refactor

| File | Action |
|------|--------|
| `include/sw/universal/number/rational/rational_fwd.hpp` | **EDIT** - add base tags, new primary template declaration |
| `include/sw/universal/number/rational/rational_impl.hpp` | **EDIT** - wrap existing impl as `rational<nbits, base2, bt>` specialization; add digit-based specialization |
| `include/sw/universal/number/rational/rational.hpp` | **EDIT** - update aliases to `rational<N, base2, bt>` syntax; add rd/ro/rh aliases |
| `include/sw/universal/number/rational/manipulators.hpp` | **EDIT** - update type_tag for new template signature |
| `include/sw/universal/number/rational/attributes.hpp` | **EDIT** - update template signatures |
| `include/sw/universal/number/rational/numeric_limits.hpp` | **EDIT** - add specializations for digit-based rationals |
| `include/sw/universal/traits/rational_traits.hpp` | **EDIT** - update template signatures |
| `static/rational/api/api.cpp` | **EDIT** - update to new template syntax (aliases like rb16 remain unchanged) |
| `static/rational/conversion/*.cpp` | **EDIT** - minor template signature updates |

## Architecture Diagram (final state)

```
                    STATIC (fixed-size)              ELASTIC (adaptive)

Binary storage:     blockbinary<nbits, bt>           einteger<bt>
                         |                                |
                    rational<N, base2, bt>           (future: erational_binary)
                    aliases: rb8..rb128

Octal storage:      blockdigit<ndigits, 8>           (future)
                    alias: blockoctal<ndigits>
                         |
                    rational<N, base8>
                    aliases: ro8..ro64

Decimal storage:    blockdigit<ndigits, 10>          edecimal
                    alias: blockdecimal<ndigits>          |
                         |                           erational
                    rational<N, base10>
                    aliases: rd8..rd64

Hex storage:        blockdigit<ndigits, 16>          (future)
                    alias: blockhexadecimal<ndigits>
                         |
                    rational<N, base16>
                    aliases: rh8..rh64
```

## Verification Plan

1. **Build blockdigit**: `cmake -DUNIVERSAL_BUILD_NUMBER_BLOCKDIGIT=ON .. && make -j4`
2. **Run blockdigit tests**: `ctest -R blockdigit`
3. **Cross-validate**: In api.cpp, compute the same arithmetic with `blockdigit<8,10>` and `edecimal`, verify matching results
4. **Triviality**: `ReportTrivialityOfType<blockdigit<8,10>>()` must pass
5. **All radixes**: Verify `blockoctal<8>`, `blockdecimal<8>`, `blockhexadecimal<8>` all construct and compute correctly
6. **Build rational**: `cmake -DUNIVERSAL_BUILD_NUMBER_RATIONALS=ON .. && make -j4`
7. **Run rational tests**: existing `static/rational/` tests must still pass with the refactored template
8. **Build with gcc and clang**: Per MEMORY.md rules, test both compilers before committing

## Implementation Order

1. Phase 1: `blockdigit.hpp` -- the core building block
2. Phase 2-3: Replace blockdecimal stub, add blockoctal/blockhexadecimal alias headers
3. Phase 4: blockdigit test suite + CMake wiring
4. Phase 5: Refactor rational to unified template with base tags, add digit-based specialization

## Key Files

| File | Action |
|------|--------|
| `include/sw/universal/internal/blockdigit/blockdigit.hpp` | **CREATE** - main implementation |
| `include/sw/universal/internal/blockdecimal/blockdecimal.hpp` | **REPLACE** - redirect to blockdigit |
| `include/sw/universal/internal/blockoctal/blockoctal.hpp` | **CREATE** - alias header |
| `include/sw/universal/internal/blockhexadecimal/blockhexadecimal.hpp` | **CREATE** - alias header |
| `static/blockdigit/CMakeLists.txt` | **CREATE** - test build |
| `static/blockdigit/api/api.cpp` | **CREATE** - API tests |
| `static/blockdigit/arithmetic/arithmetic.cpp` | **CREATE** - arithmetic tests |
| `static/blockdigit/conversion/conversion.cpp` | **CREATE** - conversion tests |
| `static/blockdigit/logic/logic.cpp` | **CREATE** - comparison tests |
| `CMakeLists.txt` | **EDIT** - add BLOCKDIGIT build option and wiring |
| `include/sw/universal/number/rational/rational_fwd.hpp` | **EDIT** - base tags + new primary template |
| `include/sw/universal/number/rational/rational_impl.hpp` | **EDIT** - base2 specialization + digit specialization |
| `include/sw/universal/number/rational/rational.hpp` | **EDIT** - new aliases |
| `include/sw/universal/number/rational/manipulators.hpp` | **EDIT** - update template sigs |
| `include/sw/universal/number/rational/attributes.hpp` | **EDIT** - update template sigs |
| `include/sw/universal/number/rational/numeric_limits.hpp` | **EDIT** - add digit-based specializations |
| `include/sw/universal/traits/rational_traits.hpp` | **EDIT** - update template sigs |
| `static/rational/api/api.cpp` | **EDIT** - update template syntax |
| `static/rational/conversion/*.cpp` | **EDIT** - update template syntax |
| `include/sw/universal/number/edecimal/edecimal_impl.hpp` | **READ ONLY** - reference for algorithms |
| `include/sw/universal/internal/blockbinary/blockbinary.hpp` | **READ ONLY** - reference for interface patterns |
