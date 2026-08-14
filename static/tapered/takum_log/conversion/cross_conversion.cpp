// cross_conversion.cpp: verification of takum <-> takum_log conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// The two variants share an encoding but not a value map, so converting between
// them is arithmetic, not reinterpretation.  Equating the two maps gives
// l * log2(sqrt(e)) = E + log2(1 + f), which is transcendental in both directions
// and exact in neither: log2(sqrt(e)) is irrational, so no non-trivial encoding
// maps onto another exactly.  Only the accuracy of the rounding is in question.
//
// Measured against a 113-bit __float128 reference, the obvious route --
// takum<n,3>{ double(takum_log<n,3>) } -- is correctly rounded through 48 bits and
// then collapses, because takum<64,3> reaches p = 59 and a double carries 53:
//
//                       naive double        dd_cascade
//     log64 -> lin64    99.14% wrong        correctly rounded
//     lin64 -> log64    99.09% wrong        correctly rounded
//
// That reference needs quadmath and cannot run here.  What this suite pins
// without it:
//
//   - the produced encoding is the NEAREST one to a reference recomputed in
//     dd_cascade.  This is the load-bearing check: it catches the naive path at
//     64 bits, ~48,000 failures per direction, while passing it at 16 and 32
//     bits where the naive path really is correctly rounded.
//   - a round trip through a wider intermediate is the identity
//   - conversion preserves order, so it cannot be scrambling values
//   - zero, NaR and sign survive
//   - saturation goes the right way at the range boundaries
//
// The round trip is a sanity check and NOT a proxy for accuracy, though the first
// draft of this file treated it as one.  A naive double-mediated conversion passed
// it, because a 16- or 32-bit source carries fewer than 53 fraction bits and so
// round-trips through anything at least as wide as a double.  An accuracy check
// has to compare against the answer, not against the input.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <universal/number/takum/takum_cross_conversion.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

// The saturation encoding of a type, for spotting a conversion that ran out of range.
template<typename T>
uint64_t wide_maxpos_magnitude() {
	T mx(sw::universal::SpecificValue::maxpos);
	return mx.magnitude_bits();
}


// Is the produced encoding the nearest one to the mathematically correct answer?
//
// The reference is recomputed here in dd_cascade, independently of the conversion
// under test.  That does not re-verify dd_cascade -- its exp2 and log1p were
// checked against a 113-bit __float128 reference offline, at 1.0e-32 and 7.6e-31
// where 2^-60 is 8.7e-19 -- but it does verify everything built on top of it: the
// exact source reconstruction, the round-to-odd extraction, and encode_fraction's
// rescaling.  Those are where the errors actually were.
//
// A ROUND TRIP IS NOT A SUBSTITUTE.  The first version of this suite round-tripped
// a 16- or 32-bit source through a 64-bit intermediate and asserted the identity;
// a deliberately naive double-mediated conversion passed it, because a source
// carrying 27 fraction bits round-trips through anything wider than a double.
// The check has to compare against the answer, not against the input.
template<typename Source, typename Target>
int VerifyNearest(const char* tag, bool reportTestCases, uint64_t stride) {
	using sw::universal::dd_cascade;
	int nrOfFailedTests = 0;
	constexpr unsigned nbits = Source::nbits;
	const uint64_t span = (1ull << (nbits - 1)) - 1;
	const uint64_t tgt_span = (1ull << (Target::nbits - 1)) - 1;
	const uint64_t tgt_saturation = wide_maxpos_magnitude<Target>();
	long exercised = 0;

	// The quantity each target rounds ON, which is c + M/2^p for both: a logarithmic
	// takum rounds on l, and a linear one on its fraction, which within a DR is
	// linear in the value.  One expression serves both -- an earlier version wrote it
	// as an if constexpr whose branches were identical, which implied a distinction
	// the code did not make.
	//
	// What matters is that this is NOT value distance for a logarithmic target.
	// Judging one that way disagrees with the format's own rounding near a midpoint,
	// because the geometric and arithmetic midpoints of two neighbours differ; the
	// offline reference showed 0.098% "failures" at 16 bits that were purely this
	// metric mismatch.
	auto target_metric = [](const typename Target::Codec::decoded& e) -> dd_cascade {
		return sw::universal::takum_xc::exact_value(e.c, e.M_bits, e.p);
	};

	for (uint64_t b = 1; b < span; b += stride) {
		Source x; x.setbits(b);
		if (x.iszero() || x.isnar()) continue;
		Target y = sw::universal::takum_convert<Target>(x);
		if (y.iszero() || y.isnar()) continue;
		if (y.magnitude_bits() == tgt_saturation) continue;                    // saturated

		// the exact answer, recomputed independently
		auto d = Source::Codec::decode(x.magnitude_bits());
		dd_cascade want;
		if constexpr (sw::universal::is_takum_log<Source>) {
			// log -> linear: log2|x| = l * log2(sqrt(e)); the linear metric is c + f
			dd_cascade l = sw::universal::takum_xc::exact_value(d.c, d.M_bits, d.p);
			dd_cascade log2v = l * sw::universal::takum_xc::log2_of_sqrt_e();
			dd_cascade E = floor(log2v);
			want = E + (exp2(log2v - E) - dd_cascade(1.0));
		}
		else {
			// linear -> log: l = 2 ln(1+f) + 2 E ln2
			dd_cascade f = sw::universal::takum_xc::exact_value(0, d.M_bits, d.p);
			want = (log1p(f) + dd_cascade(static_cast<double>(d.c)) * sw::universal::ddc_ln2)
			     * dd_cascade(2.0);
		}

		auto dist = [&](uint64_t m) -> dd_cascade {
			auto e = Target::Codec::decode(m);
			dd_cascade v = target_metric(e);
			dd_cascade diff = v - want;
			return (diff < dd_cascade(0.0)) ? (dd_cascade(0.0) - diff) : diff;
		};

		const uint64_t gm = y.magnitude_bits();
		dd_cascade mine = dist(gm);
		++exercised;
		for (int64_t o = -1; o <= 1; o += 2) {
			int64_t m = static_cast<int64_t>(gm) + o;
			if (m < 1 || static_cast<uint64_t>(m) > tgt_span) continue;
			if (dist(static_cast<uint64_t>(m)) < mine) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cout << "FAIL " << tag << " not nearest at bits=" << b
					          << " (neighbour " << o << " is closer)\n";
				}
				break;
			}
		}
	}
	if (exercised == 0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << tag << " compared nothing\n";
	}
	return nrOfFailedTests;
}

