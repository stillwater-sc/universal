// addition.cpp: addition and subtraction verification for the logarithmic takum
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Addition and subtraction are the operations with no logarithmic shortcut: the
// multiply and divide covered by multiplication.cpp are exact integer work on l,
// but a sum has to leave the logarithmic domain and come back.  They used to do
// that through a double, and takum_log<64,3> carries 59 fraction bits of l
// against a double's 53, so both operands were quantized before the addition --
// 99.22% of 64-bit sums came back incorrectly rounded (issue #1300).
//
// Wide configurations now evaluate the identity in an extended-precision local
// double-double (takum_log_arithmetic.hpp).  This suite is the evidence, in three
// layers, because a failure in any one of them means something different:
//
//   1. the local transcendentals, checked against dd_cascade's.  If exp or log is
//      wrong, everything above it is wrong for a reason that has nothing to do
//      with takum, and it is worth saying so directly.  This layer caught a real
//      defect: scaling Taylor terms by 1.0/i rather than dividing by i costs 33
//      bits, because the reciprocal of a non-power-of-two is not exact.
//
//   2. correct rounding, against a reference computed in dd_cascade by a
//      DIFFERENT formulation.  The implementation factors out the larger operand
//      and evaluates 2 log(1 +/- e^(d/2)); the reference forms e^(la/2) and
//      e^(lb/2) and adds them outright.  Using the implementation's own identity
//      as its reference would verify the arithmetic and assume the algebra.
//      (The naive form is only usable as a reference, not as an implementation:
//      it overflows for l beyond ~1420, which rbits = 5 reaches easily.)
//
//   3. exact identities and commutativity, which need no reference at all.
//
// Ties are decided rather than waved through, as in the linear takum's suite: the
// format rounds to nearest-even, so at an exact tie the chosen encoding must carry
// an even trailing field.
//
// dd_cascade is a sound reference here: measured against an 80-digit reference its
// exp is accurate to ~103 bits and its log to ~105, where a 64-bit takum_log needs
// about 2^-62 -- roughly 40 bits of margin.  It is only its SPEED that made it
// unusable as the implementation.
#include <universal/utility/directives.hpp>

#include <iostream>
#include <cstdint>
#include <universal/number/takum/takum_log.hpp>
#include <universal/number/takum/takum_log_arithmetic.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

namespace tla = sw::universal::takum_log_arith;

constexpr std::uint64_t last_encoding(unsigned nbits) noexcept { return (~0ull) >> (64u - nbits); }
constexpr std::uint64_t sample_stride(std::uint64_t last, unsigned samples) noexcept {
	const std::uint64_t step = last / samples;
	return (step < 2ull) ? 1ull : (step | 1ull);
}

// ---------------------------------------------------------------------------
// Layer 1: the local transcendentals against dd_cascade's
// ---------------------------------------------------------------------------
int VerifyLocalMath(bool reportTestCases) {
	using sw::universal::dd_cascade;
	int nrOfFailedTests = 0;

	// Both carry ~106 bits, so agreement to 2^-95 leaves room for each one's own
	// last few bits while still being far tighter than the 2^-62 the format needs.
	const double tolerance = 2.5e-29;   // ~2^-95

	auto check = [&](const char* what, double arg, const tla::ddouble& mine, const dd_cascade& theirs) {
		const double m = tla::to_double(mine);
		const double t = double(theirs);
		const double scale = (t != 0.0) ? std::abs(t) : 1.0;
		const double relative = std::abs(m - t) / scale;
		// the doubles agree far too coarsely to see 2^-95; compare limb-wise
		const double lo_diff = std::abs((mine.hi - theirs[0]) + (mine.lo - theirs[1])) / scale;
		if (relative > 1e-14 || lo_diff > tolerance) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cout << "FAIL " << what << " at " << arg
				          << ": local " << m << " vs dd_cascade " << t
				          << " (limb delta " << lo_diff << ")\n";
			}
		}
	};

	const double ys[] = { -1e-15, -1e-9, -1e-4, -0.01, -0.2, -0.5, -1.0, -3.0, -12.0, -40.0, -79.0 };
	for (double y : ys) {
		check("exp",   y, tla::exp_nonpos(tla::make(y)),   exp(dd_cascade(y)));
		check("expm1", y, tla::expm1_nonpos(tla::make(y)), expm1(dd_cascade(y)));
	}
	const double ws[] = { 1e-18, 1e-9, 0.001, 0.05, 0.3, 0.7, 1.0, 1.4, 1.75, 2.0 };
	for (double w : ws) {
		check("log", w, tla::log_pos(tla::make(w)), log(dd_cascade(w)));
	}
	return nrOfFailedTests;
}

