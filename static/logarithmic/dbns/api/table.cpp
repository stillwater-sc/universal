// table.cpp: table of values for fixed-size, arbitrary precision double base number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/dbns/table.hpp>
#include <universal/number/cfloat/cfloat.hpp>  // bit field comparisons
#include <universal/verification/test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

template<typename Real, unsigned nrBases>
class lnsBases {
public:

private:
	Real base[nrBases];
};

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dbns table of values";
	std::string test_tag    = "values";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		dbns<8, 3, uint8_t> l;
		l.debugConstexprParameters();
	}


	return 0;
	{
		std::cout << "+---------    dynamic ranges of 8-bit dbns<> configurations   --------+\n";
		std::cout << symmetry_range< dbns<8, 1> >() << '\n';
		std::cout << symmetry_range< dbns<8, 2> >() << '\n';
		std::cout << symmetry_range< dbns<8, 3> >() << '\n';
		std::cout << symmetry_range< dbns<8, 4> >() << '\n';
		std::cout << symmetry_range< dbns<8, 5> >() << '\n';
		std::cout << symmetry_range< dbns<8, 6> >() << '\n';
	}

	{
		std::cout << "+---------    Dynamic ranges of dbns<> configurations   --------+\n";
		std::cout << dynamic_range< dbns< 4, 2> >() << '\n';
		std::cout << dynamic_range< dbns< 8, 3> >() << '\n';
		std::cout << dynamic_range< dbns<12, 4> >() << '\n';
		std::cout << dynamic_range< dbns<16, 5> >() << '\n';
		std::cout << dynamic_range< dbns<20, 6> >() << '\n';
	}

	{
		// generate a value table for dbns<8,3>
		GenerateDbnsTable<8, 3>(std::cout, false);
	}

	std::cout << "\n\n\n";

	{
		dbns<8, 3> l;

		l.setbits(0x11); // 0x0.001.0001
		ReportValue(l);
	}



	
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
