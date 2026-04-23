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
		// for nbits <= 64. The encoded bit pattern must match the runtime path exactly.
		constexpr posit<32, 2>  cx_pos_42(42);
		constexpr posit<32, 2>  cx_neg_42(-42);
		constexpr posit<32, 2>  cx_zero(0);
		constexpr posit<8,  0>  cx_three(3);
		constexpr posit<16, 1>  cx_kilo(1024);
		constexpr posit<64, 3>  cx_big(123456789LL);
		constexpr posit<8,  0>  cx_sat_pos(1000);   // saturates to maxpos
		constexpr posit<8,  0>  cx_sat_neg(-1000);  // saturates to maxneg
		constexpr posit<32, 2>  cx_int_min(int32_t(-2147483647 - 1));

		// runtime construction for cross-check
		posit<32, 2>  rt_pos_42; rt_pos_42 = 42;
		posit<32, 2>  rt_neg_42; rt_neg_42 = -42;
		posit<32, 2>  rt_zero;   rt_zero   = 0;
		posit<8,  0>  rt_three;  rt_three  = 3;
		posit<16, 1>  rt_kilo;   rt_kilo   = 1024;
		posit<64, 3>  rt_big;    rt_big    = 123456789LL;
		posit<8,  0>  rt_sat_pos; rt_sat_pos = 1000;
		posit<8,  0>  rt_sat_neg; rt_sat_neg = -1000;
		posit<32, 2>  rt_int_min; rt_int_min = int32_t(-2147483647 - 1);

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
		if (!same_bits(cx_pos_42,  rt_pos_42))  { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(42)\n"; }
		if (!same_bits(cx_neg_42,  rt_neg_42))  { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(-42)\n"; }
		if (!same_bits(cx_zero,    rt_zero))    { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(0)\n"; }
		if (!same_bits(cx_three,   rt_three))   { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<8,0>(3)\n"; }
		if (!same_bits(cx_kilo,    rt_kilo))    { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<16,1>(1024)\n"; }
		if (!same_bits(cx_big,     rt_big))     { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<64,3>(123456789)\n"; }
		if (!same_bits(cx_sat_pos, rt_sat_pos)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<8,0>(1000) sat\n"; }
		if (!same_bits(cx_sat_neg, rt_sat_neg)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<8,0>(-1000) sat\n"; }
		if (!same_bits(cx_int_min, rt_int_min)) { ++nrOfFailedTestCases; std::cout << "FAIL constexpr posit<32,2>(INT_MIN)\n"; }
		if (nrOfFailedTestCases - start == 0) {
			std::cout << "PASS constexpr integer construction\n";
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
