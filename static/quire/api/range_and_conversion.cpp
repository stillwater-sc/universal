// range_and_conversion.cpp: the generalized quire's operand range, convert_to rounding, load_bits and limits (#2058)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The QA sweep in #2039 found:
//   - the operand range check compared the scale with +-half_range, which is wrong for quires
//     whose radix point is not in the middle: quire<float> accepted 2^295 (the accumulator tops
//     out at 2^285) and quire<fixpnt<16,2>> accepted 2^-20 (it bottoms out at 2^-4), and both
//     came out zero;
//   - convert_to<T> truncated 53 bits through double: 1 + 2^-53 + 2^-80 was 1, UINT64_MAX was
//     2^64 - 2^11, and convert_to<int>(1e30) was an out-of-range cast;
//   - numeric_limits<quire> was never included, and returned zero for NaN and max() for inf;
//   - load_bits had no test.
// This file runs in the default (non-throwing) build; range_exceptions.cpp checks the same range
// with QUIRE_THROW_ARITHMETIC_EXCEPTION=1.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_suite.hpp>

namespace {
	using namespace sw::universal;

	int report(bool ok, const std::string& what, bool reportTestCases) {
		if (ok) return 0;
		if (reportTestCases) std::cerr << "FAIL: " << what << '\n';
		return 1;
	}

	// the operand range: an operand whose leading bit lies past either end of the accumulator is
	// dropped; one that just fits is kept
	int VerifyOperandRange(bool reportTestCases) {
		int fails = 0;
		{
			using Q = quire<float>;          // radix point 298: the scales are [-298, 285], not +-298
			fails += report(Q::highest_scale == 285 && Q::lowest_scale == -298, "quire<float> scale limits are not [-298, 285]", reportTestCases);
			Q q;
			q = std::ldexp(1.0, 285);
			fails += report(!q.iszero() && q.convert_to<double>() == std::ldexp(1.0, 285), "quire<float> = 2^285 is not 2^285", reportTestCases);
			q = std::ldexp(1.0, 295);
			fails += report(q.iszero(), "quire<float> = 2^295 is not dropped", reportTestCases);
			q = 1.0;
			q += blocktriple<52, BlockTripleOperator::REP, std::uint32_t>(std::ldexp(1.0, 290));
			fails += report(q.convert_to<double>() == 1.0, "quire<float> += 2^290 is not skipped", reportTestCases);
		}
		{
			using F = fixpnt<16, 2, Modulo, std::uint16_t>;
			using Q = quire<F>;              // radix point 4: the scales are [-4, 57]
			fails += report(Q::lowest_scale == -4, "quire<fixpnt<16,2>> lowest scale is not -4", reportTestCases);
			Q q;
			q = std::ldexp(1.0, -4);
			fails += report(q.convert_to<double>() == 0.0625, "quire<fixpnt<16,2>> = 2^-4 is not 2^-4", reportTestCases);
			q = std::ldexp(1.0, -20);
			fails += report(q.iszero(), "quire<fixpnt<16,2>> = 2^-20 is not dropped", reportTestCases);
			fails += report(q.min_scale() == -4 && q.max_scale() == Q::highest_scale, "min_scale()/max_scale() are not the accumulator's limits", reportTestCases);
		}
		{
			// integers: the leading bit against the top of the accumulator
			using Q = quire<fixpnt<16, 8, Modulo, std::uint16_t>>;   // radix point 16, qbits 62: top scale 45
			Q q;
			q = (std::int64_t(1) << 45);
			fails += report(!q.iszero(), "quire<fixpnt<16,8>> = 2^45 is dropped", reportTestCases);
			q = (std::int64_t(1) << 46);
			fails += report(q.iszero(), "quire<fixpnt<16,8>> = 2^46 is not dropped", reportTestCases);
		}
		return fails;
	}

