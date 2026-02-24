# UNUM2: Configurable Number Lattice

## Why

Every number system makes a fixed choice about which values are representable: IEEE-754 uses powers of 2, posits use a tapered regime, fixed-point uses uniform spacing. But what if your application has a natural set of exact values that don't align with any of these? Musical frequencies follow the harmonic series, financial instruments have standard lot sizes, physical constants have known exact values.

The `unum2` lattice type lets you define *exactly which values are representable*. You specify a list of exact values, and the encoding automatically provides interpolation between them, negation, reciprocals, zero, and infinity. This is the generalization of Gustafson's UNUM framework: instead of prescribing a fixed encoding scheme, you design the number system around your problem's natural value set.

## What

`lattice<exacts...>` is a configurable number lattice:

| Parameter | Type | Description |
|-----------|------|-------------|
| `exacts...` | `int...` | Variadic list of exact representable values |

### Encoding

The lattice maps bit patterns to values in a structured way:
- Position 0 = zero
- Position N/2 = infinity
- Positions 0 to N/4 = reciprocals of exact values
- Positions N/4 to N/2 = exact values (and intervals between them)
- Positions N/2 to N = negative reflections

Non-exact bit patterns represent intervals between adjacent exact values.

### Key Properties

- **User-defined exact values**: you choose which numbers are represented exactly
- **Automatic reciprocals**: if x is exact, 1/x is also exact
- **Automatic negation**: if x is exact, -x is also exact
- **Interval semantics**: non-exact encodings represent ranges between exact values
- **Power-of-2 size**: total encoding space = 8 × number of exact values
- **Research/exploratory**: designed for investigating custom number spaces

## How It Works

Given a list of exact values, the lattice constructs a complete number line:
1. The positive exact values are placed in order
2. Their reciprocals fill the range between 0 and 1
3. Intervals between exact values get their own bit patterns
4. The negative half mirrors the positive half
5. Zero and infinity get dedicated encodings

For example, `lattice<1, 2, 3, 5, 8, 13>` (Fibonacci numbers) creates a number system where 1, 2, 3, 5, 8, 13 and their reciprocals are exact, with interval encodings for values between them.

## How to Use It

### Include

```cpp
#include <universal/number/unum2/unum2.hpp>
using namespace sw::universal;
```

### Custom Lattice

```cpp
// Define a lattice with Fibonacci exact values
lattice<1, 2, 3, 5, 8, 13> fib_lattice;

// Enumerate all representable values/intervals
for (unsigned i = 0; i < fib_lattice.size(); ++i) {
    fib_lattice.setbits(i);
    std::cout << "encoding " << i << ": " << fib_lattice << std::endl;
}
```

### Domain-Specific Number System

```cpp
// Musical frequency lattice: exact representation for harmonic series
lattice<1, 2, 3, 4, 5, 6, 7, 8> harmonic;

// Power-of-10 lattice: exact decades
lattice<1, 10, 100, 1000> decades;
```

## Problems It Solves

| Problem | How lattice Solves It |
|---------|-----------------------|
| Standard number systems don't match your problem's natural values | Define exactly which values are representable |
| Need exact representation of domain-specific constants | Place those constants as lattice points |
| Exploring trade-offs in custom number system design | Rapid prototyping of arbitrary value distributions |
| Research in UNUM framework beyond posits | Generalized lattice approach to number representation |
| Teaching number system design concepts | Concrete, configurable example of encoding trade-offs |
