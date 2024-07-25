// api.cpp: class interface tests for arbitrary configuration unum types
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//  SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
// Configure the template environment
// first: enable general or specialized configurations
#define UNUM_FAST_SPECIALIZATION
// second: enable/disable arithmetic exceptions
#define UNUM_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/number/unum/unum.hpp>
// type manipulators such as pretty printers
#include <universal/number/unum/manipulators.hpp>
#include <universal/number/unum/math_functions.hpp>

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	int nrOfFailedTestCases = 0;

	std::cout << "unum class interface tests\n";

	/////////////////////////////////////////////////////////////////////////////////////
	//// MODULAR unum Type 1 (the default)

	// construction
	{
		int start = nrOfFailedTestCases;
		// default construction using default BlockType (uint8_t)
		unum<8, 4> a, b(-8.125f), c(7.875), d(-7.875); // replace with long double init  d(-7.875l);
		// b initialized to -8.125 in modular arithmetic becomes 7.875: -8.125 = b1000.0010 > maxneg -> becomes b0111.1110
//		if (a != (c + d)) ++nrOfFailedTestCases;
//		if (a != (b - c)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : " << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		}
	}

#undef LATER
#ifdef LATER
	{
		int start = nrOfFailedTestCases;
		// construction with explicit arithmetic type and default BlockType (uint8_t)
		unum<8, 4> a, b(-8.125), c(7.875), d(-7.875);
		// b initialized to -8.125 in modular arithmetic becomes 7.875: -8.125 = b1000.0010 > maxneg -> becomes b0111.1110
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (b - c)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL: " << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
		}
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//// SATURATING unums

	{
		int start = nrOfFailedTestCases;
		// construction with explicit arithmetic type and default BlockType (uint8_t)
		unum<8, 4> a(-8.0), b(-8.125), c(7.875), d(-7.875);
		// b initialized to -8.125 in saturating arithmetic becomes -8
//		if (0 != (c + d)) ++nrOfFailedTestCases; //cout << to_binary(c + d) << endl;
		if (a != b) ++nrOfFailedTestCases;

		if (a != (d - 1)) ++nrOfFailedTestCases; // saturating to maxneg
		if (a != (d - 0.5)) ++nrOfFailedTestCases; // saturating to maxneg
		if (nrOfFailedTestCases - start > 0) {
			cout << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
			cout << to_binary(d - 1) << ' ' << to_binary(d - 0.5) << endl;
		}
	}


	/////////////////////////////////////////////////////////////////////////////////////
	//// improving efficiency for bigger unum Type 1s through explicit BlockType specification

	{
		int start = nrOfFailedTestCases;
		// construction with explicit arithmetic type and BlockType
		unum<16, 4, Modulo, uint16_t> a, b(-2048.125f), c(2047.875), d(-2047.875);
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (b - c)) ++nrOfFailedTestCases;
		//		cout << to_binary(a, true) << ' ' << to_binary(b, true) << ' ' << to_binary(c, true) << ' ' << to_binary(d, true) << endl;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL : construction " << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
			cout << a << ' ' << b << ' ' << c << ' ' << d << endl;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// selectors

	{
		int start = nrOfFailedTestCases;
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		unum<nbits, rbits> a, b;
		a = 1;
		if (!a.test(4)) ++nrOfFailedTestCases;
		b.set_raw_bits(1); // set the ULP
		if (!b.at(0)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL : selectors\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// modifiers

	{
		int start = nrOfFailedTestCases;
		// state/bit management
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		unum<nbits, rbits> a, b, c, d;
		for (size_t i = 0; i < rbits; ++i) {
			a.set(i, true);
		}
		b.set_raw_bits(0x0F); // same as the unum a above
		if ((a - b) != 0) ++nrOfFailedTestCases;
		c = b;
		// manually flip the bits of b
		for (size_t i = 0; i < nbits; ++i) {
			b.at(i) ? b.reset(i) : b.set(i);
		}
		c.flip();  // in-place 1's complement, so now b and c are the same
		if (b != c) ++nrOfFailedTestCases;	
		d.set_raw_bits(0xFFFFFFF);
		if (0 == d) ++nrOfFailedTestCases;
		d.setzero();
		if (d != 0) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL : modifiers\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// complements
	{
		int start = nrOfFailedTestCases;
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		unum<nbits, rbits> a, b;
		a.set_raw_bits(0xFF);
		b = ones_complement(a);
		if (b != 0) ++nrOfFailedTestCases;
		a = -1;
		b = twos_complement(a);
		if (b != 1) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL : complements 1\n";
		}
	}
	{
		int start = nrOfFailedTestCases;
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		unum<nbits, rbits, Modulo, uint16_t> a, b; // testing poorly selected BlockType
		a.set_raw_bits(0xFF);
		b = ones_complement(a);
		if (b != 0) ++nrOfFailedTestCases;
		a = -1;
		b = twos_complement(a);
		if (b != 1) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL : complements 2\n";
		}
	}
	{
		int start = nrOfFailedTestCases;
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		unum<nbits, rbits, Modulo, uint32_t> a, b; // testing poorly selected BlockType
		a.set_raw_bits(0xFF);
		b = ones_complement(a);
		if (b != 0) ++nrOfFailedTestCases;
		a = -1;
		b = twos_complement(a);
		if (b != 1) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL : complements 3\n";
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	// parsing of text input
	{
		/* TODO: implement parse
		constexpr size_t nbits = 128;
		constexpr size_t rbits = 64;
		unum<nbits, rbits, Modulo, uint32_t> a, b, c, d;
		a.assign("123456789.987654321");
		parse("123456789.987654321", b);
		*/
	}

	///////////////////////////////////////////////////////////////////////////////////
	// arithmetic
	{
		int start = nrOfFailedTestCases;
		constexpr size_t nbits = 16;
		constexpr size_t rbits = 8;
		constexpr bool arithmetic = Modulo;
		using blocktype = uint32_t;
		unum<nbits, rbits, arithmetic, blocktype> a, b, c, d;
		maxpos<nbits, rbits, arithmetic, blocktype>(a);
		maxneg<nbits, rbits, arithmetic, blocktype>(b);
		minpos<nbits, rbits, arithmetic, blocktype>(c);
		minneg<nbits, rbits, arithmetic, blocktype>(d);
		if ((c + d) != 0) ++nrOfFailedTestCases;

		if ((a + c) != b) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			cout << "FAIL: min/max\n";
			cout << to_binary(c + d) << " vs " << to_binary(0, nbits) << endl;
			cout << to_binary(a + c) << " vs " << to_binary(b) << endl;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// logic, in particular, all the literal constant combinations
	{
		int start = nrOfFailedTestCases;
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		constexpr bool arithmetic = Modulo;
		using blocktype = uint32_t;
		unum<nbits, rbits, arithmetic, blocktype> a, b, c, d;
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
			cout << "FAIL: logic operators\n";
		}
	}

#ifdef SHOW_STATE_SPACE
	{
		constexpr size_t nbits = 7;
		constexpr size_t rbits = 4;
		constexpr bool arithmetic = Modulo;
		constexpr size_t NR_VALUES = (1 << nbits);
		using blocktype = uint32_t;

		unum<nbits, rbits, arithmetic, blocktype> a, b, c, d;
		for (size_t i = 0; i < NR_VALUES; ++i) {
			a.set_raw_bits(i);
			float f = float(a);
			b = int(f);
			c = f;
			d = double(a);
			if (a != c && a != d) ++nrOfFailedTestCases;
			cout << setw(3) << i << ' ' << to_binary(a) << ' ' << setw(10) << a << ' ' << setw(3) << int(f) << ' ' << to_binary(b) << ' ' << b << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
		}
	}

	{
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		constexpr bool arithmetic = Modulo;
		using blocktype = uint32_t;
		unum<nbits, rbits, arithmetic, blocktype> a, b, c, d;

		for (int i = -16; i < 16; ++i) {
			a = i;
			cout << to_binary(i) << ' ' << a << ' ' << to_binary(a) << ' ' << to_binary(-a) << ' ' << -a << ' ' << to_binary(-i) << endl;
		}
	}
#endif // SHOW_STATE_SPACE

#endif // LATER

	if (nrOfFailedTestCases > 0) {
		std::cout << "FAIL\n";
	}
	else {
		std::cout << "PASS\n";
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
/*
catch (const sw::universal::unum_arithmetic_exception& err) {
	std::cerr << "Uncaught unum arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::unum_internal_exception& err) {
	std::cerr << "Uncaught unum internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
*/
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
