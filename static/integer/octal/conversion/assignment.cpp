// assignment.cpp: conversion tests for octal positional integer type
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#define POSITIONAL_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/positional/positional.hpp>
#include <universal/verification/test_reporters.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "octal positional integer assignment/conversion";
	std::string test_tag    = "oint conversion";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// round-trip through native int
	{
		int start = nrOfFailedTestCases;
		int testValues[] = { 0, 1, -1, 7, -7, 63, -63, 100, -100, 511, -511 };
		for (int v : testValues) {
			oi16 a(v);
			if (int(a) != v) {
				if (reportTestCases) std::cerr << "FAIL: int round-trip for " << v << " got " << int(a) << '\n';
				++nrOfFailedTestCases;
			}
		}
		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: int round-trip\n";
	}

	// round-trip through long long
	{
		int start = nrOfFailedTestCases;
		long long testValues[] = { 0LL, 1LL, -1LL, 1000LL, -1000LL, 100000LL, -100000LL };
		for (long long v : testValues) {
			positional<16, 8> a(v);
			if (static_cast<long long>(a) != v) {
				if (reportTestCases) std::cerr << "FAIL: long long round-trip for " << v << '\n';
				++nrOfFailedTestCases;
			}
		}
		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: long long round-trip\n";
	}

	// assignment from float and double
	{
		int start = nrOfFailedTestCases;
		oi16 a;
		a = 42.7f;
		if (int(a) != 42) ++nrOfFailedTestCases;  // truncation
		a = -99.9;
		if (int(a) != -99) ++nrOfFailedTestCases;  // truncation

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: float/double assignment\n";
	}

	// double round-trip
	{
		int start = nrOfFailedTestCases;
		oi8 a(123);
		double d = double(a);
		oi8 b(static_cast<int>(d));
		if (int(a) != int(b)) ++nrOfFailedTestCases;

		if (nrOfFailedTestCases - start > 0)
			std::cout << "FAIL: double round-trip\n";
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
