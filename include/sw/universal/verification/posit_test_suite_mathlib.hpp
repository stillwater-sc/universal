#pragma once
// posit_test_suite_mathlib.hpp : functions to aid in testing and test reporting of function evaluation on posit types.
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

// mathematical function definitions and implementations
// NOTE: the posit mathlib is included by the posit umbrella header
// (either posit.hpp or posit1.hpp), so we don't include it here.
#include <universal/verification/posit_test_suite.hpp>

namespace sw { namespace universal {

/////////////////////////////// VALIDATION TEST SUITES ////////////////////////////////

////////////////////////////////////  MATHEMATICAL FUNCTIONS  //////////////////////////////////////////

// enumerate all NATURAL LOGARITHM cases for a posit configuration
template<typename TestType>
int VerifyLog(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, plog, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		plog = sw::universal::log(pa);
		// generate reference
		double da = double(pa);
		pref = std::log(da);
		if (plog != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log", pa, plog, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "log", pa, plog, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all BINARY LOGARITHM cases for a posit configuration
template<typename TestType>
int VerifyLog2(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, plog2, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		plog2 = sw::universal::log2(pa);
		// generate reference
		double da = double(pa);
		pref = std::log2(da);
		if (plog2 != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log2", pa, plog2, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "log2", pa, plog2, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all DECIMAL LOGARITHM cases for a posit configuration
template<typename TestType>
int VerifyLog10(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, plog10, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		plog10 = sw::universal::log10(pa);
		// generate reference
		double da = double(pa);
		pref = std::log10(da);
		if (plog10 != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log10", pa, plog10, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "log10", pa, plog10, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all base-e exponent cases for a posit configuration
template<typename TestType>
int VerifyExp(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pexp, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pexp = sw::universal::exp(pa);
		// generate reference
		double da = double(pa);
		pref = std::exp(da);
		if (pexp != pref) {
			if (std::exp(da) != 0.0) { // exclude special posit rounding rule that projects to minpos
				nrOfFailedTests++;
				if (reportTestCases)	ReportOneInputFunctionError("FAIL", "exp", pa, pexp, pref);
			}
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "exp", pa, pexp, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all base-2 exponent cases for a posit configuration
template<typename TestType>
int VerifyExp2(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pexp2, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pexp2 = sw::universal::exp2(pa);
		// generate reference
		double da = double(pa);
		pref = std::exp2(da);
		if (pexp2 != pref) {
			if (std::exp2(da) != 0.0) { // exclude special posit rounding rule that projects to minpos
				nrOfFailedTests++;
				if (reportTestCases)	ReportOneInputFunctionError("FAIL", "exp2", pa, pexp2, pref);
			}
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "exp2", pa, pexp2, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all power method cases for a posit configuration
template<typename TestType>
int VerifyPowerFunction(bool reportTestCases, unsigned int maxSamples = 10000) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pb, ppow, pref;

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
					if (reportTestCases) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
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
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "pow", pa, pb, ppow, pref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("PASS", "pow", pa, pb, ppow, pref);
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
template<typename TestType>
int VerifySine(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, psin, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		psin = sw::universal::sin(pa);
		// generate reference
		double da = double(pa);
		pref = std::sin(da);
		if (psin != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "sin", pa, psin, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "sin", pa, psin, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all trigonometric cosine cases for a posit configuration
template<typename TestType>
int VerifyCosine(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pcos, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pcos = sw::universal::cos(pa);
		// generate reference
		double da = double(pa);
		pref = std::cos(da);
		if (pcos != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "cos", pa, pcos, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "cos", pa, pcos, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all trigonometric tangent cases for a posit configuration
template<typename TestType>
int VerifyTangent(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, ptan, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		ptan = sw::universal::tan(pa);
		// generate reference
		double da = double(pa);
		pref = std::tan(da);
		if (ptan != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "tan", pa, ptan, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "tan", pa, ptan, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all trigonometric cotangent cases for a posit configuration
template<typename TestType>
int VerifyAtan(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, patan, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		patan = sw::universal::atan(pa);
		// generate reference
		double da = double(pa);
		pref = std::atan(da);
		if (patan != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "atan", pa, patan, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "atan", pa, patan, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all trigonometric sec cases for a posit configuration
template<typename TestType>
int VerifyAsin(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pasin, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pasin = sw::universal::asin(pa);
		// generate reference
		double da = double(pa);
		pref = std::asin(da);
		if (pasin != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "asin", pa, pasin, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "asin", pa, pasin, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all trigonometric cosec cases for a posit configuration
template<typename TestType>
int VerifyAcos(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pacos, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pacos = sw::universal::acos(pa);
		// generate reference
		double da = double(pa);
		pref = std::acos(da);
		if (pacos != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "acos", pa, pacos, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "acos", pa, pacos, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all hyperbolic sine cases for a posit configuration
template<typename TestType>
int VerifySinh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, psinh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		psinh = sw::universal::sinh(pa);
		// generate reference
		double da = double(pa);
		pref = std::sinh(da);
		if (psinh != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "sinh", pa, psinh, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "sinh", pa, psinh, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all hyperbolic cosine cases for a posit configuration
template<typename TestType>
int VerifyCosh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pcosh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pcosh = sw::universal::cosh(pa);
		// generate reference
		double da = double(pa);
		pref = std::cosh(da);
		if (pcosh != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "cosh", pa, pcosh, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "cosh", pa, pcosh, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all hyperbolic tangent cases for a posit configuration
template<typename TestType>
int VerifyTanh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, ptanh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		ptanh = sw::universal::tanh(pa);
		// generate reference
		double da = double(pa);
		pref = std::tanh(da);
		if (ptanh != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "tanh", pa, ptanh, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "tanh", pa, ptanh, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all hyperbolic cotangent cases for a posit configuration
template<typename TestType>
int VerifyAtanh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, patanh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		patanh = sw::universal::atanh(pa);
		// generate reference
		double da = double(pa);
		pref = std::atanh(da);
		if (patanh != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "atanh", pa, patanh, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "atanh", pa, patanh, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all hyperbolic sec cases for a posit configuration
template<typename TestType>
int VerifyAsinh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pasinh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pasinh = sw::universal::asinh(pa);
		// generate reference
		double da = double(pa);
		pref = std::asinh(da);
		if (pasinh != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "asinh", pa, pasinh, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "asinh", pa, pasinh, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all hyperbolic cosec cases for a posit configuration
template<typename TestType>
int VerifyAcosh(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pacosh, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		pacosh = sw::universal::acosh(pa);
		// generate reference
		double da = double(pa);
		pref = std::acosh(da);
		if (pacosh != pref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "acosh", pa, pacosh, pref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("PASS", "acosh", pa, pacosh, pref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all hypotenuse cases for a posit configuration
template<typename TestType>
int VerifyHypot(bool reportTestCases) {
	constexpr unsigned nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType pa, pb, phypot, pref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		pa.setbits(i);
		double da = double(pa);
		for (size_t j = 1; j < NR_TEST_CASES; ++j) {
			pb.setbits(j);
			phypot = sw::universal::hypot(pa, pb);
			// generate reference
			double db = double(pb);
			pref = std::hypot(da, db);
			if (phypot != pref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "hypot", pa, pb, phypot, pref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("PASS", "hypot", pa, pb, phypot, pref);
			}
		}
	}
	return nrOfFailedTests;
}


//////////////////////////////////// RANDOMIZED TEST SUITE FOR BINARY OPERATORS ////////////////////////


}} // namespace sw::universal

