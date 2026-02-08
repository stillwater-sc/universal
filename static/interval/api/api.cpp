// api.cpp: application programming interface demonstration of parameterized interval arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/interval/interval.hpp>
#include <universal/number/cfloat/cfloat.hpp>  // for comparison with Universal types
#include <universal/verification/test_suite.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
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

	std::string test_suite  = "interval API demonstration";
	std::string test_tag    = "api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Demonstrate interval with native float
	{
		std::cout << "+---------    interval<float> basic operations   --------+\n";
		using Real = interval<float>;

		Real a(1.0f);          // degenerate interval [1, 1]
		Real b(2.0f, 3.0f);    // interval [2, 3]

		std::cout << "a = " << a << " (degenerate interval)\n";
		std::cout << "b = " << b << " (proper interval)\n";

		// Arithmetic
		Real c = a + b;
		std::cout << "a + b = " << c << '\n';

		c = a - b;
		std::cout << "a - b = " << c << '\n';

		c = a * b;
		std::cout << "a * b = " << c << '\n';

		c = a / b;
		std::cout << "a / b = " << c << '\n';

		std::cout << '\n';
	}

	// Demonstrate interval with double
	{
		std::cout << "+---------    interval<double> basic operations   --------+\n";
		using Real = interval<double>;

		Real a(1.0, 2.0);
		Real b(3.0, 4.0);

		std::cout << "a = " << a << '\n';
		std::cout << "b = " << b << '\n';

		std::cout << "a + b = " << (a + b) << '\n';
		std::cout << "a - b = " << (a - b) << '\n';
		std::cout << "a * b = " << (a * b) << '\n';
		std::cout << "a / b = " << (a / b) << '\n';

		std::cout << '\n';
	}

	// Demonstrate interval properties
	{
		std::cout << "+---------    interval properties   --------+\n";
		using Real = interval<double>;

		Real a(-1.0, 2.0);
		std::cout << "a = " << a << '\n';
		std::cout << "  mid = " << a.mid() << '\n';
		std::cout << "  rad = " << a.rad() << '\n';
		std::cout << "  width = " << a.width() << '\n';
		std::cout << "  mag = " << a.mag() << '\n';
		std::cout << "  mig = " << a.mig() << '\n';
		std::cout << "  contains_zero = " << (a.contains_zero() ? "yes" : "no") << '\n';
		std::cout << "  ispos = " << (a.ispos() ? "yes" : "no") << '\n';
		std::cout << "  isneg = " << (a.isneg() ? "yes" : "no") << '\n';

		std::cout << '\n';
	}

	// Demonstrate interval with Universal cfloat type
	{
		std::cout << "+---------    interval<cfloat<16,5>> operations   --------+\n";
		using Scalar = cfloat<16, 5, uint16_t>;
		using Real = interval<Scalar>;

		Real a(Scalar(1.0f), Scalar(2.0f));
		Real b(Scalar(0.5f), Scalar(1.5f));

		std::cout << "a = " << a << '\n';
		std::cout << "b = " << b << '\n';

		std::cout << "a + b = " << (a + b) << '\n';
		std::cout << "a - b = " << (a - b) << '\n';
		std::cout << "a * b = " << (a * b) << '\n';
		std::cout << "a / b = " << (a / b) << '\n';

		std::cout << '\n';
	}

	// Demonstrate interval containment and overlap
	{
		std::cout << "+---------    interval containment and overlap   --------+\n";
		using Real = interval<double>;

		Real a(1.0, 5.0);
		Real b(2.0, 4.0);
		Real c(4.0, 6.0);
		Real d(10.0, 12.0);

		std::cout << "a = " << a << '\n';
		std::cout << "b = " << b << '\n';
		std::cout << "c = " << c << '\n';
		std::cout << "d = " << d << '\n';

		std::cout << "b.subset_of(a) = " << (b.subset_of(a) ? "yes" : "no") << '\n';
		std::cout << "a.overlaps(c) = " << (a.overlaps(c) ? "yes" : "no") << '\n';
		std::cout << "a.overlaps(d) = " << (a.overlaps(d) ? "yes" : "no") << '\n';

		std::cout << "intersect(a, c) = " << intersect(a, c) << '\n';
		std::cout << "hull(a, c) = " << hull(a, c) << '\n';

		std::cout << '\n';
	}

	// Demonstrate mathematical functions
	{
		std::cout << "+---------    interval mathematical functions   --------+\n";
		using Real = interval<double>;

		Real a(1.0, 4.0);
		Real b(-2.0, 3.0);

		std::cout << "a = " << a << '\n';
		std::cout << "b = " << b << '\n';

		std::cout << "abs(b) = " << abs(b) << '\n';
		std::cout << "sqr(a) = " << sqr(a) << '\n';
		std::cout << "sqrt(a) = " << sqrt(a) << '\n';
		std::cout << "pow(a, 2) = " << pow(a, 2) << '\n';
		std::cout << "pow(a, 3) = " << pow(a, 3) << '\n';

		std::cout << '\n';
	}

	// Type traits
	{
		std::cout << "+---------    interval type traits   --------+\n";
		using Real = interval<float>;

		std::cout << "is_interval<interval<float>> = " << (is_interval<Real> ? "true" : "false") << '\n';
		std::cout << "is_interval<float> = " << (is_interval<float> ? "true" : "false") << '\n';

		std::cout << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
