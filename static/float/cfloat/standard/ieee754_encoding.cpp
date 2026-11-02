// ieee754_encoding.cpp: the IEEE-754 aliases of cfloat encode +-inf and NaN as IEEE-754 does (#1780, #1896)
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
// is maxpos and there is no inf (OCP E4M3FN). Without max-exponent values the fraction MSB, the
// quiet bit, tells a quiet NaN from a signalling one, as in IEEE-754 (#1896); with them, and when the
// fraction is a single bit, there are only two NaN patterns and the sign is the type.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <bit>
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
			// the quiet bit is the NaN type, whatever the sign (#1896)
			if (isNaN) {
				const bool quiet = (f & 0x200u) != 0;
				ok = ok && (h.isnan(NAN_TYPE_QUIET) == quiet) && (h.isnan(NAN_TYPE_SIGNALLING) == !quiet);
				ok = ok && ((-h).isnan(NAN_TYPE_QUIET) == quiet);   // negation does not change the type
				if (quiet) ok = ok && half(x).isnan(NAN_TYPE_QUIET);
			}
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
		auto native_bits = [](Native v) { return std::bit_cast<Bits>(v); };
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
		// NaN type is the quiet bit (#1896): the native NaNs keep their type in either direction, and
		// cfloat's own quiet NaN is the native quiet NaN pattern
		const Bits qnan = native_bits(std::numeric_limits<Native>::quiet_NaN());
		const Bits snan = native_bits(std::numeric_limits<Native>::signaling_NaN());
		c.setbits(qnan);
		if (!c.isnan(NAN_TYPE_QUIET)) fail("setbits(native qNaN) is not a quiet NaN");
		c.setbits(Bits(qnan | ninf));
		if (!c.isnan(NAN_TYPE_QUIET)) fail("setbits(native -qNaN) is not a quiet NaN");
		c.setbits(snan);
		if (!c.isnan(NAN_TYPE_SIGNALLING)) fail("setbits(native sNaN) is not a signalling NaN");
		c.setbits(Bits(snan | ninf));
		if (!c.isnan(NAN_TYPE_SIGNALLING)) fail("setbits(native -sNaN) is not a signalling NaN");
		if (!Cfloat(std::numeric_limits<Native>::quiet_NaN()).isnan(NAN_TYPE_QUIET)) fail("Cfloat(native qNaN) is not a quiet NaN");
		if (!Cfloat(-std::numeric_limits<Native>::quiet_NaN()).isnan(NAN_TYPE_QUIET)) fail("Cfloat(native -qNaN) is not a quiet NaN");
		Cfloat q;
		q.setnan(NAN_TYPE_QUIET);
		if (Bits(raw_bits(q)) != qnan) fail("setnan(quiet) is not the native quiet NaN pattern");
		if (!std::isnan(Native(q))) fail("Native(quiet NaN) is not NaN");
		q.setnan(NAN_TYPE_SIGNALLING);
		if (!q.isnan(NAN_TYPE_SIGNALLING) || (Bits(raw_bits(q)) & ninf) != pinf) fail("setnan(signalling) is not a positive signalling NaN");
		// to_native picks the native NaN of the same type: the quiet bit of the result tells
		const Bits nativeQuietBit = Bits(qnan & ~pinf);
		q.setnan(NAN_TYPE_QUIET);
		if ((native_bits(Native(q)) & nativeQuietBit) == 0) fail("Native(quiet NaN) is not a native quiet NaN");
#if UNIVERSAL_SNAN_ROUND_TRIPS_NATIVE_FP
		// only where the hardware does not quiet a signalling NaN on register contact
		q.setnan(NAN_TYPE_SIGNALLING);
		if ((native_bits(Native(q)) & nativeQuietBit) != 0) fail("Native(signalling NaN) is not a native signalling NaN");
