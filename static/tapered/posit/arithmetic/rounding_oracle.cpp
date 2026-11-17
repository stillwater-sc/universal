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
#include <universal/verification/posit_rounding_oracle.hpp>

/*
   posit arithmetic decodes its operands into blocktriples and encodes the result with one
   rounding. Both used to move the fields one bit at a time, which made a 128-bit add cost 900 ns;
   since #1921 they work a limb at a time. This test holds the arithmetic at every width to the
   rounding rule itself, through the oracle in posit_rounding_oracle.hpp: its own decoder, exact
   results on 32-bit words, and the (nbits+1)-bit midpoint patterns 2y-1 and 2y+1 of a result y.
   The same oracle is the exact reference of the random-operand verifiers (#2121).
*/

namespace sw {
namespace universal {
namespace rounding_oracle {

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
