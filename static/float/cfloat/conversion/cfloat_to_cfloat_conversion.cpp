// cfloat_to_cfloat_conversion.cpp: test suite runner for cfloat-to-cfloat cross-config conversions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the cfloat template environment
// first: enable general or specialized configurations
#define CFLOAT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// third: enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

/*
 * Regression tests for cfloat cross-configuration converting constructor.
 *
 * The converting constructor (cfloat::cfloat(const cfloat<...>&)) is exercised
 * by exhaustively iterating all source bit patterns and checking that direct
 * cfloat-to-cfloat assignment produces the same result as going through double.
 *
 * Oracle validity: double has 52 significand bits.  A cfloat<N,es> has
 * (N - es - 1) fraction bits.  The double oracle is exact iff BOTH the source
 * and target fraction-bit widths are <= 52, i.e. the value can be represented
 * in double without rounding.  A static_assert enforces this precondition so
 * that the test cannot silently produce false-positive results for wider types.
 *
 * For cfloats wider than double, a future oracle based on efloat/ereal
 * (adaptive-precision) or expression-template type analysis would be needed.
 *
 * This covers the fix for the full-precision blocktriple-based path introduced
 * to avoid the double-bottleneck that loses precision for cfloats wider than
 * IEEE-754 double.
 */

namespace sw::universal {

// Exhaustive conversion test: iterate all 2^srcbits source patterns,
// convert to target via direct assignment (converting constructor),
// and verify against double-mediated reference.
//
// PRECONDITION: both Source and Target must have fraction bits <= 52
// (i.e. all their values are exactly representable in IEEE-754 double).
// A static_assert enforces this so the test cannot silently pass on
// configurations where the double oracle is inexact.
template<typename Source, typename Target>
int VerifyExhaustiveCfloatConversion(bool reportTestCases) {
	constexpr unsigned srcFbits = Source::nbits - Source::es - 1u;
	constexpr unsigned tgtFbits = Target::nbits - Target::es - 1u;
	static_assert(srcFbits <= 52 && tgtFbits <= 52,
	    "VerifyExhaustiveCfloatConversion: double oracle is only valid when "
	    "both source and target have at most 52 fraction bits. "
	    "Use an efloat/ereal oracle for wider configurations.");

	constexpr unsigned srcbits = Source::nbits;
	static_assert(srcbits < 64u,
	    "VerifyExhaustiveCfloatConversion: srcbits >= 64 would overflow 1ull << srcbits; "
	    "use VerifyCfloatSpecialValueConversions for wide types instead.");
	constexpr unsigned long long NR_ENCODINGS = (1ull << srcbits);

	int nrOfFailedTests = 0;
	Source src{};
	Target tgt{}, ref{};
	for (unsigned long long i = 0; i < NR_ENCODINGS; ++i) {
		src.setbits(i);
		// direct cfloat-to-cfloat conversion (exercises converting constructor)
		tgt = src;

		// Short-circuit NaN: src.isnan() -> tgt must be NaN.
		// Do NOT use double(src) as oracle for NaN because
		// std::numeric_limits<double>::signaling_NaN() is not reliable
		// on all platforms (RISC-V/QEMU returns 0.0 instead of sNaN).
		if (src.isnan()) {
			if (!tgt.isnan()) {
				if (reportTestCases) {
					std::cout << "FAIL: NaN src " << to_binary(src)
					          << " -> non-NaN tgt " << to_binary(tgt) << " = " << tgt << '\n';
				}
				++nrOfFailedTests;
			}
			continue;
		}

		// reference: go through double (exact because srcFbits <= 52 and tgtFbits <= 52)
		ref = double(src);
		if (tgt != ref) {
			if (reportTestCases) {
				std::cout << "FAIL: src " << to_binary(src) << " = " << src
				          << "  ->  tgt " << to_binary(tgt) << " = " << tgt
				          << "  ref " << to_binary(ref) << " = " << ref << '\n';
			}
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// Spot-check special values: zero, +/-inf, +/-maxpos, +/-minpos, qNaN, sNaN
template<typename Source, typename Target>
int VerifyCfloatSpecialValueConversions(bool reportTestCases) {
	int nrOfFailedTests = 0;
	Source src{};
	Target tgt{}, ref{};

	auto check = [&](const char* label) {
		tgt = src;
		// Short-circuit NaN: src.isnan() -> tgt must be NaN.
		// Do NOT use double(src) as oracle for NaN because
		// std::numeric_limits<double>::signaling_NaN() is not reliable
		// on all platforms (RISC-V/QEMU returns 0.0 instead of sNaN).
		bool ok;
		if (src.isnan()) {
			ok = tgt.isnan();
		} else {
			ref = double(src);
			ok = (tgt == ref);
		}
		if (!ok) {
			if (reportTestCases) {
				std::cout << "FAIL [" << label << "]: src=" << src
				          << "  tgt=" << tgt << "  ref=" << ref << '\n';
			}
			++nrOfFailedTests;
		}
	};

	src.setzero();            check("zero");
	src.setinf(false);        check("+inf");
	src.setinf(true);         check("-inf");
	src.maxpos();             check("maxpos");
	src.maxneg();             check("maxneg");
	src.minpos();             check("minpos");
	src.minneg();             check("minneg");
	src.setnan(NAN_TYPE_QUIET);      check("qNaN");
	src.setnan(NAN_TYPE_SIGNALLING); check("sNaN");
	return nrOfFailedTests;
}

} // namespace sw::universal

// Regression testing guards: typically set by the cmake configuration
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "cfloat-to-cfloat cross-config conversion";
	std::string test_tag    = "cfloat conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// Quick smoke test in manual mode
	{
		using Src = cfloat<8, 3, uint8_t, false, false, false>;
		using Tgt = cfloat<12, 4, uint8_t, false, false, false>;
		nrOfFailedTestCases += ReportTestResult(
		    VerifyExhaustiveCfloatConversion<Src, Tgt>(true),
		    test_tag, "cfloat<8,3> -> cfloat<12,4>");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures in manual mode
#else  // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	// Narrowing: cfloat<12,4> -> cfloat<8,3>  (4096 source encodings)
	nrOfFailedTestCases += ReportTestResult(
	    VerifyExhaustiveCfloatConversion<
	        cfloat<12, 4, uint8_t, false, false, false>,
	        cfloat< 8, 3, uint8_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<12,4> -> cfloat<8,3> (narrowing)");

	// Widening: cfloat<8,2> -> cfloat<16,5>  (256 source encodings)
	nrOfFailedTestCases += ReportTestResult(
	    VerifyExhaustiveCfloatConversion<
	        cfloat< 8, 2, uint8_t, false, false, false>,
	        cfloat<16, 5, uint8_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<8,2> -> cfloat<16,5> (widening)");

	// Widening: cfloat<8,3> -> cfloat<12,4>  (256 source encodings)
	nrOfFailedTestCases += ReportTestResult(
	    VerifyExhaustiveCfloatConversion<
	        cfloat< 8, 3, uint8_t, false, false, false>,
	        cfloat<12, 4, uint8_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<8,3> -> cfloat<12,4> (widening)");

	// Special values: cfloat<32,8> -> cfloat<16,5>
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCfloatSpecialValueConversions<
	        cfloat<32, 8, uint8_t, false, false, false>,
	        cfloat<16, 5, uint8_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<32,8> -> cfloat<16,5> special values");

	// Special values: cfloat<16,5> -> cfloat<32,8>
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCfloatSpecialValueConversions<
	        cfloat<16, 5, uint8_t, false, false, false>,
	        cfloat<32, 8, uint8_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<16,5> -> cfloat<32,8> special values");

	// Wide-path: cfloat<32,8,uint64_t> -> cfloat<16,5,uint64_t>
	// srcFbits = 23, fbits+addRbits >= 64 --> exercises wide bit-by-bit copy path.
	// Use special-value spot checks since exhaustive enumeration is too large.
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCfloatSpecialValueConversions<
	        cfloat<32, 8, uint64_t, false, false, false>,
	        cfloat<16, 5, uint64_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<32,8,uint64_t> -> cfloat<16,5,uint64_t> special values (wide path)");

	// Cross-block-type: src uses uint8_t, target uses uint16_t
	// Exercises the cross-block-type double fallback path in the converting constructor.
	nrOfFailedTestCases += ReportTestResult(
	    VerifyExhaustiveCfloatConversion<
	        cfloat< 8, 3, uint8_t,  false, false, false>,
	        cfloat< 8, 3, uint16_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<8,3,uint8_t> -> cfloat<8,3,uint16_t> (cross-block-type)");
#endif

#if REGRESSION_LEVEL_2
	// Saturating + subnormals + maxexp: cfloat<16,5,ttt> -> cfloat<8,3,ttt>  (65536 source encodings)
	nrOfFailedTestCases += ReportTestResult(
	    VerifyExhaustiveCfloatConversion<
	        cfloat<16, 5, uint8_t, true, true, true>,
	        cfloat< 8, 3, uint8_t, true, true, true>>(reportTestCases),
	    test_tag, "cfloat<16,5,ttt> -> cfloat<8,3,ttt> (narrowing, sat+sub+maxexp)");

	// Widening with subnormals: cfloat<8,3,ttt> -> cfloat<16,5,ttt>
	nrOfFailedTestCases += ReportTestResult(
	    VerifyExhaustiveCfloatConversion<
	        cfloat< 8, 3, uint8_t, true, true, true>,
	        cfloat<16, 5, uint8_t, true, true, true>>(reportTestCases),
	    test_tag, "cfloat<8,3,ttt> -> cfloat<16,5,ttt> (widening, sat+sub+maxexp)");
#endif

#if REGRESSION_LEVEL_3
	// Narrowing: cfloat<16,5> -> cfloat<8,3>  (65536 source encodings)
	nrOfFailedTestCases += ReportTestResult(
	    VerifyExhaustiveCfloatConversion<
	        cfloat<16, 5, uint8_t, false, false, false>,
	        cfloat< 8, 3, uint8_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<16,5> -> cfloat<8,3> (narrowing)");

	// Widening: cfloat<8,3> -> cfloat<16,5>
	nrOfFailedTestCases += ReportTestResult(
	    VerifyExhaustiveCfloatConversion<
	        cfloat< 8, 3, uint8_t, false, false, false>,
	        cfloat<16, 5, uint8_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<8,3> -> cfloat<16,5> (widening)");
#endif

#if REGRESSION_LEVEL_4
	// Exhaustive narrowing: cfloat<16,5> -> cfloat<12,4>
	nrOfFailedTestCases += ReportTestResult(
	    VerifyExhaustiveCfloatConversion<
	        cfloat<16, 5, uint8_t, false, false, false>,
	        cfloat<12, 4, uint8_t, false, false, false>>(reportTestCases),
	    test_tag, "cfloat<16,5> -> cfloat<12,4> (narrowing)");
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
