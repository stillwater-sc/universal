// exact_truncation.cpp: hfloat arithmetic is the exact result truncated to ndigits hex digits, at every width (#2105)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Each check here failed before #2105:
//   - operator+= shifted a uint64_t by 4 * (exponent gap) bits: UB from a gap of 16 hex digits, and
//     every bit shifted off was lost: hfp64(1) + hfp64(16^-17) gave 1.0625, and hfp64(1) - 16^-14
//     gave 1.0 instead of 0.FFFFFFFFFFFFFF.
//   - hfp128 kept only the low 64 of its 112 fraction bits: hfp128(1.0) == hfp128(1.5) was true,
//     1.5 * 1.5 was 0, 1.5 / 1.0 divided by zero (SIGFPE) and isone() was always false.
//   - numeric_limits::epsilon() was 1.0 and digits counted bits for a radix-16 type.
//   - setbits() shifted a uint64_t by 64 for a 64-bit block type.
// The reference is exact integer arithmetic on integer<1536>: an operand is F * 2^(4e - fbits), a
// sum, product or quotient is formed exactly (a quotient as a floor with enough extra bits), and
// truncated to ndigits hex digits with its own leading-digit normalization.
#include <universal/utility/directives.hpp>
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>

namespace {

	using Exact = sw::universal::integer<1536, std::uint32_t, sw::universal::IntegerNumberType::IntegerNumber>;

	// a test-side builder: sets the encoding bit by bit through the protected setbit
	template<typename H>
	struct Builder : public H {
		void put(unsigned pos, bool v) { this->setbit(pos, v); }
	};

	// an hfloat with the given sign, unbiased exponent and fraction bits from the generator:
	// normalized, or with `zeros` leading hex digits cleared, as setbits() can produce
	template<typename H>
	H make(bool negative, int e, std::mt19937_64& rng, unsigned zeros = 0) {
		Builder<H> b;
		b.clear();
		for (unsigned i = 0; i < H::fbits; ++i) b.put(i, (rng() & 1u) != 0);
		b.put(H::fbits - 1u - static_cast<unsigned>(rng() % 4u), true);   // a non-zero leading hex digit
		for (unsigned i = 0; i < 4u * zeros && i + 4u < H::fbits; ++i) b.put(H::fbits - 1u - i, false);
		b.put(0, true);                                                     // never all zero
		const unsigned biased = static_cast<unsigned>(e + H::bias);
		for (unsigned i = 0; i < H::es; ++i) b.put(H::fbits + i, ((biased >> i) & 1u) != 0);
		b.put(H::nbits - 1u, negative);
		return static_cast<H>(b);
	}

	// x = sign * F * 2^E2, with E2 = 4e - fbits
	struct Value {
		bool negative;
		Exact F;
		int E2;
	};

	template<typename H>
	Value value_of(const H& x) {
		bool s; int e; typename H::frac_t f;
		x.decode(s, e, f);
		Value v{ s, Exact(0), 4 * e - static_cast<int>(H::fbits) };
		for (unsigned i = H::fbits; i-- > 0;) {
			v.F <<= 1;
			if (f.test(i)) v.F += Exact(1);
		}
		return v;
	}

	int floor_div4(int b) { return (b >= 0) ? (b / 4) : -((-b + 3) / 4); }

	// does x hold N * 2^E2 truncated toward zero to ndigits hex digits, with the given sign?
	// Outside the normal range the rule is hfloat's own (#2133; this returned 0 there, unchecked):
	// past emax it saturates to maxpos / maxneg, it has no infinity; below emin it flushes to 0, it
	// has no subnormals.
	template<typename H>
	int check(const H& x, bool negative, const Exact& N, int E2, const char* op, bool reportTestCases) {
		using namespace sw::universal;
		bool s; int e; typename H::frac_t f;
		x.decode(s, e, f);
		if (N == Exact(0)) {
			if (f.iszero()) return 0;
			if (reportTestCases) std::cerr << "FAIL " << op << ": expected 0, got " << to_hex(x) << '\n';
			return 1;
		}
		const int m = findMsb(N);
		const int er = floor_div4(m + E2) + 1;               // 16^(er-1) <= |value| < 16^er
		if (er > H::emax) {
			const H sat(negative ? SpecificValue::maxneg : SpecificValue::maxpos);
			if (to_hex(x) == to_hex(sat)) return 0;
			if (reportTestCases) std::cerr << "FAIL " << op << ": overflow gave " << to_hex(x) << ", expected " << to_hex(sat) << '\n';
			return 1;
		}
		if (er < H::emin) {
			if (f.iszero()) return 0;
			if (reportTestCases) std::cerr << "FAIL " << op << ": underflow gave " << to_hex(x) << ", expected 0\n";
			return 1;
		}
		Exact F = N;
		const int sh = E2 - 4 * er + static_cast<int>(H::fbits);
		if (sh >= 0) F <<= sh; else F >>= -sh;
		bool ok = (s == negative) && (e == er);
		for (unsigned i = 0; ok && i < H::fbits; ++i) {
			if (F.test(i) != f.test(i)) ok = false;
		}
		if (!ok && reportTestCases) std::cerr << "FAIL " << op << ": got " << to_hex(x) << ", expected hex exponent " << er << '\n';
		return ok ? 0 : 1;
	}

