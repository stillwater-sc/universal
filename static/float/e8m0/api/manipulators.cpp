// manipulators.cpp: e8m0 manipulator contracts
//
// e8m0 is eight exponent bits: no sign field, no fraction field, and therefore no field
// boundary. The tree convention is that '.' separates FIELDS and is emitted always, while
// '\'' marks NIBBLES and is emitted only when the caller asks. to_binary used to print a
// '.' at the nibble boundary unconditionally and ignore its nibbleMarker argument
// entirely, so mxfloat and nvblock, which forward their own flag into it, could not turn
// the marker on or off (#1434).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <string>

#include <universal/number/e8m0/e8m0.hpp>
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


	// ---- the nibble marker is emitted only when asked --------------------------

	int VerifyNibbleMarker(bool reportTestCases) {
		int fails = 0;

		const e8m0 v(4.0f);   // encoding 0x81
		fails += expect_text(to_binary(v, false), "0b10000001", "default: no separator", reportTestCases);
		fails += expect_text(to_binary(v, true),  "0b1000'0001", "flagged: a nibble marker", reportTestCases);

		// to_native forwards the flag rather than dropping it
		fails += expect_text(to_native(v, false), to_binary(v, false), "to_native, unflagged",
			reportTestCases);
		fails += expect_text(to_native(v, true), to_binary(v, true), "to_native, flagged", reportTestCases);
		fails += expect_true(to_native(v, true) != to_native(v, false),
			"the flag reaches to_native", reportTestCases);

		// the field separator belongs to formats that have fields; e8m0 has none
		fails += expect_true(to_binary(v, false).find('.') == std::string::npos,
			"no field separator by default", reportTestCases);
		fails += expect_true(to_binary(v, true).find('.') == std::string::npos,
			"and none when flagged either", reportTestCases);

		return fails;
	}

	// ---- every encoding renders its own eight bits ------------------------------

	int VerifyBitsRendered(bool reportTestCases) {
		int fails = 0;

		for (unsigned pattern = 0; pattern < 256u; ++pattern) {
			e8m0 v;
			v.setbits(static_cast<uint8_t>(pattern));

			std::string plain = to_binary(v, false);
			std::string marked = to_binary(v, true);
			if (plain.size() != 10u) {          // "0b" + 8 bits
				++fails;
				if (reportTestCases) std::cout << "    FAIL width of " << plain << '\n';
				continue;
			}
			if (marked.size() != 11u) {         // one marker added
				++fails;
				if (reportTestCases) std::cout << "    FAIL marked width of " << marked << '\n';
				continue;
			}
			// the rendered bits are the encoding, most significant first
			std::string wanted = "0b";
			for (int j = 7; j >= 0; --j) wanted += ((pattern & (1u << j)) ? '1' : '0');
			fails += expect_text(plain, wanted, "the rendered bits are the encoding", reportTestCases);
			// and removing the marker gives the same string back
			std::string stripped;
			for (char c : marked) if (c != '\'') stripped += c;
			fails += expect_text(stripped, plain, "the marker is the only difference", reportTestCases);
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

	std::string test_suite  = "e8m0 manipulators";
	std::string test_tag    = "e8m0 manipulators";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyNibbleMarker(reportTestCases), test_tag, "nibble marker");
	nrOfFailedTestCases += ReportTestResult(VerifyBitsRendered(reportTestCases), test_tag, "bits rendered");
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
