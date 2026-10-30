// elementary.cpp: test suite runner for the correctly rounded posit exponential, logarithm and hypot functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

// The posit functions are efloat's correctly rounded functions, rounded once into the posit by
// posit_limbs::evaluate, which retries at a wider precision when Ziv's test on the posit side fails
// (#1855). The reference evaluates the same efloat function at 4 (fbits + 1) + 64 bits, far wider
// than evaluate's first try, and rounds it with from_efloat (efloat is verified against MPFR, and
// from_efloat against the posit tie points in efloat_bridge.cpp); a reference within 2 ulps of a
// tie point is evaluated again at 4 times the width. Exact results are checked as exact, and the
// retry itself with synthetic functions whose value at W bits is a posit tie point and resolves
// only at 2W.

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

	// the reference: f evaluated at width bits and rounded once, widened while it lies within 2 ulps
	// of a posit tie point
	template<typename Posit, typename Function>
	Posit Reference(Function&& f) {
		using namespace posit_limbs;
		unsigned width = 4u * (Posit::fbits + 1u) + 64u;
		Posit p;
		for (int attempt = 0; attempt < 3; ++attempt, width *= 4u) {
			const auto y = f(width);
			from_efloat(p, y);
			if (y.iszero() || y.isnan() || y.isinf()) return p;
			auto wide(y);
			wide.set_precision(y.get_precision() + 4u);
			const auto two = ldexp(decltype(wide)(1), static_cast<int>(y.scale()) - static_cast<int>(y.get_precision()) + 2);
			Posit below, above;
			from_efloat(below, wide - two);
			from_efloat(above, wide + two);
			if (below == p && above == p) return p;
		}
		return p;   // on a tie point at every width: an exact result, rounded as it is
	}

	template<typename Posit>
	int Fail(bool reportTestCases, const std::string& fn, const Posit& x, const Posit& got, const Posit& want) {
		if (reportTestCases) std::cerr << "FAIL " << fn << '(' << to_binary(x) << ") = " << to_binary(got) << " want " << to_binary(want) << '\n';
		return 1;
	}

	// the unary functions under test, with the efloat function each one rounds
	template<typename Posit>
	int VerifyUnary(const Posit& x, bool reportTestCases) {
		using namespace posit_limbs;
		int fails = 0;
		// past efloat's own range (2^(2^62)) its inf and 0 are an overflow and an underflow: exp, exp2
		// and exp10 are positive, so they saturate at maxpos and minpos, as past the posit's range;
		// expm1 > -1 can only overflow; the logarithms' inf is a pole, NaR
		enum class Saturate { both, overflow, none };
		auto check = [&](const char* fn, const Posit& got, auto&& efn, Saturate rule) {
			Posit want = x.isnar() ? Posit(SpecificValue::nar) : Reference<Posit>([&](unsigned W) { return efn(argument(x, W)); });
			if (!x.isnar() && rule != Saturate::none && want.isnar()) want = Posit(SpecificValue::maxpos);
			if (!x.isnar() && rule == Saturate::both && want.iszero()) want = Posit(SpecificValue::minpos);
			if (got != want) fails += Fail(reportTestCases, fn, x, got, want);
		};
		check("exp",   exp(x),   [](const auto& e) { return exp(e); },   Saturate::both);
		check("exp2",  exp2(x),  [](const auto& e) { return exp2(e); },  Saturate::both);
		check("exp10", exp10(x), [](const auto& e) { return exp10(e); }, Saturate::both);
		check("expm1", expm1(x), [](const auto& e) { return expm1(e); }, Saturate::overflow);
		check("log",   log(x),   [](const auto& e) { return log(e); },   Saturate::none);
		check("log2",  log2(x),  [](const auto& e) { return log2(e); },  Saturate::none);
		check("log10", log10(x), [](const auto& e) { return log10(e); }, Saturate::none);
		check("log1p", log1p(x), [](const auto& e) { return log1p(e); }, Saturate::none);
		return fails;
	}

	template<typename Posit>
	int VerifyBinary(const Posit& x, const Posit& y, bool reportTestCases) {
		using namespace posit_limbs;
		const Posit got = hypot(x, y);
		const Posit want = (x.isnar() || y.isnar()) ? Posit(SpecificValue::nar)
			: Reference<Posit>([&](unsigned W) { return hypot(argument(x, W), argument(y, W)); });
		return (got == want) ? 0 : Fail(reportTestCases, "hypot", x, got, want);
	}

	template<typename Posit>
	int VerifyTernary(const Posit& x, const Posit& y, const Posit& z, bool reportTestCases) {
		using namespace posit_limbs;
		const Posit got = hypot(x, y, z);
		if (x.isnar() || y.isnar() || z.isnar()) return got.isnar() ? 0 : Fail(reportTestCases, "hypot3", x, got, Posit(SpecificValue::nar));
		// the reference sums the exact squares at 8 times the width of evaluate's first try
		const Posit want = Reference<Posit>([&](unsigned W) {
			auto s = efloat_detail::exact_product(to_efloat(x), to_efloat(x));
			s.set_precision(8u * W);
			s = s + efloat_detail::exact_product(to_efloat(y), to_efloat(y));
			s = s + efloat_detail::exact_product(to_efloat(z), to_efloat(z));
			auto r = sqrt(s);
			r.set_precision(W);
			return r;
		});
		return (got == want) ? 0 : Fail(reportTestCases, "hypot3", x, got, want);
	}

	template<typename Posit>
	int VerifyExhaustive(bool reportTestCases) {
		int fails = 0;
		for (std::uint64_t i = 0; i < (1ull << Posit::nbits); ++i) fails += VerifyUnary(Make<Posit>(i), reportTestCases);
		return fails;
	}

	template<typename Posit>
	int VerifyExhaustivePairs(bool reportTestCases) {
		int fails = 0;
		for (std::uint64_t i = 0; i < (1ull << Posit::nbits); ++i) {
			for (std::uint64_t j = 0; j < (1ull << Posit::nbits); ++j) fails += VerifyBinary(Make<Posit>(i), Make<Posit>(j), reportTestCases);
		}
		return fails;
	}

	// random encodings, and random values in [-64, 64], where exp does not saturate and log is
	// near its zero
	template<typename Posit>
	int VerifyRandom(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		std::mt19937_64 rng(seed);
		int fails = 0;
		for (unsigned s = 0; s < nrSamples; ++s) {
			Posit x = RandomPosit<Posit>(rng);
			if (s & 1u) x = Posit(static_cast<double>(static_cast<std::int64_t>(rng() % 128001u) - 64000) / 1000.0) + x / Posit(1u << 20);
			fails += VerifyUnary(x, reportTestCases);
			const Posit y = RandomPosit<Posit>(rng), z = RandomPosit<Posit>(rng);
			fails += VerifyBinary(x, y, reportTestCases);
			fails += VerifyTernary(x, y, z, reportTestCases);
		}
		return fails;
	}

	// exact results, and the special values
	template<typename Posit>
	int VerifyExact(bool reportTestCases) {
		int fails = 0;
		auto check = [&](const char* fn, const Posit& got, const Posit& want) {
			if (got != want) fails += Fail(reportTestCases, fn, got, got, want);
		};
		const Posit zero(0), one(1), nar(SpecificValue::nar), maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
		check("exp(0)", exp(zero), one);
		check("expm1(0)", expm1(zero), zero);
		check("log(1)", log(one), zero);
		check("log1p(0)", log1p(zero), zero);
		check("log(0)", log(zero), nar);
		check("log(-1)", log(-one), nar);
		check("log1p(-1)", log1p(-one), nar);
		check("exp(NaR)", exp(nar), nar);
		check("hypot(NaR, 1)", hypot(nar, one), nar);
		check("exp(maxpos)", exp(maxpos), maxpos);
		check("exp(-maxpos)", exp(-maxpos), minpos);
		check("exp2(-maxpos)", exp2(-maxpos), minpos);
		Posit power(1);
		for (int k = 0; k < 20 && k + 2 < static_cast<int>(Posit::nbits); ++k, power *= Posit(2)) {
			check("exp2(k)", exp2(Posit(k)), power);
			check("log2(2^k)", log2(power), Posit(k));
		}
		Posit ten(1);
		for (int k = 0; k < 6 && (Posit::fbits > 4u * static_cast<unsigned>(k) + 4u); ++k, ten *= Posit(10)) {
			check("exp10(k)", exp10(Posit(k)), ten);
			check("log10(10^k)", log10(ten), Posit(k));
		}
		check("hypot(3, 4)", hypot(Posit(3), Posit(4)), Posit(5));
		check("hypot(-3, 4)", hypot(Posit(-3), Posit(4)), Posit(5));
		check("hypot(x, 0)", hypot(Posit(-0.75), zero), Posit(0.75));
		check("hypot(1, 2, 2)", hypot(one, Posit(2), Posit(2)), Posit(3));
		check("hypot(2, 3, 6)", hypot(Posit(2), Posit(3), Posit(6)), Posit(7));
		return fails;
	}

	// evaluate() when the value at W bits sits on a posit tie point and only a wider evaluation
	// tells the side: a correct posit_limbs::evaluate retries; without the test it would round the
	// tie to even. f(W) is the tie point t plus d 2^(scale(t) - 2 W0), W0 the first width: at W0
	// bits that rounds to t, at 2 W0 it does not. And an exact tie, at every width, rounds to the even neighbour.
	template<typename Posit>
	int VerifyRetry(unsigned nrSamples, std::uint64_t seed, bool reportTestCases) {
		using namespace posit_limbs;
		using E = efloat_for<Posit::nbits>;
		using Wider = posit<Posit::nbits + 1u, Posit::es, typename Posit::BlockType>;
		std::mt19937_64 rng(seed);
		int fails = 0;
		for (unsigned s = 0; s < nrSamples; ++s) {
			Posit a = RandomPosit<Posit>(rng);
			if (a.isneg()) a = -a;
			Posit b(a);
			++b;
			if (a.iszero() || a.isnar() || b.isnar() || b.isneg()) continue;
			// the tie point: the (nbits+1)-bit pattern 2a + 1
			limb_t w[limbs(Posit::nbits + 1u)]{};
			gather(a, w);
			limb_kernels::lshift(w, w, limbs(Posit::nbits + 1u), 1u);
			set(w, 0u);
			Wider m;
			scatter(m, w);
			const auto tw = to_efloat(m);
			E t(0);
			t.assign_rounded_limbs(false, tw.limb_data(), tw.limb_count(), tw.scale(), false, static_cast<unsigned>(2u * tw.limb_count()));
			for (int d : { -1, 1 }) {
				unsigned first = 0;   // evaluate's first width: the offset sits 2 first bits below t
				const Posit got = evaluate<Posit>([&](unsigned W) {
					if (first == 0) first = W;
					E v(t);
					v.set_precision(4u * W + 64u);
					v = v + ldexp(E(d), static_cast<int>(t.scale()) - 2 * static_cast<int>(first));
					v.set_precision(W);   // rounds to t at the first width; the offset survives at twice it
					return v;
				});
				const Posit want = (d < 0) ? a : b;
				if (got != want) fails += Fail(reportTestCases, d < 0 ? "retry below tie" : "retry above tie", a, got, want);
			}
			const Posit even = a.bits().test(0) ? b : a;
			const Posit got = evaluate<Posit>([&](unsigned W) { E v(t); v.set_precision(W); return v; });
			if (got != even) fails += Fail(reportTestCases, "exact tie", a, got, even);
		}
		return fails;
	}

	// the posit functions round to nearest whatever efloat's rounding mode, and leave efloat's mode
	// and status flags as they found them, also when the evaluation throws
	template<typename Posit>
	int VerifyEnvironment(bool reportTestCases) {
		int fails = 0;
		const Posit x(0.7), y(3.25);
		const Posit e = exp(x), l = log(y), h = hypot(x, y);
		for (RoundingMode mode : { RoundingMode::RoundToZero, RoundingMode::RoundTowardPositive, RoundingMode::RoundTowardNegative }) {
			efloat_rounding_mode = mode;
			set_efloat_exceptions(ExceptionFlag::Overflow);
			const Posit e2 = exp(x), l2 = log(y), h2 = hypot(x, y);
			const bool restored = (efloat_rounding_mode == mode) && (get_efloat_exceptions() == ExceptionFlag::Overflow);
			efloat_rounding_mode = RoundingMode::RoundToNearest;
			clear_efloat_exceptions();
			if (e2 != e || l2 != l || h2 != h || !restored) {
				++fails;
				if (reportTestCases) std::cerr << "FAIL the efloat rounding mode or flags leaked into or out of the posit functions\n";
			}
		}
		// and when the evaluation throws
		efloat_rounding_mode = RoundingMode::RoundToZero;
		set_efloat_exceptions(ExceptionFlag::Overflow);
		try {
			(void)posit_limbs::evaluate<Posit>([](unsigned) -> posit_limbs::efloat_for<Posit::nbits> { throw std::runtime_error("evaluation failed"); });
		}
		catch (const std::runtime_error&) {}
		if (efloat_rounding_mode != RoundingMode::RoundToZero || get_efloat_exceptions() != ExceptionFlag::Overflow) {
			++fails;
			if (reportTestCases) std::cerr << "FAIL an exception from the evaluation left efloat's rounding mode or flags changed\n";
		}
		efloat_rounding_mode = RoundingMode::RoundToNearest;
		clear_efloat_exceptions();
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

	std::string test_suite  = "posit exp, log and hypot families, correctly rounded";
	std::string test_tag    = "elementary";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		using Posit = posit<64, 3>;
		const Posit x(0.5);
		std::cout << "exp(0.5) = " << to_binary(exp(x)) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 3, 1>>(reportTestCases), "posit< 3,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 5, 2>>(reportTestCases), "posit< 5,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 8, 0>>(reportTestCases), "posit< 8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit< 8, 2>>(reportTestCases), "posit< 8,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<10, 1>>(reportTestCases), "posit<10,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustivePairs<posit< 5, 1>>(reportTestCases), "posit< 5,1> hypot", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustivePairs<posit< 6, 2>>(reportTestCases), "posit< 6,2> hypot", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExact<posit<16, 1>>(reportTestCases), "posit<16,1> exact", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExact<posit<32, 2>>(reportTestCases), "posit<32,2> exact", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExact<posit<64, 3>>(reportTestCases), "posit<64,3> exact", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExact<posit<128, 4>>(reportTestCases), "posit<128,4> exact", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyEnvironment<posit<32, 2>>(reportTestCases), "posit<32,2> environment", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRetry<posit<16, 1>>(200, 1, reportTestCases), "posit<16,1> retry", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRetry<posit<64, 3>>(200, 2, reportTestCases), "posit<64,3> retry", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRetry<posit<256, 5>>(50, 3, reportTestCases), "posit<256,5> retry", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<16, 1>>(500, 4, reportTestCases), "posit<16,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<32, 2>>(500, 5, reportTestCases), "posit<32,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<64, 3>>(300, 6, reportTestCases), "posit<64,3>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<128, 4>>(100, 7, reportTestCases), "posit<128,4>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<256, 5>>(30, 8, reportTestCases), "posit<256,5>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<12, 1>>(reportTestCases), "posit<12,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustivePairs<posit< 8, 1>>(reportTestCases), "posit< 8,1> hypot", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<32, 2>>(10000, 9, reportTestCases), "posit<32,2>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<16, 1>>(reportTestCases), "posit<16,1>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<64, 3>>(10000, 10, reportTestCases), "posit<64,3>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<16, 2>>(reportTestCases), "posit<16,2>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<128, 4>>(10000, 11, reportTestCases), "posit<128,4>", test_tag);
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
