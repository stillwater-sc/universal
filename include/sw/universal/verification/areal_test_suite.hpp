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
		/// enumerate all conversion cases for an areal with ubits
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


	/// <summary>
    /// enumerate all negation cases for an areal configuration
    /// </summary>
    /// <param name="reportTestCases"></param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifyNegation(bool reportTestCases) {
	    constexpr size_t nbits           = TestType::nbits;
	    constexpr size_t NR_TEST_CASES   = (size_t(1) << nbits);
	    int              nrOfFailedTests = 0;
	    TestType         a(0), negated(0), ref(0);

	    for (size_t i = 1; i < NR_TEST_CASES; i++) {
		    a.setbits(i);
		    negated = -a;
		    // generate reference
		    double da = double(a);
		    ref       = -da;
		    if (negated != ref) {
			    nrOfFailedTests++;
			    if (reportTestCases)
				    ReportUnaryArithmeticError("FAIL", "-", a, negated, ref);
		    } else {
			    // if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "-", a, negated, ref);
		    }
	    }
	    return nrOfFailedTests;
    }

    /// <summary>
    /// Verify ubit propagation for addition: result.ubit = a.ubit || b.ubit || precision_lost
    ///
    /// Tests four cases:
    /// 1. exact + exact (ubit=0 + ubit=0) → result.ubit depends on precision loss
    /// 2. exact + interval (ubit=0 + ubit=1) → result.ubit must be 1
    /// 3. interval + exact (ubit=1 + ubit=0) → result.ubit must be 1
    /// 4. interval + interval (ubit=1 + ubit=1) → result.ubit must be 1
    /// </summary>
    template<typename TestType>
    int VerifyUbitPropagationAdd(bool reportTestCases) {
	    constexpr size_t nbits     = TestType::nbits;
	    constexpr size_t NR_VALUES = (size_t(1) << nbits);
	    int nrOfFailedTests = 0;

	    TestType a, b, c;

	    // Helper lambda to check if a value has ubit set
	    auto hasUbit = [](const TestType& v) -> bool {
		    return (v.block(0) & 1) != 0;
	    };

	    // Test Case 1: exact + exact - verify ubit reflects precision loss
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf()) continue;

			    c = a + b;
			    if (c.isnan() || c.isinf()) continue;

			    // Compute reference - assignment sets ubit correctly
			    double ref = double(a) + double(b);
			    TestType cref = ref;

			    // The ubit of c should match cref
			    if (hasUbit(c) != hasUbit(cref)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit mismatch (exact+exact): "
					              << to_binary(a) << " + " << to_binary(b)
					              << " = " << to_binary(c)
					              << " expected ubit=" << hasUbit(cref)
					              << " got ubit=" << hasUbit(c) << std::endl;
				    }
			    }
		    }
	    }

	    // Test Case 2 & 3: exact + interval and interval + exact → ubit must be 1
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 1; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf()) continue;

			    c = a + b;
			    if (!c.isnan() && !c.isinf() && !hasUbit(c)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit not set (exact+interval): "
					              << to_binary(a) << " + " << to_binary(b)
					              << " = " << to_binary(c) << std::endl;
				    }
			    }

			    c = b + a;
			    if (!c.isnan() && !c.isinf() && !hasUbit(c)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit not set (interval+exact): "
					              << to_binary(b) << " + " << to_binary(a)
					              << " = " << to_binary(c) << std::endl;
				    }
			    }
		    }
	    }

	    // Test Case 4: interval + interval → ubit must be 1
	    for (size_t i = 1; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 1; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf()) continue;

			    c = a + b;
			    if (!c.isnan() && !c.isinf() && !hasUbit(c)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit not set (interval+interval): "
					              << to_binary(a) << " + " << to_binary(b)
					              << " = " << to_binary(c) << std::endl;
				    }
			    }
		    }
	    }

	    return nrOfFailedTests;
    }

    /// <summary>
    /// Verify ubit propagation for multiplication
    /// </summary>
    template<typename TestType>
    int VerifyUbitPropagationMul(bool reportTestCases) {
	    constexpr size_t nbits     = TestType::nbits;
	    constexpr size_t NR_VALUES = (size_t(1) << nbits);
	    int nrOfFailedTests = 0;

	    TestType a, b, c;

	    auto hasUbit = [](const TestType& v) -> bool {
		    return (v.block(0) & 1) != 0;
	    };

	    // Test exact * exact - verify ubit reflects precision loss
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf()) continue;

			    c = a * b;
			    if (c.isnan() || c.isinf()) continue;

			    double ref = double(a) * double(b);
			    TestType cref = ref;

			    if (hasUbit(c) != hasUbit(cref)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit mismatch (exact*exact): "
					              << to_binary(a) << " * " << to_binary(b)
					              << " = " << to_binary(c)
					              << " expected ubit=" << hasUbit(cref)
					              << " got ubit=" << hasUbit(c) << std::endl;
				    }
			    }
		    }
	    }

	    // Test interval inputs - result must have ubit=1 (except for zero)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 1; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf()) continue;

			    c = a * b;
			    if (!c.isnan() && !c.isinf() && !c.iszero() && !hasUbit(c)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit not set (exact*interval): "
					              << to_binary(a) << " * " << to_binary(b)
					              << " = " << to_binary(c) << std::endl;
				    }
			    }
		    }
	    }

	    return nrOfFailedTests;
    }

    /// <summary>
    /// Verify ubit propagation for subtraction
    /// </summary>
    template<typename TestType>
    int VerifyUbitPropagationSub(bool reportTestCases) {
	    constexpr size_t nbits     = TestType::nbits;
	    constexpr size_t NR_VALUES = (size_t(1) << nbits);
	    int nrOfFailedTests = 0;

	    TestType a, b, c;

	    auto hasUbit = [](const TestType& v) -> bool {
		    return (v.block(0) & 1) != 0;
	    };

	    // Test exact - exact - verify ubit reflects precision loss
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf()) continue;

			    c = a - b;
			    if (c.isnan() || c.isinf()) continue;

			    double ref = double(a) - double(b);
			    TestType cref = ref;

			    if (hasUbit(c) != hasUbit(cref)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit mismatch (exact-exact): "
					              << to_binary(a) << " - " << to_binary(b)
					              << " = " << to_binary(c)
					              << " expected ubit=" << hasUbit(cref)
					              << " got ubit=" << hasUbit(c) << std::endl;
				    }
			    }
		    }
	    }

	    // Test interval inputs - result must have ubit=1
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 1; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf()) continue;

			    c = a - b;
			    if (!c.isnan() && !c.isinf() && !hasUbit(c)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit not set (exact-interval): "
					              << to_binary(a) << " - " << to_binary(b)
					              << " = " << to_binary(c) << std::endl;
				    }
			    }

			    c = b - a;
			    if (!c.isnan() && !c.isinf() && !hasUbit(c)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit not set (interval-exact): "
					              << to_binary(b) << " - " << to_binary(a)
					              << " = " << to_binary(c) << std::endl;
				    }
			    }
		    }
	    }

	    // Test interval - interval
	    for (size_t i = 1; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 1; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf()) continue;

			    c = a - b;
			    if (!c.isnan() && !c.isinf() && !hasUbit(c)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit not set (interval-interval): "
					              << to_binary(a) << " - " << to_binary(b)
					              << " = " << to_binary(c) << std::endl;
				    }
			    }
		    }
	    }

	    return nrOfFailedTests;
    }

    /// <summary>
    /// Verify ubit propagation for division
    /// </summary>
    template<typename TestType>
    int VerifyUbitPropagationDiv(bool reportTestCases) {
	    constexpr size_t nbits     = TestType::nbits;
	    constexpr size_t NR_VALUES = (size_t(1) << nbits);
	    int nrOfFailedTests = 0;

	    TestType a, b, c;

	    auto hasUbit = [](const TestType& v) -> bool {
		    return (v.block(0) & 1) != 0;
	    };

	    // Test exact / exact - verify ubit reflects precision loss
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf() || b.iszero()) continue;

			    c = a / b;
			    if (c.isnan() || c.isinf()) continue;

			    double ref = double(a) / double(b);
			    TestType cref = ref;

			    if (hasUbit(c) != hasUbit(cref)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit mismatch (exact/exact): "
					              << to_binary(a) << " / " << to_binary(b)
					              << " = " << to_binary(c)
					              << " expected ubit=" << hasUbit(cref)
					              << " got ubit=" << hasUbit(c) << std::endl;
				    }
			    }
		    }
	    }

	    // Test interval inputs - result must have ubit=1 (except for zero result)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 1; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf() || b.iszero()) continue;

			    c = a / b;
			    if (!c.isnan() && !c.isinf() && !c.iszero() && !hasUbit(c)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit not set (exact/interval): "
					              << to_binary(a) << " / " << to_binary(b)
					              << " = " << to_binary(c) << std::endl;
				    }
			    }
		    }
	    }

	    // Test interval / exact
	    for (size_t i = 1; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    if (a.isnan() || a.isinf()) continue;
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isnan() || b.isinf() || b.iszero()) continue;

			    c = a / b;
			    if (!c.isnan() && !c.isinf() && !c.iszero() && !hasUbit(c)) {
				    nrOfFailedTests++;
				    if (reportTestCases) {
					    std::cerr << "FAIL ubit not set (interval/exact): "
					              << to_binary(a) << " / " << to_binary(b)
					              << " = " << to_binary(c) << std::endl;
				    }
			    }
		    }
	    }

	    return nrOfFailedTests;
    }

    /// <summary>
    /// Enumerate all addition cases for an areal configuration.
    /// Uses doubles to create a reference to compare to.
    ///
    /// For areal, we only test exact values (ubit=0) as inputs because:
    /// - Values with ubit=1 represent open intervals (v, next(v)), not points
    /// - Intervals cannot be meaningfully compared against a double reference
    /// - The ubit propagation rule is: result.ubit = a.ubit || b.ubit || precision_lost
    /// - When both inputs are exact, the result's ubit correctly indicates if precision was lost
    /// </summary>
    /// <typeparam name="TestType">the number system type to verify</typeparam>
    /// <param name="reportTestCases">if yes, report on individual test failures</param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifyAddition(bool reportTestCases) {
	    constexpr size_t nbits =
	        TestType::nbits;  // number system concept requires a static member indicating its size in bits
	    constexpr size_t NR_VALUES       = (size_t(1) << nbits);
	    constexpr size_t NR_EXACT_VALUES = NR_VALUES / 2;  // only exact values (ubit=0)
	    int              nrOfFailedTests = 0;

	    double   da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	    TestType a, b, c, cref;

	    // Only iterate over exact values (even bit patterns, i.e., ubit=0)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);  // number system concept requires a member function setbits()
		    da = double(a);
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    db  = double(b);
			    ref = da + db;