// Round trip through a wider intermediate must return the original encoding.
//
// Widening first means the intermediate can hold the value to better than the
// source's own resolution, so any loss is the conversion's rather than the
// format's.  A conversion accurate to a double only would fail this as soon as
// the source outruns a double, which is exactly the regime of interest.
template<typename Source, typename Wide>
int VerifyRoundTripThroughWider(const char* tag, bool reportTestCases, uint64_t stride) {
	int nrOfFailedTests = 0;
	constexpr unsigned nbits = Source::nbits;
	const uint64_t span = 1ull << (nbits - 1);
	const uint64_t wide_saturation = wide_maxpos_magnitude<Wide>();
	long exercised = 0;

	for (uint64_t b = 1; b < span; b += stride) {
		Source x; x.setbits(b);
		if (x.iszero() || x.isnar()) continue;
		Wide   w    = sw::universal::takum_convert<Wide>(x);
		if (w.iszero() || w.isnar()) continue;          // underflowed the intermediate
		// A wider takum_log is not a superset of a narrower takum: its base is
		// sqrt(e) rather than 2, so takum_log<64,3> spans about +/-2.4e55 while
		// takum<16,3> already spans +/-5.8e76.  Values past that saturate, and a
		// round trip through a saturated intermediate says nothing about accuracy.
		if (w.magnitude_bits() == wide_saturation) continue;
		Source back = sw::universal::takum_convert<Source>(w);
		++exercised;
		if (back.raw_bits() != x.raw_bits()) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << tag << " round trip bits=" << b
				          << " -> " << back.raw_bits() << '\n';
			}
		}
		// and the sign must be carried through untouched
		if (back.sign() != x.sign()) {
			++nrOfFailedTests;
			if (reportTestCases) std::cout << "FAIL " << tag << " sign lost at bits=" << b << '\n';
		}
	}
	if (exercised == 0) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << tag << " round trip exercised nothing\n";
	}
	return nrOfFailedTests;
}

// Conversion is monotonic: it is a strictly increasing map on the reals, so it
// must not invert the order of two encodings.  Ties are allowed, since a narrower
// target can collapse neighbours.
template<typename Source, typename Target>
int VerifyOrderPreserved(const char* tag, bool reportTestCases, uint64_t stride) {
	int nrOfFailedTests = 0;
	constexpr unsigned nbits = Source::nbits;
	const uint64_t span = 1ull << (nbits - 1);

	bool have_prev = false;
	Target prev{};
	for (uint64_t b = 1; b < span; b += stride) {
		Source x; x.setbits(b);
		if (x.iszero() || x.isnar()) continue;
		Target y = sw::universal::takum_convert<Target>(x);
		if (y.isnar()) continue;
		if (have_prev && y < prev) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << tag << " order inverted at bits=" << b << '\n';
			}
		}
		prev = y; have_prev = true;
	}
	return nrOfFailedTests;
}

