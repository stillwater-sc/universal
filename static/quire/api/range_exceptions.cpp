// range_exceptions.cpp: the generalized quire's operand range with QUIRE_THROW_ARITHMETIC_EXCEPTION=1 (#2058)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The range check compared an operand's scale with +-half_range. For a quire whose radix point is
// not in the middle that let operands past either end through, even with the throw guard on:
// quire<float> = 2^295 and quire<fixpnt<16,2>> = 2^-20 were accepted and came out zero. They throw
// now, and an operand that just fits does not. range_and_conversion.cpp checks the default build.
#include <universal/utility/directives.hpp>
#define QUIRE_THROW_ARITHMETIC_EXCEPTION 1
#include <cmath>
#include <cstdint>
#include <string>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	// 0: no throw, 1: operand_too_large_for_quire, 2: operand_too_small_for_quire
	template<typename Q, typename Op>
	int outcome(Op op) {
		try { Q q; op(q); return 0; }
		catch (const operand_too_large_for_quire&) { return 1; }
		catch (const operand_too_small_for_quire&) { return 2; }
	}

	int expect(int got, int want, const std::string& what, bool reportTestCases) {
		if (got == want) return 0;
		if (reportTestCases) std::cerr << "FAIL: " << what << ": outcome " << got << ", expected " << want << '\n';
		return 1;
	}

	int VerifyThrowingRange(bool reportTestCases) {
		int fails = 0;
		using BT = blocktriple<52, BlockTripleOperator::REP, std::uint32_t>;
		{
			using Q = quire<float>;          // scales [-298, 285]
			fails += expect(outcome<Q>([](Q& q) { q = std::ldexp(1.0, 285); }), 0, "quire<float> = 2^285", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q = std::ldexp(1.0, 286); }), 1, "quire<float> = 2^286", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q = std::ldexp(1.0, 295); }), 1, "quire<float> = 2^295", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q += BT(std::ldexp(1.0, 295)); }), 1, "quire<float> += 2^295", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q = std::ldexp(1.0, -298); }), 0, "quire<float> = 2^-298", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q = std::ldexp(1.0, -299); }), 2, "quire<float> = 2^-299", reportTestCases);
		}
		{
			using Q = quire<fixpnt<16, 2, Modulo, std::uint16_t>>;   // scales [-4, 57]
			fails += expect(outcome<Q>([](Q& q) { q = std::ldexp(1.0, -4); }), 0, "quire<fixpnt<16,2>> = 2^-4", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q = std::ldexp(1.0, -20); }), 2, "quire<fixpnt<16,2>> = 2^-20", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q += BT(std::ldexp(1.0, -5)); }), 2, "quire<fixpnt<16,2>> += 2^-5", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q = std::ldexp(1.0, 57); }), 0, "quire<fixpnt<16,2>> = 2^57", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q = std::ldexp(1.0, 58); }), 1, "quire<fixpnt<16,2>> = 2^58", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q = (std::int64_t(1) << 57); }), 0, "quire<fixpnt<16,2>> = int64 2^57", reportTestCases);
			fails += expect(outcome<Q>([](Q& q) { q = (std::uint64_t(1) << 58); }), 1, "quire<fixpnt<16,2>> = uint64 2^58", reportTestCases);
		}
		return fails;
	}

}  // namespace

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

	std::string test_suite  = "generalized quire operand range with the throw guard (#2058)";
	std::string test_tag    = "quire";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyThrowingRange(reportTestCases), test_tag, "throwing range");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyThrowingRange(reportTestCases), test_tag, "throwing range");
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
