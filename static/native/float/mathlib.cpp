//  mathlib.cpp : universal math library wrapper
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <math/mathlib_shim.hpp>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename Scalar>
	Scalar UniversalMathlibShim(Scalar a, Scalar b = Scalar(1)) {
		using namespace sw::universal;
		using std::abs;
		using std::fpclassify;
		using std::isnormal;
		using std::isinf;
		using std::isnan;
		using std::isfinite;
		using std::erf;
		using std::erfc;
		using std::exp;
		using std::exp2;
//		using std::exp10;
		using std::expm1;
		using std::log;
		using std::log2;
		using std::log10;
		using std::log1p;
		using std::fmod;
		using std::remainder;
//		using std::frac;
		using std::sin;
		using std::cos;
		using std::tan;
		using std::asin;
		using std::acos;
		using std::atan;
		using std::sinh;
		using std::cosh;
		using std::tanh;
		using std::atanh;
		using std::acosh;
		using std::asinh;
		using std::hypot;
		using std::min;
		using std::max;
		using std::pow;
		using std::sqrt;
		using std::trunc;
		using std::round;
		using std::floor;
		using std::ceil;

		Scalar result{ 0 };

		std::cout << "arithmetic type   : " << type_tag(Scalar()) << '\n';
		std::cout << "abs               : " << abs(a) << '\n';

		std::cout << "fpclassify        : " << fpclassify(a) << '\n';
		std::cout << "isnormal          : " << isnormal(a) << '\n';
		std::cout << "isinf             : " << isinf(a) << '\n';
		std::cout << "isnan             : " << isnan(a) << '\n';
		std::cout << "isfinite          : " << isfinite(a) << '\n';
		std::cout << "isdenorm          : " << isdenorm(a) << '\n';

		std::cout << "erf               : " << erf(a) << '\n';
		std::cout << "erfc              : " << erfc(a) << '\n';

		std::cout << "exp               : " << exp(a) << '\n';
		std::cout << "exp2              : " << exp2(a) << '\n';
		std::cout << "exp10             : " << exp10(a) << '\n';
		std::cout << "expm1             : " << expm1(a) << '\n';

		std::cout << "log               : " << log(a) << '\n';
		std::cout << "log2              : " << log2(a) << '\n';
		std::cout << "log10             : " << log10(a) << '\n';
		std::cout << "log1p             : " << log1p(a) << '\n';

		std::cout << "fmod              : " << fmod(a, b) << '\n';
		std::cout << "remainder         : " << remainder(a, b) << '\n';
		std::cout << "frac              : " << frac(a) << '\n';

		std::cout << "sin               : " << sin(a) << '\n';
		std::cout << "cos               : " << cos(a) << '\n';
		std::cout << "tan               : " << tan(a) << '\n';
		std::cout << "atan              : " << atan(a) << '\n';
		std::cout << "acos              : " << acos(a) << '\n';
		std::cout << "asin              : " << asin(a) << '\n';

		std::cout << "sinh              : " << sinh(a) << '\n';
		std::cout << "cosh              : " << cosh(a) << '\n';
		std::cout << "tanh              : " << tanh(a) << '\n';
		std::cout << "atanh             : " << atanh(a) << '\n';
		std::cout << "acosh             : " << acosh(a) << '\n';
		std::cout << "asinh             : " << asinh(a) << '\n';

		std::cout << "hypot             : " << hypot(a, b) << '\n';

		std::cout << "min               : " << min(a, b) << '\n';
		std::cout << "max               : " << max(a, b) << '\n';

		std::cout << "pow               : " << pow(a, b) << '\n';

		std::cout << "sqrt              : " << sqrt(a) << '\n';

		std::cout << "trunc             : " << trunc(a) << '\n';
		std::cout << "round             : " << round(a) << '\n';
		std::cout << "floor             : " << floor(a) << '\n';
		std::cout << "ceil              : " << ceil(a) << '\n';

		return result;
	}

	template<typename Scalar>
	int VerifyMathlibShim(bool reportTestCases) {

		int nrOfFailedTests = 0;

		Scalar one(1);
		UniversalMathlibShim(one);
		if (reportTestCases) std::cout << "done\n";

		return nrOfFailedTests;
	}

} } // namespace sw::universal

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

	std::string test_suite  = "Universal mathlib shim verification";
	std::string test_tag    = "mathlib shim";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// test Class Template Argument Deduction (CTAD) for elementary functions
	
	{
		float f         = 1.5e-1f;
		MathlibShim(f);
	}

	{
		double d         = 1.5e-1;
		MathlibShim(d);
	}

	nrOfFailedTestCases += ReportTestResult(VerifyMathlibShim< cfloat<8, 2> >(reportTestCases), "cfloat<8,2>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyMathlibShim< posit<8,2> >(reportTestCases), "posit<8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMathlibShim< cfloat<8,2> >(reportTestCases), "cfloat<8,2>", test_tag);

#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if	REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
