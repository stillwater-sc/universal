// laplace.cpp: successive over-relaxation with adaptive unum/posit precision
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

// Stillwater BLAS library
#include <blas/blas.hpp>

/*

Mathematical 	C++ Symbol	Decimal Representation
Expression
pi				M_PI		3.14159265358979323846
pi/2			M_PI_2		1.57079632679489661923
pi/4			M_PI_4		0.785398163397448309616
1/pi			M_1_PI		0.318309886183790671538
2/pi			M_2_PI		0.636619772367581343076
2/sqrt(pi)		M_2_SQRTPI	1.12837916709551257390
sqrt(2)			M_SQRT2		1.41421356237309504880
1/sqrt(2)		M_SQRT1_2	0.707106781186547524401
e				M_E			2.71828182845904523536
log_2(e)		M_LOG2E		1.44269504088896340736
log_10(e)		M_LOG10E	0.434294481903251827651
log_e(2)		M_LN2		0.693147180559945309417
log_e(10)		M_LN10		2.30258509299404568402

*/

constexpr double pi = 3.14159265358979323846;  // best practice for C++

/*

When you say "an application study on the typical 
length of a unum during the solution of a PDE or ODE 
or optimization problem", there's a veritable mountain 
of papers that could be written on such a broad topic. 
But do you mean Type I unums? It sounds like it, since those 
are the only ones that vary in length. There's a chapter 
of The End of Error that I was not able to finish in time, 
but it still needs to be written, regarding Laplace's 
equation and other PDEs that can be solved crudely at 
first and then refined since the errors do not accumulate 
with time-stepping. Start at very low precision and work 
up, saving power and energy. 

With what we know now about posits, a much better design 
for a number format than Type I, it would be very cool 
indeed to show an example of a PDE using various posit 
precisions to minimize the power consumed to get to an 
answer. Do you have the time resources for such an effort? 
I feel like I'm pretty swamped right now, but getting 
the Draft Standard closer to complete will be one thing 
I can take off my plate if everyone thinks it's looking 
pretty close to perfect.

If I were starting the experiment, I'd try solving 
Laplace's equation on a square. Boundary condition 
f(x,y) = 1 on some part of the square like the left half 
           of the bottom border (to be asymmetrical), 
f(x,y) = 0 elsewhere on the border or maybe put a –1 value 
           somewhere (it wastes the sign bit of a posit 
		   if all the values are nonnegative), 
∇²f = 0 on the interior. Try the oldest and simplest 
method of relaxation, even though better iterative methods 
are known, with a very coarse grid and very low precision posits. 
Like, a 4-by-4 grid of points and 4-bit posits with es = 2. 
It should converge very quickly, and I think the discretization 
error goes as the inverse square of the grid spacing. 

The idea is to decrease rounding error by adding bits 
to the end of the posit solution and refine the grid, 
keeping those two sources of error in the same ballpark. 

My hypothesis is that the result will resemble a multigrid solver, 
which is actually one of the best ways to solve Laplace's equation.

If that works, then I'd try making the domain L-shaped, 
which I believe produces a singularity at the interior corner point 
and should make it harder to converge there. 

As I said, I had hoped to try this approach with Type I unums 
but ran out of time. My manuscript was six months late as it was!
*/

int main(int argc, char** argv)
try {
	using namespace sw::universal;
	using namespace sw::blas;
	using namespace sw::numeric::containers;

	constexpr size_t nbits = 16;
	constexpr size_t es = 1;
	using Scalar = posit<nbits, es>;
//	using Scalar = float;
	using Matrix = matrix<Scalar>;

	int nrOfFailedTestCases = 0;

	Scalar p(pi);
	std::cout << "PI = " << p << " " << hex_format(p) << '\n';

	Matrix A;
	laplace2D(A, 5, 5);
	std::cout << A << std::endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
