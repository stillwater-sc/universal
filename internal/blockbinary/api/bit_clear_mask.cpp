//  bit_clear_mask.cpp : test suite for the shared bit_clear_mask limb-mask helper (#1260)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>
#include <iostream>

#include <universal/internal/bit_manipulation.hpp>
#include <universal/verification/test_suite.hpp>

namespace {

	using namespace sw::universal;

	// bit_clear_mask<bt>(i, bitsInBlock) must equal a bt with every bit set except
	// bit (i % bitsInBlock) -- the mask used to clear one bit via (limb & mask).
	template<typename bt>
	int VerifyBitClearMask(const char* tag, bool reportTestCases) {
		constexpr unsigned bitsInBlock = sizeof(bt) * 8u;
		int nrOfFailures = 0;
		for (unsigned i = 0; i < bitsInBlock; ++i) {
			bt observed = bit_clear_mask<bt>(i, bitsInBlock);
			// reference built in 64-bit then explicitly narrowed (the intended value)
			bt expected = static_cast<bt>(~(static_cast<std::uint64_t>(1) << i));
			if (observed != expected) {
				++nrOfFailures;
				if (reportTestCases) std::cout << "    FAIL " << tag << " i=" << i
					<< " observed=0x" << std::hex << static_cast<std::uint64_t>(observed)
					<< " expected=0x" << static_cast<std::uint64_t>(expected) << std::dec << '\n';
			}
		}
		// i beyond bitsInBlock must wrap via the % reduction (matches the call sites)
		if (bit_clear_mask<bt>(bitsInBlock + 3u, bitsInBlock) != bit_clear_mask<bt>(3u, bitsInBlock)) {
			++nrOfFailures;
			if (reportTestCases) std::cout << "    FAIL " << tag << " modulo reduction\n";
		}
		return nrOfFailures;
	}

}  // anonymous namespace

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "bit_clear_mask limb-mask helper (#1260)";
	std::string test_tag    = "bit_clear_mask";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// compile-time: the helper is constexpr and the truncation is exact
	static_assert(bit_clear_mask<std::uint8_t>(0, 8) == std::uint8_t(0xFE), "bit_clear_mask<uint8_t>(0) must be 0xFE");
	static_assert(bit_clear_mask<std::uint8_t>(7, 8) == std::uint8_t(0x7F), "bit_clear_mask<uint8_t>(7) must be 0x7F");
	static_assert(bit_clear_mask<std::uint16_t>(15, 16) == std::uint16_t(0x7FFF), "bit_clear_mask<uint16_t>(15)");

	nrOfFailedTestCases += ReportTestResult(VerifyBitClearMask<std::uint8_t>("uint8_t", reportTestCases), "bit_clear_mask<uint8_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyBitClearMask<std::uint16_t>("uint16_t", reportTestCases), "bit_clear_mask<uint16_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyBitClearMask<std::uint32_t>("uint32_t", reportTestCases), "bit_clear_mask<uint32_t>", test_tag);
	nrOfFailedTestCases += ReportTestResult(VerifyBitClearMask<std::uint64_t>("uint64_t", reportTestCases), "bit_clear_mask<uint64_t>", test_tag);

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
