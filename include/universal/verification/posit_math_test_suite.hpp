#pragma once
//  posit_math_test_suite.hpp : functions to aid in testing and test reporting of function evaluation on posit types.
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

// mathematical function definitions and implementations
#include <universal/number/posit/mathlib.hpp>
#include <universal/verification/test_reporters.hpp>
#include <universal/verification/posit_test_suite.hpp>

namespace sw::universal {

/////////////////////////////// VALIDATION TEST SUITES ////////////////////////////////

////////////////////////////////////  MATHEMATICAL FUNCTIONS  //////////////////////////////////////////

// enumerate all NATURAL LOGARITHM cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyLog(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, plog, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		plog = sw::universal::log(pa);
		// generate reference
		double da = double(pa);
		pref = std::log(da);
		if (plog != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "log", pa, pref, plog);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "log", pa, pref, plog);
		}
	}
	return nrOfFailedTests;
}

// enumerate all BINARY LOGARITHM cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyLog2(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, plog2, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		plog2 = sw::universal::log2(pa);
		// generate reference
		double da = double(pa);
		pref = std::log2(da);
		if (plog2 != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "log2", pa, pref, plog2);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "log2", pa, pref, plog2);
		}
	}
	return nrOfFailedTests;
}


// enumerate all DECIMAL LOGARITHM cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyLog10(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, plog10, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		plog10 = sw::universal::log10(pa);
		// generate reference
		double da = double(pa);
		pref = std::log10(da);
		if (plog10 != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "log10", pa, pref, plog10);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "log10", pa, pref, plog10);
		}
	}
	return nrOfFailedTests;
}


// enumerate all base-e exponent cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyExp(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pexp, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pexp = sw::universal::exp(pa);
		// generate reference
		double da = double(pa);
		pref = std::exp(da);
		if (pexp != pref) {
			if (std::exp(da) != 0.0) { // exclude special posit rounding rule that projects to minpos
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "exp", pa, pref, pexp);
			}
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "exp", pa, pref, pexp);
		}
	}
	return nrOfFailedTests;
}

// enumerate all base-2 exponent cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyExp2(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pexp2, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pexp2 = sw::universal::exp2(pa);
		// generate reference
		double da = double(pa);
		pref = std::exp2(da);
		if (pexp2 != pref) {
			if (std::exp2(da) != 0.0) { // exclude special posit rounding rule that projects to minpos
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "exp2", pa, pref, pexp2);
			}
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "exp2", pa, pref, pexp2);
		}
	}
	return nrOfFailedTests;
}

// enumerate all power method cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyPowerFunction(bool bReportIndividualTestCases, unsigned int maxSamples = 10000) {
	constexpr size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pb, ppow, pref;

	uint32_t testNr = 0;
	for (size_t i = 0; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		double da = double(pa);
		for (size_t j = 0; j < NR_TEST_CASES; ++j) {
			pb.setbits(j);
			double db = double(pb);
#if POSIT_THROW_ARITHMETIC_EXCEPTION
			try {
				ppow = pow(pa, pb);
			}
			catch (const posit_arithmetic_exception& err) {
				if (pa.isnar()) {
					if (bReportIndividualTestCases) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
				}
				else {
					throw err;
				}
			}
#else
			ppow = pow(pa, pb);
#endif
			pref = std::pow(da, db);
			if (ppow != pref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportTwoInputFunctionError("FAIL", "pow", pa, pb, pref, ppow);
			}
			else {
				//if (bReportIndividualTestCases) ReportTwoInputFunctionSuccess("PASS", "pow", pa, pb, pref, ppow);
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

// enumerate all trigonometric sine cases for a posit configuration
template<size_t nbits, size_t es>
int VerifySine(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, psin, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		psin = sw::universal::sin(pa);
		// generate reference
		double da = double(pa);
		pref = std::sin(da);
		if (psin != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "sin", pa, pref, psin);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "sin", pa, pref, psin);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosine cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyCosine(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pcos, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pcos = sw::universal::cos(pa);
		// generate reference
		double da = double(pa);
		pref = std::cos(da);
		if (pcos != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "cos", pa, pref, pcos);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "cos", pa, pref, pcos);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric tangent cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyTangent(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, ptan, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		ptan = sw::universal::tan(pa);
		// generate reference
		double da = double(pa);
		pref = std::tan(da);
		if (ptan != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "tan", pa, pref, ptan);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "tan", pa, pref, ptan);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cotangent cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyAtan(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, patan, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		patan = sw::universal::atan(pa);
		// generate reference
		double da = double(pa);
		pref = std::atan(da);
		if (patan != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "atan", pa, pref, patan);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "atan", pa, pref, patan);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric sec cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyAsin(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pasin, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pasin = sw::universal::asin(pa);
		// generate reference
		double da = double(pa);
		pref = std::asin(da);
		if (pasin != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "asin", pa, pref, pasin);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "asin", pa, pref, pasin);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosec cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyAcos(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pacos, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pacos = sw::universal::acos(pa);
		// generate reference
		double da = double(pa);
		pref = std::acos(da);
		if (pacos != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "acos", pa, pref, pacos);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "acos", pa, pref, pacos);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic sine cases for a posit configuration
template<size_t nbits, size_t es>
int VerifySinh(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, psinh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		psinh = sw::universal::sinh(pa);
		// generate reference
		double da = double(pa);
		pref = std::sinh(da);
		if (psinh != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "sinh", pa, pref, psinh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "sinh", pa, pref, psinh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosine cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyCosh(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pcosh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pcosh = sw::universal::cosh(pa);
		// generate reference
		double da = double(pa);
		pref = std::cosh(da);
		if (pcosh != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "cosh", pa, pref, pcosh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "cosh", pa, pref, pcosh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic tangent cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyTanh(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, ptanh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		ptanh = sw::universal::tanh(pa);
		// generate reference
		double da = double(pa);
		pref = std::tanh(da);
		if (ptanh != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "tanh", pa, pref, ptanh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "tanh", pa, pref, ptanh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cotangent cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyAtanh(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, patanh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		patanh = sw::universal::atanh(pa);
		// generate reference
		double da = double(pa);
		pref = std::atanh(da);
		if (patanh != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "atanh", pa, pref, patanh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "atanh", pa, pref, patanh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic sec cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyAsinh(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pasinh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pasinh = sw::universal::asinh(pa);
		// generate reference
		double da = double(pa);
		pref = std::asinh(da);
		if (pasinh != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "asinh", pa, pref, pasinh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "asinh", pa, pref, pasinh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosec cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyAcosh(bool bReportIndividualTestCases) {
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pacosh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pacosh = sw::universal::acosh(pa);
		// generate reference
		double da = double(pa);
		pref = std::acosh(da);
		if (pacosh != pref) {
			nrOfFailedTests++;
			if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "acosh", pa, pref, pacosh);
		}
		else {
			//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "acosh", pa, pref, pacosh);
		}
	}
	return nrOfFailedTests;
}

//////////////////////////////////// RANDOMIZED TEST SUITE FOR BINARY OPERATORS ////////////////////////


} // namespace sw:universal

