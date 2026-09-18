// manipulators.cpp: hfloat manipulator contracts
//
// hfloat has no single zero encoding: ANY exponent field with an all-zero fraction is a
// zero. unpack() hides that redundancy by defining a zero's exponent as 0. to_hex did not
// -- it decoded the stored exponent and subtracted the bias unconditionally -- so two
// values that compare equal printed different exponents, e.g. 16^-64 and 16^-63 (#1448).
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <string>

#include <universal/number/hfloat/hfloat.hpp>
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


	using Hfloat = hfloat<8, 7, std::uint32_t>;

	// Build a zero carrying a chosen exponent field. The exponent's most significant bit
	// sits at nbits-2 (just below the sign), so the field starts at nbits-1-es; every
	// fraction bit stays clear, which is what makes the encoding a zero.
	Hfloat zero_with_exponent(unsigned expField) {
		Hfloat v;
		v.setbits(static_cast<std::uint64_t>(expField) << (Hfloat::nbits - 1u - Hfloat::es));
		return v;
	}

	// ---- every zero renders alike, whatever exponent its encoding carries ----------

	int VerifyZeroRendering(bool reportTestCases) {
		int fails = 0;

		Hfloat canonical;
		canonical.setzero();
		const std::string wanted = to_hex(canonical);

		// the exponent field spans the whole range; all of these are zeros
		for (unsigned expField = 0; expField < (1u << Hfloat::es); ++expField) {
			Hfloat z = zero_with_exponent(expField);
			// the value is zero -- the fraction is what carries it -- even though
			// iszero() is stricter and wants the exponent field clear too
			fails += expect_true(double(z) == 0.0, "the encoding is a zero", reportTestCases);
			fails += expect_text(to_hex(z), wanted, "every zero renders alike", reportTestCases);
			// to_native delegates to to_hex, so it follows
			fails += expect_text(to_native(z), to_native(canonical), "to_native follows", reportTestCases);
			// and the exponent printed is 0
			fails += expect_true(to_hex(z).find("16^0") != std::string::npos,
				"a zero prints exponent 0", reportTestCases);
		}

		// the canonical zero, where iszero() and the fraction test agree, unpacks to 0
		{
			bool s{ true };
			int e{ 99 };
			std::uint64_t f{ 7 };
			canonical.unpack(s, e, f);
			fails += expect_true(e == 0 && f == 0, "unpack reports exponent 0", reportTestCases);
		}

		return fails;
	}

	// ---- a value that is not zero still renders its own exponent --------------------

	int VerifyNonZeroKeepsItsExponent(bool reportTestCases) {
		int fails = 0;

		Hfloat one(1.0);
		Hfloat sixteen(16.0);
		fails += expect_true(!one.iszero() && !sixteen.iszero(), "the samples are not zeros",
			reportTestCases);
		fails += expect_true(to_hex(one) != to_hex(sixteen),
			"different magnitudes render differently", reportTestCases);
		// a non-zero is unaffected by the zero special case
		fails += expect_true(to_hex(one).find(" * 16^") != std::string::npos,
			"the radix-16 exponent is still printed", reportTestCases);

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

	std::string test_suite  = "hfloat manipulators";
	std::string test_tag    = "hfloat manipulators";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyZeroRendering(reportTestCases), test_tag, "zero rendering");
	nrOfFailedTestCases += ReportTestResult(VerifyNonZeroKeepsItsExponent(reportTestCases), test_tag, "non-zero exponent");
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
