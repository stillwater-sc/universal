// cordic_datapath.cpp: lns add/sub on a fixed-point hyperbolic CORDIC datapath (#2046)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// CORDICDatapathAddSub computes the Gauss corrections sb_add(d) = log2(1 + 2^d) and sb_sub(d) = log2(1 - 2^d)
// with the fixpnt CORDIC engine. These tests measure the log-domain error of both corrections at every
// d = -k 2^-rbits an lns difference can take, down past the cutoff where the policy returns 0, against a
// long double reference, in units of the lns encoding (2^-rbits); check that the iteration and guard knobs
// trade accuracy as a hardware unit would, and that every configuration stays within the error bound its
// lns_addsub_log_error_bound trait advertises; and run the policy through lns operator+ and operator- by
// specializing lns_addsub_traits.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/lns_addsub_cordic_datapath.hpp>
#include <universal/verification/lns_test_suite.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

// one lns type routed through the policy: operator+ and operator- of lns<8,4,uint16_t> use it in this program
using LNS8_4_cordic = sw::universal::lns<8, 4, std::uint16_t>;
namespace sw { namespace universal {
	template<>
	struct lns_addsub_traits<LNS8_4_cordic> {
		using type = CORDICDatapathAddSub<LNS8_4_cordic>;
	};
}}

namespace sw { namespace universal {

	struct CorrectionError {
		double add  = 0.0;  // sb_add, in lns units
		double sub  = 0.0;  // sb_sub for d <= -1
		double band = 0.0;  // sb_sub for d in (-1, 0), the cancellation band
	};

	// the larger of a running worst error and a sample, with a NaN sample taken as an infinite error, so a
	// NaN correction fails every bound instead of vanishing from the maximum
	inline double worse(double worst, double sample) {
		if (sample != sample) return std::numeric_limits<double>::infinity();
		return (sample > worst) ? sample : worst;
	}

	// the worst errors of sb_add and sb_sub over d = -k 2^-rbits, k = 1 .. (dmax + 2) 2^rbits, every
	// stride-th k: dmax is the default policy's cutoff, below which it returns 0, so the sweep also
	// measures the corrections the policy drops
	template<typename Policy, typename LnsType>
	CorrectionError MeasureCorrections(long long stride = 1) {
		constexpr int r = static_cast<int>(LnsType::rbits);
		const long double ulp = std::ldexp(1.0L, -r);
		const long double ln2 = 0.693147180559945309417232121458176568L;
		const long long K = static_cast<long long>(CORDICDatapathAddSub<LnsType>::dmax + 2u) << r;
		CorrectionError e;
		for (long long k = 1; k <= K; k += stride) {
			const double d = -std::ldexp(static_cast<double>(k), -r);
			const long double ld = d;
			const long double ra = std::log2(1.0L + std::exp2(ld));
			const long double rs = std::log2(-std::expm1(ld * ln2));    // 1 - 2^d without cancellation
			const double ea = static_cast<double>(std::fabs(Policy::sb_add(d) - ra) / ulp);
			const double es = static_cast<double>(std::fabs(Policy::sb_sub(d) - rs) / ulp);
			e.add = worse(e.add, ea);
			if (d > -1.0) e.band = worse(e.band, es);
			else          e.sub  = worse(e.sub, es);
		}
		return e;
	}

	template<typename Policy, typename LnsType>
	int VerifyCorrections(bool reportTestCases, const char* label, double bound, long long stride = 1) {
		const CorrectionError e = MeasureCorrections<Policy, LnsType>(stride);
		int nrOfFailedTestCases = 0;
		if (e.add > bound)  ++nrOfFailedTestCases;
		if (e.sub > bound)  ++nrOfFailedTestCases;
		if (e.band > bound) ++nrOfFailedTestCases;
		if (reportTestCases || nrOfFailedTestCases > 0) {
			std::cout << label << ": worst error in lns units  sb_add " << e.add << "  sb_sub " << e.sub
			          << "  sb_sub band " << e.band << "  (bound " << bound << ")\n";
		}
		return nrOfFailedTestCases;
	}

