# Conversion of floating-point values to decimal representation in strings

The Dragon and Grisu algorithms are robust conversion algorithms to transform 
floating-point values into strings of decimal digit representations required for human interpretation.

There are many different floating-point formats available in Universal, and 
many of these floating-point representations, such as fixpnt, lns, dbns, posit, takum, dd, qd, and priest
are arbitrary precision representations and thus very accurate configurations
cannot rely on the standard library for conversion by marshalling the values through 
the native types, float/double/long double.

We need a reusable decimal converter for floating-point types.

There are two triple formats in the Universal library:
 1. value<>: this is a (sign, scale, fraction without hidden bit) where the fraction is managed as a bit vector
 2. blocktriple<>: this is a (sign, scale, significant) triple where the significant is managed as a multi-limb set

We want to create a decimal converter that takes a triple and converts it to a decimal representation
under the direction of the standard library ioflags.
