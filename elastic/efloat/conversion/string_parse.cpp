// string_parse.cpp: regression tests for string parsing of efloat
//                  (Phase E of #835 -- operator>> hygiene; #856 -- parse impl)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// efloat::parse routes decimal / scientific literals through
// sw::universal::decimal_to_binary::convert (Phase B2a), distills the
// resulting mantissa + binary_scale into efloat's multi-limb representation,
// and supports the "nan" / "inf" / "infinity" tokens.  operator>> sets
// failbit on parse failure (#835 Phase E).

#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <streambuf>
#include <string>
#include <cstdint>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_reporters.hpp>

namespace {

struct CerrSilencer {
	std::ostringstream sink;
	std::streambuf*    old;
	CerrSilencer() : old(std::cerr.rdbuf(sink.rdbuf())) {}
	~CerrSilencer() { std::cerr.rdbuf(old); }
	CerrSilencer(const CerrSilencer&)            = delete;
	CerrSilencer& operator=(const CerrSilencer&) = delete;
};

// Check the internal representation (state, sign, scale, top limb) against
// expected values.  We assert on the top limb because efloat's to_string
// for finite values is still a "TBD" stub -- the parse contract is what we
// pin, not the round-trip-via-decimal.
template<unsigned N>
int CheckNormal(const char* input,
                bool        expected_sign,
                std::int64_t expected_scale,
                std::uint32_t expected_lim0,
                bool reportTestCases) {
	using namespace sw::universal;
	efloat<N> v;
	if (!parse(input, v)) {
		if (reportTestCases) std::cout << "FAIL parse rejected: '" << input << "'\n";
		return 1;
	}
	if (v.iszero() || v.isnan() || v.isinf()) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' parsed as non-normal\n";
		return 1;
	}
	bool sign_ok  = ((v.sign() == -1) == expected_sign);
	bool scale_ok = (v.scale() == expected_scale);
	auto bits = v.bits();
	bool lim_ok = (!bits.empty() && bits[0] == expected_lim0);
	if (!sign_ok || !scale_ok || !lim_ok) {
		if (reportTestCases) {
			std::cout << "FAIL '" << input << "': got sign=" << (v.sign() == -1 ? '-' : '+')
			          << " scale=" << v.scale()
			          << " lim0=0x" << std::hex << (bits.empty() ? 0u : bits[0]) << std::dec
			          << " expected sign=" << (expected_sign ? '-' : '+')
			          << " scale=" << expected_scale
			          << " lim0=0x" << std::hex << expected_lim0 << std::dec << '\n';
		}
		return 1;
	}
	return 0;
}

template<unsigned N>
int CheckZero(const char* input, bool expected_negative, bool reportTestCases) {
	using namespace sw::universal;
	efloat<N> v;
	if (!parse(input, v)) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' rejected (expected zero)\n";
		return 1;
	}
	if (!v.iszero()) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' not zero\n";
		return 1;
	}
	if ((v.sign() == -1) != expected_negative) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' wrong zero sign\n";
		return 1;
	}
	return 0;
}

template<unsigned N>
int CheckNaN(const char* input, bool reportTestCases) {
	using namespace sw::universal;
	efloat<N> v;
	if (!parse(input, v) || !v.isnan()) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' not nan\n";
		return 1;
	}
	return 0;
}

template<unsigned N>
int CheckInf(const char* input, bool expected_negative, bool reportTestCases) {
	using namespace sw::universal;
	efloat<N> v;
	if (!parse(input, v) || !v.isinf()) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' not inf\n";
		return 1;
	}
	if ((v.sign() == -1) != expected_negative) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' wrong inf sign\n";
		return 1;
	}
	return 0;
}

template<unsigned N>
int CheckReject(const char* input, bool reportTestCases) {
	using namespace sw::universal;
	efloat<N> v;
	if (parse(input, v)) {
		if (reportTestCases) std::cout << "FAIL '" << input << "' should reject\n";
		return 1;
	}
	return 0;
}

}  // namespace

