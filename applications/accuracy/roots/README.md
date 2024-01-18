# Notes on root finding

Newton's method alone is not a particularly good way to find polynomial roots in the way you are trying to do here.
It has well known modes of failure even when near a root, and has no guarantees about finding one. 
Furthermore its convergence can be quite slow in the presence of repeated roots.

If you have a range of x-values to search over, where there is a sign change in the function (which guarantees at 
least one root) you can use a bracketing and bisection method such as the golden ratio search. This is robust but 
slow, and only gives one root.
The standard technique is something like Brent's method (see Numerical Recipes in C, Section 9.3), which combines 
the robust bisection method with a quadratic inverse interpolation to give a much faster convergence. 
I believe there is also a section in that book describing an algorithm to generate the initial bracket if you 
have no idea of the bounds.

Once you have found one root, it may be possible to factor it out of the polynomial and search for another root, 
until the "remainder" polynomial is order 1 (http://en.wikipedia.org/wiki/Horner_scheme#Example_2), or 
there are no more roots to find.

If you actually need to know ALL the roots, then the best way I know of is to compute the eigenvalues of the 
polynomial's companion matrix (http://en.wikipedia.org/wiki/Companion_matrix).
This will give you exactly n complex roots, where n is the degree of the polynomial 
(http://en.wikipedia.org/wiki/Fundamental_theorem_of_algebra). If you only need the real roots, simply 
reject those that have a non-zero imaginary component.
For this you will obviously need a function to compute the eigenvalues of a matrix. It is a pain to write 
your own, so I suggest you look for a linear algebra library that supports it.

This latter method is the one that is used in MATLAB and other numerical computing packages, and doesn't rely on arbitrary tolerances and limits.

Edit: Added link to polynomial division example by Horner's method

You need to find a way to get rid of your step and compare constants.
For any set of these, it is easy to find a polynomial that will trick the logic you have implemented, as you have discovered with multiple non-repeated roots, or finding none where one does exist in the interval.

In general root-finding and optimisation routines are usually highly non-trivial and remain open research problems for the most part. 
For one-dimensional problems like this, there are good, provably robust solutions like Brent's method.
I have used Brent's method for one-dimensional minimisation (which is just essentially finding roots of the derivative). 
The source code in given in Numerical Recipes in C, which available online on their website:
http://www.nrbook.com/a/bookcpdf/c9-3.pdf

Take the time to read the explanation, because while the code may be a mess, the concept is not much more complex than what 
you are trying to implement here.

As for Horner's method, on the same page as the example I linked, the solution method is given 
(http://en.wikipedia.org/wiki/Horner_scheme#Polynomial_Root_Finding)
Using the Horner scheme in combination with Newton's zero finding method it is possible to approximate the real roots of a polynomial. 
The algorithm is as follows. 
Given a polynomial pn(x) of degree n with zeros zn < zn − 1 < ... < z1 make some initial guess x0 such that x0 > z1. 
Now follow the steps outline below.

1.   Using Newton's method find the largest zero, z1 of pn(x) using the guess x0.
2.   Use the Horner scheme to divide out (x − z1) to obtain pn − 1. Return to step 1 but use the polynomial pn − 1 and the initial guess z1.

These two steps are repeated until all real zeros are found for the polynomial. If the approximated zeros are not precise enough, the obtained values can be used as initial guesses for Newton's method but using the full polynomial rather than the reduced polynomials.

It then works through an example, and gives you the MATLAB/OCTAVE source code for the polynomial factorisation, which is the part you don't have yet.

Take the time to work through the example on paper, so you are sure you understand the process well enough to write the code.
