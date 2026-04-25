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

		// constexpr operator-= (exercises the twosComplement + operator+= delegation chain)
		constexpr BB sub = []() { BB t(10); t -= BB(3); return t; }();
		static_assert(sub.block(0) == 7, "constexpr blockbinary<32,uint8>(10) -= 3 == 7");

		// multi-block uint8 carry across limbs
		using BB64u8 = blockbinary<64, std::uint8_t, BinaryNumberType::Signed>;
		constexpr BB64u8 carryAcross = []() { BB64u8 t(123); t += BB64u8(456); return t; }();
		// 123 + 456 = 579 = 0x243
		static_assert(carryAcross.block(0) == 0x43, "constexpr 123+456 low byte");
		static_assert(carryAcross.block(1) == 0x02, "constexpr 123+456 high byte (carry)");

		// uint64 limb arithmetic via std::is_constant_evaluated portable carry path.
		// To actually validate cross-limb carry propagation, set low limb to
		// UINT64_MAX and add 1 -- correct propagation gives (low=0, high=1).
		using BB128u64 = blockbinary<128, std::uint64_t, BinaryNumberType::Signed>;
		constexpr BB128u64 u128carry = []() {
			BB128u64 t;
			t.setbits(static_cast<uint64_t>(-1));  // low limb = UINT64_MAX, high limb = 0
			t += BB128u64(1);
			return t;
		}();
		static_assert(u128carry.block(0) == 0, "constexpr 2-limb uint64 add: low limb wraps to 0");
		static_assert(u128carry.block(1) == 1, "constexpr 2-limb uint64 add: carry propagates to high limb");

		std::cout << "constexpr arithmetic smoke tests PASS (compile-time)\n";
	}

	// constexpr mul/div/mod (issue #759)
	{
		std::cout << "+----- mul/div/mod (issue #759)\n";

		// Single-block constexpr mul (uint8 limb).
		using BB32u8 = blockbinary<32, std::uint8_t, BinaryNumberType::Signed>;
		constexpr BB32u8 prod32u8 = []() { BB32u8 t(7); t *= BB32u8(11); return t; }();
		static_assert(prod32u8.block(0) == 77, "constexpr blockbinary<32,uint8>(7) *= 11 == 77");

		// Multi-limb constexpr mul (uint8 limbs spanning more than one block).
		using BB64u8 = blockbinary<64, std::uint8_t, BinaryNumberType::Signed>;
		constexpr BB64u8 mlMul = []() { BB64u8 t(1000); t *= BB64u8(1000); return t; }();  // 1_000_000 = 0x0F4240
		static_assert(mlMul.to_sll() == 1000000LL, "constexpr 1000*1000 == 1_000_000 (uint8 multi-limb)");

		// Multi-limb constexpr mul (uint32 limbs).
		using BB128u32 = blockbinary<128, std::uint32_t, BinaryNumberType::Signed>;
		constexpr BB128u32 milProd = []() { BB128u32 t(1000000); t *= BB128u32(1000); return t; }();
		static_assert(milProd.to_sll() == 1000000000LL, "constexpr 1M * 1k == 1B (uint32 multi-limb)");

#if defined(__SIZEOF_INT128__)
		// uint64-limb constexpr mul exercises the __int128-based portable carry path.
		using BB128u64 = blockbinary<128, std::uint64_t, BinaryNumberType::Signed>;
		constexpr BB128u64 u64Prod = []() { BB128u64 t(1ll << 30); t *= BB128u64(1ll << 30); return t; }();
		// 2^30 * 2^30 = 2^60 -- still fits in low 64-bit limb
		static_assert(u64Prod.block(0) == (uint64_t(1) << 60), "constexpr 2^30 * 2^30 == 2^60 (uint64 limb low)");
		static_assert(u64Prod.block(1) == 0,                    "constexpr 2^30 * 2^30 high limb is zero");

		// Force a cross-limb carry: 2^32 * 2^32 = 2^64 -> low=0, high=1.
		constexpr BB128u64 u64CrossCarry = []() { BB128u64 t(1ll << 32); t *= BB128u64(1ll << 32); return t; }();
		static_assert(u64CrossCarry.block(0) == 0, "constexpr 2^32 * 2^32 low limb wraps to 0");
		static_assert(u64CrossCarry.block(1) == 1, "constexpr 2^32 * 2^32 high limb captures 2^64 carry");
#endif

		// Single-block constexpr div (uint8 limb -- the nbits == sizeof(bt)*8 fast path).
		using BB8u8 = blockbinary<8, std::uint8_t, BinaryNumberType::Signed>;
		constexpr BB8u8 quot8u8 = []() { BB8u8 t(77); t /= BB8u8(11); return t; }();
		static_assert(quot8u8.block(0) == 7, "constexpr blockbinary<8,uint8>(77) /= 11 == 7");

		// Single-block constexpr mod.
		constexpr BB8u8 rem8u8 = []() { BB8u8 t(77); t %= BB8u8(13); return t; }();
		static_assert(rem8u8.block(0) == (77 % 13), "constexpr 77 %% 13 == 12");

		// Multi-limb constexpr div via longdivision.
		constexpr BB128u32 mlQuot = []() { BB128u32 t(1000000000LL); t /= BB128u32(1000); return t; }();
		static_assert(mlQuot.to_sll() == 1000000LL, "constexpr 1B / 1k == 1M (uint32 longdivision)");

		// Multi-limb constexpr mod via longdivision.
		constexpr BB128u32 mlRem = []() { BB128u32 t(1000000007LL); t %= BB128u32(13); return t; }();
		static_assert(mlRem.to_sll() == (1000000007LL % 13), "constexpr 1000000007 %% 13");

		// Free operator* / operator/ / operator% (verify the constexpr wrappers).
		constexpr BB32u8 freeProd = BB32u8(6) * BB32u8(7);
		static_assert(freeProd.block(0) == 42, "constexpr free operator* (single-block)");
		constexpr BB32u8 freeQuot = BB32u8(42) / BB32u8(6);
		static_assert(freeQuot.block(0) == 7, "constexpr free operator/ (single-block)");
		constexpr BB32u8 freeRem  = BB32u8(43) % BB32u8(6);
		static_assert(freeRem.block(0) == 1, "constexpr free operator%% (single-block)");

		// Divide-by-zero defined behavior: returns zero, no UB.
		constexpr BB8u8 divZero = []() { BB8u8 t(42); t /= BB8u8(0); return t; }();
		static_assert(divZero.iszero(), "constexpr divide-by-zero returns zero (single-block fast path)");
		constexpr BB128u32 divZeroMl = []() { BB128u32 t(42); t /= BB128u32(0); return t; }();
		static_assert(divZeroMl.iszero(), "constexpr divide-by-zero returns zero (longdivision path)");

		// uradd / ursub / urmul2 wrappers.
		constexpr blockbinary<33, std::uint8_t, BinaryNumberType::Signed> ua = uradd(BB32u8(100), BB32u8(200));
		static_assert(ua.to_sll() == 300LL, "constexpr uradd(100, 200) == 300");
		constexpr blockbinary<33, std::uint8_t, BinaryNumberType::Signed> us = ursub(BB32u8(200), BB32u8(75));
		static_assert(us.to_sll() == 125LL, "constexpr ursub(200, 75) == 125");
		constexpr blockbinary<64, std::uint8_t, BinaryNumberType::Signed> um = urmul2(BB32u8(1234), BB32u8(5678));
		static_assert(um.to_sll() == (1234LL * 5678LL), "constexpr urmul2(1234, 5678) == 7006652");

		// Helpers: msb, any, to_sll, roundingMode.
		constexpr int m = BB32u8(0x10).msb();
		static_assert(m == 4, "constexpr msb(0x10) == 4");
		constexpr int64_t s = BB32u8(-7).to_sll();
		static_assert(s == -7LL, "constexpr to_sll preserves sign");
		constexpr bool anyHi = BB64u8(0x100).any(15);  // bit 8 set, asking about bits[0..15]
		static_assert(anyHi, "constexpr any(15) sees bit 8");

		// Unary - and ~ are now constexpr too.
		constexpr BB32u8 negFive = -BB32u8(5);
		static_assert(negFive.to_sll() == -5LL, "constexpr unary - returns -5");
		constexpr BB32u8 notZero = ~BB32u8(0);
		static_assert(notZero.to_sll() == -1LL, "constexpr unary ~ on zero is all-ones");

		// Unsigned widening must zero-extend, not sign-extend (CodeRabbit fix).
		// 0x80 widened from 8 -> 16 bits should be 0x0080 (=128), NOT 0xFF80 (=65408).
		using U8  = blockbinary<8,  std::uint8_t, BinaryNumberType::Unsigned>;
		using U16 = blockbinary<16, std::uint8_t, BinaryNumberType::Unsigned>;
		constexpr U16 widened = U16(U8(0x80));  // exercises constexpr cross-template ctor + assign()
		static_assert(widened.block(0) == 0x80, "constexpr Unsigned widen low byte preserved");
		static_assert(widened.block(1) == 0x00, "constexpr Unsigned widen high byte zero-extended (NOT 0xFF)");

		// Signed widening must still sign-extend (regression guard).
		using S8  = blockbinary<8,  std::uint8_t, BinaryNumberType::Signed>;
		using S16 = blockbinary<16, std::uint8_t, BinaryNumberType::Signed>;
		constexpr S16 sx_neg = S16(S8(-1));      // 0xFF -> 0xFFFF
		static_assert(sx_neg.block(0) == 0xFF, "constexpr Signed widen low byte preserved");
		static_assert(sx_neg.block(1) == 0xFF, "constexpr Signed widen sign-extends to 0xFF");
		constexpr S16 sx_pos = S16(S8(0x7F));    // 0x7F -> 0x007F (positive, no extension)
		static_assert(sx_pos.block(0) == 0x7F, "constexpr Signed widen positive low byte");
		static_assert(sx_pos.block(1) == 0x00, "constexpr Signed widen positive: high byte stays zero");

		std::cout << "constexpr mul/div/mod smoke tests PASS (compile-time)\n";
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
