// mathlib.cpp: the elementary functions of the bounded posit (#1653)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This suite pins the library's contract at every function, including the ones still
// evaluated in double (erf, erfc, tgamma, lgamma); correct_rounding.cpp checks the correctly
// rounded ones for exact equality with an independent oracle.  Here:
//
//   - the exact functions (truncations, remainders, fabs, copysign, nextafter, min/max,
//     classification) are exact, for every encoding of the 16-bit configurations;
//   - every other function is within one encoding of the correctly rounded result,
//     against a long double reference, for every encoding at 16 bits and sampled at 32;
//   - the posit standard's promises hold: NaR propagates, a pole or domain error is NaR,
//     a finite result never becomes NaR and a nonzero one never becomes 0.
//
// The reference is long double, rounded once into the bposit.  Where long double is
// double (MSVC, Apple arm64) the one-encoding check still holds but compares two double
// evaluations.  The count of results that differ from the reference by one encoding is
// reported; for the correctly rounded functions it is zero by construction.
#include <universal/utility/directives.hpp>
#include <string>
#include <vector>
#include <cmath>
#include <cstdint>
#include <limits>
#include <universal/number/bposit/bposit.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

using sw::universal::SpecificValue;

// A function's result in exact arithmetic is one of these, which fixes what an infinity or
// a zero from the reference means.
enum class Range {
	any,       // an infinity is a pole: NaR
	finite,    // an infinity is an overflow: saturate to maxpos
	nonzero,   // finite and never 0: an underflow saturates to minpos
};

// the signed integer order of the encodings, which is value order
template<typename B>
std::int64_t ordinal(const B& v) {
	const std::uint64_t r = v.raw();
	const std::uint64_t top = 1ull << (B::nbits - 1);
	return (r & top) ? static_cast<std::int64_t>(r) - static_cast<std::int64_t>(top << 1) : static_cast<std::int64_t>(r);
}

// round a long double reference into the bposit, with the function's range promise
template<typename B>
B reference(long double r, Range range, bool negative) {
	if (r != r) return B(SpecificValue::nar);
	if (r == std::numeric_limits<long double>::infinity() || r == -std::numeric_limits<long double>::infinity()) {
		if (range == Range::any) return B(SpecificValue::nar);
		return B(r > 0 ? SpecificValue::maxpos : SpecificValue::maxneg);
	}
	if (r == 0.0l && range == Range::nonzero) return B(negative ? SpecificValue::minneg : SpecificValue::minpos);
	return B(r);
}

struct Tally {
	int fails = 0;      // more than one encoding from the reference, or a broken promise
	long offbyone = 0;  // exactly one encoding away: not correctly rounded
	long checked = 0;
};

// got must equal the reference when either is NaR, and be within one encoding otherwise
template<typename B>
void judge(const char* name, const B& x, const B& got, const B& want, bool exact, bool report, Tally& t) {
	++t.checked;
	if (got == want) return;
	const bool nar = got.isnar() || want.isnar();
	const std::int64_t d = ordinal(got) - ordinal(want);
	if (!nar && !exact && (d == 1 || d == -1)) { ++t.offbyone; return; }
	++t.fails;
	if (report && t.fails < 10) {
		std::cerr << "FAIL " << sw::universal::type_tag(x) << ' ' << name << '(' << double(x) << ") = " << double(got)
		          << " (" << sw::universal::to_binary(got) << "), want " << double(want) << " (" << sw::universal::to_binary(want) << ")\n";
	}
}

// one unary function: the bposit implementation, the long double reference, its range
template<typename B>
struct Unary {
	const char* name;
	B (*f)(const B&);
	long double (*ref)(long double);
	Range range;
	bool exact;
	bool (*negative)(long double);   // the sign of a nonzero result, for Range::nonzero
};

inline bool never(long double) { return false; }
inline bool gamma_negative(long double x) { return x < 0 && std::fmod(std::floor(x), 2.0l) != 0; }
inline long double ld_tgamma(long double x) {
	if (x <= 0 && x == std::floor(x)) return std::numeric_limits<long double>::quiet_NaN();   // poles
	return std::tgamma(x);
}
inline long double ld_lgamma(long double x) {
	if (x <= 0 && x == std::floor(x)) return std::numeric_limits<long double>::quiet_NaN();
	return std::lgamma(x);
}
inline long double ld_exp10(long double x) { return std::pow(10.0l, x); }
inline long double ld_frac(long double x) { return x - std::trunc(x); }

