// codec.cpp: verification of the shared takum field codec (takum_codec.hpp)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// takum_codec<nbits, rbits> owns everything the linear and logarithmic takum
// formats have in common: the (S, D, R, C, M) bit layout, the DR / characteristic
// / mantissa geometry, decode, and the rounded encode path.  Each variant supplies
// only the value map.  This suite locks in the representation invariants stated in
// takum_codec.hpp and docs/takum-design.md, so that adding takum_log<> on top of
// the same codec cannot silently perturb the linear format:
//
//   I1  c_stored_bits + p == maxCharBits, c_stored_bits == min(r, maxCharBits)
//   I2  decode(mag).c in [min_characteristic(), max_characteristic()]
//   I3  decode(mag).M_bits < 2^p
//   I4  encode_*() never sets the sign bit
//   I5  encode_exact(decode(mag)) == mag        (exhaustive over all magnitudes)
//   I6  mag1 < mag2 => (c1,m1) < (c2,m2)        (Prop. 4 at the codec level)
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cstdint>
#include <type_traits>
#include <universal/number/takum/takum.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// I1: the C and M fields together always occupy exactly maxCharBits bits.
template<unsigned nbits, unsigned rbits>
int VerifyLayoutInvariant(bool reportTestCases) {
	using Codec = sw::universal::takum_codec<nbits, rbits>;
	int nrOfFailedTests = 0;
	for (unsigned dr = 0; dr < Codec::nr_dr_values; ++dr) {
		auto g = Codec::layout_of(dr);
		unsigned expected_c = (g.r < Codec::maxCharBits) ? g.r : Codec::maxCharBits;
		if (g.c_stored_bits != expected_c || g.c_stored_bits + g.p != Codec::maxCharBits) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL I1 dr=" << dr << " r=" << g.r
				          << " c_stored_bits=" << g.c_stored_bits << " p=" << g.p
				          << " maxCharBits=" << Codec::maxCharBits << '\n';
			}
		}
		if (g.r != Codec::dr_to_r(dr)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL I1 r mismatch dr=" << dr << '\n';
		}
	}
	return nrOfFailedTests;
}

// I2, I3, I5, I6: exhaustive over every magnitude a nbits takum can hold.
template<unsigned nbits, unsigned rbits>
int VerifyCodecRoundTrip(bool reportTestCases) {
	using Codec = sw::universal::takum_codec<nbits, rbits>;
	int nrOfFailedTests = 0;

	const uint64_t limit = 1ull << (nbits - 1);   // magnitudes are [0, 2^(nbits-1))
	int64_t  prev_c = 0;
	uint64_t prev_m_num = 0;
	unsigned prev_p = 0;
	bool have_prev = false;

	for (uint64_t mag = 1; mag < limit; ++mag) {
		auto d = Codec::decode(mag);

		// I2: characteristic stays inside the format's range
		if (d.c < Codec::min_characteristic() || d.c > Codec::max_characteristic()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL I2 mag=" << mag << " c=" << d.c << '\n';
		}
		// I3: fraction is in [0,1)
		if (d.p > 0 && d.M_bits >= (1ull << d.p)) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL I3 mag=" << mag << " M=" << d.M_bits << " p=" << d.p << '\n';
		}
		if (d.p == 0 && d.M_bits != 0) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL I3 mag=" << mag << " p==0 but M!=0\n";
		}

		// I5: exact round-trip through encode_exact
		auto e = Codec::encode_exact(d.c, d.M_bits);
		if (!e.ok() || e.magnitude != mag) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL I5 mag=" << mag << " -> c=" << d.c << " M=" << d.M_bits
				          << " -> mag'=" << e.magnitude << " status=" << int(e.status) << '\n';
			}
		}
		// I4: the codec never sets the sign bit
		if (!e.sign_bit_clear()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL I4 mag=" << mag << " encode set the sign bit\n";
		}

		// I6: increasing magnitude implies lexicographically increasing (c, m).
		// Compare fractions on a common denominator: m = M_bits / 2^p.
		if (have_prev) {
			bool ordered;
			if (d.c != prev_c) {
				ordered = (prev_c < d.c);
			}
			else {
				// same characteristic implies the same p, so compare numerators
				ordered = (prev_p == d.p) ? (prev_m_num <= d.M_bits) : false;
			}
			if (!ordered) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL I6 mag=" << mag << " (c=" << d.c << ",M=" << d.M_bits
					          << ") not >= previous (c=" << prev_c << ",M=" << prev_m_num << ")\n";
				}
			}
		}
		prev_c = d.c; prev_m_num = d.M_bits; prev_p = d.p; have_prev = true;
	}
	return nrOfFailedTests;
}