// ---------------------------------------------------------------------------
// Layer 2: correct rounding against an independently formulated reference
// ---------------------------------------------------------------------------

// l of a magnitude, exactly, as a dd_cascade
template<typename Codec>
sw::universal::dd_cascade ref_l(std::uint64_t magnitude) {
	using sw::universal::dd_cascade;
	auto d = Codec::decode(magnitude);
	dd_cascade l(static_cast<double>(d.c));
	if (d.p > 0 && d.M_bits != 0) {
		const double hi = static_cast<double>(d.M_bits >> 32);
		const double lo = static_cast<double>(d.M_bits & 0xFFFFFFFFull);
		const double sc = std::ldexp(1.0, -static_cast<int>(d.p));
		l = l + dd_cascade(hi * std::ldexp(1.0, 32) * sc) + dd_cascade(lo * sc);
	}
	return l;
}

template<unsigned nbits, unsigned rbits>
int VerifyCorrectlyRounded(bool subtract, unsigned samples, bool reportTestCases) {
	using sw::universal::dd_cascade;
	using TL    = sw::universal::takum_log<nbits, rbits, std::uint64_t>;
	using Codec = typename TL::Codec;
	int nrOfFailedTests = 0;
	long exercised = 0, ties = 0, tiesTested = 0;
	long naiveRef = 0, factoredRef = 0;

	const std::uint64_t LAST   = last_encoding(nbits);
	const std::uint64_t stride = sample_stride(LAST, samples);
	const std::uint64_t span   = (1ull << (nbits - 1)) - 1;

	for (std::uint64_t i = 0; i <= LAST && i + stride > i; i += stride) {
		TL a; a.setbits(i);
		if (a.isnar() || a.iszero()) continue;
		for (std::uint64_t j = 0; j <= LAST && j + stride > j; j += stride) {
			TL b; b.setbits(j);
			if (b.isnar() || b.iszero()) continue;

			TL got = subtract ? (a - b) : (a + b);
			if (got.isnar() || got.iszero()) continue;
			const std::uint64_t gm = got.magnitude_bits();
			if (gm == span || gm == 1ull) continue;   // saturation is a range decision

			const dd_cascade la = ref_l<Codec>(a.magnitude_bits());
			const dd_cascade lb = ref_l<Codec>(b.magnitude_bits());
			const bool sa = a.sign();
			const bool sb = (b.sign() != subtract);
			const bool aBig = !(la < lb);
			const dd_cascade lBig = aBig ? la : lb;
			const dd_cascade d    = (aBig ? lb : la) - lBig;      // <= 0
			const bool sBig = aBig ? sa : sb;

			// Which reference to use.
			//
			// The naive one -- form e^(la/2) and e^(lb/2) and add -- is the valuable
			// one, because it reaches the answer by different ALGEBRA than the
			// implementation and so tests the identity itself, not merely the
			// arithmetic under it.  But for opposite signs it cancels exactly as
			// badly as any other subtraction: |x+y| / |x| is about |d|/2, so a d of
			// 2^-50 costs 50 of dd_cascade's ~106 bits and the reference stops being
			// a reference.  Adjudicated against an 80-digit computation, the
			// implementation was right and the naive reference wrong in every one of
			// those cases.
			//
			// So below the threshold, fall back to the factored form, which shares
			// the implementation's algebra but not its arithmetic -- still an
			// independent check of everything except the identity, and layer 1 has
			// already pinned the transcendentals it rests on.
			const bool cancels = (sa != sb) && (double(d) > -3.0e-8);   // |d| < ~2^-25
			bool refNeg;
			dd_cascade want;
			if (!cancels) {
				dd_cascade va = exp(la * dd_cascade(0.5));
				dd_cascade vb = exp(lb * dd_cascade(0.5));
				if (sa) va = dd_cascade(0.0) - va;
				if (sb) vb = dd_cascade(0.0) - vb;
				dd_cascade sum = va + vb;
				if (sum == dd_cascade(0.0)) continue;
				refNeg = (sum < dd_cascade(0.0));
				if (refNeg) sum = dd_cascade(0.0) - sum;
				want = log(sum) * dd_cascade(2.0);
				++naiveRef;
			}
			else {
				// 1 - e^(d/2) via expm1, NOT as 1.0 - exp(...).  Writing the
				// subtraction out cancels away precisely the digits that matter here
				// -- the same trap this fallback exists to escape.
				const dd_cascade w = dd_cascade(0.0) - expm1(d * dd_cascade(0.5));
				if (w == dd_cascade(0.0)) continue;             // exact cancellation
				want = lBig + log(w) * dd_cascade(2.0);
				refNeg = sBig;
				++factoredRef;
			}

			if (refNeg != got.sign()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL sign at " << i << ',' << j << '\n';
				continue;
			}

			auto dist = [&](std::uint64_t m) -> dd_cascade {
				dd_cascade d = ref_l<Codec>(m) - want;
				return (d < dd_cascade(0.0)) ? (dd_cascade(0.0) - d) : d;
			};
			const dd_cascade mine = dist(gm);
			++exercised;

			for (int k = -1; k <= 1; k += 2) {
				const std::uint64_t m = (k < 0) ? (gm - 1ull) : (gm + 1ull);
				if (m < 1ull || m > span) continue;
				const dd_cascade dn = dist(m);
				if (dn < mine) {
					++nrOfFailedTests;
					if (reportTestCases) {
						std::cout << "FAIL " << (subtract ? "subtraction" : "addition")
						          << " not correctly rounded at " << i << ',' << j << '\n';
					}
					break;
				}
				if (dn == mine) {
					// exact tie: the format rounds to nearest-even
					++ties;
					const auto dg = Codec::decode(gm);
					const auto dn2 = Codec::decode(m);
					if (dg.p == dn2.p && dg.p > 0) {
						++tiesTested;
						if ((dg.M_bits & 1ull) != 0ull) {
							++nrOfFailedTests;
							if (reportTestCases) std::cout << "FAIL tie went to odd at " << i << ',' << j << '\n';
							break;
						}
					}
				}
			}
		}
	}
	if (exercised == 0) {
		++nrOfFailedTests;
		std::cout << "FAIL correctly-rounded check compared nothing\n";
	}
	if (reportTestCases) {
		std::cout << "      takum_log<" << nbits << ',' << rbits << "> "
		          << (subtract ? "sub" : "add") << ": exercised " << exercised
		          << ", ties " << ties << " (" << tiesTested << " checked)"
		          << ", reference naive/factored " << naiveRef << '/' << factoredRef << "\n";
	}
	return nrOfFailedTests;
}

