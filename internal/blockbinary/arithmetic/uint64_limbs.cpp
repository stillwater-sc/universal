// uint64_limbs.cpp: functional tests for blockbinary with uint64_t limb arithmetic
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// uint64_t multi-block arithmetic uses carry-detection intrinsics (carry.hpp)
// rather than casting to a wider type. Since we cannot exhaustively test 128/256-bit
// configurations, these tests focus on carry/borrow boundary conditions:
//   - carry propagation across limb boundaries
//   - borrow propagation across limb boundaries
//   - single-carry vs multi-carry chains
//   - multiplication cross-limb partial products
//   - edge values: all-ones limbs, single-bit limbs, maxpos, maxneg
//
// Each test is cross-validated against uint8_t limbs (which use the proven
// cast-to-uint64_t path) for identical small values, and against __int128
// or manual construction for large values.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>
#include <iomanip>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/blockbinary_test_status.hpp>

// Verify that blockbinary<nbits, uint64_t> produces the same results as
// blockbinary<nbits, uint8_t> for all values representable by setbits(uint64_t).
// This cross-validates the uint64_t intrinsic path against the proven uint8_t path.
template<unsigned nbits>
int VerifyCrossAddition(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	// Boundary values that stress carry propagation
	const uint64_t testValues[] = {
		0ULL,
		1ULL,
		2ULL,
		0x7FULL,                       // max int8
		0x80ULL,                       // carry into bit 7
		0xFFULL,                       // all-ones byte
		0x100ULL,                      // carry past byte boundary
		0xFFFFULL,                     // all-ones 16-bit
		0x10000ULL,                    // carry past 16-bit boundary
		0x7FFFFFFFULL,                 // max int32
		0x80000000ULL,                 // carry into bit 31
		0xFFFFFFFFULL,                 // all-ones 32-bit
		0x100000000ULL,                // carry past 32-bit boundary
		0x7FFFFFFFFFFFFFFFULL,         // max int64
		0x8000000000000000ULL,         // carry into bit 63 (MSB of first limb)
		0xFFFFFFFFFFFFFFFFULL,         // all-ones 64-bit (max single limb)
		0xFFFFFFFFFFFFFFFEULL,         // max limb - 1
		0xDEADBEEFCAFEBABEULL,         // arbitrary large value
		0x0123456789ABCDEFULL,         // another arbitrary value
	};
	constexpr unsigned nrTestValues = sizeof(testValues) / sizeof(testValues[0]);

	blockbinary<nbits, uint64_t> a64, b64, result64;
	blockbinary<nbits, uint8_t>  a8, b8, result8;

	for (unsigned i = 0; i < nrTestValues; ++i) {
		for (unsigned j = 0; j < nrTestValues; ++j) {
			a64.setbits(testValues[i]);
			b64.setbits(testValues[j]);
			a8.setbits(testValues[i]);
			b8.setbits(testValues[j]);

			result64 = a64 + b64;
			result8 = a8 + b8;

			// compare block by block
			bool match = true;
			for (unsigned k = 0; k < nbits; ++k) {
				if (result64.test(k) != result8.test(k)) {
					match = false;
					break;
				}
			}
			if (!match) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) {
					std::cerr << "FAIL add: uint64[" << to_hex(a64) << " + " << to_hex(b64) << "] = " << to_hex(result64)
					          << " vs uint8[" << to_hex(a8) << " + " << to_hex(b8) << "] = " << to_hex(result8) << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// Cross-validate subtraction between uint64_t and uint8_t limb paths
template<unsigned nbits>
int VerifyCrossSubtraction(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	const uint64_t testValues[] = {
		0ULL, 1ULL, 2ULL,
		0xFFULL, 0x100ULL, 0xFFFFULL, 0x10000ULL,
		0xFFFFFFFFULL, 0x100000000ULL,
		0x7FFFFFFFFFFFFFFFULL, 0x8000000000000000ULL,
		0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFEULL,
		0xDEADBEEFCAFEBABEULL, 0x0123456789ABCDEFULL,
	};
	constexpr unsigned nrTestValues = sizeof(testValues) / sizeof(testValues[0]);

	blockbinary<nbits, uint64_t> a64, b64, result64;
	blockbinary<nbits, uint8_t>  a8, b8, result8;

	for (unsigned i = 0; i < nrTestValues; ++i) {
		for (unsigned j = 0; j < nrTestValues; ++j) {
			a64.setbits(testValues[i]);
			b64.setbits(testValues[j]);
			a8.setbits(testValues[i]);
			b8.setbits(testValues[j]);

			result64 = a64 - b64;
			result8 = a8 - b8;

			bool match = true;
			for (unsigned k = 0; k < nbits; ++k) {
				if (result64.test(k) != result8.test(k)) {
					match = false;
					break;
				}
			}
			if (!match) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) {
					std::cerr << "FAIL sub: uint64[" << to_hex(a64) << " - " << to_hex(b64) << "] = " << to_hex(result64)
					          << " vs uint8[" << to_hex(a8) << " - " << to_hex(b8) << "] = " << to_hex(result8) << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// Cross-validate multiplication between uint64_t and uint8_t limb paths
template<unsigned nbits>
int VerifyCrossMultiplication(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	const uint64_t testValues[] = {
		0ULL, 1ULL, 2ULL, 3ULL,
		0x7FULL, 0x80ULL, 0xFFULL,
		0x100ULL, 0xFFFFULL, 0x10000ULL,
		0xFFFFFFFFULL, 0x100000000ULL,
		0x7FFFFFFFFFFFFFFFULL, 0x8000000000000000ULL,
		0xFFFFFFFFFFFFFFFFULL,
		0xDEADBEEFULL,                  // fits in 32 bits: stresses cross-limb products
		0x0123456789ABCDEFULL,
	};
	constexpr unsigned nrTestValues = sizeof(testValues) / sizeof(testValues[0]);

	blockbinary<nbits, uint64_t> a64, b64, result64;
	blockbinary<nbits, uint8_t>  a8, b8, result8;

	for (unsigned i = 0; i < nrTestValues; ++i) {
		for (unsigned j = 0; j < nrTestValues; ++j) {
			a64.setbits(testValues[i]);
			b64.setbits(testValues[j]);
			a8.setbits(testValues[i]);
			b8.setbits(testValues[j]);

			result64 = a64 * b64;
			result8 = a8 * b8;

			bool match = true;
			for (unsigned k = 0; k < nbits; ++k) {
				if (result64.test(k) != result8.test(k)) {
					match = false;
					break;
				}
			}
			if (!match) {
				++nrOfFailedTests;
				if (bReportIndividualTestCases) {
					std::cerr << "FAIL mul: uint64[" << to_hex(a64) << " * " << to_hex(b64) << "] = " << to_hex(result64)
					          << " vs uint8[" << to_hex(a8) << " * " << to_hex(b8) << "] = " << to_hex(result8) << '\n';
				}
			}
		}
	}
	return nrOfFailedTests;
}

// Test carry propagation across all limb boundaries for 128-bit and 256-bit
// by adding 1 to all-ones patterns that span different numbers of limbs
template<unsigned nbits>
int VerifyCarryChain(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	constexpr unsigned nrLimbs = (nbits + 63) / 64;
	blockbinary<nbits, uint64_t> a, one(1), result;

	// Test: set limbs 0..k to all-ones, add 1, verify carry propagates into limb k+1
	for (unsigned k = 0; k < nrLimbs - 1; ++k) {
		a.clear();
		for (unsigned limb = 0; limb <= k; ++limb) {
			a.setblock(limb, 0xFFFFFFFFFFFFFFFFULL);
		}
		result = a + one;

		// All limbs 0..k should be zero, limb k+1 should have bit 0 set
		bool pass = true;
		for (unsigned limb = 0; limb <= k; ++limb) {
			if (result.block(limb) != 0) {
				pass = false;
				break;
			}
		}
		if (k + 1 < nrLimbs && result.block(k + 1) != (a.block(k + 1) + 1)) {
			pass = false;
		}
		if (!pass) {
			++nrOfFailedTests;
			if (bReportIndividualTestCases) {
				std::cerr << "FAIL carry chain: " << (k + 1) << " limbs of 0xFF..FF + 1\n";
				std::cerr << "  a      = " << to_hex(a) << '\n';
				std::cerr << "  result = " << to_hex(result) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// Test borrow propagation: subtract 1 from a value with zeros in lower limbs
// e.g., 0x0000_0000_0000_0001'0000_0000_0000_0000 - 1 = 0x0000_0000_0000_0000'FFFF_FFFF_FFFF_FFFF
template<unsigned nbits>
int VerifyBorrowChain(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	constexpr unsigned nrLimbs = (nbits + 63) / 64;
	blockbinary<nbits, uint64_t> a, one(1), result;

	// Test: set only limb k to 1, all lower limbs zero, subtract 1
	// Result should have limbs 0..k-1 all 0xFFFF..FFFF, limb k = 0
	for (unsigned k = 1; k < nrLimbs; ++k) {
		a.clear();
		a.setblock(k, 1);
		result = a - one;

		bool pass = true;
		for (unsigned limb = 0; limb < k; ++limb) {
			if (result.block(limb) != 0xFFFFFFFFFFFFFFFFULL) {
				pass = false;
				break;
			}
		}
		if (result.block(k) != 0) {
			pass = false;
		}
		if (!pass) {
			++nrOfFailedTests;
			if (bReportIndividualTestCases) {
				std::cerr << "FAIL borrow chain: limb[" << k << "]=1 minus 1\n";
				std::cerr << "  a      = " << to_hex(a) << '\n';
				std::cerr << "  result = " << to_hex(result) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// Verify multiplication where partial products span limb boundaries
// Uses specific values known to stress the mul128 + addcarry accumulation
template<unsigned nbits>
int VerifyCrossLimbMultiplication(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	blockbinary<nbits, uint64_t> a64, b64, result64;
	blockbinary<nbits, uint8_t>  a8, b8, result8;

	struct TestCase {
		uint64_t a_lo, a_hi;  // two limbs of a
		uint64_t b_lo, b_hi;  // two limbs of b
	};

	// These cases are designed to exercise:
	// 1. lo*lo overflow into hi limb
	// 2. lo*hi + hi*lo cross-terms with carry
	// 3. max limb values producing maximum carry chains
	TestCase cases[] = {
		// a_lo, a_hi, b_lo, b_hi
		{ 0xFFFFFFFFFFFFFFFFULL, 0, 2, 0 },                              // max_limb * 2: tests lo*lo carry
		{ 0xFFFFFFFFFFFFFFFFULL, 0, 0xFFFFFFFFFFFFFFFFULL, 0 },          // max_limb^2: max carry from lo*lo
		{ 0, 1, 0, 1 },                                                   // 2^64 * 2^64 = 2^128 (overflows in 128-bit)
		{ 1, 1, 1, 1 },                                                   // (2^64+1) * (2^64+1)
		{ 0xFFFFFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFFFULL, 1, 0 },          // max128 * 1
		{ 0xFFFFFFFFFFFFFFFFULL, 0x7FFFFFFFFFFFFFFFULL, 2, 0 },          // near-maxpos * 2
		{ 0x8000000000000000ULL, 0, 0x8000000000000000ULL, 0 },          // 2^63 * 2^63 = 2^126
		{ 0xDEADBEEFCAFEBABEULL, 0, 0x0123456789ABCDEFULL, 0 },         // arbitrary large * large
		{ 0xFFFFFFFF00000001ULL, 0, 0xFFFFFFFF00000001ULL, 0 },          // values near 2^64 with structure
	};
	constexpr unsigned nrCases = sizeof(cases) / sizeof(cases[0]);

	for (unsigned t = 0; t < nrCases; ++t) {
		a64.clear(); b64.clear(); a8.clear(); b8.clear();
		// Set limbs for uint64_t version
		a64.setblock(0, cases[t].a_lo);
		if constexpr (nbits > 64) a64.setblock(1, cases[t].a_hi);
		b64.setblock(0, cases[t].b_lo);
		if constexpr (nbits > 64) b64.setblock(1, cases[t].b_hi);

		// Set the same bit pattern for uint8_t version
		for (unsigned bit = 0; bit < nbits; ++bit) {
			a8.setbit(bit, a64.test(bit));
			b8.setbit(bit, b64.test(bit));
		}

		result64 = a64 * b64;
		result8 = a8 * b8;

		bool match = true;
		for (unsigned k = 0; k < nbits; ++k) {
			if (result64.test(k) != result8.test(k)) {
				match = false;
				break;
			}
		}
		if (!match) {
			++nrOfFailedTests;
			if (bReportIndividualTestCases) {
				std::cerr << "FAIL cross-limb mul case " << t << ": "
				          << to_hex(a64) << " * " << to_hex(b64) << '\n';
				std::cerr << "  uint64 result: " << to_hex(result64) << '\n';
				std::cerr << "  uint8  result: " << to_hex(result8) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// Verify increment and twosComplement at limb boundaries
template<unsigned nbits>
int VerifyIncrementBoundaries(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	blockbinary<nbits, uint64_t> a64;
	blockbinary<nbits, uint8_t>  a8;

	// Values where increment causes carry propagation across limbs
	const uint64_t testValues[] = {
		0ULL, 1ULL,
		0xFEULL, 0xFFULL,
		0xFFFEULL, 0xFFFFULL,
		0xFFFFFFFEULL, 0xFFFFFFFFULL,
		0xFFFFFFFFFFFFFFFEULL, 0xFFFFFFFFFFFFFFFFULL,
		0x7FFFFFFFFFFFFFFFULL,
	};
	constexpr unsigned nrTestValues = sizeof(testValues) / sizeof(testValues[0]);

	for (unsigned i = 0; i < nrTestValues; ++i) {
		a64.setbits(testValues[i]);
		a8.setbits(testValues[i]);

		++a64;
		++a8;

		bool match = true;
		for (unsigned k = 0; k < nbits; ++k) {
			if (a64.test(k) != a8.test(k)) {
				match = false;
				break;
			}
		}
		if (!match) {
			++nrOfFailedTests;
			if (bReportIndividualTestCases) {
				std::cerr << "FAIL increment: value 0x" << std::hex << testValues[i] << std::dec
				          << " uint64=" << to_hex(a64) << " uint8=" << to_hex(a8) << '\n';
			}
		}
	}

	// Test twosComplement at boundary values
	for (unsigned i = 0; i < nrTestValues; ++i) {
		a64.setbits(testValues[i]);
		a8.setbits(testValues[i]);

		blockbinary<nbits, uint64_t> neg64 = -a64;
		blockbinary<nbits, uint8_t>  neg8 = -a8;

		bool match = true;
		for (unsigned k = 0; k < nbits; ++k) {
			if (neg64.test(k) != neg8.test(k)) {
				match = false;
				break;
			}
		}
		if (!match) {
			++nrOfFailedTests;
			if (bReportIndividualTestCases) {
				std::cerr << "FAIL twosComplement: value 0x" << std::hex << testValues[i] << std::dec
				          << " uint64=" << to_hex(neg64) << " uint8=" << to_hex(neg8) << '\n';
			}
		}
	}

	return nrOfFailedTests;
}

// Verify division at limb boundaries (division uses subtraction internally)
template<unsigned nbits>
int VerifyCrossDivision(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	blockbinary<nbits, uint64_t> a64, b64, result64;
	blockbinary<nbits, uint8_t>  a8, b8, result8;

	// dividend/divisor pairs that stress borrow chains in the long division subtract loop
	struct DivCase { uint64_t a; uint64_t b; };
	DivCase cases[] = {
		{ 100, 3 },
		{ 0xFFFFFFFF, 7 },
		{ 0xFFFFFFFF, 0xFFFF },
		{ 0x100000000ULL, 0x10000ULL },
		{ 0x7FFFFFFFFFFFFFFFULL, 127 },
		{ 0x7FFFFFFFFFFFFFFFULL, 0x7FFFFFFFULL },
		{ 1000000007ULL, 13 },
		{ 0xDEADBEEFCAFEBABEULL, 0x12345ULL },
	};
	constexpr unsigned nrCases = sizeof(cases) / sizeof(cases[0]);

	for (unsigned t = 0; t < nrCases; ++t) {
		a64.setbits(cases[t].a); b64.setbits(cases[t].b);
		a8.setbits(cases[t].a);  b8.setbits(cases[t].b);

		result64 = a64 / b64;
		result8 = a8 / b8;

		bool match = true;
		for (unsigned k = 0; k < nbits; ++k) {
			if (result64.test(k) != result8.test(k)) {
				match = false;
				break;
			}
		}
		if (!match) {
			++nrOfFailedTests;
			if (bReportIndividualTestCases) {
				std::cerr << "FAIL div: " << to_hex(a64) << " / " << to_hex(b64)
				          << " uint64=" << to_hex(result64) << " uint8=" << to_hex(result8) << '\n';
			}
		}
	}
	return nrOfFailedTests;
}

// Exhaustive cross-validation for small bit widths where uint64_t is still a single block
// but we can verify the operator= and setbits paths work correctly
template<unsigned nbits>
int VerifyExhaustiveSmall(bool bReportIndividualTestCases) {
	using namespace sw::universal;
	constexpr unsigned NR_VALUES = (1u << nbits);
	int nrOfFailedTests = 0;

	blockbinary<nbits, uint64_t> a64, b64, result64;
	blockbinary<nbits, uint8_t>  a8, b8, result8;

	for (unsigned i = 0; i < NR_VALUES; ++i) {
		for (unsigned j = 0; j < NR_VALUES; ++j) {
			a64.setbits(i); b64.setbits(j);
			a8.setbits(i);  b8.setbits(j);

			// addition
			result64 = a64 + b64;
			result8 = a8 + b8;
			for (unsigned k = 0; k < nbits; ++k) {
				if (result64.test(k) != result8.test(k)) {
					++nrOfFailedTests;
					if (bReportIndividualTestCases) {
						std::cerr << "FAIL small add: " << i << " + " << j << '\n';
					}
					break;
				}
			}

			// multiplication
			result64 = a64 * b64;
			result8 = a8 * b8;
			for (unsigned k = 0; k < nbits; ++k) {
				if (result64.test(k) != result8.test(k)) {
					++nrOfFailedTests;
					if (bReportIndividualTestCases) {
						std::cerr << "FAIL small mul: " << i << " * " << j << '\n';
					}
					break;
				}
			}
		}
	}
	return nrOfFailedTests;
}

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	if (argc > 1) std::cout << argv[0] << std::endl;

	bool bReportIndividualTestCases = true;
	int nrOfFailedTestCases = 0;

	std::string tag = "blockbinary uint64_t limb arithmetic";

#if MANUAL_TESTING

	// hand-trace specific cases here if needed
	{
		blockbinary<128, uint64_t> a(0), b(1);
		a.setblock(0, 0xFFFFFFFFFFFFFFFFULL);
		auto c = a + b;
		std::cout << "128-bit carry: " << to_hex(a) << " + " << to_hex(b) << " = " << to_hex(c) << '\n';
	}

	nrOfFailedTestCases += ReportTestResult(VerifyCrossAddition<128>(true), "blockbinary<128,uint64_t>", "cross-add");

#else

	std::cout << "blockbinary uint64_t limb arithmetic validation\n";

	// Section 1: Exhaustive cross-validation for small bit widths (single-block uint64_t)
	// These validate that operator=, setbits, and single-block arithmetic work with uint64_t
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveSmall<4>(bReportIndividualTestCases), "blockbinary<4,uint64_t>", "exhaustive cross");
	nrOfFailedTestCases += ReportTestResult(VerifyExhaustiveSmall<8>(bReportIndividualTestCases), "blockbinary<8,uint64_t>", "exhaustive cross");

	// Section 2: Cross-validation of addition with boundary values (multi-block)
	nrOfFailedTestCases += ReportTestResult(VerifyCrossAddition<128>(bReportIndividualTestCases), "blockbinary<128,uint64_t>", "cross-add");
	nrOfFailedTestCases += ReportTestResult(VerifyCrossAddition<256>(bReportIndividualTestCases), "blockbinary<256,uint64_t>", "cross-add");

	// Section 3: Cross-validation of subtraction with boundary values
	nrOfFailedTestCases += ReportTestResult(VerifyCrossSubtraction<128>(bReportIndividualTestCases), "blockbinary<128,uint64_t>", "cross-sub");
	nrOfFailedTestCases += ReportTestResult(VerifyCrossSubtraction<256>(bReportIndividualTestCases), "blockbinary<256,uint64_t>", "cross-sub");

	// Section 4: Cross-validation of multiplication with boundary values
	nrOfFailedTestCases += ReportTestResult(VerifyCrossMultiplication<128>(bReportIndividualTestCases), "blockbinary<128,uint64_t>", "cross-mul");
	nrOfFailedTestCases += ReportTestResult(VerifyCrossMultiplication<256>(bReportIndividualTestCases), "blockbinary<256,uint64_t>", "cross-mul");

	// Section 5: Carry chain propagation tests
	nrOfFailedTestCases += ReportTestResult(VerifyCarryChain<128>(bReportIndividualTestCases), "blockbinary<128,uint64_t>", "carry-chain");
	nrOfFailedTestCases += ReportTestResult(VerifyCarryChain<256>(bReportIndividualTestCases), "blockbinary<256,uint64_t>", "carry-chain");

	// Section 6: Borrow chain propagation tests
	nrOfFailedTestCases += ReportTestResult(VerifyBorrowChain<128>(bReportIndividualTestCases), "blockbinary<128,uint64_t>", "borrow-chain");
	nrOfFailedTestCases += ReportTestResult(VerifyBorrowChain<256>(bReportIndividualTestCases), "blockbinary<256,uint64_t>", "borrow-chain");

	// Section 7: Cross-limb multiplication (multi-limb operands)
	nrOfFailedTestCases += ReportTestResult(VerifyCrossLimbMultiplication<128>(bReportIndividualTestCases), "blockbinary<128,uint64_t>", "cross-limb-mul");
	nrOfFailedTestCases += ReportTestResult(VerifyCrossLimbMultiplication<256>(bReportIndividualTestCases), "blockbinary<256,uint64_t>", "cross-limb-mul");

	// Section 8: Increment and twosComplement boundaries
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementBoundaries<128>(bReportIndividualTestCases), "blockbinary<128,uint64_t>", "increment");
	nrOfFailedTestCases += ReportTestResult(VerifyIncrementBoundaries<256>(bReportIndividualTestCases), "blockbinary<256,uint64_t>", "increment");

	// Section 9: Division cross-validation (exercises borrow in long division)
	nrOfFailedTestCases += ReportTestResult(VerifyCrossDivision<128>(bReportIndividualTestCases), "blockbinary<128,uint64_t>", "cross-div");
	nrOfFailedTestCases += ReportTestResult(VerifyCrossDivision<256>(bReportIndividualTestCases), "blockbinary<256,uint64_t>", "cross-div");


	// 512-bit and 1024-bit configurations
	nrOfFailedTestCases += ReportTestResult(VerifyCrossAddition<512>(bReportIndividualTestCases), "blockbinary<512,uint64_t>", "cross-add");
	nrOfFailedTestCases += ReportTestResult(VerifyCrossSubtraction<512>(bReportIndividualTestCases), "blockbinary<512,uint64_t>", "cross-sub");
	nrOfFailedTestCases += ReportTestResult(VerifyCarryChain<512>(bReportIndividualTestCases), "blockbinary<512,uint64_t>", "carry-chain");
	nrOfFailedTestCases += ReportTestResult(VerifyBorrowChain<512>(bReportIndividualTestCases), "blockbinary<512,uint64_t>", "borrow-chain");
	nrOfFailedTestCases += ReportTestResult(VerifyCrossLimbMultiplication<512>(bReportIndividualTestCases), "blockbinary<512,uint64_t>", "cross-limb-mul");

	nrOfFailedTestCases += ReportTestResult(VerifyCrossAddition<1024>(bReportIndividualTestCases), "blockbinary<1024,uint64_t>", "cross-add");
	nrOfFailedTestCases += ReportTestResult(VerifyCrossSubtraction<1024>(bReportIndividualTestCases), "blockbinary<1024,uint64_t>", "cross-sub");
	nrOfFailedTestCases += ReportTestResult(VerifyCarryChain<1024>(bReportIndividualTestCases), "blockbinary<1024,uint64_t>", "carry-chain");
	nrOfFailedTestCases += ReportTestResult(VerifyBorrowChain<1024>(bReportIndividualTestCases), "blockbinary<1024,uint64_t>", "borrow-chain");
	nrOfFailedTestCases += ReportTestResult(VerifyCrossLimbMultiplication<1024>(bReportIndividualTestCases), "blockbinary<1024,uint64_t>", "cross-limb-mul");

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
