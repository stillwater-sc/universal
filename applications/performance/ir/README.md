# Iterative Refinement

Iterative refinement are methods that use a very simple linear algebra operator, such as a matrix-vector multiply, 
to calculate new approximations to the solution of a system of constraints. In these cased, we can leverage
lower precision arithmetic to try to speed up the linear algebra operator at the core of the iteration.

This directory contains different iterative refinement algorithms and their responses to mixed-precision
acceleration approaches.
