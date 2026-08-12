// api.cpp: application programming interface tests for the logarithmic takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// takum_log<nbits, rbits, bt> is the LOGARITHMIC takum: arXiv:2404.18603
// Definition 2, restated as Definition 1 of arXiv:2408.10594.  It shares the
// entire (S, D, R, C, M) encoding with the linear takum<> through
// takum_codec<nbits, rbits> and differs only in the value map:
//
//     takum<>      |value| = (1 + f) * 2^c
//     takum_log<>  |value| = sqrt(e)^(c + m)
//
// This suite pins the properties that make it a distinct number system rather
// than a reinterpretation, in particular the two the paper singles out:
//
//   Proposition 6   negation is the two's complement of the whole word
//                   (shared with the linear takum)
//   Proposition 7   INVERSION is the two's complement of everything but the
//                   sign bit -- exact, and absent from the linear takum
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <universal/number/takum/takum_log.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// Values whose magnitude leaves double's range are unusable for a
// double-mediated comparison.  This is a property of wide rbits configurations
// (rbits=5 pushes the characteristic to ~2^32) and affects the linear takum
// equally; the specification fixes rbits=3, where everything is representable.
template<typename TL>
bool comparable(const TL& v) {
	if (v.iszero() || v.isnar()) return false;
	double d = double(v);
	return std::isfinite(d) && d != 0.0;
}

// Proposition 6: negating the two's complement integer negates the value.
template<unsigned nbits, unsigned rbits>
int VerifyNegation(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	for (uint64_t b = 0; b < (1ull << nbits); ++b) {
		TL x; x.setbits(b);
		if (!comparable(x)) continue;
		TL n = -x;
		if (double(n) != -double(x)) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL Prop6 bits=" << b << " x=" << double(x) << " -x=" << double(n) << '\n';
			}
		}
		if ((-n).raw_bits() != x.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL Prop6 not involutive bits=" << b << '\n';
		}
	}
	return nrOfFailedTests;
}

// Proposition 7: inversion is a bit operation and is EXACT.  This is the
// property the linear takum lacks, so it is the sharpest test that takum_log is
// really the logarithmic format and not a mislabelled takum<>.
template<unsigned nbits, unsigned rbits>
int VerifyInversion(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	for (uint64_t b = 0; b < (1ull << nbits); ++b) {
		TL x; x.setbits(b);
		if (x.isnar()) continue;
		if (x.iszero()) {
			// 1/0 is NaR by the proposition
			if (!x.reciprocal().isnar()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL Prop7 1/0 must be NaR\n";
			}
			continue;
		}
		TL r = x.reciprocal();
		// involution: 1/(1/x) == x, exactly, at the bit level
		if (r.reciprocal().raw_bits() != x.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL Prop7 not involutive bits=" << b << '\n';
		}
		if (!comparable(x) || !comparable(r)) continue;
		double want = 1.0 / double(x);
		double rel  = std::fabs((double(r) - want) / want);
		// the encoding is exact; only the double round-trip introduces error
		if (rel > 1e-12) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL Prop7 bits=" << b << " x=" << double(x)
				          << " 1/x=" << double(r) << " want=" << want << " rel=" << rel << '\n';
			}
		}
		// sign is preserved by inversion, unlike negation
		if (x.sign() != r.sign()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL Prop7 sign changed bits=" << b << '\n';
		}
	}
	return nrOfFailedTests;
}

// Proposition 4: two's complement integer ordering equals value ordering.
template<unsigned nbits, unsigned rbits>
int VerifyOrdering(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	double prev = 0.0;
	bool have_prev = false;
	// walk the signed integer range in increasing order, skipping NaR
	for (int64_t i = -(1ll << (nbits - 1)); i < (1ll << (nbits - 1)); ++i) {
		TL x; x.setbits(static_cast<uint64_t>(i) & ((1ull << nbits) - 1));
		if (x.isnar()) continue;
		double d = double(x);
		if (!std::isfinite(d)) { have_prev = false; continue; }
		if (have_prev && !(d > prev)) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL Prop4 ordering at i=" << i << " prev=" << prev << " cur=" << d << '\n';
			}
		}
		prev = d; have_prev = true;
	}
	return nrOfFailedTests;
}