	// unnormalized: the operands may have zero leading hex digits (setbits() encodings)
	// center: the exponent the operands are drawn around; near emax or emin the products, quotients
	// and sums leave the normal range, which exercises saturation and flush to zero
	template<typename H>
	int VerifyArithmetic(int maxGap, unsigned nrSamples, bool reportTestCases, bool unnormalized = false, int center = 0) {
		std::mt19937_64 rng((unnormalized ? 2106 : 2105) + static_cast<unsigned>(center + 1000));
		auto clampE = [](int e) { return e < H::emin ? H::emin : (e > H::emax ? H::emax : e); };
		int fails = 0;
		for (unsigned k = 0; k < nrSamples; ++k) {
			const int ea = clampE(center + static_cast<int>(rng() % 21u) - 10);
			const int eb = clampE(ea - static_cast<int>(rng() % static_cast<unsigned>(maxGap + 1)) * ((rng() & 1u) ? 1 : -1));
			const unsigned za = unnormalized ? static_cast<unsigned>(rng() % H::ndigits) : 0u;
			const unsigned zb = unnormalized ? static_cast<unsigned>(rng() % H::ndigits) : 0u;
			const H a = make<H>((rng() & 1u) != 0, ea, rng, za);
			const H b = make<H>((rng() & 1u) != 0, eb, rng, zb);
			const Value va = value_of(a), vb = value_of(b);
			// sum and difference: align to the smaller E2
			{
				const int E2 = (va.E2 < vb.E2) ? va.E2 : vb.E2;
				Exact A = va.F, B = vb.F;
				A <<= (va.E2 - E2);
				B <<= (vb.E2 - E2);
				if (va.negative) A = -A;
				if (vb.negative) B = -B;
				Exact S = A + B;
				const bool sn = S < Exact(0);
				fails += check(H(a + b), sn, sn ? Exact(-S) : S, E2, "a + b", reportTestCases);
				Exact D = A - B;
				const bool dn = D < Exact(0);
				fails += check(H(a - b), dn, dn ? Exact(-D) : D, E2, "a - b", reportTestCases);
			}
			// product
			fails += check(H(a * b), va.negative != vb.negative, Exact(va.F * vb.F), va.E2 + vb.E2, "a * b", reportTestCases);
			// quotient: floor(Fa * 2^S / Fb) keeps fbits + 4 bits past the leading digit, also when
			// Fa has up to ndigits - 1 leading zero digits (an unnormalized operand)
			{
				constexpr int S = static_cast<int>(H::fbits) + 8 + 4 * static_cast<int>(H::ndigits);
				Exact num = va.F;
				num <<= S;
				fails += check(H(a / b), va.negative != vb.negative, Exact(num / vb.F), va.E2 - vb.E2 - S, "a / b", reportTestCases);
			}
			if (fails > 0 && reportTestCases && fails < 12) {
				std::cerr << "  a = " << sw::universal::to_hex(a) << "\n  b = " << sw::universal::to_hex(b) << '\n';
			}
		}
		return fails;
	}

