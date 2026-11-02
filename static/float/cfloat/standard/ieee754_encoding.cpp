// ieee754_encoding.cpp: the IEEE-754 aliases of cfloat encode +-inf and NaN as IEEE-754 does (#1780)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Without max-exponent values the all-ones exponent binade of a cfloat holds no values, and +-inf is
// IEEE-754's: all-ones exponent, zero fraction, the encoding right after maxpos. Every other pattern
// in that binade is a NaN. With max-exponent values the binade holds values, and +-inf is the pattern
// after maxpos at the top of it, 0b...11...10, below the NaN 0b...11...11; saturating, that pattern
// is maxpos and there is no inf (OCP E4M3FN). Quiet and signalling NaN are told apart by the sign.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// the encoding of a cfloat of up to 64 bits, assembled from its limbs
	template<typename Cfloat>
	std::uint64_t raw_bits(const Cfloat& c) {
		static_assert(Cfloat::nbits <= 64, "raw_bits is for encodings of up to 64 bits");
		std::uint64_t r{ 0 };
		for (unsigned i = 0; i < Cfloat::nrBlocks; ++i) r |= std::uint64_t(c.block(i)) << (i * Cfloat::bitsInBlock);
		return r;
	}

	// every binary16 pattern classifies, converts and round-trips as IEEE binary16 does
	int VerifyHalfExhaustive(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		for (unsigned bits = 0; bits < 65536u; ++bits) {
			half h;
			h.setbits(bits);
			const unsigned e = (bits >> 10) & 0x1Fu, f = bits & 0x3FFu;
			const bool sign = (bits & 0x8000u) != 0;
			const bool isInf = (e == 31 && f == 0), isNaN = (e == 31 && f != 0);
			// IEEE binary16 value of a finite pattern
			double ref = (e == 0) ? std::ldexp(double(f), -24) : std::ldexp(double(f | 0x400u), int(e) - 25);
			if (sign) ref = -ref;
			const float x = float(h);
			bool ok = (h.isinf() == isInf) && (h.isnan() == isNaN) && (std::isinf(x) == isInf) && (std::isnan(x) == isNaN);
			if (isInf) ok = ok && (std::signbit(x) == sign) && h.isinf(sign ? INF_TYPE_NEGATIVE : INF_TYPE_POSITIVE);
			if (!isInf && !isNaN) ok = ok && (double(x) == ref) && (raw_bits(half(x)) == bits);
			if (!ok) {
				if (reportTestCases && nrOfFailedTestCases < 10) std::cerr << "FAIL: half pattern " << to_binary(h) << " float " << x << '\n';
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// a cfloat with the native type's shape stores +-inf in the native bit pattern, and reads it back
	template<typename Cfloat, typename Native, typename Bits>
	int VerifyNativeSpecials(bool reportTestCases, const std::string& type) {
		int nrOfFailedTestCases = 0;
		auto fail = [&](const std::string& what) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << type << ' ' << what << '\n';
		};
		auto native_bits = [](Native v) { Bits b{}; std::memcpy(&b, &v, sizeof(Bits)); return b; };
		const Native inf = std::numeric_limits<Native>::infinity();
		const Bits pinf = native_bits(inf), ninf = native_bits(-inf);
		if (Bits(raw_bits(Cfloat(inf))) != pinf) fail("Cfloat(+inf) is not the native +inf pattern");
		if (Bits(raw_bits(Cfloat(-inf))) != ninf) fail("Cfloat(-inf) is not the native -inf pattern");
		if (Bits(raw_bits(std::numeric_limits<Cfloat>::infinity())) != pinf) fail("numeric_limits::infinity() is not the native +inf pattern");
		Cfloat c;
		c.setbits(pinf);
		if (!c.isinf(INF_TYPE_POSITIVE) || !std::isinf(Native(c)) || Native(c) < 0) fail("setbits(native +inf) is not +inf");
		c.setbits(ninf);
		if (!c.isinf(INF_TYPE_NEGATIVE) || !std::isinf(Native(c)) || Native(c) > 0) fail("setbits(native -inf) is not -inf");
		// the native quiet NaN, and the pattern below the all-ones fraction, are NaN
		c.setbits(native_bits(std::numeric_limits<Native>::quiet_NaN()));
		if (!c.isnan() || c.isinf() || !std::isnan(Native(c))) fail("setbits(native qNaN) is not NaN");
		c.setbits(Bits(pinf | Bits(1)));
		if (!c.isnan() || c.isinf()) fail("all-ones exponent with fraction 1 is not NaN");
		// overflow rounds to the native +inf pattern, and the encoding after maxpos is +inf
		Cfloat mp;
		mp.maxpos();
		if (Bits(raw_bits(mp)) + Bits(1) != pinf) fail("the encoding after maxpos is not +inf");
		Cfloat next = mp;
		++next;
		if (!next.isinf(INF_TYPE_POSITIVE)) fail("++maxpos is not +inf");
		if (!(mp + mp).isinf(INF_TYPE_POSITIVE)) fail("maxpos + maxpos is not +inf");
		if (Bits(raw_bits(Cfloat(std::numeric_limits<Native>::max()))) != native_bits(std::numeric_limits<Native>::max())) fail("native max is not the native max pattern");
		if (Bits(raw_bits(mp)) != native_bits(std::numeric_limits<Native>::max())) fail("maxpos is not the native max pattern");
		return nrOfFailedTestCases;
	}

	// a multi-limb cfloat with bfloat16's shape agrees with sw::universal::bfloat16 on all 2^16 patterns
	int VerifyBfloatShapeExhaustive(bool reportTestCases) {
		using Bf = cfloat<16, 8, std::uint8_t, true, false, false>;   // two uint8_t limbs: the exponent spans both
		int nrOfFailedTestCases = 0;
		for (unsigned bits = 0; bits < 65536u; ++bits) {
			Bf c;
			c.setbits(bits);
			bfloat16 b;
			b.setbits(static_cast<std::uint16_t>(bits));
			const float x = float(c), y = float(b);
			const bool ok = (c.isinf() == std::isinf(y)) && (c.isnan() == std::isnan(y)) &&
				(std::isnan(y) ? std::isnan(x) : (x == y && std::signbit(x) == std::signbit(y)));
			if (!ok) {
				if (reportTestCases && nrOfFailedTestCases < 10) std::cerr << "FAIL: cfloat<16,8,uint8_t> pattern " << to_binary(c) << " is " << x << ", bfloat16 " << y << '\n';
				++nrOfFailedTestCases;
			}
		}
		Bf inf(-std::numeric_limits<float>::infinity());
		if (raw_bits(inf) != 0xFF80u) {
			if (reportTestCases) std::cerr << "FAIL: cfloat<16,8,uint8_t>(-inf) is not 0xFF80\n";
			++nrOfFailedTestCases;
		}
		return nrOfFailedTestCases;
	}

	// quad spans two uint64_t limbs
	int VerifyQuadSpecials(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		quad q(-std::numeric_limits<double>::infinity());
		if (!(q.block(1) == 0xFFFF'0000'0000'0000ull && q.block(0) == 0)) {
			if (reportTestCases) std::cerr << "FAIL: quad(-inf) is not 0xFFFF0000...0\n";
			++nrOfFailedTestCases;
		}
		quad p;
		p.setblock(1, 0x7FFF'0000'0000'0000ull);
		p.setblock(0, 0);
		if (!p.isinf(INF_TYPE_POSITIVE) || !std::isinf(double(p))) {
			if (reportTestCases) std::cerr << "FAIL: quad pattern 0x7FFF0000...0 is not +inf\n";
			++nrOfFailedTestCases;
		}
		p.setblock(0, 1);
		if (!p.isnan() || p.isinf()) {
			if (reportTestCases) std::cerr << "FAIL: quad pattern 0x7FFF0000...1 is not NaN\n";
			++nrOfFailedTestCases;
		}
		return nrOfFailedTestCases;
	}

	// with max-exponent values the all-ones exponent binade holds values: inf stays 0b...11...10
	int VerifyMaxExpValueLayouts(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		auto fail = [&](const std::string& what) {
			++nrOfFailedTestCases;
			if (reportTestCases) std::cerr << "FAIL: " << what << '\n';
		};
		using E = cfloat<8, 4, std::uint8_t, true, true, false>;
		using S = cfloat<8, 4, std::uint8_t, true, true, true>;
		E e;
		e.setbits(0x78u);
		if (e.isinf() || e.isnan() || double(e) != 256.0) fail("cfloat<8,4,sub,max> 0x78 is not 256");
		e.setbits(0x7Eu);
		if (!e.isinf(INF_TYPE_POSITIVE)) fail("cfloat<8,4,sub,max> 0x7E is not +inf");
		e.setbits(0x7Fu);
		if (!e.isnan(NAN_TYPE_QUIET)) fail("cfloat<8,4,sub,max> 0x7F is not a quiet NaN");
		if (raw_bits(std::numeric_limits<E>::infinity()) != 0x7Eu) fail("cfloat<8,4,sub,max> infinity() is not 0x7E");
		S s;
		s.setbits(0x7Eu);
		if (s.isinf() || double(s) != 448.0) fail("cfloat<8,4,sub,max,sat> 0x7E is not 448 (OCP E4M3FN maxpos)");
		s.setbits(0x7Fu);
		if (!s.isnan()) fail("cfloat<8,4,sub,max,sat> 0x7F is not NaN");
		// without max-exponent values, saturating or not, inf is IEEE's
		using N = cfloat<8, 4, std::uint8_t, true, false, false>;
		using T = cfloat<8, 4, std::uint8_t, false, false, true>;
		if (raw_bits(std::numeric_limits<N>::infinity()) != 0x78u) fail("cfloat<8,4,sub> infinity() is not 0x78");
		N n;
		n.setbits(0x7Eu);
		if (!n.isnan() || n.isinf()) fail("cfloat<8,4,sub> 0x7E is not NaN");
		T t;
		t.setinf(true);
		if (raw_bits(t) != 0xF8u || !t.isinf(INF_TYPE_NEGATIVE)) fail("cfloat<8,4,sat> setinf(-) is not 0xF8");
		return nrOfFailedTestCases;
	}

} }

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

	std::string test_suite  = "cfloat IEEE-754 special encodings";
	std::string test_tag    = "ieee754 encoding";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyNativeSpecials<single, float, std::uint32_t>(reportTestCases, "single"), "single", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyHalfExhaustive(reportTestCases), "half", "all 2^16 patterns");
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSpecials<single, float, std::uint32_t>(reportTestCases, "single"), "single", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyNativeSpecials<duble, double, std::uint64_t>(reportTestCases, "duble"), "duble", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyBfloatShapeExhaustive(reportTestCases), "cfloat<16,8,uint8_t>", "vs bfloat16");
	nrOfFailedTestCases += ReportTestResult(VerifyQuadSpecials(reportTestCases), "quad", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyMaxExpValueLayouts(reportTestCases), "cfloat<8,4>", "max-exponent layouts");
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
