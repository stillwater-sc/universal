// sqrt.cpp: test suite runner for sqrt function for double-double (dd) floating-point (#1883: accuracy)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <cmath>
#include <random>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>


namespace sw {
	namespace universal {

		// generate specific test case 
		template<typename Ty>
		void GenerateSqrtTestCase(Ty fa) {
			unsigned precision = 25;
			//unsigned width = 30;
			Ty fref;
			sw::universal::dd a, ref, v;
			a = fa;
			fref = std::sqrt(fa);
			ref = fref;
			v = sw::universal::sqrt(a);
			auto defaultPrecision = std::cout.precision();
			std::cout << std::setprecision(precision);
			std::cout << " -> sqrt(" << fa << ") = " << fref << std::endl;
			std::cout << " -> sqrt( " << a << ") = " << v << '\n' << to_binary(v) << '\n';
			std::cout << to_binary(ref) << "\n -> reference\n";
			std::cout << (ref == v ? "PASS" : "FAIL") << std::endl << std::endl;
			std::cout << std::setprecision(defaultPrecision);
		}

		// generate specific test case 
		template<typename Ty>
		void GenerateCbrtTestCase(Ty fa) {
			unsigned precision = 25;
			//unsigned width = 30;
			Ty fref;
			sw::universal::dd a, ref, v;
			a = fa;
			fref = std::cbrt(fa);
			ref = fref;
			v = sw::universal::cbrt(a);
			auto defaultPrecision = std::cout.precision();
			std::cout << std::setprecision(precision);
			std::cout << " -> sqrt(" << fa << ") = " << fref << std::endl;
			std::cout << " -> sqrt( " << a << ") = " << v << '\n' << to_binary(v) << '\n';
			std::cout << to_binary(ref) << "\n -> reference\n";
			std::cout << (ref == v ? "PASS" : "FAIL") << std::endl << std::endl;
			std::cout << std::setprecision(defaultPrecision);
		}


		template<typename DoubleDouble>
		int VerifySqrtFunction(bool reportTestCases, DoubleDouble a) {
			int nrOfFailedTestCases{ 0 };
			DoubleDouble b{ a };
			for (int i = 0; i < 9; ++i) {
				a *= a;
				dd c = sqrt(a);
				if (b != c) {
					if (reportTestCases) std::cerr << "FAIL : " << b << " != " << c << '\n';
					++nrOfFailedTestCases;
				}
				b *= b;
			}
			return nrOfFailedTestCases;
		}

		// relative error of sqrt against a qd reference, in units of 2^-106, over random arguments:
		// set 0 is a = u[1,4) + u[1,4) * 2^-53 (#1883's reproduction), set 1 spans 2^[-1000, 1000]
		// with a tail anywhere in (-ulp, ulp). Karp's step alone reached 7.9 units; with the Newton
		// correction the measured worst is 1.99 over 10^6 arguments, below epsilon() = 2 units. The bound leaves room for
		// FMA contraction and libm differences across platforms.
		int VerifySqrtAccuracy(bool reportTestCases, int set, int samples, double bound) {
			std::mt19937_64 g(13);
			std::uniform_real_distribution<double> u(1.0, 4.0), m(1.0, 2.0), l(-1.0, 1.0);
			std::uniform_int_distribution<int> ex(-1000, 1000);
			int nrOfFailedTestCases{ 0 };
			double worst{ 0.0 };
			for (int i = 0; i < samples; ++i) {
				dd a;
				if (set == 0) {
					a = dd(u(g)) + dd(u(g) * 0x1p-53);
				}
				else {
					const int e = ex(g);
					a = dd(std::ldexp(m(g), e)) + dd(std::ldexp(l(g), e - 53));
				}
				const qd ref = sqrt(qd(a.high(), a.low()));
				const dd r = sqrt(a);
				const double err = std::abs(double((qd(r.high(), r.low()) - ref) / ref)) / 0x1p-106;
				worst = std::max(worst, err);
				if (!(err <= bound)) {
					if (reportTestCases && nrOfFailedTestCases < 10) std::cerr << "FAIL : sqrt(" << a << ") is off by " << err << " units of 2^-106\n";
					++nrOfFailedTestCases;
				}
			}
			if (reportTestCases) std::cout << "  worst " << worst << " units of 2^-106 over " << samples << " arguments\n";
			return nrOfFailedTestCases;
		}

		template<typename DoubleDouble>
		int VerifyCbrtFunction(bool reportTestCases, DoubleDouble a) {
			using std::cbrt;
			int nrOfFailedTestCases{ 0 };
			DoubleDouble b{ a };
			for (int i = 0; i < 6; ++i) {
				a *= a * a;
				dd c = cbrt(a);
				if (b != c) {
					if (reportTestCases) std::cerr << "FAIL : " << b << " != " << c << '\n';
					++nrOfFailedTestCases;
				}
				b *= b * b;
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

	std::string test_suite  = "double-double mathlib sqrt/cbrt function validation";
	std::string test_tag    = "sqrt/cbrt";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	auto defaultPrecision = std::cout.precision();

	GenerateSqrtTestCase<double>(1.0);
	GenerateSqrtTestCase<double>(1024.0 * 1024.0);
	constexpr double minpos = std::numeric_limits<double>::min();
	GenerateSqrtTestCase<double>(minpos);
	constexpr double maxpos = std::numeric_limits<double>::max();
	GenerateSqrtTestCase<double>(maxpos);

	std::cout << std::setprecision(defaultPrecision);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

	test_tag = "sqrt";
	nrOfFailedTestCases += ReportTestResult(VerifySqrtFunction(reportTestCases, dd(2.0)), "sqrt(dd > 1.0)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtFunction(reportTestCases, dd(0.5)), "sqrt(dd < 1.0)", test_tag);

	// accuracy against a qd reference (#1883)
#if REGRESSION_LEVEL_4
	constexpr int accuracySamples = 1000000;
#elif REGRESSION_LEVEL_2
	constexpr int accuracySamples = 200000;
#else
	constexpr int accuracySamples = 50000;
#endif
	nrOfFailedTestCases += ReportTestResult(VerifySqrtAccuracy(reportTestCases, 0, accuracySamples, 2.5), "sqrt accuracy [1,4)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySqrtAccuracy(reportTestCases, 1, accuracySamples, 2.5), "sqrt accuracy 2^[-1000,1000]", test_tag);

	test_tag = "cbrt";
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtFunction(reportTestCases, dd(2.0)), "cbrt(dd > 1.0)", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyCbrtFunction(reportTestCases, dd(0.5)), "cbrt(dd < 1.0)", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyCbrtFunction(reportTestCases, 2.0), "cbrt(double > 1.0)", test_tag);

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
