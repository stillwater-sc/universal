//  quires.cpp : test suite for IEEE float quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_reporters.hpp>

#include <iostream>
#include <string>

template<size_t nbits, size_t es, size_t capacity>
int ValidateQuireAccumulation() {

	int nrOfFailedTests = 0;

	return nrOfFailedTests;
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
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

int main()
try {
	using namespace sw::universal;

	std::string test_suite          = "posit<> quire accumulation";
	std::string test_tag            = "posit<> quire";
	bool        reportTestCases     = false;
	int         nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING
	// float
	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	constexpr size_t capacity = 2; // for testing the accumulation capacity of the quire can be small
	//constexpr size_t fbits = 5;

	//GenerateUnsignedIntAssignments<nbits, es, capacity>();
	//GenerateSignedIntAssignments<nbits, es, capacity>();
	//GenerateUnsignedIntAssignments<8, 2, capacity>();

	//GenerateValueAssignments<nbits, es, capacity, fbits>();

	typedef sw::ieee::quire<nbits, es, 2> QuireFloat;
	typedef sw::ieee::quire<64, 11, 2> QuireDouble;

	std::cout << std::endl;
	std::cout << "Creating quires for float and double arithmetic" << std::endl;
	float f = 1.555555555555e-10f;
	QuireFloat fquire(f);
	std::cout << "quire<32, 8, 2>: qbits: " << QuireFloat::qbits << " dynamic range: " << QuireFloat::escale << " lower range: " << QuireFloat::half_range << " upper range: " << QuireFloat::upper_range << std::endl;
	std::cout << "float:  " << std::setw(15) << f << " " << fquire << std::endl;

	double d = 1.555555555555e16;
	QuireDouble dquire(d);
	std::cout << "quire<64, 11, 2>: qbits: " << QuireDouble::qbits << " dynamic range: " << QuireDouble::escale << " lower range: " << QuireDouble::half_range << " upper range: " << QuireDouble::upper_range << std::endl;
	std::cout << "double: " << std::setw(15) << d << " " << dquire << std::endl;

	std::cout << std::endl;
	// quire for float nbits= 32 es = 8
	quire<32, 8, capacity> q;
	sw::universal::internal::value<54> maxpos, maxpos_squared, minpos, minpos_squared;
	constexpr double dmax = std::numeric_limits<float>::max();
	maxpos = dmax;
	maxpos_squared = dmax*dmax;
	std::cout << "maxpos * maxpos = " << sw::universal::internal::to_triple(maxpos_squared) << std::endl;
	constexpr double dmin = std::numeric_limits<float>::min();
	minpos = dmin;
	minpos_squared = dmin*dmin;
	std::cout << "minpos * minpos = " << sw::universal::internal::to_triple(minpos_squared) << std::endl;
	sw::universal::internal::value<54> c(maxpos_squared);

	std::cout << "Add/Subtract propagating carry/borrows to and from capacity segment" << std::endl;
	q.clear();
	sw::universal::internal::value<54> v = maxpos;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << " <- entering capacity bits" << std::endl;
	q += c;		std::cout << q << " <- adding maxpos^2" << std::endl;
	q += c;     std::cout << q << " <- flipping another capacity bit" << std::endl;
	q += -c;	std::cout << q << " <- subtracting maxpos^2" << std::endl;
	q += -c;	std::cout << q << " <- subtracting maxpos^2" << std::endl;
	q += -v;	std::cout << q << " <- removing the capacity bit" << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << " <- should be zero" << std::endl;

	std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << std::endl;
	q = 0;
	v = 0.5;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << " <- should be zero" << std::endl;

	std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << std::endl;
	q.clear();  // equivalent to q = 0, but more articulate/informative
	v = 3.875 + 0.0625; std::cout << "v " << to_triple(v) << std::endl; // the input value is 11.1111 so hidden + 5 fraction bits
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << " <- should be zero" << std::endl;

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
/* TODO: How to unify this with posit quires so they can co-exist in the same code
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
*/
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
