//  quires.cpp : test suite for Universal cfloat quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/cfloat/fdp.hpp>
#include <universal/number/quire/quire.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <string>

template<size_t nbits, size_t es, size_t capacity>
int ValidateQuireAccumulation() {

	int nrOfFailedTests = 0;

	return nrOfFailedTests;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
// #undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#	undef REGRESSION_LEVEL_1
#	undef REGRESSION_LEVEL_2
#	undef REGRESSION_LEVEL_3
#	undef REGRESSION_LEVEL_4
#	define REGRESSION_LEVEL_1 1
#	define REGRESSION_LEVEL_2 1
#	define REGRESSION_LEVEL_3 0
#	define REGRESSION_LEVEL_4 0
#endif

#include <universal/internal/blocktriple/blocktriple.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite          = "cfloat<> quire accumulation";
	std::string test_tag            = "cfloat<> quire";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// float
	constexpr size_t nbits = 8;
	constexpr size_t es = 3;
	constexpr size_t capacity = 5; // for testing the accumulation capacity of the quire can be small
	//constexpr size_t fbits = 5;

	using Scalar = cfloat<nbits, es, std::uint8_t, true, false, false>;
	using BT     = blocktriple<4, BlockTripleOperator::MUL, std::uint8_t>;

	using QuireFloat = quire<Scalar>;

	std::cout << '\n';
	// quire for our Scalar
	quire<Scalar, capacity> q;
	Scalar                 maxpos(SpecificValue::maxpos), minpos(SpecificValue::minpos);
	auto                   v = quire_mul(maxpos, Scalar(1.0));
	auto                   c = quire_mul(maxpos, maxpos);

	std::cout << to_binary(v) << " : " << v << '\n';
	std::cout << to_binary(c) << " : " << c << '\n';

	BT bt_v(v);

	std::cout << "Add/Subtract propagating carry/borrows to and from capacity segment" << '\n';
	q.clear();
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << " <- entering capacity bits" << '\n';
	q += c;		std::cout << q << " <- adding maxpos^2" << '\n';
	q += c;     std::cout << q << " <- flipping another capacity bit" << '\n';
	q += -c;	    std::cout << q << " <- subtracting maxpos^2" << '\n';
	q += -c;	std::cout << q << " <- subtracting maxpos^2" << '\n';
	q += -v;	std::cout << q << " <- removing the capacity bit" << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << " <- should be zero" << '\n';

	std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << '\n';
	q = 0;
	v = 0.5;
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << " <- should be zero" << '\n';

	std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << '\n';
	q.clear();  // equivalent to q = 0, but more articulate/informative
	v = 3.875 + 0.0625; std::cout << "v " << to_triple(v) << '\n'; // the input value is 11.1111 so hidden + 5 fraction bits
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += v;		std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << '\n';
	q += -v;	std::cout << q << " <- should be zero" << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;  // ignore errors
#else

#	if REGRESSION_LEVEL_1

#	endif

#	if REGRESSION_LEVEL_2

#	endif

#	if REGRESSION_LEVEL_3

#	endif

#	if REGRESSION_LEVEL_4

#	endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << '\n';
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
