// conversion.cpp: conversion tests for unum Type I
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#define UNUM_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/unum/unum.hpp>
#include <universal/number/unum/manipulators.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "unum Type I conversion tests";
	std::string test_tag    = "unum conversion";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////////////////////////////////////////////////////////////////////////
	// double -> unum -> double round-trip for exact values
	std::cout << "*** exact round-trip conversions\n";
	{
		int start = nrOfFailedTestCases;
		using Unum = unum<3, 4>;

		// powers of 2 should be exact
		double test_values[] = { 1.0, 2.0, 4.0, 0.5, 0.25, 8.0, 16.0, 0.125 };
		for (double v : test_values) {
			Unum u;
			u = v;
			double rt = u.to_double();
			if (rt != v) {
				++nrOfFailedTestCases;
				std::cout << "  FAIL: " << v << " -> " << to_binary(u)
				          << " -> " << rt << '\n';
			}
			// powers of 2 should be exact (ubit=0)
			if (u.ubit()) {
				++nrOfFailedTestCases;
				std::cout << "  FAIL: " << v << " ubit should be 0 (exact)\n";
			}
		}

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: exact round-trip\n";
		}
		else {
			std::cout << "  all powers of 2 round-trip exactly\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// exact fractions
	std::cout << "*** exact fraction conversions\n";
	{
		int start = nrOfFailedTestCases;
		using Unum = unum<3, 4>;

		// 1.5 = 1 + 0.5 = 1.1 in binary (1 fraction bit needed)
		Unum u;
		u = 1.5;
		double rt = u.to_double();
		if (rt != 1.5) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: 1.5 -> " << to_binary(u) << " -> " << rt << '\n';
		}
		if (u.ubit()) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: 1.5 ubit should be 0\n";
		}

		// 1.75 = 1.11 in binary (2 fraction bits)
		u = 1.75;
		rt = u.to_double();
		if (rt != 1.75) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: 1.75 -> " << to_binary(u) << " -> " << rt << '\n';
		}

		// 1.25 = 1.01 in binary (2 fraction bits)
		u = 1.25;
		rt = u.to_double();
		if (rt != 1.25) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: 1.25 -> " << to_binary(u) << " -> " << rt << '\n';
		}

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: exact fractions\n";
		}
		else {
			std::cout << "  all exact fractions round-trip correctly\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// inexact values should set ubit
	std::cout << "*** inexact values set ubit\n";
	{
		int start = nrOfFailedTestCases;
		using Unum = unum<2, 2>;  // small config: max 3 fraction bits

		// 1/3 cannot be represented exactly in binary
		Unum u;
		u = 1.0 / 3.0;
		if (!u.ubit()) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: 1/3 ubit should be 1 (inexact)\n";
		}
		std::cout << "  1/3 -> " << to_binary(u) << " : " << u
		          << (u.ubit() ? " (inexact)" : " (exact)") << '\n';

		// pi cannot be exact
		u = 3.14159265358979;
		if (!u.ubit()) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: pi ubit should be 1\n";
		}
		std::cout << "  pi  -> " << to_binary(u) << " : " << u
		          << (u.ubit() ? " (inexact)" : " (exact)") << '\n';

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: inexact ubit\n";
		}
		else {
			std::cout << "  inexact values correctly set ubit\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// negative values
	std::cout << "*** negative value conversions\n";
	{
		int start = nrOfFailedTestCases;
		using Unum = unum<3, 4>;

		Unum u;
		u = -1.0;
		if (u.to_double() != -1.0) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: -1.0 -> " << u.to_double() << '\n';
		}
		if (!u.sign()) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: -1.0 sign should be true\n";
		}

		u = -4.5;
		if (u.to_double() != -4.5) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: -4.5 -> " << u.to_double() << '\n';
		}

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: negative values\n";
		}
		else {
			std::cout << "  negative values convert correctly\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// integer assignment
	std::cout << "*** integer assignment\n";
	{
		int start = nrOfFailedTestCases;
		using Unum = unum<3, 4>;

		Unum u;
		u = 1;
		if (u.to_double() != 1.0) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: int 1 -> " << u.to_double() << '\n';
		}

		u = -7;
		if (u.to_double() != -7.0) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: int -7 -> " << u.to_double() << '\n';
		}

		u = 0;
		if (!u.iszero()) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: int 0 should be zero\n";
		}

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: integer assignment\n";
		}
		else {
			std::cout << "  integer assignment works correctly\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// unary negation
	std::cout << "*** unary negation\n";
	{
		int start = nrOfFailedTestCases;
		using Unum = unum<3, 4>;

		Unum a;
		a = 3.5;
		Unum b = -a;
		if (b.to_double() != -3.5) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: -(3.5) -> " << b.to_double() << '\n';
		}

		// double negation
		Unum c = -b;
		if (c.to_double() != 3.5) {
			++nrOfFailedTestCases;
			std::cout << "  FAIL: -(-(3.5)) -> " << c.to_double() << '\n';
		}

		if (nrOfFailedTestCases - start > 0) {
			std::cout << "FAIL: negation\n";
		}
		else {
			std::cout << "  unary negation works correctly\n";
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// display conversions for visual verification
	std::cout << "\n*** conversion table for unum<2,2>\n";
	{
		using Unum = unum<2, 2>;
		double values[] = { 0.0, 0.25, 0.5, 1.0, 1.5, 2.0, 4.0, -1.0, -2.5 };
		for (double v : values) {
			Unum u;
			u = v;
			std::cout << "  " << std::setw(6) << v
			          << " -> " << std::setw(30) << std::left << to_binary(u) << std::right
			          << " -> " << std::setw(8) << u
			          << "  " << pretty_print(u) << '\n';
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
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
