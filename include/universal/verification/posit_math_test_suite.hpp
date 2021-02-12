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
#include <universal/number/posit/math_functions.hpp>
#include <universal/verification/posit_test_suite.hpp>

namespace sw { namespace universal {

static constexpr unsigned FLOAT_TABLE_WIDTH = 15;

template<size_t nbits, size_t es>
void ReportTwoInputFunctionError(const std::string& test_case, const std::string& op, const posit<nbits, es>& a, const posit<nbits, es>& b, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case << " " << op << "("
		<< std::setprecision(20)
		<< std::setw(FLOAT_TABLE_WIDTH) << a
		<< ","
		<< std::setw(FLOAT_TABLE_WIDTH) << b << ")"
		<< " != "
		<< std::setw(FLOAT_TABLE_WIDTH) << pref << " instead it yielded "
		<< std::setw(FLOAT_TABLE_WIDTH) << presult
		<< " " << pref.get() << " vs " << presult.get()
		<< std::setprecision(5)
		<< std::endl;
}

template<size_t nbits, size_t es>
void ReportTwoInputFunctionSuccess(const std::string& test_case, const std::string& op, const posit<nbits, es>& a, const posit<nbits, es>& b, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case << " " << op << "("
		<< std::setprecision(20)
		<< std::setw(FLOAT_TABLE_WIDTH) << a
		<< ","
		<< std::setw(FLOAT_TABLE_WIDTH) << b << ")"
		<< " == "
		<< std::setw(FLOAT_TABLE_WIDTH) << pref << " ==  "
		<< std::setw(FLOAT_TABLE_WIDTH) << presult
		<< " " << pref.get() << " vs " << presult.get()
		<< std::setprecision(5)
		<< std::endl;
}

template<size_t nbits, size_t es>
void ReportOneInputFunctionError(const std::string& test_case, const std::string& op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(FLOAT_TABLE_WIDTH) << rhs
		<< " != "
		<< std::setw(FLOAT_TABLE_WIDTH) << pref << " instead it yielded "
		<< std::setw(FLOAT_TABLE_WIDTH) << presult
		<< " " << pref.get() << " vs " << presult.get() << std::endl;
}

template<size_t nbits, size_t es>
void ReportOneInputFunctionSuccess(const std::string& test_case, const std::string& op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
	std::cerr << test_case
		<< " " << op << " "
		<< std::setw(FLOAT_TABLE_WIDTH) << rhs
		<< " == "
		<< std::setw(FLOAT_TABLE_WIDTH) << presult << " reference value is "
		<< std::setw(FLOAT_TABLE_WIDTH) << pref
		<< " " << components_to_string(presult) << std::endl;
}

/////////////////////////////// VALIDATION TEST SUITES ////////////////////////////////

////////////////////////////////////  MATHEMATICAL FUNCTIONS  //////////////////////////////////////////

// enumerate all NATURAL LOGARITHM cases for a posit configuration
template<size_t nbits, size_t es>
int VerifyLog(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, plog, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyLog2(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, plog2, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyLog10(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, plog10, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyExp(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pexp, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyExp2(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pexp2, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
		pexp2 = sw::universal::exp2(pa);
		// generate reference
		double da = double(pa);
		pref = std::exp2(da);
		if (pexp2 != pref) {
			if (std::exp(da) != 0.0) { // exclude special posit rounding rule that projects to minpos
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
int VerifyPowerFunction(const std::string& tag, bool bReportIndividualTestCases, unsigned int maxSamples = 10000) {
	const int NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pb, ppow, pref;

	uint32_t testNr = 0;
	for (int i = 0; i < NR_POSITS; i++) {
		pa.set_raw_bits(i);
		double da = double(pa);
		for (int j = 0; j < NR_POSITS; j++) {
			pb.set_raw_bits(j);
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
				i = j = NR_POSITS;
			}
		}
	}

	return nrOfFailedTests;
}

// enumerate all trigonometric sine cases for a posit configuration
template<size_t nbits, size_t es>
int VerifySine(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, psin, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyCosine(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pcos, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyTangent(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, ptan, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyAtan(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, patan, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyAsin(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pasin, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyAcos(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pacos, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifySinh(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, psinh, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyCosh(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pcosh, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyTanh(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, ptanh, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyAtanh(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, patanh, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyAsinh(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pasinh, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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
int VerifyAcosh(const std::string& tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pacosh, pref;

	for (int i = 1; i < NR_TEST_CASES; i++) {
		pa.set_raw_bits(i);
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



}} // namespace sw:universal

