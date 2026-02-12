// integer_cover.cpp: measuring the covering of the integers with a posit
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult

/*
When interacting with integer algebras, for example, prime factorization
algorithms, factorials, binomials, and discretization schemes, a posit
would need to be able to represent the integers to be error-free.

We are interested to see how well a posit configuration can capture integer values.

A sample output of this program to provide insight in the sampling behavior of 
linear and tapered floating point:
Posit Integer Cover
posit sample coverage of 8-bit unsigned integer
posit<12,0>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 23.44%
posit<12,1>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 50%
posit<12,2>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 75%
posit<13,0>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 34.38%
posit<13,1>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 75%
posit<13,2>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<14,0>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 43.75%
posit<14,1>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<14,1>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<15,0>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 62.5%
posit<15,1>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<15,2>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<16,0>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 75%
posit<16,1>: 2^8  integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit sample coverage of 10-bit unsigned integer
posit<16,1>: 2^10 integer cover is : float cover = 100%  double cover = 100%  posit cover = 75%
posit<17,1>: 2^10 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<18,1>: 2^12 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<15,2>: 2^10 integer cover is : float cover = 100%  double cover = 100%  posit cover = 75%
posit<16,2>: 2^10 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<17,2>: 2^10 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<18,2>: 2^10 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit sample coverage of 12-bit unsigned integer
posit<18,1>: 2^12 integer cover is : float cover = 100%  double cover = 100%  posit cover = 50%
posit<19,1>: 2^12 integer cover is : float cover = 100%  double cover = 100%  posit cover = 75%
posit<20,1>: 2^12 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit sample coverage of 14-bit unsigned integer
posit<20,2>: 2^14 integer cover is : float cover = 100%  double cover = 100%  posit cover = 75%
posit<24,1>: 2^14 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<28,1>: 2^14 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit sample coverage of 16-bit unsigned integer
posit<20,1>: 2^16 integer cover is : float cover = 100%  double cover = 100%  posit cover = 10.94%
posit<24,1>: 2^16 integer cover is : float cover = 100%  double cover = 100%  posit cover = 50%
posit<28,1>: 2^16 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<32,1>: 2^16 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit<32,2>: 2^16 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit sample coverage of 20-bit unsigned integer
posit<20,1>: 2^20 integer cover is : float cover = 100%  double cover = 100%  posit cover = 0.7568%
posit<26,1>: 2^20 integer cover is : float cover = 100%  double cover = 100%  posit cover = 10.94%
posit<32,2>: 2^20 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
posit sample coverage of 24-bit unsigned integer
posit<26,1>: 2^24 integer cover is : float cover = 100%  double cover = 100%  posit cover = 0.7568%
posit<32,2>: 2^24 integer cover is : float cover = 100%  double cover = 100%  posit cover = 75%
posit<34,2>: 2^24 integer cover is : float cover = 100%  double cover = 100%  posit cover = 100%
*/

// Calculate the sample cover for a posit representing an unsigned long.
// unsigned is the worst case as all the values are mapped to just a
// quarter of the posit encodings (the North-East quadrant of the unit circle projection of the reals)
// The number of samples in a posit quadrant is 2^(nbits - 2). 
// Thus mathematically the 2^(nbits-2) samples need to cover the 2^ibits values of the integer.
template<size_t nbits, size_t es, size_t ibits>
std::string CalculateIntegerCover() {
	sw::universal::posit<nbits, es> pInt;

	constexpr unsigned long long nrSamples = (uint64_t)1 << ibits;
	unsigned long long fcover = 0, dcover = 0, pcover = 0;
	for (unsigned long long integer = 0; integer < nrSamples; ++integer) {
		// float cover
		unsigned long long rounded = (unsigned long long)(float(integer));
		if (rounded == integer) ++fcover;
		// double cover
		rounded = (unsigned long long)(double(integer));
		if (rounded == integer) ++dcover;
		// posit cover
		pInt = integer;
		rounded = (unsigned long long)(pInt);
		if (rounded == integer) {
			++pcover;
		}
	}
	std::stringstream ss;
	ss << std::setprecision(4);
	ss << "float cover = " << 100.0 * (double)fcover / nrSamples << "%  ";
	ss << "double cover = " << 100.0 * (double)dcover / nrSamples << "%  ";
	ss << "posit cover = " << 100.0 * (double)pcover / nrSamples << "%";
	return ss.str();
}

// set by the build process to modulate the number of test cases
#define _FULL_REGRESSION

