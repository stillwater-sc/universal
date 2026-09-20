// manipulators.cpp: bfloat16 manipulator surface
//
// type_field(bfloat16) read BfloatType::fbits, which did not exist -- bfloat16 declared
// nbits and es only. A function template's body is not checked until it is instantiated,
// and nothing in the test suite instantiated this one, so it sat there uncompilable
// (#1453). This suite instantiates the WHOLE manipulator surface, which is the coverage
// gap that let it happen.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <string>

#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_suite.hpp>

// set to 1 to run the exploratory walk-through instead of the regression suite
#define MANUAL_TESTING 0

namespace {

	using namespace sw::universal;

	int expect_text(const std::string& actual, const std::string& wanted, const char* what, bool reportTestCases) {
		if (actual == wanted) return 0;
		if (reportTestCases) {
			std::cout << "    FAIL " << what << ": got \"" << actual << "\", expected \"" << wanted << "\"\n";
		}
		return 1;
	}

	int expect_true(bool actual, const char* what, bool reportTestCases) {
		if (actual) return 0;
		if (reportTestCases) std::cout << "    FAIL " << what << '\n';
		return 1;
	}


	// bfloat16 now exposes its fraction width, as every other float-like type does
	static_assert(bfloat16::nbits == 16, "bfloat16 is 16 bits");
	static_assert(bfloat16::es == 8, "with an 8-bit exponent");
	static_assert(bfloat16::fbits == 7, "and 7 explicit fraction bits");
	static_assert(bfloat16::nbits == 1u + bfloat16::es + bfloat16::fbits, "the fields tile the encoding");

	int VerifyTypeField(bool reportTestCases) {
		int fails = 0;

		fails += expect_text(type_field(bfloat16()), "fields(s:1|e:8|m:7)", "type_field", reportTestCases);
		// the default argument form has to work too
		fails += expect_text(type_field<bfloat16>(), "fields(s:1|e:8|m:7)", "type_field, defaulted",
			reportTestCases);

		return fails;
	}

	// ---- every manipulator is instantiated, which is the point of this suite --------
	//
	// Instantiating is not checking. A non-empty assertion is how info_print went on
	// returning the literal "TBD" through the equivalent list in the areal suite: "TBD" is
	// not empty (#1556). bfloat16 has no info_print at all -- it is one of the two dozen
	// number systems that do not -- but the same weak assertion guards everything here,
	// so it gets the same two strengthenings: not a placeholder, and different values
	// must render differently.

	int VerifyManipulatorSurface(bool reportTestCases) {
		int fails = 0;

		const bfloat16 v(1.5f);
		const bfloat16 w(2.5f);
		struct Named { std::string text; std::string other; const char* what; };
		const Named rendered[] = {
			{ type_tag(v),         type_tag(w),         "type_tag" },
			{ type_field(v),       type_field(w),       "type_field" },
			{ to_binary(v, false), to_binary(w, false), "to_binary" },
			{ to_binary(v, true),  to_binary(w, true),  "to_binary, marked" },
			{ to_hex(v),           to_hex(w),           "to_hex" },
			{ hex_print(v),        hex_print(w),        "hex_print" },
			{ to_triple(v),        to_triple(w),        "to_triple" },
			{ color_print(v),      color_print(w),      "color_print" },
		};
		for (const Named& r : rendered) {
			fails += expect_true(!r.text.empty(), r.what, reportTestCases);
			fails += expect_true(r.text.find("TBD") == std::string::npos
			                  && r.text.find("tbd") == std::string::npos, r.what, reportTestCases);
			// type_tag and type_field are properties of the type, not of the value, so they
			// are the two renderers allowed to say the same thing about both
			const std::string what(r.what);
			if (what != "type_tag" && what != "type_field") {
				fails += expect_true(r.text != r.other, r.what, reportTestCases);
			}
		}

		// the nibble marker reaches the renderers that take one
		fails += expect_true(to_binary(v, true) != to_binary(v, false),
			"to_binary honours the nibble marker", reportTestCases);

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

	std::string test_suite  = "bfloat16 manipulator surface";
	std::string test_tag    = "bfloat16 manipulators";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyTypeField(reportTestCases), test_tag, "type_field");
	nrOfFailedTestCases += ReportTestResult(VerifyManipulatorSurface(reportTestCases), test_tag, "manipulator surface");
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
