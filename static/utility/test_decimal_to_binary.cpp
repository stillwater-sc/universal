// test_decimal_to_binary.cpp: tests for the high-precision decimal-to-binary
//                            converter utility (Phase B2a of issue #835)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.

#include <universal/utility/directives.hpp>
#include <cstdint>
#include <cstring>      // std::memcpy
#include <string>
#include <universal/utility/decimal_to_binary.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal { namespace decimal_to_binary {

// Encode a successful decimal_to_binary::result as IEEE-754 binary64 with
// round-half-to-even semantics. Returns the encoded double bit-pattern as
// uint64_t (so callers can compare bit-for-bit).
//
// Convention: convert() was called with target_mantissa_bits = 53, so
// `r.mantissa` has its MSB at position 52 (the IEEE double mantissa width).
// The MSB is the implicit-leading-1 bit.
//
// Returns false on overflow / underflow into denormals / extreme cases so the
// caller knows to skip those (we still set out_bits to something reasonable).
inline bool encode_as_double(const result& r, std::uint64_t& out_bits) {
	const std::uint64_t sign_bit = r.negative ? (std::uint64_t(1) << 63) : 0;
	if (r.is_zero) {
		out_bits = sign_bit;  // signed zero
		return true;
	}
	// mantissa is in [2^52, 2^53).
	// Lift out the low 53 bits as uint64_t.
	std::uint64_t mant = 0;
	for (int i = 52; i >= 0; --i) {
		mant <<= 1;
		if (r.mantissa.test(static_cast<unsigned>(i))) mant |= 1u;
	}
	// Round-half-to-even using guard + sticky.
	bool round_up = false;
	if (r.guard_bit) {
		if (r.sticky_bit) round_up = true;
		else              round_up = (mant & 1u) != 0;  // tie -> round to even
	}
	std::int64_t scale = r.binary_scale;
	if (round_up) {
		mant += 1;
		if (mant == (std::uint64_t(1) << 53)) {
			mant >>= 1;
			scale += 1;
		}
	}
	// IEEE bias.
	std::int64_t biased_exp = scale + 1023;
	if (biased_exp >= 2047) {
		// Overflow -> infinity.
		out_bits = sign_bit | (std::uint64_t(2047) << 52);
		return false;
	}
	if (biased_exp <= 0) {
		// Subnormal or underflow; not covered by this simple encoder.
		out_bits = sign_bit;  // best-effort: signed zero
		return false;
	}
	// Strip the implicit leading 1.
	std::uint64_t frac52 = mant & ((std::uint64_t(1) << 52) - 1);
	out_bits = sign_bit | (std::uint64_t(biased_exp) << 52) | frac52;
	return true;
}

inline std::uint64_t double_to_bits(double d) {
	std::uint64_t bits;
	std::memcpy(&bits, &d, sizeof(bits));
	return bits;
}

}}}  // namespace sw::universal::decimal_to_binary

