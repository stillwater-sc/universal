# ucalc Double Interchange Analysis

## Problem Statement

ucalc's type dispatch layer uses `double` as the interchange format between
type-erased operations. The `Value` struct stores every result as `Value.num`
(a double), and all cross-type operations, display comparisons, and error
computations flow through this double field. The `std::any native` field
preserves the full-precision type value for same-type arithmetic, but any
operation that reads `Value.num` sees only double precision.

This works for types with <= 52 bits of significand precision (which is most
of the 42+ registered types). But for types with more precision than double,
the interchange silently destroys information the type actually carried.

## Types Affected by Double-Interchange Round-off

### Confirmed Lossy (significand > 52 bits)

| Type | Sig bits | Decimal digits | Precision lost | Notes |
|------|----------|----------------|---------------|-------|
| posit64 | 59 | ~17.8 | ~6 bits near useed | Tapered: only lossy in the sweet spot |
| fp128 | 112 | ~33.8 | ~60 bits | Entire extended fraction lost |
| dd | 105 | ~31.6 | ~53 bits | Entire second limb lost |
| dd_cascade | 104 | ~31.3 | ~52 bits | Entire second limb lost |
| td_cascade | 158 | ~47.6 | ~106 bits | Two of three limbs lost |
| qd | 209 | ~63.0 | ~157 bits | Three of four limbs lost |
| qd_cascade | 209 | ~63.0 | ~157 bits | Three of four limbs lost |

### Edge Cases (lossy in specific scenarios)

| Type | Issue |
|------|-------|
| int64 | Integers > 2^53 (~9e15) lose low bits. 9007199254740993 displays as 9007199254740992. |
| decimal64 | 49 binary digits (~15 decimal). Right at double's boundary -- some 16th-digit values round-trip incorrectly. |
| rational32 | 30 binary digits fits in double, but the exact fraction semantics (1/3 is exact in rational, not in double) are lost. |
| dfixpnt16_8 | 26 bits, safe now. Larger future configs would cross the threshold. |

### Safe (significand <= 52 bits)

float, double, posit8-32, bfloat16, fp16, fp32, fp64, all FP8 variants,
fixpnt16/32, lns8-32, int8-32, takum8-32, hfloat32/64, decimal32,
rational8-16.

### Adaptive Types (not yet registered)

| Type | Max precision | Status |
|------|--------------|--------|
| ereal<8> | ~127 decimal digits (8 double limbs) | Would lose 110+ digits |
| efloat<1024> | Thousands of digits | Would lose nearly everything |
| einteger | Arbitrary precision integer | Would lose all bits above 2^53 |
| edecimal | Arbitrary precision decimal | Would lose all digits beyond 16 |
| erational | Arbitrary precision rational | Would lose exact fraction semantics |

## What Breaks

The double interchange affects these operations:

1. **Display**: `Value.num` printed via `std::setprecision` shows double-precision
   digits, not the type's full precision. Mitigated by `native_rep` (from
   `operator<<`) and `native_enc` (from `to_native()`), which use the type's
   own formatter.

2. **Error computation**: `trace`, `audit`, `cancel` compute ULP error as
   `|result.num - reference.num|`. For types wider than double, both sides
   are truncated to double, so the error metric is meaningless.

3. **Cross-type comparison**: `compare` shows `Value.num` for all types.
   Wide types show only 17 significant digits even though they carry more.

4. **Reference replay**: `trace`/`audit` replay operations in qd via
   `ref_ops->from_double(operand.num)`. For dd/qd operands, this truncates
   the operand to double before the qd replay, defeating the reference.

5. **Quantization**: `quantize` reads data as doubles from CSV. This is
   acceptable since the input data is double-precision anyway.

### What Still Works Correctly

- **Same-type arithmetic**: `extract<T>(value)` pulls from `std::any native`,
  preserving full precision. All arithmetic ops (add, sub, mul, div, functions)
  operate at full type precision.
- **Native display**: `native_rep` and `native_enc` use the type's own
  `operator<<` and `to_native()`, showing full precision.
- **Type properties**: `range`, `precision`, `epsilon` query `numeric_limits`
  directly, not through the double interchange.

## Design Options for Supporting Wide and Adaptive Types

### Design A: Register as Static Configurations, Accept Limitations

Register adaptive types (ereal<2>, ereal<4>, ereal<8>) as regular types via
`register_type<T>`. The `std::any native` field preserves full precision for
same-type arithmetic. Commands that display native_rep show full precision.
Commands that compute error metrics give meaningless results.

**Pros**: Minimal effort. Most commands work. Arithmetic is correct.
**Cons**: Error analysis commands (trace, audit, cancel) give wrong results.
No honest way to compare adaptive types against static types.
**Effort**: Small (add registry entries only).

### Design B: Dual-Path Value with Adaptive Awareness

