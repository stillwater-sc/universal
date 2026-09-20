// manipulators.cpp: every dd_cascade string producer, called
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// to_hex(dd_cascade) formats each limb with to_hex(double, bool, bool). When #1429 split
// dd_cascade into layers, manipulators.hpp stopped seeing that overload -- it lives in the
// text half of the native IEEE-754 support -- so the double converted back to dd_cascade
// and to_hex called itself until the stack overflowed. No test called it.
//
// INCLUDE ORDER IS THE POINT: dd_cascade.hpp comes first. test_reporters.hpp includes
// native/ieee754.hpp itself, so a test that included it earlier would supply the missing
// overload and hide exactly the defect this file exists to catch.
#include <universal/utility/directives.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <iostream>
#include <string>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

inline int VerifyToHex(bool reportTestCases) {
	int nrOfFailedTests = 0;
	for (double a : { 1.5, -3.0e-300, 1.0 / 3.0, 0.0, 1.0e300 }) {
		dd_cascade v = dd_cascade(a) / dd_cascade(7.0);
		for (bool nibbles : { false, true }) {
			for (bool third : { false, true }) {
				std::string expected = std::string("dd_cascade[") + to_hex(v.high(), nibbles, third) + ", "
				                     + to_hex(v.low(), nibbles, third) + "]";
				std::string actual = to_hex(v, nibbles, third);
				if (actual != expected) {
					++nrOfFailedTests;
					if (reportTestCases) std::cerr << "FAIL: to_hex " << actual << " != " << expected << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// the rest must produce text too; each call is a compile-and-run check, not a format check
//
// "produced text" alone is not enough of a check: info_print returned the literal "TBD"
// in nine other number systems and passed exactly this assertion in the areal suite,
// because "TBD" is not empty (#1556). dd_cascade's is a real delegate to pretty_print, so
// this is a guard against a regression rather than a fix -- it must not be a placeholder,
// and it must say something different about a different value.
inline int VerifyEveryManipulatorRuns(bool reportTestCases) {
	int nrOfFailedTests = 0;
	dd_cascade v = dd_cascade(1.0) / dd_cascade(3.0);
	dd_cascade w = dd_cascade(1.0) / dd_cascade(7.0);
	const std::string produced[] = {
		type_tag(v), to_pair(v), to_triple(v), to_binary(v), to_binary(v, true), to_native(v),
		to_components(v), color_print(v), pretty_print(v), info_print(v),
	};
	for (const std::string& s : produced) {
		if (s.empty()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: a manipulator produced an empty string\n";
		}
		if (s.find("TBD") != std::string::npos || s.find("tbd") != std::string::npos) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: a manipulator produced a placeholder: " << s << '\n';
		}
	}
	// the value-dependent renderers must read the value: type_tag is the one that is a
	// property of the type rather than of the value
	const std::string ofV[] = {
		to_pair(v), to_triple(v), to_binary(v), to_native(v),
		to_components(v), color_print(v), pretty_print(v), info_print(v),
	};
	const std::string ofW[] = {
		to_pair(w), to_triple(w), to_binary(w), to_native(w),
		to_components(w), color_print(w), pretty_print(w), info_print(w),
	};
	for (unsigned i = 0; i < sizeof(ofV) / sizeof(ofV[0]); ++i) {
		if (ofV[i] == ofW[i]) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: a manipulator rendered 1/3 and 1/7 identically: " << ofV[i] << '\n';
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "dd_cascade manipulators";
	std::string test_tag    = "manipulators";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << to_hex(dd_cascade(1.5)) << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyToHex(reportTestCases), "to_hex composes the limbs' native hex", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEveryManipulatorRuns(reportTestCases), "every manipulator", test_tag);
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
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
