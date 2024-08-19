// ieee754.cpp : native IEEE-754 operations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/utility/bit_cast.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	int VerifyFloatingPointScales(bool reportTestCases) {
		using namespace sw::universal;
		int nrOfFailedTests = 0;

		long long largestScale = std::numeric_limits<Real>::max_exponent - 1;
		Real r = sw::universal::ipow<Real>(static_cast<size_t>(largestScale));
		for (long long i = 0; i < largestScale + 1; ++i) {
			if (largestScale - i != scale(r)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL : " << std::setw(4) << largestScale - i << " : " << scale(r) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
			}
			r /= 2.0;
		}
		// this gets us to 1.0, next enumerate the negative scaled normals
		int smallestScale = std::numeric_limits<Real>::min_exponent - 1;
		for (int i = -1; i > smallestScale; --i) {
			if (i != scale(r)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL : " << std::setw(4) << i << " : " << scale(r) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
			}
			r /= 2.0;
		}
		// this gets us to the smallest normal, next enumerate the subnormals
		for (int i = 0; i < sw::universal::ieee754_parameter<Real>::fbits; ++i) {
			if (smallestScale - i != scale(r)) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << std::setw(4) << (smallestScale - i) << " : " << scale(r) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
			}
			r /= 2.0;
		}

		return nrOfFailedTests;
	}

} } // namespace sw::universal

template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
void NativeEnvironment(Real r) {
	using namespace sw::universal;

	std::cout << "scale of " << r << " is 2^" << scale(r) << " ~ 10^" << int(scale(r) / 3.3) << '\n';
	std::cout << to_binary(r, true) << " " << r << '\n';
	std::cout << color_print(r) << " " << r << '\n';
}

template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
void DescendingScales() {
	std::string size("unknown");

	switch (sizeof(Real)) {
	case 4:
		size = "single";
		break;
	case 8:
		size = "double";
		break;
	case 16:
		size = "quadruple";
		break;
	}
	std::cout << "IEEE-754 " << size << " precision scales:             in descending order\n";

	auto oldPrecision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Real>::digits10);

	long long largestScale = std::numeric_limits<Real>::max_exponent - 1;
	Real r = sw::universal::ipow<double>(largestScale);
	for (long long i = 0; i < largestScale + 1; ++i) {
		std::cout << std::setw(4) << largestScale - i << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		r /= 2.0;
	}
	// this gets us to 1.0, next enumerate the negative scaled normals
	int smallestScale = std::numeric_limits<Real>::min_exponent - 1;
	for (int i = -1; i > smallestScale; --i) {
		std::cout << std::setw(4) << i << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		r /= 2.0;
	}
	// this gets us to the smallest normal, next enumerate the subnormals
	for (int i = 0; i < sw::universal::ieee754_parameter<Real>::fbits; ++i) {
		std::cout << std::setw(4) << (smallestScale - i) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		r /= 2.0;
	}
	std::cout << std::setprecision(oldPrecision);
}

template<typename RealType,
	     typename = typename std::enable_if< std::is_floating_point<RealType>::value, RealType>::type>
void InfinityAdditions() {
	std::cout << "IEEE-754 addition with infinites\n";
	constexpr RealType fa = std::numeric_limits<RealType>::infinity();
	constexpr RealType fb = -fa;
	constexpr unsigned COLWITH = 15;
	std::cout << std::setw(COLWITH) << fa << " + " << std::setw(COLWITH) << fa << " = " << std::setw(COLWITH) << (fa + fa) << " : " << sw::universal::to_binary(fa + fa) << '\n';
	std::cout << std::setw(COLWITH) << fa << " + " << std::setw(COLWITH) << fb << " = " << std::setw(COLWITH) << (fa + fb) << " : " << sw::universal::to_binary(fa + fb) << '\n';
	std::cout << std::setw(COLWITH) << fb << " + " << std::setw(COLWITH) << fa << " = " << std::setw(COLWITH) << (fb + fa) << " : " << sw::universal::to_binary(fb + fa) << '\n';
	std::cout << std::setw(COLWITH) << fb << " + " << std::setw(COLWITH) << fb << " = " << std::setw(COLWITH) << (fb + fb) << " : " << sw::universal::to_binary(fb + fb) << '\n';
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

	std::string test_suite  = "IEEE-754 floating-point operators";
	std::string test_tag    = "special cases";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// compare bits of different real number representations
	
	float f         = 1.0e1;
	double d        = 1.0e10;
#if LONG_DOUBLE_SUPPORT
	long double ld  = 1.0e100;
#else
	std::cout << "This environment does not support a native long double format\n";
#endif

	NativeEnvironment(f);
	NativeEnvironment(d);
#if LONG_DOUBLE_SUPPORT
	NativeEnvironment(ld);
#endif

	// show all the different presentations for the different IEEE-754 native formats
	valueRepresentations(f);
	valueRepresentations(d);
#if LONG_DOUBLE_SUPPORT
	valueRepresentations(ld);
#endif

	// show the scales that an IEEE-754 type contains
	DescendingScales<float>();

	// show the results of addition with infinites
	InfinityAdditions<float>();

	int largestScale = std::numeric_limits<float>::max_exponent - 1;
	float r = sw::universal::ipow<float>(static_cast<size_t>(largestScale));
	std::cout << "largest scale  : " << largestScale << " value : " << r << '\n';
	int smallestScale = std::numeric_limits<float>::min_exponent - 1;
	r = sw::universal::ipow<float>(static_cast<size_t>(smallestScale));
	std::cout << "smallest scale : " << smallestScale << " value : " << r << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<float>(reportTestCases), "float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<double>(reportTestCases), "double", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	// show the results of addition with infinites
	InfinityAdditions<float>();
	InfinityAdditions<double>();

	std::cout << "\nNative floating-point ranges\n";
	std::cout << float_range() << '\n';
	std::cout << double_range() << '\n';
	std::cout << longdouble_range() << '\n';

	std::cout << "\nTest cases\n";
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<float>(reportTestCases), "float", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<double>(reportTestCases), "double", test_tag);
#if LONG_DOUBLE_SUPPORT
	nrOfFailedTestCases += ReportTestResult(VerifyFloatingPointScales<long double>(reportTestCases), "long double", test_tag);
#endif

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
