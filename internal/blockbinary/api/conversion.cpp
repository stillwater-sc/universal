//  conversion.cpp : test suite runner for blockbinary construction and conversion of blockbinary
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <iostream>

#include <universal/internal/blockbinary/blockbinary.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "blockbinary conversion validation";
	std::string test_tag    = "conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		// scenario that happens in unrounded add/sub where blockbinary is used as storage type for fraction or significant
		constexpr size_t fbits = 8;
		constexpr size_t fhbits = fbits + 1;
		constexpr size_t abits = fhbits + 3;
		constexpr size_t sumbits = abits + 1;
		size_t msbMask = 1;
		blockbinary<fhbits, uint8_t> a;
		for (size_t i = 0; i < fbits; ++i) {
			a.setbits(msbMask);
			blockbinary<sumbits, uint8_t> b(a);
			std::cout << to_binary(a, true) << '\n';
			std::cout << to_binary(b, true) << '\n';
			msbMask <<= 1;
		}
	}

	// Cross-template (widening) copy ctor: Signed must sign-extend, Unsigned must
	// zero-extend. Regression for the assign() bug surfaced on PR #760: widening
	// 0x80 (Unsigned 8-bit) was sign-extending to 0xFF80 instead of 0x0080.
	{
		std::cout << "+----- cross-template widening (Signed sign-extend, Unsigned zero-extend)\n";

		const int failures_before = nrOfFailedTestCases;
		auto check = [&](const char* name, bool ok) {
			if (!ok) { ++nrOfFailedTestCases; std::cout << "FAIL " << name << '\n'; }
		};

		// Signed: negative widens by sign-extension
		{
			using S8  = blockbinary<8,  std::uint8_t, BinaryNumberType::Signed>;
			using S16 = blockbinary<16, std::uint8_t, BinaryNumberType::Signed>;
			S16 sx_neg(S8(-1));
			check("Signed widen -1: low byte == 0xFF",  sx_neg.block(0) == 0xFF);
			check("Signed widen -1: high byte == 0xFF (sign-ext)", sx_neg.block(1) == 0xFF);
			check("Signed widen -1: to_sll() == -1",    sx_neg.to_sll() == -1LL);

			// Signed: positive (high bit clear) -- no extension, high bytes are zero
			S16 sx_pos(S8(0x7F));
			check("Signed widen 0x7F: low byte == 0x7F", sx_pos.block(0) == 0x7F);
			check("Signed widen 0x7F: high byte == 0x00", sx_pos.block(1) == 0x00);
			check("Signed widen 0x7F: to_sll() == 127",   sx_pos.to_sll() == 127LL);
		}

		// Unsigned: high bit is data, MUST zero-extend (regression: was sign-extending)
		{
			using U8  = blockbinary<8,  std::uint8_t, BinaryNumberType::Unsigned>;
			using U16 = blockbinary<16, std::uint8_t, BinaryNumberType::Unsigned>;
			U16 zx_msb(U8(0x80));
			check("Unsigned widen 0x80: low byte == 0x80",  zx_msb.block(0) == 0x80);
			check("Unsigned widen 0x80: high byte == 0x00 (zero-ext, NOT 0xFF)", zx_msb.block(1) == 0x00);

			U16 zx_all(U8(0xFF));
			check("Unsigned widen 0xFF: low byte == 0xFF",  zx_all.block(0) == 0xFF);
			check("Unsigned widen 0xFF: high byte == 0x00 (zero-ext, NOT 0xFF)", zx_all.block(1) == 0x00);
		}

		// Multi-block widening (uint16 limbs, 32 -> 64 bits): same Signed/Unsigned semantics
		{
			using U32 = blockbinary<32, std::uint16_t, BinaryNumberType::Unsigned>;
			using U64 = blockbinary<64, std::uint16_t, BinaryNumberType::Unsigned>;
			U32 src; src.setbits(0xFFFFFFFFull);
			U64 widened(src);
			check("Unsigned 32->64 widen 0xFFFFFFFF: low limb",        widened.block(0) == 0xFFFF);
			check("Unsigned 32->64 widen 0xFFFFFFFF: limb 1",          widened.block(1) == 0xFFFF);
			check("Unsigned 32->64 widen 0xFFFFFFFF: limb 2 zero",     widened.block(2) == 0x0000);
			check("Unsigned 32->64 widen 0xFFFFFFFF: limb 3 zero",     widened.block(3) == 0x0000);
		}

		// Sanity: blockdecimal-flavored use (blockbinary<N, bt, Unsigned> via uint64 limbs)
		{
			using U64u64 = blockbinary<64,  std::uint64_t, BinaryNumberType::Unsigned>;
			using U128u64 = blockbinary<128, std::uint64_t, BinaryNumberType::Unsigned>;
			U64u64 src; src.setbits(static_cast<uint64_t>(-1));  // all 64 bits set
			U128u64 widened(src);
			check("Unsigned 64->128 widen UINT64_MAX: low limb is all-ones",  widened.block(0) == static_cast<uint64_t>(-1));
			check("Unsigned 64->128 widen UINT64_MAX: high limb is zero",     widened.block(1) == 0);
		}

		if (nrOfFailedTestCases == failures_before) {
			std::cout << "+----- cross-template widening regression: PASS\n";
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
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
