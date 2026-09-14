// stream_output.cpp: decimal operator<< of integer<> against native integers
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
#include <utility>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_suite.hpp>

/*
   operator<< writes decimal digits in blocks of 10^k, the largest power of ten a limb
   holds: 100, 10^4, 10^9 or 10^18. The block used to live in integer<nbits + 1>, where it
   wrapped whenever it did not fit (#1494): integer<12, uint16_t> printed 1808 as "10000",
   for all three number types. A WholeNumber value just under a full limb also wrote "whole
   number cannot be zero" to stderr, because the formatter divided in a WholeNumber type.

   Every width from 2 to 12 bits is streamed exhaustively, for all four limb types and all
   three number types, and 15 wider widths up to 64 bits are sampled, each against
   std::to_string. The 128-bit extremes are checked against their known decimal strings.
*/

namespace sw { namespace universal {

namespace stream_output {

inline const char* Name(IntegerNumberType t) {
	switch (t) {
	case IntegerNumberType::IntegerNumber: return "IntegerNumber";
	case IntegerNumberType::WholeNumber:   return "WholeNumber";
	default:                               return "NaturalNumber";
	}
}

// operator<< of the bit pattern u against std::to_string of its value
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
int VerifyValue(std::uint64_t u, bool reportTestCases) {
	constexpr std::uint64_t mask = (nbits == 64) ? ~std::uint64_t(0) : ((std::uint64_t(1) << nbits) - 1u);
	const bool negative = (NumberType == IntegerNumberType::IntegerNumber) && ((u >> (nbits - 1)) & 1u);
	const std::string expected =
	    negative ? std::to_string(static_cast<std::int64_t>(u | ~mask)) : std::to_string(u);

	integer<nbits, BlockType, NumberType> v;
	v.setbits(u);
	std::ostringstream s;
	s << v;
	if (s.str() == expected) return 0;
	if (reportTestCases)
		std::cerr << "FAIL: integer<" << nbits << ", " << sizeof(BlockType) * 8 << "-bit limbs, " << Name(NumberType)
		          << "> bits " << u << " printed " << s.str() << ", expected " << expected << '\n';
	return 1;
}

// every value when exhaustive, else the given number of random ones from both halves
template<unsigned nbits, typename BlockType, IntegerNumberType NumberType>
int VerifyWidth(bool exhaustive, unsigned samples, bool reportTestCases) {
	int nrOfFailedTestCases = 0;
	auto check = [&](std::uint64_t u) {
		// without reportTestCases, show the first failure of the width only
		const int fails = VerifyValue<nbits, BlockType, NumberType>(u, reportTestCases);
		if (fails && nrOfFailedTestCases == 0 && !reportTestCases) VerifyValue<nbits, BlockType, NumberType>(u, true);
		nrOfFailedTestCases += fails;
	};
	constexpr std::uint64_t mask = (nbits == 64) ? ~std::uint64_t(0) : ((std::uint64_t(1) << nbits) - 1u);
	if constexpr (nbits <= 20) {
		if (exhaustive) {
			for (std::uint64_t u = 0; u <= mask; ++u) check(u);
			return nrOfFailedTestCases;
		}
	}
	std::mt19937_64 rng(1494u + nbits);
	check(0);
	check(mask);                                  // -1, or the largest unsigned value
	check(std::uint64_t(1) << (nbits - 1));       // maxneg, or the top bit alone
	check((std::uint64_t(1) << (nbits - 1)) - 1); // maxpos
	for (unsigned k = 0; k < samples; ++k) check((rng() & mask) >> (rng() % nbits));
	return nrOfFailedTestCases;
}

// all three number types of one width and limb type
template<unsigned nbits, typename BlockType>
int VerifyAllTypes(bool exhaustive, unsigned samples, bool reportTestCases) {
	return VerifyWidth<nbits, BlockType, IntegerNumberType::IntegerNumber>(exhaustive, samples, reportTestCases) +
	       VerifyWidth<nbits, BlockType, IntegerNumberType::WholeNumber>(exhaustive, samples, reportTestCases) +
	       VerifyWidth<nbits, BlockType, IntegerNumberType::NaturalNumber>(exhaustive, samples, reportTestCases);
}

// widths first, first + 1, ..., first + count - 1
template<unsigned first, typename BlockType, unsigned... offset>
int VerifyWidths(std::integer_sequence<unsigned, offset...>, bool exhaustive, unsigned samples, bool reportTestCases) {
	return (0 + ... + VerifyAllTypes<first + offset, BlockType>(exhaustive, samples, reportTestCases));
}

// every width of the range for one limb type
template<unsigned first, unsigned last, typename BlockType>
int VerifyRange(bool exhaustive, unsigned samples, bool reportTestCases) {
	return VerifyWidths<first, BlockType>(std::make_integer_sequence<unsigned, last - first + 1>{}, exhaustive, samples,
	                                      reportTestCases);
}

// sampled widths above 12 bits: either side of each 10^k block width (7, 14, 30 and 60 bits)
// and of the limb widths, kept to a list because every width instantiates its own types
template<typename BlockType>
int VerifySampledWidths(unsigned samples, bool reportTestCases) {
	return VerifyAllTypes<13, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<14, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<15, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<16, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<20, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<24, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<29, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<30, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<31, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<32, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<48, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<59, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<60, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<63, BlockType>(false, samples, reportTestCases) +
	       VerifyAllTypes<64, BlockType>(false, samples, reportTestCases);
}

// the 128-bit extremes, which take several blocks of digits
template<typename BlockType>
int VerifyWide(bool reportTestCases) {
	int nrOfFailedTestCases = 0;
	auto check = [&](const std::string& what, const std::string& got, const std::string& expected) {
		if (got == expected) return;
		++nrOfFailedTestCases;
		if (reportTestCases || nrOfFailedTestCases == 1)
			std::cerr << "FAIL: " << what << " printed " << got << ", expected " << expected << '\n';
	};
	auto text = [](const auto& v) {
		std::ostringstream s;
		s << v;
		return s.str();
	};
	integer<128, BlockType, IntegerNumberType::IntegerNumber> i;
	i.clear();
	i.setbit(127);
	check("integer<128> maxneg", text(i), "-170141183460469231731687303715884105728");
	i.flip();
	check("integer<128> maxpos", text(i), "170141183460469231731687303715884105727");
	integer<128, BlockType, IntegerNumberType::WholeNumber> w;
	w.clear();
	w.flip();
	check("integer<128, WholeNumber> max", text(w), "340282366920938463463374607431768211455");
	return nrOfFailedTestCases;
}

} // namespace stream_output

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
	using namespace sw::universal::stream_output;

	std::string test_suite  = "integer decimal stream output";
	std::string test_tag    = "operator<<";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	integer<12, std::uint16_t> a;
	a.setbits(1808);
	std::cout << a << '\n';  // was 10000
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyAllTypes<12, std::uint16_t>(true, 0, true), "integer<12, uint16_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else

#if REGRESSION_LEVEL_1
	// exhaustive: every width with a 10^k block that did not fit integer<nbits + 1>
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyRange<2, 12, std::uint8_t>(true, 0, reportTestCases), "2-12 bits, uint8_t", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyRange<2, 12, std::uint16_t>(true, 0, reportTestCases), "2-12 bits, uint16_t", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyRange<2, 12, std::uint32_t>(true, 0, reportTestCases), "2-12 bits, uint32_t", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyRange<2, 12, std::uint64_t>(true, 0, reportTestCases), "2-12 bits, uint64_t", test_tag);
	// sampled: widths up to 64 bits, where a 64-bit reference still covers them
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySampledWidths<std::uint8_t>(1000, reportTestCases), "13-64 bits, uint8_t", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySampledWidths<std::uint16_t>(1000, reportTestCases), "13-64 bits, uint16_t", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySampledWidths<std::uint32_t>(1000, reportTestCases), "13-64 bits, uint32_t", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySampledWidths<std::uint64_t>(1000, reportTestCases), "13-64 bits, uint64_t", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide<std::uint8_t>(reportTestCases), "128 bits, uint8_t", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide<std::uint32_t>(reportTestCases), "128 bits, uint32_t", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyWide<std::uint64_t>(reportTestCases), "128 bits, uint64_t", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases +=
	    ReportTestResult(VerifyRange<13, 16, std::uint8_t>(true, 0, reportTestCases), "13-16 bits, uint8_t", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRange<13, 16, std::uint16_t>(true, 0, reportTestCases),
	                                        "13-16 bits, uint16_t", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRange<13, 16, std::uint32_t>(true, 0, reportTestCases),
	                                        "13-16 bits, uint32_t", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyRange<17, 20, std::uint16_t>(true, 0, reportTestCases),
	                                        "17-20 bits, uint16_t", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyRange<17, 20, std::uint32_t>(true, 0, reportTestCases),
	                                        "17-20 bits, uint32_t", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySampledWidths<std::uint32_t>(100000, reportTestCases), "13-64 bits, uint32_t", test_tag);
	nrOfFailedTestCases +=
	    ReportTestResult(VerifySampledWidths<std::uint64_t>(100000, reportTestCases), "13-64 bits, uint64_t", test_tag);
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
