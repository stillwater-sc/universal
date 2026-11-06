// rounding_oracle.cpp: posit add, sub, mul, div and conversion from double against the rounding rule
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

/*
   posit arithmetic decodes its operands into blocktriples and encodes the result with one
   rounding. Both used to move the fields one bit at a time, which made a 128-bit add cost 900 ns;
   since #1921 they work a limb at a time. The other posit tests reach wide posits only through
   double, so this test holds the arithmetic at every width to the rounding rule itself:

   y is the correctly rounded value of an exact result X != 0 when y has X's sign and
       mid(y-1, y) <= |X| <= mid(y, y+1),
   where the midpoints are the (nbits+1)-bit patterns 2y-1 and 2y+1, and a tie (equality) goes to
   the even encoding. Nothing rounds to zero or NaR: there is no lower midpoint for minpos and no
   upper one for maxpos. An exact zero gives zero.

   The oracle has its own decoder, reading the encoding through bits(), and computes X exactly on
   32-bit words: a + b and a * b directly, and a / b by comparing |a| with mid * |b|.
*/

namespace sw {
namespace universal {
namespace rounding_oracle {

// an unsigned integer on 32-bit words, least significant first, no leading zero words
struct Nat {
	std::vector<std::uint32_t> w;
	void trim() { while (!w.empty() && w.back() == 0u) w.pop_back(); }
	bool zero() const { return w.empty(); }
};
Nat shl(const Nat& a, unsigned s) {
	if (a.zero()) return a;
	Nat r;
	r.w.assign(s / 32u, 0u);
	const unsigned b = s % 32u;
	std::uint32_t carry = 0;
	for (std::uint32_t x : a.w) {
		r.w.push_back((x << b) | carry);
		carry = (b == 0u) ? 0u : (x >> (32u - b));
	}
	r.w.push_back(carry);
	r.trim();
	return r;
}
int cmp(const Nat& a, const Nat& b) {
	if (a.w.size() != b.w.size()) return a.w.size() < b.w.size() ? -1 : 1;
	for (std::size_t i = a.w.size(); i-- > 0;) {
		if (a.w[i] != b.w[i]) return a.w[i] < b.w[i] ? -1 : 1;
	}
	return 0;
}
Nat add(const Nat& a, const Nat& b) {
	Nat r;
	std::uint64_t carry = 0;
	for (std::size_t i = 0; i < std::max(a.w.size(), b.w.size()); ++i) {
		const std::uint64_t s = carry + (i < a.w.size() ? a.w[i] : 0u) + (i < b.w.size() ? b.w[i] : 0u);
		r.w.push_back(static_cast<std::uint32_t>(s));
		carry = s >> 32;
	}
	r.w.push_back(static_cast<std::uint32_t>(carry));
	r.trim();
	return r;
}
Nat sub(const Nat& a, const Nat& b) {  // a >= b
	Nat r;
	std::int64_t borrow = 0;
	for (std::size_t i = 0; i < a.w.size(); ++i) {
		std::int64_t d = std::int64_t(a.w[i]) - borrow - std::int64_t(i < b.w.size() ? b.w[i] : 0u);
		borrow = (d < 0) ? 1 : 0;
		if (d < 0) d += (std::int64_t(1) << 32);
		r.w.push_back(static_cast<std::uint32_t>(d));
	}
	r.trim();
	return r;
}
Nat mul(const Nat& a, const Nat& b) {
	Nat r;
	if (a.zero() || b.zero()) return r;
	r.w.assign(a.w.size() + b.w.size(), 0u);
	for (std::size_t i = 0; i < a.w.size(); ++i) {
		std::uint64_t carry = 0;
		for (std::size_t j = 0; j < b.w.size(); ++j) {
			const std::uint64_t t = std::uint64_t(a.w[i]) * b.w[j] + r.w[i + j] + carry;
			r.w[i + j] = static_cast<std::uint32_t>(t);
			carry = t >> 32;
		}
		r.w[i + b.w.size()] = static_cast<std::uint32_t>(carry);
	}
	r.trim();
	return r;
}

// the dyadic (-1)^neg * m * 2^e
struct Dyadic {
	bool neg{ false };
	Nat  m;
	int  e{ 0 };
};
// |x| compared with |y|
int cmpMagnitude(const Dyadic& x, const Dyadic& y) {
	if (x.m.zero() || y.m.zero()) return x.m.zero() ? (y.m.zero() ? 0 : -1) : 1;
	const int e = std::min(x.e, y.e);
	return cmp(shl(x.m, static_cast<unsigned>(x.e - e)), shl(y.m, static_cast<unsigned>(y.e - e)));
}
Dyadic addExact(const Dyadic& x, const Dyadic& y) {
	if (x.m.zero()) return y;
	if (y.m.zero()) return x;
	const int e = std::min(x.e, y.e);
	const Nat a = shl(x.m, static_cast<unsigned>(x.e - e)), b = shl(y.m, static_cast<unsigned>(y.e - e));
	Dyadic r;
	r.e = e;
	if (x.neg == y.neg) {
		r.neg = x.neg;
		r.m = add(a, b);
	}
	else if (cmp(a, b) >= 0) {
		r.neg = x.neg;
		r.m = sub(a, b);
	}
	else {
		r.neg = y.neg;
		r.m = sub(b, a);
	}
	return r;
}
Dyadic mulExact(const Dyadic& x, const Dyadic& y) {
	Dyadic r;
	r.neg = x.neg != y.neg;
	r.m = mul(x.m, y.m);
	r.e = x.e + y.e;
	return r;
}

// the value of a positive posit pattern of L bits, bits[i] = bit i, bit L-1 (the sign) is 0
Dyadic decodePattern(const std::vector<bool>& bits, unsigned es) {
	const int L = static_cast<int>(bits.size());
	int i = L - 2;
	const bool r0 = bits[i];
	int run = 0;
	while (i >= 0 && bits[i] == r0) { ++run; --i; }
	--i;  // the terminator, if any
	const int k = r0 ? run - 1 : -run;
	int exponent = 0;
	for (unsigned j = 0; j < es; ++j) {
		exponent = (exponent << 1) | ((i >= 0 && bits[i]) ? 1 : 0);
		--i;
	}
	Dyadic d;
	Nat f;
	f.w.push_back(1u);  // the hidden bit, then the fraction bits below it
	int nf = 0;
	for (; i >= 0; --i, ++nf) {
		f = shl(f, 1u);
		if (bits[i]) f = add(f, Nat{ { 1u } });
	}
	d.m = f;
	d.e = k * (1 << es) + exponent - nf;
	return d;
}

template<typename Posit>
std::vector<bool> patternOf(const Posit& p) {
	std::vector<bool> b(Posit::nbits);
	const auto raw = p.bits();
	for (unsigned i = 0; i < Posit::nbits; ++i) b[i] = raw.test(i);
	return b;
}
// two's complement of a pattern, in place
void negatePattern(std::vector<bool>& b) {
	bool carry = true;
	for (std::size_t i = 0; i < b.size(); ++i) {
		const bool x = !b[i];
		b[i] = x != carry;
		carry = x && carry;
	}
}
// the magnitude pattern of a nonzero, non-NaR posit, and its sign
template<typename Posit>
std::vector<bool> magnitudePattern(const Posit& p, bool& negative) {
	std::vector<bool> b = patternOf(p);
	negative = b.back();
	if (negative) negatePattern(b);
	return b;
}
template<typename Posit>
Dyadic valueOf(const Posit& p) {
	if (p.iszero()) return Dyadic{};
	bool negative = false;
	Dyadic d = decodePattern(magnitudePattern(p, negative), Posit::es);
	d.neg = negative;
	return d;
}
Dyadic valueOf(double x) {
	Dyadic d;
	if (x == 0.0) return d;
	d.neg = x < 0;
	int e = 0;
	const double m = std::frexp(std::fabs(x), &e);
	const std::uint64_t M = static_cast<std::uint64_t>(std::ldexp(m, 53));
	d.m.w = { static_cast<std::uint32_t>(M), static_cast<std::uint32_t>(M >> 32) };
	d.m.trim();
	d.e = e - 53;
	return d;
}

// the midpoints of a positive pattern y: lower = 2y-1 and upper = 2y+1, at nbits+1 bits
std::vector<bool> upperMid(const std::vector<bool>& y) {
	std::vector<bool> m(y.size() + 1);
	m[0] = true;
	for (std::size_t i = 0; i < y.size(); ++i) m[i + 1] = y[i];
	return m;
}
std::vector<bool> lowerMid(std::vector<bool> y) {
	for (std::size_t i = 0; i < y.size(); ++i) {  // y - 1
		if (y[i]) { y[i] = false; break; }
		y[i] = true;
	}
	return upperMid(y);
}

// is y the correctly rounded value of the exact result? cmpTo(m) returns the sign of |X| - m
template<typename Posit, typename Compare>
bool correctlyRounded(const Posit& y, bool exactZero, bool exactNegative, Compare cmpTo) {
	if (exactZero) return y.iszero();
	if (y.iszero() || y.isnar()) return false;
	bool negative = false;
	const std::vector<bool> Y = magnitudePattern(y, negative);
	if (negative != exactNegative) return false;
	bool isMinpos = Y[0], isMaxpos = true;   // minpos = 0.00..01, maxpos = 0.11..11
	for (unsigned i = 1; i < Posit::nbits; ++i) isMinpos = isMinpos && !Y[i];
	for (unsigned i = 0; i + 1 < Posit::nbits; ++i) isMaxpos = isMaxpos && Y[i];
	const bool even = !Y[0];
	if (!isMinpos) {
		const int c = cmpTo(decodePattern(lowerMid(Y), Posit::es));
		if (c < 0 || (c == 0 && !even)) return false;
	}
	if (!isMaxpos) {
		const int c = cmpTo(decodePattern(upperMid(Y), Posit::es));
		if (c > 0 || (c == 0 && !even)) return false;
	}
	return true;
}

struct Rng {  // splitmix64: reproducible on every platform
	std::uint64_t state;
	std::uint64_t next() {
		std::uint64_t z = (state += 0x9E3779B97F4A7C15ull);
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return z ^ (z >> 31);
	}
};
template<typename Posit>
void randomize(Posit& p, Rng& rng) {
	blockbinary<Posit::nbits, typename Posit::BlockType, BinaryNumberType::Signed> b;
	b.clear();
	std::uint64_t word = 0;
	for (unsigned i = 0; i < Posit::nbits; ++i) {
		if ((i % 64u) == 0) word = rng.next();
		b.setbit(i, ((word >> (i % 64u)) & 1ull) != 0);
	}
	p.setbits(b);
}

template<typename Posit>
int CheckOperands(const Posit& a, const Posit& b, bool reportTestCases) {
	int fails = 0;
	if (a.isnar() || b.isnar()) return 0;
	const Dyadic x = valueOf(a), y = valueOf(b);
	auto report = [&](const char* op, const Posit& r) {
		++fails;
		if (reportTestCases && fails < 4)
			std::cerr << "FAIL " << to_binary(a) << ' ' << op << ' ' << to_binary(b) << " gave " << to_binary(r) << '\n';
	};
	{
		const Dyadic s = addExact(x, y);
		const Posit r = a + b;
		if (!correctlyRounded(r, s.m.zero(), s.neg, [&](const Dyadic& m) { return cmpMagnitude(s, m); })) report("+", r);
	}
	{
		Dyadic ny = y;
		ny.neg = !ny.neg;
		const Dyadic s = addExact(x, ny);
		const Posit r = a - b;
		if (!correctlyRounded(r, s.m.zero(), s.neg, [&](const Dyadic& m) { return cmpMagnitude(s, m); })) report("-", r);
	}
	{
		const Dyadic s = mulExact(x, y);
		const Posit r = a * b;
		if (!correctlyRounded(r, s.m.zero(), s.neg, [&](const Dyadic& m) { return cmpMagnitude(s, m); })) report("*", r);
	}
	if (!b.iszero()) {
		const Posit r = a / b;
		// |a / b| against m is |a| against m * |b|
		if (!correctlyRounded(r, x.m.zero(), x.neg != y.neg, [&](const Dyadic& m) { return cmpMagnitude(x, mulExact(m, y)); })) report("/", r);
	}
	return fails;
}

// every pair of encodings
template<typename Posit>
int VerifyExhaustive(bool reportTestCases) {
	constexpr unsigned N = 1u << Posit::nbits;
	int fails = 0;
	Posit a, b;
	for (unsigned i = 0; i < N; ++i) {
		a.setbits(i);
		for (unsigned j = 0; j < N; ++j) {
			b.setbits(j);
			fails += CheckOperands(a, b, reportTestCases);
		}
	}
	return fails;
}

// random encodings; values near 1 with full fractions; and sums that land on a midpoint, just
// above it and just below it
template<typename Posit>
int VerifyRandom(int trials, bool reportTestCases) {
	Rng rng{ 0x1921ull * Posit::nbits + Posit::es * 17u + sizeof(typename Posit::BlockType) };
	int fails = 0;
	for (int t = 0; t < trials; ++t) {
		Posit a, b;
		randomize(a, rng);
		switch (t % 4) {
		case 0:
			randomize(b, rng);
			break;
		case 1: {
			// a near 1 with random low bits, b a random encoding or near 1
			const double d = std::ldexp(double(rng.next() >> 11) + 1.0, -52 + static_cast<int>(rng.next() % 5u) - 2);
			Posit low;
			randomize(low, rng);
			a = (rng.next() & 1u) ? d : -d;
			auto bits = a.bits();
			for (unsigned i = 0; i + 16u < Posit::nbits && i < 200u; ++i) bits.setbit(i, low.bits().test(i));
			a.setbits(bits);
			b = std::ldexp(double(rng.next() >> 11) + 1.0, -52 + static_cast<int>(rng.next() % 9u) - 4);
			break;
		}
		default: {
			// a near 1, b half an ulp of a times (1 + delta): the sum is at, just above or just below a midpoint
			a = 1.0 + std::ldexp(double(rng.next() >> 12), -52);
			const Dyadic v = valueOf(a);
			const double scale = (t % 4 == 2) ? 1.0 : ((rng.next() & 1u) ? 1.0 + 0x1.0p-20 : 1.0 - 0x1.0p-20);
			b = std::ldexp(scale, v.e - 1);   // v.e is the exponent of a's last fraction bit
			if (rng.next() & 1u) b = -b;
			break;
		}
		}
		fails += CheckOperands(a, b, reportTestCases);
		// conversion from double
		const double d = std::ldexp(double(rng.next() >> 11) + 1.0, static_cast<int>(rng.next() % 400u) - 252) * ((rng.next() & 1u) ? -1.0 : 1.0);
		const Posit r = d;
		const Dyadic x = valueOf(d);
		if (!correctlyRounded(r, false, x.neg, [&](const Dyadic& m) { return cmpMagnitude(x, m); })) {
			++fails;
			if (reportTestCases && fails < 4) std::cerr << "FAIL posit(" << d << ") gave " << to_binary(r) << '\n';
		}
	}
	return fails;
}

}  // namespace rounding_oracle
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
	using namespace sw::universal::rounding_oracle;

