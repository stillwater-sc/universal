# Naming of number systems

We have fixed and adaptive precision number systems. 

And with generic programming, we have parameterized number systems that can be arbitrary configurations.

* Arbitrary precision, arbitrary dynamic range, arbitrary configuration is produced by generic programming
* Fixed and adaptive precision is produced by the algorithmic implementation of the arithmetic.

A fixed configuration implementation would specify specific attributes, such as size of representation,
size of exponent fields, etc. An adaptive number system might not need these parameters. But it might 
specify upper bounds, as for example in unum Type 1.

Arbitrary configuration is thus associated with the template parameterization, and it could yield
fixed and adaptive implementations.

Is there a naming scheme that makes this intuitively clear?

GNU MP is using the multiprecision label to describe adaptive precision. This collides with the use
of multi-precision algorithms that mix different number systems. Aha, that could be the term to 
disambiguate adaptive and multiprecision in algorithms that 'mix' precisions.

Multiprecision number systems in the form of GNU MP, MPFR, and Boost::multiprecision

Mixed precision algorithms in Krylov, eigenvalue, optimization.

Adaptive precision number systems in Universal
Arbitrary precision number systems that are fixed in size.

arbitrary, adaptive, mixed

mixed precision pertains to the algorithm level of abstraction
fixed, arbitrary, and adaptive precision pertain to the number system level of abstraction
arbitrary precision and adaptive precision can both have fixed precision configurations.
For adaptive precision such fixed precision would be upper bounds.

Is adaptive precision more descriptive than multi-precision? I think adaptive is less ambiguous than multiprecision.

How do we name linear floating-point number systems that disambiguate between arbitrary and adaptive precision?

* arbitrary precision linear floating point: areal<nbits, es>
* adaptive precision linear floating point : afloat
* arbitrary precision integer: integer<nbits>
* adaptive precision integer : decimal, aint