// Conversion round-trip: every representable value must survive
// takum_log -> double -> takum_log unchanged.
template<unsigned nbits, unsigned rbits>
int VerifyRoundTrip(bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits>;
	int nrOfFailedTests = 0;
	for (uint64_t b = 0; b < (1ull << nbits); ++b) {
		TL x; x.setbits(b);
		if (!comparable(x)) continue;
		TL back{ double(x) };
		if (back.raw_bits() != x.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL round-trip bits=" << b << " value=" << double(x)
				          << " came back as bits=" << back.raw_bits() << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// scale() must follow the library-wide base-2 convention: the integer
// power-of-two exponent of the value actually stored.  takum_log's characteristic
// is in units of sqrt(e), so scale() has to convert; std::ilogb gives the exact
// reference exponent.
//
// stride > 1 samples instead of enumerating, so that the widest configurations,
// whose encoding space is far too large to walk, are still covered.  It is
// deliberately coprime with the encoding structure so the sample sweeps every DR.
template<unsigned nbits, unsigned rbits, typename bt = std::uint8_t>
int VerifyScaleConvention(bool reportTestCases, uint64_t stride = 1) {
	using TL = sw::universal::takum_log<nbits, rbits, bt>;
	int nrOfFailedTests = 0;
	const uint64_t limit = (nbits >= 64) ? ~0ull : (1ull << nbits);
	for (uint64_t b = 0; b < limit && b + stride > b; b += stride) {
		TL x; x.setbits(b);
		if (!comparable(x)) continue;
		int64_t want = std::ilogb(double(x));
		if (x.scale() != want) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL scale bits=" << b << " value=" << double(x)
				          << " scale()=" << x.scale() << " ilogb=" << want << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// scale() where a double cannot follow.
//
// VerifyScaleConvention checks against std::ilogb(double(x)), so it can only see
// encodings whose magnitude a double can hold.  At rbits = 5 the characteristic
// reaches ~2^32 and about a third of all encodings overflow a double, so that
// entire region went unchecked -- which is how scale() came to narrow a ~3.1e9
// result to int.  That is undefined behaviour, and it silently returned INT_MIN
// for maxpos and INT_MAX for minpos, an inverted range, without any suite noticing.
//
// Check the properties that survive without a double oracle: scale() is
// non-decreasing in the magnitude, and it separates the extremes in the right
// order.  Either one alone catches the inversion.
template<unsigned nbits, unsigned rbits, typename bt = std::uint8_t>
int VerifyWideScale(bool reportTestCases, uint64_t stride = 1) {
	using TL = sw::universal::takum_log<nbits, rbits, bt>;
	int nrOfFailedTests = 0;

	TL lo(sw::universal::SpecificValue::minpos), hi(sw::universal::SpecificValue::maxpos);
	if (!(lo.scale() < hi.scale())) {
		++nrOfFailedTests;
		if (reportTestCases) {
			std::cout << "FAIL scale(minpos)=" << lo.scale()
			          << " must be below scale(maxpos)=" << hi.scale() << '\n';
		}
	}

	// Positive magnitudes only, so increasing bits means increasing value (Prop. 4).
	const uint64_t limit = 1ull << (nbits - 1);
	int64_t prev = 0;
	bool have_prev = false;
	for (uint64_t b = 1; b < limit; b += stride) {
		TL x; x.setbits(b);
		if (x.iszero() || x.isnar()) continue;
		int64_t s = x.scale();
		if (have_prev && s < prev) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL scale not monotonic at bits=" << b
				          << ": " << s << " after " << prev << '\n';
			}
		}
		prev = s; have_prev = true;
	}
	return nrOfFailedTests;
}

// The two variants must agree bit-for-bit on the shared encoding: same special
// values, same field geometry, same ordering structure -- and disagree on value.
template<unsigned nbits, unsigned rbits>
int VerifySharedEncoding(bool reportTestCases) {
	using TLIN = sw::universal::takum<nbits, rbits>;
	using TLOG = sw::universal::takum_log<nbits, rbits>;
	using sw::universal::SpecificValue;
	int nrOfFailedTests = 0;

	auto same_bits = [&](const char* what, uint64_t a, uint64_t b) {
		if (a != b) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL shared encoding: " << what << " differs\n";
		}
	};
	same_bits("zero",   TLIN(SpecificValue::zero).raw_bits(),   TLOG(SpecificValue::zero).raw_bits());
	same_bits("nar",    TLIN(SpecificValue::nar).raw_bits(),    TLOG(SpecificValue::nar).raw_bits());
	same_bits("minpos", TLIN(SpecificValue::minpos).raw_bits(), TLOG(SpecificValue::minpos).raw_bits());
	same_bits("maxpos", TLIN(SpecificValue::maxpos).raw_bits(), TLOG(SpecificValue::maxpos).raw_bits());
	same_bits("minneg", TLIN(SpecificValue::minneg).raw_bits(), TLOG(SpecificValue::minneg).raw_bits());
	same_bits("maxneg", TLIN(SpecificValue::maxneg).raw_bits(), TLOG(SpecificValue::maxneg).raw_bits());

	// same characteristic decode for every bit pattern -- it comes from one codec
	for (uint64_t b = 0; b < (1ull << nbits); ++b) {
		TLIN a; a.setbits(b);
		TLOG c; c.setbits(b);
		if (a.characteristic() != c.characteristic()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL shared codec: characteristic differs at bits=" << b << '\n';
			break;
		}
		if (a.dr_field() != c.dr_field()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL shared codec: dr_field differs at bits=" << b << '\n';
			break;
		}
	}

	// ...and the value maps must genuinely differ wherever the characteristic is
	// non-zero: 2^c versus sqrt(e)^c.  At c == 0 with m == 0 both give 1.0, and
	// agreeing there is correct rather than a defect, so seek a pattern with c != 0.
	{
		bool found_divergence = false, found_c_nonzero = false;
		for (uint64_t b = 1; b < (1ull << (nbits - 1)); ++b) {
			TLIN a; a.setbits(b);
			TLOG c; c.setbits(b);
			if (a.characteristic() == 0) continue;
			found_c_nonzero = true;
			double da = double(a), dc = double(c);
			if (std::isfinite(da) && std::isfinite(dc) && da != dc) { found_divergence = true; break; }
		}
		if (!found_c_nonzero || !found_divergence) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL the two value maps never diverged\n";
		}
	}
	return nrOfFailedTests;
}

} // anonymous namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
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

	std::string test_suite  = "takum_log (logarithmic takum) API verification";
	std::string test_tag    = "api";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// ----------------------------------------------------------------------------
	// Compile-time: takum_log shares the codec with takum and re-exports the same
	// geometry, so generic code can treat the encoding uniformly.
	// ----------------------------------------------------------------------------
	{
		using TL16 = takum_log<16, 3, std::uint8_t>;
		using T16  = takum<16, 3, std::uint8_t>;
		static_assert(std::is_same_v<TL16::Codec, T16::Codec>, "both variants share one codec");
		static_assert(TL16::nbits == 16u && TL16::rbits == 3u, "template parameters");
		static_assert(TL16::overhead == T16::overhead,             "shared overhead");
		static_assert(TL16::maxCharBits == T16::maxCharBits,       "shared maxCharBits");
		static_assert(TL16::max_characteristic() == T16::max_characteristic(), "shared characteristic range");
		static_assert(TL16::min_characteristic() == T16::min_characteristic(), "shared characteristic range");

		// the traits separate them despite the shared encoding
		static_assert(is_takum_log<TL16>,  "takum_log satisfies is_takum_log");
		static_assert(!is_takum<TL16>,     "takum_log is NOT a linear takum");
		static_assert(is_takum<T16>,       "takum satisfies is_takum");
		static_assert(!is_takum_log<T16>,  "takum is NOT a logarithmic takum");
		static_assert(is_any_takum<TL16> && is_any_takum<T16>, "both satisfy is_any_takum");
	}

	// ----------------------------------------------------------------------------
	// Special values and the value base
	// ----------------------------------------------------------------------------
	{
		using TL = takum_log<32, 3>;
		TL z(SpecificValue::zero), n(SpecificValue::nar);
		if (!z.iszero()) { ++nrOfFailedTestCases; std::cout << "FAIL zero\n"; }
		if (!n.isnar())  { ++nrOfFailedTestCases; std::cout << "FAIL nar\n"; }
		if (double(TL(1.0)) != 1.0) { ++nrOfFailedTestCases; std::cout << "FAIL exact 1.0\n"; }

		// sqrt(e) is the value base, so it must land exactly on c=1, m=0
		TL base(TL::value_base);
		if (base.characteristic() != 1) {
			++nrOfFailedTestCases;
			std::cout << "FAIL value_base should encode as characteristic 1, got " << base.characteristic() << '\n';
		}
		// scale() follows the library-wide base-2 convention even though the value
		// base is sqrt(e).  Note that a power of two is generally NOT representable
		// here, so the contract is about the stored value: scale() == floor(log2|v|),
		// which std::ilogb computes exactly.
		if (TL(1.0).scale() != 0) {
			++nrOfFailedTestCases;
			std::cout << "FAIL scale(1.0) should be 0, got " << TL(1.0).scale() << '\n';
		}
	}

	// ----------------------------------------------------------------------------
	// Dynamic range: the logarithmic variant trades range for uniform precision.
	// takum<32,3> spans ~1e+-77 (base 2), takum_log<32,3> ~1e+-55 (base sqrt(e)).
	// ----------------------------------------------------------------------------
	{
		double lin_max = double(takum<32, 3>(SpecificValue::maxpos));
		double log_max = double(takum_log<32, 3>(SpecificValue::maxpos));
		if (!(log_max < lin_max)) {
			++nrOfFailedTestCases;
			std::cout << "FAIL expected the logarithmic range to be narrower: log=" << log_max
			          << " linear=" << lin_max << '\n';
		}
		if (!(log_max > 1e50 && log_max < 1e60)) {
			++nrOfFailedTestCases;
			std::cout << "FAIL takum_log<32,3> maxpos out of expected 1e50..1e60 band: " << log_max << '\n';
		}
	}

	// ----------------------------------------------------------------------------
	// numeric_limits
	// ----------------------------------------------------------------------------
	{
		using TL  = takum_log<32, 3>;
		using lim = std::numeric_limits<TL>;
		if (!lim::is_specialized)  { ++nrOfFailedTestCases; std::cout << "FAIL numeric_limits not specialized\n"; }
		if (lim::has_infinity)     { ++nrOfFailedTestCases; std::cout << "FAIL takum_log has no infinity\n"; }
		if (!lim::has_quiet_NaN)   { ++nrOfFailedTestCases; std::cout << "FAIL NaR serves as quiet NaN\n"; }
		if (lim::radix != 2) {
			++nrOfFailedTestCases;
			std::cout << "FAIL radix is the integer representation radix\n";
		}
		if (!(double(lim::min()) > 0.0 && double(lim::max()) > double(lim::min()))) {
			++nrOfFailedTestCases; std::cout << "FAIL min/max ordering\n";
		}
		if (!(double(lim::lowest()) < 0.0)) { ++nrOfFailedTestCases; std::cout << "FAIL lowest should be negative\n"; }
	}

