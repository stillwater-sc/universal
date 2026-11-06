// efloat_bridge.cpp: test suite runner for the posit to and from efloat conversions of the posit math library
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <random>
#include <universal/number/posit/posit.hpp>
#include <universal/number/efloat/efloat.hpp>   // ldexp
#include <universal/verification/test_suite.hpp>

// posit_limbs::to_efloat is exact and posit_limbs::from_efloat rounds once, ties to even (#1855). The
// round trip must return every posit. Between two neighbouring posits a and b, the tie point, the
// value of the (nbits+1)-bit pattern 2a + 1, must round to the even one of the two; nudged by a bit
// far below the posit, past the encoder's window of the significand, it must round toward that
// side. Far past maxpos and below minpos the posit saturates.

namespace {

	using namespace sw::universal;

	template<typename Posit>
	Posit Make(std::uint64_t encoding) {
		Posit p;
		p.setbits(encoding);
		return p;
	}

	template<typename Posit>
	Posit RandomPosit(std::mt19937_64& rng) {
		using bt = typename Posit::BlockType;
		auto bits = Posit().bits();
		for (unsigned i = 0; i < bits.nrBlocks; ++i) bits.setblock(i, static_cast<bt>(rng()));
		Posit p;
		p.setbits(bits);
		return p;
	}

	template<typename Posit>
	int Fail(bool reportTestCases, const char* what, const Posit& a, const Posit& got, const Posit& want) {
		if (reportTestCases) std::cerr << "FAIL " << what << ' ' << to_binary(a) << ": " << to_binary(got) << " want " << to_binary(want) << '\n';
		return 1;
	}

	// the tie point between a positive posit a and its upper neighbour: the value of the (nbits+1)-bit
	// pattern 2a + 1. Where the regime cuts off exponent bits it is not (a + b) / 2: posits round on
	// their encoding.
	template<typename Posit>
	posit_limbs::efloat_for<Posit::nbits> Midpoint(const Posit& a) {
		using namespace posit_limbs;
		using Wider = posit<Posit::nbits + 1u, Posit::es, typename Posit::BlockType>;
		limb_t w[limbs(Posit::nbits + 1u)]{};
		gather(a, w);
		limb_kernels::lshift(w, w, limbs(Posit::nbits + 1u), 1u);
		set(w, 0u);
		Wider m;
		scatter(m, w);
		const auto e = to_efloat(m);   // efloat_for<nbits + 1>: copy its limbs into the narrower type
		efloat_for<Posit::nbits> v(0);
		v.assign_rounded_limbs(false, e.limb_data(), e.limb_count(), e.scale(), false, static_cast<unsigned>(2u * e.limb_count()));
		return v;
	}

	// a and its upper neighbour b, both positive and finite: the tie point and the tie point nudged
	// either way by 2^-3000 relative to it
	template<typename Posit>
	int VerifyNeighbours(const Posit& a, bool reportTestCases) {
		using namespace posit_limbs;
		Posit b(a);
		++b;
		if (a.isnar() || a.iszero() || a.isneg() || b.isnar() || b.isneg()) return 0;
		int fails = 0;
		auto mid = Midpoint(a);
		mid.set_precision(4000);   // room for a nudge far below any posit here
		const Posit even = (a.bits().test(0)) ? b : a;
		Posit p;
		if (from_efloat(p, mid) != even) fails += Fail(reportTestCases, "midpoint", a, p, even);
		const auto nudge = ldexp(decltype(mid)(1), static_cast<int>(mid.scale()) - 3000);
		if (from_efloat(p, mid + nudge) != b) fails += Fail(reportTestCases, "midpoint + nudge", a, p, b);
		if (from_efloat(p, mid - nudge) != a) fails += Fail(reportTestCases, "midpoint - nudge", a, p, a);
		if (from_efloat(p, -(mid + nudge)) != -b) fails += Fail(reportTestCases, "-(midpoint + nudge)", a, p, -b);
		return fails;
	}

