// long_double_conversion.cpp: conversion between long double and the Universal number types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/rational/rational.hpp>
#include <universal/number/einteger/einteger.hpp>
#include <universal/number/edecimal/edecimal.hpp>
#include <universal/number/erational/erational.hpp>
#include <universal/verification/test_suite.hpp>

/*
   Assignment from a long double was wrong on every platform whose long double is not x87 (#1515):
   on aarch64 Linux (IEEE binary128) 0.75 became 0.5 and 3 became 2, on POWER (IBM double-double)
   0.75 became 0, in cfloat, posit, fixpnt, lns, dbns, rational, einteger, edecimal and erational.
   extractFields() read those formats' bits with the x87 decoder. It now computes x87-shaped fields
   from the value there, the significand cut to 64 bits with the rest folded into the last bit
   (round to odd), so that a type rounding it to 62 bits or fewer rounds correctly. On POWER, lns
   and fixpnt also converted TO long double wrongly (every lns to 1): their starting weight was
   ieee754_parameter<long double>::minNormal, x87's 2^-16382, which double-double holds as 0.

   Checked here, on whatever long double the platform has:
   - every type: a value a long double and a double both hold assigns the same from either;
   - cfloat (float's and double's layout), posit and fixpnt: a long double one bit past a rounding
     tie, or at it, rounds as the tie rule says. The bit sits at the long double's last place, so
     on binary128 and double-double it lies below the 64-bit cut and reaches the type only through
     the round-to-odd bit; the expected results are exact values, built by construction;
   - a signalling long double NaN stays signalling in a cfloat. x87's qnanmask used to be double's
     pattern, which classified every x87 NaN with a payload as quiet;
   - fixpnt and lns values that are exact in a long double convert to it exactly.

   The 64 bits extractFields() hands out lose the rest of a binary128 or double-double significand
   in a type that keeps more (#1517). cfloat (63 or more significand bits), posit (nbits > 64),
   blocktriple, einteger, edecimal and erational now read all of it through
   long_double_significand. Checked here:
   - cfloat<128,15>, the binary128 layout, holds every sample long double exactly, compared with
     an encoding built from frexp: on x87 through that path too;
   - cfloat<80,15>, cfloat<96,15> and posit<96,2> round at ties whose sticky bit lies past 64 bits;
   - einteger, edecimal and erational keep 2^100 + 1 and 1 + 2^-100, and on double-double
     2^1000 + 1 and 1 + 2^-1000, whose two doubles are far apart;
   - blocktriple<112> keeps every bit, and blocktriple<62> rounds to nearest even at a tie.
   The second part of #1517 does the same for fixpnt (more than 62 bits), areal (a word at a time
   until the blocktriple is full, the ubit only for bits that do not fit), rational (its uint64_t core
   widened, which also removes the scale-64 cap and message of #1519 and fixes the sign of -1.0 and
   of a negative overflow) and lns (the log2's significand into a wide exponent). Checked here:
   fixpnt<128,64> and areal<128,15> exact past 64 bits, areal<80,15> truncated with the ubit,
   rational<128> exact and past scale 64, and the bits of lns<80,60>'s exponent.
*/