Extend `Value` with a `std::string precise_rep` field storing the full-precision
decimal string from `operator<<`. Add a `TypeOps::is_adaptive` (or
`is_wide_precision`) flag. Commands branch on this flag:

- Inspection commands (`show`, `compare`, `types`, `range`, `precision`):
  use `precise_rep` for display. Work at full precision.
- Arithmetic: unchanged (already uses `std::any native` for same-type).
- Error analysis (`trace`, `audit`, `cancel`): skip ULP computation for
  wide types, show "N/A" or compute error via string comparison.
  Alternatively, compute error using the type's own arithmetic
  (subtract result from reference in the type itself).
- Quantization (`quantize`, `clip`): work as-is since input is double anyway.
- `increment`/`decrement`: work via `TypeOps::next`/`prev` which use
  `std::any native`, unaffected by double interchange.

**Key insight**: The double interchange is only needed for two purposes:
(1) cross-type numeric comparison, and (2) ULP/error computation. For
adaptive types, cross-type comparison is inherently imprecise (you're
comparing types with different precision semantics), and ULP is undefined.
So the double field can be treated as a "best-effort approximation" rather
than the authoritative value.

**Pros**: Clean separation. Existing static-type path unchanged. Adaptive
types get honest behavior. Most commands work.
**Cons**: Some commands show "N/A" for wide types. String-based comparison
is slower but only used for display.
**Effort**: Medium (add flag, precise_rep, conditional branches in ~5 commands).

### Design C: Native Arithmetic Engine (No Double Interchange)

Replace `double num` entirely with type-erased arithmetic. `Value` stores only
`std::any native` and string representations. All operations dispatch through
`TypeOps` function pointers operating on `std::any` values. Cross-type operations
convert through a canonical high-precision intermediary (ereal or string-based).

**Pros**: Architecturally correct. All types are first-class. No precision loss.
**Cons**: Near-complete rewrite of type dispatch and all 20+ commands. Performance
regression for narrow types. High risk of regression in existing functionality.
**Effort**: Large (months of work, ucalc v2).

## Recommendation

**Design B** is the pragmatic choice. It adds wide/adaptive types with honest
behavior while preserving the existing static-type path. The `is_wide_precision`
flag and `precise_rep` field provide the migration path to Design C if needed
later.

The 7 currently affected types (posit64, fp128, dd, dd/td/qd_cascade, qd) would
also benefit from Design B's `precise_rep` field -- their native_rep already
shows full precision, but error metrics would be flagged as approximate.

## Implementation Sequence

1. Add `precise_rep` to `Value`, populated from `native_rep` for all types
2. Add `is_wide` flag to `TypeOps` (true when `max_digits10 > 17`)
3. Show `precise_rep` in `compare` output for wide types
4. Flag error metrics as approximate in `trace`/`audit` for wide types
5. Register ereal<2>, ereal<4>, ereal<8> and efloat<64>, efloat<256>
6. Test: verify ereal arithmetic preserves full precision through `std::any`


## Current assessment

Types Susceptible to Double-Interchange Round-off

Confirmed Lossy (significand > 52 bits)


|    Type    | Significand bits | Decimal digits |        Precision lost         |
|------------|------------------|----------------|-------------------------------|
| posit64    | 59               | ~17.8          | ~6 bits near useed            |
| fp128      | 112              | ~33.8          | ~60 bits                      |
| dd         | 105              | ~31.6          | ~53 bits (entire second limb) |
| dd_cascade | 104              | ~31.3          | ~52 bits                      |
| td_cascade | 158              | ~47.6          | ~106 bits                     |
| qd         | 209              | ~63.0          | ~157 bits                     |
| qd_cascade | 209              | ~63.0          | ~157 bits                     |


### Edge Cases (lossy in specific scenarios)


|    Type     |                           Issue                        |
|-------------|--------------------------------------------------------|
| int64       | Integers > 2^53 (~9e15) lose low bits. Value 9007199254740993 displays as 9007199254740992. |
| decimal64   | 49 binary digits (~15 decimal). Right at double's boundary -- some 16th-digit values round-trip incorrectly. |
| rational32  | 30 binary digits. Safe for the numeric value, but the exact fraction semantics (1/3 is exact in rational, not in double) are lost in the interchange. |
| dfixpnt16_8 | 26 binary digits. Safe, but if larger dfixpnt configs are added, they'd cross the threshold. |


### Safe (significand <= 52 bits)

Everything else: float, posit8-32, bfloat16, fp16, fp32, fp64, all FP8 variants, fixpnt16/32, lns8-32, int8-32, takum8-32, hfloat32, decimal32, rational8-16.

### Impact

The 7 confirmed lossy types (posit64, fp128, dd, dd_cascade, td_cascade, qd, qd_cascade) are the same types that would need Design B or C to work correctly. Same set that adaptive types (ereal, efloat) would join. So the architectural limitation already affects 7 of the 42 registered types today.
