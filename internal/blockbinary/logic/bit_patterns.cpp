// bit_patterns.cpp : regression test for bit pattern queries (any, none, all, anyAfter, count)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>
#include <string>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is set here
#define MANUAL_TESTING 0
// REGRESSION_LEVEL controls test depth
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 0
#define REGRESSION_LEVEL_3 0
#define REGRESSION_LEVEL_4 0

namespace sw { namespace universal {

/// Verify any() returns false for zero and true for non-zero across all bit patterns
template<unsigned nbits, typename BlockType>
int VerifyAny(bool reportTestCases) {
	int nrOfFailedTests = 0;
	constexpr unsigned NR_VALUES = (1u << nbits);
	blockbinary<nbits, BlockType> a;
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		bool expected = (i != 0);
		bool result = a.any();
		if (result != expected) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: any() for " << to_binary(a) << " = " << result
				          << " (expected " << expected << ")\n";
			}
		}
	}
	return nrOfFailedTests;
}

/// Verify none() returns true for zero and false for non-zero across all bit patterns
template<unsigned nbits, typename BlockType>
int VerifyNone(bool reportTestCases) {
	int nrOfFailedTests = 0;
	constexpr unsigned NR_VALUES = (1u << nbits);
	blockbinary<nbits, BlockType> a;
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		bool expected = (i == 0);
		bool result = a.none();
		if (result != expected) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: none() for " << to_binary(a) << " = " << result
				          << " (expected " << expected << ")\n";
			}
		}
	}
	return nrOfFailedTests;
}

/// Verify all() returns true only when every bit within the nbits range is set
template<unsigned nbits, typename BlockType>
int VerifyAll(bool reportTestCases) {
	int nrOfFailedTests = 0;
	constexpr unsigned NR_VALUES = (1u << nbits);
	constexpr unsigned ALL_BITS_SET = NR_VALUES - 1;
	blockbinary<nbits, BlockType> a;
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		bool expected = (i == ALL_BITS_SET);
		bool result = a.all();
		if (result != expected) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: all() for " << to_binary(a) << " = " << result
				          << " (expected " << expected << ")\n";
			}
		}
	}
	return nrOfFailedTests;
}

/// Verify any() and none() are always consistent (never both true, never both false for same value)
template<unsigned nbits, typename BlockType>
int VerifyAnyNoneConsistency(bool reportTestCases) {
	int nrOfFailedTests = 0;
	constexpr unsigned NR_VALUES = (1u << nbits);
	blockbinary<nbits, BlockType> a;
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		bool anyResult = a.any();
		bool noneResult = a.none();
		// any() and none() must be complementary
		if (anyResult == noneResult) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: any()==none() for " << to_binary(a)
				          << " any=" << anyResult << " none=" << noneResult << "\n";
			}
		}
	}
	return nrOfFailedTests;
}

/// Verify anyAfter(bitIndex) by comparing against bit-by-bit reference
template<unsigned nbits, typename BlockType>
int VerifyAnyAfter(bool reportTestCases) {
	int nrOfFailedTests = 0;
	constexpr unsigned NR_VALUES = (1u << nbits);
	blockbinary<nbits, BlockType> a;
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		for (unsigned bitIndex = 0; bitIndex <= nbits; ++bitIndex) {
			// reference: check if any bit below bitIndex is set
			bool expected = false;
			unsigned limit = (bitIndex < nbits) ? bitIndex : nbits;
			for (unsigned b = 0; b < limit; ++b) {
				if (a.test(b)) { expected = true; break; }
			}
			bool result = a.anyAfter(bitIndex);
			if (result != expected) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cerr << "FAIL: anyAfter(" << bitIndex << ") for "
					          << to_binary(a) << " = " << result
					          << " (expected " << expected << ")\n";
				}
			}
		}
	}
	return nrOfFailedTests;
}

