//  constexpr.cpp : compile-time tests for constexpr of blockbinary type
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

	std::string test_suite  = "blockbinary constexpr compile-time validation";
	std::string test_tag    = "constexpr";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	{
		constexpr blockbinary<8, uint8_t> b8_1b(0x5555);
		constexpr blockbinary<8, uint16_t> b8_2b(0x5555);
		constexpr blockbinary<8, uint32_t> b8_4b(0x5555);

		std::cout << b8_1b << '\n' << b8_2b << '\n' << b8_4b << '\n';
	}

	{
		constexpr blockbinary<16, uint8_t> b16_2(0x5555);
		constexpr blockbinary<16, uint16_t> b16_1(0x5555);
		constexpr blockbinary<16, uint32_t> b16_4b(0x5555);

		std::cout << b16_1 << '\n' << b16_2 << '\n' << b16_4b << '\n';
	}

	// constexpr arithmetic (issue #715)
	{
		using BB = blockbinary<32, std::uint8_t, BinaryNumberType::Signed>;

		// constexpr operator+= via lambda
		constexpr BB sum = []() { BB t(5); t += BB(7); return t; }();
		static_assert(sum.block(0) == 12, "constexpr blockbinary<32,uint8>(5) += 7 == 12");

		// constexpr operator++
		constexpr BB inc = []() { BB t(0); ++t; ++t; ++t; return t; }();
		static_assert(inc.block(0) == 3, "constexpr blockbinary<32,uint8> ++x3 == 3");

		// constexpr operator<<=
		constexpr BB shl = []() { BB t(1); t <<= 5; return t; }();
		static_assert(shl.block(0) == 32, "constexpr blockbinary<32,uint8>(1) <<= 5 == 32");

		// constexpr operator|=
		constexpr BB orResult = []() { BB t(0xF); t |= BB(0xF0); return t; }();
		static_assert(orResult.block(0) == 0xFF, "constexpr blockbinary<32,uint8>(0xF) |= 0xF0 == 0xFF");

		// constexpr free twosComplement (negation)
		constexpr BB neg = twosComplement(BB(5));
		static_assert(neg.block(0) == 0xFB, "constexpr twosComplement(blockbinary<32,uint8>(5)) low byte == 0xFB");

		// multi-block uint8 carry across limbs
		using BB64u8 = blockbinary<64, std::uint8_t, BinaryNumberType::Signed>;
		constexpr BB64u8 carryAcross = []() { BB64u8 t(123); t += BB64u8(456); return t; }();
		// 123 + 456 = 579 = 0x243
		static_assert(carryAcross.block(0) == 0x43, "constexpr 123+456 low byte");
		static_assert(carryAcross.block(1) == 0x02, "constexpr 123+456 high byte (carry)");

		// uint64 limb arithmetic via std::is_constant_evaluated portable carry path
		using BB128u64 = blockbinary<128, std::uint64_t, BinaryNumberType::Signed>;
		constexpr BB128u64 u128inc = []() { BB128u64 t(0); ++t; return t; }();
		static_assert(u128inc.block(0) == 1, "constexpr 2-limb uint64 ++ low limb");
		static_assert(u128inc.block(1) == 0, "constexpr 2-limb uint64 ++ high limb");

		std::cout << "constexpr arithmetic smoke tests PASS (compile-time)\n";
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
