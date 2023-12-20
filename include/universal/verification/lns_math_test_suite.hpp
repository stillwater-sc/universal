#pragma once
// lns_math_test_suite.hpp : test suite runners for math library functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is art of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

// mathematical function definitions and implementations
#include <universal/number/lns/mathlib.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

/////////////////////////////// VALIDATION TEST SUITES ////////////////////////////////

////////////////////////////////////  MATHEMATICAL FUNCTIONS  //////////////////////////////////////////

// enumerate all NATURAL LOGARITHM cases for an lns configuration
template<typename TestType>
int VerifyLog(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::log(a);
		// generate reference
		double da = double(a);
		ref = std::log(da);
		if (result != ref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "log", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all BINARY LOGARITHM cases for an lns configuration
template<typename TestType>
int VerifyLog2(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::log2(a);
		// generate reference
		double da = double(a);
		ref = std::log2(da);
		if (result != ref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log2", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "log2", a, result, ref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all DECIMAL LOGARITHM cases for an lns configuration
template<typename TestType>
int VerifyLog10(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::log10(a);
		// generate reference
		double da = double(a);
		ref = std::log10(da);
		if (result != ref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log10", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "log10", a, result, ref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all base-e exponent cases for an lns configuration
template<typename TestType>
int VerifyExp(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::exp(a);
		// generate reference
		double da = double(a);
		double dref = std::exp(da);
		ref = dref;
		if (result != ref) {
			// filter out inconsistencies among different math library implementations
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
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "exp", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all base-2 exponent cases for an lns configuration
template<typename TestType>
int VerifyExp2(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::exp2(a);
		// generate reference
		double da = double(a);
		double dref = std::exp2(da);
		ref = dref;
		if (result != ref) {
			// filter out inconsistencies among different math library implementations
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
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "exp2", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all power method cases for an lns configuration
template<typename TestType>
int VerifyPowerFunction(bool reportTestCases, unsigned int maxSamples = 10000) {
	constexpr unsigned nbits = TestType::nbits; 
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	TestType a, b, result, ref;

	uint32_t testNr = 0;
	for (unsigned i = 0; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		double da = double(a);
		for (unsigned j = 0; j < NR_TEST_CASES; ++j) {
			b.setbits(j);
			double db = double(b);
#if LNS_THROW_ARITHMETIC_EXCEPTION
			try {
				result = pow(a, b);
			}
			catch (const lns_arithmetic_exception& err) {
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
				if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "pow", a, b, result, ref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("aSS", "pow", a, b, result, ref);
			}
			++testNr;
			if (testNr > maxSamples) {
				std::cerr << "VerifyPower has been truncated\n";
				i = j = NR_TEST_CASES;
			}
		}
	}

	return nrOfFailedTests;
}

// enumerate all trigonometric sine cases for an lns configuration
template<typename TestType>
int VerifySine(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::sin(a);
		// generate reference
		double da = double(a);
		ref = std::sin(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "sin", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "sin", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosine cases for an lns configuration
template<typename TestType>
int VerifyCosine(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::cos(a);
		// generate reference
		double da = double(a);
		ref = std::cos(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "cos", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "cos", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric tangent cases for an lns configuration
template<typename TestType>
int VerifyTangent(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::tan(a);
		// generate reference
		double da = double(a);
		ref = std::tan(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "tan", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "tan", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cotangent cases for an lns configuration
template<typename TestType>
int VerifyAtan(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::atan(a);
		// generate reference
		double da = double(a);
		ref = std::atan(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "atan", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "atan", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric sec cases for an lns configuration
template<typename TestType>
int VerifyAsin(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::asin(a);
		// generate reference
		double da = double(a);
		ref = std::asin(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "asin", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "asin", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosec cases for an lns configuration
template<typename TestType>
int VerifyAcos(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::acos(a);
		// generate reference
		double da = double(a);
		ref = std::acos(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "acos", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "acos", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic sine cases for an lns configuration
template<typename TestType>
int VerifySinh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::sinh(a);
		// generate reference
		double da = double(a);
		ref = std::sinh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "sinh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "sinh", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosine cases for an lns configuration
template<typename TestType>
int VerifyCosh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::cosh(a);
		// generate reference
		double da = double(a);
		ref = std::cosh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "cosh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "cosh", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic tangent cases for an lns configuration
template<typename TestType>
int VerifyTanh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::tanh(a);
		// generate reference
		double da = double(a);
		ref = std::tanh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "tanh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "tanh", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cotangent cases for an lns configuration
template<typename TestType>
int VerifyAtanh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::atanh(a);
		// generate reference
		double da = double(a);
		ref = std::atanh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "atanh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "atanh", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic sec cases for an lns configuration
template<typename TestType>
int VerifyAsinh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::asinh(a);
		// generate reference
		double da = double(a);
		ref = std::asinh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "asinh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "asinh", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosec cases for an lns configuration
template<typename TestType>
int VerifyAcosh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		result = sw::universal::acosh(a);
		// generate reference
		double da = double(a);
		ref = std::acosh(da);
		if (result != ref) {
			if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "acosh", a, result, ref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "acosh", a, result, ref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hypotenuse cases for an lns configuration
template<typename TestType>
int VerifyHypot(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr unsigned NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, b, result, ref;

	for (unsigned i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		double da = double(a);
		for (unsigned j = 1; j < NR_TEST_CASES; ++j) {
			b.setbits(j);
			result = sw::universal::hypot(a, b);
			// generate reference
			double db = double(b);
			ref = std::hypot(da, db);
			if (result != ref) {
				if (result.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "hypot", a, b, result, ref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("PASS", "hypot", a, b, result, ref);
			}
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal

