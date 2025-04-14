// oneMinusCosDivSin.cpp: experiments with accuracy and precision 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/qd/qd.hpp>

/*
Precision is the number of digits available for expressing a numerical value.
We usually express precision in bits if discussing a computer format, or
decimal digits if discussing numbers in human-friendly format. Notice that
this definition says nothing about the correctness of any of the digits,
simply their availability.

Absolute error is

				 |computed value – correct value|.

It is most appropriate when describing results with a number format that
represents uniformly-spaced values (integer and fixed-point format).

When working with values that range over several magnitudes, we find ourselves
wanting something better than absolute accuracy. If the correct value is 1008
and we computed 1009, that seems fairly accurate; yet it has the same absolute
accuracy as when the correct value is 0.1008 and we computed 1.1008, almost
ten times too large a result. Our instincts might then lead us to normalize
by the correct value, leading to the traditional definition of relative error:

Relative error (traditional) is:

				 Absolute error / correct value.

This is the most common definition of relative error. Notice that it is not
useful when the correct value is zero, nor when the correct value has infinite
magnitude. It has another shortcoming: If we know x to some relative error,
then we know 1/x to a different relative error. For example, say the strength
of a lens is supposed to be 5 diopters but we instead have one with strength
4 diopters. A diopter is the inverse of the focal length in meters. The traditional
relative error is then |5 – 4| / 5 = 0.2, that is, off by 20%. But if we measure
the lens strength by focal length, then the correct strength is 1/5 diopters = 0.2 meter
but instead we have a lens with strength 1/4 diopters = 0.25 meter. The traditional
relative error is |0.2 – 0.25| / 0.2 = 0.25, that is, off by 25%.

A better definition for number systems that cover a wide range of magnitudes is
to take the absolute value of the logarithm of the ratio of the correct and
computed values, and that is the definition we use here:

Relative error =

				  |ln(computed value/ correct value)|,

 where we require that the correct value is finite and nonzero, and the computed
 value has the same sign as the correct value. Relative error is otherwise treated
 as undefined. Peter Lindstrom notes that the natural logarithm is the right one to use,
 because the relative error of 1 + 𝜀 is close to 𝜀 when the correct value is 1,
 which agrees with our intuition and also closely matches the traditional definition
 of relative error. Now, however, if we use our lens example, the relative error
 is |ln(5/4)| = |ln(4/5)| ≈ 0.223 and it doesn’t matter which way we measure the
 strength of the lens.

Now that we have a sound measure of error, we can use that to define accuracy:

			 Accuracy = 1 / relative error

			 Decimal accuracy = log10(accuracy)

			 Binary accuracy = log2(accuracy)

We can think of the last two as defining the number of correct decimal digits or
correct bits in the answer, but it need not be a whole number. For the lens example,
we know the focal length to ~0.65 decimal digits, or ~2.16 bits.
*/

template<typename Real>
Real OneMinusCosDivSin(const Real& x) {
    using std::cos, std::sin;
    return (Real(1.0) - cos(x)) / sin(x);
}

template<typename Real>
void Report(const Real& x) {
	using namespace sw::universal;
	std::cout << std::setw(30) << type_tag(Real()) << " : (1 - cos(x)) / sin(x) = " << OneMinusCosDivSin(x) << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::cout << "Experiments in precision and accuracy\n";

	double dx{ 1.0e-8 };

	Report(qd{ dx });
	Report(posit<32, 2>{ dx });
	Report(cfloat<64, 11, uint32_t, true, false, false>{ dx });

	return EXIT_SUCCESS;
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
