// manipulators.cpp: areal manipulator surface
//
// components(areal) declared blockbinary<v.fbits> -- a non-static member in a constant
// expression -- and called a decode() overload that does not exist. Like
// type_field(bfloat16), it was never instantiated, so it was never compiled (#1453). It
// also printed "TBD" for the fraction and the uncertainty bit in place of the value.
// This suite instantiates the whole manipulator surface.
//
// Instantiating is not checking, though: info_print went on returning the literal "TBD"
// through the surface list below, because the assertion was that the text is non-empty --
// and "TBD" is not empty (#1556). The surface check now also rejects a placeholder and
// requires that a renderer say something different about a different value, which a
// constant cannot do, and info_print has a check of its own.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <sstream>
#include <string>

#include <universal/number/areal/areal.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	bool is_placeholder(const std::string& text) {
		return text.find("TBD") != std::string::npos || text.find("tbd") != std::string::npos;
	}


	using Areal = areal<8, 2>;

	int VerifyComponents(bool reportTestCases) {
		int fails = 0;

		const Areal v(1.5f);
		const std::string text = components(v);

		// every field is named, and the value is the value -- not the uncertainty bit,
		// and not the literal "TBD" the fraction used to render as
		for (const char* field : { "Sign", "Exponent", "Fraction", "Uncertainty", "Value" }) {
			fails += expect_true(text.find(field) != std::string::npos, field, reportTestCases);
		}
		fails += expect_true(text.find("TBD") == std::string::npos,
			"the fraction is rendered, not stubbed", reportTestCases);
		fails += expect_true(text.find("1.5") != std::string::npos,
			"the value is the value", reportTestCases);

		// the exponent and fraction fields are rendered at their declared widths
		fails += expect_true(components(Areal(0.0f)).size() > 0, "a zero renders too", reportTestCases);
		fails += expect_true(components(Areal(-1.5f)).find("Sign :  1") != std::string::npos,
			"a negative value reports its sign", reportTestCases);

		return fails;
	}

	// ---- to_string renders what operator<< renders -----------------------------------
	//
	// It used to return an empty string for every value but zero and infinity (#1554).
	// operator<< is the reference: [v] for an exact value, (v, next) for an uncertain one.

	int VerifyToString(bool reportTestCases) {
		int fails = 0;

		for (unsigned bits = 0; bits < 256u; ++bits) {
			Areal a;
			a.setbits(bits);
			const std::string text = to_string(a);
			std::stringstream reference;
			reference << a;
			if (text.empty() || text != reference.str()) {
				++fails;
				if (reportTestCases && fails < 5) {
					std::cout << "    FAIL encoding " << bits << ": to_string '" << text
					          << "' vs operator<< '" << reference.str() << "'\n";
				}
			}
		}

		// the shapes, spelled out
		fails += expect_true(to_string(Areal(1.5f)) == "[1.5]", "an exact value is bracketed", reportTestCases);
		fails += expect_true(to_string(Areal(0.0f)) == "[0]", "so is zero", reportTestCases);
		{
			Areal third(1.0f / 3.0f);
			fails += expect_true(third.ubit(), "1/3 is uncertain in areal<8,2>", reportTestCases);
			const std::string t = to_string(third);
			fails += expect_true(!t.empty() && t.front() == '(' && t.back() == ')',
				"an uncertain value is an open interval", reportTestCases);
		}

		return fails;
	}

	// ---- every manipulator is instantiated ------------------------------------------
	//
	// Instantiating them proves only that they compile. Asserting they are non-empty is
	// how info_print went on returning the literal "TBD" through this very list: "TBD" is
	// not empty (#1556). So each one must also not be a placeholder, and must say
	// something different about a different value -- which a constant cannot do.

	int VerifyManipulatorSurface(bool reportTestCases) {
		int fails = 0;

		const Areal v(1.5f);
		const Areal w(2.5f);
		struct Named { std::string text; std::string other; const char* what; };
		const Named rendered[] = {
			{ type_tag(v),     type_tag(w),     "type_tag" },
			{ to_binary(v),    to_binary(w),    "to_binary" },
			{ to_hex(v),       to_hex(w),       "to_hex" },
			{ hex_print(v),    hex_print(w),    "hex_print" },
			{ to_string(v),    to_string(w),    "to_string" },
			{ components(v),   components(w),   "components" },
			{ pretty_print(v), pretty_print(w), "pretty_print" },
			{ info_print(v),   info_print(w),   "info_print" },
			{ color_print(v),  color_print(w),  "color_print" },
		};
		for (const Named& r : rendered) {
			fails += expect_true(!r.text.empty(), r.what, reportTestCases);
			fails += expect_true(!is_placeholder(r.text), r.what, reportTestCases);
			// type_tag is a property of the type, not of the value, so it is the one
			// renderer that is allowed to say the same thing about both
			if (std::string(r.what) != "type_tag") {
				fails += expect_true(r.text != r.other, r.what, reportTestCases);
			}
		}

		return fails;
	}

	// ---- info_print reports the encoding and the interval ---------------------------
	//
	// It returned the literal "TBD" and ignored both of its arguments (#1556).

	int VerifyInfoPrint(bool reportTestCases) {
		int fails = 0;

		const std::string text = info_print(Areal(1.5f));
		fails += expect_true(text.rfind("raw: ", 0) == 0, "info_print leads with the raw encoding", reportTestCases);
		fails += expect_true(text.find(to_binary(Areal(1.5f))) != std::string::npos,
			"the raw field is the encoding", reportTestCases);
		fails += expect_true(text.find(" : value ") != std::string::npos, "info_print reports a value", reportTestCases);
		fails += expect_true(text.find("[1.5]") != std::string::npos,
			"an exact value is the bracketed value", reportTestCases);
		fails += expect_true(text.find(" u0") != std::string::npos,
			"an exact value reports an unset uncertainty bit", reportTestCases);

		// the uncertainty bit is what an areal is for, so it has to reach the rendering
		{
			Areal third(1.0f / 3.0f);
			fails += expect_true(third.ubit(), "1/3 is uncertain in areal<8,2>", reportTestCases);
			const std::string t = info_print(third);
			fails += expect_true(t.find(" u1") != std::string::npos,
				"an uncertain value reports a set uncertainty bit", reportTestCases);
			const std::size_t valueField = t.find(" : value ");
			fails += expect_true(valueField != std::string::npos && t[valueField + 9] == '(',
				"an uncertain value renders as an open interval", reportTestCases);
		}

		// a negative value is distinguishable from its magnitude
		fails += expect_true(info_print(Areal(-1.5f)) != info_print(Areal(1.5f)),
			"the sign reaches the rendering", reportTestCases);

		// printPrecision is the second argument and must not be ignored
		{
			Areal third(1.0f / 3.0f);
			fails += expect_true(info_print(third, 3).size() <= info_print(third, 17).size(),
				"a wider printPrecision does not shorten the rendering", reportTestCases);
		}

		return fails;
	}

}  // anonymous namespace

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

	std::string test_suite  = "areal manipulator surface";
	std::string test_tag    = "areal manipulators";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyComponents(reportTestCases), test_tag, "components");
	nrOfFailedTestCases += ReportTestResult(VerifyManipulatorSurface(reportTestCases), test_tag, "manipulator surface");
	nrOfFailedTestCases += ReportTestResult(VerifyToString(reportTestCases), test_tag, "to_string");
	nrOfFailedTestCases += ReportTestResult(VerifyInfoPrint(reportTestCases), test_tag, "info_print");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
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