#if THROW_ARITHMETIC_EXCEPTION
			    // catching overflow
			    try {
				    c = a + b;
			    } catch (...) {
                    // set the saturation clamps
                    TestType maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
				    if (ref < double(maxneg) || ref > double(maxpos)) {
					    // correctly caught the overflow exception
					    continue;
				    } else {
					    nrOfFailedTests++;
				    }
			    }
#else
			    c = a + b;
#endif  // THROW_ARITHMETIC_EXCEPTION
			    cref = ref;
			    if (c != cref) {
				    if (ref == 0 and c.iszero())
					    continue;  // mismatched is ignored as compiler optimizes away negative zero
				    if (c.isnan() && cref.isnan())
					    continue;  // both NaN is acceptable (NaN representation may vary)
				    nrOfFailedTests++;
				    if (reportTestCases)
					    ReportBinaryArithmeticError("FAIL", "+", a, b, c, ref);
			    } else {
				    // if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, c, ref);
			    }
		    }
		    if constexpr (NR_EXACT_VALUES > 256 * 256) {
			    if ((i/2) % (NR_EXACT_VALUES / 25) == 0)
				    std::cout << '.';
		    }
	    }
	    std::cout << std::endl;
	    return nrOfFailedTests;
    }

    /// <summary>
    /// Enumerate all in-place (+=) addition cases for an areal configuration.
    /// Uses doubles to create a reference to compare to.
    ///
    /// For areal, we only test exact values (ubit=0) as inputs.
    /// </summary>
    /// <typeparam name="TestType">the number system type to verify</typeparam>
    /// <param name="reportTestCases">if yes, report on individual test failures</param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifyInPlaceAddition(bool reportTestCases) {
	    constexpr size_t nbits =
	        TestType::nbits;  // number system concept requires a static member indicating its size in bits
	    constexpr size_t NR_VALUES       = (size_t(1) << nbits);
	    constexpr size_t NR_EXACT_VALUES = NR_VALUES / 2;  // only exact values (ubit=0)
	    int              nrOfFailedTests = 0;

	    double   da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	    TestType a, b, c, cref;

	    // Only iterate over exact values (even bit patterns, i.e., ubit=0)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);  // number system concept requires a member function setbits()
		    da = double(a);
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    db  = double(b);
			    ref = da + db;
