# Posit C API Library

This directory contains the C API of the posit number system.

This is designed for C99 compliance but if you are building with a C11 or newer compiler, there
is an additional set of generic functions which will be turned on as well.

## Types

In general, all functions are available for the following types:

*   `posit4_t` - a 4 bit posit with 0 exponent bits (storage size is 1 byte)
*   `posit8_t` - an 8 bit posit with 0 exponent bits
*   `posit16_t` - a 16 bit posit with 1 exponent bit
*   `posit32_t` - a 32 bit posit with 2 exponent bits
*   `posit64_t` - a 64 bit posit with 3 exponent bits
*   `posit128_t` - a 128 bit posit with 4 exponent bits
*   `posit256_t` - a 256 bit posit with 5 exponent bits

Additionally, the following types exist which can be converted to and from posits:

*   `long double`: **ld**
*   `double`: **d**
*   `float`: **f**
*   `long long`: **sll**
*   `long`: **sl**
*   `int`: **si**
*   `unsigned long long`: **ull**
*   `unsigned long`: **ul**
*   `unsigned int`: **ui**

In this document, you will see descriptions of functions accepting non-posit type arguments
which will be named according to the type of argument they accept. For example, when you see
`posit8_toX()` described in the documentation as returning type `X`, you know that
`posit8_tosl()` will return a `long` while `posit8_told()` will return a `long double`.

Finally, `X` can also be a posit, where the shorthand is `p` followed by the nbits of the posit.
For example: `posit8_top32()` converts a posit8 type to a `posit32_t`.

## Creating posits

Posits can be created by either *converting* a number to a posit, or by reinterpreting the bits
in an unsigned integer or buffer as the bit representation of the posit.

### Generic conversion

The generic conversion API is quite simple, there is a converter macro for each posit type
which bears the name of the posit type, sans the _t. This function works with all of the
above types

```c
// convert an integer to a posit
posit32_t zero = posit32(0);

// convert a double to a posit16
posit16_t one_point_one = posit16(1.1);

// convert one posit type to another
posit8_t zeroEight = posit8(zero);
```

### C99 conversion

C99 conversion is similar to generic conversion but you must specify the type of which you are
converting from. In this case you will use `positN_fromX()` function.

```c
// convert an integer to a posit
posit32_t zero = posit32_fromsi(0);

// convert a double to a posit16
posit16_t one_point_one = posit16_fromd(1.1);

// convert one posit type to another
posit8_t zeroEight = posit8_fromp32(zero);
```

### Reinterpret

The reinterpret function is somewhat special in that it takes different arguments for different
posit types. The reinterpret functions are as follows:

```c
posit4_t   posit4_reinterpret(uint8_t n);
posit8_t   posit8_reinterpret(uint8_t n);
posit16_t  posit16_reinterpret(uint16_t n)
posit32_t  posit32_reinterpret(uint32_t n);
posit64_t  posit64_reinterpret(uint64_t n);
posit128_t posit128_reinterpret(uint64_t n[static 2])
posit256_t posit256_reinterpret(uint64_t n[static 4])
```

You can use reinterpret as follows:

```c
// 0x4000 is the binary representation of 1 in the posit16 format.
posit16_t one = posit16_reinterpret(0x4000);
```

## Converting posits into other things

Because there's no way to know what the expected return type is, conversion *from* posits
always requires the type to be specified.

### Conversion to numbers

Generic conversion allows you to leave out the posit type, you only need to specify
what you want to convert it to. The generic conversion function is `posit_toX()` while
the C99 conversion function is `positN_toX()`.

```c
// convert to double in order to print the number with printf with generic conversion
printf("The value is %f\n", posit_tod(p));

// C99 conversion is the same but you must specify the posit type
printf("The value is %f\n", posit32_tod(p));
```

### Debug string

You can convert the posit to a debug string using the `posit_str()` function. First you
must create a buffer which is big enough to hold the output, this is different for each
posit type but there is an enum of sizes called `positN_str_SIZE` so you can make the
buffer big enough.

```c
char out[posit64_str_SIZE];
posit_str(out, p);
printf("The value is %s\n", out);
```

This will result in a string looking like `64.3x1234567812345678p`, which is the posit
width followed by a dot followed by the posit exponent size, then an x, then the numeric
value and finally a p.

