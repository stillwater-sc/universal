// trigonometry.cpp: test suite runner for trigonometry functions for quad-double (qd) floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <numbers>
#include <universal/number/qd/qd.hpp>
#include <universal/verification/test_suite.hpp>

template<typename Real>
int VerifySinFunction(bool reportTestCases) {
	using std::sin, std::abs;
	constexpr bool bTraceError{ false };
	int nrOfFailedTestCases{ 0 };

	const double d2pi       = 6.283185307179586476925286766559;
	//const double piOver4  = 0.78539816339744830961566084581988;
	//const double piOver8  = 0.39269908169872415480783042290994;
	//const double piOver16 = 0.19634954084936207740391521145497;
	const double piOver32   = 0.01227184630308512983774470071594;

	// walk the unit circle in steps of pi/32
	double dinc{ piOver32 };
	unsigned samples{ static_cast<unsigned>(d2pi / dinc) };
	Real increment{ piOver32 };
	for (unsigned i = 0; i < samples; ++i) {
		Real angle = Real(i) * increment;
		double dangle = double(i) * dinc;
		double ref = sin(dangle);
		Real result = sin(angle);
		Real error = abs(result - Real(ref));
		if (error > 1e-10) {
			if (reportTestCases) std::cerr << "sin( " << angle << ") : " << sin(angle) << " : error " << error << '\n';
			++nrOfFailedTestCases;
		}
		else {
			if constexpr (bTraceError) std::cerr << "sin( " << angle << ") : error " << error << '\n';
		}
	}

	return nrOfFailedTestCases;
}

template<typename Real>
int VerifyCosFunction(bool reportTestCases) {
	using std::cos, std::abs;
	constexpr bool bTraceError{ false };
	int nrOfFailedTestCases{ 0 };

	const double d2pi       = 6.283185307179586476925286766559;
	//const double piOver4  = 0.78539816339744830961566084581988;
	//const double piOver8  = 0.39269908169872415480783042290994;
	//const double piOver16 = 0.19634954084936207740391521145497;
	const double piOver32   = 0.01227184630308512983774470071594;

	// walk the unit circle in steps of pi/32
	double dinc{ piOver32 };
	unsigned samples{ static_cast<unsigned>(d2pi / dinc) };
	Real increment{ piOver32 };
	for (unsigned i = 0; i < samples; ++i) {
		Real angle = Real(i) * increment;
		double dangle = double(i) * dinc;
		double ref = cos(dangle);
		Real result = cos(angle);
		Real error = abs(result - Real(ref));
		if (error > 1e-10) {
			if (reportTestCases) std::cerr << "cos( " << angle << ") : " << cos(angle) << " : error " << error << '\n';
			++nrOfFailedTestCases;
		}
		else {
			if constexpr (bTraceError) std::cerr << "cos( " << angle << ") : error " << error << '\n';
		}
	}

	return nrOfFailedTestCases;
}

template<typename Real>
int VerifyTanFunction(bool reportTestCases) {
	using std::tan, std::abs;
	constexpr bool bTraceError{ false };
	int nrOfFailedTestCases{ 0 };

	const double d2pi      = 6.283185307179586476925286766559;
	//const double piOver2 = 1.5707963267948966192313216916398;
	//const double piOver4 = 0.78539816339744830961566084581988;
	//const double piOver8 = 0.39269908169872415480783042290994;
	//const double piOver16 = 0.19634954084936207740391521145497;
	const double piOver32 = 0.01227184630308512983774470071594;

	// walk the unit circle in steps of pi/32
	double dinc{ piOver32 };
	unsigned samples{ static_cast<unsigned>(d2pi / dinc) };
	Real increment{ piOver32 };
	// tan(x) is inf at pi/2 and 3pi/4
	// they are at 1/4 and 3/4s of the sample sequence
	for (unsigned i = 0; i < samples; ++i) {
		Real angle = Real(i) * increment;
		double dangle = double(i) * dinc;
		double ref = tan(dangle);
		Real result = tan(angle);
		Real error = abs(result - Real(ref));
		if (error > 1e-10) {
			if (i == samples / 4 || i == 3 * samples / 4) {
				// tan(x) approximation is expected to have a much smaller error
				// std::cout << samples << " : " << i << '\n';
				if (error > 1e-01) continue;
				std::cerr << "error : " << error << '\n';
			}
			if (reportTestCases) std::cerr << "tan( " << angle << ") : " << tan(angle) << " : error " << error << '\n';
			++nrOfFailedTestCases;
		}
		else {
			if constexpr (bTraceError) std::cerr << "tan( " << angle << ") : error " << error << '\n';
		}
	}

	return nrOfFailedTestCases;
}

template<typename Real>
int VerifyArcsinFunction(bool reportTestCases) {
	using std::asin, std::sin, std::abs;
	constexpr bool bTraceError{ false };
	int nrOfFailedTestCases{ 0 };

	// walk the domain of arcsin = [-1, 1] to the range of [ -pi/2, pi/2 ]
	int samples{ 64 };
	double dinc{ 2.0 / double(samples) };
	Real increment{ dinc };
	for (int i = -samples / 2; i < samples / 2; ++i) {
		Real   rx = Real(i) * increment;
		double dx = double(i) * dinc;
		// std::cout << "dx " << dx << '\n';
		double ref = asin(dx);
		Real result = asin(rx);
		Real error = abs(result - Real(ref));
		if (error > 1e-10) {
			if (reportTestCases) std::cout << "arcsin( " << rx << ") : " << asin(rx) << " : error " << error << '\n';
			++nrOfFailedTestCases;
		}
		else {
			if constexpr (bTraceError) std::cout << "arcsin( " << rx << ") : error " << error << '\n';
		}
	}

	return nrOfFailedTestCases;
}

