// increment_decrement.cpp : prefix and postfix ++ and -- of integer<>, alone and inside expressions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

/*
   Postfix a++ and a-- return a copy of the old value and change a, so an expression such as
   c = a++ + b adds the old a, while prefix ++a and --a change a and return a reference to
   it. Both forms rest on the prefix operators, which since #1500 add and subtract 1 on the
   limbs instead of through a temporary integer(1); the postfix forms still copy the old
   value, as they must.

   This pins the semantics for all three number types and every limb type:
   - the value each form yields and the value it leaves in the variable, alone and as an
     operand of +, - and *, on the left and on the right;
   - carry and borrow across limbs, and wraparound at both ends of the encoding;
   - widths above 64 bits, up to elreal's integer<256, uint32_t> exponent, where the
     expected values come from operator+= instead of a native integer.
   Up to 64 bits the reference is native 64-bit arithmetic modulo 2^nbits, which is the same
   bit arithmetic for IntegerNumber, WholeNumber and NaturalNumber.
*/

namespace sw { namespace universal {

namespace increment_decrement {

inline const char* Name(IntegerNumberType t) {
	switch (t) {
	case IntegerNumberType::IntegerNumber: return "IntegerNumber";
	case IntegerNumberType::WholeNumber:   return "WholeNumber";
	default:                               return "NaturalNumber";
	}
}

template<typename Integer>
std::uint64_t Bits(const Integer& v) {
	std::uint64_t u = 0;
	for (unsigned i = 0; i < Integer::nrBlocks && i * Integer::bitsInBlock < 64; ++i) {
		u |= static_cast<std::uint64_t>(v.block(i)) << (i * Integer::bitsInBlock);
	}
	return u;
}

// a failure report, printed for the first few failures of a configuration
struct Failures {
	int count = 0;
	bool report;
	explicit Failures(bool reportTestCases) : report(reportTestCases) {}
	void operator()(const std::string& what, std::uint64_t got, std::uint64_t expected) {
		if (report || count < 8) std::cerr << "FAIL: " << what << " = " << got << ", expected " << expected << '\n';
		++count;
	}
};

// every form of ++ and -- on the value u, and as an operand with w
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
void VerifyValue(std::uint64_t u, std::uint64_t w, Failures& fail) {
	using Integer = integer<nbits, BlockType, NumberType>;
	constexpr std::uint64_t mask = (nbits == 64) ? ~std::uint64_t(0) : ((std::uint64_t(1) << (nbits % 64)) - 1u);
	auto wrap = [](std::uint64_t x) { return x & mask; };
	const std::string cfg = std::string("integer<") + std::to_string(nbits) + ", " +
	                        std::to_string(sizeof(BlockType) * 8) + "-bit limbs, " + Name(NumberType) + ">";
	auto tag = [&](const char* expr) {
		return cfg + " a=" + std::to_string(u) + " b=" + std::to_string(w) + ": " + expr;
	};

	Integer v, b;
	v.setbits(u);
	b.setbits(w);
	auto expect = [&](const char* what, const Integer& got, std::uint64_t want) {
		if (Bits(got) != want) fail(tag(what), Bits(got), want);
	};

	// alone: what the expression yields, and what it leaves in a
	{ Integer a(v); Integer r = a++; expect("r = a++ -> r", r, u);           expect("r = a++ -> a", a, wrap(u + 1)); }
	{ Integer a(v); Integer r = a--; expect("r = a-- -> r", r, u);           expect("r = a-- -> a", a, wrap(u - 1)); }
	{ Integer a(v); Integer r = ++a; expect("r = ++a -> r", r, wrap(u + 1)); expect("r = ++a -> a", a, wrap(u + 1)); }
	{ Integer a(v); Integer r = --a; expect("r = --a -> r", r, wrap(u - 1)); expect("r = --a -> a", a, wrap(u - 1)); }
	// prefix returns the variable itself, so it can be incremented again in place
	{ Integer a(v); ++(++a); expect("++(++a) -> a", a, wrap(u + 2)); }
	{ Integer a(v); --(--a); expect("--(--a) -> a", a, wrap(u - 2)); }
	{
		Integer a(v);
		if (&(++a) != &a) fail(tag("&(++a) == &a"), 0, 1);
		if (&(--a) != &a) fail(tag("&(--a) == &a"), 0, 1);
	}

	// as an operand: postfix contributes the old value, prefix the new one
	{
		Integer a(v);
		Integer c = a++ + b;
		expect("c = a++ + b -> c", c, wrap(u + w));
		expect("c = a++ + b -> a", a, wrap(u + 1));
	}
	{
		Integer a(v);
		Integer c = b + a++;
		expect("c = b + a++ -> c", c, wrap(w + u));
		expect("c = b + a++ -> a", a, wrap(u + 1));
	}
	{
		Integer a(v);
		Integer c = a-- - b;
		expect("c = a-- - b -> c", c, wrap(u - w));
		expect("c = a-- - b -> a", a, wrap(u - 1));
	}
	{
		Integer a(v);
		Integer c = b - a--;
		expect("c = b - a-- -> c", c, wrap(w - u));
		expect("c = b - a-- -> a", a, wrap(u - 1));
	}
	{
		Integer a(v);
		Integer c = ++a + b;
		expect("c = ++a + b -> c", c, wrap(u + 1 + w));
		expect("c = ++a + b -> a", a, wrap(u + 1));
	}
	{
		Integer a(v);
		Integer c = --a - b;
		expect("c = --a - b -> c", c, wrap(u - 1 - w));
		expect("c = --a - b -> a", a, wrap(u - 1));
	}
	if constexpr (NumberType == IntegerNumberType::IntegerNumber) {
		// operator*= multiplies signed operands; the product modulo 2^nbits is the native one
		{
			Integer a(v);
			Integer c = a++ * b;
			expect("c = a++ * b -> c", c, wrap(u * w));
			expect("c = a++ * b -> a", a, wrap(u + 1));
		}
		{ Integer a(v); Integer c = ++a * b; expect("c = ++a * b -> c", c, wrap((u + 1) * w)); }
	}

	// sequences: two postfix increments, and three up then three down
	{
		Integer a(v);
		Integer x = a++;
		x = a++;
		expect("x = a++; x = a++ -> x", x, wrap(u + 1));
		expect("x = a++; x = a++ -> a", a, wrap(u + 2));
	}
	{
		Integer a(v);
		a++; ++a; a++;
		a--; --a; a--;
		expect("three up, three down -> a", a, u);
	}
}

// every value (and pair, up to 8 bits) when exhaustive, else random ones, plus the limb boundaries
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
int VerifyConfiguration(bool exhaustive, unsigned samples, bool reportTestCases) {
	Failures fail(reportTestCases);
	constexpr std::uint64_t mask = (nbits == 64) ? ~std::uint64_t(0) : ((std::uint64_t(1) << (nbits % 64)) - 1u);
	constexpr unsigned bitsInBlock = sizeof(BlockType) * 8;
	if constexpr (nbits <= 16) {
		if (exhaustive) {
			for (std::uint64_t a = 0; a <= mask; ++a) {
				if constexpr (nbits <= 8) {
					for (std::uint64_t b = 0; b <= mask; ++b) VerifyValue<nbits, BlockType, NumberType>(a, b, fail);
				}
				else {
					VerifyValue<nbits, BlockType, NumberType>(a, (a * 2654435761u + 12345u) & mask, fail);
				}
			}
			return fail.count;
		}
	}
	// the values either side of every limb boundary, where ++ carries and -- borrows
	std::mt19937_64 rng(1501u + nbits);
	for (unsigned k = bitsInBlock; k < nbits; k += bitsInBlock) {
		const std::uint64_t edge = std::uint64_t(1) << k;
		for (std::uint64_t u : {edge - 1u, edge, edge + 1u, (edge << 1) - 1u})
			VerifyValue<nbits, BlockType, NumberType>(u & mask, rng() & mask, fail);
	}
	for (std::uint64_t u : {std::uint64_t(0), std::uint64_t(1), mask, mask - 1u, std::uint64_t(1) << (nbits - 1),
	                        (std::uint64_t(1) << (nbits - 1)) - 1u})
		VerifyValue<nbits, BlockType, NumberType>(u, rng() & mask, fail);
	for (unsigned k = 0; k < samples; ++k) VerifyValue<nbits, BlockType, NumberType>(rng() & mask, rng() & mask, fail);
	return fail.count;
}

template<unsigned nbits, typename BlockType>
int VerifyAllTypes(bool exhaustive, unsigned samples, bool reportTestCases) {
	return VerifyConfiguration<nbits, BlockType, IntegerNumberType::IntegerNumber>(exhaustive, samples,
	                                                                               reportTestCases) +
	       VerifyConfiguration<nbits, BlockType, IntegerNumberType::WholeNumber>(exhaustive, samples, reportTestCases) +
	       VerifyConfiguration<nbits, BlockType, IntegerNumberType::NaturalNumber>(exhaustive, samples,
	                                                                               reportTestCases);
}

// above 64 bits: ++ must equal a + 1 and -- must equal a + (all ones), both through operator+=,
// across the carry chain of every limb; postfix must still return the old value
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
int VerifyWide(bool reportTestCases) {
	using Integer = integer<nbits, BlockType, NumberType>;
	constexpr unsigned bitsInBlock = sizeof(BlockType) * 8;
	Failures fail(reportTestCases);
	const std::string cfg = std::string("integer<") + std::to_string(nbits) + ", " + std::to_string(bitsInBlock) +
	                        "-bit limbs, " + Name(NumberType) + ">";
	Integer one, minusOne, zero;
	one.setbits(1);
	zero.setbits(0);
	minusOne.setbits(0);
	minusOne.flip();  // all ones: adding it subtracts 1 modulo 2^nbits
	auto check = [&](const Integer& a, const std::string& name) {
		const Integer up = a + one, down = a + minusOne;
		{ Integer x(a); ++x;             if (!(x == up))   fail(cfg + " ++" + name, 0, 1); }
		{ Integer x(a); --x;             if (!(x == down)) fail(cfg + " --" + name, 0, 1); }
		{ Integer x(a); Integer r = x++; if (!(r == a) || !(x == up))   fail(cfg + " r = " + name + "++", 0, 1); }
		{ Integer x(a); Integer r = x--; if (!(r == a) || !(x == down)) fail(cfg + " r = " + name + "--", 0, 1); }
		{
			Integer x(a);
			Integer c = x++ + one;
			if (!(c == up) || !(x == up))
				fail(cfg + " c = " + name + "++ + 1", 0, 1);
		}
	};
	check(zero, "0");
	check(minusOne, "(all ones)");
	// 2^k - 1 and 2^k at every limb boundary: ++ carries through k/bitsInBlock limbs
	for (unsigned k = bitsInBlock; k < nbits; k += bitsInBlock) {
		Integer edge;
		edge.setbits(0);
		edge.setbit(k);
		Integer below = edge + minusOne;
		check(edge, "2^" + std::to_string(k));
		check(below, "(2^" + std::to_string(k) + " - 1)");
		Integer x(below);
		++x;
		if (!(x == edge)) fail(cfg + " ++(2^" + std::to_string(k) + " - 1) == 2^" + std::to_string(k), 0, 1);
		--x;
		if (!(x == below)) fail(cfg + " --2^" + std::to_string(k) + " == 2^" + std::to_string(k) + " - 1", 0, 1);
	}
	// wraparound at both ends
	{ Integer x(minusOne); ++x; if (!(x == zero)) fail(cfg + " ++(all ones) == 0", 0, 1); }
	{ Integer x(zero); --x; if (!(x == minusOne)) fail(cfg + " --0 == all ones", 0, 1); }
	return fail.count;
}

} // namespace increment_decrement

}} // namespace sw::universal

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
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0
#endif

