// limits_and_helpers.cpp: numeric_limits<posit>, the attributes.hpp helpers, reciprocal, rsqrt, ulp at
// maxpos, positExponent comparison and the stream extractor (#2070)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each check here failed, or did not compile, before #2070:
//   - numeric_limits<posit>::max_exponent was maxpos's scale; it is one more, as min_exponent is
//     one more than minpos's scale. has_infinity was true and is_bounded false, for a type with
//     NaR, no infinity, and a finite set of values.
//   - useed_value<nbits, es>() shifted 1 by 2^es, undefined for es >= 6.
//   - fraction_value() sized its fraction nbits - 3 - es, which wraps for es + 2 >= nbits.
//   - reciprocal() used blockbinary helpers that no longer exist, so neither it nor rsqrt compiled.
//   - ulp(maxpos) was NaR: the encoding after maxpos is NaR.
//   - positExponent's == and < named members that do not exist.
//   - operator>> wrote to std::cerr on a parse failure.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <sstream>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/posit_test_suite.hpp>

namespace sw { namespace universal {

	int report(bool ok, const std::string& what, bool reportTestCases) {
		if (!ok && reportTestCases) std::cerr << "FAIL " << what << '\n';
		return ok ? 0 : 1;
	}

	// radix^(max_exponent - 1) is maxpos's binade and radix^(min_exponent - 1) minpos's
	template<unsigned nbits, unsigned es>
	int VerifyExponentRange(bool reportTestCases) {
		using P = posit<nbits, es>;
		using L = std::numeric_limits<P>;
		const std::string tag = type_tag(P());
		int fails = 0;
		const P maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
		fails += report(L::max_exponent - 1 == maxpos.scale(), tag + " max_exponent - 1 is maxpos's scale", reportTestCases);
		fails += report(L::min_exponent - 1 == minpos.scale(), tag + " min_exponent - 1 is minpos's scale", reportTestCases);
		fails += report(L::is_bounded && !L::has_infinity && L::has_quiet_NaN && !L::has_signaling_NaN, tag + " bounded, no infinity, NaR", reportTestCases);
		fails += report(L::infinity() == L::max() && L::quiet_NaN().isnar(), tag + " infinity() is maxpos, quiet_NaN() is NaR", reportTestCases);
		return fails;
	}

	int VerifyLimits(bool reportTestCases) {
		int fails = 0;
		fails += report(std::numeric_limits<posit<16, 1>>::max_exponent == 29, "posit<16,1> max_exponent is 29", reportTestCases);
		fails += VerifyExponentRange<8, 0>(reportTestCases);
		fails += VerifyExponentRange<16, 1>(reportTestCases);
		fails += VerifyExponentRange<32, 2>(reportTestCases);
		fails += VerifyExponentRange<12, 4>(reportTestCases);
		return fails;
	}

	int VerifyAttributeHelpers(bool reportTestCases) {
		int fails = 0;
		static_assert(useed<3>() == 256u, "useed<3> is 2^8");
		static_assert(useed_value<16, 3>() == 256.0, "useed_value<16,3> is 2^8");
		static_assert(useed_value<32, 6>() == 18446744073709551616.0, "useed_value<32,6> is 2^64");
		static_assert(useed_value<64, 7>() == 340282366920938463463374607431768211456.0, "useed_value<64,7> is 2^128");
		fails += report(useed<16, 6>() == std::ldexp(1.0, 64), "useed<16,6>() is 2^64", reportTestCases);
		// es + 2 >= nbits: no fraction bits, and the fraction value is 0
		const posit<5, 3> p(1.0);
		fails += report(fraction_value(p) == 0.0L, "fraction_value(posit<5,3>(1)) is 0", reportTestCases);
		fails += report(fraction_value(posit<8, 0>(1.5)) == 0.5L, "fraction_value(posit<8,0>(1.5)) is 0.5", reportTestCases);
		return fails;
	}