template<typename B>
std::vector<Unary<B>> unary_functions() {
	using namespace sw::universal;
	return {
		{ "floor", [](const B& x) { return floor(x); }, [](long double x) { return std::floor(x); }, Range::any, true, never },
		{ "ceil",  [](const B& x) { return ceil(x);  }, [](long double x) { return std::ceil(x);  }, Range::any, true, never },
		{ "trunc", [](const B& x) { return trunc(x); }, [](long double x) { return std::trunc(x); }, Range::any, true, never },
		{ "round", [](const B& x) { return round(x); }, [](long double x) { return std::round(x); }, Range::any, true, never },
		{ "frac",  [](const B& x) { return frac(x);  }, ld_frac, Range::any, true, never },
		{ "fabs",  [](const B& x) { return fabs(x);  }, [](long double x) { return std::fabs(x); }, Range::any, true, never },
		{ "sqrt",  [](const B& x) { return sqrt(x);  }, [](long double x) { return std::sqrt(x); }, Range::any, false, never },
		{ "cbrt",  [](const B& x) { return cbrt(x);  }, [](long double x) { return std::cbrt(x); }, Range::any, false, never },
		{ "exp",   [](const B& x) { return exp(x);   }, [](long double x) { return std::exp(x);   }, Range::nonzero, false, never },
		{ "exp2",  [](const B& x) { return exp2(x);  }, [](long double x) { return std::exp2(x);  }, Range::nonzero, false, never },
		{ "exp10", [](const B& x) { return exp10(x); }, ld_exp10, Range::nonzero, false, never },
		{ "expm1", [](const B& x) { return expm1(x); }, [](long double x) { return std::expm1(x); }, Range::finite, false, never },
		{ "log",   [](const B& x) { return log(x);   }, [](long double x) { return std::log(x);   }, Range::any, false, never },
		{ "log2",  [](const B& x) { return log2(x);  }, [](long double x) { return std::log2(x);  }, Range::any, false, never },
		{ "log10", [](const B& x) { return log10(x); }, [](long double x) { return std::log10(x); }, Range::any, false, never },
		{ "log1p", [](const B& x) { return log1p(x); }, [](long double x) { return std::log1p(x); }, Range::any, false, never },
		{ "sin",   [](const B& x) { return sin(x);   }, [](long double x) { return std::sin(x);   }, Range::any, false, never },
		{ "cos",   [](const B& x) { return cos(x);   }, [](long double x) { return std::cos(x);   }, Range::any, false, never },
		{ "tan",   [](const B& x) { return tan(x);   }, [](long double x) { return std::tan(x);   }, Range::finite, false, never },
		{ "asin",  [](const B& x) { return asin(x);  }, [](long double x) { return std::asin(x);  }, Range::any, false, never },
		{ "acos",  [](const B& x) { return acos(x);  }, [](long double x) { return std::acos(x);  }, Range::any, false, never },
		{ "atan",  [](const B& x) { return atan(x);  }, [](long double x) { return std::atan(x);  }, Range::any, false, never },
		{ "sinh",  [](const B& x) { return sinh(x);  }, [](long double x) { return std::sinh(x);  }, Range::finite, false, never },
		{ "cosh",  [](const B& x) { return cosh(x);  }, [](long double x) { return std::cosh(x);  }, Range::finite, false, never },
		{ "tanh",  [](const B& x) { return tanh(x);  }, [](long double x) { return std::tanh(x);  }, Range::any, false, never },
		{ "asinh", [](const B& x) { return asinh(x); }, [](long double x) { return std::asinh(x); }, Range::any, false, never },
		{ "acosh", [](const B& x) { return acosh(x); }, [](long double x) { return std::acosh(x); }, Range::any, false, never },
		{ "atanh", [](const B& x) { return atanh(x); }, [](long double x) { return std::atanh(x); }, Range::any, false, never },
		{ "erf",   [](const B& x) { return erf(x);   }, [](long double x) { return std::erf(x);   }, Range::any, false, never },
		{ "erfc",  [](const B& x) { return erfc(x);  }, [](long double x) { return std::erfc(x);  }, Range::nonzero, false, never },
		{ "tgamma",[](const B& x) { return tgamma(x);}, ld_tgamma, Range::nonzero, false, gamma_negative },
		{ "lgamma",[](const B& x) { return lgamma(x);}, ld_lgamma, Range::finite, false, never },
	};
}

template<typename B>
void check_unary(const Unary<B>& u, const B& x, bool report, Tally& t) {
	const long double v = static_cast<long double>(x);
	const B want = x.isnar() ? B(SpecificValue::nar) : reference<B>(u.ref(v), u.range, u.negative(v));
	judge(u.name, x, u.f(x), want, u.exact, report, t);
}