// ---------------------------------------------------------------------------
// Layer 3: identities, which need no reference
// ---------------------------------------------------------------------------
template<unsigned nbits, unsigned rbits>
int VerifyExactIdentities(unsigned samples, bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	const std::uint64_t LAST   = last_encoding(nbits);
	const std::uint64_t stride = sample_stride(LAST, samples);
	TL zero; zero.setzero();
	TL nar;  nar.setnar();

	auto fail = [&](const char* what, std::uint64_t bits) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL " << what << " at bits=" << bits << '\n';
	};

	for (std::uint64_t i = 0; i <= LAST && i + stride > i; i += stride) {
		TL a; a.setbits(i);
		if (a.isnar()) continue;
		if ((a + zero).raw_bits() != a.raw_bits()) fail("a + 0 != a", i);
		if ((a - zero).raw_bits() != a.raw_bits()) fail("a - 0 != a", i);
		if (!(a - a).iszero())                     fail("a - a != 0", i);
		if (!(a + nar).isnar())                    fail("a + NaR must be NaR", i);
		if (!(a - nar).isnar())                    fail("a - NaR must be NaR", i);
		if (!a.iszero()) {
			if ((zero + a).raw_bits() != a.raw_bits()) fail("0 + a != a", i);
			if (((-a) + a).raw_bits() != 0ull)         fail("-a + a != 0", i);
		}
	}
	return nrOfFailedTests;
}

