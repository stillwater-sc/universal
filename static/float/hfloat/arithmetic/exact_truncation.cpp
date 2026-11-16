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

	// a normalized hfloat with the given sign, unbiased exponent and fraction bits from the generator
	template<typename H>
	H make(bool negative, int e, std::mt19937_64& rng) {
		Builder<H> b;
		b.clear();
		for (unsigned i = 0; i < H::fbits; ++i) b.put(i, (rng() & 1u) != 0);
		b.put(H::fbits - 1u - static_cast<unsigned>(rng() % 4u), true);   // a non-zero leading hex digit
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
	// Returns 0 when it does or when the exact value is outside the normal range (not checked here).
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
		if (er > H::emax || er < H::emin) return 0;
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

	template<typename H>
	int VerifyArithmetic(int maxGap, unsigned nrSamples, bool reportTestCases) {
		std::mt19937_64 rng(2105);
		int fails = 0;
		for (unsigned k = 0; k < nrSamples; ++k) {
			const int ea = static_cast<int>(rng() % 21u) - 10;
			const int eb = ea - static_cast<int>(rng() % static_cast<unsigned>(maxGap + 1)) * ((rng() & 1u) ? 1 : -1);
			const H a = make<H>((rng() & 1u) != 0, ea, rng);
			const H b = make<H>((rng() & 1u) != 0, eb, rng);
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
			// quotient: floor(Fa * 2^S / Fb) keeps fbits + 4 bits past the leading digit
			{
				constexpr int S = static_cast<int>(H::fbits) + 8;
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