#if THROW_ARITHMETIC_EXCEPTION
			    // catching overflow
			    try {
				    c = a;
				    c += b;
			    } catch (...) {
                    // set the saturation clamps
                    TestType maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
				    if (ref < double(maxneg) || ref > double(maxpos)) {
					    // correctly caught the overflow exception
					    continue;
				    } else {
					    nrOfFailedTests++;
				    }
			    }
#else
			    c = a;
			    c += b;
#endif  // THROW_ARITHMETIC_EXCEPTION
			    cref = ref;
			    if (c != cref) {
				    if (ref == 0 and c.iszero())
					    continue;  // mismatched is ignored as compiler optimizes away negative zero
				    if (c.isnan() && cref.isnan())
					    continue;  // both NaN is acceptable (NaN representation may vary)
				    nrOfFailedTests++;
				    if (reportTestCases)
					    ReportBinaryArithmeticError("FAIL", "+=", a, b, c, ref);
			    } else {
				    // if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+=", a, b, c, ref);
			    }
		    }
		    if constexpr (NR_EXACT_VALUES > 256 * 256) {
			    if ((i/2) % (NR_EXACT_VALUES / 25) == 0)
				    std::cout << '.';
		    }
	    }
	    std::cout << std::endl;
	    return nrOfFailedTests;
    }

    /// <summary>
    /// Enumerate all subtraction cases for an areal configuration.
    /// Uses doubles to create a reference to compare to.
    ///
    /// For areal, we only test exact values (ubit=0) as inputs.
    /// </summary>
    /// <typeparam name="TestType">the number system type to verify</typeparam>
    /// <param name="reportTestCases">if yes, report on individual test failures</param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifySubtraction(bool reportTestCases) {
	    constexpr size_t nbits =
	        TestType::nbits;  // number system concept requires a static member indicating its size in bits
	    constexpr size_t NR_VALUES       = (size_t(1) << nbits);
	    constexpr size_t NR_EXACT_VALUES = NR_VALUES / 2;  // only exact values (ubit=0)
	    int              nrOfFailedTests = 0;

	    double   da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	    TestType a, b, c, cref;

	    // Only iterate over exact values (even bit patterns, i.e., ubit=0)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);  // number system concept requires a member function setbits()
		    da = double(a);
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    db  = double(b);
			    ref = da - db;
