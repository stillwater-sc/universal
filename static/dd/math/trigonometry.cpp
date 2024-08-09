// trigonometry.cpp: test suite runner for trigonometry functions for double-double floating-point
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <numbers>
#include <universal/number/dd/dd.hpp>
#include <universal/verification/test_suite.hpp>

// generate specific test case
template<typename Ty>
void GenerateLogTestCase(Ty fa) {
	unsigned precision = 25;
	unsigned width = 30;
	Ty fref;
	sw::universal::dd a, ref, v;
	a = fa;
	fref = std::log(fa);
	ref = fref;
	v = sw::universal::log(a);
	auto oldPrec = std::cout.precision();
	std::cout << std::setprecision(precision);
	std::cout << " -> log(" << fa << ") = " << std::setw(width) << fref << std::endl;
	std::cout << " -> log( " << a << ") = " << v << '\n' << to_binary(v) << '\n';
	std::cout << to_binary(ref) << "\n -> reference\n";
	std::cout << (ref == v ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(oldPrec);
}

template<typename Real>
int VerifySinFunction(bool reportTestCases) {
	using std::sin;
	int nrOfFailedTestCases{ 0 };

	//const double piOver4  = 0.78539816339744830961566084581988;
	//const double piOver8  = 0.39269908169872415480783042290994;
	const double piOver16 = 0.19634954084936207740391521145497;
	//const double piOver32 = 0.01227184630308512983774470071594;

	// walk the unit circle in steps of pi/16
	Real increment{ piOver16 };
	Real angle{ 0.0 };
	for (unsigned i = 0; i < 32; ++i) {
		angle = Real(i) * increment;
		std::cout << "sin( " << angle << ") : " << sin(angle) << '\n';
	}

	return nrOfFailedTestCases;
}

template<typename Real>
int VerifyCosFunction(bool reportTestCases) {
	using std::cos, std::abs;
	int nrOfFailedTestCases{ 0 };

	//const double piOver4 = 0.78539816339744830961566084581988;
	//const double piOver8 = 0.39269908169872415480783042290994;
	const double piOver16 = 0.19634954084936207740391521145497;
	//const double piOver32 = 0.01227184630308512983774470071594;

	// walk the unit circle in steps of pi/16
	Real increment{ piOver16 };
	Real angle{ 0.0 };
	double dinc{ piOver16 };
	double dangle{ 0.0 };
	for (unsigned i = 0; i < 32; ++i) {
		angle = Real(i) * increment;
		dangle = double(i) * dinc;
		double ref = cos(dangle);
		Real result = cos(angle);
		Real error = abs(result - Real(ref));
		if (error > 1e-10) {
			if (reportTestCases) std::cout << "cos( " << angle << ") : " << cos(angle) << " : error " << error << '\n';
			++nrOfFailedTestCases;
		}
		else {
			std::cout << "cos( " << angle << ") : error " << error << '\n';
		}
	}

	return nrOfFailedTestCases;
}

template<typename Real>
int VerifyTanFunction(bool reportTestCases) {
	using std::tan;
	int nrOfFailedTestCases{ 0 };

	//const double piOver4 = 0.78539816339744830961566084581988;
	//const double piOver8 = 0.39269908169872415480783042290994;
	const double piOver16 = 0.19634954084936207740391521145497;
	//const double piOver32 = 0.01227184630308512983774470071594;

	// walk the unit circle in steps of pi/16
	Real increment{ piOver16 };
	Real angle{ 0.0 };
	for (unsigned i = 0; i < 32; ++i) {
		angle = Real(i) * increment;
		std::cout << "tan( " << angle << ") : " << tan(angle) << '\n';
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

	std::string test_suite  = "doubledouble mathlib trigonometry function validation";
	std::string test_tag    = "trigonometry";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	
	std::cout << std::setw(10) << "sin(pi/4)" << " : " << sin(dd_pi4) << '\n';
	std::cout << std::setw(10) << "cos(pi/4)" << " : " << cos(dd_pi4) << '\n';
	std::cout << std::setw(10) << "tan(pi/4)" << " : " << tan(dd_pi4) << '\n';

	std::cout << std::setw(10) << "asin(pi/4)" << " : " << asin(dd_pi4) << '\n';
	std::cout << std::setw(10) << "acos(pi/4)" << " : " << acos(dd_pi4) << '\n';
	std::cout << std::setw(10) << "atan(pi/4)" << " : " << atan(dd_pi4) << '\n';

	VerifySinFunction<double>(reportTestCases);

	dd piOver4("0.78539816339744830961566084581988");
	dd piOver8("0.39269908169872415480783042290994");
	dd piOver16("0.19634954084936207740391521145497");
	dd piOver32("0.01227184630308512983774470071594");

	dd a = sin(piOver4);

	std::cout << "pi/16 : " << std::setprecision(32) << piOver16 << '\n';

//	nrOfFailedTestCases = ReportTestResult(VerifySinFunction<dd>(reportTestCases), "sin function", "sin(dd)");
	nrOfFailedTestCases = ReportTestResult(VerifyCosFunction<dd>(reportTestCases), "cos function", "cos(dd)");
//	nrOfFailedTestCases = ReportTestResult(VerifyTanFunction<dd>(reportTestCases), "tan function", "tan(dd)");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else


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
