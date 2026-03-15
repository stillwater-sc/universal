// api.cpp: class interface tests for unum Type I
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// Configure the unum template environment
#define UNUM_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/unum/unum.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "unum Type I class interface tests";
	std::string test_tag    = "unum api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	// type tag and configuration
	std::cout << "*** type configuration\n";
	{
		unum<2, 2> u;
		std::cout << type_tag(u) << '\n';
		std::cout << "  max exponent bits: " << unum<2, 2>::maxesize << '\n';
		std::cout << "  max fraction bits: " << unum<2, 2>::maxfsize << '\n';
		std::cout << "  utag size:         " << unum<2, 2>::utagsize << '\n';
		std::cout << "  max word bits:     " << unum<2, 2>::maxbits << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// construction and zero
	std::cout << "*** construction and zero\n";
	{
		int start = nrOfFailedTestCases;
		unum<2, 2> a;
		if (!a.iszero()) ++nrOfFailedTestCases;

		unum<3, 3> b;
		b.setzero();
		if (!b.iszero()) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: construction/zero\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// NaN
	std::cout << "*** NaN\n";
	{
		int start = nrOfFailedTestCases;
		unum<2, 2> nan;
		nan.setnan();
		if (!nan.isnan()) ++nrOfFailedTestCases;

		unum<3, 3> nan2;
		nan2.setnan();
		if (!nan2.isnan()) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: NaN\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// setbits and field decoding
	std::cout << "*** setbits and utag decoding\n";
	{
		int start = nrOfFailedTestCases;
		unum<2, 2> u;

		// For unum<2,2>: utag = 1(ubit) + 2(fsize) + 2(esize) = 5 bits
		// Set a known bit pattern and verify field decoding
		// Pattern: ubit=0, fsize=01(=1), esize=10(=2)
		// Bits from LSB: 0 | 01 | 10 = 0b10010 = 18
		u.setbits(0b10010);
		if (u.ubit() != false) ++nrOfFailedTestCases;
		if (u.fsize() != 1) ++nrOfFailedTestCases;
		if (u.esize() != 2) ++nrOfFailedTestCases;

		// Pattern: ubit=1, fsize=11(=3), esize=01(=1)
		// Bits from LSB: 1 | 11 | 01 = 0b01111 = 15
		u.setbits(0b01111);
		if (u.ubit() != true) ++nrOfFailedTestCases;
		if (u.fsize() != 3) ++nrOfFailedTestCases;
		if (u.esize() != 1) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: setbits/utag decoding\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// to_binary display
	std::cout << "*** to_binary\n";
	{
		unum<2, 2> u;
		u.setzero();
		std::cout << "  zero:  " << to_binary(u) << " : " << u << '\n';

		u.setnan();
		std::cout << "  NaN:   " << to_binary(u) << " : " << u << '\n';

		// set a simple bit pattern: esize=0 (1 exp bit), fsize=0 (0 frac bits), ubit=0
		// sign=0, exp=1 -> value = 2^(1 - bias), bias = 2^0 - 1 = 0, so 2^1 = 2
		// Actually with esize=0: 1 exponent bit, bias = 0, exp=1 -> 2^1 = 2
		// Bit layout: sign(0) exp(1) fsize(00) esize(00) ubit(0) = 0b0_1_00_00_0
		// From LSB: ubit=0, fsize=00, esize=00, exp=1, sign=0
		// = 0b0100000 but that's 7 bits. Let me compute: bit positions
		// bit 0: ubit=0, bits 1-2: fsize=00, bits 3-4: esize=00, bit 5: exp=1, bit 6: sign=0
		u.setbits(0b0100000);
		std::cout << "  2.0:   " << to_binary(u) << " : " << u << '\n';
		std::cout << "  info:  " << info_print(u) << '\n';
		std::cout << "  color: " << color_print(u) << '\n';
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// nbits_used for different esize/fsize configurations
	std::cout << "*** word size varies with esize/fsize\n";
	{
		unum<2, 2> u;
		// esize=0, fsize=0: 1(sign) + 1(exp) + 0(frac) + 2(esize) + 2(fsize) + 1(ubit) = 7
		u.setbits(0);  // esize=0, fsize=0
		std::cout << "  esize=0, fsize=0: " << u.nbits_used() << " bits\n";

		// esize=3, fsize=3: 1 + 4(exp) + 3(frac) + 2 + 2 + 1 = 13
		// set esize=3 and fsize=3 in utag: bits 1-2=11, bits 3-4=11
		// = ubit(0) | fsize(11) | esize(11) = 0b11110
		u.setbits(0b11110);
		std::cout << "  esize=3, fsize=3: " << u.nbits_used() << " bits\n";
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// equality
	std::cout << "*** equality\n";
	{
		int start = nrOfFailedTestCases;
		unum<2, 2> a, b;
		a.setzero();
		b.setzero();
		if (a != b) ++nrOfFailedTestCases;

		a.setnan();
		b.setnan();
		if (a != b) ++nrOfFailedTestCases;  // bit-level equality

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: equality\n";
		}
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::unum_arithmetic_exception& err) {
	std::cerr << "Uncaught unum arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::unum_internal_exception& err) {
	std::cerr << "Uncaught unum internal exception: " << err.what() << std::endl;
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
