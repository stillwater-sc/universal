# Data Serialization Architecture for Universal Number Systems

Issue: [#541](https://github.com/stillwater-sc/universal/issues/541)
Related: [#509](https://github.com/stillwater-sc/universal/issues/509)

## Problem Statement

Universal supports tens of thousands of number type configurations through its
template parameterization. Each `cfloat<nbits, es, bt, sub, sup, sat>` alone
has 6 template parameters producing a combinatorial explosion of distinct types.
A serialization facility that tries to enumerate all possible types at
deserialization time (the current approach in `restoreCollection()`) is
fundamentally unworkable.

The current design also conflates two distinct concerns:
1. **File format** -- how type metadata and raw bits are written/read
2. **Type instantiation** -- which C++ template specializations exist

These must be separated. The file format is universal; the type instantiation
is client-specific.

## Key Insight: Client-Directed Deserialization

Unlike HDF5, which maps to a fixed set of native types (int8..float64), our
serialization facility must handle an open-ended set of types. The solution
is **client-directed deserialization**: the client declares the types it
supports, and only those template instantiations are compiled.

Real-world analogy: NVIDIA's TensorRT serialization only deserializes to
types the NVIDIA runtime supports (FP32, FP16, INT8, TF32, FP8-E4M3,
NVFP4). An AMD runtime cannot load a TensorRT file with TF32 data because
AMD GPUs don't support TF32. The client (runtime) defines the type universe.

## Architecture

### Layer 1: Type Registry (compile-time)

A **type registry** is a compile-time construct that maps type IDs (from the
file) to factory functions (template instantiations). The client builds a
registry by listing its supported types:

```cpp
// An NVIDIA DNN inference client
using NvidiaTypes = type_list<
    float, double,
    cfloat<8, 4, uint8_t, true, true, false>,   // E4M3
    cfloat<8, 5, uint8_t, true, true, false>,    // E5M2
    cfloat<16, 5, uint16_t, true, false, false>,  // FP16
    cfloat<19, 8, uint32_t, true, false, false>   // TF32
>;

// An AMD DNN inference client
using AmdTypes = type_list<
    float, double,
    cfloat<16, 5, uint16_t, true, false, false>,  // FP16
    cfloat<16, 8, uint16_t, true, false, false>    // BF16
>;

// A numerical analysis client
using NumericsTypes = type_list<
    float, double,
    dd, qd,
    posit<32, 2>, posit<64, 2>,
    cfloat<32, 8, uint32_t, true, false, false>    // single
>;
```

The registry generates the necessary `restore<T>()` instantiations at
compile time through template parameter pack expansion. No runtime
enumeration of types is needed.

### Layer 2: File Format (runtime)

The file format is independent of the type registry. It stores:

```
[magic number]
[type_id] [nr_parameters] [param0 param1 ...]  -- type descriptor
[aggregate_type] [dimensions...]                -- shape descriptor
[raw hex data...]                               -- payload
[name]                                          -- dataset name
... (repeat for each dataset)
[0]                                             -- termination
```

The type descriptor uses the existing `generateScalarTypeId()` encoding,
which captures the full template parameterization (nbits, es, bt, flags).
This is sufficient to uniquely identify any Universal type.

The payload is raw hex (bit-exact, no floating-point formatting loss).

### Layer 3: Restore Dispatch (compile-time generated, runtime executed)

When restoring, the framework reads the type descriptor from the file
and dispatches to the matching factory function in the client's registry.

```cpp
// Pseudocode for the dispatch mechanism
template<typename TypeList>
class typed_datafile {
    bool restore(std::istream& istr) {
        auto [typeId, params] = read_type_descriptor(istr);
        auto [aggType, dims] = read_shape_descriptor(istr);

        // dispatch<TypeList> is generated at compile time
        // it contains an if-else chain (or jump table) for
        // exactly the types in TypeList
        bool matched = dispatch<TypeList>(typeId, params,
            [&](auto tag) {  // tag is a type_identity<T>
                using T = typename decltype(tag)::type;
                restore_collection<T>(istr, aggType, dims);
            });

        if (!matched) {
            report_unsupported_type(typeId, params);
            skip_payload(istr);
        }
        return matched;
    }
};
```

The `dispatch` function is a compile-time-generated chain that compares the
runtime type descriptor against each type in the TypeList:

```cpp
template<typename... Types>
bool dispatch(uint32_t typeId, uint32_t* params, auto&& handler) {
    return (try_match<Types>(typeId, params, handler) || ...);
}

template<typename T>
bool try_match(uint32_t typeId, uint32_t* params, auto&& handler) {
    uint32_t myId, myNrParams, myParams[16];
    generateScalarTypeId<T>(myId, myNrParams, myParams);
    if (myId == typeId && params_match(myNrParams, myParams, params)) {
        handler(std::type_identity<T>{});
        return true;
    }
    return false;
}
```

### Layer 4: Translation Utilities

To convert data files between client type sets:

```cpp
template<typename SourceTypes, typename TargetTypes>
class translator {
    // Read with source registry, convert through double, write with target types
    bool translate(std::istream& in, std::ostream& out,
                   const type_map& conversions);
};

// Usage: convert NVIDIA data to AMD format
type_map nvidia_to_amd = {
    {type_id<cfloat<19,8,...>>(), type_id<cfloat<16,8,...>>()},  // TF32 -> BF16
    {type_id<cfloat<8,4,...>>(), type_id<cfloat<8,4,...>>()}      // E4M3 -> E4M3 (same)
};
translator<NvidiaTypes, AmdTypes> t;
t.translate(nvidia_file, amd_file, nvidia_to_amd);
```

The translation goes through a common intermediate (double or qd) to
handle the value conversion. This loses precision for types beyond double,
but for DNN quantization types (which are all <= 32 bits) this is lossless.

## Comparison with Current Design

| Aspect | Current Design | Proposed Design |
|--------|---------------|-----------------|
| Type enumeration | Giant switch in `restoreCollection()` | Compile-time dispatch from client's type_list |
| Adding a type | Edit datafile.hpp, add case | Client adds type to its type_list |
| Compilation cost | All types always compiled | Only client's types compiled |
| Client coupling | Framework knows all types | Client declares its types |
| Translation | Not supported | Explicit type_map between registries |
| File format | Text with decimal/hex | Same format, but hex-only for bit-exact fidelity |

## What Needs to Change

### Phase 1: Core Infrastructure

**New files:**
- `include/sw/blas/serialization/type_registry.hpp` -- `type_list`, `dispatch`,
  `try_match`, `generateScalarTypeId` improvements
- `include/sw/blas/serialization/typed_datafile.hpp` -- new `typed_datafile<TypeList>`
  class replacing the current `datafile`

**Modified files:**
- `include/sw/blas/serialization/datafile.hpp` -- refactor `save()` to always use
  hex format for payload (bit-exact); extract format reading/writing into
  standalone functions; keep the class as a convenience wrapper
- `generateScalarTypeId()` -- add support for posit (v2), dd, qd, dd_cascade,
  td_cascade, qd_cascade, bfloat16, microfloat types

**Key design constraints:**
- `type_list` uses variadic templates, no virtual dispatch
- `dispatch` uses fold expressions (`(try_match<Types>(...) || ...)`)
- Factory functions return `std::unique_ptr<ICollection>` for type erasure
  at the collection level (not at the scalar level)

### Phase 2: Translation Layer

**New files:**
- `include/sw/blas/serialization/translator.hpp` -- `translator<Src, Dst>`
  with `type_map` for explicit conversions

**Key design constraint:**
- Translation through double is acceptable for types <= 64 bits
- For dd/qd, translation preserves the multi-component structure

### Phase 3: Test Infrastructure

**Modified files:**
- `linalg/data/serialization.cpp` -- rewrite using `typed_datafile<TestTypes>`
  with a specific test type list; add regression level 1 tests for save/restore
  round-trips; verify bit-exact fidelity using hex format

**New test scenarios:**
- Round-trip: save vector<T>, restore, compare bit-for-bit
- Type mismatch: save with type A, restore with registry lacking A -> graceful error
- Translation: save with NvidiaTypes, translate to AmdTypes, verify values

### Phase 4: CI Integration

- Enable `UNIVERSAL_BUILD_LINEAR_ALGEBRA_DATA` in CI or move serialization
  tests to a more standard location
- Add serialization to the regression test suite

## File Format Specification (v2)

```
UNIVERSAL_DATA_FILE_MAGIC_NUMBER (0xAAA0)
--- per dataset ---
TYPE_ID                         uint32 (e.g., 0x0303 for cfloat)
NR_PARAMETERS                   uint32
PARAMETERS                      uint32[NR_PARAMETERS]
# comment line                  string (human-readable type description)
AGGREGATE_TYPE                  uint32 (scalar/vector/matrix/tensor)
DIMENSIONS                      uint32[rank] (e.g., [rows, cols] for matrix)
DATA                            hex strings, 10 per line
DATASET_NAME                    string
--- end per dataset ---
0                               termination token
```

The only change from v1: payload is always hex (no decimal option for
production use). Decimal remains available for human-readable debug dumps
but is not the canonical format.

## Summary

The architecture shifts from "framework enumerates all types" to "client
declares its types." This is achieved through:

1. **type_list** -- compile-time type set declared by the client
2. **typed_datafile<TypeList>** -- parameterized by the client's types
3. **dispatch via fold expression** -- O(N) compile-time generated chain
   for N types in the client's list, O(0) for types not in the list
4. **translator** -- converts between type sets through a common intermediate

The file format stays backward-compatible. Only the restore path changes.
