// limb_widths.cpp : IntegerNumber arithmetic at widths that do not fill their last limb
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

/*
   On MSVC, integer<nbits, uint64_t> with nbits < 64 -- one limb the value does not fill --
   gave wrong results that gcc and clang did not (#1500): every negative value printed as
   "-0", and 2^59 / 10^18 came out as quotient 1, remainder 2^59. Both follow if
   twosComplement() gives 0.

   Each primitive on the way is checked separately against native 64-bit arithmetic, so a
   failure names the step: integer(1), flip(), ++, --, twosComplement(), unary minus, then
   + - * / % and <. Every limb type is covered; the uint64_t widths narrower than 64 bits
   are the ones under suspicion, the others are the control.
*/

namespace sw { namespace universal {

namespace limb_widths {

// failures counted per operation; the first two of each are printed
struct Failures {
	std::map<std::string, int> count;
	bool report;
	explicit Failures(bool reportTestCases) : report(reportTestCases) {}
	void operator()(const std::string& op, const std::string& what, std::uint64_t got, std::uint64_t expected) {
		if (report || count[op] < 2)
			std::cerr << "FAIL: " << what << " = " << got << ", expected " << expected << '\n';
		++count[op];
	}
	int total() const {
		int n = 0;
		for (const auto& kv : count) n += kv.second;
		return n;
	}
	void summary(const std::string& config) const {
		if (total() == 0) return;
		std::cerr << "      " << config << " failures by operation:";
		for (const auto& kv : count) std::cerr << ' ' << kv.first << '=' << kv.second;
		std::cerr << '\n';
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

template<unsigned nbits>
struct Reference {
	static constexpr std::uint64_t mask = (nbits == 64) ? ~std::uint64_t(0) : ((std::uint64_t(1) << (nbits % 64)) - 1u);
	static std::int64_t sval(std::uint64_t u) {
		const bool neg = (u >> (nbits - 1)) & 1u;
		return neg ? static_cast<std::int64_t>(u | ~mask) : static_cast<std::int64_t>(u);
	}
	static std::uint64_t wrap(std::uint64_t u) { return u & mask; }
	static std::uint64_t quotient(std::uint64_t a, std::uint64_t b) {
		const std::int64_t x = sval(a), y = sval(b);
		if (y == -1) return wrap(std::uint64_t(0) - a);  // maxneg / -1 wraps to maxneg
		return wrap(static_cast<std::uint64_t>(x / y));
	}
	static std::uint64_t remainder(std::uint64_t a, std::uint64_t b) {
		const std::int64_t x = sval(a), y = sval(b);
		if (y == -1) return 0;
		return wrap(static_cast<std::uint64_t>(x % y));
	}
};

// the one-operand steps of negation, and unary minus
template<unsigned nbits, typename BlockType>
void VerifyUnary(std::uint64_t u, Failures& fail, const std::string& config) {
	using Integer = integer<nbits, BlockType, IntegerNumberType::IntegerNumber>;
	using Ref = Reference<nbits>;
	auto what = [&](const char* op) { return config + " " + op + " of bits " + std::to_string(u); };
	Integer v;
	v.setbits(u);
	{
		Integer r(v);
		r.flip();
		if (Bits(r) != Ref::wrap(~u))
			fail("flip", what("flip()"), Bits(r), Ref::wrap(~u));
	}
	{
		Integer r(v);
		++r;
		if (Bits(r) != Ref::wrap(u + 1u))
			fail("++", what("++"), Bits(r), Ref::wrap(u + 1u));
	}
	{
		Integer r(v);
		--r;
		if (Bits(r) != Ref::wrap(u - 1u))
			fail("--", what("--"), Bits(r), Ref::wrap(u - 1u));
	}
	{
		Integer r(v);
		r.twosComplement();
		if (Bits(r) != Ref::wrap(std::uint64_t(0) - u))
			fail("twosComplement", what("twosComplement()"), Bits(r), Ref::wrap(std::uint64_t(0) - u));
	}
	{
		Integer r = -v;
		if (Bits(r) != Ref::wrap(std::uint64_t(0) - u))
			fail("negate", what("unary -"), Bits(r), Ref::wrap(std::uint64_t(0) - u));
	}
}

// + - * / % and <
template<unsigned nbits, typename BlockType>
void VerifyBinary(std::uint64_t ua, std::uint64_t ub, Failures& fail, const std::string& config) {
	using Integer = integer<nbits, BlockType, IntegerNumberType::IntegerNumber>;
	using Ref = Reference<nbits>;
	auto what = [&](const char* op) {
		return config + " bits " + std::to_string(ua) + " " + op + " " + std::to_string(ub);
	};
	Integer a, b;
	a.setbits(ua);
	b.setbits(ub);
	if (Bits(a + b) != Ref::wrap(ua + ub)) fail("+", what("+"), Bits(a + b), Ref::wrap(ua + ub));
	if (Bits(a - b) != Ref::wrap(ua - ub)) fail("-", what("-"), Bits(a - b), Ref::wrap(ua - ub));
	if (Bits(a * b) != Ref::wrap(ua * ub)) fail("*", what("*"), Bits(a * b), Ref::wrap(ua * ub));
	const bool lt = Ref::sval(ua) < Ref::sval(ub);
	if ((a < b) != lt) fail("<", what("<"), (a < b), lt);
	if (ub != 0) {
		if (Bits(a / b) != Ref::quotient(ua, ub)) fail("/", what("/"), Bits(a / b), Ref::quotient(ua, ub));
		if (Bits(a % b) != Ref::remainder(ua, ub)) fail("%", what("%"), Bits(a % b), Ref::remainder(ua, ub));
	}
}

// every value (and pair, up to 8 bits) when exhaustive, else random ones from both halves
template<unsigned nbits, typename BlockType>
int VerifyWidth(bool exhaustive, unsigned samples, bool reportTestCases) {
	using Integer = integer<nbits, BlockType, IntegerNumberType::IntegerNumber>;
	const std::string config = std::string("integer<") + std::to_string(nbits) + ", uint" +
	                           std::to_string(sizeof(BlockType) * 8) + "_t>";
	Failures fail(reportTestCases);
	constexpr std::uint64_t mask = Reference<nbits>::mask;

	const Integer one(1);  // the constant ++ and -- add
	if (Bits(one) != 1u) fail("integer(1)", config + " integer(1)", Bits(one), 1u);

	if constexpr (nbits <= 12) {
		if (exhaustive) {
			for (std::uint64_t a = 0; a <= mask; ++a) {
				VerifyUnary<nbits, BlockType>(a, fail, config);
				if constexpr (nbits <= 8) {
					for (std::uint64_t b = 0; b <= mask; ++b) VerifyBinary<nbits, BlockType>(a, b, fail, config);
				}
				else {
					VerifyBinary<nbits, BlockType>(a, (a * 2654435761u) & mask, fail, config);
				}
			}
			fail.summary(config);
			return fail.total();
		}
	}
	std::mt19937_64 rng(1500u + nbits);
	const std::uint64_t msb = std::uint64_t(1) << (nbits - 1);
	auto draw = [&]() {
		const std::uint64_t u = rng() & mask;
		switch (rng() % 4) {
		case 0: return u | msb;         // negative
		case 1: return u & (msb - 1u);  // positive
		case 2: return u >> (rng() % nbits);
		default: return (rng() % 2) ? mask : msb;  // -1 and maxneg
		}
	};
	for (unsigned k = 0; k < samples; ++k) {
		const std::uint64_t a = draw(), b = draw();
		VerifyUnary<nbits, BlockType>(a, fail, config);
		VerifyBinary<nbits, BlockType>(a, b, fail, config);
	}
	fail.summary(config);
	return fail.total();
}

// the exhaustive widths 2 to 12 for one limb type
template<typename BlockType, unsigned... offset>
int VerifySmallWidths(std::integer_sequence<unsigned, offset...>, bool reportTestCases) {
	return (0 + ... + VerifyWidth<2 + offset, BlockType>(true, 0, reportTestCases));
}

} // namespace limb_widths

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
	using namespace sw::universal::limb_widths;

