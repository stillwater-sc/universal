// string_parse.cpp: regression tests for decimal-string parsing of hfloat
//                  (Resolves issue #849)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// hfloat is IBM System/360 hexadecimal floating-point. Notable constraints:
//   - No NaN or Inf encoding: nan / inf / infinity tokens are rejected
//   - Truncation rounding only (no round-up at the cut)
//   - Overflow saturates to maxpos / maxneg
//   - Wobbling precision: 0-3 leading zero bits in the MSB hex digit

#include <universal/utility/directives.hpp>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>
// Configure the hfloat template environment
#define HFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/verification/test_suite.hpp>


// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
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

namespace {
struct CerrSilencer {
	std::ostringstream sink;
	std::streambuf*    old;
	CerrSilencer() : old(std::cerr.rdbuf(sink.rdbuf())) {}
	~CerrSilencer() { std::cerr.rdbuf(old); }
	CerrSilencer(const CerrSilencer&)            = delete;
	CerrSilencer& operator=(const CerrSilencer&) = delete;
};
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "hfloat decimal string parse (issue #849)";
	bool reportTestCases    = false;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// generate individual testcases to hand trace/debug

	hfp32 a, b, c;
	parse("0.1", a);
	parse("0.0625", b);
	c = a + b;
	ReportValue(a, "a");
	ReportValue(b, "b");
	ReportValue(c, "a + b");

	a.setbits(0x7FFF'FFFF);
	ReportValue(a, "0x7FFF'FFFF");
	b.maxpos();
	ReportValue(b, "maxpos");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#if REGRESSION_LEVEL_1

	// ----- hfp32: canonical decimals match constructor -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "0",     0.0   },
			{ "1.0",   1.0   },
			{ "-1.0", -1.0   },
			{ "1.5",   1.5   },
			{ "-3.25", -3.25 },
			{ "0.5",   0.5   },
			{ "16.0",  16.0  },  // exact in hex-float (one hex digit)
		    { "256.0", 256.0}, 
			{ "1.25e3", 1250.0}, 
			{ "7.237005e+75", 7.237005e+75},
		};
		for (const auto& c : cases) {
			hfp32 ours, ref(c.v);
			if (!parse(c.s, ours)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  parse failed: " << c.s << '\n';
				continue;
			}
			if (ours != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  mismatch on \"" << c.s << "\":\n"
					          << "    string -> " << double(ours) << '\n'
					          << "    ref    -> " << double(ref)  << '\n';
				}
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfp32 canonical decimals\n";
	}

	// ----- hfp64: canonical decimals match constructor -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; double v; };
		Case cases[] = {
			{ "1.0",        1.0      },
			{ "-2.5",      -2.5      },
			{ "0.25",       0.25     },
			{ "100",        100.0    },
			{ "1024",       1024.0   },
			{ "0.0625",     0.0625   },  // 2^-4 = 16^-1, exact
			{ "1e10",       1e10     },
		};
		for (const auto& c : cases) {
			hfp64 ours, ref(c.v);
			if (!parse(c.s, ours)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  parse failed: " << c.s << '\n';
				continue;
			}
			if (ours != ref) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  mismatch on \"" << c.s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfp64 canonical decimals\n";
	}

	// precision-win: hfp64 has 56 fbits, which is MORE than double's 53 bits.
	// The new d2b-based parse delivers a 56-bit-exact result, while
	// the legacy via-double path inherited IEEE round-to-nearest plus a
	// trailing-zero pad. Pin both bit patterns to lock the contract.
	{
		int start = nrOfFailedTestCases;
		hfp64 via_string, via_double(0.1);
		if (!parse("0.1", via_string)) ++nrOfFailedTestCases;
		// Direct d2b + truncation rounding yields ...01100110011001 (the last
		// hex digit is 9, truncated from the 0.1 repeating pattern). The via-
		// double path inherited IEEE round-to-nearest's bump on bit 53, which
		// became hex digit A. Both are valid binary patterns but only the
		// truncated one matches hfloat's documented truncation-rounding
		// contract.
		const std::string pinned_via_string =
		    "0b0.1000000.00011001100110011001100110011001100110011001100110011001";
		if (to_binary(via_string) != pinned_via_string) {
			++nrOfFailedTestCases;
			if (reportTestCases) {
				std::cout << "  pin mismatch for 0.1 (d2b path):\n"
				          << "    got      " << to_binary(via_string) << '\n'
				          << "    expected " << pinned_via_string     << '\n';
			}
		}
		if (via_string == via_double) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << "  expected divergence vs via-double; got identity\n";
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfp64 precision-win for 0.1\n";
	}

