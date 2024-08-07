// sterbenz_lemma.cpp: Demonstration of Sterbenz Lemma
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// pull in the posit number system
#include <universal/number/posit/posit.hpp>

/*

Wikipedia: https://en.wikipedia.org/wiki/Sterbenz_lemma

Sterbenz Lemma is a theorem giving conditions under which floating-point differences
are computed exactly. It is named after Pat H. Sterbenz, who published it as
Theorem 4.3.1 in his 1974 book "Floating-Point Computation"

The Sterbenz lemma states that for a floating-point number system WITH subnormal
numbers, such as IEEE-754, any two numbers of the same format, but excluding NaR,
satisfying the constraint
                 a/2 <= b <= 2*a => a subop b = a - b
otherwise stated, a - b is exactly representable in the number system.

Posits have a dynamic set of precision bits across their domains. 
For a given posit format, if the fraction field length of 2^l is p >= 1 bits, 
then for all d in [0,p], the fraction field length of 2^(l-p) is at least (p - d).

The proof in general case is based on the following lemma:

For a given posit format, if the fraction field length of 2^m is p >= m bits, 
then for all d in [0,p], the fraction field length of 2^(m-d) is at least (p - d).
*/

template<typename Real>
int SterbenzCheck(const Real& a) {
	using namespace sw::universal;

	Real c = a - (a / 2);
	std::cout << color_print(c) << '\n';

	int nrOfFailures = 0;
	float fa = float(a);
	for (Real b = a / 2; b <= 2 * a; ++b) {   // only works for Universal number systems as it overloads increment/decrement as next/previous value on the Real line
		float fb = float(b);
		float fc = fa - fb;
		c = a - b;
		if (fc != c) {
			std::cout << "FAIL: " << a << " - " << b << " = " << c << " reference = " << fc << '\n';
			++nrOfFailures;
		}
	}
	if (nrOfFailures == 0) {
		std::cout << "PASS\n";
	}
	return nrOfFailures;
}

int main()
try {
	using namespace sw::universal;

	constexpr size_t nbits = 8;
	constexpr size_t es = 0;
	using Real = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	// set an a
	Real a = 1.0f;
	// how many samples are there in the range [a/2, 2a]?
	// a/2 = 0011000000000000 
	// a   = 0100000000000000 
	// 2*a = 0101000000000000
	std::cout << "[ " << color_print(a / 2) << " " << color_print(a) << " " << color_print(2 * a) << " ]" << '\n';
	SterbenzCheck(a);

	a.minpos();
	a *= 2;
	SterbenzCheck(a);
	a *= 2;
	SterbenzCheck(a);
	a *= 2;
	SterbenzCheck(a);
	a *= 2;
	SterbenzCheck(a);

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

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