	// convert_to<float/double>: round to nearest even from every bit, overflow and subnormals
	template<typename LimbType>
	int VerifyConvertToNative(bool reportTestCases) {
		int fails = 0;
		using Q = quire<double, 30, LimbType>;
		const double u = std::ldexp(1.0, -52);
		auto sum = [](std::initializer_list<double> terms) { Q q; for (double t : terms) q += blocktriple<52, BlockTripleOperator::REP, std::uint32_t>(t); return q; };
		fails += report(sum({ 1.0, u / 2, std::ldexp(1.0, -80) }).template convert_to<double>() == 1.0 + u, "1 + 2^-53 + 2^-80 does not round up to 1 + 2^-52", reportTestCases);
		fails += report(sum({ 1.0, u / 2 }).template convert_to<double>() == 1.0, "the tie 1 + 2^-53 does not round to even (1)", reportTestCases);
		fails += report(sum({ 1.0, u, u / 2 }).template convert_to<double>() == 1.0 + 2 * u, "the tie 1 + 3 2^-53 does not round to even (1 + 2^-51)", reportTestCases);
		fails += report(sum({ -1.0, -u / 2, -std::ldexp(1.0, -200) }).template convert_to<double>() == -(1.0 + u), "a negative sticky tail does not round away", reportTestCases);
		{
			Q q;
			q = std::numeric_limits<std::uint64_t>::max();
			fails += report(q.template convert_to<double>() == std::ldexp(1.0, 64), "UINT64_MAX does not round to 2^64", reportTestCases);
		}
		// float overflow: FLT_MAX plus half its ulp is a tie, and FLT_MAX is odd: inf
		const double fmax = double(std::numeric_limits<float>::max());
		fails += report(std::isinf(sum({ fmax, std::ldexp(1.0, 103) }).template convert_to<float>()), "FLT_MAX + ulp/2 does not overflow to inf", reportTestCases);
		fails += report(sum({ fmax, std::ldexp(1.0, 102) }).template convert_to<float>() == std::numeric_limits<float>::max(), "FLT_MAX + ulp/4 is not FLT_MAX", reportTestCases);
		fails += report(sum({ -fmax, -std::ldexp(1.0, 104) }).template convert_to<float>() == -std::numeric_limits<float>::infinity(), "-(FLT_MAX + ulp) is not -inf", reportTestCases);
		// double overflow: 2^1050, built by doubling
		{
			Q q;
			q = std::ldexp(1.0, 1000);
			for (int i = 0; i < 50; ++i) q += q;
			fails += report(std::isinf(q.template convert_to<double>()), "2^1050 does not overflow to inf", reportTestCases);
		}
		// float subnormals: the quantum is 2^-149
		const double t = std::ldexp(1.0, -149);
		fails += report(sum({ 1.5 * t }).template convert_to<float>() == float(2 * t), "1.5 2^-149 does not round to even (2^-148)", reportTestCases);
		fails += report(sum({ 0.5 * t }).template convert_to<float>() == 0.0f, "2^-150 does not round to even (0)", reportTestCases);
		fails += report(sum({ 0.5 * t, std::ldexp(1.0, -170) }).template convert_to<float>() == float(t), "2^-150 + 2^-170 does not round up to 2^-149", reportTestCases);
		fails += report(sum({ std::ldexp(1.0, -160) }).template convert_to<float>() == 0.0f, "2^-160 does not round to 0", reportTestCases);
		return fails;
	}

	// convert_to an integral type: truncation toward zero, saturated
	int VerifyConvertToIntegral(bool reportTestCases) {
		int fails = 0;
		using Q = quire<double>;
		auto q_of = [](double v) { Q q; q = v; return q; };
		fails += report(q_of(1e30).convert_to<int>() == std::numeric_limits<int>::max(), "1e30 does not saturate to INT_MAX", reportTestCases);
		fails += report(q_of(-1e30).convert_to<int>() == std::numeric_limits<int>::min(), "-1e30 does not saturate to INT_MIN", reportTestCases);
		fails += report(q_of(2.75).convert_to<int>() == 2 && q_of(-2.75).convert_to<int>() == -2, "2.75 and -2.75 do not truncate to 2 and -2", reportTestCases);
		fails += report(q_of(0.75).convert_to<int>() == 0, "0.75 does not truncate to 0", reportTestCases);
		fails += report(q_of(-5.0).convert_to<std::uint32_t>() == 0u, "-5 does not saturate to 0 for uint32_t", reportTestCases);
		fails += report(q_of(std::ldexp(1.0, 63)).convert_to<std::int64_t>() == std::numeric_limits<std::int64_t>::max(), "2^63 does not saturate to INT64_MAX", reportTestCases);
		fails += report(q_of(-std::ldexp(1.0, 63)).convert_to<std::int64_t>() == std::numeric_limits<std::int64_t>::min(), "-2^63 is not INT64_MIN", reportTestCases);
		fails += report(q_of(std::ldexp(1.0, 63)).convert_to<std::uint64_t>() == (std::uint64_t(1) << 63), "2^63 is not 2^63 as uint64_t", reportTestCases);
		Q big;
		big = std::numeric_limits<std::int64_t>::max();
		fails += report(big.convert_to<std::int64_t>() == std::numeric_limits<std::int64_t>::max(), "INT64_MAX does not convert exactly", reportTestCases);
		return fails;
	}

