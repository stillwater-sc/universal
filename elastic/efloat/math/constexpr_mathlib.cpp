// constexpr_mathlib.cpp: the efloat math library evaluated by the compiler (#1819, phase 3)
//
//   agreement    every function of the math library at a set of arguments, each value a constant
//                evaluation of its own, against the run time: the limbs, the scale, the sign, the
//                state and the precision of every result are the same. The functions round
//                correctly, so the two agree although a constant evaluation recomputes the
//                constants the run time caches per thread (ln 2, pi, the log table, the Stirling
//                coefficients) and rounds to nearest without the thread's mode.
//   exact cases  the values a function returns exactly, in static_assert
//   conversion   efloat from and to float and double in static_assert: the fields of the
//                encoding, with no <cmath> call
//   constants    exp(1), pi = acos(-1) and sqrt(2) at 1024 bits by the compiler, their leading
//                limbs in static_assert, every limb the run time's
//
// lgamma and tgamma sum Stirling's series from coefficients a constant evaluation cannot cache:
// they run past clang's and MSVC's default step limits, which CMakeLists.txt raises for this
// test (-fconstexpr-steps, /constexpr:steps).
//
// REGRESSION_LEVEL convention (intensity progression):
//   LEVEL 1 -- every function at 128 bits (the gamma functions at 64), the exact cases, the
//              conversions, and exp(1), pi and sqrt(2) at 1024 bits.
//   LEVEL 2 -- every function at 512 bits as well (the gamma functions at 128).
//   LEVEL 3 -- as level 2.
//   LEVEL 4 -- as level 3.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using sw::universal::efloat;

	// the state, sign, scale, precision and limbs of a value
	struct digest {
		std::array<std::uint64_t, 40> word{};
		std::size_t n = 0;
		constexpr void put(std::uint64_t v) { if (n < word.size()) word[n] = v; ++n; }
		template<unsigned nlimbs>
		constexpr void put(const efloat<nlimbs>& v) {
			put(static_cast<std::uint64_t>(v.scale()));
			put((v.iszero() ? 1u : 0u) | (v.isinf() ? 2u : 0u) | (v.isnan() ? 4u : 0u) | (v.sign() < 0 ? 8u : 0u));
			put(v.get_precision());
			put(v.limb_count());
			for (std::size_t i = 0; i < v.limb_count(); ++i) put(v.limb_data()[i]);
		}
		constexpr bool operator==(const digest&) const = default;
	};

	// num / den at `bits` bits
	template<unsigned nlimbs>
	constexpr efloat<nlimbs> ratio(int num, int den, unsigned bits) {
		efloat<nlimbs> v(num);
		v.set_precision(bits);
		v /= den;
		return v;
	}

	// the arguments: inside the functions' domains and outside them, near 1, near pi, a tiny one
	// and one past 2^10
	struct fraction { int num, den; };
	constexpr std::array<fraction, 6> arguments{ { { 3, 7 }, { -5, 3 }, { 1, 1000 }, { 1000001, 1000000 }, { 355, 113 }, { 2049, 2 } } };

	template<unsigned nlimbs, unsigned bits, typename F, std::size_t i>
	constexpr digest unary_value() {
		digest d;
		const F f{};
		d.put(f(ratio<nlimbs>(arguments[i].num, arguments[i].den, bits)));
		return d;
	}

	template<unsigned nlimbs, unsigned bits, typename F, std::size_t i>
	constexpr digest binary_value() {
		constexpr std::size_t j = (i + 2) % arguments.size();
		digest d;
		const F f{};
		d.put(f(ratio<nlimbs>(arguments[i].num, arguments[i].den, bits), ratio<nlimbs>(arguments[j].num, arguments[j].den, bits)));
		return d;
	}

	template<unsigned nlimbs, unsigned bits, typename F, std::size_t i>
	constexpr digest ternary_value() {
		constexpr std::size_t j = (i + 2) % arguments.size();
		constexpr std::size_t k = (i + 5) % arguments.size();
		digest d;
		const F f{};
		d.put(f(ratio<nlimbs>(arguments[i].num, arguments[i].den, bits), ratio<nlimbs>(arguments[j].num, arguments[j].den, bits),
		        ratio<nlimbs>(arguments[k].num, arguments[k].den, bits)));
		return d;
	}

	template<unsigned nlimbs, unsigned bits, typename F, int arity, std::size_t i>
	constexpr digest value_of() {
		if constexpr (arity == 1) return unary_value<nlimbs, bits, F, i>();
		else if constexpr (arity == 2) return binary_value<nlimbs, bits, F, i>();
		else return ternary_value<nlimbs, bits, F, i>();
	}

	// one argument: its value by the compiler, a constant evaluation of its own, and at run time
	template<unsigned nlimbs, unsigned bits, typename F, int arity, std::size_t i>
	int verify_one(const char* name, bool reportTestCases) {
		constexpr digest compiled = value_of<nlimbs, bits, F, arity, i>();
		static_assert(compiled.n <= compiled.word.size(), "the digest holds every word");
		const digest run = value_of<nlimbs, bits, F, arity, i>();
		if (compiled == run) return 0;
		if (reportTestCases) {
			std::cerr << "FAIL: " << name << " at " << bits << " bits, argument " << arguments[i].num << '/' << arguments[i].den
			          << ": constant evaluation differs from the run time\n";
		}
		return 1;
	}

	template<unsigned nlimbs, unsigned bits, typename F, int arity, std::size_t... i>
	int verify_all(const char* name, bool reportTestCases, std::index_sequence<i...>) {
		return (verify_one<nlimbs, bits, F, arity, i>(name, reportTestCases) + ...);
	}

	template<unsigned bits, int arity = 1, typename F>
	int VerifyFunction(const char* name, F, bool reportTestCases) {
		constexpr unsigned nlimbs = bits / 32 + 2;
		return verify_all<nlimbs, bits, F, arity>(name, reportTestCases, std::make_index_sequence<arguments.size()>{});
	}

