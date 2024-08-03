// constants.cpp: test suite runner for creating and verifying doubledouble constants
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>
#include <universal/verification/test_suite_randoms.hpp>

namespace sw {
	namespace universal {

		sw::universal::dd GenerateDoubleDouble(const std::string& str) {
			using namespace sw::universal;
			dd v(str);
			auto oldPrec = std::cout.precision();
			// 53 bits = 16 decimal digits, 17 to include last, 15 typical valid digits
			std::cout << std::setprecision(std::numeric_limits<double>::max_digits10);
			std::cout << to_pair(v) << '\n';
			std::cout << std::setprecision(oldPrec);
			return v;
		}

		void report(const sw::universal::dd& v, int precision = 17) {
			auto oldPrec = std::cout.precision();
			std::cout << std::setprecision(precision) << to_pair(v) << " : " << v << '\n';
			std::cout << std::setprecision(oldPrec);
		}

		void EnumerateConstants() {
			dd _zero("0.0"); report(_zero);
			dd _one("1.0"); report(_one);
			dd _ten("10.0"); report(_ten);

			dd _tenth("0.1"); report(_tenth);
			dd _third("0.333333333333333333333333333333333333"); report(_third);

			dd _2pi("6.283185307179586476925286766559005768"); report(_2pi);
			dd _pi("3.141592653589793238462643383279502884"); report(_pi);
			dd _pi2("1.570796326794896619231321691639751442"); report(_pi2);
			dd _pi4("0.785398163397448309615660845819875721"); report(_pi4);
			dd _3pi4 = _pi2 + _pi4;	report(_3pi4);

			dd _e("2.718281828459045235360287471352662498"); report(_e);

			dd _ln2("0.693147180559945309417232121458176568"); report(_ln2);
			dd _ln10("2.302585092994045684017991454684364208"); report(_ln10);

			dd _lge("1.442695040888963407359924681001892137"); report(_lge);
			dd _lg10("3.321928094887362347870319429489390176"); report(_lg10);

			dd _log2("0.301029995663981195213738894724493027"); report(_log2);
			dd _loge("0.434294481903251827651128918916605082"); report(_loge);

			dd _sqrt2("1.414213562373095048801688724209698079"); report(_sqrt2);

			dd _inv_pi("0.318309886183790671537767526745028724"); report(_inv_pi);
			dd _inv_pi2("0.636619772367581343075535053490057448"); report(_inv_pi2);
			dd _inv_e("0.367879441171442321595523770161460867"); report(_inv_e);
			dd _inv_sqrt2("0.707106781186547524400844362104849039"); report(_inv_sqrt2);
		}

	}
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

	std::string test_suite         = "doubledouble conversion validation";
	std::string test_tag           = "doubledouble conversion";
	bool reportTestCases           = false;
	int nrOfFailedTestCases        = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	dd a, b, c;
	double _third = 0.3333333333333333333333333333333;
	double _third2 = _third * pow(2.0, -53.0);
	double _short = 0.3333333333333333;
	ReportValue(_short, "0.3333333333333333", 35, 32);
	ReportValue(_third, "0.3333333333333333333333333333333", 35, 32);

	a = _third;
	b = _third2;
	ReportValue(a, "0.3333....", 35, 32);
	ReportValue(b, "0.3333....", 35, 32);
	c = a + b;
	ReportValue(c, "0.3333....", 35, 32);
	std::cout << to_pair(c) << '\n';

	dd d(_third, _third2);
	ReportValue(d, "0.3333....", 35, 32);
	std::cout << to_pair(d) << '\n';

	dd e("0.3333333333333333333333333333333333333333333333333");
	ReportValue(e, "0.3333....", 35, 32);
	std::cout << to_pair(e) << '\n';

	dd f(0.3333333333333333, 1.8503717077085935e-17);
	ReportValue(f, "0.3333....", 35, 32);
	std::cout << to_pair(f) << '\n';



	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else  // !MANUAL_TESTING

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