	// a Universal target goes through double, rounded to odd: posit saturates instead of NaR
	int VerifyConvertToUniversal(bool reportTestCases) {
		int fails = 0;
		using P = posit<32, 2>;
		using Q = quire<P>;
		Q q;
		q = std::ldexp(1.0, 240);                      // maxpos^2
		const P p = q.convert_to<P>();
		fails += report(p == P(SpecificValue::maxpos), "maxpos^2 does not saturate to maxpos", reportTestCases);
		// the posit quire takes every bit of a double, as quire<double> does (#2054)
		q = 1.0 + std::ldexp(1.0, -52);
		fails += report(q.convert_to<double>() == 1.0 + std::ldexp(1.0, -52), "quire<posit<32,2>> = 1 + 2^-52 loses the last bit", reportTestCases);
		// round to odd: 1 + 2^-28 + 2^-80 is above the posit tie 1 + 2^-28 (27 fraction bits at scale 0)
		Q r;
		r += blocktriple<52, BlockTripleOperator::REP, std::uint32_t>(1.0 + std::ldexp(1.0, -28));
		r += blocktriple<52, BlockTripleOperator::REP, std::uint32_t>(std::ldexp(1.0, -80));
		fails += report(double(r.convert_to<P>()) == 1.0 + std::ldexp(1.0, -27), "1 + 2^-28 + 2^-80 does not round up in posit<32,2>", reportTestCases);
		Q nan(SpecificValue::qnan);
		fails += report(nan.convert_to<P>().isnar(), "a NaN quire does not convert to NaR", reportTestCases);
		return fails;
	}

	// load_bits: the format, the round trip and the rejections
	int VerifyLoadBits(bool reportTestCases) {
		int fails = 0;
		using Q = quire<fixpnt<16, 8, Modulo, std::uint16_t>>;   // radix point 16, qbits 62
		Q q;
		fails += report(q.load_bits("+:101.11") && q.convert_to<double>() == 5.75, "load_bits(+:101.11) is not 5.75", reportTestCases);
		fails += report(q.load_bits("-:1_0.0'1") && q.convert_to<double>() == -2.25, "load_bits with separators is not -2.25", reportTestCases);
		Q before;
		before = 3.0;
		Q r(before);
		fails += report(!r.load_bits("101.1") && r == before, "a missing sign is accepted or changes the quire", reportTestCases);
		fails += report(!r.load_bits("+101.1") && r == before, "a missing ':' is accepted", reportTestCases);
		fails += report(!r.load_bits("+:1011") && r == before, "a missing radix point is accepted", reportTestCases);
		fails += report(!r.load_bits("+:1.0.1") && r == before, "two radix points are accepted", reportTestCases);
		fails += report(!r.load_bits("+:12.1") && r == before, "a digit other than 0 and 1 is accepted", reportTestCases);
		fails += report(!r.load_bits("") && r == before, "an empty string is accepted", reportTestCases);
		// 17 fraction bits do not fit below a radix point of 16, and 47 integer bits not above it
		fails += report(!r.load_bits("+:1." + std::string(17, '1')) && r == before, "17 fraction bits are accepted", reportTestCases);
		fails += report(r.load_bits("+:1." + std::string(16, '1')), "16 fraction bits are rejected", reportTestCases);
		fails += report(!r.load_bits("+:" + std::string(47, '1') + ".0"), "47 integer bits are accepted", reportTestCases);
		fails += report(r.load_bits("+:" + std::string(46, '1') + ".0"), "46 integer bits are rejected", reportTestCases);
		// a non-finite quire becomes finite on a successful load
		Q n(SpecificValue::qnan);
		fails += report(n.load_bits("+:1.0") && !n.isnan() && n.convert_to<double>() == 1.0, "load_bits does not clear the NaN state", reportTestCases);
		return fails;
	}

