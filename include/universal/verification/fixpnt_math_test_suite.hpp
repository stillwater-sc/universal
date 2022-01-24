#pragma once
//  fixpnt_math_test_suite.hpp : test suite runners for math library functions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is art of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

// mathematical function definitions and implementations
#include <universal/number/fixpnt/mathlib.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

/////////////////////////////// VALIDATION TEST SUITES ////////////////////////////////

////////////////////////////////////  MATHEMATICAL FUNCTIONS  //////////////////////////////////////////

// enumerate all NATURAL LOGARITHM cases for a fixpnt configuration
template<typename TestType>
int VerifyLog(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, alog, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		alog = sw::universal::log(a);
		// generate reference
		double da = double(a);
		aref = std::log(da);
		if (alog != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "log", a, aref, alog);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "log", a, aref, alog);
		}
	}
	return nrOfFailedTests;
}

// enumerate all BINARY LOGARITHM cases for a fixpnt configuration
template<typename TestType>
int VerifyLog2(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, plog2, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		plog2 = sw::universal::log2(a);
		// generate reference
		double da = double(a);
		aref = std::log2(da);
		if (plog2 != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "log2", a, aref, plog2);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "log2", a, aref, plog2);
		}
	}
	return nrOfFailedTests;
}


// enumerate all DECIMAL LOGARITHM cases for a fixpnt configuration
template<typename TestType>
int VerifyLog10(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, plog10, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		plog10 = sw::universal::log10(a);
		// generate reference
		double da = double(a);
		aref = std::log10(da);
		if (plog10 != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "log10", a, aref, plog10);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "log10", a, aref, plog10);
		}
	}
	return nrOfFailedTests;
}


// enumerate all base-e exponent cases for a fixpnt configuration
template<typename TestType>
int VerifyExp(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, cexp, cref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		cexp = sw::universal::exp(a);
		// generate reference
		double da = double(a);
		double dref = std::exp(da);
		cref = dref;
		if (cexp != cref) {
			// filter out inconsistencies among different math library implementations
			if (dref == 0.0) {
				static bool firstRoundingFilterEvent = true;
				if (firstRoundingFilterEvent && bReportIndividualTestCases) {
					std::cerr << "filtering fixpnt rounding to minpos\n";
					firstRoundingFilterEvent = false;
				}
			}
			else {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "exp", a, cref, cexp);
			}
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "exp", a, cref, cexp);
		}
	}
	return nrOfFailedTests;
}

// enumerate all base-2 exponent cases for a fixpnt configuration
template<typename TestType>
int VerifyExp2(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, cexp2, cref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		cexp2 = sw::universal::exp2(a);
		// generate reference
		double da = double(a);
		double dref = std::exp2(da);
		cref = dref;
		if (cexp2 != cref) {
			// filter out inconsistencies among different math library implementations
			if (dref == 0.0) {
				static bool firstRoundingFilterEvent = true;
				if (firstRoundingFilterEvent && bReportIndividualTestCases) {
					std::cerr << "filtering fixpnt rounding to minpos\n";
					firstRoundingFilterEvent = false;
				}
			}
			else {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "exp2", a, cref, cexp2);
			}
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "exp2", a, cref, cexp2);
		}
	}
	return nrOfFailedTests;
}