	// ----- hfp128 parse correctness. fbits=112; parse()
	//       routes the decimal payload through decimal_to_binary requesting
	//       fbits of precision and feeds the multi-limb mantissa straight
	//       into the hex-aligned fraction. The whole point of parse() is to
	//       deliver precision beyond what double / long double can carry --
	//       a value like 1.23456789e-10 in hfp128 has 112 meaningful fraction
	//       bits, far past double's 53. Issue #870 documented the prior bug
	//       where parse() clipped through a uint64_t intermediate, producing
	//       either 8.27e-25 (saturation) or hfp64-precision-in-hfp128 (the
	//       first fix attempt) instead of the full 112 bits. -----
	{
		int start = nrOfFailedTestCases;
		using ExtH = hfloat<28, 7, std::uint32_t>;
		ExtH via_parse, via_double(1.0);
		if (!parse("1.0", via_parse)) ++nrOfFailedTestCases;
		if (via_parse.iszero())       ++nrOfFailedTestCases;
		// 1.0 happens to land in a single fraction bit (bit 108) for both
		// paths, so via_parse == via_double here. For non-trivial decimals
		// the wide d2b path now LEGITIMATELY diverges from via_double --
		// parse delivers full 112 bits, via_double only ~53.
		if (via_parse != via_double)  ++nrOfFailedTestCases;
		// Saturation still works for hfp128
		if (!parse("1e1000", via_parse)) ++nrOfFailedTestCases;
		if (via_parse != ExtH(SpecificValue::maxpos)) ++nrOfFailedTestCases;

		// Issue #870 regression: hfp128 must parse small-magnitude decimals
		// to the correct value, NOT to the all-ones-saturation garbage in
		// the low fraction bits that the pre-fix code produced.
		ExtH ext_small;
		if (!parse("1.23456789e-10", ext_small)) ++nrOfFailedTestCases;
		if (ext_small.iszero())                  ++nrOfFailedTestCases;
		if (ext_small == ExtH(SpecificValue::maxpos)) ++nrOfFailedTestCases;
		// Round-trip via double catches the original bug decisively: pre-fix
		// produced 8.27e-25 (15 orders of magnitude off); the post-fix value
		// is bounded by double's 53-bit window on the parsed bits, hence the
		// generous tolerance below relative to the round-trip's true error.
		{
			double d = double(ext_small);
			double expected = 1.23456789e-10;
			double rel_err  = (d - expected) / expected;
			if (rel_err < 0) rel_err = -rel_err;
			if (rel_err > 1.0e-6) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  hfp128 parse(\"1.23456789e-10\") = " << d
					          << " (expected ~" << expected
					          << ", rel_err=" << rel_err << ")\n";
				}
			}
		}
		// hfp128 unbiased exponent must agree with hfp64's for the same
		// input (the bug case had the right exponent but garbage fraction;
		// keep this invariant pinned to catch fraction-corruption regressions
		// that happen to land in a valid exponent slot).
		{
			hfp64 ref;
			parse("1.23456789e-10", ref);
			if (ext_small.scale() != ref.scale()) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  hfp128 scale " << ext_small.scale()
					          << " differs from hfp64 scale " << ref.scale() << '\n';
				}
			}
		}
		// parse() must deliver precision past double's 53 bits -- the very
		// reason for routing decimal literals through d2b instead of a
		// double round-trip. Pin this by parsing a long pi literal and
		// checking that the fraction bits below position (fbits - 53) =
		// (112 - 53) = 59 are not all zero. If parse() ever regresses to
		// a uint64_t intermediate this assertion fires.
		{
			ExtH ext_pi;
			const char* pi_str = "3.141592653589793238462643383279502884197169";
			if (!parse(pi_str, ext_pi)) ++nrOfFailedTestCases;
			bool any_low_bit_set = false;
			for (unsigned i = 0; i < 59u; ++i) {
				if (ext_pi.getbit(i)) { any_low_bit_set = true; break; }
			}
			if (!any_low_bit_set) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  hfp128 parse(pi) populated only bits >= 59 --"
					             " parse() regressed to double-precision intermediate\n"
					          << "  bits: " << to_binary(ext_pi) << '\n';
				}
			}
		}

		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfp128 smoke + saturation + #870\n";
	}

	// ----- scientific notation across a wide |e| range. Verify parse
	//       succeeds and the result has the expected sign + non-zero
	//       (or properly saturated) shape. For non-exact-in-double
	//       inputs the direct d2b path legally diverges from
	//       hfp64(double_val) -- the precision-win pinned for
	//       parse("0.1", ...) above generalizes here, so we don't
	//       compare against via-double. -----
	{
		int start = nrOfFailedTestCases;
		struct Case { const char* s; bool negative; };
		Case cases[] = {
			{ "1e0",        false },
			{ "1e10",       false },
			{ "1e-10",      false },
			{ "1e50",       false },
			{ "1e-50",      false },
			{ "1.5e30",     false },
			{ "-3.14e-25",  true  },
			{ "-1.5e30",    true  },
		};
		for (const auto& c : cases) {
			hfp64 p;
			if (!parse(c.s, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  parse failed: " << c.s << '\n';
				continue;
			}
			if (p.iszero()) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  unexpectedly zero: " << c.s << '\n';
				continue;
			}
			if (p.sign() != c.negative) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  sign mismatch on " << c.s << '\n';
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat scientific-notation sweep\n";
	}

	// ----- signed zero: hfloat has only one zero encoding, so all "-0"
	//       spellings parse to that same zero (no signed zero) -----
	{
		int start = nrOfFailedTestCases;
		hfp64 p;
		for (const char* s : { "0", "0.0", "-0", "-0.0", "+0", "+0.0" }) {
			if (!parse(s, p)) ++nrOfFailedTestCases;
			if (!p.iszero()) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat zero / signed-zero handling\n";
	}

	// ----- nan / inf tokens are REJECTED (hfloat has no NaN/Inf encoding) -----
	{
		int start = nrOfFailedTestCases;
		hfp32 p;
		for (const char* s : { "nan", "NaN", "+nan", "-nan",
		                       "inf", "Inf", "infinity", "INFINITY",
		                       "+inf", "-inf", "-infinity" }) {
			if (parse(s, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  unexpectedly accepted: \"" << s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat nan/inf rejection\n";
	}

	// ----- overflow saturates to maxpos / maxneg (no Inf) -----
	{
		int start = nrOfFailedTestCases;
		hfp32 p, maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
		// hfp32 max is 16^63 ~ 7.24e75. Use a value well above that.
		if (!parse("1e100", p)) ++nrOfFailedTestCases;
		if (p != maxpos)        ++nrOfFailedTestCases;
		if (!parse("-1e100", p)) ++nrOfFailedTestCases;
		if (p != maxneg)         ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat overflow saturation\n";
	}

	// ----- malformed input is rejected -----
	{
		int start = nrOfFailedTestCases;
		hfp32 p;
		for (const char* s : { "", "+", "-", ".", "e10",
		                       "not-a-number", "abc", "1.5abc",
		                       "1.5.0", "1e", "1ea" }) {
			if (parse(s, p)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "  unexpectedly accepted: \"" << s << "\"\n";
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat malformed input rejection\n";
	}

	// ----- operator>> sets failbit on a bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		hfp32 p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat operator>> failbit\n";
	}

	// ----- operator>> handles EOF (empty stream) without crashing -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("");
		hfp32 p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hfloat operator>> EOF\n";
	}

#	endif

#	if REGRESSION_LEVEL_2
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
