// number_types.cpp : all three IntegerNumberType flavours checked against native integers
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

/*
   Only IntegerNumber has a sign bit. For WholeNumber and NaturalNumber the most significant
   bit is a data bit, but sign() used to return it raw, and every value in the upper half of
   the range went wrong (#1410): to_string(200) was "-56", 200 >> 1 was 228, hex output
   refused the value as negative, and division, which shifts, gave garbage. The one-block
   / and % cast the operands to a signed native type, and the unsigned operator< did not
   stop at the first block that differs, so multi-block division failed as well.

   Each configuration is checked against native 64-bit arithmetic: sign(), isneg(), decimal
   and hex text, the parse round trip, >>, scale(), widening, / and %, and the relational
   operators. Operands cover the whole encoding, both halves.

   Out-of-domain results (a negative or zero Whole number) are not exercised: with
   INTEGER_THROW_ARITHMETIC_EXCEPTION at its default, 0, those wrap modulo 2^nbits. The
   exception is a Whole quotient that would be 0, which the one-block division reports
   rather than wraps, so Whole division skips a < b.
*/

namespace sw { namespace universal {

namespace number_types {

// the reference: the bit pattern, and its value as the number type reads it
template<unsigned nbits, IntegerNumberType NumberType>
struct Reference {
	static constexpr std::uint64_t mask = (nbits == 64) ? ~std::uint64_t(0) : ((std::uint64_t(1) << nbits) - 1u);
	static constexpr bool isSigned = (NumberType == IntegerNumberType::IntegerNumber);

