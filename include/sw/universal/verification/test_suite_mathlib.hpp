#pragma once
// test_suite_mathlib.hpp : mathlib test suite for arbitrary universal number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <cmath> // for sqrt, log, log2, log10, exp, exp2, expm1
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

namespace sw { namespace universal {
	
////////////////////////////////////  MATHLIB FUNCTIONS  //////////////////////////////////////////

	constexpr unsigned TEST_SUITE_MATHLIB_MAX_ERRORS = 25;  // maximum number of errors to report before stopping the test suite

/////////////////////////////////////////////////////////////////////////////////////////
///                            square root operator                                   ///
/////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// verify sqrt function for a number system configuration
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases"></param>
/// <param name="maxSamples">maximum number of test cases to run</param>
/// <returns>number of failed test cases</returns>
template<typename TestType, typename RefType = double>
int VerifySqrt(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; i++) {
		TestType a, result, ref;
		a.setbits(i);
		result = sqrt(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::sqrt(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "sqrt", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "sqrt", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/////////////////////////////////////////////////////////////////////////////////////////
///                             logarithm operators                                   ///
/////////////////////////////////////////////////////////////////////////////////////////

// enumerate all NATURAL LOGARITHM cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyLog(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = log(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::log(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "log", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all BINARY LOGARITHM cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyLog2(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = log2(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::log2(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log2", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "log2", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all DECIMAL LOGARITHM cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyLog10(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = log10(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::log10(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log10", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "log10", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all 1.0/log(p) cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyLog1p(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = log1p(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::log1p(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log1p", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "log1p", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/////////////////////////////////////////////////////////////////////////////////////////
///                           exponential operators                                   ///
/////////////////////////////////////////////////////////////////////////////////////////
 
// enumerate all base-e exponent cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyExp(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = exp(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::exp(da);
		if (result != ref) {
			// filter out inconsistencies among different math library implementations
			RefType dref = std::exp(da);
			if (dref == 0.0) {
				static bool firstRoundingFilterEvent = true;
				if (firstRoundingFilterEvent && reportTestCases) {
					std::cerr << "filtering lns rounding to minpos\n";
					firstRoundingFilterEvent = false;
				}
			}
			else if (result.isnan() && ref.isnan()) {
				static bool firstSofteningNanEvent = true;
				if (firstSofteningNanEvent && reportTestCases) {
					std::cerr << "filtering snan to nan softening\n";
					firstSofteningNanEvent = false;
				}
			}
			else {
				nrOfFailedTests++;
				if (reportTestCases)	ReportOneInputFunctionError("FAIL", "exp", a, result, ref);
			}
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "exp", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all base-2 exponent cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyExp2(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = exp2(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::exp2(da);
		if (result != ref) {
			// filter out inconsistencies among different math library implementations
			RefType dref = std::exp(da);
			if (dref == 0.0) {
				static bool firstRoundingFilterEvent = true;
				if (firstRoundingFilterEvent && reportTestCases) {
					std::cerr << "filtering lns rounding to minpos\n";
					firstRoundingFilterEvent = false;
				}
			}
			else if (result.isnan() && ref.isnan()) {
				static bool firstSofteningNanEvent = true;
				if (firstSofteningNanEvent && reportTestCases) {
					std::cerr << "filtering snan to nan softening\n";
					firstSofteningNanEvent = false;
				}
			}
			else {
				nrOfFailedTests++;
				if (reportTestCases)	ReportOneInputFunctionError("FAIL", "exp2", a, result, ref);
			}
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "exp2", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all exp(x)-1 cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyExpm1(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = expm1(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::expm1(da);
		if (result != ref) {
			// filter out inconsistencies among different math library implementations
			RefType dref = std::exp(da);
			if (dref == 0.0) {
				static bool firstRoundingFilterEvent = true;
				if (firstRoundingFilterEvent && reportTestCases) {
					std::cerr << "filtering lns rounding to minpos\n";
					firstRoundingFilterEvent = false;
				}
			}
			else if (result.isnan() && ref.isnan()) {
				static bool firstSofteningNanEvent = true;
				if (firstSofteningNanEvent && reportTestCases) {
					std::cerr << "filtering snan to nan softening\n";
					firstSofteningNanEvent = false;
				}
			}
			else {
				nrOfFailedTests++;
				if (reportTestCases)	ReportOneInputFunctionError("FAIL", "expm1", a, result, ref);
			}
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "expm1", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/////////////////////////////////////////////////////////////////////////////////////////
///                                power function                                     ///
/////////////////////////////////////////////////////////////////////////////////////////
 
// enumerate all power method cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyPow(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits; 
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	TestType a, b, result{ 0 }, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 0; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		RefType da = RefType(a);
		for (unsigned j = 0; j < NR_TEST_CASES; ++j) {
			b.setbits(j);
			RefType db = RefType(b);
#if THROW_ARITHMETIC_EXCEPTION
			try {
				result = pow(a, b);
			}
			catch (const universal_arithmetic_exception& err) {
				if (a.isnan()) {
					if (reportTestCases) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
				}
				else {
					throw err;
				}
			}
#else
			result = pow(a, b);
#endif
			ref = std::pow(da, db);
			if (result != ref) {
				if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "pow", a, b, result, ref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("PASS", "pow", a, b, result, ref);
			}
			++testNr;
			if (maxSamples > 0 && testNr > maxSamples) {
				std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
				i = j = NR_TEST_CASES;
			}
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}

	return nrOfFailedTests;
}

/////////////////////////////////////////////////////////////////////////////////////////
///                          trigonometry operators                                   ///
/////////////////////////////////////////////////////////////////////////////////////////
 
// enumerate all trigonometric sine cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifySine(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sin(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::sin(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "sin", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "sin", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosine cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyCosine(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = cos(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::cos(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "cos", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "cos", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric tangent cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyTangent(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = tan(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::tan(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "tan", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "tan", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cotangent cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyAtan(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = atan(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::atan(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "atan", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "atan", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric sec cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyAsin(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = asin(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::asin(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "asin", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "asin", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosec cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyAcos(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = acos(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::acos(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "acos", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "acos", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}


/////////////////////////////////////////////////////////////////////////////////////////
///                            hyperbolic operators                                   ///
/////////////////////////////////////////////////////////////////////////////////////////

// enumerate all hyperbolic sine cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifySinh(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sinh(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::sinh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "sinh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "sinh", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosine cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyCosh(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = cosh(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::cosh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "cosh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "cosh", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic tangent cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyTanh(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = tanh(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::tanh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "tanh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "tanh", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cotangent cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyAtanh(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = atanh(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::atanh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "atanh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "atanh", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic sec cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyAsinh(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = asinh(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::asinh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "asinh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "asinh", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << std::endl;
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosec cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyAcosh(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = acosh(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::acosh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases) ReportOneInputFunctionError("FAIL", "acosh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "acosh", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << std::endl;
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/////////////////////////////////////////////////////////////////////////////////////////
///                            hypothenuse operator                                   ///
/////////////////////////////////////////////////////////////////////////////////////////

// enumerate all hypotenuse cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyHypot(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, b, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		RefType da = RefType(a);
		for (unsigned j = 1; j < NR_TEST_CASES; ++j) {
			b.setbits(j);
			result = hypot(a, b);
			// generate reference
			RefType db = RefType(b);
			ref = std::hypot(da, db);
			if (result != ref) {
				//auto prec = std::cout.precision();
				//std::cout << std::setprecision(25) << result << " != " << ref << std::setprecision(prec) << '\n';
				//std::cout << to_binary(result) << '\n' << to_binary(ref) << '\n';
				if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "hypot", a, b, result, ref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("PASS", "hypot", a, b, result, ref);
			}
			++testNr;
			if (maxSamples > 0 && testNr > maxSamples) {
				std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
				i = j = NR_TEST_CASES;
			}
			if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
		}
	}
	return nrOfFailedTests;
}

/////////////////////////////////////////////////////////////////////////////////////////
///                            truncation operators                                   ///
/////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// verify round function for a number system configuration
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases"></param>
/// <param name="maxSamples">maximum number of test cases to run</param>
/// <returns>number of failed test cases</returns>
template<typename TestType, typename RefType = double>
int VerifyRound(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; i++) {
		TestType a, result, ref;
		a.setbits(i);
		result = round(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::round(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "round", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "round", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/// <summary>
/// verify trunc function for a number system configuration
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases"></param>
/// <param name="maxSamples">maximum number of test cases to run</param>
/// <returns>number of failed test cases</returns>
template<typename TestType, typename RefType = double>
int VerifyTrunc(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; i++) {
		TestType a, result, ref;
		a.setbits(i);
		result = trunc(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::trunc(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "trunc", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "trunc", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/// <summary>
/// verify floor function for a number system configuration
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases"></param>
/// <param name="maxSamples">maximum number of test cases to run</param>
/// <returns>number of failed test cases</returns>
template<typename TestType, typename RefType = double>
int VerifyFloor(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; i++) {
		TestType a, result, ref;
		a.setbits(i);
		result = floor(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::floor(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "floor", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "floor", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/// <summary>
/// verify ceil function for a number system configuration
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases"></param>
/// <param name="maxSamples">maximum number of test cases to run</param>
/// <returns>number of failed test cases</returns>
template<typename TestType, typename RefType = double>
int VerifyCeil(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; i++) {
		TestType a, result, ref;
		a.setbits(i);
		result = ceil(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::ceil(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "ceil", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "ceil", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/////////////////////////////////////////////////////////////////////////////////////////
///                            fractional operators                                   ///
/////////////////////////////////////////////////////////////////////////////////////////

/// <summary>
/// verify fmod function for a number system configuration
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases"></param>
/// <param name="maxSamples">maximum number of test cases to run</param>
/// <returns>number of failed test cases</returns>
template<typename TestType, typename RefType = double>
int VerifyFmod(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	TestType a, b, result{ 0 }, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 0; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		RefType da = RefType(a);
		for (unsigned j = 0; j < NR_TEST_CASES; ++j) {
			b.setbits(j);
			RefType db = RefType(b);
#if THROW_ARITHMETIC_EXCEPTION
			try {
				result = fmod(a, b);
			}
			catch (const universal_arithmetic_exception& err) {
				if (a.isnan()) {
					if (reportTestCases) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
				}
				else {
					throw err;
				}
			}
#else
			result = fmod(a, b);
#endif
			ref = std::fmod(da, db);
			if (result != ref) {
				if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "fmod", a, b, result, ref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("PASS", "fmod", a, b, result, ref);
			}
			++testNr;
			if (maxSamples > 0 && testNr > maxSamples) {
				std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
				i = j = NR_TEST_CASES;
			}
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/// <summary>
/// verify remainder function for a number system configuration
/// </summary>
/// <typeparam name="TestType">the number system type to verify</typeparam>
/// <param name="reportTestCases"></param>
/// <param name="maxSamples">maximum number of test cases to run</param>
/// <returns>number of failed test cases</returns>
template<typename TestType, typename RefType = double>
int VerifyRemainder(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr size_t nbits = TestType::nbits;  // number system concept requires a static member indicating its size in bits
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	TestType a, b, result{ 0 }, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 0; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		RefType da = RefType(a);
		for (unsigned j = 0; j < NR_TEST_CASES; ++j) {
			b.setbits(j);
			RefType db = RefType(b);
#if THROW_ARITHMETIC_EXCEPTION
			try {
				result = remainder(a, b);
			}
			catch (const universal_arithmetic_exception& err) {
				if (a.isnan()) {
					if (reportTestCases) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
				}
				else {
					throw err;
				}
			}
#else
			result = remainder(a, b);
#endif
			ref = std::remainder(da, db);
			if (result != ref) {
				if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "remainder", a, b, result, ref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("PASS", "remainder", a, b, result, ref);
			}
			++testNr;
			if (maxSamples > 0 && testNr > maxSamples) {
				std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
				i = j = NR_TEST_CASES;
			}
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/////////////////////////////////////////////////////////////////////////////////////////
///                         error and gamma functions                                 ///
/////////////////////////////////////////////////////////////////////////////////////////

// enumerate all erf cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyErf(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = erf(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::erf(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "erf", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "erf", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all complementary error function cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyErfc(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = erfc(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::erfc(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "erfc", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "erfc", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

// enumerate all gamma function cases for an arbitrary universal type configuration
template<typename TestType, typename RefType = double>
int VerifyTgamma(bool reportTestCases, unsigned int maxSamples = 100) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	unsigned testNr{ 0 };
	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = tgamma(a);
		// generate reference
		RefType da = RefType(a);
		ref = std::tgamma(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) continue; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "tgamma", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "tgamma", a, result, ref);
		}
		++testNr;
		if (maxSamples > 0 && testNr > maxSamples) {
			std::cerr << "nr testcases has been truncated to " << maxSamples << '\n';
			i = NR_TEST_CASES;
		}
		if (nrOfFailedTests > TEST_SUITE_MATHLIB_MAX_ERRORS) return nrOfFailedTests;
	}
	return nrOfFailedTests;
}

/////////////////////////////////////////////////////////////////////////////////////////
///                             nextafter/towards TEST SUITES                         ///
/////////////////////////////////////////////////////////////////////////////////////////

/*
	 double nextafter (double x     , double y );
	  float nextafter (float x      , float y );
long double nextafter (long double x, long double y );
	 double nextafter (Type1 x      , Type2 y);        // additional overloads

	x
Base value.
y
Value toward which the return value is approximated.
If both parameters compare equal, the function returns y.

Return Value
The next representable value after x in the direction of y.

If x is the largest finite value representable in the type, and the result is infinite or not representable, an overflow range error occurs.

If an overflow range error occurs:
- And math_errhandling has MATH_ERRNO set: the global variable errno is set to ERANGE.
- And math_errhandling has MATH_ERREXCEPT set: FE_OVERFLOW is raised.

*/
template<typename TestType, typename RefType = double>
int VerifyNextafter(bool reportTestCases, unsigned int maxSamples = 100) {
	int nrOfFailedTests = 0;

	/*
	 next representable value after x in the direction of y
	 requires four quadrants to test:

	  x =  1.0 ->  2.0  = 1 + ULP
	  x =  1.0 ->  0.5  = 1 - ULP
	  x = -1.0 -> -2.0  = 1 - ULP
	  x = -1.0 -> -0.5  = 1 + ULP

	  plus all the boundary cases where x and/or y can be NaN
	 */

	TestType x{ 1.0f }, xpp{ 1.0f }, xmm{ 1.0f };
	++xpp;  // x + 1ULP
	--xmm;  // x - 1ULP
	TestType y;

	// positive quadrants
	y = nextafter(x, xpp);
	if (y != xpp) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << to_binary(x) << " -> " << to_binary(y) << " ref " << to_binary(xpp) << '\n';
	}
	y = nextafter(x, xmm);
	if (y != xmm) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << to_binary(x) << " -> " << to_binary(y) << " ref " << to_binary(xmm) << '\n';
	}

	// negative quadrants
	y = nextafter(-x, -xpp);
	if (y != -xpp) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << to_binary(-x) << " -> " << to_binary(y) << " ref " << to_binary(-xpp) << '\n';
	}
	y = nextafter(-x, -xmm);
	if (y != -xmm) {
		++nrOfFailedTests;
		if (reportTestCases) std::cout << to_binary(-x) << " -> " << to_binary(y) << " ref " << to_binary(-xmm) << '\n';
	}

	return nrOfFailedTests;
}


/*
*
* C++11
	 double nexttoward  (double x     , long double y);
	  float nexttowardf (float x      , long double y);
long double nexttowardl (long double x, long double y);

x
Base value.
y
Value toward which the return value is approximated.

If both parameters compare equal, the function returns y (converted to the return type).

Return Value
The next representable value after x in the direction of y.

If x is the largest finite value representable in the type, and the result is infinite or not representable, an overflow range error occurs.

If an overflow range error occurs:
- And math_errhandling has MATH_ERRNO set: the global variable errno is set to ERANGE.
- And math_errhandling has MATH_ERREXCEPT set: FE_OVERFLOW is raised.
 */

template<typename TestType, typename RefType = double>
int VerifyNextoward(bool reportTestCases, unsigned int maxSamples = 100) {
	int nrOfFailedTests = 0;

	// TODO: how do you set the target precision in a generic way?
	// targets:
	//   posit<256, 5>
	//   cfloat<128,15>
	// fixpnt? areal? valid?

	return nrOfFailedTests;
}

}} // namespace sw::universal