namespace sw {
namespace universal {

namespace long_double_conversion {

#if LONG_DOUBLE_SUPPORT

struct Failures {
	int  count = 0;
	bool report;
	explicit Failures(bool reportTestCases) : report(reportTestCases) {}
	void fail(const std::string& what) {
		if (report || count < 8)
			std::cerr << "FAIL: " << what << '\n';
		++count;
	}
};

inline std::string Hex(long double v) {
	char buf[64];
	std::snprintf(buf, sizeof(buf), "%La", v);
	return buf;
}

// a value both a long double and a double hold assigns the same from either; the adaptive types
// may reduce a fraction differently, so equal values count as the same
template<typename T>
void VerifyExact(const std::string& name, Failures& f) {
	for (double d : {0.75, 3.0, -2.5, 1024.5, 5.0, 12.0, -0.375, 100.0}) {
		T fromLong{}, fromDouble{};
		fromLong   = static_cast<long double>(d);
		fromDouble = d;
		if (!(fromLong == fromDouble) && double(fromLong) != double(fromDouble))
			f.fail(name + " = " + std::to_string(d) + "l gave " + std::to_string(double(fromLong)) + ", from the double " +
			       std::to_string(double(fromDouble)));
	}
}

inline int VerifyExactValues(bool reportTestCases) {
	Failures f(reportTestCases);
	VerifyExact<cfloat<32, 8, std::uint32_t, true, false, false>>("cfloat<32,8>", f);
	VerifyExact<cfloat<64, 11, std::uint64_t, true, false, false>>("cfloat<64,11>", f);
	VerifyExact<cfloat<128, 15, std::uint32_t, true, false, false>>("cfloat<128,15>", f);
	VerifyExact<posit<32, 2>>("posit<32,2>", f);
	VerifyExact<lns<16, 8, std::uint16_t>>("lns<16,8>", f);
	VerifyExact<dbns<8, 3, std::uint8_t>>("dbns<8,3>", f);
	VerifyExact<fixpnt<32, 16>>("fixpnt<32,16>", f);
	VerifyExact<takum<32>>("takum<32>", f);
	VerifyExact<areal<32, 8, std::uint32_t>>("areal<32,8>", f);
	VerifyExact<rational<32>>("rational<32>", f);
	VerifyExact<einteger<std::uint32_t>>("einteger", f);
	VerifyExact<edecimal>("edecimal", f);
	VerifyExact<erational>("erational", f);
	return f.count;
}

// Rounding a long double at a tie of a type whose unit in the last place near 1 is 2^-q. The
// sticky bit 2^t is the long double's last place, t = 2 - digits: past the 64-bit cut on binary128
// and double-double. The cases need t < -(q + 1), so that the sticky bit lies below the tie.
template<typename T>
void VerifyTies(const std::string& name, int q, Failures& f) {
	constexpr int digits = std::numeric_limits<long double>::digits;
	const int     t      = 2 - digits;
	if (t >= -(q + 1))
		return;  // the long double has no bit below this type's tie
	const long double ulp = std::ldexp(1.0l, -q), half = ulp / 2.0l, sticky = std::ldexp(1.0l, t);
	struct Case {
		long double x;
		int         ulps;  // the expected result, 1 + ulps * 2^-q
	};
	const Case cases[] = {
	    {1.0l + half, 0},                 // tie, even below
	    {1.0l + half + sticky, 1},        // just past the tie: only the sticky bit says so
	    {1.0l + half - sticky, 0},        // just short of it
	    {1.0l + ulp + half, 2},           // tie, even above
	    {1.0l + ulp + half - sticky, 1},  // just short of it
	    {1.0l + ulp + sticky, 1},         // far from a tie
	};
	for (const Case& c : cases) {
		for (int sign : {1, -1}) {
			// the expected value in the type's own arithmetic: 1 + 2 ulp need not fit a double
			T got{}, one{}, step{};
			got  = sign * c.x;
			one  = 1.0;
			step = std::ldexp(static_cast<double>(c.ulps), -q);  // exact in a double
			T expected = one + step;
			if (sign < 0) expected = -expected;
			if (!(got == expected))
				f.fail(name + " = " + Hex(sign * c.x) + " gave " + to_binary(got) + ", expected " + to_binary(expected));
		}
	}
}

inline int VerifyRounding(bool reportTestCases) {
	Failures f(reportTestCases);
	VerifyTies<cfloat<32, 8, std::uint32_t, true, false, false>>("cfloat<32,8>", 23, f);
	VerifyTies<cfloat<64, 11, std::uint64_t, true, false, false>>("cfloat<64,11>", 52, f);
	VerifyTies<posit<32, 2>>("posit<32,2>", 27, f);  // near 1: 32 - sign - regime 2 - es 2 = 27 fraction bits
	VerifyTies<fixpnt<32, 16>>("fixpnt<32,16>", 16, f);
	// wider than the 64-bit significand extractFields() hands out: binary128 and double-double
	// reach the sticky bit only through the full significand (#1517)
	VerifyTies<cfloat<80, 15, std::uint32_t, true, false, false>>("cfloat<80,15>", 64, f);
	VerifyTies<cfloat<96, 15, std::uint16_t, true, false, false>>("cfloat<96,15>", 80, f);
	VerifyTies<posit<96, 2>>("posit<96,2>", 91, f);  // near 1: 96 - sign - regime 2 - es 2 = 91 fraction bits
	VerifyTies<fixpnt<128, 64>>("fixpnt<128,64>", 64, f);
	return f.count;
}

// fixpnt, areal, rational and lns wider than the 64 bits extractFields() hands out: the second part
// of #1517. The expected values come from the types' own arithmetic or are set bit by bit.
inline int VerifyWideFixedAndRational(bool reportTestCases) {
	Failures f(reportTestCases);
	constexpr int digits = std::numeric_limits<long double>::digits;
	if (digits >= 101) {  // 2^40 + 2^-60 and 1 + 2^-100 are exact in the long double
		for (int sign : {1, -1}) {
			fixpnt<128, 64> got{}, big{}, tiny{};
			got  = sign * (std::ldexp(1.0l, 40) + std::ldexp(1.0l, -60));
			big  = std::ldexp(1.0, 40);
			tiny = std::ldexp(1.0, -60);
			fixpnt<128, 64> expected = big + tiny;
			if (sign < 0) expected = -expected;
			if (!(got == expected)) f.fail("fixpnt<128,64> = " + Hex(sign * (std::ldexp(1.0l, 40) + std::ldexp(1.0l, -60))) + " gave " + to_binary(got));
		}
		const long double x = 1.0l + std::ldexp(1.0l, -100);
		{
			// areal<128,15> holds it exactly: fraction bit 2^-100 sits at index fbits - 100 + 1
			using A = areal<128, 15, std::uint32_t>;
			A got{}, expected(1.0);
			got = x;
			expected.set(A::fbits - 100u + 1u, true);
			if (!(got == expected)) f.fail("areal<128,15> = 1 + 2^-100 gave " + to_binary(got) + ", expected " + to_binary(expected));
		}
		{
			// areal<80,15> has 64 fraction bits: truncated to 1, with the ubit set
			using A = areal<80, 15, std::uint32_t>;
			A got{}, expected(1.0);
			got = x;
			expected.set(0, true);
			if (!(got == expected)) f.fail("areal<80,15> = 1 + 2^-100 gave " + to_binary(got) + ", expected (1, next)");
		}
		{
			rational<128> got, one, tiny;
			got  = x;
			one  = 1.0;
			tiny = std::ldexp(1.0, -100);
			rational<128> expected = one + tiny;
			if (!(got == expected)) f.fail("rational<128> = 1 + 2^-100 gave " + to_binary(got));
		}
	}
	// rational from a double past scale 64, which saturated or went to zero with a message (#1519)
	for (int k : {60, 64, 70, 100, 120}) {
		for (double v : {std::ldexp(1.0, k), std::ldexp(3.0, k - 2), std::ldexp(1.0, -k), -std::ldexp(3.0, -k)}) {
			rational<128> r;
			r = v;
			if (double(r) != v) f.fail("rational<128> = " + Hex(static_cast<long double>(v)) + " gave " + std::to_string(double(r)));
		}
	}
	{
		rational<8> a, b;
		a = -1.0;     // used to give +1
		b = -1000.0;  // used to saturate to maxpos
		rational<8> maxneg;
		maxneg.maxneg();
		if (double(a) != -1.0) f.fail("rational<8> = -1.0 gave " + std::to_string(double(a)));
		if (!(b == maxneg)) f.fail("rational<8> = -1000.0 gave " + to_binary(b) + ", expected maxneg");
		rational<16> c;
		c = std::ldexp(1.0, -1030);  // a subnormal double: used to be left unconverted
		if (!c.iszero()) f.fail("rational<16> = 2^-1030 gave " + to_binary(c) + ", expected 0");
	}
	return f.count;
}

// lns<80,60>: the exponent is round(log2(v) * 2^60) in two's complement, 79 bits: with the 7 integer
// bits of log2(1e30) that is 67 significant bits, past the 64 extractFields() gives. The expected
// bits come from the long double log2 itself, scaled by 2^60 and rounded with rint, and are read
// with exact long double arithmetic: floor(E / 2^i) mod 2. Only values above 1, whose log is
// positive. (lns needs rbits < 64.)
inline int VerifyWideLns(bool reportTestCases) {
	using L = lns<80, 60, std::uint32_t>;
	Failures f(reportTestCases);
	for (long double v : {1.5l, 3.0l, 10.0l, 1.0e30l, 1.0l + std::ldexp(1.0l, -40)}) {
		const long double E = std::rint(std::ldexp(std::log2(v), 60));
		for (int sign : {1, -1}) {
			L got;
			got = sign * v;
			bool ok = (got.at(79) == (sign < 0));
			for (unsigned i = 0; i < 79 && ok; ++i) {
				const bool expected = std::fmod(std::floor(std::ldexp(E, -static_cast<int>(i))), 2.0l) != 0.0l;
				ok = (got.at(i) == expected);
			}
			if (!ok) f.fail("lns<80,60> = " + Hex(sign * v) + " gave " + to_binary(got));
		}
	}
	return f.count;
}

// the cfloat<128,15> encoding of a long double, built bit by bit from frexp rather than with the
// conversion under test: sign, biased exponent, the 112 fraction bits. Exact: cfloat<128,15> with
// subnormals is IEEE binary128, which holds every x87 and binary128 long double, and a
// double-double whose two doubles are within 112 bits of each other
using Binary128 = cfloat<128, 15, std::uint32_t, true, false, false>;

inline Binary128 Encode128(long double x) {
	constexpr int fbits = 112, bias = 16383;
	Binary128     c;
	c.clear();
	c.setbit(127, std::signbit(x));
	int e = 0;
	std::frexp(std::fabs(x), &e);  // |x| in [2^(e-1), 2^e)
	const int   biased = e - 1 + bias;
	long double t      = (biased >= 1) ? std::ldexp(std::fabs(x), 1 - e) - 1.0l  // the fraction of 1.f
	                                   : std::ldexp(std::fabs(x), bias - 1);     // subnormal: 0.f * 2^(1 - bias)
	const int   field  = (biased >= 1) ? biased : 0;
	for (int i = 0; i < 15; ++i)
		c.setbit(unsigned(fbits + i), ((field >> i) & 1) != 0);
	for (int i = fbits - 1; i >= 0; --i) {
		t *= 2.0l;
		const bool bit = (t >= 1.0l);
		c.setbit(unsigned(i), bit);
		if (bit)
			t -= 1.0l;
	}
	return c;
}

// every long double of up to 112 significant bits assigns to cfloat<128,15> exactly: on x87 (64
// bits) through the new wide path too, on binary128 and double-double past the 64-bit cut
inline int VerifyBinary128(bool reportTestCases) {
	using limit = std::numeric_limits<long double>;
	Failures                 f(reportTestCases);
	std::vector<long double> xs = {0.75l,
	                               1.0l / 3.0l,
	                               0.1l,
	                               1.0l + std::ldexp(1.0l, 2 - limit::digits),  // the last place
	                               std::ldexp(1.0l, 100) + 1.0l,
	                               limit::min(),
	                               limit::min() * 3.0l,
	                               limit::min() / 2.0l};
	if (limit::digits <= 112) xs.push_back(limit::max());  // a quad's 113th bit does not fit
	if (std::ilogb(limit::denorm_min()) >= -16494) xs.push_back(limit::denorm_min());
	for (long double x : xs) {
		for (long double y : {x, -x}) {
			Binary128 got;
			got = y;
			const Binary128 expected = Encode128(y);
			if (!(got == expected))
				f.fail("cfloat<128,15> = " + Hex(y) + " gave " + to_binary(got) + ", expected " + to_binary(expected));
		}
	}
	return f.count;
}

// the exact types keep every bit, however many words the significand takes: 2^100 + 1 and
// 1 + 2^-100 on binary128 and double-double, and on a double-double also values whose two
// doubles are far apart (1 + 2^-1000 spans 1001 bits). The expected values are built with the
// types' own exact arithmetic from doubles.
inline int VerifyExactTypes(bool reportTestCases) {
	Failures f(reportTestCases);
	auto representable = [](long double big, long double small) { return (big + small) != big; };
	for (int k : {100, 1000}) {
		const long double big = std::ldexp(1.0l, k);
		if (!std::isfinite(big) || !representable(big, 1.0l)) continue;
		for (int sign : {1, -1}) {
			const long double x = sign * (big + 1.0l);
			einteger ei, eexp;
			ei   = x;
			eexp = std::ldexp(1.0, k);
			eexp += einteger(1);
			if (sign < 0) eexp.setsign(true);
			if (!(ei == eexp)) f.fail("einteger = " + Hex(x) + " is not exact");
			edecimal ed, dexp;
			ed   = x;
			dexp = std::ldexp(1.0, k);
			dexp += edecimal(1);
			if (sign < 0) dexp = -dexp;
			if (!(ed == dexp)) f.fail("edecimal = " + Hex(x) + " is not exact");
		}
		const long double tiny = std::ldexp(1.0l, -k);
		if (!representable(1.0l, tiny)) continue;
		for (int sign : {1, -1}) {
			const long double x = sign * (1.0l + tiny);
			erational er, one(1.0), t;
			er = x;
			t  = std::ldexp(1.0, -k);
			erational expected = one + t;
			if (sign < 0) expected = -expected;
			if (!(er == expected)) f.fail("erational = " + Hex(x) + " is not exact");
		}
	}
	// truncation toward zero keeps the integer part only
	if (representable(std::ldexp(1.0l, 100), 3.75l)) {
		einteger ei, eexp;
		ei   = -(std::ldexp(1.0l, 100) + 3.75l);
		eexp = std::ldexp(1.0, 100);
		eexp += einteger(3);
		eexp.setsign(true);
		if (!(ei == eexp)) f.fail("einteger = -(2^100 + 3.75) is not -(2^100 + 3)");
	}
	return f.count;
}

// blocktriple from a long double: every bit kept at fbits = 112, and rounded to nearest even at
// fbits = 62 with the sticky bit past the 64-bit cut
inline int VerifyBlocktriple(bool reportTestCases) {
	Failures f(reportTestCases);
	constexpr int digits = std::numeric_limits<long double>::digits;
	{
		using B = blocktriple<112, BlockTripleOperator::REP, std::uint32_t>;
		for (long double x : {1.0l / 3.0l, 0.1l, -0.75l, std::ldexp(1.0l, 100) + 1.0l}) {
			if (digits > 112) continue;
			B b;
			b = x;
			int e = 0;
			long double t = std::frexp(std::fabs(x), &e) * 2.0l - 1.0l;  // the fraction of 1.f
			bool ok = (b.sign() == std::signbit(x)) && (b.scale() == e - 1) && b.significand().test(112);
			for (int i = 111; i >= 0 && ok; --i) {
				t *= 2.0l;
				const bool bit = (t >= 1.0l);
				if (bit) t -= 1.0l;
				ok = (b.significand().test(unsigned(i)) == bit);
			}
			if (!ok) f.fail("blocktriple<112> = " + Hex(x) + " is " + to_binary(b));
		}
	}
	if (2 - digits < -64) {  // a sticky bit below blocktriple<62>'s tie
		using B = blocktriple<62, BlockTripleOperator::REP, std::uint32_t>;
		const long double half = std::ldexp(1.0l, -63), sticky = std::ldexp(1.0l, 2 - digits);
		struct Case {
			long double x;
			bool        up;
		};
		for (const Case& c : {Case{1.0l + half, false}, Case{1.0l + half + sticky, true}, Case{1.0l + half - sticky, false}}) {
			B b;
			b = c.x;
			const bool ok = b.significand().test(62) && (b.significand().test(0) == c.up) && b.scale() == 0;
			if (!ok) f.fail("blocktriple<62> = " + Hex(c.x) + " is " + to_binary(b) + (c.up ? ", expected 1 + ulp" : ", expected 1"));
		}
	}
	return f.count;
}

// Rounding at the bottom of the range, at the subnormal tie of cfloat<64,15>: minpos 2^-16430, the
// tie 2^-16431. The sticky bit is the long double's denorm_min: 2^-16445 on x87, 2^-16494 on
// binary128, where it lies below x87's range and reaches the cfloat only through the round-to-odd
// bit that extractFields() folds every such value into. That bit is at x87's 2^-16445, far below
// this tie, so the rounding is still correct; only a target resolving 2^-16445 itself would see
// the difference (#1517). Needs a long double reaching 2^-16433 (not double-double).
inline int VerifyDeepSubnormals(bool reportTestCases) {
	using C     = cfloat<64, 15, std::uint32_t, true, false, false>;
	using limit = std::numeric_limits<long double>;
	Failures f(reportTestCases);
	if (limit::min_exponent > -16381)
		return 0;
	const long double minpos = std::ldexp(1.0l, -16430), tie = std::ldexp(1.0l, -16431), sticky = limit::denorm_min();
	C                 cminpos, czero;
	cminpos.minpos();
	czero.setzero();
	struct Case {
		long double x;
		C           expected;
	};
	const Case cases[] = {
	    {minpos, cminpos},                  // exact
	    {tie, czero},                       // tie, even below
	    {tie + sticky, cminpos},            // just past the tie: only the sticky bit says so
	    {std::ldexp(3.0l, -16432), cminpos},  // 0.75 minpos
	    {sticky, czero},                    // denorm_min: far below half minpos
	    {std::ldexp(1.0l, -16440), czero},  // below half minpos
	};
	for (const Case& c : cases) {
		for (int sign : {1, -1}) {
			C got{}, expected = c.expected;
			got = sign * c.x;
			if (sign < 0) expected.setsign(!expected.sign());
			if (!(got == expected))
				f.fail("cfloat<64,15> = " + Hex(sign * c.x) + " gave " + to_binary(got) + ", expected " + to_binary(expected));
		}
	}
	return f.count;
}

// a signalling NaN stays signalling, a quiet one quiet; inf stays inf
inline int VerifySpecials(bool reportTestCases) {
	using C = cfloat<32, 8, std::uint32_t, true, false, false>;
	Failures f(reportTestCases);
	C        q, s, i, n;
	q = std::numeric_limits<long double>::quiet_NaN();
	s = std::numeric_limits<long double>::signaling_NaN();
	i = std::numeric_limits<long double>::infinity();
	n = -std::numeric_limits<long double>::infinity();
	if (!q.isnan(NAN_TYPE_QUIET)) f.fail("cfloat = quiet NaN long double is " + to_binary(q));
	if (!s.isnan(NAN_TYPE_SIGNALLING)) f.fail("cfloat = signalling NaN long double is " + to_binary(s));
	if (!i.isinf(INF_TYPE_POSITIVE)) f.fail("cfloat = +inf long double is " + to_binary(i));
	if (!n.isinf(INF_TYPE_NEGATIVE)) f.fail("cfloat = -inf long double is " + to_binary(n));
	return f.count;
}

// values a long double holds exactly convert to it exactly
inline int VerifyToLongDouble(bool reportTestCases) {
	Failures f(reportTestCases);
	for (double d : {0.75, -2.5, 1024.5, 0.0625}) {
		fixpnt<32, 16> x(d);
		if (static_cast<long double>(x) != static_cast<long double>(d))
			f.fail("long double(fixpnt<32,16>(" + std::to_string(d) + ")) is " + Hex(static_cast<long double>(x)));
	}
	for (double d : {0.5, 4.0, -8.0, 0.25}) {  // powers of two: exact in the log domain
		lns<16, 8, std::uint16_t> x(d);
		if (static_cast<long double>(x) != static_cast<long double>(d))
			f.fail("long double(lns<16,8>(" + std::to_string(d) + ")) is " + Hex(static_cast<long double>(x)));
	}
	return f.count;
}

#endif  // LONG_DOUBLE_SUPPORT

}  // namespace long_double_conversion

}  // namespace universal
}  // namespace sw

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
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

	std::string test_suite          = "conversion between long double and Universal number types";
	std::string test_tag            = "long double";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if LONG_DOUBLE_SUPPORT
	using namespace sw::universal::long_double_conversion;
	std::cout << "long double: " << std::numeric_limits<long double>::digits << " significand bits\n";

