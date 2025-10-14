// api.cpp: functional tests of the value type API
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
// configure the value<> environment
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
//#include <universal/number/edecimal/edecimal.hpp>
#include <universal/internal/value/value.hpp>		// TODO remove: INTERNAL class: not part of the public Universal API
#include <universal/verification/test_suite.hpp>
#include <math/constants/float_constants.hpp>
#include <math/constants/double_constants.hpp>

using namespace sw::universal;
using namespace sw::universal::internal;

template<unsigned fbits>
int Check(const value<fbits>& v, double ref, bool reportTestCases) {
	int fails = 0;
	if (v.to_double() != ref) {
		++fails;
		if (reportTestCases) {
			std::cout << v << " != " << ref << '\n';
		}
	}
	return fails;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::internal;

	std::string test_suite  = "value class API";
	std::string test_tag    = "value";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';
	std::cout << (reportTestCases ? " " : "not ") << "reporting individual testcases\n";

#if MANUAL_TESTING

	float     f = f_pi;
	std::cout << to_triple(f, true) << " : " << f << '\n';

	std::cout << "---------------- value<23> arithmetic --------------" << std::endl;
	value<23> a, b, c;

	a = 1.5f; b = 2.5f;


	c = a + b;
	std::cout << c << " : reference " << c.to_float() << '\n';

	c = b - a;
	std::cout << c << " : reference " << c.to_float() << '\n';

	c = a * b;
	std::cout << c << " : reference " << c.to_float() << '\n';

	c = b / a;
	std::cout << c << " : reference " << c.to_float() << '\n';


	std::cout << "---------------------- pi -------------------" << std::endl;
	a = f_pi;
	std::cout << to_triple(a) << " : " << a << '\n';
	b = 2.0f;
	std::cout << to_triple(b) << " : " << b << '\n';
	c = a * b;
	std::cout << to_triple(c) << " : " << c << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
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
