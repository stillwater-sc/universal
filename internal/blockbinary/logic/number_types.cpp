// number_types.cpp : Signed and Unsigned blockbinary checked against native integers
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
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_suite.hpp>

/*
   Only a Signed blockbinary has a sign bit. For Unsigned the most significant bit is a data
   bit, but sign() returned it raw, and the relational operators started from ispos() and
   isneg(), so every value >= 2^(N-1) sorted below every value < 2^(N-1) (#1479):
   blockbinary<8, uint8_t, Unsigned> 200 < 100 was true, half of all pairs ordered wrong
   under each of <, <=, > and >=. The same raw bit made >>= fill with ones and to_sll()
   sign-extend.

   Every configuration is checked against native 64-bit integers: the six relational
   operators, sign(), ispos(), isneg(), >>= by every amount, and to_sll(). Signed values
   are checked by the same reference, so the Signed behaviour is pinned as well.
*/

namespace sw { namespace universal {

namespace number_types {

inline const char* Name(BinaryNumberType t) { return t == BinaryNumberType::Signed ? "Signed" : "Unsigned"; }

// the reference: the bit pattern, and its value as the number type reads it
template<unsigned nbits, BinaryNumberType NumberType>
struct Reference {
	static constexpr std::uint64_t mask = (nbits == 64) ? ~std::uint64_t(0) : ((std::uint64_t(1) << (nbits % 64)) - 1u);
	static constexpr bool isSigned = (NumberType == BinaryNumberType::Signed);