#	if MANUAL_TESTING

	cfloat<32, 8, std::uint32_t, true, false, false> c;
	c = 0.75l;
	std::cout << "cfloat<32,8> = 0.75l : " << c << '\n';
	nrOfFailedTestCases += ReportTestResult(VerifyRounding(true), "rounding", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#	else

#		if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyExactValues(reportTestCases), "exact values", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRounding(reportTestCases), "rounding at ties", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyDeepSubnormals(reportTestCases), "deep subnormals", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyBinary128(reportTestCases), "cfloat<128,15> exact", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExactTypes(reportTestCases), "exact types", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyBlocktriple(reportTestCases), "blocktriple", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWideFixedAndRational(reportTestCases), "fixpnt, areal, rational", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWideLns(reportTestCases), "lns<80,60>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifySpecials(reportTestCases), "nan and inf", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToLongDouble(reportTestCases), "to long double", test_tag);
#		endif

#		if REGRESSION_LEVEL_2
#		endif

#		if REGRESSION_LEVEL_3
#		endif

#		if REGRESSION_LEVEL_4
#		endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#	endif  // MANUAL_TESTING
#else
	std::cout << test_suite << ": LONG_DOUBLE_SUPPORT is 0, nothing to test\n";
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#endif  // LONG_DOUBLE_SUPPORT
} catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