template<typename Real>
int VerifyArccosFunction(bool reportTestCases) {
	using std::acos, std::cos, std::abs;
	constexpr bool bTraceError{ false };
	int nrOfFailedTestCases{ 0 };

	// walk the domain of arccos = [-1, 1] to the range of [0, pi]
	int samples{ 64 };
	double dinc{ 2.0 / double(samples) };
	Real increment{ dinc };
	for (int i = -samples / 2; i < samples / 2; ++i) {
		Real   rx = Real(i) * increment;
		double dx = double(i) * dinc;
		// std::cout << "dx " << dx << '\n';
		double ref = acos(dx);
		Real result = acos(rx);
		Real error = abs(result - Real(ref));
		if (error > 1e-10) {
			if (reportTestCases) std::cout << "arccos( " << rx << ") : " << acos(rx) << " : error " << error << '\n';
			++nrOfFailedTestCases;
		}
		else {
			if constexpr (bTraceError) std::cout << "arccos( " << rx << ") : error " << error << '\n';
		}
	}

	return nrOfFailedTestCases;
}

template<typename Real>
int VerifyArctanFunction(bool reportTestCases) {
	using std::atan, std::tan, std::abs;
	constexpr bool bTraceError{ false };
	int nrOfFailedTestCases{ 0 };

	// walk the domain of arctan = [ -inf, inf ] to the range of [ -pi/2, pi/2 ]
	// we are going to use tan(x) to generate the values to inverse
	const double d2pi     = 6.283185307179586476925286766559;
	const double piOver32 = 0.01227184630308512983774470071594;

	// walk the unit circle in steps of pi/32
	double dinc{ piOver32 };
	unsigned samples{ static_cast<unsigned>(d2pi / dinc) };
	Real increment{ piOver32 };
	// tan(x) is inf at pi/2 and 3pi/4
	// they are at 1/4 and 3/4s of the sample sequence
	for (unsigned i = 0; i < samples; ++i) {

		double dangle = double(i) * dinc;
		double dx = tan(dangle);

		Real angle = Real(i) * increment;
		Real rx = tan(angle);

		double ref = atan(dx);
		Real result = atan(rx);
		Real error = abs(result - Real(ref));
		if (error > 1e-10) {
			if (reportTestCases) std::cout << "arctan( " << rx << ") : " << atan(rx) << " : error " << error << '\n';
			++nrOfFailedTestCases;
		}
		else {
			if constexpr (bTraceError) std::cout << "arctan( " << rx << ") : error " << error << '\n';
		}
	}

	return nrOfFailedTestCases;
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

	std::string test_suite  = "quad-double mathlib trigonometry function validation";
	std::string test_tag    = "sin/cos/tan  asin/acos/atan";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	
	std::cout << "TRIGONOMETRY FUNCTIONS ARE SHIMS TO DOUBLE\n";

	std::cout << std::setw(10) << "sin(pi/4)" << " : " << sin(qd_pi4) << '\n';
	std::cout << std::setw(10) << "cos(pi/4)" << " : " << cos(qd_pi4) << '\n';
	std::cout << std::setw(10) << "tan(pi/4)" << " : " << tan(qd_pi4) << '\n';

	{
		qd a = sin(qd_pi4);
		qd b = asin(a);
		std::cout << "pi/4            : " << qd_pi4 << '\n';
		std::cout << "sin(pi/4)       : " << a << '\n';
		std::cout << "asin(sin(pi/4)  : " << b << '\n';
	}
//	std::cout << std::setw(10) << "asin(sin(pi/4))" << " : " << asin(sin(qd_pi4)) << '\n';
	std::cout << std::setw(10) << "acos(cos(pi/4))" << " : " << acos(cos(qd_pi4)) << '\n';
	std::cout << std::setw(10) << "atan(tan(pi/4))" << " : " << atan(tan(qd_pi4)) << '\n';

	VerifySinFunction<double>(reportTestCases);

	qd piOver4("0.78539816339744830961566084581988");
	qd piOver8("0.39269908169872415480783042290994");
	qd piOver16("0.19634954084936207740391521145497");
	qd piOver32("0.01227184630308512983774470071594");

	qd a = sin(piOver4);

	std::cout << "pi/4 : " << std::setprecision(32) << piOver4 << '\n';
	std::cout << "pi/8 : " << std::setprecision(32) << piOver8 << '\n';
	std::cout << "pi/16 : " << std::setprecision(32) << piOver16 << '\n';
	std::cout << "pi/32 : " << std::setprecision(32) << piOver32 << '\n';

	qd b{};
	b = asin(qd(0));
	std::cout << b << '\n';
	b = asin(qd(-1.0));
	std::cout << b << '\n';
	b = asin(qd(1.0));
	std::cout << b << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases = ReportTestResult(VerifySinFunction<qd>(reportTestCases)  , "   sin function", " sin(qd)");
	nrOfFailedTestCases = ReportTestResult(VerifyCosFunction<qd>(reportTestCases)  , "   cos function", " cos(qd)");
	nrOfFailedTestCases = ReportTestResult(VerifyTanFunction<qd>(reportTestCases)  , "   tan function", " tan(qd)");

	nrOfFailedTestCases = ReportTestResult(VerifyArcsinFunction<qd>(reportTestCases), "arcsin function", "asin(qd)");
	nrOfFailedTestCases = ReportTestResult(VerifyArccosFunction<qd>(reportTestCases), "arccos function", "acos(qd)");
	nrOfFailedTestCases = ReportTestResult(VerifyArctanFunction<qd>(reportTestCases), "arctan function", "atan(qd)");
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
