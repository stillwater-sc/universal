#pragma once
// test_suite_exceptions.hpp : test suite for arithmetic exceptions for arbitrary universal number systems
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>

// CALLING ENVIRONMENT PREREQUISITE!!!!!
// We want the test suite to be used with different configurations of number systems
// so the calling environment needs to set the configuration
// This usually entails setting environment variables, such as <NUMBERSYSTEM>_THROW_ARITHMETIC_EXCEPTIONS
// as a function of the configured state of the number system.
// If it is not set, default is to turn it on.
//#ifndef THROW_ARITHMETIC_EXCEPTION
//#define THROW_ARITHMETIC_EXCEPTION 1
//#endif

#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/common/exceptions.hpp>

namespace sw::universal {


////////////////////////////// VERIFICATION TEST SUITES ///////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
///                     ARITHMETIC EXCEPTIONS TEST SUITES                           ///
///////////////////////////////////////////////////////////////////////////////////////

template<typename Scalar>
int TestDivisionByZero() {
	using namespace sw::universal;
	bool thrownAndCaught = false;
	int nrOfFailedTestCases = 0;
	try {
		Scalar a{ 1 }, b{ 0 }, c;
		c = a / b;   // this will throw rational_divide_by_zero();
	}
	catch (const universal_arithmetic_error& err) {
		std::cerr << "correctly caught arithmetic error: " << err.what() << std::endl;
		thrownAndCaught = true;
	}
	catch (...) {
		std::cerr << "incorrectly caught unknown error" << std::endl;
		++nrOfFailedTestCases;
	}
	if (!thrownAndCaught) {
		++nrOfFailedTestCases;
	}
	return nrOfFailedTestCases;
}

template<typename Scalar>
int TestNegativeSqrtArgument() {
	using namespace sw::universal;
	bool thrownAndCaught = false;
	int nrOfFailedTestCases = 0;
	try {
		Scalar a{ -1 };
		sqrt(a);
	}
	catch (const universal_arithmetic_error& err) {
		std::cerr << "correctly caught arithmetic error: " << err.what() << std::endl;
		thrownAndCaught = true;
	}
	catch (...) {
		std::cerr << "incorrectly caught unknown error" << std::endl;
		++nrOfFailedTestCases;
	}
	if (!thrownAndCaught) {
		++nrOfFailedTestCases;
	}
	return nrOfFailedTestCases;
}

} // namespace sw::universal