// encode_rounded agrees with encode_exact when the fraction lands exactly on a
// representable trailing-field value, and saturates outside the characteristic range.
template<unsigned nbits, unsigned rbits>
int VerifyEncodeRounded(bool reportTestCases) {
	using Codec = sw::universal::takum_codec<nbits, rbits>;
	using sw::universal::takum_encode_status;
	int nrOfFailedTests = 0;

	const uint64_t limit = 1ull << (nbits - 1);
	for (uint64_t mag = 1; mag < limit; ++mag) {
		auto d = Codec::decode(mag);
		double m = d.fraction();
		auto e = Codec::encode_rounded(d.c, m);
		if (!e.ok() || e.magnitude != mag) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL encode_rounded mag=" << mag << " c=" << d.c << " m=" << m
				          << " -> " << e.magnitude << " status=" << int(e.status) << '\n';
			}
		}
	}

	// saturation reporting: the codec signals, the caller decides
	auto over = Codec::encode_rounded(Codec::max_characteristic() + 1, 0.0);
	if (over.status != takum_encode_status::overflow) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL encode_rounded did not report overflow\n";
	}
	auto under = Codec::encode_rounded(Codec::min_characteristic() - 1, 0.0);
	if (under.status != takum_encode_status::underflow) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL encode_rounded did not report underflow\n";
	}
	return nrOfFailedTests;
}