int main()
try {
	using namespace sw::universal;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	std::cout << "Posit Integer Cover\n";

	std::cout << "posit sample coverage of 8-bit unsigned integer\n";
	std::cout << "posit<12,0>: 2^8  integer cover is : " << CalculateIntegerCover<12, 0, 8>() << "\n";
	std::cout << "posit<12,1>: 2^8  integer cover is : " << CalculateIntegerCover<12, 1, 8>() << "\n";
	std::cout << "posit<12,2>: 2^8  integer cover is : " << CalculateIntegerCover<12, 2, 8>() << "\n";

	std::cout << "posit<13,0>: 2^8  integer cover is : " << CalculateIntegerCover<13, 0, 8>() << "\n";
	std::cout << "posit<13,1>: 2^8  integer cover is : " << CalculateIntegerCover<13, 1, 8>() << "\n";
	std::cout << "posit<13,2>: 2^8  integer cover is : " << CalculateIntegerCover<13, 2, 8>() << "\n";

	std::cout << "posit<14,0>: 2^8  integer cover is : " << CalculateIntegerCover<14, 0, 8>() << "\n";
	std::cout << "posit<14,1>: 2^8  integer cover is : " << CalculateIntegerCover<14, 1, 8>() << "\n";
	std::cout << "posit<14,1>: 2^8  integer cover is : " << CalculateIntegerCover<14, 2, 8>() << "\n";

	std::cout << "posit<15,0>: 2^8  integer cover is : " << CalculateIntegerCover<15, 0, 8>() << "\n";
	std::cout << "posit<15,1>: 2^8  integer cover is : " << CalculateIntegerCover<15, 1, 8>() << "\n";	
	std::cout << "posit<15,2>: 2^8  integer cover is : " << CalculateIntegerCover<15, 2, 8>() << "\n";

	std::cout << "posit<16,0>: 2^8  integer cover is : " << CalculateIntegerCover<16, 0, 8>() << "\n";
	std::cout << "posit<16,1>: 2^8  integer cover is : " << CalculateIntegerCover<16, 1, 8>() << "\n";

	std::cout << "posit sample coverage of 10-bit unsigned integer\n";
	std::cout << "posit<16,1>: 2^10 integer cover is : " << CalculateIntegerCover<16, 1, 10>() << "\n";
	std::cout << "posit<17,1>: 2^10 integer cover is : " << CalculateIntegerCover<17, 1, 10>() << "\n";
	std::cout << "posit<18,1>: 2^12 integer cover is : " << CalculateIntegerCover<18, 1, 10>() << "\n";
	std::cout << "posit<15,2>: 2^10 integer cover is : " << CalculateIntegerCover<15, 2, 10>() << "\n";
	std::cout << "posit<16,2>: 2^10 integer cover is : " << CalculateIntegerCover<16, 2, 10>() << "\n";
	std::cout << "posit<17,2>: 2^10 integer cover is : " << CalculateIntegerCover<17, 2, 10>() << "\n";
	std::cout << "posit<18,2>: 2^10 integer cover is : " << CalculateIntegerCover<18, 2, 10>() << "\n";

	std::cout << "posit sample coverage of 12-bit unsigned integer\n";
	std::cout << "posit<18,1>: 2^12 integer cover is : " << CalculateIntegerCover<18, 1, 12>() << "\n";
	std::cout << "posit<19,1>: 2^12 integer cover is : " << CalculateIntegerCover<19, 1, 12>() << "\n";
	std::cout << "posit<20,1>: 2^12 integer cover is : " << CalculateIntegerCover<20, 1, 12>() << "\n";

	std::cout << "posit sample coverage of 14-bit unsigned integer\n";
	std::cout << "posit<20,2>: 2^14 integer cover is : " << CalculateIntegerCover<20, 2, 14>() << "\n";
	std::cout << "posit<24,1>: 2^14 integer cover is : " << CalculateIntegerCover<24, 1, 14>() << "\n";
	std::cout << "posit<28,1>: 2^14 integer cover is : " << CalculateIntegerCover<28, 1, 14>() << "\n";

	std::cout << "posit sample coverage of 16-bit unsigned integer\n";
	std::cout << "posit<20,1>: 2^16 integer cover is : " << CalculateIntegerCover<20, 1, 16>() << "\n";
	std::cout << "posit<24,1>: 2^16 integer cover is : " << CalculateIntegerCover<24, 1, 16>() << "\n";
	std::cout << "posit<28,1>: 2^16 integer cover is : " << CalculateIntegerCover<28, 1, 16>() << "\n";
	std::cout << "posit<32,1>: 2^16 integer cover is : " << CalculateIntegerCover<32, 1, 16>() << "\n";
	std::cout << "posit<32,2>: 2^16 integer cover is : " << CalculateIntegerCover<32, 2, 16>() << "\n";

#ifdef FULL_REGRESSION
	std::cout << "posit sample coverage of 20-bit unsigned integer\n";
	std::cout << "posit<20,1>: 2^20 integer cover is : " << CalculateIntegerCover<20, 1, 20>() << "\n";
	std::cout << "posit<26,1>: 2^20 integer cover is : " << CalculateIntegerCover<26, 1, 20>() << "\n";
	std::cout << "posit<32,2>: 2^20 integer cover is : " << CalculateIntegerCover<32, 2, 20>() << "\n";

	std::cout << "posit sample coverage of 24-bit unsigned integer\n";
	std::cout << "posit<26,1>: 2^24 integer cover is : " << CalculateIntegerCover<26, 1, 24>() << "\n";
	std::cout << "posit<32,2>: 2^24 integer cover is : " << CalculateIntegerCover<32, 2, 24>() << "\n";
	std::cout << "posit<34,2>: 2^24 integer cover is : " << CalculateIntegerCover<34, 2, 24>() << "\n";

	/*
	std::cout << "posit sample coverage of 32-bit unsigned integer\n";
	std::cout << "posit<32,2>: 2^32 integer cover is : " << CalculateIntegerCover<32, 2, 32>() << "\n";
	std::cout << "posit<40,2>: 2^32 integer cover is : " << CalculateIntegerCover<40, 2, 32>() << "\n";
	*/

#endif
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