	static bool negative(std::uint64_t u) { return isSigned && ((u >> (nbits - 1)) & 1u); }
	// sign-extended value, meaningful when isSigned
	static std::int64_t sval(std::uint64_t u) {
		if (negative(u)) return static_cast<std::int64_t>(u | ~mask);
		return static_cast<std::int64_t>(u);
	}
	static std::string text(std::uint64_t u) { return isSigned ? std::to_string(sval(u)) : std::to_string(u); }
	static bool less(std::uint64_t a, std::uint64_t b) { return isSigned ? sval(a) < sval(b) : a < b; }
	static std::uint64_t shr(std::uint64_t u, unsigned s) {
		return isSigned ? (static_cast<std::uint64_t>(sval(u) >> s) & mask) : (u >> s);
	}
	static std::uint64_t quotient(std::uint64_t a, std::uint64_t b) {
		if (!isSigned) return a / b;
		const std::int64_t x = sval(a), y = sval(b);
		if (y == -1) return (static_cast<std::uint64_t>(0) - a) & mask;  // -x, and maxneg / -1 wraps to maxneg
		return static_cast<std::uint64_t>(x / y) & mask;
	}
	static std::uint64_t remainder(std::uint64_t a, std::uint64_t b) {
		if (!isSigned) return a % b;
		const std::int64_t x = sval(a), y = sval(b);
		if (y == -1) return 0;
		return static_cast<std::uint64_t>(x % y) & mask;
	}
	static long scale(std::uint64_t u) {
		// magnitude; maxneg negates to itself, whose log2 is nbits - 1
		std::uint64_t m = negative(u) ? ((static_cast<std::uint64_t>(0) - u) & mask) : u;
		long s = 0;
		while (m > 1) { m >>= 1; ++s; }
		return s;
	}
	static std::string hex(std::uint64_t u) {
		std::ostringstream s;
		s << std::uppercase << std::hex << u;
		return s.str();
	}
};

template<typename Integer>
std::uint64_t Bits(const Integer& v) {
	std::uint64_t u = 0;
	for (unsigned i = 0; i < Integer::nrBlocks && i * Integer::bitsInBlock < 64; ++i) {
		u |= static_cast<std::uint64_t>(v.block(i)) << (i * Integer::bitsInBlock);
	}
	return u;
}

inline const char* Name(IntegerNumberType t) {
	switch (t) {
	case IntegerNumberType::IntegerNumber: return "IntegerNumber";
	case IntegerNumberType::WholeNumber:   return "WholeNumber";
	default:                               return "NaturalNumber";
	}
}

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

// the operations of one operand: sign, text, parse, shift, scale, widen
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
void VerifyUnary(std::uint64_t u, Failures& fail) {
	using Integer = integer<nbits, BlockType, NumberType>;
	using Ref     = Reference<nbits, NumberType>;
	Integer v; v.setbits(u);
	// the tag is built only when a check fails
	auto tag = [&](const char* op) { return std::string(Name(NumberType)) + " bits " + std::to_string(u) + ": " + op; };

	if (v.sign() != Ref::negative(u)) fail(tag("sign()"), std::to_string(v.sign()), std::to_string(Ref::negative(u)));
	if (v.isneg() != Ref::negative(u))
		fail(tag("isneg()"), std::to_string(v.isneg()), std::to_string(Ref::negative(u)));

	const std::string expected = Ref::text(u);
	if (to_string(v) != expected) fail(tag("to_string"), to_string(v), expected);
	std::ostringstream dec; dec << v;
	if (dec.str() != expected) fail(tag("operator<<"), dec.str(), expected);
	if (!Ref::negative(u)) {  // hex output declines negative values by design
		std::ostringstream hex; hex << std::hex << v;
		if (hex.str() != Ref::hex(u)) fail(tag("hex"), hex.str(), Ref::hex(u));
	}

	Integer p; p.setbits(0);
	if (!parse(expected, p) || Bits(p) != u)
		fail(tag("parse of to_string"), std::to_string(Bits(p)), std::to_string(u));

	for (unsigned s = 0; s < nbits; ++s) {
		Integer r(v); r >>= static_cast<int>(s);
		if (Bits(r) != Ref::shr(u, s))
			fail(tag(">>") + " " + std::to_string(s), std::to_string(Bits(r)), std::to_string(Ref::shr(u, s)));
	}

	if (u != 0 && scale(v) != Ref::scale(u))
		fail(tag("scale"), std::to_string(scale(v)), std::to_string(Ref::scale(u)));

	if constexpr (nbits + 8 <= 64) {
		using Wide = integer<nbits + 8, BlockType, NumberType>;
		Wide w(v);
		const std::uint64_t ew = Ref::negative(u) ? (u | (~Ref::mask & Reference<nbits + 8, NumberType>::mask)) : u;
		if (Bits(w) != ew) fail(tag("widen"), std::to_string(Bits(w)), std::to_string(ew));
	}
}

// the operations of two operands: / % and the relational operators
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
void VerifyBinary(std::uint64_t ua, std::uint64_t ub, Failures& fail) {
	using Integer = integer<nbits, BlockType, NumberType>;
	using Ref     = Reference<nbits, NumberType>;
	Integer a, b; a.setbits(ua); b.setbits(ub);
	// the tag is built only when a check fails
	auto tag = [&](const char* op) {
		return std::string(Name(NumberType)) + " bits " + std::to_string(ua) + " " + op + " " + std::to_string(ub);
	};

	const bool lt = Ref::less(ua, ub), gt = Ref::less(ub, ua), eq = (ua == ub);
	if ((a < b) != lt)   fail(tag("<"),  std::to_string(a < b),  std::to_string(lt));
	if ((a > b) != gt)   fail(tag(">"),  std::to_string(a > b),  std::to_string(gt));
	if ((a <= b) != !gt) fail(tag("<="), std::to_string(a <= b), std::to_string(!gt));
	if ((a >= b) != !lt) fail(tag(">="), std::to_string(a >= b), std::to_string(!lt));
	if ((a == b) != eq)  fail(tag("=="), std::to_string(a == b), std::to_string(eq));

	if (ub == 0) return;
	if (NumberType == IntegerNumberType::WholeNumber && ua < ub) return;  // quotient 0 is outside the Whole domain
	const Integer q = a / b, r = a % b;
	if (Bits(q) != Ref::quotient(ua, ub))
		fail(tag("/"), std::to_string(Bits(q)), std::to_string(Ref::quotient(ua, ub)));
	if (Bits(r) != Ref::remainder(ua, ub))
		fail(tag("%"), std::to_string(Bits(r)), std::to_string(Ref::remainder(ua, ub)));
}

// every operand and every pair when exhaustive, else the given number of random ones;
// random operands are drawn from both halves of the encoding
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
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
	{
		std::mt19937_64 rng(1410u + nbits);
		const std::uint64_t msb = std::uint64_t(1) << (nbits - 1);
		auto draw = [&]() {
			std::uint64_t u = rng() & mask;
			switch (rng() % 4) {
			case 0: return u | msb;                  // upper half
			case 1: return u & (msb - 1u);           // lower half
			case 2: return u >> (rng() % nbits);     // short values
			default: return u;
			}
		};
		for (unsigned k = 0; k < samples; ++k) {
			const std::uint64_t a = draw(), b = draw();
			VerifyUnary<nbits, BlockType, NumberType>(a, fail);
			VerifyBinary<nbits, BlockType, NumberType>(a, b, fail);
		}
	}
	return fail.count;
}

// the three number types of one configuration
template<unsigned nbits, typename BlockType>
int VerifyConfiguration(bool exhaustive, unsigned samples, bool reportTestCases) {
	int nrOfFailedTestCases = 0;
	nrOfFailedTestCases +=
	    VerifyNumberType<nbits, BlockType, IntegerNumberType::IntegerNumber>(exhaustive, samples, reportTestCases);
	nrOfFailedTestCases +=
	    VerifyNumberType<nbits, BlockType, IntegerNumberType::WholeNumber>(exhaustive, samples, reportTestCases);
	nrOfFailedTestCases +=
	    VerifyNumberType<nbits, BlockType, IntegerNumberType::NaturalNumber>(exhaustive, samples, reportTestCases);
	return nrOfFailedTestCases;
}

// the cases #1410 reported, and the ones found while fixing it
inline int VerifyReportedCases(bool reportTestCases) {
	Failures fail(reportTestCases);
	auto check = [&](const std::string& what, const std::string& got, const std::string& expected) {
		if (got != expected) fail(what, got, expected);
	};
	integer<8, std::uint8_t, WholeNumber> w; w.setbits(200);
	integer<8, std::uint8_t, NaturalNumber> n; n.setbits(200);
	check("to_string(Whole 200)", to_string(w), "200");
	check("to_string(Natural 200)", to_string(n), "200");
	check("Whole 200 >> 1", to_string(w >> 1), "100");
	check("Whole 200 / 3", to_string(w / integer<8, std::uint8_t, WholeNumber>(3)), "66");
	check("Whole 200 % 3", to_string(w % integer<8, std::uint8_t, WholeNumber>(3)), "2");
	check("scale(Whole 200)", std::to_string(scale(w)), "7");
	std::ostringstream hex; hex << std::hex << w;
	check("hex(Whole 200)", hex.str(), "C8");

	integer<16, std::uint8_t, WholeNumber> a, b; a.setbits(512); b.setbits(261);
	check("Whole<16,uint8_t> 512 < 261", std::to_string(a < b), "0");
	check("Whole<16,uint8_t> 512 > 261", std::to_string(a > b), "1");
	a.setbits(40000);
	check("Whole<16,uint8_t> 40000 / 3", to_string(a / integer<16, std::uint8_t, WholeNumber>(3)), "13333");

	integer<8, std::uint8_t, IntegerNumber> i; i.setbits(200);
	check("to_string(Integer bits 200)", to_string(i), "-56");
	check("Integer bits 200 >> 1", to_string(i >> 1), "-28");
	return fail.count;
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

	std::string test_suite  = "integer number types: IntegerNumber, WholeNumber, NaturalNumber";
	std::string test_tag    = "number types";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifyReportedCases(true), "reported cases", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyConfiguration<8, std::uint8_t>(true, 0, true), "integer< 8, uint8_t >", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyReportedCases(reportTestCases), "reported cases", test_tag);
	// exhaustive: one block (the native fast paths), and two blocks that do not fill the last
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<8, std::uint8_t>(true, 0, reportTestCases),
	                                        "integer< 8, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<10, std::uint8_t>(true, 0, reportTestCases),
	                                        "integer<10, uint8_t >", test_tag);
	// sampled: exact-fit single blocks, and multi-block long division
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<16, std::uint16_t>(false, 20000, reportTestCases),
	                                        "integer<16, uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<16, std::uint8_t>(false, 20000, reportTestCases),
	                                        "integer<16, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<32, std::uint32_t>(false, 20000, reportTestCases),
	                                        "integer<32, uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<32, std::uint8_t>(false, 10000, reportTestCases),
	                                        "integer<32, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<64, std::uint64_t>(false, 10000, reportTestCases),
	                                        "integer<64, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<64, std::uint32_t>(false, 10000, reportTestCases),
	                                        "integer<64, uint32_t>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<12, std::uint8_t>(true, 0, reportTestCases),
	                                        "integer<12, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<12, std::uint16_t>(true, 0, reportTestCases),
	                                        "integer<12, uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<24, std::uint8_t>(false, 200000, reportTestCases),
	                                        "integer<24, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<48, std::uint16_t>(false, 200000, reportTestCases),
	                                        "integer<48, uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<64, std::uint8_t>(false, 1000000, reportTestCases),
	                                        "integer<64, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyConfiguration<64, std::uint64_t>(false, 1000000, reportTestCases),
	                                        "integer<64, uint64_t>", test_tag);
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