	static bool negative(std::uint64_t u) { return isSigned && ((u >> (nbits - 1)) & 1u); }
	static std::int64_t sval(std::uint64_t u) {
		return negative(u) ? static_cast<std::int64_t>(u | ~mask) : static_cast<std::int64_t>(u);
	}
	static bool less(std::uint64_t a, std::uint64_t b) { return isSigned ? sval(a) < sval(b) : a < b; }
	static std::uint64_t shr(std::uint64_t u, unsigned s) {
		return isSigned ? (static_cast<std::uint64_t>(sval(u) >> s) & mask) : (u >> s);
	}
};

// a failure report, printed for the first few failures of a configuration
struct Failures {
	int count = 0;
	bool report;
	explicit Failures(bool reportTestCases) : report(reportTestCases) {}
	void operator()(const std::string& what, const std::string& got, const std::string& expected) {
		if (report || count < 8) std::cerr << "FAIL: " << what << " = " << got << ", expected " << expected << '\n';
		++count;
	}
};

// the operations of one operand: sign(), ispos(), isneg(), >>= and to_sll()
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
void VerifyUnary(std::uint64_t u, Failures& fail) {
	using Ref = Reference<nbits, NumberType>;
	blockbinary<nbits, BlockType, NumberType> v;
	v.setbits(u);
	auto tag = [&](const char* op) { return std::string(Name(NumberType)) + " bits " + std::to_string(u) + ": " + op; };

	const bool neg = Ref::negative(u);
	if (v.sign() != neg) fail(tag("sign()"), std::to_string(v.sign()), std::to_string(neg));
	if (v.isneg() != neg) fail(tag("isneg()"), std::to_string(v.isneg()), std::to_string(neg));
	if (v.ispos() != !neg) fail(tag("ispos()"), std::to_string(v.ispos()), std::to_string(!neg));
	for (unsigned s = 0; s < nbits; ++s) {
		blockbinary<nbits, BlockType, NumberType> r(v);
		r >>= static_cast<int>(s);
		if (r.to_ull() != Ref::shr(u, s))
			fail(tag(">>=") + " " + std::to_string(s), std::to_string(r.to_ull()), std::to_string(Ref::shr(u, s)));
	}
	if constexpr (nbits < 64) {
		if (v.to_sll() != Ref::sval(u)) fail(tag("to_sll()"), std::to_string(v.to_sll()), std::to_string(Ref::sval(u)));
	}
}

// the six relational operators
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
void VerifyBinary(std::uint64_t ua, std::uint64_t ub, Failures& fail) {
	using Ref = Reference<nbits, NumberType>;
	blockbinary<nbits, BlockType, NumberType> a, b;
	a.setbits(ua);
	b.setbits(ub);
	auto tag = [&](const char* op) {
		return std::string(Name(NumberType)) + " bits " + std::to_string(ua) + " " + op + " " + std::to_string(ub);
	};
	const bool lt = Ref::less(ua, ub), gt = Ref::less(ub, ua), eq = (ua == ub);
	if ((a < b) != lt)   fail(tag("<"), std::to_string(a < b), std::to_string(lt));
	if ((a <= b) != !gt) fail(tag("<="), std::to_string(a <= b), std::to_string(!gt));
	if ((a > b) != gt)   fail(tag(">"), std::to_string(a > b), std::to_string(gt));
	if ((a >= b) != !lt) fail(tag(">="), std::to_string(a >= b), std::to_string(!lt));
	if ((a == b) != eq)  fail(tag("=="), std::to_string(a == b), std::to_string(eq));
	if ((a != b) != !eq) fail(tag("!="), std::to_string(a != b), std::to_string(!eq));
}

// every operand and pair when exhaustive, else random ones from both halves of the encoding
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
int VerifyNumberType(bool exhaustive, unsigned samples, bool reportTestCases) {
	Failures fail(reportTestCases);
	constexpr std::uint64_t mask = Reference<nbits, NumberType>::mask;
	if constexpr (nbits <= 16) {
		if (exhaustive) {
			for (std::uint64_t a = 0; a <= mask; ++a) {
				VerifyUnary<nbits, BlockType, NumberType>(a, fail);
				for (std::uint64_t b = 0; b <= mask; ++b) VerifyBinary<nbits, BlockType, NumberType>(a, b, fail);
			}
			return fail.count;
		}
	}
	std::mt19937_64 rng(1479u + nbits);
	const std::uint64_t msb = std::uint64_t(1) << (nbits - 1);
	auto draw = [&]() {
		const std::uint64_t u = rng() & mask;
		switch (rng() % 3) {
		case 0: return u | msb;         // upper half
		case 1: return u & (msb - 1u);  // lower half
		default: return u >> (rng() % nbits);
		}
	};
	for (unsigned k = 0; k < samples; ++k) {
		const std::uint64_t a = draw(), b = draw();
		VerifyUnary<nbits, BlockType, NumberType>(a, fail);
		VerifyBinary<nbits, BlockType, NumberType>(a, b, fail);
		VerifyBinary<nbits, BlockType, NumberType>(a, a, fail);
	}
	return fail.count;
}

// setblock() keeps the bits above nbits clear, as every other setter does, so a most
// significant block written with them set compares and reads as its nbits value
template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
int VerifySetblockMasks(bool reportTestCases) {
	using BB = blockbinary<nbits, BlockType, NumberType>;
	Failures fail(reportTestCases);
	const std::string tag = std::string("blockbinary<") + std::to_string(nbits) + ", " +
	                        std::to_string(sizeof(BlockType) * 8) + "-bit, " + Name(NumberType) + ">: ";
	BB zero, a;
	zero.setbits(0);
	a.setbits(0);
	a.setblock(BB::MSU, static_cast<BlockType>(~BlockType(0)));  // every storage bit of the MSU
	const std::uint64_t expected =
	    Reference<nbits, NumberType>::mask & ~((std::uint64_t(1) << (BB::MSU * BB::bitsInBlock)) - 1u);
	if (a.to_ull() != expected)
		fail(tag + "setblock(MSU, all ones) value", std::to_string(a.to_ull()), std::to_string(expected));
	a.setbits(0);
	a.setblock(BB::MSU, static_cast<BlockType>(~BlockType(0) & ~BB::MSU_MASK));  // only bits above nbits
	if (!(a == zero)) fail(tag + "setblock(MSU, bits above nbits) == 0", "false", "true");
	if (zero < a || a < zero) fail(tag + "setblock(MSU, bits above nbits) ordered apart from 0", "true", "false");
	return fail.count;
}

// both number types of one configuration
template<unsigned nbits, typename BlockType>
int VerifyConfiguration(bool exhaustive, unsigned samples, bool reportTestCases) {
	return VerifyNumberType<nbits, BlockType, BinaryNumberType::Signed>(exhaustive, samples, reportTestCases) +
	       VerifyNumberType<nbits, BlockType, BinaryNumberType::Unsigned>(exhaustive, samples, reportTestCases);
}

} // namespace number_types

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
	using namespace sw::universal::number_types;