	std::string test_suite  = "integer arithmetic at widths that do not fill their last limb";
	std::string test_tag    = "limb widths";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	integer<12, std::uint64_t> a;
	a.setbits(0xFFF);  // -1
	a.twosComplement();
	std::cout << "twosComplement(-1) = " << a << " (1 expected)\n";
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyWidth<12, std::uint64_t>(true, 0, true), "integer<12, uint64_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	// the suspects: one uint64_t limb, not filled
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySmallWidths<std::uint64_t>(std::make_integer_sequence<unsigned, 11>{}, reportTestCases),
	                     "2-12 bits, uint64_t", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<13, std::uint64_t>(false, 5000, reportTestCases),
	                                        "integer<13, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<32, std::uint64_t>(false, 5000, reportTestCases),
	                                        "integer<32, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<40, std::uint64_t>(false, 5000, reportTestCases),
	                                        "integer<40, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<61, std::uint64_t>(false, 5000, reportTestCases),
	                                        "integer<61, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<63, std::uint64_t>(false, 5000, reportTestCases),
	                                        "integer<63, uint64_t>", test_tag);
	// the controls: a full uint64_t limb, and narrower limbs
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<64, std::uint64_t>(false, 5000, reportTestCases),
	                                        "integer<64, uint64_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySmallWidths<std::uint32_t>(std::make_integer_sequence<unsigned, 11>{}, reportTestCases),
	                     "2-12 bits, uint32_t", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<32, std::uint32_t>(false, 5000, reportTestCases),
	                                        "integer<32, uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<40, std::uint32_t>(false, 5000, reportTestCases),
	                                        "integer<40, uint32_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyWidth<12, std::uint16_t>(true, 0, reportTestCases), "integer<12, uint16_t>", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyWidth<12, std::uint8_t>(true, 0, reportTestCases), "integer<12, uint8_t >", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<63, std::uint8_t>(false, 5000, reportTestCases),
	                                        "integer<63, uint8_t >", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<40, std::uint64_t>(false, 200000, reportTestCases),
	                                        "integer<40, uint64_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<61, std::uint64_t>(false, 200000, reportTestCases),
	                                        "integer<61, uint64_t>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<48, std::uint16_t>(false, 200000, reportTestCases),
	                                        "integer<48, uint16_t>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyWidth<63, std::uint64_t>(false, 1000000, reportTestCases),
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