#define EFLOAT_FUNCTION_1(fn) [](const auto& x) { return fn(x); }
#define EFLOAT_FUNCTION_2(fn) [](const auto& x, const auto& y) { return fn(x, y); }

	template<unsigned bits>
	int VerifyMathLibrary(bool reportTestCases) {
		int failures = 0;
		failures += VerifyFunction<bits>("exp", EFLOAT_FUNCTION_1(exp), reportTestCases);
		failures += VerifyFunction<bits>("exp2", EFLOAT_FUNCTION_1(exp2), reportTestCases);
		failures += VerifyFunction<bits>("exp10", EFLOAT_FUNCTION_1(exp10), reportTestCases);
		failures += VerifyFunction<bits>("expm1", EFLOAT_FUNCTION_1(expm1), reportTestCases);
		failures += VerifyFunction<bits>("log", EFLOAT_FUNCTION_1(log), reportTestCases);
		failures += VerifyFunction<bits>("log2", EFLOAT_FUNCTION_1(log2), reportTestCases);
		failures += VerifyFunction<bits>("log10", EFLOAT_FUNCTION_1(log10), reportTestCases);
		failures += VerifyFunction<bits>("log1p", EFLOAT_FUNCTION_1(log1p), reportTestCases);
		failures += VerifyFunction<bits>("sin", EFLOAT_FUNCTION_1(sin), reportTestCases);
		failures += VerifyFunction<bits>("cos", EFLOAT_FUNCTION_1(cos), reportTestCases);
		failures += VerifyFunction<bits>("tan", EFLOAT_FUNCTION_1(tan), reportTestCases);
		failures += VerifyFunction<bits>("asin", EFLOAT_FUNCTION_1(asin), reportTestCases);
		failures += VerifyFunction<bits>("acos", EFLOAT_FUNCTION_1(acos), reportTestCases);
		failures += VerifyFunction<bits>("atan", EFLOAT_FUNCTION_1(atan), reportTestCases);
		failures += VerifyFunction<bits>("sinh", EFLOAT_FUNCTION_1(sinh), reportTestCases);
		failures += VerifyFunction<bits>("cosh", EFLOAT_FUNCTION_1(cosh), reportTestCases);
		failures += VerifyFunction<bits>("tanh", EFLOAT_FUNCTION_1(tanh), reportTestCases);
		failures += VerifyFunction<bits>("asinh", EFLOAT_FUNCTION_1(asinh), reportTestCases);
		failures += VerifyFunction<bits>("acosh", EFLOAT_FUNCTION_1(acosh), reportTestCases);
		failures += VerifyFunction<bits>("atanh", EFLOAT_FUNCTION_1(atanh), reportTestCases);
		failures += VerifyFunction<bits>("erf", EFLOAT_FUNCTION_1(erf), reportTestCases);
		failures += VerifyFunction<bits>("erfc", EFLOAT_FUNCTION_1(erfc), reportTestCases);
		failures += VerifyFunction<bits>("sqrt", EFLOAT_FUNCTION_1(sqrt), reportTestCases);
		failures += VerifyFunction<bits>("cbrt", EFLOAT_FUNCTION_1(cbrt), reportTestCases);
		failures += VerifyFunction<bits>("trunc", EFLOAT_FUNCTION_1(trunc), reportTestCases);
		failures += VerifyFunction<bits>("floor", EFLOAT_FUNCTION_1(floor), reportTestCases);
		failures += VerifyFunction<bits>("ceil", EFLOAT_FUNCTION_1(ceil), reportTestCases);
		failures += VerifyFunction<bits>("round", EFLOAT_FUNCTION_1(round), reportTestCases);
		failures += VerifyFunction<bits>("rint", EFLOAT_FUNCTION_1(rint), reportTestCases);
		failures += VerifyFunction<bits>("nearbyint", EFLOAT_FUNCTION_1(nearbyint), reportTestCases);
		failures += VerifyFunction<bits, 2>("pow", EFLOAT_FUNCTION_2(pow), reportTestCases);
		failures += VerifyFunction<bits, 2>("atan2", EFLOAT_FUNCTION_2(atan2), reportTestCases);
		failures += VerifyFunction<bits, 2>("hypot", EFLOAT_FUNCTION_2(hypot), reportTestCases);
		failures += VerifyFunction<bits, 2>("fmod", EFLOAT_FUNCTION_2(fmod), reportTestCases);
		failures += VerifyFunction<bits, 2>("remainder", EFLOAT_FUNCTION_2(remainder), reportTestCases);
		failures += VerifyFunction<bits, 2>("nextafter", EFLOAT_FUNCTION_2(nextafter), reportTestCases);
		failures += VerifyFunction<bits, 2>("fdim", EFLOAT_FUNCTION_2(fdim), reportTestCases);
		auto fused = [](const auto& x, const auto& y, const auto& z) { return fma(x, y, z); };
		failures += VerifyFunction<bits, 3>("fma", fused, reportTestCases);
		return failures;
	}

	// lgamma and tgamma, over the arguments that are not poles
	template<unsigned bits>
	int VerifyGamma(bool reportTestCases) {
		int failures = 0;
		failures += VerifyFunction<bits>("lgamma", EFLOAT_FUNCTION_1(lgamma), reportTestCases);
		failures += VerifyFunction<bits>("tgamma", EFLOAT_FUNCTION_1(tgamma), reportTestCases);
		return failures;
	}

	// the values the functions return exactly, and the conversions from and to float and double
	int VerifyStaticAsserts() {
		using E = efloat<8>;
		constexpr E zero(0), one(1), two(2), three(3), four(4);

		static_assert(exp(zero) == one && exp2(E(10)) == E(1024) && exp10(three) == E(1000), "exp(0), 2^10, 10^3");
		static_assert(log(one).iszero() && log2(E(1024)) == E(10) && log10(E(1000)) == three, "log(1), log2(1024), log10(1000)");
		static_assert(log(zero).isinf() && log(zero).sign() == -1 && log(-one).isnan(), "log(0) = -inf, log(-1) = NaN");
		static_assert(expm1(zero).iszero() && log1p(zero).iszero(), "expm1(0), log1p(0)");
		static_assert(sin(zero).iszero() && cos(zero) == one && tan(zero).iszero() && atan(zero).iszero(), "trig at 0");
		static_assert(acos(one).iszero() && asin(two).isnan(), "acos(1) = 0, asin(2) = NaN");
		static_assert(sinh(zero).iszero() && cosh(zero) == one && tanh(zero).iszero() && acosh(one).iszero(), "hyperbolic at 0 and 1");
		static_assert(erf(zero).iszero() && erfc(zero) == one, "erf(0), erfc(0)");
		static_assert(cbrt(E(27)) == three && cbrt(E(-8)) == -two && hypot(three, four) == E(5), "cbrt(27), cbrt(-8), hypot(3, 4)");
		static_assert(pow(two, E(10)) == E(1024) && pow(four, E(1) / two) == two && pow(two, -two) == one / four, "exact powers");
		static_assert(fma(two, three, E(-6)).iszero() && fma(two, three, E(-6)).sign() == 1 && fma(three, four, one) == E(13), "fma: an exact cancellation is +0");
		static_assert(fmod(E(7), three) == one && remainder(E(7), three) == one && fmod(E(-7), three) == -one, "fmod, remainder");
		static_assert(floor(E(5) / two) == two && ceil(E(5) / two) == three && round(E(5) / two) == three && rint(E(5) / two) == two, "rounding 5/2");
		static_assert(exp(log(two) * E(10)) > E(1023) && exp(log(two) * E(10)) < E(1025), "exp(10 ln 2) ~= 1024");

		// pi three ways: acos(-1), 4 atan(1), atan2(0, -1)
		constexpr E pi = acos(-one);
		static_assert(pi == four * atan(one) && pi == atan2(zero, -one), "acos(-1) = 4 atan(1) = atan2(0, -1)");
		static_assert(double(pi) == 3.141592653589793 && float(pi) == 3.14159265f, "pi rounded to double and float");
		static_assert(double(exp(one)) == 2.718281828459045, "e rounded to double");

		// conversions from and to float and double by the fields of the encoding
		static_assert(double(E(0.1)) == 0.1 && float(E(0.1f)) == 0.1f, "0.1 round trips");
		static_assert(double(E(5e-324)) == 5e-324 && double(E(2.2250738585072009e-308)) == 2.2250738585072009e-308, "subnormal doubles");
		static_assert(double(E(1.7976931348623157e308)) == 1.7976931348623157e308 && double(E(-0.0)) == 0.0, "the largest double, -0");
		static_assert(E(0.5) == one / two && E(-3.0) == -three && E(1e300).scale() == 996, "values from double");
		static_assert(double(one / three) == 1.0 / 3.0 && float(one / three) == 1.0f / 3.0f, "1/3 rounded to nearest");
		static_assert(double(E(1e308) * E(10)) > 1.7976931348623157e308, "past the largest double: inf");
		return 0;
	}

	// the limbs of a function value at 1024 bits, least significant first
	struct limbs1024 {
		std::array<std::uint64_t, 16> limb{};
		std::int64_t scale = 0;
	};

	template<typename F>
	constexpr limbs1024 at1024(F f) {
		using E = efloat<34>;
		const E v = f(E{});
		limbs1024 r;
		for (std::size_t i = 0; i < v.limb_count() && i < r.limb.size(); ++i) r.limb[i] = v.limb_data()[i];
		r.scale = v.scale();
		return r;
	}

	// exp(1), pi and sqrt(2) at 1024 bits (#1819): by the compiler, and bit for bit the run time's
	int VerifyConstants(bool reportTestCases) {
		constexpr auto e_of = [](auto x) { decltype(x) one(1); one.set_precision(1024); return exp(one); };
		constexpr auto pi_of = [](auto x) { decltype(x) one(-1); one.set_precision(1024); return acos(one); };
		constexpr auto root2_of = [](auto x) { decltype(x) two(2); two.set_precision(1024); return sqrt(two); };
		constexpr limbs1024 e = at1024(e_of);
		constexpr limbs1024 pi = at1024(pi_of);
		constexpr limbs1024 root2 = at1024(root2_of);
		static_assert(e.limb[15] == 0xADF8'5458'A2BB'4A9Aull && e.scale == 1, "e = 10.1011011111100001010...");
		static_assert(pi.limb[15] == 0xC90F'DAA2'2168'C234ull && pi.scale == 1, "pi = 11.0010010000111111011...");
		static_assert(root2.limb[15] == 0xB504'F333'F9DE'6484ull && root2.scale == 0, "sqrt(2) = 1.0110101000001001111...");
		int failures = 0;
		const limbs1024 run[3] = { at1024(e_of), at1024(pi_of), at1024(root2_of) };
		const limbs1024 compiled[3] = { e, pi, root2 };
		const char* names[3] = { "exp(1)", "pi", "sqrt(2)" };
		for (int k = 0; k < 3; ++k) {
			if (run[k].limb != compiled[k].limb || run[k].scale != compiled[k].scale) {
				++failures;
				if (reportTestCases) std::cerr << "FAIL: " << names[k] << " at 1024 bits: constant evaluation differs from the run time\n";
			}
		}
		return failures;
	}

}  // namespace

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 0
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

int main() try {
	using namespace sw::universal;

	std::string test_suite          = "efloat mathlib constexpr verification";
	std::string test_tag            = "constexpr mathlib";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyStaticAsserts(), "efloat", "static_assert");
	nrOfFailedTestCases += ReportTestResult(VerifyFunction<128>("exp", EFLOAT_FUNCTION_1(exp), reportTestCases), "efloat", "exp at 128 bits");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;

#else

#	if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyStaticAsserts(), "efloat", "static_assert");
	nrOfFailedTestCases += ReportTestResult(VerifyMathLibrary<128>(reportTestCases), "efloat", "math library at 128 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyGamma<64>(reportTestCases), "efloat", "lgamma, tgamma at 64 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyConstants(reportTestCases), "efloat", "exp(1), pi, sqrt(2) at 1024 bits");
#	endif

#	if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyMathLibrary<512>(reportTestCases), "efloat", "math library at 512 bits");
	nrOfFailedTestCases += ReportTestResult(VerifyGamma<128>(reportTestCases), "efloat", "lgamma, tgamma at 128 bits");
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
