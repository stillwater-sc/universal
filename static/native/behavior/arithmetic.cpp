// arithmetic.cpp : test runner for arithmetic behavior enums
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <string>
#include <universal/behavior/arithmetic.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// Verify that Behavior enum values are correctly defined
	int VerifyBehaviorEnum(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test that the enum values exist and have distinct underlying values
		constexpr uint8_t saturating = static_cast<uint8_t>(Behavior::Saturating);
		constexpr uint8_t wrapping = static_cast<uint8_t>(Behavior::Wrapping);

		if (saturating == wrapping) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: Behavior::Saturating and Behavior::Wrapping have same underlying value\n";
		}

		// Verify enum can be used in constexpr context
		constexpr Behavior b1 = Behavior::Saturating;
		constexpr Behavior b2 = Behavior::Wrapping;

		if (b1 == b2) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: Behavior::Saturating == Behavior::Wrapping\n";
		}

		// Test comparison operators
		if (!(b1 != b2)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: !(Behavior::Saturating != Behavior::Wrapping)\n";
		}

		return nrOfFailedTests;
	}

	// Verify the type_tag function returns correct strings
	int VerifyBehaviorTypeTag(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test Saturating
		{
			std::string tag = type_tag(Behavior::Saturating);
			if (tag != "Saturating") {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: type_tag(Behavior::Saturating) = \"" << tag << "\" (expected \"Saturating\")\n";
			}
		}

		// Test Wrapping
		{
			std::string tag = type_tag(Behavior::Wrapping);
			if (tag != "Wrapping") {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: type_tag(Behavior::Wrapping) = \"" << tag << "\" (expected \"Wrapping\")\n";
			}
		}

		// Test unknown value (cast an invalid value to trigger default case)
		{
			Behavior unknown = static_cast<Behavior>(255);
			std::string tag = type_tag(unknown);
			if (tag != "unknown arithmetic behavior") {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: type_tag(unknown) = \"" << tag << "\" (expected \"unknown arithmetic behavior\")\n";
			}
		}

		return nrOfFailedTests;
	}

	// Verify Behavior can be used as a template parameter
	template<Behavior behavior>
	int VerifyBehaviorAsTemplateParam(bool reportTestCases) {
		int nrOfFailedTests = 0;

		// Test that we can use the behavior in constexpr if
		if constexpr (behavior == Behavior::Saturating) {
			if (reportTestCases) std::cout << "Template instantiated with Saturating behavior\n";
		}
		else if constexpr (behavior == Behavior::Wrapping) {
			if (reportTestCases) std::cout << "Template instantiated with Wrapping behavior\n";
		}

		// Verify type_tag works with template parameter
		std::string expected = (behavior == Behavior::Saturating) ? "Saturating" : "Wrapping";
		std::string tag = type_tag(behavior);
		if (tag != expected) {
			++nrOfFailedTests;
			if (reportTestCases) std::cerr << "FAIL: type_tag in template = \"" << tag << "\" (expected \"" << expected << "\")\n";
		}

		return nrOfFailedTests;
	}

	// Verify Behavior works in switch statements (common usage pattern)
	int VerifyBehaviorSwitch(bool reportTestCases) {
		int nrOfFailedTests = 0;

		auto testSwitch = [&](Behavior b, const std::string& expected) {
			std::string result;
			switch (b) {
			case Behavior::Saturating:
				result = "saturating";
				break;
			case Behavior::Wrapping:
				result = "wrapping";
				break;
			default:
				result = "unknown";
				break;
			}
			if (result != expected) {
				++nrOfFailedTests;
				if (reportTestCases) std::cerr << "FAIL: switch result = \"" << result << "\" (expected \"" << expected << "\")\n";
			}
		};

		testSwitch(Behavior::Saturating, "saturating");
		testSwitch(Behavior::Wrapping, "wrapping");
		testSwitch(static_cast<Behavior>(255), "unknown");

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "arithmetic behavior verification";
	std::string test_tag    = "Behavior enum";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	reportTestCases = true;
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorEnum(reportTestCases), "Behavior", "enum values");
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorTypeTag(reportTestCases), "Behavior", "type_tag");
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorAsTemplateParam<Behavior::Saturating>(reportTestCases), "Behavior", "template<Saturating>");
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorAsTemplateParam<Behavior::Wrapping>(reportTestCases), "Behavior", "template<Wrapping>");
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorSwitch(reportTestCases), "Behavior", "switch statement");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorEnum(reportTestCases), "Behavior", "enum values");
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorTypeTag(reportTestCases), "Behavior", "type_tag");
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorAsTemplateParam<Behavior::Saturating>(reportTestCases), "Behavior", "template<Saturating>");
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorAsTemplateParam<Behavior::Wrapping>(reportTestCases), "Behavior", "template<Wrapping>");
	nrOfFailedTestCases += ReportTestResult(VerifyBehaviorSwitch(reportTestCases), "Behavior", "switch statement");
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