	// the worst error of a configuration, in lns units, checked against the log-domain bound of its
	// lns_addsub_log_error_bound trait; a nonfinite error fails
	template<typename Policy, typename LnsType>
	double WorstWithinBound(int& nrOfFailedTestCases, bool reportTestCases, const char* label, long long stride = 1) {
		const CorrectionError e = MeasureCorrections<Policy, LnsType>(stride);
		const double worst = worse(worse(e.add, e.sub), e.band);
		const double bound = lns_addsub_log_error_bound_v<Policy> * std::ldexp(1.0, static_cast<int>(LnsType::rbits));
		if (!(worst <= bound)) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cout << label << ": worst error " << worst << " lns units, trait bound " << bound << '\n';
		}
		return worst;
	}

	// the knobs: fewer iterations or fewer guard bits must not be more accurate, the default must be the
	// most accurate of the configurations measured, and each must stay within its trait's bound
	template<typename LnsType>
	int VerifyKnobs(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		const double it6   = WorstWithinBound<CORDICDatapathAddSub<LnsType, 6>, LnsType>(nrOfFailedTestCases, reportTestCases, "iterations 6");
		const double it10  = WorstWithinBound<CORDICDatapathAddSub<LnsType, 10>, LnsType>(nrOfFailedTestCases, reportTestCases, "iterations 10");
		const double it14  = WorstWithinBound<CORDICDatapathAddSub<LnsType, 14>, LnsType>(nrOfFailedTestCases, reportTestCases, "iterations 14");
		const double g0    = WorstWithinBound<CORDICDatapathAddSub<LnsType, 0, 0>, LnsType>(nrOfFailedTestCases, reportTestCases, "guard 0");
		const double g2    = WorstWithinBound<CORDICDatapathAddSub<LnsType, 0, 2>, LnsType>(nrOfFailedTestCases, reportTestCases, "guard 2");
		const double full  = WorstWithinBound<CORDICDatapathAddSub<LnsType>, LnsType>(nrOfFailedTestCases, reportTestCases, "default");
		if (!(it6 > it10 && it10 > it14 && it14 > full)) ++nrOfFailedTestCases;
		if (!(g0 > g2 && g2 > full)) ++nrOfFailedTestCases;
		if (reportTestCases || nrOfFailedTestCases > 0) {
			std::cout << "knobs, worst error in lns units: iterations 6 " << it6 << ", 10 " << it10 << ", 14 " << it14
			          << "; guard 0 " << g0 << ", 2 " << g2 << "; default " << full << '\n';
		}
		return nrOfFailedTestCases;
	}

