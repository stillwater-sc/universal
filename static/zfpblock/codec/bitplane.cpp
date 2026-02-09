// bitplane.cpp: unit tests for ZFP bit-plane encode/decode
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define ZFPBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/zfpblock/zfpblock.hpp>
#include <universal/verification/test_suite.hpp>

#include <cstring>

// Verify bit-plane encode/decode round-trip for uint32 (4 elements)
int VerifyBitplaneRoundTrip4_32(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	uint32_t patterns[][4] = {
		{ 0, 0, 0, 0 },
		{ 1, 2, 3, 4 },
		{ 0xFFFFFFFF, 0, 0xAAAAAAAA, 0x55555555 },
		{ 0x80000000, 0x40000000, 0x20000000, 0x10000000 },
		{ 42, 42, 42, 42 },
		{ 1, 0, 0, 0 },
	};

	for (auto& pat : patterns) {
		uint8_t buffer[64];
		std::memset(buffer, 0, sizeof(buffer));

		zfp_bitstream writer(buffer, sizeof(buffer));
		encode_bitplanes<uint32_t, 4>(writer, pat, 32, sizeof(buffer) * 8);

		zfp_bitstream reader(buffer, sizeof(buffer));
		uint32_t decoded[4];
		decode_bitplanes<uint32_t, 4>(reader, decoded, 32, writer.total_bits());

		for (int i = 0; i < 4; ++i) {
			if (decoded[i] != pat[i]) {
				std::cerr << tag << " FAIL: index " << i
				          << " expected=0x" << std::hex << pat[i]
				          << " got=0x" << decoded[i] << std::dec << '\n';
				++nrOfFailedTests;
			}
		}
	}
	return nrOfFailedTests;
}

// Verify bit-plane encode/decode with 16 elements (2D block)
int VerifyBitplaneRoundTrip16_32(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	uint32_t input[16];
	for (int i = 0; i < 16; ++i) input[i] = static_cast<uint32_t>(i * 0x01010101);

	uint8_t buffer[256];
	std::memset(buffer, 0, sizeof(buffer));

	zfp_bitstream writer(buffer, sizeof(buffer));
	encode_bitplanes<uint32_t, 16>(writer, input, 32, sizeof(buffer) * 8);

	zfp_bitstream reader(buffer, sizeof(buffer));
	uint32_t decoded[16];
	decode_bitplanes<uint32_t, 16>(reader, decoded, 32, writer.total_bits());

	for (int i = 0; i < 16; ++i) {
		if (decoded[i] != input[i]) {
			std::cerr << tag << " FAIL: index " << i
			          << " expected=0x" << std::hex << input[i]
			          << " got=0x" << decoded[i] << std::dec << '\n';
			++nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

// Verify that truncated bit-plane decoding works (fewer bits than full)
int VerifyBitplaneTruncation(const std::string& tag) {
	using namespace sw::universal;
	int nrOfFailedTests = 0;

	uint32_t input[4] = { 0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x9ABCDEF0 };

	uint8_t buffer[64];
	std::memset(buffer, 0, sizeof(buffer));

	// Encode with full precision
	zfp_bitstream writer(buffer, sizeof(buffer));
	size_t full_bits = encode_bitplanes<uint32_t, 4>(writer, input, 32, 32 * 4);

	// Decode with limited bits (half)
	zfp_bitstream reader(buffer, sizeof(buffer));
	uint32_t decoded[4];
	decode_bitplanes<uint32_t, 4>(reader, decoded, 32, full_bits / 2);

	// Just verify it doesn't crash and produces some output
	// With fewer bits, we expect lossy reconstruction
	std::cout << tag << " truncated decode completed OK (no crash)\n";

	return nrOfFailedTests;
}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "zfpblock bit-plane codec tests";
	int nrOfFailedTestCases = 0;

	std::cout << test_suite << '\n';

	nrOfFailedTestCases += VerifyBitplaneRoundTrip4_32("uint32 4-elem bitplane");
	nrOfFailedTestCases += VerifyBitplaneRoundTrip16_32("uint32 16-elem bitplane");
	nrOfFailedTestCases += VerifyBitplaneTruncation("uint32 truncated bitplane");

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