// every encoding through every unary function
template<typename B>
int VerifyUnaryExhaustive(bool report) {
	Tally t;
	const std::uint64_t N = 1ull << B::nbits;
	for (const auto& u : unary_functions<B>()) {
		for (std::uint64_t i = 0; i < N; ++i) {
			B x; x.setbits(i);
			check_unary(u, x, report, t);
		}
	}
	if (report) std::cout << "  " << sw::universal::type_tag(B()) << ": " << t.checked << " results, "
	                      << t.offbyone << " one encoding from correctly rounded\n";
	return t.fails;
}

std::uint64_t lcg(std::uint64_t& state) {
	state = state * 6364136223846793005ull + 1442695040888963407ull;
	return state;
}

template<typename B>
int VerifyUnarySampled(unsigned count, bool report) {
	Tally t;
	std::uint64_t state = 0x9E3779B97F4A7C15ull;
	for (const auto& u : unary_functions<B>()) {
		for (unsigned k = 0; k < count; ++k) {
			B x; x.setbits(lcg(state) >> (64 - B::nbits));
			check_unary(u, x, report, t);
		}
	}
	if (report) std::cout << "  " << sw::universal::type_tag(B()) << ": " << t.checked << " results, "
	                      << t.offbyone << " one encoding from correctly rounded\n";
	return t.fails;
}

// the binary functions, on random pairs
template<typename B>
int VerifyBinarySampled(unsigned count, bool report) {
	using namespace sw::universal;
	Tally t;
	std::uint64_t state = 0xD1B54A32D192ED03ull;
	for (unsigned k = 0; k < count; ++k) {
		B x; x.setbits(lcg(state) >> (64 - B::nbits));
		B y; y.setbits(lcg(state) >> (64 - B::nbits));
		const long double a = static_cast<long double>(x), b = static_cast<long double>(y);
		const bool nar = x.isnar() || y.isnar();
		auto ref = [&](long double r, Range range, bool negative) { return nar ? B(SpecificValue::nar) : reference<B>(r, range, negative); };
		// atan2(0, 0) is 0 in libm and here
		judge("atan2", x, atan2(x, y), ref(std::atan2(a, b), Range::any, false), false, report, t);
		judge("hypot", x, hypot(x, y), ref(std::hypot(a, b), Range::finite, false), false, report, t);
		judge("fmod", x, fmod(x, y), ref(std::fmod(a, b), Range::any, false), true, report, t);
		judge("remainder", x, remainder(x, y), ref(std::remainder(a, b), Range::any, false), true, report, t);
		judge("copysign", x, copysign(x, y), nar ? B(SpecificValue::nar) : B(std::copysign(a, y.isneg() ? -1.0l : 1.0l)), true, report, t);
		judge("min", x, min(x, y), nar ? B(SpecificValue::nar) : B(std::fmin(a, b)), true, report, t);
		judge("max", x, max(x, y), nar ? B(SpecificValue::nar) : B(std::fmax(a, b)), true, report, t);
		// pow: a nonzero base gives a nonzero power; a zero base is handled in VerifySpecialValues
		if (!x.iszero()) {
			const bool negative = a < 0 && std::fmod(b, 2.0l) != 0;
			judge("pow", x, pow(x, y), ref(std::pow(a, b), Range::nonzero, negative), false, report, t);
		}
	}
	if (report) std::cout << "  " << type_tag(B()) << ": " << t.checked << " results, "
	                      << t.offbyone << " one encoding from correctly rounded\n";
	return t.fails;
}

// nextafter steps one encoding, and the classification reads the encoding
template<typename B>
int VerifyNextAndClassify(bool report) {
	using namespace sw::universal;
	int fails = 0;
	auto expect = [&](bool ok, const char* what, const B& x) {
		if (ok) return;
		++fails;
		if (report && fails < 10) std::cerr << "FAIL " << type_tag(x) << ' ' << what << " at " << to_binary(x) << '\n';
	};
	const B maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	const std::uint64_t N = 1ull << B::nbits;
	for (std::uint64_t i = 0; i < N; ++i) {
		B x; x.setbits(i);
		const bool nar = x.isnar(), zero = x.iszero();
		expect(isnan(x) == nar && isfinite(x) == !nar && !isinf(x), "isnan/isfinite/isinf", x);
		expect(isnormal(x) == (!nar && !zero), "isnormal", x);
		expect(fpclassify(x) == (nar ? FP_NAN : zero ? FP_ZERO : FP_NORMAL), "fpclassify", x);
		expect(signbit(x) == x.sign(), "signbit", x);
		if (nar) {
			expect(nextafter(x, maxpos).isnar() && nextafter(maxpos, x).isnar(), "nextafter(NaR)", x);
			continue;
		}
		B up(x); ++up;
		B down(x); --down;
		if (x != maxpos) expect(nextafter(x, maxpos) == up, "nextafter up", x);
		if (x != maxneg) expect(nextafter(x, maxneg) == down, "nextafter down", x);
		expect(nextafter(x, x) == x, "nextafter(x, x)", x);
		if (x != maxpos) expect(nexttoward(x, std::numeric_limits<long double>::max()) == up, "nexttoward up", x);
	}
	return fails;
}