// enumerate all power method cases for a fixpnt configuration
template<typename TestType>
int VerifyPowerFunction(bool bReportIndividualTestCases, unsigned int maxSamples = 10000) {
	constexpr size_t nbits = TestType::nbits; 
	constexpr size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	TestType a, b, cpow, cref;

	uint32_t testNr = 0;
	for (size_t i = 0; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		double da = double(a);
		for (size_t j = 0; j < NR_TEST_CASES; ++j) {
			b.setbits(j);
			double db = double(b);
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			try {
				cpow = pow(a, b);
			}
			catch (const fixpnt_arithmetic_exception& err) {
				if (a.isnan()) {
					if (bReportIndividualTestCases) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
				}
				else {
					throw err;
				}
			}
#else
			cpow = pow(a, b);
#endif
			cref = std::pow(da, db);
			if (cpow != cref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportTwoInputFunctionError("FAIL", "pow", a, b, cref, cpow);
			}
			else {
				//if (bReportIndividualTestCases) ReportTwoInputFunctionSuccess("aSS", "pow", a, b, cref, cpow);
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

// enumerate all trigonometric sine cases for a fixpnt configuration
template<typename TestType>
int VerifySine(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, psin, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		psin = sw::universal::sin(a);
		// generate reference
		double da = double(a);
		aref = std::sin(da);
		if (psin != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "sin", a, aref, psin);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "sin", a, aref, psin);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosine cases for a fixpnt configuration
template<typename TestType>
int VerifyCosine(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, pcos, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		pcos = sw::universal::cos(a);
		// generate reference
		double da = double(a);
		aref = std::cos(da);
		if (pcos != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "cos", a, aref, pcos);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "cos", a, aref, pcos);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric tangent cases for a fixpnt configuration
template<typename TestType>
int VerifyTangent(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, ptan, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		ptan = sw::universal::tan(a);
		// generate reference
		double da = double(a);
		aref = std::tan(da);
		if (ptan != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "tan", a, aref, ptan);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "tan", a, aref, ptan);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cotangent cases for a fixpnt configuration
template<typename TestType>
int VerifyAtan(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, atan, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		atan = sw::universal::atan(a);
		// generate reference
		double da = double(a);
		aref = std::atan(da);
		if (atan != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "atan", a, aref, atan);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "atan", a, aref, atan);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric sec cases for a fixpnt configuration
template<typename TestType>
int VerifyAsin(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, asin, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		asin = sw::universal::asin(a);
		// generate reference
		double da = double(a);
		aref = std::asin(da);
		if (asin != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "asin", a, aref, asin);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "asin", a, aref, asin);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosec cases for a fixpnt configuration
template<typename TestType>
int VerifyAcos(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, acos, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		acos = sw::universal::acos(a);
		// generate reference
		double da = double(a);
		aref = std::acos(da);
		if (acos != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "acos", a, aref, acos);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "acos", a, aref, acos);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic sine cases for a fixpnt configuration
template<typename TestType>
int VerifySinh(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, psinh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		psinh = sw::universal::sinh(a);
		// generate reference
		double da = double(a);
		aref = std::sinh(da);
		if (psinh != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "sinh", a, aref, psinh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "sinh", a, aref, psinh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosine cases for a fixpnt configuration
template<typename TestType>
int VerifyCosh(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, pcosh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		pcosh = sw::universal::cosh(a);
		// generate reference
		double da = double(a);
		aref = std::cosh(da);
		if (pcosh != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "cosh", a, aref, pcosh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "cosh", a, aref, pcosh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic tangent cases for a fixpnt configuration
template<typename TestType>
int VerifyTanh(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, ptanh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		ptanh = sw::universal::tanh(a);
		// generate reference
		double da = double(a);
		aref = std::tanh(da);
		if (ptanh != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "tanh", a, aref, ptanh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "tanh", a, aref, ptanh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cotangent cases for a fixpnt configuration
template<typename TestType>
int VerifyAtanh(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, atanh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		atanh = sw::universal::atanh(a);
		// generate reference
		double da = double(a);
		aref = std::atanh(da);
		if (atanh != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "atanh", a, aref, atanh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "atanh", a, aref, atanh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic sec cases for a fixpnt configuration
template<typename TestType>
int VerifyAsinh(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, aasinh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		aasinh = sw::universal::asinh(a);
		// generate reference
		double da = double(a);
		aref = std::asinh(da);
		if (aasinh != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "asinh", a, aref, aasinh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "asinh", a, aref, aasinh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosec cases for a fixpnt configuration
template<typename TestType>
int VerifyAcosh(bool bReportIndividualTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, aacosh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		aacosh = sw::universal::acosh(a);
		// generate reference
		double da = double(a);
		aref = std::acosh(da);
		if (aacosh != aref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "acosh", a, aref, aacosh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("aSS", "acosh", a, aref, aacosh);
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal

