// decimal_converter.cpp: test suite runner for blockbinary decimal conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <string>
#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/internal/blockbinary/manipulators.hpp>
#include <universal/verification/test_suite.hpp>

namespace sw { namespace universal {

	// every encoding of a narrow blockbinary against what the native type says the value is. This
	// covers the most negative value, which used to be complemented in place -- and -2^(nbits-1) has
	// no positive twin in nbits, so it stayed negative and the digit loop emitted characters below
	// '0' (#1395).
	template<unsigned nbits, typename BlockType, BinaryNumberType NumberType>
	int VerifyToDecimal(bool reportTestCases) {
		static_assert(nbits <= 16, "blockbinary state space is too large to exhaustively test with VerifyToDecimal");
		constexpr unsigned NR_ENCODINGS = (1u << nbits);
		constexpr long long half        = (1ll << (nbits - 1));
		int nrOfFailedTestCases = 0;

		for (unsigned bits = 0; bits < NR_ENCODINGS; ++bits) {
			blockbinary<nbits, BlockType, NumberType> a;
			a.setbits(bits);
			long long value = static_cast<long long>(bits);
			if constexpr (NumberType == BinaryNumberType::Signed) {
				if (value >= half) value -= 2 * half;
			}
			const std::string expected = std::to_string(value);
			const std::string observed = to_decimal(a);
			if (observed != expected) {
				++nrOfFailedTestCases;
				if (reportTestCases)
					std::cerr << "FAIL: to_decimal(" << to_binary(a) << ") = " << observed
					          << " expected " << expected << '\n';
				if (nrOfFailedTestCases > 9) return nrOfFailedTestCases;
			}
		}
		return nrOfFailedTestCases;
	}

	// values a native type cannot hold, against reference strings. A wide blockbinary used to divide
	// its way to the digits, and division above 64 bits is longdivision, which is signed-only: a wide
	// UNSIGNED blockbinary did not compile at all once to_decimal was instantiated (#1395). Every
	// instantiation below is part of the test.
	int VerifyWideToDecimal(bool reportTestCases) {
		int nrOfFailedTestCases = 0;

		auto check = [&](const std::string& observed, const std::string& expected, const char* what) {
			if (observed != expected) {
				++nrOfFailedTestCases;
				if (reportTestCases)
					std::cerr << "FAIL: " << what << " = " << observed << " expected " << expected << '\n';
			}
		};

		{   // signed, 128 bits
			using BB = blockbinary<128, std::uint32_t, BinaryNumberType::Signed>;
			BB a;
			a.clear();
			a.setbit(127);                      // -2^127
			check(to_decimal(a), "-170141183460469231731687303715884105728", "signed<128> maxneg");

			a.clear();
			for (unsigned i = 0; i < 127; ++i) a.setbit(i);   // 2^127 - 1
			check(to_decimal(a), "170141183460469231731687303715884105727", "signed<128> maxpos");

			a.clear();
			a.setbit(64);                       // 2^64, one past what a native type holds
			check(to_decimal(a), "18446744073709551616", "signed<128> 2^64");

			a.clear();
			a.setbit(100);                      // 2^100 + 12345
			a.setbit(13); a.setbit(12); a.setbit(5); a.setbit(4); a.setbit(3); a.setbit(0);
			check(to_decimal(a), "1267650600228229401496703217721", "signed<128> 2^100 + 12345");

			BB b(-1);
			check(to_decimal(b), "-1", "signed<128> -1");
			BB c(0);
			check(to_decimal(c), "0", "signed<128> zero");
		}

		{   // unsigned, 128 bits: the instantiation that did not compile
			using BB = blockbinary<128, std::uint32_t, BinaryNumberType::Unsigned>;
			BB a;
			a.clear();
			for (unsigned i = 0; i < 128; ++i) a.setbit(i);   // 2^128 - 1
			check(to_decimal(a), "340282366920938463463374607431768211455", "unsigned<128> maxpos");

			a.clear();
			a.setbit(64);
			check(to_decimal(a), "18446744073709551616", "unsigned<128> 2^64");

			BB b(42);
			check(to_decimal(b), "42", "unsigned<128> 42");
		}

		{   // the 64-bit boundary, where the old implementation switched paths
			blockbinary<64, std::uint32_t, BinaryNumberType::Signed> a;
			a.clear();
			a.setbit(63);
			check(to_decimal(a), "-9223372036854775808", "signed<64> maxneg");

			blockbinary<64, std::uint32_t, BinaryNumberType::Unsigned> b;
			b.clear();
			for (unsigned i = 0; i < 64; ++i) b.setbit(i);
			check(to_decimal(b), "18446744073709551615", "unsigned<64> maxpos");

			blockbinary<65, std::uint32_t, BinaryNumberType::Unsigned> c;
			c.clear();
			c.setbit(64);
			check(to_decimal(c), "18446744073709551616", "unsigned<65> 2^64");
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

	std::string test_suite  = "blockbinary decimal conversion";
	std::string test_tag    = "to_decimal";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	{
		blockbinary<128, std::uint32_t, BinaryNumberType::Signed> a;
		a.clear();
		a.setbit(127);
		std::cout << "signed<128> most negative : " << to_decimal(a) << '\n';
		blockbinary<128, std::uint32_t, BinaryNumberType::Unsigned> b(42);
		std::cout << "unsigned<128> 42          : " << to_decimal(b) << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyToDecimal<8, std::uint8_t, BinaryNumberType::Signed>(reportTestCases), "blockbinary<8, uint8_t, Signed>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyWideToDecimal(reportTestCases), "blockbinary<64|65|128>", test_tag);

	nrOfFailedTestCases += ReportTestResult(VerifyToDecimal<8, std::uint8_t, BinaryNumberType::Signed>(reportTestCases), "blockbinary<8, uint8_t, Signed>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToDecimal<8, std::uint8_t, BinaryNumberType::Unsigned>(reportTestCases), "blockbinary<8, uint8_t, Unsigned>", test_tag);
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifyToDecimal<12, std::uint8_t, BinaryNumberType::Signed>(reportTestCases), "blockbinary<12, uint8_t, Signed>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToDecimal<12, std::uint16_t, BinaryNumberType::Unsigned>(reportTestCases), "blockbinary<12, uint16_t, Unsigned>", test_tag);
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifyToDecimal<16, std::uint8_t, BinaryNumberType::Signed>(reportTestCases), "blockbinary<16, uint8_t, Signed>", test_tag);
#endif

#if REGRESSION_LEVEL_4
	nrOfFailedTestCases += ReportTestResult(VerifyToDecimal<16, std::uint16_t, BinaryNumberType::Unsigned>(reportTestCases), "blockbinary<16, uint16_t, Unsigned>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyToDecimal<16, std::uint32_t, BinaryNumberType::Signed>(reportTestCases), "blockbinary<16, uint32_t, Signed>", test_tag);
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