#if THROW_ARITHMETIC_EXCEPTION
			    // catching overflow
			    try {
				    c = a - b;
			    } catch (...) {
                    // set the saturation clamps
                    TestType maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
				    if (ref < double(maxneg) || ref > double(maxpos)) {
					    // correctly caught the overflow exception
					    continue;
				    } else {
					    nrOfFailedTests++;
				    }
			    }
#else
			    c = a - b;
#endif  // THROW_ARITHMETIC_EXCEPTION
			    cref = ref;
			    if (c != cref) {
				    if (ref == 0 and c.iszero())
					    continue;  // mismatched is ignored as compiler optimizes away negative zero
				    if (c.isnan() && cref.isnan())
					    continue;  // both NaN is acceptable (NaN representation may vary)
				    nrOfFailedTests++;
				    if (reportTestCases)
					    ReportBinaryArithmeticError("FAIL", "-", a, b, c, ref);
			    } else {
				    // if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, c, ref);
			    }
			    if (nrOfFailedTests > 24)
				    return nrOfFailedTests;
		    }
		    if constexpr (NR_EXACT_VALUES > 256 * 256) {
			    if ((i/2) % (NR_EXACT_VALUES / 25) == 0)
				    std::cout << '.';
		    }
	    }
	    std::cout << std::endl;
	    return nrOfFailedTests;
    }

    /// <summary>
    /// Enumerate all in-place (-=) subtraction cases for an areal configuration.
    /// Uses doubles to create a reference to compare to.
    ///
    /// For areal, we only test exact values (ubit=0) as inputs.
    /// </summary>
    /// <typeparam name="TestType">the number system type to verify</typeparam>
    /// <param name="reportTestCases">if yes, report on individual test failures</param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifyInPlaceSubtraction(bool reportTestCases) {
	    constexpr size_t nbits =
	        TestType::nbits;  // number system concept requires a static member indicating its size in bits
	    constexpr size_t NR_VALUES       = (size_t(1) << nbits);
	    constexpr size_t NR_EXACT_VALUES = NR_VALUES / 2;  // only exact values (ubit=0)
	    int              nrOfFailedTests = 0;

	    double   da, db, ref;  // make certain that IEEE doubles are sufficient as reference
	    TestType a, b, c, cref;

	    // Only iterate over exact values (even bit patterns, i.e., ubit=0)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);  // number system concept requires a member function setbits()
		    da = double(a);
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    db  = double(b);
			    ref = da - db;
