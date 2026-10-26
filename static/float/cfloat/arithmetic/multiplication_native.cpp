// multiplication_native.cpp: cfloat single and double multiplication against the native hardware result, for every block type
//
// Guards issue #1743: blocksignificand::mul, the multiplication under every cfloat, did a
// full-width add and shift for every bit of one operand. It is now schoolbook multiplication a
// 32-bit word at a time (limb::multiply_low), which reads and writes the significand through
// 32-bit limbs whatever the block type. cfloat<32,8> and cfloat<64,11> with subnormals,
// without max-exponent values and non-saturating are IEEE-754 single and double, so the native
// product is an exact oracle for every block width: 8, 16, 32 and 64 bits. It also checks
// blocksignificand::mul with the receiver as an operand.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <cstring>
#include <random>
#include <string>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/verification/cfloat_test_suite.hpp>

namespace {

using namespace sw::universal;

// n random products of finite operands, compared bit for bit with the native product. Every
// third pair has a small first operand, reaching the subnormal results; every third multiplies
// two neighbouring encodings.
template<typename Cfloat, typename Native, typename UInt>
int VerifyAgainstNative(const std::string& tag, int n, bool reportTestCases) {
	std::mt19937_64 rng(1743);
	int nrOfFailedTests = 0;
	for (int i = 0; i < n; ++i) {
		UInt ra = static_cast<UInt>(rng());
		UInt rb = static_cast<UInt>(rng());
		if (i % 3 == 1) ra = static_cast<UInt>(ra & ~(UInt(0x7F) << (sizeof(UInt) * 8 - 8)));
		if (i % 3 == 2) rb = static_cast<UInt>(ra ^ UInt(1));
		Native a, b;
		std::memcpy(&a, &ra, sizeof(a));
		std::memcpy(&b, &rb, sizeof(b));
		if (!std::isfinite(a) || !std::isfinite(b)) continue;
		const Native reference = a * b;
		const Native result = Native(Cfloat(a) * Cfloat(b));
		const bool same = (std::memcmp(&result, &reference, sizeof(result)) == 0) || (std::isnan(result) && std::isnan(reference));
		if (!same) {
			++nrOfFailedTests;
			if (reportTestCases && nrOfFailedTests < 10) {
				std::cerr << "FAIL: " << tag << ' ' << std::hexfloat << a << " * " << b << " = " << reference
				          << ", cfloat gives " << result << std::defaultfloat << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

template<typename BlockType>
int VerifySingleAndDouble(const std::string& tag, int n, bool reportTestCases) {
	int nrOfFailedTests = 0;
	nrOfFailedTests += VerifyAgainstNative<cfloat<32, 8, BlockType, true, false, false>, float, std::uint32_t>(tag + " single", n, reportTestCases);
	nrOfFailedTests += VerifyAgainstNative<cfloat<64, 11, BlockType, true, false, false>, double, std::uint64_t>(tag + " double", n, reportTestCases);
	return nrOfFailedTests;
}

// blocksignificand::mul with the receiver as one of its operands, x.mul(x, y) or y.mul(x, y),
// must give the product of the unaliased call: the operands are read before the receiver is
// written
template<unsigned nbits, typename BlockType>
int VerifyAliasedMultiplication(int n, bool reportTestCases) {
	using Significand = blocksignificand<nbits, BlockType>;
	std::mt19937_64 rng(17430);
	int nrOfFailedTests = 0;
	for (int i = 0; i < n; ++i) {
		Significand a, b;
		// operands of half the width, as a blocktriple<MUL> holds them, so the product fits
		for (unsigned k = 0; k < nbits / 2; ++k) { a.setbit(k, (rng() & 1u) != 0); b.setbit(k, (rng() & 1u) != 0); }
		Significand reference;
		reference.mul(a, b);
		Significand x(a), y(b);
		x.mul(x, b);
		y.mul(a, y);
		for (unsigned k = 0; k < Significand::nrBlocks; ++k) {
			if (x.block(k) != reference.block(k) || y.block(k) != reference.block(k)) {
				++nrOfFailedTests;
				if (reportTestCases && nrOfFailedTests < 5) std::cerr << "FAIL: aliased blocksignificand<" << nbits << "> multiplication differs\n";
				break;
			}
		}
	}
	return nrOfFailedTests;
}

}  // namespace

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

	std::string test_suite  = "cfloat multiplication against native";
	std::string test_tag    = "multiplication";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	nrOfFailedTestCases += ReportTestResult(VerifySingleAndDouble<std::uint8_t>("uint8_t", 1000, true), "cfloat uint8_t", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;   // ignore errors
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifySingleAndDouble<std::uint8_t >("uint8_t",  20000, reportTestCases), "cfloat uint8_t",  "single and double");
	nrOfFailedTestCases += ReportTestResult(VerifySingleAndDouble<std::uint16_t>("uint16_t", 20000, reportTestCases), "cfloat uint16_t", "single and double");
	nrOfFailedTestCases += ReportTestResult(VerifySingleAndDouble<std::uint32_t>("uint32_t", 20000, reportTestCases), "cfloat uint32_t", "single and double");
	nrOfFailedTestCases += ReportTestResult(VerifySingleAndDouble<std::uint64_t>("uint64_t", 20000, reportTestCases), "cfloat uint64_t", "single and double");
	nrOfFailedTestCases += ReportTestResult(VerifyAliasedMultiplication< 77, std::uint32_t>(1000, reportTestCases), "blocksignificand<77,uint32_t>", "aliased multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyAliasedMultiplication<162, std::uint8_t >(1000, reportTestCases), "blocksignificand<162,uint8_t>", "aliased multiplication");
	// exhaustive small configurations with a 16-bit block, against the double oracle
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 3, std::uint16_t, true, false, false> >(reportTestCases), "cfloat<10,3,uint16_t,t,f,f>", "multiplication");
	nrOfFailedTestCases += ReportTestResult(VerifyCfloatMultiplication< cfloat<10, 4, std::uint16_t, false, false, false> >(reportTestCases), "cfloat<10,4,uint16_t,f,f,f>", "multiplication");
#endif

#if REGRESSION_LEVEL_2
	nrOfFailedTestCases += ReportTestResult(VerifySingleAndDouble<std::uint8_t >("uint8_t",  200000, reportTestCases), "cfloat uint8_t",  "single and double");
	nrOfFailedTestCases += ReportTestResult(VerifySingleAndDouble<std::uint64_t>("uint64_t", 200000, reportTestCases), "cfloat uint64_t", "single and double");
#endif

#if REGRESSION_LEVEL_3
	nrOfFailedTestCases += ReportTestResult(VerifySingleAndDouble<std::uint16_t>("uint16_t", 1000000, reportTestCases), "cfloat uint16_t", "single and double");
	nrOfFailedTestCases += ReportTestResult(VerifySingleAndDouble<std::uint32_t>("uint32_t", 1000000, reportTestCases), "cfloat uint32_t", "single and double");
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
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Caught unexpected cfloat arithmetic exception : " << err.what() << std::endl;
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