#if MANUAL_TESTING

	// Scratch space for investigating a single configuration.
	nrOfFailedTestCases += ReportTestResult(
		VerifyInversion<12, 3>(true), "takum_log<12,3>", "inversion (Prop. 7)");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures while iterating manually
#else

	// ----------------------------------------------------------------------------
	// Level 1: the defining structural properties, exhaustive over the two narrow
	// configurations.  takum_log<12,3> is the narrowest layout that still has a
	// trailing field; <16,3> is the spec regime at a practical width.
	// ----------------------------------------------------------------------------
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifyNegation<12, 3>(reportTestCases), "takum_log<12,3>", "negation (Prop. 6)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInversion<12, 3>(reportTestCases), "takum_log<12,3>", "inversion (Prop. 7)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyOrdering<12, 3>(reportTestCases), "takum_log<12,3>", "ordering (Prop. 4)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<12, 3>(reportTestCases), "takum_log<12,3>", "double round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyScaleConvention<12, 3>(reportTestCases), "takum_log<12,3>", "scale() base-2 convention");
	// rbits=5 puts a third of the encodings out of a double's reach, so scale() there
	// is only checkable structurally.  Cheap, and it is the configuration that broke.
	nrOfFailedTestCases += ReportTestResult(
		VerifyWideScale<16, 5>(reportTestCases), "takum_log<16,5>", "scale() beyond double range");
	nrOfFailedTestCases += ReportTestResult(
		VerifyWideScale<16, 4>(reportTestCases), "takum_log<16,4>", "scale() beyond double range");
	nrOfFailedTestCases += ReportTestResult(
		VerifySharedEncoding<12, 3>(reportTestCases), "takum_log<12,3>", "shared codec with takum<>");