	int VerifyReciprocalAndRsqrt(bool reportTestCases) {
		int fails = 0;
		fails += report(double(rsqrt(posit<16, 1>(4))) == 0.5, "rsqrt(posit<16,1>(4)) is 0.5", reportTestCases);
		fails += report(double(rsqrt(posit<32, 2>(0.25))) == 2.0, "rsqrt(posit<32,2>(0.25)) is 2", reportTestCases);
		fails += report(posit<16, 1>(0).reciprocal().isnar(), "reciprocal of 0 is NaR", reportTestCases);
		posit<16, 1> nar;
		nar.setnar();
		fails += report(nar.reciprocal().isnar(), "reciprocal of NaR is NaR", reportTestCases);
		// every posit<10,1>: 1/x, rounded once, as the division gives it
		for (unsigned i = 0; i < 1024u; ++i) {
			posit<10, 1> a;
			a.setbits(i);
			if (a.iszero() || a.isnar()) continue;
			const posit<10, 1> expected = posit<10, 1>(1.0 / double(a));   // 1/x of a 7-bit fraction: double rounds once here
			if (a.reciprocal() != expected) {
				++fails;
				if (reportTestCases && fails < 10) std::cerr << "FAIL reciprocal(" << a << ") = " << a.reciprocal() << ", expected " << expected << '\n';
			}
		}
		return fails;
	}

	template<unsigned nbits, unsigned es>
	int VerifyUlpAtMaxpos(bool reportTestCases) {
		using P = posit<nbits, es>;
		P m(SpecificValue::maxpos), below(SpecificValue::maxpos);
		--below;
		const P u = ulp(m);
		int fails = report(!u.isnar() && u == m - below, type_tag(m) + " ulp(maxpos) is maxpos - its predecessor", reportTestCases);
		fails += report(ulp(P(1.0)) == std::numeric_limits<P>::epsilon(), type_tag(m) + " ulp(1) is epsilon", reportTestCases);
		return fails;
	}

	int VerifyComponentsAndStreams(bool reportTestCases) {
		int fails = 0;
		positExponent<16, 2, std::uint8_t> e1, e2;
		fails += report(e1 == e2 && !(e1 != e2) && !(e1 < e2) && e1 <= e2 && e1 >= e2, "positExponent comparisons", reportTestCases);

		// posit_fields_io.hpp prints the components, debug.hpp the class parameters and limbs
		{
			bool s{ false };
			positRegime<16, 2, std::uint8_t> r;
			positExponent<16, 2, std::uint8_t> e;
			positFraction<11, std::uint8_t> f;
			decode(posit<16, 2>(3.0).bits(), s, r, e, f);
			std::ostringstream fields;
			fields << r << ' ' << e << ' ' << f;
			fails += report(!fields.str().empty(), "posit_fields_io prints regime, exponent and fraction", reportTestCases);
			posit<16, 2>(3.0).constexprClassParameters();
			posit<16, 2>(3.0).showLimbs();
		}

		// a failed parse sets failbit and leaves the posit and std::cerr alone
		std::ostringstream captured;
		std::streambuf* saved = std::cerr.rdbuf(captured.rdbuf());
		std::istringstream is("not-a-posit");
		posit<16, 1> p(1.0);
		is >> p;
		std::cerr.rdbuf(saved);
		fails += report(is.fail() && p == posit<16, 1>(1.0), "operator>> of garbage sets failbit, leaves the posit", reportTestCases);
		fails += report(captured.str().empty(), "operator>> of garbage writes nothing to std::cerr", reportTestCases);
		std::istringstream good("16.1x4000p");
		posit<16, 1> q;
		good >> q;
		fails += report(!good.fail() && q == posit<16, 1>(1.0), "operator>> of 16.1x4000p is 1", reportTestCases);
		// parse() twice: the regex is built once, the results are independent
		posit<8, 0> r1, r2;
		fails += report(parse("8.0x40p", r1) && parse("8.0x20p", r2) && r1 == posit<8, 0>(1.0) && r2 == posit<8, 0>(0.5), "parse() twice", reportTestCases);
		return fails;
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

	std::string test_suite  = "posit limits and helpers (#2070)";
	std::string test_tag    = "limits and helpers";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyLimits(reportTestCases), test_tag, "numeric_limits");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyLimits(reportTestCases), test_tag, "numeric_limits");
	nrOfFailedTestCases += ReportTestResult(VerifyAttributeHelpers(reportTestCases), test_tag, "useed and fraction_value");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocalAndRsqrt(reportTestCases), test_tag, "reciprocal and rsqrt");
	nrOfFailedTestCases += ReportTestResult(VerifyReciprocation<posit<8, 0>>(reportTestCases), test_tag, "VerifyReciprocation posit<8,0>");
	nrOfFailedTestCases += ReportTestResult((VerifyUlpAtMaxpos<16, 1>(reportTestCases)), test_tag, "ulp posit<16,1>");
	nrOfFailedTestCases += ReportTestResult((VerifyUlpAtMaxpos<32, 2>(reportTestCases)), test_tag, "ulp posit<32,2>");
	nrOfFailedTestCases += ReportTestResult(VerifyComponentsAndStreams(reportTestCases), test_tag, "positExponent and streams");
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