The C99 version of posit_str is the same except you must specify the posit type, for example:

```c
void posit32_str(char out[posit32_str_SIZE], posit32_t p);
```

### Getting the bits

For posits of sizes smaller than 128 bits, you can do the inverse of `positN_reinterpret()`
by calling `posit_bits()` or `positN_bits()`. These functions all return an unsigned number
of the relevant size (`posit4_bits()` returns a `uint8_t`).

```c
posit32_t p = posit32(123.456);
printf("The bit pattern is [%08x] for posit [%f]\n", posit_bits(p), posit_tod(p));

// C99, no generics:
posit32_t p = posit32_fromd(123.456);
printf("The bit pattern is [%08x] for posit [%f]\n", posit32_bits(p), posit32_tod(p));
```

To get the bits from a posit128 or posit256, consider using memcpy or a union type.

## Posit Math

Now for the fun part, this library has the 4 main functions (**add**, **sub**, **mul**, **div**)
as well as a comparison function and a number of single argument functions which we will
cover later.

Every 2-argument function has the following general form: `positN_opX(positN_t, X)` and
also has an inverse form: `positN_Xop(X, positN_t)`. For example, there is
`posit32_addf(posit32_t, float)` and there's also `posit32_fadd(float, posit32_t)`.

```c
// Adding a float to a posit32_t
posit32_t two = posit32(2);
posit32_t four = posit32_addf(two, 2.0f);

// Adding a posit32_t to a posit8_t (this becomes posit8 maxval)
posit32_t two_hundred = posit32(200);
posit8_t one = posit8(1);
posit8_t p8max = posit8_addp32(one, two_hundred);

// But this will work...
posit32_t two_hundred_one = posit32_addp8(two_hundred, one);
```

The generic form of these functions makes this much easier, but beware when you're doing
an operation on posits of different types that you don't end up doing the wrong conversion.
The generic form first takes the left argument, if the left argument is a posit then it will
output that posit type, if it's not a posit then it will output the posit of the righthand
argument.

```c
// Generic adding a float to a posit32_t
posit32_t two = posit32(2);
posit32_t four = posit_add(two, 2.0f);

// Adding a posit32_t to a posit8_t (this becomes posit8 maxval)
posit32_t two_hundred = posit32(200);
posit8_t one = posit8(1);
posit8_t p8max = posit_add(one, two_hundred);

// But this will work...
posit32_t two_hundred_one = posit_add(two_hundred, one);

// Or this...
posit32_t also_two_hundred_one = posit_add(posit32(one), two_hundred);
```

### Posit comparison

Posit comparison is similar to 2 argument posit math, but instead of returning a posit, it
returns an integer which is negative if the first argument is less than the second argument,
positive if the second argument is less than the first argument and zero is they're the same.

The definition of the comparison function is:

```c
int positN_cmpX(positN_t, X);
int positN_Xcmp(X, positN_t);
```

But as with others, the generic form is simply `posit_cmp()` and it will use the same logic
as with the math functions to determine which posit type should be used (if 2 different size
posits are being compared).

```c
posit32_t two_hundred = posit32(200);
posit8_t p8max = posit8(128);

if (posit_cmp(two_hundred, p8max) > 0) { // two_hundred > p8max
    printf("two_hundred is bigger than p8max\n");
}
if (posit_cmp(p8max, two_hundred) == 0) {
    printf("be careful, this comparison converts two_hundred to a posit8 first so it's true!\n");
}
if (posit32_p8cmp(p8max, two_hundred) < 0) {
    printf("This non-generic form does the comparison with posit32s so it's correct\n");
}

```

## Single argument math functions

Ths library supports the following functions which require only a single argument:

*   `positN_sqrt()` Return the square root of a posit, as the same size posit
*   `positN_log()` Return the natural logarithm as a posit of the same size (same as math.h `log()`)
*   `positN_exp()` Returns the base-e exponential function of x (same as math.h `exp()`)

## Bugs and cautions

*   Conversions between posits is currently done by converting to a double and back, see: https://github.com/stillwater-sc/universal/issues/90
*   `positN_sqrt()` `positN_log()` and `positN_exp()` use math.h implementations with double type, see https://github.com/stillwater-sc/universal/issues/9 for more information.