	std::string test_suite          = "posit arithmetic against the rounding rule";
	std::string test_tag            = "rounding";
	bool        reportTestCases     = true;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += VerifyRandom<posit<128, 2, std::uint64_t>>(1000, reportTestCases);
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore failures
#else

#	if REGRESSION_LEVEL_1
	// every pair of encodings
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<6, 0, std::uint8_t>>(reportTestCases), "posit<6,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<8, 0, std::uint8_t>>(reportTestCases), "posit<8,0>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustive<posit<8, 2, std::uint8_t>>(reportTestCases), "posit<8,2>", test_tag);

	// random operands, one limb to four
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<16, 1, std::uint16_t>>(2000, reportTestCases), "posit<16,1,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<32, 2, std::uint32_t>>(2000, reportTestCases), "posit<32,2,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<64, 2, std::uint64_t>>(2000, reportTestCases), "posit<64,2,uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<64, 3, std::uint8_t>>(2000, reportTestCases), "posit<64,3,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<65, 2, std::uint8_t>>(2000, reportTestCases), "posit<65,2,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<80, 3, std::uint16_t>>(2000, reportTestCases), "posit<80,3,uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<128, 2, std::uint64_t>>(2000, reportTestCases), "posit<128,2,uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<128, 4, std::uint8_t>>(2000, reportTestCases), "posit<128,4,uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<196, 2, std::uint32_t>>(1000, reportTestCases), "posit<196,2,uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRandom<posit<256, 2, std::uint64_t>>(1000, reportTestCases), "posit<256,2,uint64_t>", test_tag);
#	endif

#	if REGRESSION_LEVEL_2
#	endif

#	if REGRESSION_LEVEL_3
#	endif

#	if REGRESSION_LEVEL_4
#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
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
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
} catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
