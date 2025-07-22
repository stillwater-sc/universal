// api.cpp: test suite runner for class interface tests of the static (fixed-size) decimal integer type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the fixpnt template environment
// first: enable general or specialized fixed-point configurations
#define INTEGER_FAST_SPECIALIZATION
// second: enable/disable fixpnt arithmetic exceptions
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/integer/integer.hpp>
#include <universal/verification/test_reporters.hpp> 

///////////////////////////// TBD!!!! ///////////////////////////////////
// this is currently just a binary integer skeleton until we can figure
// out how to name this decimal integer class


int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "decimal integer class API test suite ";
	std::string test_tag    = "decimal integer class API";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	//// MODULAR integers

	// whole numbers
	{
		int start = nrOfFailedTestCases;
		bool exceptionThrown;
		integer<16, std::uint16_t, IntegerNumberType::WholeNumber> a, b, c;
		try {
			exceptionThrown = false;
			a = 0;  // can't assign 0 to a whole number
		}
		catch (const integer_encoding_exception& e) {
			std::cout << "Correctly caught illegal assignment: " << e.what() << '\n';
			exceptionThrown = true;
		}
		if (!exceptionThrown) {
			std::cout << "Incorrect: illegal assignment to 0 did not throw an exception\n";
			++nrOfFailedTestCases;
		}

		// whole numbers cannot be negative
		try {
			exceptionThrown = false;
			b = -1;  // can't assign negative values to a whole number
		}
		catch (const integer_encoding_exception& e) {
			std::cout << "Correctly caught illegal assignment: " << e.what() << '\n';
			exceptionThrown = true;
		}
		if (!exceptionThrown) {
			std::cout << "Incorrect: illegal assignment to negative value did not throw an exception\n";
			++nrOfFailedTestCases;
		}

		try {
			exceptionThrown = false;
			a = 1;
			b = 2;
			c = a / b;  // can't represent result
			std::cout << a << " / " << b << " = " << c << '\n';
		}
		catch (const integer_encoding_exception& e) {
			std::cout << "Correctly caught impossible result: " << e.what() << '\n';
			exceptionThrown = true;
		}
		if (!exceptionThrown) {
			std::cout << "Incorrect: impossible value did not throw an exception\n";
			++nrOfFailedTestCases;
		}

		try {
			exceptionThrown = false;
			a = 1;
			b = 2;
			c = a - b;  // can't represent result
			std::cout << a << " - " << b << " = " << c << '\n';
		}
		catch (const integer_encoding_exception& e) {
			std::cout << "Correctly caught impossible result: " << e.what() << '\n';
			exceptionThrown = true;
		}
		if (!exceptionThrown) {
			std::cout << "Incorrect: impossible value did not throw an exception\n";
			++nrOfFailedTestCases;
		}

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : whole number test cases\n";
		}
	}

	// natural numbers
	{
		int start = nrOfFailedTestCases;
		bool exceptionThrown;
		integer<16, std::uint16_t, IntegerNumberType::NaturalNumber> a, b, c;

		// natural numbers cannot be negative
		try {
			exceptionThrown = false;
			b = -1;  // can't assign negative values to a natural number
		}
		catch (const integer_encoding_exception& e) {
			std::cout << "Correctly caught illegal assignment: " << e.what() << '\n';
			exceptionThrown = true;
		}
		if (!exceptionThrown) {
			std::cout << "Incorrect: illegal assignment to negative value did not throw an exception\n";
			++nrOfFailedTestCases;
		}

		try {
			exceptionThrown = false;
			a = 1;
			b = 2;
			c = a / b;  // natural numbers can represent 0
			if (reportTestCases) std::cout << a << " / " << b << " = " << c << '\n';
		}
		catch (...) {
			std::cout << "Incorrect: exception thrown\n";
			++nrOfFailedTestCases;
		}

		try {
			exceptionThrown = false;
			a = 1;
			b = 2;
			c = a - b;  // natural numbers can represent negative: throw
			if (reportTestCases) std::cout << a << " - " << b << " = " << c << '\n';
		}
		catch (const integer_encoding_exception& e) {
			std::cout << "Correctly caught exception: " << e.what() << '\n';
			exceptionThrown = true;
		}
		if (!exceptionThrown) {
			std::cout << "Incorrect: illegal assignment to negative value did not throw an exception\n";
			++nrOfFailedTestCases;
		}

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : natural number test cases\n";
		}
	}

	// construction
	{
		int start = nrOfFailedTestCases;
		// default construction using default arithmetic (Modulo) and default BlockType (uint8_t)
		integer<8> a(0), b(-8), c(7), d(-7);
		
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a != (1 + b + c)) ++nrOfFailedTestCases;
		if (a - 1 != (b + c)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : " << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		}
	}

	// assignment
	{
		int start = nrOfFailedTestCases;
		integer<128, std::uint32_t> a, b;

		a = -1;
		b = 1;
		if (a != -b) ++nrOfFailedTestCases;
		std::string s("123456789");
		integer<64, std::uint8_t> c(s);
		integer<64, std::uint8_t> d;
		d.assign(s);
		if (c != d) ++nrOfFailedTestCases;
		if (convert_to_decimal_string(c) != s) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : " << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//// improving efficiency for bigger integers through explicit BlockType specification

	{
		int start = nrOfFailedTestCases;
		// construction with explicit arithmetic type and BlockType
		integer<16, std::uint16_t, IntegerNumberType::IntegerNumber> a(0), b(-2048), c(2047), d(-2047);
		if (a != (c + d)) ++nrOfFailedTestCases;
		if (a - 1 != (b + c)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
//			std::cout << "FAIL : construction " << to_binary(a) << ' ' << to_binary(b) << ' ' << to_binary(c) << ' ' << to_binary(d) << '\n';
			std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// selectors

		// type tag to identify the type without having to depend on demangle
	{
		using Integer = integer<16, uint16_t, IntegerNumberType::IntegerNumber>;
		Integer a{ 0 };
		std::cout << "type identifier : " << type_tag(a) << '\n';
		std::cout << "type identifier : " << type_tag(integer< 8>()) << '\n';
		std::cout << "type identifier : " << type_tag(integer< 8, uint16_t, IntegerNumberType::WholeNumber>()) << '\n';
		std::cout << "type identifier : " << type_tag(integer<32, uint32_t, IntegerNumberType::IntegerNumber>()) << '\n';
		std::cout << "type identifier : " << type_tag(integer<64, uint64_t, IntegerNumberType::IntegerNumber>()) << '\n';
		std::cout << "type identifier : " << type_tag(integer<96, uint32_t, IntegerNumberType::NaturalNumber>()) << '\n';
	}

	{
		int start = nrOfFailedTestCases;
		constexpr unsigned nbits = 8;
		integer<nbits, std::uint8_t, IntegerNumberType::IntegerNumber> a, b;
		a = 1;
		if (a.test(4)) ++nrOfFailedTestCases;
		if (!a.test(0)) ++nrOfFailedTestCases;
		b.setbits(0x01); // set the ULP
		if (!b.at(0)) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : selectors\n";
			std::cout << a << ' ' << b << '\n';
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// modifiers

	{
		int start = nrOfFailedTestCases;
		// state/bit management
		constexpr size_t nbits = 8;
		integer<nbits, std::uint8_t, IntegerNumberType::IntegerNumber> a, b, c, d;
		// set all bits of 'a' which represents -1
		for (unsigned i = 0; i < nbits; ++i) {
			a.setbit(i, true);
		}
		b.setbits(0x0f);
		if ((a + b) == 0) ++nrOfFailedTestCases;
		c = b;
		// manually flip the bits of b: don't use flip() as we are going to confirm flip() is correct
		for (unsigned i = 0; i < nbits; ++i) {
			b.setbit(i, !b.test(i));
		}
		c.flip();  // in-place 1's complement, so now b and c are the same
		if (b != c) ++nrOfFailedTestCases;	
		d.setbits(0xFFFFFFF);
		if (0 == d) ++nrOfFailedTestCases;
		d.setzero();
		if (d != 0) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : modifiers\n";
			std::cout << a << ' ' << b << ' ' << c << ' ' << d << '\n';
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// complements
	{
		int start = nrOfFailedTestCases;
		constexpr size_t nbits = 8;
		integer<nbits, std::uint8_t, IntegerNumberType::IntegerNumber> a, b;
		a.setbits(0xFF);
		b = onesComplement(a);
		if (b != 0) ++nrOfFailedTestCases;
		a = -1;
		b = twosComplement(a);
		if (b != 1) ++nrOfFailedTestCases;
		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL : complements 1\n";
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// arithmetic

	{
		int start = nrOfFailedTestCases;
		constexpr size_t nbits = 16;
		using blocktype = std::uint8_t;
		integer<nbits, blocktype, IntegerNumberType::IntegerNumber> a, b, c, d, e(SpecificValue::minpos);
		a.maxpos();
		b.maxneg();
		c.minpos();
		d.minneg();

		if ((a + b) != -1) ++nrOfFailedTestCases;
		if ((c + d) != 0) ++nrOfFailedTestCases;
		if ((a + c) != b) ++nrOfFailedTestCases;
		if ((a - a) != (b - b)) ++nrOfFailedTestCases;
		e += e;
		e -= c;
		if (c != e) ++nrOfFailedTestCases;

		a = 1;
		b = 10;
		c = 100;
		d = 1000;
		if (a * d != d) ++nrOfFailedTestCases;
		if (d / c != b) ++nrOfFailedTestCases;
		if (d % a != 0) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: arithmetic\n";
		}
	}

	///////////////////////////////////////////////////////////////////////////////////
	// logic, in particular, all the literal constant combinations
	{
		int start = nrOfFailedTestCases;
		constexpr size_t nbits = 8;
		using blocktype = uint32_t;
		integer<nbits, blocktype, IntegerNumberType::IntegerNumber> a, b, c, d;
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
			std::cout << "FAIL: logic operators\n";
		}
	}


	///////////////////////////////////////////////////////////////////////////////////
	// printing of large integers
	{
		constexpr size_t nbits = 8;
		integer<nbits, std::uint8_t, IntegerNumberType::IntegerNumber> a{ 1 };
		std::cout << std::showpos;
		for (unsigned i = 0; i < nbits; ++i) {
			std::cout << to_binary(a) << " : ";
			std::cout << a << '\n';
			a *= 2;
		}
		a.setbits(0x80);
		std::cout << a << '\n';

		uint8_t a0, b0;
		a0 = 0x80;
		b0 = 100;
		int8_t a0s = int8_t(a0);
		int8_t b0s = int8_t(b0);
		uint8_t c0 = static_cast<uint8_t>(a0s / b0s);
		std::cout << to_binary(c0, true, 8) << " : " << unsigned(c0) << '\n';  // -128/100 = -1

	}
	{
		constexpr size_t nbits = 8;
		integer<nbits, std::uint8_t, IntegerNumberType::IntegerNumber> a{ 1 };
		std::cout << std::oct;
		for (unsigned i = 0; i < nbits; ++i) {
			std::cout << to_binary(a) << " : ";
			std::cout << '0' << a << '\n';
			a *= 2;
		}
	}
	{
		constexpr size_t nbits = 8;
		integer<nbits, std::uint8_t, IntegerNumberType::IntegerNumber> a{ 1 };
		std::cout << std::hex;
		for (unsigned i = 0; i < nbits; ++i) {
			std::cout << to_binary(a) << " : ";
			std::cout << "0x" << a << '\n';
			a *= 2;
		}
	}
	std::cout << std::dec;
	{
		integer<32, std::uint32_t, IntegerNumberType::IntegerNumber> a{ 1 };
		std::cout << std::showpos;
		for (unsigned i = 0; i < 32; ++i) {
			std::cout << to_binary(a) << " : ";
			std::cout << std::setw(11) << a << '\n';
			a *= 2;
		}
		a.setbits(0x8000'0001);
		std::cout << to_binary(a) << " : ";
		std::cout << a << '\n';
		std::cout << std::noshowpos;
		a = -1;
		for (unsigned i = 0; i < 32; ++i) {
			std::cout << to_binary(a) << " : ";
			std::cout << std::setw(11) << a << '\n';
			a *= 2;
		}
		int32_t _a = -1;
		for (unsigned i = 0; i < 32; ++i) {
			std::cout << to_binary(_a, false, 32) << " : " << std::setw(11) << _a << '\n';
			_a *= 2;
		}
		_a = 0x8000'0001;
		std::cout << to_binary(_a, false, 32) << " : " << _a << '\n';
	}

	{
		integer<32, std::uint32_t, IntegerNumberType::IntegerNumber> a{ 128 };
		std::cout << a << '\n';
	}

	{
		integer<1024, std::uint32_t, IntegerNumberType::IntegerNumber> a;
		a = 1;
		constexpr unsigned NR_DIGITS = 10;
		for (unsigned i = 0; i < NR_DIGITS; ++i) {
			std::cout << std::setw(NR_DIGITS) << a << '\n';
			a = a * 10;
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
