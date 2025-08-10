# BLAS function overload modifiers

<universal/blas/modifiers/...> contains overloads that modify the default operators contained in <universal/blas/...>.

Custom arithmetic can deliver new BLAS functionality to solve numerical, performance, or accuracy problems.
For example, posits offer the quire and the fused dot product operator to deliver reproducible linear algebra algorithms.
Logarithmic number systems offer large dynamic range, but frequently need special care to accumulate dot products
correctly, as the large dynamic range cause the sum of products to stagnate. 
Similarly, multi-base number systems, typically need custom accumulators for dot products.

As the Universal BLAS offers MATLAB-like operator overloading for dot, matrix-vector, and matrix-matrix products,
the library needs a mechanism to inject new "operator*()" overloads to adjust to the needs of the custom arithmetic.
This directory contains these custom arithmetic overloads to replace the default algorithms.
