// sqrt.cpp: test suite runner for sqrt function for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>


namespace sw {
	namespace universal {

		template<typename Ty>
		void GenerateSqrtTestCase(Ty fa) {
			unsigned precision = 25;
			//unsigned width = 30;
			Ty fref;
			sw::universal::qd a, ref, v;
			a = fa;
			fref = std::sqrt(fa);
			ref = fref;
			v = sw::universal::sqrt(a);
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(precision);
			std::cout << " -> sqrt(" << fa << ") = " << fref << std::endl;
			std::cout << " -> sqrt( " << a << ") = " << v << '\n' << to_binary(v) << '\n';
			std::cout << to_binary(ref) << "\n -> reference\n";
			std::cout << (ref == v ? "PASS" : "FAIL") << std::endl << std::endl;
			std::cout << std::setprecision(oldPrec);
		}

		template<typename QuadDouble>
		int VerifySqrtFunction(bool reportTestCases, QuadDouble a) {
			int nrOfFailedTestCases{ 0 };
			QuadDouble b{ a };
			for (int i = 0; i < 9; ++i) {
				a *= a;
				qd c = sqrt(a);
				if (b != c) {
					if (reportTestCases) std::cerr << "FAIL : " << b << " != " << c << '\n';
					++nrOfFailedTestCases;
				}
				b *= b;
			}
			return nrOfFailedTestCases;
		}
	}
}


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

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "quad-double mathlib sqrt function validation";
	std::string test_tag    = "sqrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug
	GenerateSqrtTestCase<double>(1.0);
	GenerateSqrtTestCase<double>(1024.0 * 1024.0);
	constexpr double minpos = std::numeric_limits<double>::min();
	GenerateSqrtTestCase<double>(minpos);
	constexpr double maxpos = std::numeric_limits<double>::max();
	GenerateSqrtTestCase<double>(maxpos);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	nrOfFailedTestCases += ReportTestResult(VerifySqrtFunction<qd>(reportTestCases, qd(2.0)), "sqrt(qd > 1.0)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtFunction<qd>(reportTestCases, qd(0.5)), "sqrt(qd < 1.0)", test_tag);


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