// the promises of the posit standard, and the values that have an exact answer
template<typename B>
int VerifySpecialValues(bool report) {
	using namespace sw::universal;
	int fails = 0;
	auto expect = [&](const B& got, const B& want, const char* what) {
		if (got == want) return;
		++fails;
		if (report) std::cerr << "FAIL " << type_tag(got) << ' ' << what << " = " << to_binary(got) << ", want " << to_binary(want) << '\n';
	};
	const B nar(SpecificValue::nar), zero(0), one(1), two(2), mone(-1);
	const B maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	const B minpos(SpecificValue::minpos), minneg(SpecificValue::minneg);

	// NaR propagates
	for (const auto& u : unary_functions<B>()) expect(u.f(nar), nar, u.name);
	expect(pow(nar, one), nar, "pow(NaR, 1)");
	expect(pow(one, nar), nar, "pow(1, NaR)");
	expect(atan2(nar, one), nar, "atan2(NaR, 1)");
	expect(hypot(one, nar), nar, "hypot(1, NaR)");
	expect(min(nar, one), nar, "min(NaR, 1)");
	expect(max(one, nar), nar, "max(1, NaR)");
	expect(copysign(one, nar), nar, "copysign(1, NaR)");

	// poles and domain errors are NaR
	expect(log(zero), nar, "log(0)");
	expect(log(mone), nar, "log(-1)");
	expect(log1p(mone), nar, "log1p(-1)");
	expect(sqrt(mone), nar, "sqrt(-1)");
	expect(asin(two), nar, "asin(2)");
	expect(acosh(zero), nar, "acosh(0)");
	expect(atanh(one), nar, "atanh(1)");
	expect(tgamma(zero), nar, "tgamma(0)");
	expect(tgamma(mone), nar, "tgamma(-1)");
	expect(lgamma(B(-2)), nar, "lgamma(-2)");
	expect(pow(zero, mone), nar, "pow(0, -1)");
	expect(pow(mone, B(0.5)), nar, "pow(-1, 0.5)");
	expect(fmod(one, zero), nar, "fmod(1, 0)");

	// a finite result saturates at maxpos and a nonzero one at minpos, beyond double's range too
	expect(exp(maxpos), maxpos, "exp(maxpos)");
	expect(exp(maxneg), minpos, "exp(maxneg)");
	expect(exp2(maxneg), minpos, "exp2(maxneg)");
	expect(exp10(maxpos), maxpos, "exp10(maxpos)");
	expect(expm1(maxpos), maxpos, "expm1(maxpos)");
	expect(sinh(maxpos), maxpos, "sinh(maxpos)");
	expect(sinh(maxneg), maxneg, "sinh(maxneg)");
	expect(cosh(maxneg), maxpos, "cosh(maxneg)");
	expect(erfc(maxpos), minpos, "erfc(maxpos)");
	expect(tgamma(maxpos), maxpos, "tgamma(maxpos)");
	expect(lgamma(maxpos), maxpos, "lgamma(maxpos)");
	expect(pow(two, maxpos), maxpos, "pow(2, maxpos)");
	expect(pow(two, maxneg), minpos, "pow(2, maxneg)");
	expect(pow(B(-2), B(255)), maxneg, "pow(-2, 255)");
	expect(pow(B(-2), B(-255)), minneg, "pow(-2, -255)");
	if constexpr (B::maxscale >= 8) {
		// gamma(-200.5) is about -1e-375: it underflows double, and is negative
		expect(tgamma(B(-200.5)), minneg, "tgamma(-200.5)");
	}

	// exact values
	expect(pow(zero, zero), one, "pow(0, 0)");
	expect(pow(zero, two), zero, "pow(0, 2)");
	expect(pow(two, 10), B(1024), "pow(2, 10)");
	expect(pow(B(9), B(0.5)), B(3), "pow(9, 0.5)");
	expect(sqrt(B(4)), two, "sqrt(4)");
	expect(cbrt(B(-8)), B(-2), "cbrt(-8)");
	expect(hypot(B(3), B(4)), B(5), "hypot(3, 4)");
	expect(hypot(B(2), B(3), B(6)), B(7), "hypot(2, 3, 6)");
	expect(exp(zero), one, "exp(0)");
	expect(log(one), zero, "log(1)");
	expect(log2(B(8)), B(3), "log2(8)");
	expect(sin(zero), zero, "sin(0)");
	expect(cos(zero), one, "cos(0)");
	expect(atan2(zero, zero), zero, "atan2(0, 0)");
	expect(floor(B(-2.5)), B(-3), "floor(-2.5)");
	expect(ceil(B(-2.5)), B(-2), "ceil(-2.5)");
	expect(round(B(2.5)), B(3), "round(2.5)");
	expect(round(B(-2.5)), B(-3), "round(-2.5)");
	expect(trunc(B(-2.5)), B(-2), "trunc(-2.5)");
	expect(frac(B(-2.5)), B(-0.5), "frac(-2.5)");
	expect(fmod(B(7), B(3)), one, "fmod(7, 3)");
	expect(remainder(B(7), B(3)), one, "remainder(7, 3)");
	expect(remainder(B(8), B(3)), mone, "remainder(8, 3)");
	expect(copysign(two, mone), B(-2), "copysign(2, -1)");
	expect(fabs(B(-2)), two, "fabs(-2)");
	expect(min(one, two), one, "min(1, 2)");
	expect(max(one, two), two, "max(1, 2)");
	expect(nexttoward(maxpos, std::numeric_limits<long double>::max()), maxpos, "nexttoward(maxpos, +big)");
	expect(nexttoward(maxneg, -std::numeric_limits<long double>::max()), maxneg, "nexttoward(maxneg, -big)");
	return fails;
}

}  // anonymous namespace

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
	std::string test_suite  = "bposit mathlib";
	std::string test_tag    = "mathlib";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;
	ReportTestSuiteHeader(test_suite, reportTestCases);

	using bposit16_42 = bposit<16, 4, 2, std::uint16_t>;
	using bposit16_52 = bposit<16, 5, 2, std::uint16_t>;
	using bposit32_42 = bposit<32, 4, 2, std::uint32_t>;