#if THROW_ARITHMETIC_EXCEPTION
			    // catching overflow
			    try {
				    c = a;
				    c -= b;
			    } catch (...) {
                    // set the saturation clamps
                    TestType maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
				    if (ref < double(maxneg) || ref > double(maxpos)) {
					    // correctly caught the overflow exception
					    continue;
				    } else {
					    nrOfFailedTests++;
				    }
			    }
#else
			    c = a;
			    c -= b;
#endif  // THROW_ARITHMETIC_EXCEPTION
			    cref = ref;
			    if (c != cref) {
				    if (ref == 0 and c.iszero())
					    continue;  // mismatched is ignored as compiler optimizes away negative zero
				    if (c.isnan() && cref.isnan())
					    continue;  // both NaN is acceptable (NaN representation may vary)
				    nrOfFailedTests++;
				    if (reportTestCases)
					    ReportBinaryArithmeticError("FAIL", "-=", a, b, c, ref);
			    } else {
				    // if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-=", a, b, c, ref);
			    }
			    if (nrOfFailedTests > 24)
				    return nrOfFailedTests;
		    }
		    if constexpr (NR_EXACT_VALUES > 256 * 256) {
			    if ((i/2) % (NR_EXACT_VALUES / 25) == 0)
				    std::cout << '.';
		    }
	    }
	    std::cout << std::endl;
	    return nrOfFailedTests;
    }

    /// <summary>
    /// Enumerate all multiplication cases for an areal configuration.
    /// Uses doubles to create a reference to compare to.
    ///
    /// For areal, we only test exact values (ubit=0) as inputs.
    /// </summary>
    /// <typeparam name="TestType">the number system type to verify</typeparam>
    /// <param name="reportTestCases">if yes, report on individual test failures</param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifyMultiplication(bool reportTestCases) {
	    constexpr size_t nbits =
	        TestType::nbits;  // number system concept requires a static member indicating its size in bits
	    constexpr size_t NR_VALUES       = (size_t(1) << nbits);
	    constexpr size_t NR_EXACT_VALUES = NR_VALUES / 2;  // only exact values (ubit=0)
	    int              nrOfFailedTests = 0;

	    TestType a, b, c, cref;

	    // Only iterate over exact values (even bit patterns, i.e., ubit=0)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    double da = double(a);
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    double db  = double(b);
			    double ref = da * db;  // make certain that IEEE doubles are sufficient as reference
