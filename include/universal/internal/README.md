# DESIGN and IMPLEMENTATION HISTORY of the floating-point engine

The first posit implementation used a bit-level abstraction
for access and storage, and a conversion to a tailored floating-point
representation for arithmetic. The tailoring was to the specific
number of fraction bits that the posit had to represent around 1.0.

The first floating-point back-end design, value<fbits>, had a fraction 
bit parameter to select among different normalizations for 
addition, multiplication, and division. Inside, these operators
we would expand and align the operands as needed, requiring a copy.
   
But the normalization is NOT a generic op, it is very specific for 
add, mul, div, or sqrt, thus having a fully parameterized interface 
creates a state space for bugs that could get triggered by incorrect 
calling of the normalize method. Secondly, no efficient unit test was 
feasible as most of the state space would NOT be valid conversions.
Given that context of the experience with value<> we decided to clamp down
on this parameterization overkill and create explicit normalization 
conversions for add, mul, div, and sqrt. 

We stay with the design that the number system defines the bit encoding
for storage, and that encoding gets transformed to an internal floating-point
format for arithmetic operations. In diagram form:

   -> load 
         -> bit encoding 
         -> convert to floating-point 
         -> arithmetic operator 
         -> round result 
         -> convert to bit encoding 
   -> store