	int VerifyNumericLimits(bool reportTestCases) {
		int fails = 0;
		using Q = quire<double>;
		using L = std::numeric_limits<Q>;
		fails += report(L::is_specialized, "numeric_limits<quire> is not specialized", reportTestCases);
		fails += report(L::has_infinity && L::infinity().isinf() && !L::infinity().sign(), "infinity() is not +inf", reportTestCases);
		fails += report(L::has_quiet_NaN && L::quiet_NaN().isnan(), "quiet_NaN() is not NaN", reportTestCases);
		fails += report(L::max() > L::min() && L::lowest() < L::min() && L::lowest().sign(), "max/min/lowest are not ordered", reportTestCases);
		// 2^-2148 is below double's range, so inspect the quire itself: positive, bit 0 only
		const Q mn = L::min();
		fails += report(!mn.iszero() && !mn.sign() && mn.testbit(0) && mn.scale() == Q::lowest_scale, "min() is not 2^-radix_point (bit 0)", reportTestCases);
		fails += report(L::digits == static_cast<int>(Q::qbits), "digits is not qbits", reportTestCases);
		fails += report(L::round_error().iszero() && L::is_exact, "round_error() is not 0", reportTestCases);
		return fails;
	}

#if defined(__SIZEOF_INT128__)
	__extension__ typedef __int128 int128;   // a gcc/clang extension: -Wpedantic would flag a bare __int128
	// randomized quire + quire and < against __int128, on integers (exact in both)
	template<typename LimbType>
	int VerifyRandomizedAgainstInt128(bool reportTestCases) {
		int fails = 0;
		using Q = quire<fixpnt<32, 16, Modulo, std::uint32_t>, 30, LimbType>;   // top scale 61
		std::mt19937_64 rng(2058);
		Q acc;
		int128 ref = 0;
		for (int i = 0; i < 2000; ++i) {
			const std::int64_t v = static_cast<std::int64_t>(rng() >> 24) * ((rng() & 1u) ? 1 : -1);   // |v| < 2^40
			Q term;
			term = v;
			const Q previous = acc;
			const int128 previousRef = ref;
			acc += term;
			ref += v;
			const bool lessRef = previousRef < ref, lessQ = previous < acc;
			if (acc.template convert_to<std::int64_t>() != static_cast<std::int64_t>(ref) || lessRef != lessQ) {
				++fails;
				if (reportTestCases && fails < 4) std::cerr << "FAIL: step " << i << " sum or order disagrees with __int128\n";
			}
		}
		return fails;
	}
#endif

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

	std::string test_suite  = "generalized quire range, conversion, load_bits and limits (#2058)";
	std::string test_tag    = "quire";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyOperandRange(reportTestCases), test_tag, "operand range");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyOperandRange(reportTestCases), test_tag, "operand range");
	nrOfFailedTestCases += ReportTestResult(VerifyConvertToNative<std::uint32_t>(reportTestCases), test_tag, "convert_to native (uint32 limbs)");
	nrOfFailedTestCases += ReportTestResult(VerifyConvertToNative<std::uint64_t>(reportTestCases), test_tag, "convert_to native (uint64 limbs)");
	nrOfFailedTestCases += ReportTestResult(VerifyConvertToNative<std::uint8_t>(reportTestCases), test_tag, "convert_to native (uint8 limbs)");
	nrOfFailedTestCases += ReportTestResult(VerifyConvertToIntegral(reportTestCases), test_tag, "convert_to integral");
	nrOfFailedTestCases += ReportTestResult(VerifyConvertToUniversal(reportTestCases), test_tag, "convert_to posit");
	nrOfFailedTestCases += ReportTestResult(VerifyLoadBits(reportTestCases), test_tag, "load_bits");
	nrOfFailedTestCases += ReportTestResult(VerifyNumericLimits(reportTestCases), test_tag, "numeric_limits");
#if defined(__SIZEOF_INT128__)
	nrOfFailedTestCases += ReportTestResult(VerifyRandomizedAgainstInt128<std::uint32_t>(reportTestCases), test_tag, "randomized vs __int128 (uint32 limbs)");
	nrOfFailedTestCases += ReportTestResult(VerifyRandomizedAgainstInt128<std::uint64_t>(reportTestCases), test_tag, "randomized vs __int128 (uint64 limbs)");
#endif
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
