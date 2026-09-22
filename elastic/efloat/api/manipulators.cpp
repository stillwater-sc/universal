// manipulators.cpp: efloat manipulator surface
//
// to_hex, hex_print, pretty_print and color_print each returned the literal string "tbd"
// -- to_hex and hex_print over commented-out bodies that indexed a nibble(n) and an nbits
// an efloat does not have (#1582). info_print was the same stub until #1556.
//
// The assertions here are the ones a placeholder cannot satisfy: the text is not a
// placeholder, and -- the one that matters -- DIFFERENT VALUES RENDER DIFFERENTLY. A stub
// returns one constant for every input, so that single check catches the whole class
// without needing to know what any given renderer is supposed to emit.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <string>

#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	using Efloat = efloat<4>;

	bool is_placeholder(const std::string& text) {
		return text.find("TBD") != std::string::npos || text.find("tbd") != std::string::npos;
	}

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}

	int VerifyManipulatorSurface(bool reportTestCases) {
		int fails = 0;

		const Efloat v(1.5);
		const Efloat w(2.5);
		struct Named { std::string text; std::string other; const char* what; };
		const Named rendered[] = {
			{ type_tag(v),      type_tag(w),      "type_tag" },
			{ to_binary(v),     to_binary(w),     "to_binary" },
			{ to_triple(v),     to_triple(w),     "to_triple" },
			{ to_hex(v),        to_hex(w),        "to_hex" },
			{ hex_print(v),     hex_print(w),     "hex_print" },
			{ components(v),    components(w),    "components" },
			{ pretty_print(v),  pretty_print(w),  "pretty_print" },
			{ info_print(v),    info_print(w),    "info_print" },
			{ color_print(v),   color_print(w),   "color_print" },
		};
		for (const Named& r : rendered) {
			fails += expect_true(!r.text.empty(), r.what, reportTestCases);
			fails += expect_true(!is_placeholder(r.text), r.what, reportTestCases);
			// type_tag is a property of the type, not of the value
			if (std::string(r.what) != "type_tag") {
				fails += expect_true(r.text != r.other, r.what, reportTestCases);
			}
		}

		return fails;
	}

	// The four that #1582 repaired, in detail.
	int VerifyRepairedRenderings(bool reportTestCases) {
		int fails = 0;

		const Efloat v(1.5);

		// to_hex reports sign, scale and the significand words
		const std::string hex = to_hex(v);
		fails += expect_true(hex.rfind("0x", 0) == 0, "to_hex carries the hex prefix", reportTestCases);
		fails += expect_true(to_hex(v, false, false).rfind("0x", 0) != 0,
			"to_hex honours hexPrefix=false", reportTestCases);
		fails += expect_true(to_hex(v, true) != to_hex(v, false),
			"to_hex honours the nibble marker", reportTestCases);
		fails += expect_true(hex.find_first_of("0123456789ABCDEF") != std::string::npos,
			"to_hex carries hex digits", reportTestCases);

		// hex_print names the configuration -- the limb count -- and tags the type
		const std::string hp = hex_print(v);
		fails += expect_true(hp.find(to_hex(v)) != std::string::npos,
			"hex_print embeds to_hex", reportTestCases);
		fails += expect_true(!hp.empty() && hp.back() == 'e', "hex_print tags the type", reportTestCases);

		// pretty_print separates the fields
		const std::string pp = pretty_print(v);
		fails += expect_true(pp.find(':') != std::string::npos, "pretty_print separates fields", reportTestCases);
		fails += expect_true(pp.find_first_of("01") != std::string::npos,
			"pretty_print renders the significand in binary", reportTestCases);

		// color_print emits escape sequences, and strips to something non-empty
		const std::string cp = color_print(v);
		fails += expect_true(cp.find('\033') != std::string::npos,
			"color_print emits colour codes", reportTestCases);
		fails += expect_true(color_print(v, true) != color_print(v, false),
			"color_print honours the nibble marker", reportTestCases);

		// the special encodings render as themselves rather than as a significand
		fails += expect_true(to_hex(Efloat(0.0)).find(".0") != std::string::npos,
			"zero renders as zero", reportTestCases);
		fails += expect_true(!is_placeholder(pretty_print(Efloat(0.0))), "zero is not a placeholder", reportTestCases);

		// a negative value is distinguishable from its magnitude
		fails += expect_true(to_hex(Efloat(-1.5)) != to_hex(Efloat(1.5)),
			"the sign reaches to_hex", reportTestCases);
		fails += expect_true(pretty_print(Efloat(-1.5)) != pretty_print(Efloat(1.5)),
			"the sign reaches pretty_print", reportTestCases);

		return fails;
	}

}  // anonymous namespace

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

	std::string test_suite  = "efloat manipulator surface";
	std::string test_tag    = "efloat manipulators";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	{
		Efloat v(1.5);
		std::cout << "type_tag     : " << type_tag(v) << '\n';
		std::cout << "to_binary    : " << to_binary(v) << '\n';
		std::cout << "to_triple    : " << to_triple(v) << '\n';
		std::cout << "to_hex       : " << to_hex(v) << '\n';
		std::cout << "hex_print    : " << hex_print(v) << '\n';
		std::cout << "components   : " << components(v) << '\n';
		std::cout << "pretty_print : " << pretty_print(v) << '\n';
		std::cout << "info_print   : " << info_print(v) << '\n';
		std::cout << "color_print  : " << color_print(v) << '\n';
	}
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyManipulatorSurface(reportTestCases), test_tag, "manipulator surface");
	nrOfFailedTestCases += ReportTestResult(VerifyRepairedRenderings(reportTestCases), test_tag, "repaired renderings");
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