namespace sp = sw::universal::string_parse;
namespace d2b = sw::universal::decimal_to_binary;

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "decimal_to_binary high-precision converter (Phase B2a of #835)";
	std::string test_tag    = "decimal_to_binary";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----- direct output checks for hand-computable values (53-bit target) -----
	{
		int start = nrOfFailedTestCases;

		// 1.0 -> mantissa = 2^52, binary_scale = 0, no guard/sticky
		{
			auto r = d2b::convert("1.0", 53);
			if (!r.valid || r.negative || r.is_zero
			    || r.binary_scale != 0
			    || r.guard_bit || r.sticky_bit
			    || !r.mantissa.test(52) || r.mantissa.test(53)) ++nrOfFailedTestCases;
		}
		// 2.0 -> mantissa = 2^52, binary_scale = 1
		{
			auto r = d2b::convert("2.0", 53);
			if (!r.valid || r.binary_scale != 1
			    || !r.mantissa.test(52) || r.mantissa.test(53)
			    || r.guard_bit || r.sticky_bit) ++nrOfFailedTestCases;
		}
		// 0.5 -> mantissa = 2^52, binary_scale = -1
		{
			auto r = d2b::convert("0.5", 53);
			if (!r.valid || r.binary_scale != -1
			    || !r.mantissa.test(52) || r.mantissa.test(53)
			    || r.guard_bit || r.sticky_bit) ++nrOfFailedTestCases;
		}
		// 3.0 = 1.1b * 2^1 -> mantissa has bits 52 and 51 set, binary_scale = 1
		{
			auto r = d2b::convert("3.0", 53);
			if (!r.valid || r.binary_scale != 1
			    || !r.mantissa.test(52) || !r.mantissa.test(51)
			    || r.mantissa.test(53)
			    || r.guard_bit || r.sticky_bit) ++nrOfFailedTestCases;
		}
		// -1.0
		{
			auto r = d2b::convert("-1.0", 53);
			if (!r.valid || !r.negative || r.binary_scale != 0
			    || !r.mantissa.test(52)) ++nrOfFailedTestCases;
		}
		// 0 (and variants)
		{
			auto r = d2b::convert("0", 53);
			if (!r.valid || !r.is_zero) ++nrOfFailedTestCases;
		}
		{
			auto r = d2b::convert("0.0", 53);
			if (!r.valid || !r.is_zero) ++nrOfFailedTestCases;
		}
		{
			auto r = d2b::convert("-0.0", 53);
			if (!r.valid || !r.is_zero || !r.negative) ++nrOfFailedTestCases;
		}
		// Empty -> invalid
		{
			auto r = d2b::convert("", 53);
			if (r.valid) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: hand-computable cases\n";
	}

	// ----- bit-for-bit comparison against std::stod for canonical doubles -----
	{
		int start = nrOfFailedTestCases;
		const char* cases[] = {
			"1.0",
			"2.0",
			"3.0",
			"0.5",
			"-1.0",
			"1.5",
			"3.14",
			"3.141592653589793",
			"1e0",
			"1e10",
			"1e-10",
			"42",
			"100",
			"1000000",
			"-3.14e2",
			"123456789",
			"1.23456789e+15",
			// Powers of 2 are exact in both decimal and binary
			"4",
			"8",
			"16",
			"1024",
			"4503599627370496",  // 2^52
			// Powers of 10 -- non-trivial decimal-to-binary
			"10",
			"100000",
			"1e100",
			"1e-100",
		};
		for (const char* s : cases) {
			auto r = d2b::convert(s, 53);
			std::uint64_t ours = 0;
			bool encoded = d2b::encode_as_double(r, ours);
			std::uint64_t theirs = d2b::double_to_bits(std::stod(s));
			// None of the cases in this list should overflow or underflow
			// into denormals when encoded as IEEE double. If encode_as_double
			// returns false, that's a real regression -- a converter bug, a
			// changed encoder, or a misclassified case -- not something to
			// silently skip.
			if (!encoded) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  encode_as_double returned false on \"" << s << "\"\n";
				}
				continue;
			}
			if (ours != theirs) {
				++nrOfFailedTestCases;
				if (reportTestCases) {
					std::cout << "  mismatch on \"" << s << "\":\n"
					          << "    ours   = 0x" << std::hex << ours   << '\n'
					          << "    stod() = 0x" << std::hex << theirs << '\n' << std::dec;
				}
			}
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: stod oracle comparison\n";
	}

	// ----- target_mantissa_bits bounds validation -----
	{
		int start = nrOfFailedTestCases;
		// 0 mantissa bits is meaningless: must reject.
		{
			auto r = d2b::convert("1.0", 0);
			if (r.valid) ++nrOfFailedTestCases;
		}
		// Above the internal bigint width is unreachable: must reject.
		{
			auto r = d2b::convert("1.0", d2b::default_big_bits + 1u);
			if (r.valid) ++nrOfFailedTestCases;
		}
		// Largest valid target_mantissa_bits should still succeed for a
		// trivially-representable value.
		{
			auto r = d2b::convert("1.0", d2b::default_big_bits);
			if (!r.valid || r.is_zero) ++nrOfFailedTestCases;
		}
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: target_mantissa_bits bounds\n";
	}

	// ----- the classic 0.1 (non-terminating binary): match stod -----
	{
		int start = nrOfFailedTestCases;
		auto r = d2b::convert("0.1", 53);
		std::uint64_t ours = 0;
		bool encoded = d2b::encode_as_double(r, ours);
		std::uint64_t theirs = d2b::double_to_bits(0.1);
		if (!encoded || ours != theirs) ++nrOfFailedTestCases;
		// 0.2
		r = d2b::convert("0.2", 53);
		encoded = d2b::encode_as_double(r, ours);
		theirs = d2b::double_to_bits(0.2);
		if (!encoded || ours != theirs) ++nrOfFailedTestCases;
		// 0.3
		r = d2b::convert("0.3", 53);
		encoded = d2b::encode_as_double(r, ours);
		theirs = d2b::double_to_bits(0.3);
		if (!encoded || ours != theirs) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) std::cout << "FAIL: non-terminating-binary cases\n";
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
