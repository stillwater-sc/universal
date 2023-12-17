========================================================================
    Posit Project Overview
========================================================================

Random 32 bits which is intentionally zero heavy
-----------------
Sign
|
|     
v    
1100 1000 1100 0100 0100 0010 0001 0001

How to decode this?

Here is some pseudocode that should work on any bit sequence. Suppose

  n     is the number of bits (which in the example is 32),
  es    is the number of exponent bits (es = 3 in the example since n = 32),
  p     is the bit sequence treated as a signed integer.

Also suppose, though it is not the convention, that the bits are numbered from left to right starting with 1, 
so the sign bit is bit 1 and the rightmost (least significant) bit is bit n. That numbering makes the pseudocode 
more readable. Assume we have a quick machine instruction FindFirstOne(p) that finds the position of the first 1 bit 
in p, returning n+1 if all bits are zero, and a similar instruction FindFirstZero(p). The function Bits(j..k, p) 
extracts bits j through k of p, clipped to bit n. If j is greater than n, Bits(j..k, p) returns zero.

We seek x, the value represented by p. The green text indicates what would happen for the 32 bits in the example.

(* Check for exception values: *)
If Bits(2..n, p) == 0, (* Check for a string of all zeros after the first bit. *)
  If Bits(1..1, p) == 0, (* If the first bit is also zero, then x is zero. *)
    x ← 0;
  Else, (* The first bit must be one, indicating x is unsigned infinity. *)
    x ← ±∞;
  End If;
  End; (* Can cut out early. *)
End If;

The example sequence fails the exception value tests, so continue.

(* Process sign bit: *)
If Bits(1..1, p) == 1, sign ← –1; p ← –p; (* 2's complement negate p. *)
  Else sign ← 1;
End If;

The first bit is a 1, so sign = -1 and p gets negated. Flip all the bits and add 1:
0011 0111 0011 1011 1011 1101 1110 1111

(* Process regime bits. The second bit, r, determines the sign of the regime. *)
r ← Bits(2..2, p)
Bits(1..1, p) ← r (* Duplicate first regime bit into sign bit so the "FindFirst..." instructions work. *)
If r == 0, (* regime value is negative *)
  k = FindFirstOne(p);
  shift ← –(2^es) × (k – 2); (* Actually a shift of k-2 by a shift by es, so this is fast. *)
Else (* regime value is zero or positive *)
  k = FindFirstZero(p);
  shift ← (2^es) × (k – 3);
End If;

The example 001... has r = 0 so the regime value is negative. The first 1 occurs in bit 3, 
so the regime represents –1 and contributes a shift of –(2^es) = –8. This leaves k pointing to bit 3. 
The exponent will be in the next es bits after bit 3, which will fine-tune the shift roughed out by the regime bits.

(* Add the exponent bits, as an unsigned integer, to the shift. *)
shift ← shift + Bits(k+1..k+es, p);

The three exponent bits in the example are the underlined ones: 0011 0111 0011 1011 1011 1101 1110 1111. 
As an unsigned binary integer, 101 means decimal 5, so the shift is –8 + 5 = –3.

(* The remaining bits are the fraction, with a hidden bit of 1, so we're done. It looks like a regular float: *)
x ← sign × 2^shift × 1.Bits(k+es+1..n) 
End

The fraction bits in the example are underlined: 0011 0111 0011 1011 1011 1101 1110 1111. That fraction, 
plus the hidden bit 1, is 121355759/(2^26), or exactly 1.80834172666072845458984375. 
Scale this by 2^(shift) = 2^(–3) = 1/8, and the sign, to get –0.22604271583259105682373046875. 
Notice that there are three more bits in the fraction than there are in an IEEE 32-bit float, 
giving almost a full decimal more accuracy.

I didn't actually run the above pseudocode so it may contain a typo or two, but it is closely based on the 
Mathematica code for decoding a posit and that has been debugged and in use for months. 
It is possible to decode the bit string _without_ doing 2's complement negation if the sign bit is zero 
(the hidden bit for negative posits represents –2, not 1!), which according to Isaac Yonemoto leads to 
simpler hardware... but it creates a more cryptic pseudocode explanation, so I used the 2's complement negation here.

/////////////////////////////////////////////////////////////////////////////
