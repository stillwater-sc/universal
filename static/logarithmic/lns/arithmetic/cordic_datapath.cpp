// cordic_datapath.cpp: lns add/sub on a fixed-point hyperbolic CORDIC datapath (#2046)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// CORDICDatapathAddSub computes the Gauss corrections sb_add(d) = log2(1 + 2^d) and sb_sub(d) = log2(1 - 2^d)
// with the fixpnt CORDIC engine. These tests measure the log-domain error of both corrections at every
// d = -k 2^-rbits an lns difference can take, against a long double reference, in units of the lns
// encoding (2^-rbits); check that the iteration and guard knobs trade accuracy as a hardware unit would;
// and run the policy through lns operator+ and operator- by specializing lns_addsub_traits.
#include <universal/utility/directives.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/lns/lns_addsub_cordic_datapath.hpp>
#include <universal/verification/lns_test_suite.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>
#include <cmath>
#include <iostream>

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

	// the worst errors of sb_add and sb_sub over d = -k 2^-rbits, k = 1 .. (rbits + 2) 2^rbits, every
	// stride-th k (stride 1 is every d the policy computes a nonzero correction for)
	template<typename Policy, typename LnsType>
	CorrectionError MeasureCorrections(long long stride = 1) {
		constexpr int r = static_cast<int>(LnsType::rbits);
		const long double ulp = std::ldexp(1.0L, -r);
		const long double ln2 = 0.693147180559945309417232121458176568L;
		const long long K = static_cast<long long>(LnsType::rbits + 2u) << r;
		CorrectionError e;
		for (long long k = 1; k <= K; k += stride) {
			const double d = -std::ldexp(static_cast<double>(k), -r);
			const long double ld = d;
			const long double ra = std::log2(1.0L + std::exp2(ld));
			const long double rs = std::log2(-std::expm1(ld * ln2));    // 1 - 2^d without cancellation
			const double ea = static_cast<double>(std::fabs(Policy::sb_add(d) - ra) / ulp);
			const double es = static_cast<double>(std::fabs(Policy::sb_sub(d) - rs) / ulp);
			if (ea > e.add) e.add = ea;
			if (d > -1.0) { if (es > e.band) e.band = es; }
			else if (es > e.sub) e.sub = es;
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

	// the knobs: fewer iterations or fewer guard bits must not be more accurate, and the default must be the
	// most accurate of the configurations measured
	template<typename LnsType>
	int VerifyKnobs(bool reportTestCases) {
		auto worst = [](const CorrectionError& e) { return std::fmax(e.add, std::fmax(e.sub, e.band)); };
		const double it6   = worst(MeasureCorrections<CORDICDatapathAddSub<LnsType, 6>, LnsType>());
		const double it10  = worst(MeasureCorrections<CORDICDatapathAddSub<LnsType, 10>, LnsType>());
		const double it14  = worst(MeasureCorrections<CORDICDatapathAddSub<LnsType, 14>, LnsType>());
		const double g0    = worst(MeasureCorrections<CORDICDatapathAddSub<LnsType, 0, 0>, LnsType>());
		const double g2    = worst(MeasureCorrections<CORDICDatapathAddSub<LnsType, 0, 2>, LnsType>());
		const double full  = worst(MeasureCorrections<CORDICDatapathAddSub<LnsType>, LnsType>());
		int nrOfFailedTestCases = 0;
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
#endif

#if REGRESSION_LEVEL_2
	using LNS24_12 = lns<24, 12, std::uint32_t>;
	nrOfFailedTestCases += ReportTestResult(
	    VerifyCorrections<CORDICDatapathAddSub<LNS24_12>, LNS24_12>(reportTestCases, "lns<24,12>", bound), "lns<24,12>", "corrections");
	nrOfFailedTestCases += ReportTestResult(VerifyKnobs<LNS12_6>(reportTestCases), "lns<12,6>", "iteration and guard knobs");
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