#if MANUAL_TESTING
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<bposit16_42>(true), "bposit<16,4,2>", "special values");
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else
#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<bposit16_42>(reportTestCases), "bposit<16,4,2>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<bposit16>(reportTestCases), "bposit<16,6,5>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<bposit32>(reportTestCases), "bposit<32,6,5>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifySpecialValues<bposit64>(reportTestCases), "bposit<64,6,5>", "special values");
	nrOfFailedTestCases += ReportTestResult(VerifyNextAndClassify<bposit16_42>(reportTestCases), "bposit<16,4,2>", "next and classify");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryExhaustive<bposit16_42>(reportTestCases), "bposit<16,4,2>", "exhaustive unary");
	nrOfFailedTestCases += ReportTestResult(VerifyBinarySampled<bposit16_42>(20000, reportTestCases), "bposit<16,4,2>", "sampled binary");
#endif
#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryExhaustive<bposit16_52>(reportTestCases), "bposit<16,5,2>", "exhaustive unary");
	nrOfFailedTestCases += ReportTestResult(VerifyUnaryExhaustive<bposit16>(reportTestCases), "bposit<16,6,5>", "exhaustive unary");
	nrOfFailedTestCases += ReportTestResult(VerifyNextAndClassify<bposit16>(reportTestCases), "bposit<16,6,5>", "next and classify");
	nrOfFailedTestCases += ReportTestResult(VerifyBinarySampled<bposit16>(20000, reportTestCases), "bposit<16,6,5>", "sampled binary");
#endif
#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyUnarySampled<bposit32_42>(20000, reportTestCases), "bposit<32,4,2>", "sampled unary");
	nrOfFailedTestCases += ReportTestResult(VerifyUnarySampled<bposit32>(20000, reportTestCases), "bposit<32,6,5>", "sampled unary");
	nrOfFailedTestCases += ReportTestResult(VerifyBinarySampled<bposit32_42>(20000, reportTestCases), "bposit<32,4,2>", "sampled binary");
	nrOfFailedTestCases += ReportTestResult(VerifyBinarySampled<bposit32>(20000, reportTestCases), "bposit<32,6,5>", "sampled binary");
#endif
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
