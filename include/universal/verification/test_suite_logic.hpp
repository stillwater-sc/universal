#pragma once
// test_suite_logic.hpp : boolean logic relationship test suite for arbitrary universal number systems
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
// This usually entails setting environment variables, such as
// #define POSIT_THOW_ARITHMETIC_EXCEPTIONS 1
// or
// #define AREAL_FAST_SPECIALIZATION 1

#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>  // error/success reporting

namespace sw::universal {


/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
///                             BOOLEAN LOGIC TEST SUITES                             ///
/////////////////////////////////////////////////////////////////////////////////////////

	template<typename TestType>
	int VerifyLogicEqual() {
		constexpr size_t nbits = TestType::nbits;  // standard interface to Universal number system classes
		size_t NR_TEST_CASES = (size_t(1) << nbits);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.set_raw_bits(j);
				// generate the golden reference
				bool ref = (i == j);	 // the same bit pattern should clearly be the same value
				// generate the number system's answer
				bool result = (a == b);
				if (ref != result) {
					nrOfFailedTestCases++;
					std::cout << a << " == " << b << " fails: reference is " << ref << " actual is " << result << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	template<typename TestType>
	int VerifyLogicNotEqual() {
		constexpr size_t nbits = TestType::nbits;  // standard interface to Universal number system classes
		size_t NR_TEST_CASES = (size_t(1) << nbits);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.set_raw_bits(j);
				// generate the golden reference
				bool ref = (i != j);	 // the same bit pattern should clearly be the same value
				// generate the number system's answer
				bool result = (a != b);
				if (ref != result) {
					nrOfFailedTestCases++;
					std::cout << a << " != " << b << " fails: reference is " << ref << " actual is " << result << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	template<typename TestType>
	int VerifyLogicLessThan() {
		constexpr size_t nbits = TestType::nbits;  // standard interface to Universal number system classes
		size_t NR_TEST_CASES = (size_t(1) << nbits);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			TestType a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				TestType b;
				b.set_raw_bits(j);

				// generate the golden reference
				bool ref;
				if (a.isnan() || b.isnan()) {
					return false;
				}
				else {
					// same ordering as IEEE floats
					ref = (double(a) < double(b));
				}
				bool result = (a < b);
				if (ref != result) {
					nrOfFailedTestCases++;
					std::cout << a << " < " << b << " fails: reference is " << ref << " actual is " << result << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

#if 0

	// Posit greater-than diverges from IEEE float in dealing with INFINITY/NAN
	// Any number is greater-than posit NaR
	template<size_t nbits, size_t es>
	int VerifyPositLogicGreaterThan() {
		constexpr size_t max = nbits > 10 ? 10 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.set_raw_bits(j);

				// generate the golden reference
				bool ref = (double(a) > double(b)); // same behavior as IEEE floats 
				if (!a.isnar() && b.isnar()) {
					ref = true; // special case of posit NaR
				}
				bool presult = (a > b);
				if (ref != presult) {
					nrOfFailedTestCases++;
					std::cout << a << " > " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit less-or-equal-than diverges from IEEE float in dealing with INFINITY/NAN
	// Posit NaR is smaller or equal than any other value
	template<size_t nbits, size_t es>
	int VerifyPositLogicLessOrEqualThan() {
		constexpr size_t max = nbits > 10 ? 10 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.set_raw_bits(j);

				// set the golden reference			
				bool ref = (double(a) <= double(b));// same behavior as IEEE floats
				if (a.isnar()) {
					// special case of posit <= for NaR
					ref = true;
				}
				bool presult = (a <= b);
				if (ref != presult) {
					nrOfFailedTestCases++;
					std::cout << a << " <= " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit greater-or-equal-than diverges from IEEE float in dealing with INFINITY/NAN
	// Any number is greater-or-equal-than posit NaR
	template<size_t nbits, size_t es>
	int VerifyPositLogicGreaterOrEqualThan() {
		constexpr size_t max = nbits > 10 ? 10 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.set_raw_bits(j);

				// set the golden reference			
				bool ref = (double(a) >= double(b));// same behavior as IEEE floats
				if (b.isnar()) {
					// special case of posit >= for NaR
					ref = true;
				}
				bool presult = (a >= b);
				if (ref != presult) {
					nrOfFailedTestCases++;
					std::cout << a << " >= " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

#endif

} // namespace sw::universal
