#pragma once
//  real_test_helpers.hpp : arbitrary real verification functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

#include <math/stub/classify.hpp> // fpclassify, isnormal, issubnormal, isinf, isnan
#include <universal/verification/test_reporters.hpp> 

namespace sw { namespace universal {

	template<typename SrcType, typename TestType>
	void ReportIntervalConversionError(const std::string& test_case, const std::string& op, SrcType input, const TestType& reference, const TestType& result) {
		// constexpr unsigned nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		auto old_precision = std::cerr.precision();
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(NUMBER_COLUMN_WIDTH) << input
			<< " did not convert to "
			<< std::setw(NUMBER_COLUMN_WIDTH) << reference << " instead it yielded  "
			<< std::setw(NUMBER_COLUMN_WIDTH) << result
			<< "  raw " << to_binary(result)
			<< std::setprecision(old_precision)
			<< std::endl;
	}

	template<typename SrcType, typename TestType>
	void ReportIntervalConversionSuccess(const std::string& test_case, const std::string& op, SrcType input, const TestType& reference, const TestType& result) {
		constexpr unsigned nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(NUMBER_COLUMN_WIDTH) << input
			<< " success            "
			<< std::setw(NUMBER_COLUMN_WIDTH) << result << " golden reference is "
			<< std::setw(NUMBER_COLUMN_WIDTH) << reference
			<< "  raw " << std::setw(nbits) << to_binary(result)
			<< std::endl;
	}

	template<typename SrcType, typename TestType>
	int Compare(SrcType input, const TestType& testValue, const TestType& reference, bool reportTestCases) {
		int fail = 0;
		if (testValue != reference) {
			fail++;
			if (reportTestCases)	ReportIntervalConversionError("FAIL", "=", input, reference, testValue);
		}
		else {
			//if (reportTestCases) ReportIntervalConversionSuccess("PASS", "=", input, reference, testValue);
		}
		return fail;
	}

	/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

		/// <summary>
		/// enumerate all conversion cases for a number system with ubits
		/// </summary>
		/// <typeparam name="TestType">the test configuration</typeparam>
		/// <typeparam name="SrcType">the source type to convert from</typeparam>
		/// <param name="tag">string to indicate what is being tested</param>
		/// <param name="reportTestCases">if true print results of each test case. Default is false.</param>
		/// <returns>number of failed test cases</returns>
	template<typename TestType, typename SrcType>
	int VerifyArealIntervalConversion(bool reportTestCases) {
		// areal<> is organized as a set of exact samples followed by an interval to the next exact value
		//
		// vprev    exact value          ######-0     ubit = false     some value [vprev,vprev]
		//          interval value       ######-1     ubit = true      (vprev, v)
		// v        exact value          ######-0     ubit = false     some value [v,v]
		//          interval value       ######-1     ubit = true      (v, vnext)
		// vnext    exact value          ######-0     ubit = false     some value [vnext,vnext]
		//          interval value       ######-1     ubit = true      (vnext, vnextnext)
		//
		// the assignment test can thus be constructed by enumerating the exact values of a configuration
		// and taking a -diff to obtain the interval value of vprev, 
		// and taking a +diff to obtain the interval value of v
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);

		const unsigned max = nbits > 20 ? 20 : nbits + 1;
		unsigned max_tests = (unsigned(1) << max);
		if (max_tests < NR_TEST_CASES) {
			std::cout << "VerifyArealIntervalConversion " << typeid(TestType).name() << ": NR_TEST_CASES = " << NR_TEST_CASES << " clipped by " << max_tests << std::endl;
		}

		// execute the test
		int nrOfFailedTests = 0;
		TestType minpos(sw::universal::SpecificValue::minpos);
		SrcType dminpos = SrcType(minpos);

		// NUT: number under test
		TestType nut;
		TestType debugTarget;
//		debugTarget.setbits(0x1FE); // set it to something to catch

