# quire as an arithmetic type

The quire, or Kulisch Super-accumulator, is a fixed-point accumulator used to implement 
the fused dot product operator. It can receive arithmetic values, unrounded products,
and implements arithmetic operators:

    quire += value
    quire -= value
    convert to native arithmetic
    sqrt

and assignment operators
    quire = value

The quire can be loaded and stored error free.

The size of the quire to support FDP operators is proportional to the dynamic range
between the smallest and largest product for the arithmetic supported. For posits,
this dynamic range can be controlled more finely than for IEEE-754, and thus the
quire size for posits is more reasonable than for IEEE-754.

The intent of this directory is to elevate the quire to a reusable super-accumulator
across different number systems. This may or may not be reasonable as we do not
have operational experience yet with quires across different number systems.

The arithmetic types for which the FDP operator would be attractive are:
  - fixpnt
  - cfloat
  - posit
  - lns
  - dblns or lns2b

These are all types that have very different parameterizations, and it would be
attractive to simplify the usage of the quire to adapt to the supporting arithmetic,
as demonstrated in the quire for posits, where the quire<> takes the same
parameterization as the supporting posit.

Right now we have a specialized quire in the posit number system, and we have
a prototype quire for IEEE-754 floating-point in <universal/number/float>

We need to add the cfloat and lns types to see what interface emerges that
could become a standalone quire number type.