#if THROW_ARITHMETIC_EXCEPTION
			    try {
				    c = a * b;
			    } catch (...) {
				    if (a.isnan() || b.isnan()) {
					    // correctly caught the exception
					    c.setnan(true);  // TODO: unify quiet vs signalling propagation among real number systems
					    // posits behave differently than floats, so this may need a least common denominator approach
				    } else {
					    throw;  // rethrow
				    }
			    }
#else
			    c = a * b;
#endif
			    cref = ref;
			    if (c != cref) {
				    if (ref == 0.0 && c.iszero())
					    continue;  // signed zero mismatch
				    if (c.isnan() && cref.isnan())
					    continue;  // both NaN is acceptable (NaN representation may vary)
				    if (reportTestCases)
					    ReportBinaryArithmeticError("FAIL", "*", a, b, c, ref);
				    nrOfFailedTests++;
			    } else {
				    // if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, c, ref);
			    }
		    }
		    if constexpr (NR_EXACT_VALUES > 256 * 256) {
			    if ((i/2) % (NR_EXACT_VALUES / 25) == 0)
				    std::cout << '.';
		    }
	    }
	    std::cout << std::endl;
	    return nrOfFailedTests;
    }

    /// <summary>
    /// Enumerate all in-place (*=) multiplication cases for an areal configuration.
    /// Uses doubles to create a reference to compare to.
    ///
    /// For areal, we only test exact values (ubit=0) as inputs.
    /// </summary>
    /// <typeparam name="TestType">the number system type to verify</typeparam>
    /// <param name="reportTestCases">if yes, report on individual test failures</param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifyInPlaceMultiplication(bool reportTestCases) {
	    constexpr size_t nbits =
	        TestType::nbits;  // number system concept requires a static member indicating its size in bits
	    constexpr size_t NR_VALUES       = (size_t(1) << nbits);
	    constexpr size_t NR_EXACT_VALUES = NR_VALUES / 2;  // only exact values (ubit=0)
	    int              nrOfFailedTests = 0;

	    TestType a, b, c, cref;

	    // Only iterate over exact values (even bit patterns, i.e., ubit=0)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    double da = double(a);
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    double db  = double(b);
			    double ref = da * db;  // make certain that IEEE doubles are sufficient as reference
#if THROW_ARITHMETIC_EXCEPTION
			    try {
				    c = a;
				    c *= b;
			    } catch (...) {
				    if (a.isnan() || b.isnan()) {
					    // correctly caught the exception
					    c.setnan(true);  // TODO: unify quiet vs signalling propagation among real number systems
					    // posits behave differently than floats, so this may need a least common denominator approach
				    } else {
					    throw;  // rethrow
				    }
			    }
#else
			    c = a;
			    c *= b;