#endif
		// invalid operations deliver a quiet NaN (IEEE-754 7.2)
		const Cfloat pi(inf), ni(-inf), zero(0);
		if (!(pi + ni).isnan(NAN_TYPE_QUIET)) fail("inf + -inf is not a quiet NaN");
		if (!(pi - pi).isnan(NAN_TYPE_QUIET)) fail("inf - inf is not a quiet NaN");
		if (!(zero * pi).isnan(NAN_TYPE_QUIET)) fail("0 * inf is not a quiet NaN");
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
		if (!p.isnan(NAN_TYPE_SIGNALLING) || p.isinf()) {
			if (reportTestCases) std::cerr << "FAIL: quad pattern 0x7FFF0000...1 is not a signalling NaN\n";
			++nrOfFailedTestCases;
		}
		// the quiet bit sits in the upper limb
		quad n;
		n.setnan(NAN_TYPE_QUIET);
		if (!(n.block(1) == 0x7FFF'8000'0000'0000ull && n.block(0) == 0) || !n.isnan(NAN_TYPE_QUIET)) {
			if (reportTestCases) std::cerr << "FAIL: quad quiet NaN is not 0x7FFF8000...0\n";
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
		e.setbits(0xFFu);
		if (!e.isnan(NAN_TYPE_SIGNALLING)) fail("cfloat<8,4,sub,max> 0xFF is not a signalling NaN");
		e.setnan(NAN_TYPE_QUIET);
		if (raw_bits(e) != 0x7Fu) fail("cfloat<8,4,sub,max> setnan(quiet) is not 0x7F");
		e.setnan(NAN_TYPE_SIGNALLING);
		if (raw_bits(e) != 0xFFu) fail("cfloat<8,4,sub,max> setnan(signalling) is not 0xFF");
		// the type survives a conversion between the two typings
		half hs;
		hs.setnan(NAN_TYPE_SIGNALLING);
		if (!E(hs).isnan(NAN_TYPE_SIGNALLING) || !half(E(hs)).isnan(NAN_TYPE_SIGNALLING)) fail("half sNaN -> cfloat<8,4,sub,max> -> half loses the type");
		hs.setnan(NAN_TYPE_QUIET);
		if (!E(hs).isnan(NAN_TYPE_QUIET) || !half(E(hs)).isnan(NAN_TYPE_QUIET)) fail("half qNaN -> cfloat<8,4,sub,max> -> half loses the type");
		// a one-bit fraction has no room for a signalling payload: the sign is the type
		using F1 = cfloat<4, 2, std::uint8_t, true, false, false>;
		F1 f1;
		f1.setbits(0x7u);
		if (!f1.isnan(NAN_TYPE_QUIET)) fail("cfloat<4,2,sub> 0b0111 is not a quiet NaN");
		f1.setbits(0xFu);
		if (!f1.isnan(NAN_TYPE_SIGNALLING)) fail("cfloat<4,2,sub> 0b1111 is not a signalling NaN");
		f1.setnan(NAN_TYPE_SIGNALLING);
		if (raw_bits(f1) != 0xFu) fail("cfloat<4,2,sub> setnan(signalling) is not 0b1111");
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

	// the four blocktriple normalizations carry the NaN type in the blocktriple's sign (set = signalling)
	template<typename Cfloat>
	int VerifyNormalizeNanType(bool reportTestCases, const std::string& type) {
		using bt = typename Cfloat::BlockType;
		constexpr unsigned fbits = Cfloat::fbits;
		int nrOfFailedTestCases = 0;
		for (int t : { NAN_TYPE_QUIET, NAN_TYPE_SIGNALLING }) {
			const bool signalling = (t == NAN_TYPE_SIGNALLING);
			Cfloat c;
			c.setnan(t);
			blocktriple<fbits, BlockTripleOperator::REP, bt> rep;
			blocktriple<fbits, BlockTripleOperator::ADD, bt> add;
			blocktriple<fbits, BlockTripleOperator::MUL, bt> mul;
			blocktriple<fbits, BlockTripleOperator::DIV, bt> div;
			c.normalize(rep);
			c.normalizeAddition(add);
			c.normalizeMultiplication(mul);
			c.normalizeDivision(div);
			const bool ok = rep.isnan() && rep.sign() == signalling && add.isnan() && add.sign() == signalling &&
				mul.isnan() && mul.sign() == signalling && div.isnan() && div.sign() == signalling;
			if (!ok) {
				if (reportTestCases) std::cerr << "FAIL: " << type << (signalling ? " signalling" : " quiet") << " NaN changes type in a normalization\n";
				++nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// ++ walks the raw encodings: the largest positive NaN steps to the all-ones pattern, not to a canonical NaN
	template<typename Cfloat>
	int VerifyIncrementPastNaN(bool reportTestCases, const std::string& type) {
		int nrOfFailedTestCases = 0;
		Cfloat c;
		c.setbits(0x7FFFu);
		++c;
		if (raw_bits(c) != 0xFFFFu) {
			if (reportTestCases) std::cerr << "FAIL: " << type << " ++0x7FFF is " << to_binary(c) << ", not 0xFFFF\n";
			++nrOfFailedTestCases;
		}
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
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementPastNaN<half>(reportTestCases, "half"), "half", "++ past NaN");
	nrOfFailedTestCases += ReportTestResult(VerifyNormalizeNanType<half>(reportTestCases, "half"), "half", "normalize NaN type");
	nrOfFailedTestCases += ReportTestResult(VerifyNormalizeNanType<cfloat<8, 4, std::uint8_t, true, true, false>>(reportTestCases, "cfloat<8,4,sub,max>"), "cfloat<8,4,sub,max>", "normalize NaN type");
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementPastNaN<cfloat<16, 5, std::uint8_t, true, false, false>>(reportTestCases, "cfloat<16,5,uint8_t>"), "cfloat<16,5,uint8_t>", "++ past NaN");
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
