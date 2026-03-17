# unum Type I Implementation Roadmap

Epic: [#192](https://github.com/stillwater-sc/universal/issues/192)

## Overview

Gustafson's original universal number (unum Type I) provides variable-precision
arithmetic with uncertainty tracking via the ubit. The ubox interval dynamics
enable rigorous numerical analysis of computation. This connects to the valid
number system for comparing computational problems like root finding and
discontinuous function evaluations.

## Current State

Scaffold exists in `include/sw/universal/number/unum/` but all methods are
no-ops with no data members. CMake wiring is in place. Test files in
`elastic/unum/` have extensive commented-out API tests under `#ifdef LATER`.

## Template Design

```cpp
template<unsigned esizesize, unsigned fsizesize, typename bt = uint8_t>
class unum;
```

- `esizesize`: bits for the esize field (e.g., 3 means esize can be 0..7)
- `fsizesize`: bits for the fsize field (e.g., 4 means fsize can be 0..15)
- `bt`: block type for underlying storage

### Bit Layout

```
[sign | exponent(esize bits) | fraction(fsize bits) | esize_field(esizesize bits) | fsize_field(fsizesize bits) | ubit]
```

The word length is variable, ranging from:
- Minimum: `1 + 1 + 0 + esizesize + fsizesize + 1` bits
- Maximum: `1 + 2^esizesize + 2^fsizesize + esizesize + fsizesize + 1` bits

## Phases

### Phase 1: Core Type & Storage ([#564](https://github.com/stillwater-sc/universal/issues/564))

**Goal**: A unum that can hold a value and round-trip through encode/decode.

- Add storage member (`blockbinary<MAXBITS, bt>` to stay trivially copyable)
- Implement `encode(sign, esize, exponent, fsize, fraction, ubit)` / `decode(...)` to pack/unpack the variable-width word
- Implement `setbits()`, `bits()`, `clear()`, `setzero()`, `setnan()`
- Add `unum_fwd.hpp` with forward declarations
- Implement `minpos`, `maxpos`, `minneg`, `maxneg` helpers
- Wire `to_binary()`, `type_tag()` in manipulators
- Activate basic construction tests in `elastic/unum/`

### Phase 2: Conversions ([#565](https://github.com/stillwater-sc/universal/issues/565))

**Goal**: Convert between unum and native IEEE-754 types.

- `operator=(float/double/long double)` -- find tightest unum representation, set ubit if inexact
- `operator float/double/long double()` -- decode to native
- `operator=(int/long/long long)` -- integer assignment
- Test round-trip conversions for exact values and ubit-flagged inexact values
- Add `elastic/unum/conversion/` test directory

### Phase 3: Comparison & Logic ([#566](https://github.com/stillwater-sc/universal/issues/566))

**Goal**: All six comparison operators with proper ubit semantics.

- `==, !=, <, >, <=, >=` -- compare decoded values, handle ubit semantics
- NaN/uNaN comparisons (all-ones encoding)
- Add `elastic/unum/logic/` tests

### Phase 4: Arithmetic ([#567](https://github.com/stillwater-sc/universal/issues/567))

**Goal**: Four basic operations with ubit propagation.

Supersedes closed sub-issues #188 (multiplication), #189 (addition), #190 (division).

- `+, -, *, /` -- convert to high-precision intermediate, compute, find tightest unum result
- Set ubit=1 when exact result is not representable
- `++, --` (move to next/prev unum in the encoding)
- Exception handling: divide-by-zero with `UNUM_THROW_ARITHMETIC_EXCEPTION` guard
- Add `elastic/unum/arithmetic/` tests

### Phase 5: IO & Display ([#568](https://github.com/stillwater-sc/universal/issues/568))

**Goal**: Complete display and parsing support.

Supersedes closed sub-issue #193 (ostream operator).

- `operator<<` -- formatted output
- `operator>>` / `parse()` -- string input (decimal and hex format)
- `color_print()` -- sign(red), exponent(green), fraction(cyan), esize/fsize(yellow), ubit(magenta)
- `info_print()`, `pretty_print()`
- Complete `numeric_limits<>` specialization

### Phase 6: Math Functions ([#569](https://github.com/stillwater-sc/universal/issues/569))

**Goal**: Standard math library for unum.

- `abs, sqrt, exp, log, sin, cos, tan` etc.
- Leverage native conversion for initial implementation
- Add `elastic/unum/math/` tests

### Phase 7: ubox / Interval Operations ([#570](https://github.com/stillwater-sc/universal/issues/570))

**Goal**: The ubox interval arithmetic that makes unum Type I unique.

Related: [#196](https://github.com/stillwater-sc/universal/issues/196) (ubox visualization)

- When `ubit==1`, the unum represents open interval `(x, next(x))`
- Implement ubound pairs (two unums forming a closed interval)
- Interval arithmetic: `[a,b] op [c,d]` with proper endpoint rounding
- Connect to `valid` number system (re-implement using unum pairs instead of posit pairs)

### Phase 8: Exhaustive Validation ([#571](https://github.com/stillwater-sc/universal/issues/571))

**Goal**: Full verification for small configurations.

Supersedes closed sub-issue #191 (performance baseline).

- For `unum<2,2>` and `unum<3,3>`: enumerate all bit patterns, verify decoded values match Gustafson's tables ("The End of Error" Ch. 4)
- Verify ubit propagation for all arithmetic combinations
- Performance baseline

## Dependency Graph

```
Phase 1 (core) --> Phase 2 (conversions) --> Phase 3 (logic)
                                         --> Phase 4 (arithmetic) --> Phase 7 (ubox) --> #196
                                         --> Phase 5 (IO)
                                         --> Phase 6 (math)
All phases --> Phase 8 (validation)
```

## References

- Gustafson, "The End of Error: Unum Computing" (2015), especially Chapter 4
- Existing scaffold: `include/sw/universal/number/unum/`
- Related valid type: `include/sw/universal/number/valid/`
- einteger pattern for dynamic storage: `include/sw/universal/number/einteger/`
