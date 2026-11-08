// shift.cpp: regression tests for einteger shift operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under
// an MIT Open Source license.
//
// Issue #862: einteger::operator>>= leaked a spurious limb when the shift
// crossed more than half the limb count (uint8_t blocks were the trigger
// in practice), and also left the input unchanged when shift == nbits()
// instead of returning 0.  Both are pinned here.

#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <string>
#include <universal/number/einteger/einteger.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_reporters.hpp>

namespace {

// Pack a little-endian byte payload into an einteger<Block>, regardless of the
// chosen block width.  Bit i*8 of the integer is byte i of the payload.
template<typename Block>
sw::universal::einteger<Block> pack_le(const std::uint8_t* bytes, unsigned n) {
	using namespace sw::universal;
	einteger<Block> v;
	for (unsigned i = 0; i < n; ++i) {
		unsigned bit  = i * 8u;
		unsigned limb = bit / (sizeof(Block) * 8u);
		unsigned off  = bit % (sizeof(Block) * 8u);
		Block existing = (limb < v.limbs()) ? v.block(limb) : Block{};
		Block updated  = static_cast<Block>(existing
		               | (static_cast<Block>(bytes[i]) << off));
		v.setblock(limb, updated);
	}
	return v;
}

template<typename Block>
int CheckShift(const char* tag, const std::uint8_t* bytes, unsigned n,
               int shift, const char* expected_decimal, bool reportTestCases) {
	auto v = pack_le<Block>(bytes, n);
	v >>= shift;
	std::ostringstream os;
	os << v;
	if (os.str() != std::string(expected_decimal)) {
		if (reportTestCases) {
			std::cout << "FAIL " << tag << " shift=" << shift
			          << " got=" << os.str()
			          << " expected=" << expected_decimal << '\n';
		}
		return 1;
	}
	return 0;
}

template<typename Block>
int CheckShiftZero(const char* tag, const std::uint8_t* bytes, unsigned n,
                   int shift, bool reportTestCases) {
	auto v = pack_le<Block>(bytes, n);
	v >>= shift;
	if (!v.iszero()) {
		if (reportTestCases) {
			std::ostringstream os; os << v;
			std::cout << "FAIL " << tag << " shift=" << shift
			          << " got=" << os.str() << " expected=0\n";
		}
		return 1;
	}
	return 0;
}

}

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "einteger shift operators (issue #862)";
	bool reportTestCases   = true;
	int  nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// 15-byte little-endian payload: 0x1921FB54442D17BD21B8D78573DE4E.
	// This is the value that surfaced #862 (top bytes of pi * 10^15 << 100
	// divided by 5^15, see #842).  We test it across all three block widths.
	static const std::uint8_t payload15[15] = {
		0x4e, 0xde, 0x73, 0x85, 0xd7, 0xb8, 0x21, 0xbd,
		0x17, 0x2d, 0x44, 0x54, 0xfb, 0x21, 0x19
	};

	// >> 64: top 56 bits = 0x001921FB54442D17 = 7074237752028439
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckShift<std::uint8_t >(
			"u8  15-limb >> 64", payload15, 15, 64,
			"7074237752028439", reportTestCases);
		nrOfFailedTestCases += CheckShift<std::uint16_t>(
			"u16 8-limb  >> 64", payload15, 15, 64,
			"7074237752028439", reportTestCases);
		nrOfFailedTestCases += CheckShift<std::uint32_t>(
			"u32 4-limb  >> 64", payload15, 15, 64,
			"7074237752028439", reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start,
			"15-byte payload >> 64", "einteger shift");
	}

	// >> 24: drop low 3 bytes, top 96 bits = 0x1921FB54442D17BD21B8D785
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckShift<std::uint8_t >(
			"u8  >> 24", payload15, 15, 24,
			"7778206666007220823048902533", reportTestCases);
		nrOfFailedTestCases += CheckShift<std::uint16_t>(
			"u16 >> 24", payload15, 15, 24,
			"7778206666007220823048902533", reportTestCases);
		nrOfFailedTestCases += CheckShift<std::uint32_t>(
			"u32 >> 24", payload15, 15, 24,
			"7778206666007220823048902533", reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start,
			"15-byte payload >> 24", "einteger shift");
	}

	// shift == nbits boundary: every significant bit shifts out, so result = 0
	{
		int start = nrOfFailedTestCases;
		{
			einteger<std::uint32_t> v;
			v.setblock(0, 0xFFFFFFFFu);
			v >>= 32;
			if (!v.iszero()) ++nrOfFailedTestCases;
		}
		{
			einteger<std::uint8_t> v;
			for (unsigned i = 0; i < 3; ++i) v.setblock(i, 0xFFu);
			v >>= 24;
			if (!v.iszero()) ++nrOfFailedTestCases;
		}
		{
			einteger<std::uint16_t> v;
			for (unsigned i = 0; i < 4; ++i) v.setblock(i, 0xFFFFu);
			v >>= 64;
			if (!v.iszero()) ++nrOfFailedTestCases;
		}
		ReportTestResult(nrOfFailedTestCases - start,
			"shift == nbits boundary", "einteger shift");
	}

	// shift > nbits boundary (already-correct path, pinned here)
	{
		int start = nrOfFailedTestCases;
		nrOfFailedTestCases += CheckShiftZero<std::uint32_t>(
			"u32 1-limb >> 64", payload15, 4, 64, reportTestCases);
		nrOfFailedTestCases += CheckShiftZero<std::uint8_t >(
			"u8  4-limb >> 200", payload15, 4, 200, reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start,
			"shift > nbits boundary", "einteger shift");
	}

	// Multi-byte shift not aligned to a limb boundary (mixed block + in-block).
	// Cross-check against an independent integer<2048> reference rather than
	// a pre-baked literal -- avoids the test asserting an implementation
	// echoes its own output.
	{
		int start = nrOfFailedTestCases;
		using Big = integer<2048>;
		Big ref(0);
		for (int i = 14; i >= 0; --i) {
			ref *= Big(256);
			ref += Big(static_cast<int>(payload15[i]));
		}
		ref >>= 17;
		std::ostringstream os; os << ref;
		std::string expected = os.str();
		nrOfFailedTestCases += CheckShift<std::uint8_t >(
			"u8  >> 17", payload15, 15, 17, expected.c_str(), reportTestCases);
		nrOfFailedTestCases += CheckShift<std::uint16_t>(
			"u16 >> 17", payload15, 15, 17, expected.c_str(), reportTestCases);
		nrOfFailedTestCases += CheckShift<std::uint32_t>(
			"u32 >> 17", payload15, 15, 17, expected.c_str(), reportTestCases);
		ReportTestResult(nrOfFailedTestCases - start,
			"unaligned shift vs integer<2048>", "einteger shift");
	}

	// A negative shift count shifts the other way: a >>= -k is a <<= k and a <<= -k is a >>= k.
	// operator>>= with a negative count called itself with -shift, so 8 >>= -2 gave 2 (#1987).
	// >> truncates the magnitude toward zero and keeps the sign, so -5 >> 1 is -2, and a negative
	// value shifted to zero is +0, not a -0 (#1987).
	{
		int start = nrOfFailedTestCases;
		auto check = [&](const einteger<std::uint32_t>& got, long long expected, const std::string& what) {
			std::ostringstream os, es;
			os << got;
			es << expected;
			if (os.str() != es.str() || (got.iszero() && got.isneg())) {
				++nrOfFailedTestCases;
				if (reportTestCases) std::cout << "FAIL " << what << " got=" << os.str() << (got.isneg() ? " (negative)" : "") << " expected=" << es.str() << '\n';
			}
		};
		for (long long v : { 0ll, 1ll, 3ll, 8ll, 255ll, 123456789ll, -1ll, -3ll, -5ll, -8ll, -123456789ll }) {
			for (int k = 0; k <= 24; ++k) {
				const long long scale = 1ll << k;
				einteger<std::uint32_t> a(v), b(v), c(v), d(v);
				a >>= -k;   // left by k
				b <<= -k;   // right by k
				c >>= k;
				d <<= k;
				const std::string tag = std::to_string(v) + " k=" + std::to_string(k);
				check(a, v * scale, tag + " >>= -k");
				check(b, v / scale, tag + " <<= -k");
				check(c, v / scale, tag + " >>= k");
				check(d, v * scale, tag + " <<= k");
			}
		}
		// shifts by whole limbs, and past the top, of a negative value
		for (int k : { 32, 64, 96, 200 }) {
			einteger<std::uint32_t> a(-1);
			a <<= 40;
			a >>= 40 + k;
			check(a, 0, "-2^40 >> " + std::to_string(40 + k));
		}
		ReportTestResult(nrOfFailedTestCases - start,
			"negative shift counts and signs", "einteger shift");
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception\n";
	return EXIT_FAILURE;
}
