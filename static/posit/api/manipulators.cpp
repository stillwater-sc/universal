// manipulators.cpp: manipulator tests for arbitrary configuration posit types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable general or specialized configurations
#define POSIT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define POSIT_ENABLE_LITERALS 1
// fourth: enable/disable error-free serialization I/O
#define POSIT_ERROR_FREE_IO_FORMAT 0
// minimum set of include files to reflect source code dependencies
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_reporters.hpp>


template<unsigned nbits, unsigned es>
void GenerateTable() {
	using namespace sw::universal;
	posit<nbits, es> p;
	unsigned COLUMNWIDTH = 15u;
	std::cout << std::setw(COLUMNWIDTH) << "raw" << " : " <<
		std::setw(COLUMNWIDTH) << "to_binary" << " : " <<
		std::setw(COLUMNWIDTH) << "color_print" << " : " <<
		std::setw(COLUMNWIDTH) << "value" << '\n';
	for (unsigned i = 0; i < (1ul << nbits); ++i) {
		p.setbits(i);
		std::cout << std::setw(COLUMNWIDTH) << p.get() << " : " 
			<< std::setw(COLUMNWIDTH) << to_binary(p, false) << " : " 
			<< "          " << color_print(p) << " : "
			<< std::setw(COLUMNWIDTH) << p << '\n';
	}
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "generalized posit manipulator functions";
	std::string test_tag    = "manipulators";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// posit manipulators

	{
		// report the type
		posit<8, 2> a(SpecificValue::maxpos), b(SpecificValue::minneg), c;
		std::cout << type_tag(a) << '\n';
		c = a * b;
		std::cout << a << " * " << b << " = " << c << '\n';
		std::cout << color_print(a) << " * " << color_print(b) << " = " << color_print(c) << '\n';
	}

	{
		std::cout << "\nTable of encodings\n";
		constexpr unsigned nbits = 5;
		constexpr unsigned es = 2;
		posit<nbits, es> p5;
		std::cout << type_tag(p5) << '\n';
		GenerateTable<nbits, es>();
	}


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