#endif

	// ----------------------------------------------------------------------------
	// Level 2: the same properties at 16 bits, plus a narrower regime field.
	// ----------------------------------------------------------------------------
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyNegation<16, 3>(reportTestCases), "takum_log<16,3>", "negation (Prop. 6)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyInversion<16, 3>(reportTestCases), "takum_log<16,3>", "inversion (Prop. 7)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyOrdering<16, 3>(reportTestCases), "takum_log<16,3>", "ordering (Prop. 4)");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<16, 3>(reportTestCases), "takum_log<16,3>", "double round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTrip<16, 1>(reportTestCases), "takum_log<16,1>", "double round-trip");
	nrOfFailedTestCases += ReportTestResult(
		VerifyScaleConvention<16, 3>(reportTestCases), "takum_log<16,3>", "scale() base-2 convention");
	nrOfFailedTestCases += ReportTestResult(
		VerifySharedEncoding<16, 3>(reportTestCases), "takum_log<16,3>", "shared codec with takum<>");
#endif

	// ----------------------------------------------------------------------------
	// Level 3: 32 bits.  Too wide to enumerate, so the encoding space is sampled
	// with a stride coprime to the field structure, which sweeps every DR.
	// ----------------------------------------------------------------------------
#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyScaleConvention<32, 3>(reportTestCases, 5749ull), "takum_log<32,3>", "scale() base-2 convention");
#endif

	// ----------------------------------------------------------------------------
	// Level 4: 64 bits, where the trailing field runs past a double's 53 significand
	// bits and scale() has the least headroom.  64-bit limbs, so the sample does not
	// pay for byte-at-a-time block access.
	// ----------------------------------------------------------------------------
#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
		VerifyScaleConvention<64, 3, std::uint64_t>(reportTestCases, 0x2AAAAAAAABull),
		"takum_log<64,3>", "scale() base-2 convention");
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
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