int main()
try {
	using namespace sw::universal;
	using namespace sw::universal::increment_decrement;

	std::string test_suite  = "integer prefix and postfix increment and decrement";
	std::string test_tag    = "++ --";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	integer<16> a(5), b(10);
	integer<16> c = a++ + b;
	std::cout << "c = a++ + b : c = " << c << " (15), a = " << a << " (6)\n";
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllTypes<8, std::uint8_t>(true, 0, true), "integer< 8, uint8_t >", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	{
		// the example: c = a++ + b adds the old a
		integer<16> a(5), b(10);
		integer<16> c = a++ + b;
		const int fails = (c == 15 ? 0 : 1) + (a == 6 ? 0 : 1);
		if (fails) std::cerr << "FAIL: c = a++ + b with a = 5, b = 10 gave c = " << c << ", a = " << a << '\n';
		nrOfFailedTestCases += ReportTestResult(fails, "integer<16>", "c = a++ + b");
	}
	// exhaustive: every pair of a one-limb 8-bit value, and every value of two limbs that do
	// not fill the last one (carry from the low limb)
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllTypes<8, std::uint8_t>(true, 0, reportTestCases), "integer< 8, uint8_t >", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllTypes<12, std::uint8_t>(true, 0, reportTestCases), "integer<12, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<12, std::uint64_t>(true, 0, reportTestCases),
	                                        "integer<12, uint64_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllTypes<16, std::uint8_t>(true, 0, reportTestCases), "integer<16, uint8_t >", test_tag);
	// sampled, with every limb boundary
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<40, std::uint8_t>(false, 2000, reportTestCases),
	                                        "integer<40, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<40, std::uint16_t>(false, 2000, reportTestCases),
	                                        "integer<40, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<61, std::uint64_t>(false, 2000, reportTestCases),
	                                        "integer<61, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<64, std::uint32_t>(false, 2000, reportTestCases),
	                                        "integer<64, uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<64, std::uint64_t>(false, 2000, reportTestCases),
	                                        "integer<64, uint64_t>", test_tag);
	// above 64 bits, including elreal's integer<256, uint32_t> exponent
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyWide<128, std::uint8_t, IntegerNumberType::IntegerNumber>(reportTestCases),
	                     "integer<128, uint8_t >", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyWide<128, std::uint32_t, IntegerNumberType::IntegerNumber>(reportTestCases),
	                     "integer<128, uint32_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyWide<128, std::uint64_t, IntegerNumberType::IntegerNumber>(reportTestCases),
	                     "integer<128, uint64_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyWide<100, std::uint64_t, IntegerNumberType::WholeNumber>(reportTestCases),
	                     "integer<100, uint64_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyWide<256, std::uint32_t, IntegerNumberType::IntegerNumber>(reportTestCases),
	                     "integer<256, uint32_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<16, std::uint16_t>(true, 0, reportTestCases),
	                                        "integer<16, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<14, std::uint64_t>(true, 0, reportTestCases),
	                                        "integer<14, uint64_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<48, std::uint32_t>(false, 200000, reportTestCases),
	                                        "integer<48, uint32_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyAllTypes<63, std::uint64_t>(false, 1000000, reportTestCases),
	                                        "integer<63, uint64_t>", test_tag);
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
