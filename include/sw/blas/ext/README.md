# BLAS function extensions

<universal/blas/ext/...> contains extensions to the default operators contained in <universal/blas/...>.

Custom arithmetic can deliver new BLAS functionality to solve numerical, performance, or accuracy problems.
For example, posits offer the quire functionality to restore associativity and distributive laws of algebra
to linear algebra algorithms. 

Similarly, logarithmic and multi-base number systems, typically need custom accumulators for dot products.

The blas extention directory allows custom functions to be offered for dot, matrix-vector, and matrix-matrix products,