template<unsigned nbits, unsigned rbits>
int VerifyCommutative(unsigned samples, bool reportTestCases) {
	using TL = sw::universal::takum_log<nbits, rbits, std::uint64_t>;
	int nrOfFailedTests = 0;
	const std::uint64_t LAST   = last_encoding(nbits);
	const std::uint64_t stride = sample_stride(LAST, samples);
	for (std::uint64_t i = 0; i <= LAST && i + stride > i; i += stride) {
		TL a; a.setbits(i);
		if (a.isnar()) continue;
		for (std::uint64_t j = 0; j <= LAST && j + stride > j; j += stride) {
			TL b; b.setbits(j);
			if (b.isnar()) continue;
			if ((a + b).raw_bits() != (b + a).raw_bits()) {
				++nrOfFailedTests;
				if (reportTestCases) std::cout << "FAIL addition not commutative at " << i << ',' << j << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// The reported symptom of #1300 on the logarithmic side.
int VerifyIssue1300Repro(bool reportTestCases) {
	using namespace sw::universal;
	using TL = takum_log<64, 3, std::uint64_t>;
	int nrOfFailedTests = 0;
	TL one(1.0), eps, zero;
	zero.setzero();
	eps.setbits(one.raw_bits() + 1ull);
	if ((eps + zero).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum_log<64,3>: eps + 0 lost the low bit\n";
	}
	if ((eps - zero).raw_bits() != eps.raw_bits()) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << "FAIL takum_log<64,3>: eps - 0 lost the low bit\n";
	}
	return nrOfFailedTests;
}

} // anonymous namespace

#define MANUAL_TESTING 0
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

	std::string test_suite  = "takum_log addition and subtraction verification";
	std::string test_tag    = "addition";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// The reference costs two dd_cascade transcendentals per pair at ~30 us, so the
	// rounding sweeps stay modest; the cheap layers carry the volume.
	[[maybe_unused]] constexpr unsigned rounding_samples    = 90;    // ~8k pairs
	[[maybe_unused]] constexpr unsigned identity_samples    = 2048;
	[[maybe_unused]] constexpr unsigned commutative_samples = 200;

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifyLocalMath(true), "local math", "exp/log vs dd_cascade");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(false, rounding_samples, true), "takum_log<64,3>", "correctly rounded add");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
		VerifyLocalMath(true), "local math", "exp/expm1/log vs dd_cascade");
	nrOfFailedTestCases += ReportTestResult(
		VerifyIssue1300Repro(true), "takum_log<64,3>", "issue 1300 repro");
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<64, 3>(identity_samples, reportTestCases), "takum_log<64,3>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(false, rounding_samples, reportTestCases), "takum_log<64,3>", "correctly rounded add");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 3>(true, rounding_samples, reportTestCases), "takum_log<64,3>", "correctly rounded sub");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<64, 3>(commutative_samples, reportTestCases), "takum_log<64,3>", "commutativity");
	// The narrow configurations keep the double path; verified against the SAME
	// reference, so the boundary the gate draws is measured rather than asserted.
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(false, rounding_samples, reportTestCases), "takum_log<32,3>", "correctly rounded add");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<32, 3>(true, rounding_samples, reportTestCases), "takum_log<32,3>", "correctly rounded sub");
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<32, 3>(identity_samples, reportTestCases), "takum_log<32,3>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<32, 3>(commutative_samples, reportTestCases), "takum_log<32,3>", "commutativity");
#endif

#if REGRESSION_LEVEL_4
	// takum_log<58,3> is the narrowest configuration on the extended path, and
	// takum_log<64,1> the widest fraction the format allows at 64 bits (p = 61),
	// which is what fixes the round-to-odd width at 63.
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<58, 3>(identity_samples, reportTestCases), "takum_log<58,3>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<58, 3>(false, rounding_samples, reportTestCases), "takum_log<58,3>", "correctly rounded add");
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<64, 1>(identity_samples, reportTestCases), "takum_log<64,1>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCorrectlyRounded<64, 1>(false, rounding_samples, reportTestCases), "takum_log<64,1>", "correctly rounded add");
	// rbits = 5 drives the characteristic past 2^31, where the naive reference
	// formulation overflows outright -- which is exactly why the implementation
	// factors the larger operand out.  Covered structurally.
	nrOfFailedTestCases += ReportTestResult(
		VerifyExactIdentities<64, 5>(identity_samples, reportTestCases), "takum_log<64,5>", "exact identities");
	nrOfFailedTestCases += ReportTestResult(
		VerifyCommutative<64, 5>(commutative_samples, reportTestCases), "takum_log<64,5>", "commutativity");
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