	// the sweep's repros and the operations that were wrong for hfp128
	int VerifyRepros(bool reportTestCases) {
		using namespace sw::universal;
		int fails = 0;
		auto expect = [&](bool ok, const char* what) {
			if (!ok) { ++fails; if (reportTestCases) std::cerr << "FAIL " << what << '\n'; }
		};
		expect(double(hfp64(1.0) + hfp64(std::ldexp(1.0, -70))) == 1.0, "hfp64(1) + 2^-70 is 1");
		expect(double(hfp64(1.0) + hfp64(std::ldexp(1.0, -68))) == 1.0, "hfp64(1) + 16^-17 is 1");
		expect(to_hex(hfp64(1.0) - hfp64(std::ldexp(1.0, -56))) == "+0x0.FFFFFFFFFFFFFF * 16^0", "hfp64(1) - 16^-14 is 0.FFFFFFFFFFFFFF");
		expect(to_hex(hfp32(1.0) - hfp32(std::ldexp(1.0, -40))) == "+0x0.FFFFFF * 16^0", "hfp32(1) - 2^-40 is 0.FFFFFF");
		expect(!(hfp128(1.0) == hfp128(1.5)), "hfp128 1.0 != 1.5");
		expect(hfp128(1.0) < hfp128(1.5), "hfp128 1.0 < 1.5");
		expect(double(hfp128(1.5) * hfp128(1.5)) == 2.25, "hfp128 1.5 * 1.5 is 2.25");
		expect(double(hfp128(1.5) / hfp128(1.0)) == 1.5, "hfp128 1.5 / 1.0 is 1.5");
		expect(hfp128(1.0).isone() && hfp64(1.0).isone() && hfp32(1.0).isone(), "isone() at every width");
		expect(double(hfp128(1) / hfp128(3)) == 1.0 / 3.0, "hfp128 1/3 rounds to the double 1/3");
		// integers wider than a double's 53 bits are exact in hfp128
		expect(hfp128((1ll << 60) + 1) - hfp128(1ll << 60) == hfp128(1), "hfp128 2^60 + 1 is exact");
		// numeric_limits
		expect(std::numeric_limits<hfp32>::epsilon() == hfp32(std::ldexp(1.0, -20)), "hfp32 epsilon is 16^-5");
		expect(std::numeric_limits<hfp64>::epsilon() == hfp64(std::ldexp(1.0, -52)), "hfp64 epsilon is 16^-13");
		expect((hfp64(1) + std::numeric_limits<hfp64>::epsilon()) != hfp64(1), "1 + epsilon is not 1");
		expect(std::numeric_limits<hfp32>::digits == 6 && std::numeric_limits<hfp128>::digits == 28, "digits counts hex digits");
		// encodings with a zero leading hex digit (CodeRabbit, #2109): the exact sum of
		// 0x40000001 and 0xBD001001 is -2^-36, and 0x40000001 / 0x40100001 truncates to 0x3BFFFFF0
		{
			hfp32 a, b, c;
			a.setbits(0x40000001u);
			b.setbits(0xBD001001u);
			c.setbits(0x40100001u);
			expect(double(a + b) == -std::ldexp(1.0, -36), "unnormalized a + b is -2^-36");
			hfp32 r;
			r.setbits(0x3BFFFFF0u);
			expect(to_hex(a / c) == to_hex(r), "unnormalized a / c truncates to 0x3BFFFFF0");
		}
		// a fraction with zero leading digits converts at its own weight (CodeRabbit, #2109):
		// hfp128 setbits(1) is 2^-112 * 16^-64 = 2^-368, and came out 2^-320
		{
			hfp128 x;
			x.setbits(1u);
			expect(double(x) == std::ldexp(1.0, -368), "hfp128 setbits(1) -> double is 2^-368");
		}
		// a double result below 2^-1022 rounds once, at the subnormal precision
		{
			using H = hfloat<28, 10>;
			H x;
			x.assign_from_mantissa64(false, -1022, (std::uint64_t(1) << 63) | (std::uint64_t(1) << 11) | (std::uint64_t(1) << 6));
			expect(double(x) == std::ldexp(1.0, -1023) + std::ldexp(1.0, -1074), "hfloat<28,10> -> subnormal double rounds once");
		}
#if LONG_DOUBLE_SUPPORT
		// a long double wider than a double keeps its bits
		if (std::numeric_limits<long double>::digits > 53) {
			expect(hfp128(9007199254740993.0L) == hfp128(9007199254740993LL), "hfp128(long double 2^53 + 1) is exact");
		}
		// binary128 carries 113 bits: all of them reach hfp128's 112-bit fraction (CodeRabbit, #2109)
		if (std::numeric_limits<long double>::digits > 100) {
			const hfp128 x(1.0L + std::ldexp(1.0L, -100));
			expect(x - hfp128(1) == hfp128(std::ldexp(1.0, -100)), "hfp128(long double 1 + 2^-100) keeps 2^-100");
		}
#endif
#if LONG_DOUBLE_SUPPORT
		// a constant-evaluated long double conversion truncates like the run-time one (#2133): it went
		// through double, which rounds to nearest first, so 1 - 2^-60 became 1.0 at hfp64 instead of
		// 0.FFFFFFFFFFFFFF
		if constexpr (std::numeric_limits<long double>::digits > 56) {
			constexpr long double v = 1.0L - 1.0L / 1152921504606846976.0L;   // 1 - 2^-60
			constexpr hfp64 atCompileTime(v);
			volatile long double vr = v;
			const hfp64 atRunTime(static_cast<long double>(vr));
			expect(to_hex(atCompileTime) == to_hex(atRunTime), "constexpr hfp64(long double) matches run time");
			expect(to_hex(atCompileTime) == "+0x0.FFFFFFFFFFFFFF * 16^0", "constexpr hfp64(1 - 2^-60) truncates to 0.FFFFFFFFFFFFFF");
		}
#endif
		// the decimal exponents: min_exponent10 = ceil(log10(min())), max_exponent10 = floor(log10(max()))
		// (#2133: int(emin * 1.204) gave -77 for hfp32's -78, and -4 for hfloat<2,3>'s -6)
		{
			auto exp10ok = [](auto tag) {
				using H = decltype(tag);
				using L = std::numeric_limits<H>;
				// through double: without LONG_DOUBLE_SUPPORT hfloat has no long double conversion
				const double lmin = std::log10(static_cast<double>(L::min()));
				const double lmax = std::log10(static_cast<double>(L::max()));
				return L::min_exponent10 == static_cast<int>(std::ceil(lmin)) && L::max_exponent10 == static_cast<int>(std::floor(lmax));
			};
			expect(std::numeric_limits<hfp32>::min_exponent10 == -78 && std::numeric_limits<hfp32>::max_exponent10 == 75, "hfp32 exponent10 is -78 / 75");
			// max() = (15/16) 2^262140 at hfloat<1,17>: log10 is 78911.975, so 10^78912 exceeds it (#2133 review)
			expect(std::numeric_limits<hfloat<1, 17>>::max_exponent10 == 78911, "hfloat<1,17> max_exponent10 is 78911, below max()");
			// es = 1: emax is 0 and max() = 1 - 16^-ndigits < 1, at any width, also where 16^-ndigits
			// underflows a double (hfloat<269,1>: 2^-1076)
			expect(std::numeric_limits<hfloat<2, 1>>::max_exponent10 == -1 && std::numeric_limits<hfloat<269, 1>>::max_exponent10 == -1, "es = 1: max_exponent10 is -1");
			expect(exp10ok(hfp32{}) && exp10ok(hfp64{}) && exp10ok(hfp128{}) && exp10ok(hfloat<2, 3>{}) && exp10ok(hfloat<6, 4>{}), "exponent10 from log10 of min() and max()");
		}
		// setbits with a 64-bit block
		using H64 = hfloat<14, 7, std::uint64_t>;
		H64 x;
		x.setbits(0x4110000000000000ull);   // 0x0.1 * 16^1 = 1.0
		expect(double(x) == 1.0, "hfloat<14,7,uint64_t>::setbits");
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

	std::string test_suite  = "hfloat exact truncation (#2105)";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors

#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyRepros(reportTestCases), "hfloat", "sweep repros");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp32>(10, 2000, reportTestCases), "hfp32", "+ - * / against exact");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp64>(20, 2000, reportTestCases), "hfp64", "+ - * / against exact");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp128>(34, 1000, reportTestCases), "hfp128", "+ - * / against exact");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfloat<14, 7, std::uint64_t>>(20, 500, reportTestCases), "hfloat<14,7,u64>", "+ - * / against exact");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfloat<14, 7, std::uint8_t>>(20, 500, reportTestCases), "hfloat<14,7,u8>", "+ - * / against exact");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp32>(10, 2000, reportTestCases, true), "hfp32", "unnormalized operands");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp128>(34, 500, reportTestCases, true), "hfp128", "unnormalized operands");
	// the edges of the range: overflow saturates, underflow flushes to 0 (#2133)
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp32>(10, 2000, reportTestCases, false, hfp32::emax - 4), "hfp32", "near emax: saturation");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp32>(10, 2000, reportTestCases, false, hfp32::emin + 4), "hfp32", "near emin: flush to 0");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp64>(20, 1000, reportTestCases, false, hfp64::emax - 4), "hfp64", "near emax: saturation");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp64>(20, 1000, reportTestCases, false, hfp64::emin + 4), "hfp64", "near emin: flush to 0");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp128>(34, 500, reportTestCases, false, hfp128::emax - 4), "hfp128", "near emax: saturation");
	nrOfFailedTestCases += ReportTestResult(VerifyArithmetic<hfp128>(34, 500, reportTestCases, false, hfp128::emin + 4), "hfp128", "near emin: flush to 0");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
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
