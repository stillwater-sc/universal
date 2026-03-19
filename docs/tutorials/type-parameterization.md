# Universal Number System Type Parameterization

When we look at the number system types in _Universal_ next to each other, it should
become apparant how the _Universal_ library design has categorized the attributes
that define a number system.

These are all _static_ arithmetic types, which implies that they are fixed-size
during their life time.

```cpp
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType> class integer;
template<unsigned nbits, unsigned rbits, bool Arithmetic, typename BlockType> class fixpnt;
template<unsigned nbits, unsigned es, typename BlockType, bool hasSubnormals, bool hasMaxExpValues, bool isSaturating> class cfloat;
template<unsigned nbits, unsigned es, typename BlockType> class posit;
template<unsigned nbits, unsigned rbits, typename BlockType, auto... xtra> class lns;
```

All types are first defined by the total number of bits of their encoding. This combined
with the parameter type **BlockType** provides all the attributes that would define
the memory layout of a composit data structure, such as a vector, matrix, or tensor.

```text
The BlockType defines the type of the limbs that make up the storage
format of the arithmetic type, and declares the memory address alignment 
of its types. Each value is composed of the minimum number of limbs that will contain nbits.
```

**fixpnt** types need a parameter to define their radix point, which
in the _Universal_ design is defined by the non-type parameter **rbits**. For example, a **fixpnt<8,4>** will consists of 8 total
bits, divided in 4 integer bits, and 4 fraction bits. Fixed-point
types are encoded in 2's complement. Similarly, **lns** types are logarithmic types with a fixed-point as exponent. We are using the same parameterization of the non-type parameter **rbits** to capture the radix point of the exponent. Thus, a **lns<8,4>**, represents an 8-bit logarithmic number system with 1 sign bit, and a 7-bit fixed-point exponent with 4 fraction bits.

When we progress from integer and fixed-point types into floating-point types, we need to encode additional fields. In the classic floating-point definition this is encoded by the non-type parameter **es**, short for exponent bits. For example, a traditional, single precision IEEE-754 floating-point would be articulated as **cfloat<32, 8, std::uint32_t, hasSubnormals>**. We have a total of 32 bits for the encoding, and an 8-bit exponent field. The sign bit is implicit, and the number of mantissa bits is derived: 

    fbits = nbits - 1 - es

and assumes that there is a hidden bit as this is a binary floating-point format. There are special ranges in a classic floating-point that require special handling. In IEEE-754 we have the subnormal range defined by the encodings that have all their exponent bits set to 0. In _Universal_ we also offer a max-exponent value range defined by the encodings that have all their exponent bits set. Finally, a floating-point arithmetic can clip to +- infinity, or it can saturate to the largest possible value, and that can be controlled by the last non-type parameter, _isSaturating_.

The final arithmetic type is a tapered floating-point called **posit**. Posits are encoded using four fields, a sign bit, a regime, an exponent, and a mantissa field, but the last three fields are dynamic. By using a very clever encoding, a **posit** can be specified with just its size and the maximum size of the exponent field. The other fields are derived and dynamic. For example, a **posit<32,2>** defines a single precision standard **posit** of 32-bits, with maximally 2 exponent bits. The size of the exponent field defines an internal regime exponential.

All of the number systems try to follow this basic pattern: the defition of the total number of bits, followed by additional non-type parameters that define special fields or arithmetic behavior.