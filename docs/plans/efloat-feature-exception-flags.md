# `efloat` Feature: Exception and Status Flags

*   **Related Epic**: [efloat-implementation-epic.md](efloat-implementation-epic.md)

## Problem Description

For rigorous numerical work, it's essential to know when an operation resulted in an exceptional event (e.g., overflow, underflow, producing an inexact result). MPFR and the IEEE 754 standard provide status flags for this purpose.

`efloat` currently has a compile-time switch to throw exceptions, but it lacks a standard-compliant status flag mechanism.

## Proposed Solution

1.  **Define Status Flags**: Create an `enum` or bitmask for the standard floating-point exceptions:
    *   `FLAG_INEXACT`
    *   `FLAG_UNDERFLOW`
    *   `FLAG_OVERFLOW`
    *   `FLAG_INVALID_OPERATION` (e.g., `0/0`, `sqrt(-1)`)
    *   `FLAG_DIV_BY_ZERO`

2.  **Integrate Flags into Arithmetic**: All arithmetic operations and math functions must be updated to set these flags when an exceptional event occurs. For example, any operation that requires rounding should set the `FLAG_INEXACT`.

3.  **Provide a Control Mechanism**: Implement a thread-local or global mechanism for a user to:
    *   **Query** the current status flags.
    *   **Clear** the flags.
    *   **Save/Restore** the full flag state.

This allows for a "sticky" flag model, where flags are accumulated until the user explicitly clears them.

## Acceptance Criteria

*   A status flag mechanism is implemented.
*   All `efloat` arithmetic and math functions correctly set the flags.
*   User-facing functions to query and clear flags are provided.
*   Tests are created to verify that exceptional operations (like `1.0 / 0.0` or an operation that requires rounding) correctly set the appropriate flags.