/// Verify count() returns the correct popcount for all bit patterns
template<unsigned nbits, typename BlockType>
int VerifyCount(bool reportTestCases) {
	int nrOfFailedTests = 0;
	constexpr unsigned NR_VALUES = (1u << nbits);
	blockbinary<nbits, BlockType> a;
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		// reference popcount
		unsigned expected = 0;
		for (unsigned b = 0; b < nbits; ++b) {
			if (i & (1u << b)) ++expected;
		}
		unsigned result = a.count();
		if (result != expected) {
			++nrOfFailedTests;
			if (reportTestCases) {
				std::cerr << "FAIL: count() for " << to_binary(a) << " = " << result
				          << " (expected " << expected << ")\n";
			}
		}
	}
	return nrOfFailedTests;
}

/// Verify any(msb) - the overload that checks bits [0, msb]
template<unsigned nbits, typename BlockType>
int VerifyAnyMsb(bool reportTestCases) {
	int nrOfFailedTests = 0;
	constexpr unsigned NR_VALUES = (1u << nbits);
	blockbinary<nbits, BlockType> a;
	for (unsigned i = 0; i < NR_VALUES; ++i) {
		a.setbits(i);
		for (unsigned msb = 0; msb < nbits; ++msb) {
			// reference: any bit set in [0..msb]?
			unsigned mask = (1u << (msb + 1)) - 1;
			bool expected = (i & mask) != 0;
			bool result = a.any(msb);
			if (result != expected) {
				++nrOfFailedTests;
				if (reportTestCases) {
					std::cerr << "FAIL: any(" << msb << ") for "
					          << to_binary(a) << " = " << result
					          << " (expected " << expected << ")\n";
				}
			}
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "blockbinary bit pattern verification";
	std::string test_tag = "bit_patterns";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

#if MANUAL_TESTING
	// quick manual smoke test
	{
		blockbinary<8> zero;
		zero.clear();
		std::cout << "zero.any()  = " << zero.any()  << " (expected 0)\n";
		std::cout << "zero.none() = " << zero.none() << " (expected 1)\n";
		std::cout << "zero.all()  = " << zero.all()  << " (expected 0)\n";

		blockbinary<8> allones;
		allones.setbits(0xFF);
		std::cout << "all.any()   = " << allones.any()  << " (expected 1)\n";
		std::cout << "all.none()  = " << allones.none() << " (expected 0)\n";
		std::cout << "all.all()   = " << allones.all()  << " (expected 1)\n";
	}

#else // !MANUAL_TESTING

#if REGRESSION_LEVEL_1
	// any() exhaustive verification across block types
	nrOfFailedTestCases += ReportTestResult(VerifyAny<4, uint8_t>(reportTestCases), "blockbinary<4,uint8_t>", "any()");
	nrOfFailedTestCases += ReportTestResult(VerifyAny<8, uint8_t>(reportTestCases), "blockbinary<8,uint8_t>", "any()");
	nrOfFailedTestCases += ReportTestResult(VerifyAny<8, uint16_t>(reportTestCases), "blockbinary<8,uint16_t>", "any()");
	nrOfFailedTestCases += ReportTestResult(VerifyAny<12, uint8_t>(reportTestCases), "blockbinary<12,uint8_t>", "any()");
	nrOfFailedTestCases += ReportTestResult(VerifyAny<12, uint16_t>(reportTestCases), "blockbinary<12,uint16_t>", "any()");
	nrOfFailedTestCases += ReportTestResult(VerifyAny<16, uint8_t>(reportTestCases), "blockbinary<16,uint8_t>", "any()");
	nrOfFailedTestCases += ReportTestResult(VerifyAny<16, uint16_t>(reportTestCases), "blockbinary<16,uint16_t>", "any()");

	// none() exhaustive verification
	nrOfFailedTestCases += ReportTestResult(VerifyNone<4, uint8_t>(reportTestCases), "blockbinary<4,uint8_t>", "none()");
	nrOfFailedTestCases += ReportTestResult(VerifyNone<8, uint8_t>(reportTestCases), "blockbinary<8,uint8_t>", "none()");
	nrOfFailedTestCases += ReportTestResult(VerifyNone<12, uint8_t>(reportTestCases), "blockbinary<12,uint8_t>", "none()");
	nrOfFailedTestCases += ReportTestResult(VerifyNone<16, uint8_t>(reportTestCases), "blockbinary<16,uint8_t>", "none()");

	// all() exhaustive verification
	nrOfFailedTestCases += ReportTestResult(VerifyAll<4, uint8_t>(reportTestCases), "blockbinary<4,uint8_t>", "all()");
	nrOfFailedTestCases += ReportTestResult(VerifyAll<8, uint8_t>(reportTestCases), "blockbinary<8,uint8_t>", "all()");
	nrOfFailedTestCases += ReportTestResult(VerifyAll<12, uint8_t>(reportTestCases), "blockbinary<12,uint8_t>", "all()");
	nrOfFailedTestCases += ReportTestResult(VerifyAll<16, uint8_t>(reportTestCases), "blockbinary<16,uint8_t>", "all()");

	// any/none consistency
	nrOfFailedTestCases += ReportTestResult(VerifyAnyNoneConsistency<8, uint8_t>(reportTestCases), "blockbinary<8,uint8_t>", "any/none consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyAnyNoneConsistency<12, uint8_t>(reportTestCases), "blockbinary<12,uint8_t>", "any/none consistency");
	nrOfFailedTestCases += ReportTestResult(VerifyAnyNoneConsistency<16, uint16_t>(reportTestCases), "blockbinary<16,uint16_t>", "any/none consistency");

	// anyAfter() exhaustive verification
	nrOfFailedTestCases += ReportTestResult(VerifyAnyAfter<4, uint8_t>(reportTestCases), "blockbinary<4,uint8_t>", "anyAfter()");
	nrOfFailedTestCases += ReportTestResult(VerifyAnyAfter<8, uint8_t>(reportTestCases), "blockbinary<8,uint8_t>", "anyAfter()");
	nrOfFailedTestCases += ReportTestResult(VerifyAnyAfter<12, uint8_t>(reportTestCases), "blockbinary<12,uint8_t>", "anyAfter()");

	// count() exhaustive verification
	nrOfFailedTestCases += ReportTestResult(VerifyCount<4, uint8_t>(reportTestCases), "blockbinary<4,uint8_t>", "count()");
	nrOfFailedTestCases += ReportTestResult(VerifyCount<8, uint8_t>(reportTestCases), "blockbinary<8,uint8_t>", "count()");
	nrOfFailedTestCases += ReportTestResult(VerifyCount<12, uint8_t>(reportTestCases), "blockbinary<12,uint8_t>", "count()");
	nrOfFailedTestCases += ReportTestResult(VerifyCount<16, uint8_t>(reportTestCases), "blockbinary<16,uint8_t>", "count()");

	// any(msb) overload verification
	nrOfFailedTestCases += ReportTestResult(VerifyAnyMsb<4, uint8_t>(reportTestCases), "blockbinary<4,uint8_t>", "any(msb)");
	nrOfFailedTestCases += ReportTestResult(VerifyAnyMsb<8, uint8_t>(reportTestCases), "blockbinary<8,uint8_t>", "any(msb)");
	nrOfFailedTestCases += ReportTestResult(VerifyAnyMsb<12, uint8_t>(reportTestCases), "blockbinary<12,uint8_t>", "any(msb)");
#endif

#if REGRESSION_LEVEL_2
	// larger sizes
	nrOfFailedTestCases += ReportTestResult(VerifyAny<16, uint32_t>(reportTestCases), "blockbinary<16,uint32_t>", "any()");
	nrOfFailedTestCases += ReportTestResult(VerifyNone<16, uint32_t>(reportTestCases), "blockbinary<16,uint32_t>", "none()");
	nrOfFailedTestCases += ReportTestResult(VerifyAll<16, uint32_t>(reportTestCases), "blockbinary<16,uint32_t>", "all()");
	nrOfFailedTestCases += ReportTestResult(VerifyAnyAfter<16, uint8_t>(reportTestCases), "blockbinary<16,uint8_t>", "anyAfter()");
	nrOfFailedTestCases += ReportTestResult(VerifyAnyAfter<16, uint16_t>(reportTestCases), "blockbinary<16,uint16_t>", "anyAfter()");
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);

#endif // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