// Special values and the range boundaries.
template<typename Source, typename Target>
int VerifySpecials(const char* tag, bool reportTestCases) {
	int nrOfFailedTests = 0;
	auto fail = [&](const char* what) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << tag << ": " << what << '\n';
	};

	Source nar; nar.setnar();
	Source zero; zero.setzero();
	if (!sw::universal::takum_convert<Target>(nar).isnar())   fail("NaR must convert to NaR");
	if (!sw::universal::takum_convert<Target>(zero).iszero()) fail("zero must convert to zero");

	// one converts to one in both directions: the value 1 is c == 0 with an empty
	// trailing field in either map, the one encoding the two variants agree on.
	Source one(1.0);
	Target t_one = sw::universal::takum_convert<Target>(one);
	if (!(std::fabs(double(t_one) - 1.0) < 1e-6)) fail("1.0 must convert to 1.0");

	// a negative value stays negative
	Source neg(-2.0);
	Target t_neg = sw::universal::takum_convert<Target>(neg);
	if (!t_neg.sign()) fail("a negative value must stay negative");

	// saturation direction: maxpos converts to something large and positive or
	// saturates upward, never to zero or a negative
	Source big(sw::universal::SpecificValue::maxpos);
	Target t_big = sw::universal::takum_convert<Target>(big);
	if (t_big.iszero() || t_big.sign()) fail("maxpos must not convert to zero or a negative");
	Source small(sw::universal::SpecificValue::minpos);
	Target t_small = sw::universal::takum_convert<Target>(small);
	if (t_small.sign()) fail("minpos must not convert to a negative");
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

	std::string test_suite  = "takum <-> takum_log cross-conversion verification";
	std::string test_tag    = "cross conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using L16 = takum_log<16, 3, std::uint64_t>;
	using N16 = takum<16, 3, std::uint64_t>;
	using L32 = takum_log<32, 3, std::uint64_t>;
	using N32 = takum<32, 3, std::uint64_t>;
	using L64 = takum_log<64, 3, std::uint64_t>;
	using N64 = takum<64, 3, std::uint64_t>;

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTripThroughWider<L32, N64>("log32->lin64->log32", true, 4093ull),
		"takum_log<32,3>", "round trip through takum<64,3>");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecials<L32, N32>("log32->lin32", reportTestCases), "takum_log<32,3>", "special values");
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecials<N32, L32>("lin32->log32", reportTestCases), "takum<32,3>", "special values");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNearest<L16, N16>("log16->lin16", reportTestCases, 1ull),
		"takum_log<16,3>", "correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNearest<N16, L16>("lin16->log16", reportTestCases, 1ull),
		"takum<16,3>", "correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTripThroughWider<L16, N64>("log16->lin64->log16", reportTestCases, 1ull),
		"takum_log<16,3>", "round trip through takum<64,3>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTripThroughWider<N16, L64>("lin16->log64->lin16", reportTestCases, 1ull),
		"takum<16,3>", "round trip through takum_log<64,3>");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyOrderPreserved<L16, N16>("log16->lin16", reportTestCases, 1ull),
		"takum_log<16,3>", "order preserved");
	nrOfFailedTestCases += ReportTestResult(
		VerifyOrderPreserved<N16, L16>("lin16->log16", reportTestCases, 1ull),
		"takum<16,3>", "order preserved");
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecials<L16, N16>("log16->lin16", reportTestCases), "takum_log<16,3>", "special values");
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecials<N16, L16>("lin16->log16", reportTestCases), "takum<16,3>", "special values");
#endif

	// The width the dd_cascade kernel exists for.  A double-mediated conversion
	// fails this round trip for essentially every input at 64 bits.
#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyNearest<L64, N64>("log64->lin64", reportTestCases, 0xAAAAAAAAAAABull),
		"takum_log<64,3>", "correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNearest<N64, L64>("lin64->log64", reportTestCases, 0xAAAAAAAAAAABull),
		"takum<64,3>", "correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTripThroughWider<L32, N64>("log32->lin64->log32", reportTestCases, 4093ull),
		"takum_log<32,3>", "round trip through takum<64,3>");
	nrOfFailedTestCases += ReportTestResult(
		VerifyRoundTripThroughWider<N32, L64>("lin32->log64->lin32", reportTestCases, 4093ull),
		"takum<32,3>", "round trip through takum_log<64,3>");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
		VerifyNearest<L32, N32>("log32->lin32", reportTestCases, 4093ull),
		"takum_log<32,3>", "correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyNearest<N32, L32>("lin32->log32", reportTestCases, 4093ull),
		"takum<32,3>", "correctly rounded");
	nrOfFailedTestCases += ReportTestResult(
		VerifyOrderPreserved<L32, N32>("log32->lin32", reportTestCases, 4093ull),
		"takum_log<32,3>", "order preserved");
	nrOfFailedTestCases += ReportTestResult(
		VerifyOrderPreserved<N32, L32>("lin32->log32", reportTestCases, 4093ull),
		"takum<32,3>", "order preserved");
	nrOfFailedTestCases += ReportTestResult(
		VerifyOrderPreserved<N64, L64>("lin64->log64", reportTestCases, 0xAAAAAAAAAAABull),
		"takum<64,3>", "order preserved");
	nrOfFailedTestCases += ReportTestResult(
		VerifyOrderPreserved<L64, N64>("log64->lin64", reportTestCases, 0xAAAAAAAAAAABull),
		"takum_log<64,3>", "order preserved");
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecials<L64, N64>("log64->lin64", reportTestCases), "takum_log<64,3>", "special values");
	nrOfFailedTestCases += ReportTestResult(
		VerifySpecials<N64, L64>("lin64->log64", reportTestCases), "takum<64,3>", "special values");
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
