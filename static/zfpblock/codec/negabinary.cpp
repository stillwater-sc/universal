// negabinary.cpp: unit tests for ZFP negabinary conversion
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

// Verify int2uint / uint2int round-trip for int32
int VerifyNegabinaryRoundTrip32(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	// test specific known values
	int32_t test_values[] = {
		0, 1, -1, 2, -2, 127, -128, 1000, -1000,
		INT32_MAX, INT32_MIN + 1,  // avoid INT32_MIN as negation is UB
		0x7FFFFFFF, -0x7FFFFFFF,
		42, -42, 0x55555555, -0x55555555
	};

	for (int32_t val : test_values) {
		uint32_t u = int2uint<int32_t, uint32_t>(val);
		int32_t  back = uint2int<int32_t, uint32_t>(u);
		if (back != val) {
			std::cerr << tag << " FAIL: val=" << val
			          << " encoded=" << u << " decoded=" << back << '\n';
			++nrOfFailedTests;
		}
	}

	// verify known encoding: 0 → 0xAAAAAAAA ^ 0xAAAAAAAA = 0
	{
		uint32_t u = int2uint<int32_t, uint32_t>(0);
		if (u != 0) {
			std::cerr << tag << " FAIL: int2uint(0) = " << u << " expected 0\n";
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

// Verify int2uint / uint2int round-trip for int64
int VerifyNegabinaryRoundTrip64(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	int64_t test_values[] = {
		0, 1, -1, 2, -2, 1000000LL, -1000000LL,
		INT64_MAX, INT64_MIN + 1,
		0x7FFFFFFFFFFFFFFFLL, -0x7FFFFFFFFFFFFFFFLL,
		42, -42
	};

	for (int64_t val : test_values) {
		uint64_t u = int2uint<int64_t, uint64_t>(val);
		int64_t  back = uint2int<int64_t, uint64_t>(u);
		if (back != val) {
			std::cerr << tag << " FAIL: val=" << val
			          << " encoded=" << u << " decoded=" << back << '\n';
			++nrOfFailedTests;
		}
	}

	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfpblock negabinary conversion tests";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	nrOfFailedTestCases += VerifyNegabinaryRoundTrip32("int32 negabinary");
	nrOfFailedTestCases += VerifyNegabinaryRoundTrip64("int64 negabinary");

	std::cout << (nrOfFailedTestCases == 0 ? "PASS" : "FAIL")
	          << " : " << nrOfFailedTestCases << " failures\n";

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