	template<typename Posit>
	int VerifyRoundTrip(const Posit& a, bool reportTestCases) {
		Posit p;
		posit_limbs::from_efloat(p, posit_limbs::to_efloat(a));
		return (p == a || (p.isnar() && a.isnar())) ? 0 : Fail(reportTestCases, "round trip", a, p, a);
	}

	template<typename Posit>
	int VerifyExhaustive(bool reportTestCases) {
		int fails = 0;
		for (std::uint64_t i = 0; i < (1ull << Posit::nbits); ++i) {
			const Posit a = Make<Posit>(i);
			fails += VerifyRoundTrip(a, reportTestCases);
			fails += VerifyNeighbours(a, reportTestCases);
		}
		return fails;
	}

	template<typename Posit>
	int VerifyRandom(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		std::mt19937_64 rng(seed);
		int fails = 0;
		for (unsigned s = 0; s < nrSamples; ++s) {
			const Posit a = RandomPosit<Posit>(rng);
			fails += VerifyRoundTrip(a, reportTestCases);
			fails += VerifyNeighbours(a.isneg() ? -a : a, reportTestCases);
		}
		return fails;
	}

	// far past maxpos and below minpos, in efloat's range: the posit saturates, keeping the sign
	template<typename Posit>
	int VerifySaturation(bool reportTestCases) {
		using E = posit_limbs::efloat_for<Posit::nbits>;
		const Posit maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
		int fails = 0;
		Posit p;
		for (int e : { 100000, 1 << 20, 1 << 30 }) {
			if (posit_limbs::from_efloat(p, ldexp(E(1), e)) != maxpos) fails += Fail(reportTestCases, "huge", maxpos, p, maxpos);
			if (posit_limbs::from_efloat(p, ldexp(E(-3), e)) != -maxpos) fails += Fail(reportTestCases, "-huge", maxpos, p, -maxpos);
			if (posit_limbs::from_efloat(p, ldexp(E(1), -e)) != minpos) fails += Fail(reportTestCases, "tiny", minpos, p, minpos);
			if (posit_limbs::from_efloat(p, ldexp(E(-3), -e)) != -minpos) fails += Fail(reportTestCases, "-tiny", minpos, p, -minpos);
		}
		E nan(0);
		nan.setnan();
		if (!posit_limbs::from_efloat(p, nan).isnar()) fails += Fail(reportTestCases, "NaN", p, p, Posit(SpecificValue::nar));
		return fails;
	}

}  // namespace

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

	std::string test_suite  = "posit to and from efloat";
	std::string test_tag    = "efloat bridge";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Posit = posit<16, 1>;
		const Posit a(1.5);
		std::cout << to_binary(a) << " -> " << to_binary(posit_limbs::from_efloat(*new Posit, posit_limbs::to_efloat(a))) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 3, 1>>(reportTestCases), "posit< 3,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 5, 2>>(reportTestCases), "posit< 5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 8, 0>>(reportTestCases), "posit< 8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 8, 2>>(reportTestCases), "posit< 8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<12, 1>>(reportTestCases), "posit<12,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<32, 2>>(5000, 1, reportTestCases), "posit<32,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<64, 3>>(5000, 2, reportTestCases), "posit<64,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult((VerifyRandom<posit<64, 3, std::uint64_t>>(2000, 3, reportTestCases)), "posit<64,3,uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<128, 4>>(2000, 4, reportTestCases), "posit<128,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<256, 5>>(1000, 5, reportTestCases), "posit<256,5>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturation<posit<16, 1>>(reportTestCases), "posit<16,1> saturation", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturation<posit<64, 3>>(reportTestCases), "posit<64,3> saturation", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySaturation<posit<256, 5>>(reportTestCases), "posit<256,5> saturation", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<16, 1>>(reportTestCases), "posit<16,1>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<128, 4>>(100000, 6, reportTestCases), "posit<128,4>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<20, 2>>(reportTestCases), "posit<20,2>", test_tag);
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