	// every sum and difference of an exhaustive lns, by the policy and by DirectEvaluationAddSub, must agree
	// to within one encoding step: the corrections differ by a few hundredths of a unit, so the two can only
	// round to neighbours
	template<typename LnsType>
	int VerifyAgainstDirect(bool reportTestCases) {
		using Policy = CORDICDatapathAddSub<LnsType>;
		using Direct = DirectEvaluationAddSub<LnsType>;
		constexpr std::size_t NR_ENCODINGS = (std::size_t(1) << LnsType::nbits);
		const double step = std::exp2(std::ldexp(1.0, -static_cast<int>(LnsType::rbits))) - 1.0;
		int nrOfFailedTestCases = 0;
		for (std::size_t i = 0; i < NR_ENCODINGS; ++i) {
			LnsType a; a.setbits(i);
			if (a.isnan()) continue;
			for (std::size_t j = 0; j < NR_ENCODINGS; ++j) {
				LnsType b; b.setbits(j);
				if (b.isnan()) continue;
				for (int op = 0; op < 2; ++op) {
					LnsType c = a, cref = a;
					if (op == 0) { Policy::add_assign(c, b); Direct::add_assign(cref, b); }
					else         { Policy::sub_assign(c, b); Direct::sub_assign(cref, b); }
					if (c == cref || (c.isnan() && cref.isnan())) continue;
					const double v = double(c), vref = double(cref);
					const double mag = std::fmax(std::fabs(vref), double(LnsType(SpecificValue::minpos)));
					if (std::fabs(v - vref) <= step * mag * 1.0001) continue;
					++nrOfFailedTestCases;
					if (reportTestCases) {
						std::cout << "FAIL " << double(a) << (op == 0 ? " + " : " - ") << double(b) << " = " << v
						          << ", DirectEvaluation " << vref << '\n';
					}
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// The saturating subtractor (#2080).  one_minus(v) = 1 - v must stay a positive bus value when a
	// truncated exp unit gives v >= 1, or log2 of it is -inf or NaN.  The unit checks pin the
	// branch directly; the reach check proves a real configuration takes it, which the error-bound
	// sweep alone did not: at 4 iterations no d in (-1, 0) reaches it, at 12 a handful do.
	template<typename LnsType, unsigned Iterations>
	int VerifySaturatingSubtractor(bool reportTestCases) {
		using Policy = CORDICDatapathAddSub<LnsType, Iterations>;
		using W = typename Policy::Wide;
		int nrOfFailedTestCases = 0;
		W minpos; minpos.setbits(1u);
		W one(1), above(1); above += minpos;
		if (!(Policy::one_minus(one) == minpos))   ++nrOfFailedTestCases;   // 1 - 1 = 0 saturates
		if (!(Policy::one_minus(above) == minpos)) ++nrOfFailedTestCases;   // 1 - (1 + ulp) < 0 saturates
		if (!(Policy::one_minus(W(0.25)) == W(0.75))) ++nrOfFailedTestCases; // an ordinary difference is exact
		// the band d in (-1, 0) on the lns grid: where 2^d comes out >= 1, sb_sub must stay finite and in bound
		constexpr int r = static_cast<int>(LnsType::rbits);
		const double bound = lns_addsub_log_error_bound_v<Policy> * std::ldexp(1.0, r);
		const long double ulp = std::ldexp(1.0L, -r);
		const long double ln2 = 0.693147180559945309417232121458176568L;
		int reached = 0;
		for (long long k = 1; k < (1ll << r); ++k) {
			const double d = -std::ldexp(static_cast<double>(k), -r);
			if (Policy::template pow2<W>(W(d)) < W(1)) continue;
			++reached;
			const double v = Policy::sb_sub(d);
			const long double ref = std::log2(-std::expm1(static_cast<long double>(d) * ln2));
			const double err = static_cast<double>(std::fabs(v - ref) / ulp);
			if (!std::isfinite(v) || !(err <= bound)) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL saturated sb_sub(" << d << ") = " << v << ", error " << err << " lns units\n";
			}
		}
		if (reached == 0) ++nrOfFailedTestCases;                              // the branch must actually be reached
		if (reportTestCases) std::cout << "saturating subtractor reached at " << reached << " band arguments\n";
		return nrOfFailedTestCases;
	}

	// The datapath and DirectEvaluation round to the same or to neighbouring encodings over sampled sums
	// and differences (#2080).  They are NOT bit-identical, as #2048's commit message said: about 0.8% of
	// lns<16,8> results and 0.4% of lns<24,12> results differ by one encoding, where the two corrections
	// straddle a rounding boundary.  So this pins neighbours, and that differences stay rare.
	template<typename LnsType>
	int VerifyNeighboursOfDirect(bool reportTestCases, int nrSamples, double maxFraction) {
		using Policy = CORDICDatapathAddSub<LnsType>;
		using Direct = DirectEvaluationAddSub<LnsType>;
		const double step = std::exp2(std::ldexp(1.0, -static_cast<int>(LnsType::rbits))) - 1.0;
		std::uint64_t state = 0x2080u;
		auto next = [&state]() { state = state * 6364136223846793005ull + 1442695040888963407ull; return state >> 11; };
		int nrOfFailedTestCases = 0, differ = 0, total = 0;
		for (int i = 0; i < nrSamples; ++i) {
			const double x = (static_cast<double>(next()) / 9007199254740992.0 - 0.5) * 2000.0;
			const double y = (static_cast<double>(next()) / 9007199254740992.0 - 0.5) * 2000.0;
			const LnsType a(x), b(y);
			for (int op = 0; op < 2; ++op) {
				LnsType c = a, cref = a;
				if (op == 0) { Policy::add_assign(c, b); Direct::add_assign(cref, b); }
				else         { Policy::sub_assign(c, b); Direct::sub_assign(cref, b); }
				++total;
				if (c == cref || (c.isnan() && cref.isnan())) continue;
				++differ;
				const double v = double(c), vref = double(cref);
				const double mag = std::fmax(std::fabs(vref), double(LnsType(SpecificValue::minpos)));
				if (std::fabs(v - vref) <= step * mag * 1.0001) continue;
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL " << x << (op == 0 ? " + " : " - ") << y << " = " << v << ", DirectEvaluation " << vref << '\n';
			}
		}
		if (static_cast<double>(differ) > maxFraction * static_cast<double>(total)) ++nrOfFailedTestCases;
		if (reportTestCases) std::cout << "datapath and DirectEvaluation differ in " << differ << " of " << total << " results\n";
		return nrOfFailedTestCases;
	}

	// one iteration below the default: still within its trait's bound, and not more accurate than the default
	template<typename LnsType>
	int VerifyJustBelowDefault(bool reportTestCases) {
		using Default = CORDICDatapathAddSub<LnsType>;
		constexpr unsigned n = Default::template iterations<typename Default::Narrow>();
		using Below = CORDICDatapathAddSub<LnsType, n - 1u>;
		int nrOfFailedTestCases = 0;
		const double below = WorstWithinBound<Below, LnsType>(nrOfFailedTestCases, reportTestCases, "one iteration below the default");
		const double full  = WorstWithinBound<Default, LnsType>(nrOfFailedTestCases, reportTestCases, "default");
		if (!(below >= full)) ++nrOfFailedTestCases;
		if (reportTestCases) std::cout << "iterations " << (n - 1u) << ": " << below << ", default " << n << ": " << full << " lns units\n";
		return nrOfFailedTestCases;
	}

}}  // namespace sw::universal

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
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "lns add/sub on a fixpnt hyperbolic CORDIC datapath (#2046)";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	using LNS8_4   = lns< 8,  4, std::uint8_t>;
	using LNS12_6  = lns<12,  6, std::uint16_t>;
	using LNS16_8  = lns<16,  8, std::uint16_t>;
	using LNS32_24 = lns<32, 24, std::uint32_t>;

#if MANUAL_TESTING

	std::cout << "datapath vs double model, worst error in lns units:\n";
	nrOfFailedTestCases += VerifyCorrections<CORDICDatapathAddSub<LNS16_8>, LNS16_8>(true, "datapath lns<16,8>", 0.1);
	nrOfFailedTestCases += VerifyCorrections<CORDICAddSub<LNS16_8>, LNS16_8>(true, "double model lns<16,8>", 8.0);
	nrOfFailedTestCases += VerifyKnobs<LNS16_8>(true);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

	// the worst errors measured at the default knobs are 0.026 to 0.044 lns units; 0.1 leaves margin and still
	// fails a datapath that loses a guard bit (0.72 units at g = 2)
	constexpr double bound = 0.1;

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCorrections<CORDICDatapathAddSub<LNS8_4>, LNS8_4>(reportTestCases, "lns<8,4>", bound), "lns<8,4>", "corrections");
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCorrections<CORDICDatapathAddSub<LNS12_6>, LNS12_6>(reportTestCases, "lns<12,6>", bound), "lns<12,6>", "corrections");
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCorrections<CORDICDatapathAddSub<LNS16_8>, LNS16_8>(reportTestCases, "lns<16,8>", bound), "lns<16,8>", "corrections");
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCorrections<CORDICDatapathAddSub<LNS32_24>, LNS32_24>(reportTestCases, "lns<32,24>", bound, 4099), "lns<32,24>", "corrections");
	nrOfFailedTestCases += ReportTestResult(VerifyKnobs<LNS16_8>(reportTestCases), "lns<16,8>", "iteration and guard knobs");
	nrOfFailedTestCases += ReportTestResult(VerifyAgainstDirect<LNS8_4>(reportTestCases), "lns<8,4>", "add/sub vs DirectEvaluation");
	nrOfFailedTestCases += ReportTestResult(VerifyAddition<LNS8_4_cordic>(reportTestCases), "lns<8,4,uint16_t>", "addition");
	nrOfFailedTestCases += ReportTestResult(VerifySubtraction<LNS8_4_cordic>(reportTestCases), "lns<8,4,uint16_t>", "subtraction");
	nrOfFailedTestCases += ReportTestResult((VerifySaturatingSubtractor<lns<32, 16, std::uint32_t>, 12>(reportTestCases)), "lns<32,16>, 12 iterations", "saturating subtractor");
	nrOfFailedTestCases += ReportTestResult(VerifyNeighboursOfDirect<LNS16_8>(reportTestCases, 4000, 0.02), "lns<16,8>", "neighbours of DirectEvaluation");
	nrOfFailedTestCases += ReportTestResult(VerifyJustBelowDefault<LNS16_8>(reportTestCases), "lns<16,8>", "one iteration below the default");