// Wide trailing fields.  takum_codec<64,3> reaches p = 59, past a double's 53
// significand bits, so an exhaustive sweep is impossible and the two encode
// entry points diverge in what they can carry:
//
//   encode_exact   takes M_bits as an integer and is lossless at every p
//   encode_rounded takes a double, whose scale step m * 2^p stays exact at any
//                  p, but which cannot express a trailing field needing more
//                  than 53 significant bits in the first place
//
// This is the reason the two entry points exist; verify both halves of it.
template<unsigned nbits, unsigned rbits>
int VerifyWideTrailingField(bool reportTestCases) {
	using Codec = sw::universal::takum_codec<nbits, rbits>;
	int nrOfFailedTests = 0;

	for (unsigned dr = 0; dr < Codec::nr_dr_values; ++dr) {
		auto g = Codec::layout_of(dr);
		int64_t c = Codec::dr_to_c_bias(dr);

		// A spread of trailing-field patterns, including ones that need the full
		// width and would not survive a trip through a double.
		const uint64_t patterns[] = {
			0ull,
			1ull,
			(g.p > 0) ? ((1ull << g.p) - 1ull) : 0ull,                       // all ones
			(g.p > 1) ? (1ull << (g.p - 1)) : 0ull,                          // top bit
			(g.p > 0) ? (((1ull << (g.p - 1))) | 1ull) : 0ull,               // top and bottom
		};
		for (uint64_t M : patterns) {
			if (g.p == 0 && M != 0) continue;          // I3 precondition
			if (g.p > 0 && M >= (1ull << g.p)) continue;

			auto e = Codec::encode_exact(c, M);
			if (!e.ok() || !e.sign_bit_clear()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL wide encode_exact dr=" << dr << " M=" << M << '\n';
				continue;
			}
			auto d = Codec::decode(e.magnitude);
			if (d.c != c || d.M_bits != M || d.p != g.p) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL wide round-trip dr=" << dr << " p=" << g.p
					          << " M=" << M << " -> c=" << d.c << " M'=" << d.M_bits << '\n';
				}
			}
		}

		// encode_rounded introduces no error of its own: whenever the fraction is
		// one a double can hold exactly, it must reproduce encode_exact's result.
		//
		// Two fractions, chosen to separate the two ways this can be true:
		//   m = 0.5                 exactly representable at EVERY p, so this
		//                           reaches p = 59 and demonstrates that the
		//                           m * 2^p scale step is exact past 53 bits
		//   m = (2^(p-1) + 1)/2^p   needs p significant bits, so a double can
		//                           only carry it while p <= 53
		uint64_t hard = (g.p > 0) ? ((1ull << (g.p - 1)) | 1ull) : 0ull;
		const uint64_t rounded_cases[] = {
			(g.p > 0) ? (1ull << (g.p - 1)) : 0ull,
			(g.p > 0 && g.p <= 53) ? hard : 0ull,
		};
		for (uint64_t M : rounded_cases) {
			if (g.p == 0 || M == 0) continue;
			auto exact   = Codec::encode_exact(c, M);
			auto rounded = Codec::encode_rounded(c, static_cast<double>(M) / static_cast<double>(1ull << g.p));
			if (!rounded.ok() || rounded.magnitude != exact.magnitude) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL wide encode_rounded dr=" << dr << " p=" << g.p << " M=" << M
					          << " exact=" << exact.magnitude << " rounded=" << rounded.magnitude << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

} // anonymous namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "takum shared codec verification";
	std::string test_tag    = "codec";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----------------------------------------------------------------------------
	// Compile-time: the codec's geometry matches what takum<> re-exports, so the
	// extraction cannot drift from the class API that manipulators, numeric_limits
	// and api/constexpr.cpp depend on.
	// ----------------------------------------------------------------------------
	{
		using tk16 = takum<16, 3, std::uint8_t>;
		using C16  = takum_codec<16, 3>;
		static_assert(std::is_same_v<tk16::Codec, C16>, "takum<>::Codec is takum_codec<nbits, rbits>");
		static_assert(tk16::overhead == C16::overhead,           "overhead re-export");
		static_assert(tk16::dr_bits == C16::dr_bits,             "dr_bits re-export");
		static_assert(tk16::nr_dr_values == C16::nr_dr_values,   "nr_dr_values re-export");
		static_assert(tk16::max_r == C16::max_r,                 "max_r re-export");
		static_assert(tk16::r_mask == C16::r_mask,               "r_mask re-export");
		static_assert(tk16::dr_field_mask == C16::dr_field_mask, "dr_field_mask re-export");
		static_assert(tk16::maxCharBits == C16::maxCharBits,     "maxCharBits re-export");
		static_assert(tk16::max_characteristic() == C16::max_characteristic(), "max_characteristic re-export");
		static_assert(tk16::min_characteristic() == C16::min_characteristic(), "min_characteristic re-export");

		// I1 holds at compile time for every DR of the reference configuration
		static_assert(C16::layout_of(0).c_stored_bits + C16::layout_of(0).p == C16::maxCharBits, "I1 dr=0");
		static_assert(C16::layout_of(15).c_stored_bits + C16::layout_of(15).p == C16::maxCharBits, "I1 dr=15");

		// decode / encode_exact are constant-evaluable
		constexpr auto d = C16::decode(1ull);
		static_assert(d.c == C16::min_characteristic(), "constexpr decode of minpos magnitude");
		constexpr auto e = C16::encode_exact(d.c, d.M_bits);
		static_assert(e.magnitude == 1ull, "constexpr encode_exact round-trip");
		static_assert(e.status == takum_encode_status::ok, "constexpr encode_exact status");
	}

	// ----------------------------------------------------------------------------
	// I1: field geometry, over every DR of every supported regime width
	// ----------------------------------------------------------------------------
	nrOfFailedTestCases += ReportTestResult(VerifyLayoutInvariant<16, 1>(reportTestCases), "codec<16,1>", "layout I1");
	nrOfFailedTestCases += ReportTestResult(VerifyLayoutInvariant<16, 2>(reportTestCases), "codec<16,2>", "layout I1");
	nrOfFailedTestCases += ReportTestResult(VerifyLayoutInvariant<16, 3>(reportTestCases), "codec<16,3>", "layout I1");
	nrOfFailedTestCases += ReportTestResult(VerifyLayoutInvariant<16, 5>(reportTestCases), "codec<16,5>", "layout I1");
	nrOfFailedTestCases += ReportTestResult(VerifyLayoutInvariant<32, 3>(reportTestCases), "codec<32,3>", "layout I1");

	// ----------------------------------------------------------------------------
	// I2/I3/I4/I5/I6: exhaustive over every magnitude.  takum<16,5> and takum<12,5>
	// exercise the degenerate r > maxCharBits path where C is stored truncated.
	// ----------------------------------------------------------------------------
	nrOfFailedTestCases += ReportTestResult(VerifyCodecRoundTrip< 8, 2>(reportTestCases), "codec< 8,2>", "round-trip I2/I3/I5/I6");
	nrOfFailedTestCases += ReportTestResult(VerifyCodecRoundTrip<12, 1>(reportTestCases), "codec<12,1>", "round-trip I2/I3/I5/I6");
	nrOfFailedTestCases += ReportTestResult(VerifyCodecRoundTrip<12, 3>(reportTestCases), "codec<12,3>", "round-trip I2/I3/I5/I6");
	nrOfFailedTestCases += ReportTestResult(VerifyCodecRoundTrip<12, 5>(reportTestCases), "codec<12,5>", "round-trip I2/I3/I5/I6");
	nrOfFailedTestCases += ReportTestResult(VerifyCodecRoundTrip<16, 3>(reportTestCases), "codec<16,3>", "round-trip I2/I3/I5/I6");
	nrOfFailedTestCases += ReportTestResult(VerifyCodecRoundTrip<16, 5>(reportTestCases), "codec<16,5>", "round-trip I2/I3/I5/I6");

	// ----------------------------------------------------------------------------
	// encode_rounded: agreement with encode_exact on representable fractions,
	// and saturation signalling at the range boundaries
	// ----------------------------------------------------------------------------
	nrOfFailedTestCases += ReportTestResult(VerifyEncodeRounded< 8, 2>(reportTestCases), "codec< 8,2>", "encode_rounded");
	nrOfFailedTestCases += ReportTestResult(VerifyEncodeRounded<12, 3>(reportTestCases), "codec<12,3>", "encode_rounded");
	nrOfFailedTestCases += ReportTestResult(VerifyEncodeRounded<16, 3>(reportTestCases), "codec<16,3>", "encode_rounded");
	nrOfFailedTestCases += ReportTestResult(VerifyEncodeRounded<16, 5>(reportTestCases), "codec<16,5>", "encode_rounded");

	// ----------------------------------------------------------------------------
	// Wide trailing fields: codec<64,3> reaches p = 59, past a double's 53
	// significand bits.  Exercises the reason encode_exact and encode_rounded are
	// separate entry points.
	// ----------------------------------------------------------------------------
	nrOfFailedTestCases += ReportTestResult(VerifyWideTrailingField<32, 3>(reportTestCases), "codec<32,3>", "wide trailing field");
	nrOfFailedTestCases += ReportTestResult(VerifyWideTrailingField<56, 3>(reportTestCases), "codec<56,3>", "wide trailing field");
	nrOfFailedTestCases += ReportTestResult(VerifyWideTrailingField<64, 3>(reportTestCases), "codec<64,3>", "wide trailing field");
	nrOfFailedTestCases += ReportTestResult(VerifyWideTrailingField<64, 5>(reportTestCases), "codec<64,5>", "wide trailing field");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
