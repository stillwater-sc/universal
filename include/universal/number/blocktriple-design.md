# blocktriple

The blocktriple is a parameterized (sign, scale, significant) triple to represent
floating-point values.

A blocktriple is composed of the three information segments, plus decoded state
variables that label special state, such as, NaN, Inf, or Zero.

    sign         int exp       blockfraction
    nan/inf/zero

The goal is to have an efficient representation of the fraction bits to serve 
as the ready-to-use inputs to add/sub/mul/div/sqrt
Let's call this representation: blockfraction, to indicate that it is implemented 
using a scalable block segmented representation and associated algorithms

blockfraction API needs to be able to 
  - efficiently receive bits from source number systems: areal, cfloat, posit, lns    <---- ESSENTIAL for performance
      special convert method for add/sub
      special convert method for mul
      special convert method for div
      special convert method for sqrt
  - expand and normalize to align operands for add/sub
	for correct rounding, you need 3 additional bits: round, guard, and sticky bit
        for add/sub, the alignment can create non-zero bits in round|guard|sticky,
        and this implies that you need to expand depending on the operation, 
	and at the time of assignment from src to blockfraction
  - right shift to normalize results where the radix has shifted (1.0 + 1.0 = 2.0 = b10.00000)
  - for add/sub use a 2's complement, for mul use signed-magnitude, what is best for div and sqrt?
  - for add/sub, need an extra bit to encode largest negative result (-1.0 + 1.0 = -2.0 = b110.00000


When generating test cases for triples, we have sign, exponent, and fraction bit enumerations.
When the number is negative, and we need to add, the ALU will benefit from being in 2's complement mode.
Is there a form in which the triple is in tuple form (exp, 2's complement fraction)?
How do you normalize a 2's complement number? 
   if positive, simply shift. 
   if negative, first take 2's complement, then shift.

1.110011 = 1 + 0.5 + 0.25 + 0.03125 + 0.015625 = 1.796875
(-1, 5, 1.110011)  -1 * 2^5 * 1.796875 =  -57.5
( 1, 7, 1.110011)   1 * 2^7 * 1.796875 =  230.0


```
sign for addition is dependent on the quadrant
       y
 - +   |   + +
-------0------- x
 - -   |   + -


 +-1
-1 = (1, 0, 01.000)
+1 = (0, 0, 01.000)

------------------------------------------------  ADDITION  ---------------------------------------------------------

------------------ QUADRANT: Left-Upper -------------------
-1 + +1                      if fractions are the same and signs are different -> zero is result
(1, 0, 001.000) -> 2's complement (1, 0,   111.000)
(0, 0, 001.000)                   (0, 0,   001.000)
                                  (0, 0, 1|000.000) -> ignore carry, MSB == false -> positve = true, interpret special case of 0 by checking input arguments
                                  (0, 0,   000.000) -> +0
-1 + -1
(1, 0, 001.000) -> 2's complement (1, 0,   111.000)
(1, 0, 001.000) -> 2's complement (1, 0,   111.000)
                                  (1, 0, 1|110.000) -> ignore carry, MSB == true -> negative -> 2's complement
                                  (1, 0,   010.000) -> MSB-1 == true -> normalize
                                  (1, 1,   001.000) -> -2.0
-1.25 + +1.5 
(1, 0, 001.010) -> 2's complement (1, 0,   110.110)
(0, 0, 001.100)                   (0, 0,   001.100)
                                  (0, 0, 1|000.010) -> ignore carry, MSB == false -> positive = true
                                  (0, 0,   000.010) -> +0.25
-1.125 + +1 
(1, 0, 001.001) -> 2's complement (1, 0,   110.111)
(0, 0, 001.000)                   (0, 0,   001.000)
                                  (1, 0,   111.111) -> MSB == true -> negative = true -> 2's complement (000.001)
                                  (1, 0,   000.001) -> -0.125
-1.5 + +1.25 
(1, 0, 001.100) -> 2's complement (1, 0,   110.100)
(0, 0, 001.010)                   (0, 0,   001.010)
                                  (1, 0,   111.110) -> MSB == true -> negative = true -> 2's complement (000.010)
                                  (1, 0,   000.010) -> -0.25

------------------ QUADRANT: Right-Lower -------------------
+1 + -1                      if fractions are the same and signs are different -> zero is result
(0, 0, 001.000)                   (0, 0,   001.000)
(1, 0, 001.000) -> 2's complement (1, 0,   111.000)
                                  (0, 0, 1|000.000) -> ignore carry, MSB == false -> positive = true, interpret special case of 0
                                  (0, 0,   000.000) -> +0
+1.5 + -1.25 
(0, 0, 001.100)                   (0, 0,   001.100)
(1, 0, 001.010) -> 2's complement (1, 0,   110.110)
                                  (0, 0, 1|000.010) -> ignore carry, MSB == false -> positive = true
                                  (0, 0,   000.010) -> +0.25
+1 + -1.125 
(0, 0, 001.000)                   (0, 0,   001.000)
(1, 0, 001.001) -> 2's complement (-, 0,   110.111)
                                  (?, 0,   111.111) -> MSB == true -> negative = true -> 2's complement (000.001)
                                  (1, 0,   000.001) -> -0.125

------------------ QUADRANT: Right-Upper -------------------
+1 + +1 : value is always positive -> sign = 0
(0, 0, 001.000) ->                (0, 0,   001.000)
(0, 0, 001.000)                   (0, 0,   001.000)
                                  (0, 0,   010.000) -> MSB-1 == true -> normalize -> (0, 1, 001.000)
                                  (0, 1,   001.000) -> +2

------------------ QUADRANT: Left-Lower -------------------
-1 + -1 : value is always negative -> sign = 1
(1, 0, 001.000)                   (1, 0,   001.000)
(1, 0, 001.000)                   (1, 0,   001.000)
                                  (1, 0,   010.000) -> MSB-1 == true -> normalize -> (1, 1, 001.000)
                                  (1, 1,   001.000) -> -2.0


align arguments
+0.25 + +1.0 : value always positive -> sign = 0 
(0, -2, 001.000) -> shift right 2 (0, 0,   000.010)
(0,  0, 001.000)                  (0, 0,   001.000)
                                  (0, 0,   001.010) -> 1.25 

+0.25 + +1.75 : value always positive -> sign = 0 
(0, -2, 001.000) -> shift right 2 (0, 0,   000.010)
(0,  0, 001.110)                  (0, 0,   001.110)
                                  (0, 0,   010.000) -> MSB == true -> normalize -> (0, 1, 01.000)
                                  (0, 1,   001.000) -> 2.0 


------------------------------------------------  SUBTRACTION ---------------------------------------------------------
sign for subtraction is dependent on the quadrant
       y
 - -   |   + -            ( - - ) means that the result is always negative
-------0------- x         ( + - ) means the result can be positive or negative depending on the magnitude of the values
 - +   |   + +

------------------ QUADRANT: Left-Upper -------------------
-1 - +1  : value always negative -> sign = 1 
(1, 0, 001.000)                   (1, 0,   001.000)
(0, 0, 001.000)                   (0, 0,   001.000)+
                                  (?, 0,   010.000) -> MSB == true -> normalize -> (0, 1, 01.000)
                                  (1, 0,   001.000) -> -2.0
-1.125 - +1 
(1, 0, 001.001)                   (-, 0,   001.001)
(0, 0, 001.000)                   (0, 0,   001.000)+
                                  (?, 0,   010.001) -> MSB == true -> normalize -> (0, 1, 01.000)
                                  (1, 0,   001.000) -> -2.0  rounding -2.125 to even -2.0: [-2.0 | -2.125 | -2.25]

------------------ QUADRANT: Right-Lower -------------------
+1 - -1 : value is always positive -> sign = 0
(0, 0, 001.000)                   (0, 0,   001.000)
(1, 0, 001.000) -> -- becomes +   (0, 0,   001.000)+
                                  (0, 0,   010.000) -> MSB == true -> normalize -> (0, 1, 01.000)
                                  (0, 1,   001.000) -> +2
+1 - -1.125 
(0, 0, 001.000)                   (0, 0,   001.000)
(1, 0, 001.001) -> -- becomes +   (0, 0,   001.001)+
                                  (0, 0,   010.001) -> MSB == true -> normalize -> (0, 1, 01.000)
                                  (0, 1,   001.000) -> +2          rounding 2.125 to even 2.0: [2.0 - 2.125 - 2.25]
+1 - -1.25 
(0, 0, 001.000)                   (0, 0,   001.000)
(1, 0, 001.010) -> -- becomes +   (0, 0,   001.010)+
                                  (0, 0,   010.010) -> MSB == true -> normalize -> (0, 1, 01.001)
                                  (0, 1,   001.001) -> +2.25

------------------ QUADRANT: Right-Upper -------------------
+1 - +1 = +1 + -1            if fractions are the same and signs are the same -> zero is result
(0, 0, 001.000)                   (0, 0,   001.000)
(0, 0, 001.000) -> 2's complement (1, 0,   111.000)+
                                  (0, 0, 1|000.000) -> ignore carry, special case zero
                                  (0, 0,   000.000) -> 0

+1 - +1.5 = +1 + -1.5      
(0, 0, 001.000)                   (0, 0,   001.000)
(0, 0, 001.100) -> 2's complement (0, 0,   110.100)+
                                  (0, 0,   111.100) -> MSB == true -> negative -> 2's complement (000.100)  
                                  (1, 0,   000.100) -> MSB =@ radix-1 -> subnormal normalize -> (1, -1, 001.000)
                                  (1, -1,  001.000) -> -0.5

------------------ QUADRANT: Left-Lower -------------------
-1 - -1 = -1 + +1            if fractions are the same and signs are the same -> zero is result
(1, 0, 001.000) -> 2's complement (1, 0,   111.000)
(1, 0, 001.000)                   (0, 0,   001.000)+
                                  (?, 0, 1|000.000) -> ignore carry, special case zero
                                  (0, 0,   000.000) -> 0
-1 - -1.5 = -1 + +1.5
(1, 0, 001.000) -> 2's complement (1, 0,   111.000)
(1, 0, 001.100)                   (0, 0,   001.100)+
                                  (0, 0, 1|000.100) -> ignore carry, MSB == false -> sign = 0
                                  (0, 0,   000.100) -> 0.5

------------------------------------------------  MULTIPLICATION ---------------------------------------------------------
for multiplication, we can directly operate on the fraction bits without having to change encodings based on sign

+4 * +1.875 = 7.5
(0, 2, 01.000)                            01.000  *
(0, 0, 01.111)                         0.001+000  1
                                       0.010+00-  1
                                       0.100+0--  1
                                                  .
                                      01.000+---  1
                                     000.000+---  0  <--- this summation can be removed as it will always be 0
                                    -------------
                                       1.111+000  -> (0, 2, 01.111) -> 7.5

+1.875 * +1.875 = 3.515625
(0, 0, 01.111)                            01.111  *
(0, 0, 01.111)                         0.001+111  1
                                       0.011+11-  1
                                       0.111+1--  1
                                                  .
                                      01.111+---  1
                                    -------------
                                      11.100+001  -> MSB == 1 -> normalize -> (0, 1, 01.110) -> 3.5

The radix point moves to double the fraction bits.

```
------------------------------------------------  DIVISION ---------------------------------------------------------

```
operator+=()
  // test special cases

  // arithmetic operation
	internal::value<abits + 1> sum;
	internal::value<fbits> a, b;
  // transform the inputs into (sign,scale,fraction) triples
	normalize(a);             // copy of fraction bits into triple<>
	rhs.normalize(b);         // copy of fraction bits into triple<>
	module_add(a, b, sum);    // add the two inputs <--- inside another copy (expansion, alignment) of fraction bits into the arithmetic form
  // handle test special cases
  // convert back from triple to src number encoding
  convert(*this);
  return *this 
}

operator*=()
  // test special cases

  // arithmetic operation
	internal::value<mbits> product;
	internal::value<fbits> a, b;
  // transform the inputs into (sign,scale,fraction) triples
	normalize(a);       // copy of fraction bits into triple<>
	rhs.normalize(b);   // copy of fraction bits into triple<>
	module_multiply(a, b, product);    // multiply the two inputs
  // handle test special cases
  // convert back from triple to src number encoding
  convert(*this);
  return *this 
}

```

LIBRARY ARCHITECTURE


     +---------------+             +---------------+            +---------------+             +---------------+           
     |     cfloat    |             |      areal    |            |      posit    |             |       lns     |
     +---------------+             +---------------+            +---------------+             +---------------+  
       bit encoding

       operator=+() {
        // static objects on stack
	// so no allocation overhead
        blocktriple<abits + 1> sum; // unrounded
        blocktriple<abits> a,b;
	// number system specific bit decode
	// yield a blockfraction that is copied, 
	// expanded, and normalized to 01.fffffaaa
	// you need the form 01. as the computation may need to take a 2's complement
        alu_add<abits>(a);  
        rhs.alu_add<abits>(b);
        uradd(a,b,sum); // unrounded add
        // handle nan/inf/zero
        return convert(sum)  // round and copy bits back into native encoding
       }
       
  convert is a free function converting number system encodings into blocktriples and back
   semantic equivalent to convert_to, that is, convert src to tgt
  convert( const NumberSystem<nbits>& src, blocktriple<fbits>& tgt) {}
  convert( const blocktriple<fbits>&  src, NumberSystem<nbits>& tgt) {}
because it represents NumberSystem knowledge about the encoding, convert() is defined along side the number system class


                             +----------------------------------------------------------+
                             |                        blocktriple                       |      blocktriple is independent of number system
                             | sign | scale |            blockfraction                  |      blocktriple is different for different use cases
                             +----------------------------------------------------------+

                                ADD             SUB              MUL               DIV


	// input to add/sub : 01.ffffaaa   with expansion and encoded in 2's complement fixed-point
	// input to mul     :  1.ffffmmmm  with expansion and hidden bit materialized
	// input to div     :  1.ffffddddd with expansion and hidden bit materialized
	// input to sqrt    :  1.ffffsss   with expension and hidden bit materialized
        // representation   :   .ffff      without the hidden bit

// protected member functions of the number system that encode this use-case customization
void generate_add_input(blocktriple<addbits>& a) {}    expands to addition input, materializes hidden bit, and encodes in 2's complement fixed-point
void generate_sub_input(blocktriple<subbits>& a) {}    expands to subtraction input, materializes hidden bit, and encodes in 2's complement fixed-point
void generate_mul_input(blocktriple<mulbits>& a) {}    expands to multiplication input and materializes hidden bit
void generate_div_input(blocktriple<divbits>& a) {}    expands to division input and materializes hidden bit
void generate_sqrt_input(blocktriple<sqrtbits>& a) {}


                               ADD             SUB              MUL               DIV
	uradd(a, b, sum) {
          int exp_diff = a.exp() - b.exp();
          a.align(exp_diff); // just a righ shift if > 0  avoid a secondary copy
          b.align(exp_diff); // just a right shift if > 0 avoid a secondary copy

				// the basic design of blocktriple tries to minimize the number of copies of fraction bits
				// otherwise stated, the convertion needs to yield a blocktriple that is ready
				// to be used in arithmetic operations without any further need to be manipulated.
				// 
				// set of possible formats :
				// representation   :   .ffff      with hidden bit hidden
				// input to add/sub : 01.ffffaaa   with expansion and encoded in 2's complement fixed-point
				// input to mul     :  1.ffffmmmm  with expansion and hidden bit materialized
				// input to div     :  1.ffffddddd with expansion and hidden bit materialized
				// input to sqrt    :  1.ffffsss   with expension and hidden bit materialized


