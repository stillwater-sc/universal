// text_roundtrip.cpp: posit text functions: special values and the native round trip
//
// Three text functions mishandled posit's two special values or its native format:
//
//   - quadrant() documents its results as 0, SE, NE, NaR, NW, SW but classified by sign
//     and magnitude before looking for either special value, so zero came back SE and
//     NaR, whose sign bit is set, came back NW (#1391)
//   - to_base2_scientific() decoded zero and NaR as if they had fields, rendering zero
//     as +1.---e2^-30 and NaR as -1.---e2^+30 (#1391)
//   - hex_format() wrote "16.1x0x40'00p" -- a 0x prefix and nibble markers the
//     documented nbits.esxNN...NNp form does not have -- and parse() rejected it, so
//     the native format did not round-trip at ANY width. parse() also read the digits
//     through a uint64_t, which dropped every bit above 63 of a wider posit (#1391)
//
// and operator<< on a positFraction pre-incremented its counter where to_string()
// post-increments, so the two disagreed on the last valid fraction bit (#1412).
//
// The round trip is checked over EVERY encoding of several small configurations, and
// over a sample of posit<80,3> for the bits a uint64_t cannot hold.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <sstream>
#include <string>

#include <universal/number/posit/posit.hpp>
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


	// ---- quadrant names all six documented regions -----------------------------------

	int VerifyQuadrant(bool reportTestCases) {
		int fails = 0;
		using P = posit<16, 1>;

		P zero(0), nar;
		nar.setnar();
		fails += expect_text(quadrant(zero), "0", "zero", reportTestCases);
		fails += expect_text(quadrant(nar), "NaR", "NaR", reportTestCases);
		fails += expect_text(quadrant(P(0.5)), "SE", "0.5", reportTestCases);
		fails += expect_text(quadrant(P(2.0)), "NE", "2.0", reportTestCases);
		fails += expect_text(quadrant(P(-0.5)), "SW", "-0.5", reportTestCases);
		fails += expect_text(quadrant(P(-2.0)), "NW", "-2.0", reportTestCases);

		// every encoding lands in exactly one region, and only zero and NaR in theirs
		for (unsigned bits = 0; bits < 256u; ++bits) {
			posit<8, 2> p;
			p.setbits(bits);
			const std::string q = quadrant(p);
			const bool special = p.iszero() || p.isnar();
			const bool isSpecialName = (q == "0" || q == "NaR");
			if (special != isSpecialName) {
				++fails;
				if (reportTestCases) std::cout << "    FAIL encoding " << bits << " -> " << q << '\n';
			}
		}

		return fails;
	}

	// ---- zero and NaR have no fields to decode ----------------------------------------

	int VerifyBase2Scientific(bool reportTestCases) {
		int fails = 0;
		using P = posit<16, 1>;

		P zero(0), nar;
		nar.setnar();
		fails += expect_text(to_base2_scientific(zero), "0", "zero", reportTestCases);
		fails += expect_text(to_base2_scientific(nar), "nar", "NaR", reportTestCases);
		// the spelling of NaR matches to_string's
		fails += expect_text(to_base2_scientific(nar), to_string(nar), "NaR agrees with to_string",
			reportTestCases);

		// ordinary values keep the scientific form
		const std::string one = to_base2_scientific(P(1.0));
		fails += expect_true(one.rfind("+1.", 0) == 0, "1.0 renders as +1.xxx", reportTestCases);
		fails += expect_true(one.find("e2^+0") != std::string::npos, "with a binary exponent of 0",
			reportTestCases);
		fails += expect_true(to_base2_scientific(P(-4.0)).find("e2^+2") != std::string::npos,
			"-4.0 has a binary exponent of 2", reportTestCases);

		return fails;
	}

	// ---- the native format is the documented one ---------------------------------------

	int VerifyNativeFormat(bool reportTestCases) {
		int fails = 0;
		using P = posit<16, 1>;

		fails += expect_text(hex_format(P(1.0)), "16.1x4000p", "1.0 in native form", reportTestCases);
		fails += expect_true(hex_format(P(1.0)).find("0x") == std::string::npos, "no 0x prefix",
			reportTestCases);
		fails += expect_true(hex_format(P(1.0)).find('\'') == std::string::npos, "no nibble markers",
			reportTestCases);

		// strings written in the old form, with the prefix and markers, still read back
		{
			P p;
			fails += expect_true(parse("16.1x0x40'00p", p), "the legacy form parses", reportTestCases);
			fails += expect_true(p == P(1.0), "to the same value", reportTestCases);
		}
		// the configuration must match the target, and a bit the posit cannot hold is refused
		{
			P p(0.25);
			const P before = p;
			fails += expect_true(!parse("32.2x40000000p", p), "a different configuration is refused",
				reportTestCases);
			fails += expect_true(!parse("16.1x14000p", p), "a 17th set bit is refused", reportTestCases);
			fails += expect_true(p == before, "and a refusal leaves the target alone", reportTestCases);
			// leading zero digits beyond the width are harmless
			fails += expect_true(parse("16.1x04000p", p) && p == P(1.0), "leading zeros are accepted",
				reportTestCases);
		}

		return fails;
	}

	// ---- every encoding survives posit -> text -> posit ---------------------------------

	// stride 1 visits every encoding; a larger stride samples them, and the top encoding
	// is always checked too, so the end of the range is never skipped
	template<unsigned nbits, unsigned es>
	int VerifyRoundTrip(bool reportTestCases, std::uint64_t stride = 1) {
		int fails = 0;
		using P = posit<nbits, es>;
		constexpr std::uint64_t nrEncodings = std::uint64_t(1) << nbits;

		auto roundTrips = [&](std::uint64_t bits) {
			P p;
			p.setbits(bits);
			const std::string text = hex_format(p);
			P back;
			back.setzero();
			if (!parse(text, back)) {
				++fails;
				if (reportTestCases && fails < 5) std::cout << "    FAIL " << text << " did not parse\n";
				return;
			}
			// compare encodings, not values: NaR is not equal to itself
			if (!(back.bits() == p.bits())) {
				++fails;
				if (reportTestCases && fails < 5) std::cout << "    FAIL " << text << " came back different\n";
			}
		};

		for (std::uint64_t bits = 0; bits < nrEncodings; bits += stride) roundTrips(bits);
		if ((nrEncodings - 1) % stride != 0) roundTrips(nrEncodings - 1);

		return fails;
	}

	// posit<80,3> has 16 bits a uint64_t cannot hold; each sample sets some of them
	int VerifyWideRoundTrip(bool reportTestCases) {
		int fails = 0;
		using P = posit<80, 3>;

		const double samples[] = { 1.0, -1.0, 3.141592653589793, 1e-20, -2.5e15, 0.1 };
		for (double v : samples) {
			P p(v);
			p = p + P(1e-18);                      // perturb, so the low-order bits are busy
			const std::string text = hex_format(p);
			P back;
			back.setzero();
			fails += expect_true(parse(text, back), "the wide native form parses", reportTestCases);
			fails += expect_true(back.bits() == p.bits(), "every one of the 80 bits comes back",
				reportTestCases);
		}

		// a pattern that lives entirely above bit 63 -- a uint64_t reads it as zero
		{
			P p;
			p.setnar();                                // only the sign bit, bit 79, is set
			P back;
			back.setzero();
			fails += expect_true(parse(hex_format(p), back) && back.isnar(),
				"a value held only above bit 63 comes back", reportTestCases);
		}

		return fails;
	}

	// ---- operator<< and to_string render a fraction identically ---------------------------

	template<unsigned nbits, unsigned es>
	int VerifyFractionRendering(bool reportTestCases) {
		int fails = 0;
		using P = posit<nbits, es>;
		using bt = typename P::BlockType;
		constexpr unsigned fbits = (es + 2 >= nbits ? 0 : nbits - 3 - es);

		for (std::uint64_t bits = 0; bits < (std::uint64_t(1) << nbits); ++bits) {
			P p;
			p.setbits(bits);
			if (p.iszero() || p.isnar()) continue;
			bool s{ false };
			positRegime<nbits, es, bt> r;
			positExponent<nbits, es, bt> e;
			positFraction<fbits, bt> f;
			extract_fields(p.bits(), s, r, e, f);
			std::stringstream streamed;
			streamed << f;
			if (streamed.str() != to_string(f, true)) {
				++fails;
				if (reportTestCases && fails < 5) {
					std::cout << "    FAIL encoding " << bits << ": operator<< '" << streamed.str()
					          << "' vs to_string '" << to_string(f, true) << "'\n";
				}
			}
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

	std::string test_suite  = "posit text functions";
	std::string test_tag    = "posit text";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyQuadrant(reportTestCases), test_tag, "quadrant");
	nrOfFailedTestCases += ReportTestResult(VerifyBase2Scientific(reportTestCases), test_tag, "to_base2_scientific");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeFormat(reportTestCases), test_tag, "native format");
	nrOfFailedTestCases += ReportTestResult((VerifyRoundTrip<8, 0>(reportTestCases)), test_tag, "round trip posit<8,0>");
	nrOfFailedTestCases += ReportTestResult((VerifyRoundTrip<8, 2>(reportTestCases)), test_tag, "round trip posit<8,2>");
	// every encoding of posit<16,1> is 65,536 parses through std::regex: about 3 s in
	// Release but nearly two minutes in the -O0 sanitizer jobs, so level 1 samples it
	// and level 2 sweeps it
	nrOfFailedTestCases += ReportTestResult((VerifyRoundTrip<16, 1>(reportTestCases, 61)), test_tag, "round trip posit<16,1>, sampled");
	nrOfFailedTestCases += ReportTestResult(VerifyWideRoundTrip(reportTestCases), test_tag, "round trip posit<80,3>");
	nrOfFailedTestCases += ReportTestResult((VerifyFractionRendering<8, 1>(reportTestCases)), test_tag, "fraction rendering <8,1>");
	nrOfFailedTestCases += ReportTestResult((VerifyFractionRendering<12, 2>(reportTestCases)), test_tag, "fraction rendering <12,2>");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult((VerifyRoundTrip<16, 1>(reportTestCases)), test_tag, "round trip posit<16,1>, every encoding");
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