#endif
			    cref = ref;
			    if (c != cref) {
				    if (ref == 0.0 && c.iszero())
					    continue;  // signed zero mismatch
				    if (c.isnan() && cref.isnan())
					    continue;  // both NaN is acceptable (NaN representation may vary)
				    if (reportTestCases)
					    ReportBinaryArithmeticError("FAIL", "*=", a, b, c, ref);
				    nrOfFailedTests++;
			    } else {
				    // if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*=", a, b, c, ref);
			    }
		    }
		    if constexpr (NR_EXACT_VALUES > 256 * 256) {
			    if ((i/2) % (NR_EXACT_VALUES / 25) == 0)
				    std::cout << '.';
		    }
	    }
	    std::cout << std::endl;
	    return nrOfFailedTests;
    }

    /// <summary>
    /// Enumerate all division cases for an areal configuration.
    /// Uses doubles to create a reference to compare to.
    ///
    /// For areal, we only test exact values (ubit=0) as inputs.
    /// Note: Division by infinity is skipped because areal returns 0 with ubit=1 (uncertain)
    /// while IEEE returns exactly 0. This is a semantic difference, not a bug.
    /// </summary>
    /// <typeparam name="TestType">the number system type to verify</typeparam>
    /// <param name="reportTestCases">if yes, report on individual test failures</param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifyDivision(bool reportTestCases) {
	    constexpr size_t nbits =
	        TestType::nbits;  // number system concept requires a static member indicating its size in bits
	    constexpr size_t NR_VALUES       = (size_t(1) << nbits);
	    constexpr size_t NR_EXACT_VALUES = NR_VALUES / 2;  // only exact values (ubit=0)
	    int              nrOfFailedTests = 0;

	    TestType a, b, c, cref;

	    // Only iterate over exact values (even bit patterns, i.e., ubit=0)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    double da = double(a);
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isinf()) continue;  // skip inf divisor (areal semantics differ from IEEE)
			    double db = double(b);
			    double ref{0};  // make certain that IEEE doubles are sufficient as reference
#if THROW_ARITHMETIC_EXCEPTION
			    try {
				    c   = a / b;
				    ref = da / db;
			    } catch (...) {
				    if (b.iszero()) {
					    // correctly caught the exception
					    c.setnan(true);  // TODO: unify quiet vs signalling propagation among real number systems
					    // posits behave differently than floats, so this may need a least common denominator approach
				    } else if (a.isnan() || b.isnan()) {
					    // Universal will throw a divide_by_nar or numerator_is_nar exception for posits
					    c.setnan(true);  // TODO: unify quiet vs signalling propagation among real number systems
				    } else {
					    throw;  // rethrow
				    }
			    }
#else
			    c   = a / b;
			    ref = da / db;
