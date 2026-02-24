// precision.cpp: characterization of rational precision as a function of size
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the rational template environment
// enable/disable arithmetic exceptions
#define RATIONAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/rational/rational.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw {
	namespace universal {
		
		template<unsigned nbits, unsigned es>
		void epsilon() {
			std::cout << std::setw(5) << nbits << "\t" 
				<< std::setw(15) << std::numeric_limits<cfloat<nbits, es>>::epsilon() << "\t"
				<< std::setw(15) << std::numeric_limits<posit<nbits, 2>>::epsilon() << "\t"
				<< std::setw(15) << std::numeric_limits<rational<nbits, base16>>::epsilon() << '\n';
		}
	}
}


int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "hexadecimal rational precision characterization";
	std::string test_tag    = "hexadecimal rational precision";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// what is the progression of precision for increasingly larger binary rational types
	std::cout << "epsilon for different sizes\n"
		<< std::setw(5) << "nbits" 
		<< std::setw(15) << "cfloat"
		<< std::setw(15) << "posit"
		<< std::setw(15) << "rational\n";
	epsilon<4,2>();
	epsilon<8,2>();
	epsilon<12,5>();
	epsilon<16,5>();
	epsilon<20,8>();
	epsilon<24,8>();
	epsilon<28,8>();
	epsilon<32,8>();
	epsilon<40,11>();
	epsilon<48,11>();
	epsilon<56,11>();
	epsilon<64,11>();
	epsilon<80,15>();
	epsilon<96,15>();
	epsilon<112,15>();
	epsilon<128,15>();

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