		for (unsigned i = 0; i < NR_TEST_CASES && i < max_tests; i += 2) {
			TestType current, interval;
			SrcType testValue{ 0.0 };
			current.setbits(i);
			interval.setbits(i + 1);  // sets the ubit
			SrcType da = SrcType(current);

			// basic design of the test suite
			// generate a reference, called da, which is an IEEE native format (float/double/long double)
			// from that generate the test cases
			// da - delta = falls into the previous interval == (prev, current)
			// da         = is exact                         == [current]
			// da + delta = falls into the next interval     == (current, next)

			// debug condition to catch specific failures
//			if (current == debugTarget) {
//				std::cout << "found debug target : " << debugTarget << " : interval " << interval << std::endl;
//			}

			if (current.iszero()) {
				SrcType delta = SrcType(dminpos / 4.0);  // the test value between 0 and minpos
				if (current.sign()) {
					// da         = [-0]
					testValue = da;
					nut = testValue;
					if (!nut.iszero()) {
						// working around optimizing compilers ignoring or flipping the sign on 0
						nrOfFailedTests += Compare(testValue, nut, current, reportTestCases);
					}
					// da - delta = (-0,-minpos)
					testValue = da - delta;
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, interval, reportTestCases);
				}
				else {
					// da         = [0]
					testValue = da;
					nut = testValue;
					if (!nut.iszero()) {
						// working around optimizing compilers ignoring of flipping the sign on 0
						nrOfFailedTests += Compare(testValue, nut, current, reportTestCases);
					}
					// da + delta = (0,minpos)
					testValue = da + delta;
					if (isdenorm(testValue)) { std::cout << testValue << " is denormalized\n"; }
					nut = testValue;
					nrOfFailedTests += Compare(testValue, nut, interval, reportTestCases);
				}
			}
			else if (current.isinf(INF_TYPE_NEGATIVE)) {
				std::cout << "-inf tbd\n";
			}
			else if (current.isinf(INF_TYPE_POSITIVE)) {
				std::cout << "+inf tbd\n";
			}
			else if (current.isnan(NAN_TYPE_SIGNALLING)) {  // sign is true
				// can never happen as snan is odd, i.e. ubit = 1 and this loop enumerates only even encodings
			}
			else if (current.isnan(NAN_TYPE_QUIET)) {       // sign is false
				// can never happen as snan is odd, i.e. ubit = 1 and this loop enumerates only even encodings
			}
			else {
				TestType previous, previousInterval;
				previous.setbits(i - 2);
				previousInterval.setbits(i - 1);
				SrcType prev = SrcType(previous);
				SrcType delta = SrcType(SrcType(da - prev) / SrcType(2.0));  // NOTE: the sign will flip the relationship between the enumeration and the values
				int currentFailures = nrOfFailedTests;
				if (current == debugTarget) {
					std::cout << "previous: " << to_binary(previous) << " : " << previous << std::endl;
					std::cout << "interval: " << to_binary(previousInterval) << " : " << previousInterval << std::endl;
					std::cout << "current : " << to_binary(current) << " : " << current << std::endl;
					std::cout << "interval: " << to_binary(interval) << " : " << interval << std::endl;
					std::cout << "delta   : " << delta << " : " << to_binary(delta) << std::endl;
				}
				// da - delta = (prev,current) == previous + ubit = previous interval value
				testValue = da - delta;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, previousInterval, reportTestCases);
				// da         = [v]
				testValue = da;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, current, reportTestCases);
				// da + delta = (v+,next) == current + ubit = current interval value
				testValue = da + delta;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, interval, reportTestCases);

				if (nrOfFailedTests - currentFailures) {
					std::cout << "previous: " << to_binary(previous) << " : " << previous << std::endl;
					std::cout << "interval: " << to_binary(previousInterval) << " : " << previousInterval << std::endl;
					std::cout << "current : " << to_binary(current) << " : " << current << std::endl;
					std::cout << "interval: " << to_binary(interval) << " : " << interval << std::endl;
					std::cout << "delta   : " << delta << " : " << to_binary(delta) << std::endl;
				}

			}
			if (nrOfFailedTests > 24) {
				std::cout << "Too many errors: exiting\n";
				break;
			}
		}
		return nrOfFailedTests;
	}

	// validate the increment operator++
	template<unsigned nbits, unsigned es>
	int VerifyIncrement(bool reportTestCases)
	{
		std::vector< areal<nbits, es> > set;
		//	GenerateOrderedPositSet(set); // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

		int nrOfFailedTestCases = 0;

		areal<nbits, es> p, ref;
		// starting from NaR iterating from -maxpos to maxpos through zero
		for (typename std::vector < areal<nbits, es> >::iterator it = set.begin(); it != set.end() - 1; ++it) {
			p = *it;
			p++;
			ref = *(it + 1);
			if (p != ref) {
				if (reportTestCases) std::cout << " FAIL " << p << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

	// validate the decrement operator--
	template<unsigned nbits, unsigned es>
	int VerifyDecrement(bool reportTestCases)
	{
		std::vector< areal<nbits, es> > set;
		//	GenerateOrderedPositSet(set); // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

		int nrOfFailedTestCases = 0;

		areal<nbits, es> p, ref;
		// starting from maxpos iterating to -maxpos, and finally NaR via zero
		for (typename std::vector < areal<nbits, es> >::iterator it = set.end() - 1; it != set.begin(); --it) {
			p = *it;
			p--;
			ref = *(it - 1);
			if (p != ref) {
				if (reportTestCases) std::cout << " FAIL " << p << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

}} // namespace sw::universal