#endif
			    cref = ref;
			    if (c != cref) {
				    if (ref == 0.0 && c.iszero())
					    continue;  // signed zero mismatch
				    if (c.isnan() && cref.isnan())
					    continue;  // both NaN is acceptable (NaN representation may vary)
				    if (reportTestCases)
					    ReportBinaryArithmeticError("FAIL", "/", a, b, c, ref);
				    nrOfFailedTests++;
			    } else {
				    // if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", a, b, c, ref);
			    }
		    }
		    if constexpr (NR_EXACT_VALUES > 256 * 256) {
			    if ((i/2) % (NR_EXACT_VALUES / 25) == 0)
				    std::cout << '.';
		    }
	    }
	    std::cout << std::endl;
	    return nrOfFailedTests;
    }

    /// <summary>
    /// Enumerate all in-place (/=) division cases for an areal configuration.
    /// Uses doubles to create a reference to compare to.
    ///
    /// For areal, we only test exact values (ubit=0) as inputs.
    /// Note: Division by infinity is skipped because areal returns 0 with ubit=1 (uncertain)
    /// while IEEE returns exactly 0. This is a semantic difference, not a bug.
    /// </summary>
    /// <typeparam name="TestType">the number system type to verify</typeparam>
    /// <param name="reportTestCases">if yes, report on individual test failures</param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifyInPlaceDivision(bool reportTestCases) {
	    constexpr size_t nbits =
	        TestType::nbits;  // number system concept requires a static member indicating its size in bits
	    constexpr size_t NR_VALUES       = (size_t(1) << nbits);
	    constexpr size_t NR_EXACT_VALUES = NR_VALUES / 2;  // only exact values (ubit=0)
	    int              nrOfFailedTests = 0;

	    TestType a, b, c, cref;

	    // Only iterate over exact values (even bit patterns, i.e., ubit=0)
	    for (size_t i = 0; i < NR_VALUES; i += 2) {
		    a.setbits(i);
		    double da = double(a);
		    for (size_t j = 0; j < NR_VALUES; j += 2) {
			    b.setbits(j);
			    if (b.isinf()) continue;  // skip inf divisor (areal semantics differ from IEEE)
			    double db = double(b);
			    double ref{0};  // make certain that IEEE doubles are sufficient as reference
#if THROW_ARITHMETIC_EXCEPTION
			    try {
				    c = a;
				    c /= b;
				    ref = da / db;
			    } catch (...) {
				    if (b.iszero()) {
					    // correctly caught the exception
					    c.setnan(true);  // TODO: unify quiet vs signalling propagation among real number systems
					    // posits behave differently than floats, so this may need a least common denominator approach
				    }
				    if (a.isnan() || b.isnan()) {
					    // Universal will throw a divide_by_nar or numerator_is_nar exception for posits
					    c.setnan(true);  // TODO: unify quiet vs signalling propagation among real number systems
				    } else {
					    throw;  // rethrow
				    }
			    }
#else
			    c = a;
			    c /= b;
			    ref = da / db;
#endif
			    cref = ref;
			    if (c != cref) {
				    if (ref == 0.0 && c.iszero())
					    continue;  // signed zero mismatch
				    if (c.isnan() && cref.isnan())
					    continue;  // both NaN is acceptable (NaN representation may vary)
				    if (reportTestCases)
					    ReportBinaryArithmeticError("FAIL", "/=", a, b, c, ref);
				    nrOfFailedTests++;
			    } else {
				    // if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/=", a, b, c, ref);
			    }
		    }
		    if constexpr (NR_EXACT_VALUES > 256 * 256) {
			    if ((i/2) % (NR_EXACT_VALUES / 25) == 0)
				    std::cout << '.';
		    }
	    }
	    std::cout << std::endl;
	    return nrOfFailedTests;
    }

    /// <summary>
    /// Enumerate all reciprocation cases for an areal configuration.
    /// Uses doubles to create a reference to compare to.
    /// </summary>
    /// <typeparam name="TestType">the number system type to verify</typeparam>
    /// <param name="reportTestCases">if yes, report on individual test failures</param>
    /// <returns>number of failed test cases</returns>
    template<typename TestType>
    int VerifyReciprocation(bool reportTestCases) {
	    constexpr size_t nbits =
	        TestType::nbits;  // number system concept requires a static member indicating its size in bits
	    const unsigned NR_TEST_CASES   = (unsigned(1) << nbits);
	    int            nrOfFailedTests = 0;
	    for (unsigned i = 0; i < NR_TEST_CASES; i++) {
		    TestType a, reciprocal, ref;
		    a.setbits(i);
		    double da = double(a);
#if THROW_ARITHMETIC_EXCEPTION
		    try {
			    reciprocal = a.reciprocal();
			    ref        = 1.0 / da;
		    } catch (...) {
			    if (a.iszero()) {
				    // correctly caught divide by zero exception
			    }
		    }
#else
		    reciprocal = a.reciprocate();
		    ref        = 1.0 / da;
#endif

		    if (reciprocal != ref) {
			    nrOfFailedTests++;
			    if (reportTestCases)
				    ReportUnaryArithmeticError("FAIL", "reciprocate", a, reciprocal, ref);
		    } else {
			    // if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "reciprocate", a, reciprocal, ref);
		    }
	    }
	    return nrOfFailedTests;
    }
    }} // namespace sw::universal

