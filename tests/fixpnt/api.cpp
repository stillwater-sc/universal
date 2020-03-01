// api.cpp: class interface tests for arbitrary configuration fixed-point addition
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define FIXPNT_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 1

// minimum set of include files to reflect source code dependencies
#include <universal/fixpnt/fixed_point.hpp>
// fixed-point type manipulators such as pretty printers
#include <universal/fixpnt/fixpnt_manipulators.hpp>
#include <universal/fixpnt/math_functions.hpp>

// conditional compile flags
#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	int nrOfFailedTestCases = 0;

	cout << "fixed-point class interface tests" << endl;

	/////////////////////////////////////////////////////////////////////////////////////
	//// MODULAR fixed-point (the default)

	// construction
	{
		// default construction using default arithmetic (Modular) and default BlockType (uint8_t)
		fixpnt<8, 4> a, b(-8.125f), c(7.875), d(-7.875); // replace with long double init  d(-7.875l);
		// b initialized to -8.125 in modular arithmetic becomes 7.875: -8.125 = b1000.0010 > maxneg -> becomes b0111.1110
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (b - c)) ++nrOfFailedTestCases;
		//cout << a << ' ' << b << ' ' << c << ' ' << d << endl;
	}

	{
		// construction with explicit arithmetic type and default BlockType (uint8_t)
		fixpnt<8, 4, Modular> a, b(-8.125), c(7.875), d(-7.875);
		// b initialized to -8.125 in modular arithmetic becomes 7.875: -8.125 = b1000.0010 > maxneg -> becomes b0111.1110
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (b - c)) ++nrOfFailedTestCases;
		//cout << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// SATURATING fixed-point

	{
		// construction with explicit arithmetic type and default BlockType (uint8_t)
		fixpnt<8, 4, Saturation> a(-8.0), b(-8.125), c(7.875), d(-7.875);
		// b initialized to -8.125 in saturating arithmetic becomes -8
//		if (0 != (c + d)) ++nrOfFailedTestCases; //cout << to_binary(c + d) << endl;
		if (a != b) ++nrOfFailedTestCases;
		// TODO: don't have saturating arithmetic yet
		//if (a != (d - 1)) ++nrOfFailedTestCases; // saturating to maxneg
		//if (a != (d - 0.5)) ++nrOfFailedTestCases; // saturating to maxneg
		//cout << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
		//cout << to_binary(d - 1) << ' ' << to_binary(d - 0.5) << endl;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// improving efficiency for bigger fixed-points through explicit BlockType specification

	{
		// construction with explicit arithmetic type and BlockType
		fixpnt<16, 4, Modular, uint16_t> a, b(-2048.125f), c(2047.875), d(-2047.875);
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (b - c)) ++nrOfFailedTestCases;
		//		cout << to_binary(a, true) << ' ' << to_binary(b, true) << ' ' << to_binary(c, true) << ' ' << to_binary(d, true) << endl;
		//cout << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << endl;
		//cout << a << ' ' << b << ' ' << c << ' ' << d << endl;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// modifiers

	{
		// state/bit management
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		fixpnt<nbits, rbits> a, b, c, d;
		for (size_t i = 0; i < rbits; ++i) {
			a.set(i, true);
		}
		b.set_raw_bits(0x0F); // same as the fixpnt a above
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
	}

	/////////////////////////////////////////////////////////////////////////////
	// complements
	{
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		fixpnt<nbits, rbits> a, b;
		a.set_raw_bits(0xFF);
		b = ones_complement(a);
		if (b != 0) ++nrOfFailedTestCases;
		a = -1;
		b = twos_complement(a);
		if (b != 1) ++nrOfFailedTestCases;
	}
	{
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		fixpnt<nbits, rbits, Modular, uint16_t> a, b; // testing poorly selected BlockType
		a.set_raw_bits(0xFF);
		b = ones_complement(a);
		if (b != 0) ++nrOfFailedTestCases;
		a = -1;
		b = twos_complement(a);
		if (b != 1) ++nrOfFailedTestCases;
	}
	{
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		fixpnt<nbits, rbits, Modular, uint32_t> a, b; // testing poorly selected BlockType
		a.set_raw_bits(0xFF);
		b = ones_complement(a);
		if (b != 0) ++nrOfFailedTestCases;
		a = -1;
		b = twos_complement(a);
		if (b != 1) ++nrOfFailedTestCases;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// parsing of text input
	{
		/* TODO: implement parse
		constexpr size_t nbits = 128;
		constexpr size_t rbits = 64;
		fixpnt<nbits, rbits, Modular, uint32_t> a, b, c, d;
		a.assign("123456789.987654321");
		parse("123456789.987654321", b);
		*/
	}

	///////////////////////////////////////////////////////////////////////////////////
	// arithmetic
	{
		constexpr size_t nbits = 16;
		constexpr size_t rbits = 8;
		constexpr bool arithmetic = Modular;
		using blocktype = uint32_t;
		fixpnt<nbits, rbits, arithmetic, blocktype> a, b, c, d;
		a = maxpos_fixpnt<nbits, rbits, arithmetic, blocktype>();
		b = maxneg_fixpnt<nbits, rbits, arithmetic, blocktype>();
		c = minpos_fixpnt<nbits, rbits, arithmetic, blocktype>();
		d = minneg_fixpnt<nbits, rbits, arithmetic, blocktype>();
		if ((c + d) != 0) ++nrOfFailedTestCases;
		//cout << to_binary(c + d) << " vs " << to_binary(0,nbits) << endl;

		if ((a + c) != b) ++nrOfFailedTestCases;
		//cout << to_binary(a + c) << " vs " << to_binary(b) << endl;
	}

	///////////////////////////////////////////////////////////////////////////////////
	// logic, in particular, all the literal constant combinations
	{
		constexpr size_t nbits = 8;
		constexpr size_t rbits = 4;
		constexpr bool arithmetic = Modular;
		using blocktype = uint32_t;
		fixpnt<nbits, rbits, arithmetic, blocktype> a, b, c, d;
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
	}

#ifdef SHOW_STATE_SPACE
	{
		constexpr size_t nbits = 7;
		constexpr size_t rbits = 4;
		constexpr bool arithmetic = Modular;
		constexpr size_t NR_VALUES = (1 << nbits);
		using blocktype = uint32_t;

		fixpnt<nbits, rbits, arithmetic, blocktype> a, b, c, d;
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
		constexpr bool arithmetic = Modular;
		using blocktype = uint32_t;
		fixpnt<nbits, rbits, arithmetic, blocktype> a, b, c, d;

		for (int i = -16; i < 16; ++i) {
			a = i;
			cout << to_binary(i) << ' ' << a << ' ' << to_binary(a) << ' ' << to_binary(-a) << ' ' << -a << ' ' << to_binary(-i) << endl;
		}
	}
#endif // LATER

	if (nrOfFailedTestCases > 0) {
		cout << "FAIL" << endl;
	}
	else {
		cout << "PASS" << endl;
	}
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::unum::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