	std::string test_suite  = "blockbinary number types: Signed, Unsigned";
	std::string test_tag    = "number types";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	blockbinary<8, std::uint8_t, BinaryNumberType::Unsigned> a, b;
	a.setbits(200);
	b.setbits(100);
	std::cout << "200 < 100 : " << (a < b) << " (was 1)\n";
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyConfiguration<8, std::uint8_t>(true, 0, true), "blockbinary< 8, uint8_t >", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	{
		// the case reported in #1479
		blockbinary<8, std::uint8_t, BinaryNumberType::Unsigned> a, b;
		a.setbits(200);
		b.setbits(100);
		const int fails = ((a < b) ? 1 : 0) + ((b < a) ? 0 : 1);
		if (fails) std::cerr << "FAIL: Unsigned 200 < 100 = " << (a < b) << ", 100 < 200 = " << (b < a) << '\n';
		nrOfFailedTestCases += ReportTestResult(fails, "blockbinary< 8, uint8_t >", "reported case");
	}
	// setblock() with bits above nbits: one block, and the top block of two (CodeRabbit on #1499)
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySetblockMasks<12, std::uint16_t, BinaryNumberType::Unsigned>(reportTestCases) +
	                         VerifySetblockMasks<12, std::uint16_t, BinaryNumberType::Signed>(reportTestCases) +
	                         VerifySetblockMasks<12, std::uint8_t, BinaryNumberType::Unsigned>(reportTestCases) +
	                         VerifySetblockMasks<12, std::uint8_t, BinaryNumberType::Signed>(reportTestCases),
	                     "blockbinary<12>", "setblock masks");
	// exhaustive: one block, two blocks (the multi-block case the issue asks for), and one
	// block that the value does not fill
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<8, std::uint8_t>(true, 0, reportTestCases),
	                                        "blockbinary< 8, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<12, std::uint8_t>(true, 0, reportTestCases),
	                                        "blockbinary<12, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<12, std::uint16_t>(true, 0, reportTestCases),
	                                        "blockbinary<12, uint16_t>", test_tag);
	// sampled: wider values and every block size
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<16, std::uint8_t>(false, 20000, reportTestCases),
	                                        "blockbinary<16, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<32, std::uint32_t>(false, 20000, reportTestCases),
	                                        "blockbinary<32, uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<40, std::uint16_t>(false, 20000, reportTestCases),
	                                        "blockbinary<40, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<64, std::uint64_t>(false, 20000, reportTestCases),
	                                        "blockbinary<64, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<64, std::uint8_t>(false, 20000, reportTestCases),
	                                        "blockbinary<64, uint8_t >", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<14, std::uint8_t>(true, 0, reportTestCases),
	                                        "blockbinary<14, uint8_t >", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<16, std::uint16_t>(false, 1000000, reportTestCases),
	                                        "blockbinary<16, uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<48, std::uint32_t>(false, 1000000, reportTestCases),
	                                        "blockbinary<48, uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<64, std::uint64_t>(false, 1000000, reportTestCases),
	                                        "blockbinary<64, uint64_t>", test_tag);
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
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