int main()
try {
	using namespace sw::universal;
	using Float = efloat<2>;  // 64-bit significand, lines up with native double layout

	std::string test_suite  = "efloat string parse (issues #835 Phase E, #856)";
	bool reportTestCases    = true;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- canonical small values: bit-exact match against the hand-derived
	//       normalized binary representation (MSB at bit 31 of _limb[0]) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckNormal<2>("1",        false,  0, 0x80000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("-1",       true,   0, 0x80000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("2",        false,  1, 0x80000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("4",        false,  2, 0x80000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("0.5",      false, -1, 0x80000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("0.25",     false, -2, 0x80000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("1.5",      false,  0, 0xC0000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("-2.5",     true,   1, 0xA0000000u, reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "canonical normals", "efloat parse");
	}

	// ----- scientific notation with large |exponent| -----
	{
		int start = nrOfFailedTestCases;
		// 1e10 = 0x9502F900_00000000 * 2^33 (verified against the d2b output;
		// scale equals floor(log2(1e10)) = 33).
		nrOfFailedTestCases += CheckNormal<2>("1e10",   false,   33, 0x9502F900u, reportTestCases);
		// 1e-10 mantissa rounds to nearest at 64 bits; scale = -34.
		nrOfFailedTestCases += CheckNormal<2>("1e-10",  false,  -34, 0xDBE6FECEu, reportTestCases);
		// 1e100 scale = 332 (floor(log2(1e100)) = 332)
		nrOfFailedTestCases += CheckNormal<2>("1e100",  false,  332, 0x924D692Cu, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("1e-100", false, -333, 0xDFF97724u, reportTestCases);
		// 3.14e2 = 314 = 0b100111010 = 1.0011101 * 2^8
		nrOfFailedTestCases += CheckNormal<2>("3.14e2", false,    8, 0x9D000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("-3.14e2", true,    8, 0x9D000000u, reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "scientific notation", "efloat parse");
	}

	// ----- 3.14 bit pattern matches the d2b output (64-bit RTNE) and
	//       agrees with native double in the top 53 bits after rounding -----
	{
		int start = nrOfFailedTestCases;
		// 3.14 normalized to 64 explicit bits with round-to-nearest-even:
		//   sig = 0xC8F5C28F_5C28F5C3, scale = 1.
		// Verify the full 64-bit pattern instead of extracting 53 bits via
		// naive truncation (which would drop the round-up that took the
		// 53-bit value from ...EB851E to ...EB851F to match native double).
		efloat<2> v;
		bool parsed = parse("3.14", v);
		auto b = v.bits();
		if (!parsed || b.size() < 2) {
			std::cout << "FAIL 3.14: parsed=" << parsed
			          << " limbs=" << b.size() << '\n';
			++nrOfFailedTestCases;
			ReportTestResult(nrOfFailedTestCases - start, "3.14 matches IEEE-754 double", "efloat parse");
			return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
		}
		std::uint64_t sig = (static_cast<std::uint64_t>(b[0]) << 32) | b[1];
		const std::uint64_t expected = 0xC8F5C28F5C28F5C3ULL;
		if (v.scale() != 1 || sig != expected) {
			std::cout << "FAIL 3.14 sig = 0x" << std::hex << sig
			          << " scale=" << std::dec << v.scale()
			          << " expected sig=0x" << std::hex << expected
			          << " scale=1\n" << std::dec;
			++nrOfFailedTestCases;
		}
		// Cross-check against native double: rounding the parsed 64-bit
		// significand to 53 bits (round-half-to-even on bit 10) must match
		// the bit pattern of 3.14 stored in an IEEE-754 double.
		const std::uint64_t round_bias = 1ULL << 10;  // round half-up at bit 11 cut
		std::uint64_t round_input = sig + round_bias;
		// If exactly halfway (bottom 11 bits == 0x400) and result bit 11 == 0,
		// strip the round bias for round-to-even.  3.14 is not exactly halfway,
		// so this branch is not taken; we add the simplest check.
		std::uint64_t top53 = round_input >> 11;
		std::uint64_t double_bits = 0x40091EB851EB851FULL;
		std::uint64_t double_sig  = (double_bits & 0x000FFFFFFFFFFFFFULL) | 0x0010000000000000ULL;
		if (top53 != double_sig) {
			std::cout << "FAIL 3.14 top53 = 0x" << std::hex << top53
			          << " double_sig = 0x" << double_sig << std::dec << '\n';
			++nrOfFailedTestCases;
		}
		ReportTestResult(nrOfFailedTestCases - start, "3.14 matches IEEE-754 double", "efloat parse");
	}

	// ----- signed zero (both sign-bearing inputs collapse to the same state) -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckZero<2>("0",       false, reportTestCases);
		nrOfFailedTestCases += CheckZero<2>("+0",      false, reportTestCases);
		nrOfFailedTestCases += CheckZero<2>("-0",      true,  reportTestCases);
		nrOfFailedTestCases += CheckZero<2>("0.0",     false, reportTestCases);
		nrOfFailedTestCases += CheckZero<2>("-0.0",    true,  reportTestCases);
		nrOfFailedTestCases += CheckZero<2>("0e10",    false, reportTestCases);
		nrOfFailedTestCases += CheckZero<2>("-0.0e5",  true,  reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "signed zero", "efloat parse");
	}

	// ----- nan / inf token routing -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckNaN<2>("nan",      reportTestCases);
		nrOfFailedTestCases += CheckNaN<2>("NaN",      reportTestCases);
		nrOfFailedTestCases += CheckNaN<2>("NAN",      reportTestCases);
		nrOfFailedTestCases += CheckInf<2>("inf",      false, reportTestCases);
		nrOfFailedTestCases += CheckInf<2>("Inf",      false, reportTestCases);
		nrOfFailedTestCases += CheckInf<2>("-inf",     true,  reportTestCases);
		nrOfFailedTestCases += CheckInf<2>("+inf",     false, reportTestCases);
		nrOfFailedTestCases += CheckInf<2>("infinity", false, reportTestCases);
		nrOfFailedTestCases += CheckInf<2>("-Infinity", true, reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "nan/inf tokens", "efloat parse");
	}

	// ----- malformed input must be rejected -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckReject<2>("",       reportTestCases);
		nrOfFailedTestCases += CheckReject<2>("abc",    reportTestCases);
		nrOfFailedTestCases += CheckReject<2>("1.2.3",  reportTestCases);
		nrOfFailedTestCases += CheckReject<2>("1e",     reportTestCases);
		nrOfFailedTestCases += CheckReject<2>(".",      reportTestCases);
		nrOfFailedTestCases += CheckReject<2>("1e3.5",  reportTestCases);
		nrOfFailedTestCases += CheckReject<2>("42x",    reportTestCases);
		nrOfFailedTestCases += CheckReject<2>("0xFF",   reportTestCases);
		nrOfFailedTestCases += CheckReject<2>("nann",   reportTestCases);
		nrOfFailedTestCases += CheckReject<2>("inff",   reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "malformed reject", "efloat parse");
	}

	// ----- operator>> failbit on bad token -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("not-a-number");
		Float p;
		{
			CerrSilencer silence;
			is >> p;
		}
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> failbit on bad", "efloat parse");
	}

	// ----- operator>> success on a valid scientific token in whitespace -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("  3.14e2  ");
		Float p;
		is >> p;
		if (is.fail() || p.iszero() || p.isnan() || p.isinf()) ++nrOfFailedTestCases;
		// scale should be 8 (314 normalized to 1.001110100 * 2^8)
		if (p.scale() != 8) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> scientific", "efloat parse");
	}

	// ----- operator>> on empty stream sets failbit -----
	{
		int start = nrOfFailedTestCases;
		std::istringstream is("");
		Float p;
		is >> p;
		if (!is.fail()) ++nrOfFailedTestCases;
		ReportTestResult(nrOfFailedTestCases - start, "operator>> empty stream", "efloat parse");
	}

	// ----- different nlimbs widths see the same scale and same top-limb pattern -----
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckNormal<1>("1.5",  false, 0, 0xC0000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<2>("1.5",  false, 0, 0xC0000000u, reportTestCases);
		nrOfFailedTestCases += CheckNormal<4>("1.5",  false, 0, 0xC0000000u, reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start, "nlimbs parity (1/2/4)", "efloat parse");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