#endif

#if REGRESSION_LEVEL_2
	using LNS24_12 = lns<24, 12, std::uint32_t>;
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCorrections<CORDICDatapathAddSub<LNS24_12>, LNS24_12>(reportTestCases, "lns<24,12>", bound), "lns<24,12>", "corrections");
	nrOfFailedTestCases += ReportTestResult(VerifyKnobs<LNS12_6>(reportTestCases), "lns<12,6>", "iteration and guard knobs");
	{
		// truncated so far that 2^d rounds to 1 or above in the cancellation band: the subtractor saturates, the
		// correction stays finite, and the error stays within the capped trait bound
		using LNS32_16t = lns<32, 16, std::uint32_t>;
		int failed = 0;
		WorstWithinBound<CORDICDatapathAddSub<LNS32_16t, 4>, LNS32_16t>(failed, reportTestCases, "lns<32,16>, 4 iterations", 31);
		WorstWithinBound<CORDICDatapathAddSub<LNS32_16t, 12>, LNS32_16t>(failed, reportTestCases, "lns<32,16>, 12 iterations", 31);
		nrOfFailedTestCases += ReportTestResult(failed, "lns<32,16>", "truncated units within bound");
	}
	{
		// a denser level-2 sweep of lns<32,16> than level 3's stride 7 reaches at level 1 (#2080)
		using LNS32_16d = lns<32, 16, std::uint32_t>;
		nrOfFailedTestCases += ReportTestResult(
		    VerifyCorrections<CORDICDatapathAddSub<LNS32_16d>, LNS32_16d>(reportTestCases, "lns<32,16>", bound, 61), "lns<32,16>", "corrections, stride 61");
	}
	nrOfFailedTestCases += ReportTestResult(VerifyNeighboursOfDirect<lns<24, 12, std::uint32_t>>(reportTestCases, 4000, 0.02), "lns<24,12>", "neighbours of DirectEvaluation");
	nrOfFailedTestCases += ReportTestResult(VerifyJustBelowDefault<LNS12_6>(reportTestCases), "lns<12,6>", "one iteration below the default");
#endif

#if REGRESSION_LEVEL_3
	using LNS32_16 = lns<32, 16, std::uint32_t>;
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCorrections<CORDICDatapathAddSub<LNS32_16>, LNS32_16>(reportTestCases, "lns<32,16>", bound, 7), "lns<32,16>", "corrections");
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCorrections<CORDICDatapathAddSub<LNS32_24>, LNS32_24>(reportTestCases, "lns<32,24>", bound, 401), "lns<32,24>", "corrections");
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
