// api.cpp: class interface tests for arbitrary configuration posit types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the posit template environment
// first: enable general or specialized configurations
#define POSIT_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
// third: enable support for native literals in logic and arithmetic operations
#define POSIT_ENABLE_LITERALS 1
// fourth: enable/disable error-free serialization I/O
#define POSIT_ERROR_FREE_IO_FORMAT 0
// fifth: potential trace conversions or arithmetic
//#define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_CONVERSION
// minimum set of include files to reflect source code dependencies
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/test_case.hpp>

/*
 examples to how to use the posit number system
 */
int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "generalized posit number system API";
	std::string test_tag    = "api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// posit construction, initialization, assignment and comparisions

	std::cout << "+-----------------   constexpr integer construction (issue #713)\n";
	{
		// constexpr construction from integer literals must succeed at compile time
		// for nbits <= 64. The encoded bit pattern must match an INDEPENDENT
		// reference path (convert_ieee754 via double-cast) so that a bug in
		// encode_positive_uint64 cannot mask itself.
		// All test values fit exactly in double's 53-bit mantissa, so the double
		// cast is bit-exact and provides a valid reference.
		constexpr posit<32, 2>  cx_pos_42(42);
		constexpr posit<32, 2>  cx_neg_42(-42);
		constexpr posit<32, 2>  cx_zero(0);
		constexpr posit<8,  0>  cx_three(3);
		constexpr posit<16, 1>  cx_kilo(1024);
		constexpr posit<64, 3>  cx_big(123456789LL);
		constexpr posit<8,  0>  cx_sat_pos(1000);   // saturates to maxpos
		constexpr posit<8,  0>  cx_sat_neg(-1000);  // saturates to maxneg
		constexpr posit<32, 2>  cx_int_min(int32_t(-2147483647 - 1));

		// Reference path: float literal constructor still routes through the
		// pre-existing convert_ieee754, independent of the new constexpr code.
		posit<32, 2>  ref_pos_42 (42.0);
		posit<32, 2>  ref_neg_42 (-42.0);
		posit<32, 2>  ref_zero   (0.0);
		posit<8,  0>  ref_three  (3.0);
		posit<16, 1>  ref_kilo   (1024.0);
		posit<64, 3>  ref_big    (123456789.0);
		posit<8,  0>  ref_sat_pos(1000.0);
		posit<8,  0>  ref_sat_neg(-1000.0);
		posit<32, 2>  ref_int_min(static_cast<double>(int32_t(-2147483647 - 1)));

		auto same_bits = [](auto cx, auto ref) {
			auto a = cx.bits();
			auto b = ref.bits();
			constexpr unsigned nrBlocks = decltype(a)::nrBlocks;
			for (unsigned i = 0; i < nrBlocks; ++i) {
				if (a.block(i) != b.block(i)) return false;
			}
			return true;
		};

		int start = nrOfFailedTestCases;
		if (!same_bits(cx_pos_42,  ref_pos_42))  { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(42)\n"; }
		if (!same_bits(cx_neg_42,  ref_neg_42))  { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(-42)\n"; }
		if (!same_bits(cx_zero,    ref_zero))    { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(0)\n"; }
		if (!same_bits(cx_three,   ref_three))   { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<8,0>(3)\n"; }
		if (!same_bits(cx_kilo,    ref_kilo))    { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<16,1>(1024)\n"; }
		if (!same_bits(cx_big,     ref_big))     { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<64,3>(123456789)\n"; }
		if (!same_bits(cx_sat_pos, ref_sat_pos)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<8,0>(1000) sat\n"; }
		if (!same_bits(cx_sat_neg, ref_sat_neg)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<8,0>(-1000) sat\n"; }
		if (!same_bits(cx_int_min, ref_int_min)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(INT_MIN)\n"; }

		// Plain-char regression: char is implementation-defined as signed or
		// unsigned, so the reference value must match the platform's char model.
		// On signed-char platforms (the common case) char(-3) is -3; on
		// unsigned-char platforms it wraps to 253. The dispatch in operator=(char)
		// is what we are validating: it should produce the matching reference.
		constexpr posit<32, 2> cx_char_neg3(static_cast<char>(-3));
		if constexpr (std::is_signed_v<char>) {
			posit<32, 2> ref_char_neg3(-3.0);
			if (!same_bits(cx_char_neg3, ref_char_neg3)) {
				++nrOfFailedTestCases;
				std::cout << "FAIL constexpr posit<32,2>(char(-3)) on signed-char platform\n";
			}
		}
		else {
			posit<32, 2> ref_char_neg3(253.0);
			if (!same_bits(cx_char_neg3, ref_char_neg3)) {
				++nrOfFailedTestCases;
				std::cout << "FAIL constexpr posit<32,2>(char(-3)) on unsigned-char platform\n";
			}
		}

		if (nrOfFailedTestCases - start == 0) {
			std::cout << "PASS constexpr integer construction\n";
		}
	}

	std::cout << "+-----------------   constexpr IEEE-754 construction (Phase 2 of #713)\n";
	{
		// Construct posits from float / double literals at compile time. The
		// new convert_ieee754 path uses bit-cast extractFields + raw-exponent
		// NaN/Inf checks (no std::frexp / std::isnan / std::isinf), so it is
		// constexpr on platforms where __builtin_bit_cast is constexpr (gcc,
		// clang, MSVC).
		constexpr posit<32, 2>  cxf_pi   (3.14);
		constexpr posit<32, 2>  cxf_npi  (-3.14);
		constexpr posit<32, 2>  cxf_zero (0.0);
		constexpr posit<32, 2>  cxf_one  (1.0);
		constexpr posit<32, 2>  cxf_two  (2.0);
		constexpr posit<32, 2>  cxf_half (0.5);
		constexpr posit<32, 2>  cxf_subn (1e-40f);   // subnormal float -> normalize via find_msb
		constexpr posit<8,  0>  cxf_pi8  (3.14);
		constexpr posit<16, 1>  cxf_pi16 (3.14);
		constexpr posit<64, 3>  cxf_pi64 (3.14159265358979);

		// Reference: same arithmetic but at runtime (the same convert_ieee754 path,
		// which is the only path now -- there is no separate reference here. So
		// we just verify the constexpr values evaluated and produce the same bits
		// on every invocation by also constructing them at runtime.)
		posit<32, 2>  rt_pi(3.14);
		posit<32, 2>  rt_npi(-3.14);
		posit<32, 2>  rt_zero(0.0);
		posit<32, 2>  rt_one(1.0);
		posit<32, 2>  rt_two(2.0);
		posit<32, 2>  rt_half(0.5);
		posit<32, 2>  rt_subn(1e-40f);
		posit<8,  0>  rt_pi8(3.14);
		posit<16, 1>  rt_pi16(3.14);
		posit<64, 3>  rt_pi64(3.14159265358979);

		auto same_bits = [](auto cx, auto rt) {
			auto a = cx.bits();
			auto b = rt.bits();
			constexpr unsigned nrBlocks = decltype(a)::nrBlocks;
			for (unsigned i = 0; i < nrBlocks; ++i) {
				if (a.block(i) != b.block(i)) return false;
			}
			return true;
		};

		int start = nrOfFailedTestCases;
		if (!same_bits(cxf_pi,   rt_pi))   { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(3.14)\n"; }
		if (!same_bits(cxf_npi,  rt_npi))  { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(-3.14)\n"; }
		if (!same_bits(cxf_zero, rt_zero)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(0.0)\n"; }
		if (!same_bits(cxf_one,  rt_one))  { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(1.0)\n"; }
		if (!same_bits(cxf_two,  rt_two))  { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(2.0)\n"; }
		if (!same_bits(cxf_half, rt_half)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(0.5)\n"; }
		if (!same_bits(cxf_subn, rt_subn)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(1e-40f) - subnormal\n"; }
		if (!same_bits(cxf_pi8,  rt_pi8))  { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<8,0>(3.14)\n"; }
		if (!same_bits(cxf_pi16, rt_pi16)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<16,1>(3.14)\n"; }
		if (!same_bits(cxf_pi64, rt_pi64)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<64,3>(pi)\n"; }
		if (nrOfFailedTestCases - start == 0) {
			std::cout << "PASS constexpr IEEE-754 construction\n";
		}
	}

	std::cout << "+-----------------   posit construction, initialization, comparisons\n";
	{
		int start = nrOfFailedTestCases;
		// maxpos of a posit<8,0> = 64
		posit<8, 0> a(-64), b(-128), c(64), d(-64);
		// b initialized to -128 in saturating arithmetic becomes -64
		if (0 != (c + d)) ++nrOfFailedTestCases; //cout << to_binary(c + d) << endl;
		if (a != b) ++nrOfFailedTestCases;

		if (a != (d - 32)) ++nrOfFailedTestCases; // saturating to maxneg
		if (a != (d - 0.5)) ++nrOfFailedTestCases; // saturating to maxneg
        std::cout << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << '\n';
		if (nrOfFailedTestCases - start > 0) {
			std::cout << to_binary(d - 1) << ' ' << to_binary(d - 0.5) << '\n';
		}
	}


	// type tag to identify the type without having to depend on demangle
	std::cout << "+-----------------   type_tag of the standard posit configurations\n";
	{
		using Posit = posit<16, 2>; // default BlockType
		Posit a{ 0 };
		std::cout << "type identifier : " << type_tag(a) << '\n';
		std::cout << "standard posit  : " << type_tag(posit<  8, 2, std::uint8_t>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit< 16, 2, std::uint16_t>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit< 32, 2, std::uint32_t>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit< 64, 2, std::uint64_t>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit<128, 2, std::uint64_t>()) << '\n';
		std::cout << "standard posit  : " << type_tag(posit<256, 2, std::uint64_t>()) << '\n';
	}

	std::cout << "+-----------------   special cases\n";
	{
		using BlockType = std::uint8_t;
		using Posit = posit<8, 0, BlockType>;
		Posit a;
		a.setnar();		ReportValue(a, "NaR     ");
		a.maxpos();		ReportValue(a, "maxpos  ");
		a = maxprecision_max<8, 0, BlockType>(); ReportValue(a, "maxr0   ");
		a = 1;  		ReportValue(a, "  1     ");
		a = maxprecision_min<8, 0, BlockType>(); ReportValue(a, "minr-1  ");
		a.minpos();		ReportValue(a, "minpos  ");
		a.setzero();	ReportValue(a, "zero    ");
		a.minneg();		ReportValue(a, "minneg  ");
		a = -1;			ReportValue(a, " -1     ");
		a.maxneg();		ReportValue(a, "maxneg  ");
	}

	std::cout << "+-----------------   binary, color, and value printing\n";
	{
		using Posit = posit<5, 1>;
		Posit a;
		for (unsigned i = 0; i < 32; ++i) {
			a.setbits(i);
			std::cout << to_binary(a) << " : " << color_print(a) << " : " << a << '\n';
		}
	}

	std::cout << "+-----------------   pretty and info printing\n";
	{
		using Posit = posit<5, 1>;
		Posit a;
		for (unsigned i = 0; i < 32; ++i) {
			a.setbits(i);
			std::cout << std::left << std::setw(30) << pretty_print(a) << " : " << info_print(a) << '\n';
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// improving efficiency for posits through explicit BlockType specification

	std::cout << "+-----------------   BlockType\n";
	{
		int start = nrOfFailedTestCases;

		posit<16, 2, std::uint8_t> a{ 0 }, b{ -0.984375f }, c{ 0.984375 }, d{ -0.984375 };
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (-b - c)) ++nrOfFailedTestCases;
		std::cout << to_binary(a, true) << ' ' << to_binary(b, true) << ' ' << to_binary(c, true) << ' ' << to_binary(d, true) << '\n';
		if (nrOfFailedTestCases - start > 0) {
			std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		}
    
        posit<256, 2, std::uint64_t> p(SpecificValue::maxpos);  // a ginormous number
        std::cout << to_binary(p, true) << " : " << p << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// selectors

	std::cout << "+-----------------   selectors\n";
	{
		int start = nrOfFailedTestCases;

        posit<13,3> p;
        p = -1;
        if (!p.sign()) ++nrOfFailedTestCases;
        if (!p.isminusone()) ++nrOfFailedTestCases;
        if (!p.isneg()) ++nrOfFailedTestCases;
        p.setnar();
        if (!p.isnar())  ++nrOfFailedTestCases;
        p = 0;
        if (!p.iszero()) ++nrOfFailedTestCases;
        p = 1;
        if (!p.isone()) ++nrOfFailedTestCases;
        if (!p.ispos()) ++nrOfFailedTestCases;
        for (int i = 0; i < 10; ++i) {
            if (!p.ispowerof2()) ++nrOfFailedTestCases;
            if (!p.isinteger()) ++nrOfFailedTestCases;
            ReportValue(p, "power of 2");
            p *= 2;
        }
        p = 12;
        if (p.ispowerof2()) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : selectors\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// modifiers

	std::cout << "+-----------------   modifiers\n";
	{
		int start = nrOfFailedTestCases;
		// state/bit management
        posit<22, 2> p(SpecificValue::maxpos);
        ReportValue(p, "maxpos   : ");
        p.clear();
        ReportValue(p, "zero     : ");
        if (!p.iszero()) ++nrOfFailedTestCases;
        p.flip();
        ReportValue(p, "minneg   : ");
        posit<22, 2> minneg(SpecificValue::minneg);
        if (p != minneg)  ++nrOfFailedTestCases;
        p.setzero();
        ReportValue(p, "zero     : ");
        if (!p.iszero()) ++nrOfFailedTestCases;

        p.setbits(0x05a5a5);
        ReportValue(p, "0x05a5a5  : ");
        p.setbits(blockbinary<22>(0x05a5a5));
        ReportValue(p, "0x05a5a5  : ");

        auto v = p.to_value();
        std::cout << type_tag(v) << " : " << to_binary(v) << " : " << v << '\n';

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : modifiers\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// complements
	std::cout << "+-----------------   complements\n";
	{
		int start = nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : complements 1\n";
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// parsing of text input
	std::cout << "+-----------------   parsing\n";
	{
		int start = nrOfFailedTestCases;

		// parse a decimal floating-point string
		posit<32, 2> a;
		if (!parse("1.5", a)) ++nrOfFailedTestCases;
		if (a != posit<32, 2>(1.5)) ++nrOfFailedTestCases;

		// parse a negative value
		if (!parse("-3.25", a)) ++nrOfFailedTestCases;
		if (a != posit<32, 2>(-3.25)) ++nrOfFailedTestCases;

		// parse scientific notation
		if (!parse("1.25e3", a)) ++nrOfFailedTestCases;
		if (a != posit<32, 2>(1250.0)) ++nrOfFailedTestCases;

		// parse a posit hex format: nbits.esxHEXVALUEp
		posit<8, 0> b;
		if (!parse("8.0x40p", b)) ++nrOfFailedTestCases;
		if (b != posit<8, 0>(1.0)) ++nrOfFailedTestCases;

		// parse via istream operator>>
		std::istringstream is("2.5");
		posit<16, 2> c;
		is >> c;
		if (c != posit<16, 2>(2.5)) ++nrOfFailedTestCases;

        if (!parse("1234567.8901234", a)) ++nrOfFailedTestCases;
        ReportValue(a, "1234567.8901234 : " );

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : parsing\n";
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// arithmetic
	std::cout << "+-----------------   arithmetic\n";
	{
		int start = nrOfFailedTestCases;
		constexpr unsigned nbits = 16;
		constexpr unsigned es    =  2;
		using BlockType          = std::uint16_t;
		posit<nbits, es, BlockType> a, b, c, d;
		a.maxpos();
		b.maxneg();
		c.minpos();
		d.minneg();
		if ((c + d) != 0) ++nrOfFailedTestCases;

		if ((a + c) != a) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: min/max\n";
			std::cout << to_binary(c + d) << " vs " << to_binary(posit<nbits, es>(0)) << '\n';
			std::cout << to_binary(a + c) << " vs " << to_binary(b) << '\n';
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// logic, in particular, all the literal constant combinations
	std::cout << "+-----------------   logic comparisons\n";
	{
		int start = nrOfFailedTestCases;
		constexpr unsigned nbits = 8;
		constexpr unsigned es    = 2;
		posit<nbits, es> a, b, c, d;
		a = 1;
		b = 2l;
		c = 3ll;
		d = 0ull;
		// unsigned literals
		if (a != 1u) ++nrOfFailedTestCases;
		if (b != 2ul) ++nrOfFailedTestCases;
		if (c != 3ull) ++nrOfFailedTestCases;
		if (1u != a) ++nrOfFailedTestCases;
		if (2ul != b) ++nrOfFailedTestCases;
		if (3ull != c) ++nrOfFailedTestCases;
		if (d != c - b - a) ++nrOfFailedTestCases;
		// signed literals
		if (-a != -1) ++nrOfFailedTestCases;
		if (-b != -2l) ++nrOfFailedTestCases;
		if (-c != -3ll) ++nrOfFailedTestCases;
		if (-1 != -a) ++nrOfFailedTestCases;
		if (-2l != -b) ++nrOfFailedTestCases;
		if (-3ll != -c) ++nrOfFailedTestCases;

		// less than unsigned literal
		d = 4.0f;
		if (d < 1u) ++nrOfFailedTestCases;
		if (d < 2ul) ++nrOfFailedTestCases;
		if (d < 3ull) ++nrOfFailedTestCases;
		d = 0.0;
		if (1u < d) ++nrOfFailedTestCases;
		if (2ul < d) ++nrOfFailedTestCases;
		if (3ull < d) ++nrOfFailedTestCases;

		// greater than unsigned literal
		if (d > 1u) ++nrOfFailedTestCases;
		if (d > 2ul) ++nrOfFailedTestCases;
		if (d > 3ull) ++nrOfFailedTestCases;
		d = 4ll;
		std::cout << to_binary(d) << " : " << d << '\n';
		if (1u > d) ++nrOfFailedTestCases;
		if (2ul > d) ++nrOfFailedTestCases;
		if (3ull > d) ++nrOfFailedTestCases;

		// less than or equal unsigned literal
		if (d <= 1u) ++nrOfFailedTestCases;
		if (d <= 2ul) ++nrOfFailedTestCases;
		if (d <= 3ull) ++nrOfFailedTestCases;
		d = 0.0f;
		if (1u <= d) ++nrOfFailedTestCases;
		if (2ul <= d) ++nrOfFailedTestCases;
		if (3ull <= d) ++nrOfFailedTestCases;

		// greater than or equal unsigned literal
		if (d >= 1u) ++nrOfFailedTestCases;
		if (d >= 2ul) ++nrOfFailedTestCases;
		if (d >= 3ull) ++nrOfFailedTestCases;
		d = 4.0;
		if (1u >= d) ++nrOfFailedTestCases;
		if (2ul >= d) ++nrOfFailedTestCases;
		if (3ull >= d) ++nrOfFailedTestCases;

		// comparisons with signed literals
		// less than signed literal
		d = 4.0f;
		if (d < 1) ++nrOfFailedTestCases;
		if (d < 2l) ++nrOfFailedTestCases;
		if (d < 3ll) ++nrOfFailedTestCases;
		d = 0.0;
		if (1 < d) ++nrOfFailedTestCases;
		if (2l < d) ++nrOfFailedTestCases;
		if (3ll < d) ++nrOfFailedTestCases;

		// greater than signed literal
		if (d > 1) ++nrOfFailedTestCases;
		if (d > 2l) ++nrOfFailedTestCases;
		if (d > 3ll) ++nrOfFailedTestCases;
		d = 4ll;
		if (1 > d) ++nrOfFailedTestCases;
		if (2l > d) ++nrOfFailedTestCases;
		if (3ll > d) ++nrOfFailedTestCases;

		// less than or equal signed literal
		if (d <= 1) ++nrOfFailedTestCases;
		if (d <= 2l) ++nrOfFailedTestCases;
		if (d <= 3ll) ++nrOfFailedTestCases;
		d = 0.0f;
		if (1 <= d) ++nrOfFailedTestCases;
		if (2l <= d) ++nrOfFailedTestCases;
		if (3ll <= d) ++nrOfFailedTestCases;

		// greater than or equal signed literal
		if (d >= 1) ++nrOfFailedTestCases;
		if (d >= 2l) ++nrOfFailedTestCases;
		if (d >= 3ll) ++nrOfFailedTestCases;
		d = 4.0;
		if (1 >= d) ++nrOfFailedTestCases;
		if (2l >= d) ++nrOfFailedTestCases;
		if (3ll >= d) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: logic operators\n";
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
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
