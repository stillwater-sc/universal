#pragma once
// cfloat_test_suite_mathlib.hpp : test suite runners for math library functions
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
#include <universal/number/cfloat/mathlib.hpp>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

/////////////////////////////// VALIDATION TEST SUITES ////////////////////////////////

////////////////////////////////////  MATHEMATICAL FUNCTIONS  //////////////////////////////////////////

// enumerate all NATURAL LOGARITHM cases for a cfloat configuration
template<typename TestType>
int VerifyLog(bool reportTestCases) {
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
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log", a, alog, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "log", a, alog, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all BINARY LOGARITHM cases for a cfloat configuration
template<typename TestType>
int VerifyLog2(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, alog2, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		alog2 = sw::universal::log2(a);
		// generate reference
		double da = double(a);
		aref = std::log2(da);
		if (alog2 != aref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log2", a, alog2, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "log2", a, alog2, aref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all DECIMAL LOGARITHM cases for a cfloat configuration
template<typename TestType>
int VerifyLog10(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, alog10, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		alog10 = sw::universal::log10(a);
		// generate reference
		double da = double(a);
		aref = std::log10(da);
		if (alog10 != aref) {
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "log10", a, alog10, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "log10", a, alog10, aref);
		}
	}
	return nrOfFailedTests;
}


// enumerate all base-e exponent cases for a cfloat configuration
template<typename TestType>
int VerifyExp(bool reportTestCases) {
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
				if (firstRoundingFilterEvent && reportTestCases) {
					std::cerr << "filtering cfloat rounding to minpos\n";
					firstRoundingFilterEvent = false;
				}
			}
			else if (cexp.isnan() && cref.isnan()) { 
				static bool firstSofteningNanEvent = true;
				if (firstSofteningNanEvent && reportTestCases) {
					std::cerr << "filtering snan to nan softening\n";
					firstSofteningNanEvent = false;
				}
			}
			else {
				nrOfFailedTests++;
				if (reportTestCases)	ReportOneInputFunctionError("FAIL", "exp", a, cexp, cref);
			}
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "exp", a, cexp, cref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all base-2 exponent cases for a cfloat configuration
template<typename TestType>
int VerifyExp2(bool reportTestCases) {
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
				if (firstRoundingFilterEvent && reportTestCases) {
					std::cerr << "filtering cfloat rounding to minpos\n";
					firstRoundingFilterEvent = false;
				}
			}
			else if (cexp2.isnan() && cref.isnan()) {
				static bool firstSofteningNanEvent = true;
				if (firstSofteningNanEvent && reportTestCases) {
					std::cerr << "filtering snan to nan softening\n";
					firstSofteningNanEvent = false;
				}
			}
			else {
				nrOfFailedTests++;
				if (reportTestCases)	ReportOneInputFunctionError("FAIL", "exp2", a, cexp2, cref);
			}
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "exp2", a, cexp2, cref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all power method cases for a cfloat configuration
template<typename TestType>
int VerifyPowerFunction(bool reportTestCases, unsigned int maxSamples = 10000) {
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
#if CFLOAT_THROW_ARITHMETIC_EXCEPTION
			try {
				cpow = pow(a, b);
			}
			catch (const cfloat_arithmetic_exception& err) {
				if (a.isnan()) {
					if (reportTestCases) std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
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
				if (cpow.isnan() && cref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "pow", a, b, cpow, cref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("aSS", "pow", a, b, cpow, cref);
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

// enumerate all trigonometric sine cases for a cfloat configuration
template<typename TestType>
int VerifySine(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, asin, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		asin = sw::universal::sin(a);
		// generate reference
		double da = double(a);
		aref = std::sin(da);
		if (asin != aref) {
			if (asin.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "sin", a, asin, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "sin", a, asin, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosine cases for a cfloat configuration
template<typename TestType>
int VerifyCosine(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, acos, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		acos = sw::universal::cos(a);
		// generate reference
		double da = double(a);
		aref = std::cos(da);
		if (acos != aref) {
			if (acos.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "cos", a, acos, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "cos", a, acos, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric tangent cases for a cfloat configuration
template<typename TestType>
int VerifyTangent(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, atan, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		atan = sw::universal::tan(a);
		// generate reference
		double da = double(a);
		aref = std::tan(da);
		if (atan != aref) {
			if (atan.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "tan", a, atan, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "tan", a, atan, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cotangent cases for a cfloat configuration
template<typename TestType>
int VerifyAtan(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, aatan, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		aatan = sw::universal::atan(a);
		// generate reference
		double da = double(a);
		aref = std::atan(da);
		if (aatan != aref) {
			if (aatan.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "atan", a, aatan, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "atan", a, aatan, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric sec cases for a cfloat configuration
template<typename TestType>
int VerifyAsin(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, aasin, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		aasin = sw::universal::asin(a);
		// generate reference
		double da = double(a);
		aref = std::asin(da);
		if (aasin != aref) {
			if (aasin.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "asin", a, aasin, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "asin", a, aasin, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all trigonometric cosec cases for a cfloat configuration
template<typename TestType>
int VerifyAcos(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, aacos, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		aacos = sw::universal::acos(a);
		// generate reference
		double da = double(a);
		aref = std::acos(da);
		if (aacos != aref) {
			if (aacos.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "acos", a, aacos, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "acos", a, aacos, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic sine cases for a cfloat configuration
template<typename TestType>
int VerifySinh(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, asinh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		asinh = sw::universal::sinh(a);
		// generate reference
		double da = double(a);
		aref = std::sinh(da);
		if (asinh != aref) {
			if (asinh.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "sinh", a, asinh, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "sinh", a, asinh, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosine cases for a cfloat configuration
template<typename TestType>
int VerifyCosh(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, acosh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		acosh = sw::universal::cosh(a);
		// generate reference
		double da = double(a);
		aref = std::cosh(da);
		if (acosh != aref) {
			if (acosh.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "cosh", a, acosh, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "cosh", a, acosh, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic tangent cases for a cfloat configuration
template<typename TestType>
int VerifyTanh(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, atanh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		atanh = sw::universal::tanh(a);
		// generate reference
		double da = double(a);
		aref = std::tanh(da);
		if (atanh != aref) {
			if (atanh.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "tanh", a, atanh, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "tanh", a, atanh, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cotangent cases for a cfloat configuration
template<typename TestType>
int VerifyAtanh(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, aatanh, aref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		aatanh = sw::universal::atanh(a);
		// generate reference
		double da = double(a);
		aref = std::atanh(da);
		if (aatanh != aref) {
			if (aatanh.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "atanh", a, aatanh, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "atanh", a, aatanh, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic sec cases for a cfloat configuration
template<typename TestType>
int VerifyAsinh(bool reportTestCases) {
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
			if (aasinh.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "asinh", a, aasinh, aref);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "asinh", a, aasinh, aref);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hyperbolic cosec cases for a cfloat configuration
template<typename TestType>
int VerifyAcosh(bool reportTestCases) {
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
			if (aacosh.isnan() && aref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
			nrOfFailedTests++;
			if (reportTestCases)	ReportOneInputFunctionError("FAIL", "acosh", a, aref, aacosh);
		}
		else {
			//if (reportTestCases) ReportOneInputFunctionSuccess("aSS", "acosh", a, aref, aacosh);
		}
	}
	return nrOfFailedTests;
}

// enumerate all hypotenuse cases for a cfloat configuration
template<typename TestType>
int VerifyHypot(bool reportTestCases) {
	constexpr size_t nbits = TestType::nbits;
	constexpr size_t NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	TestType a, b, hypot, ref;

	for (size_t i = 1; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		double da = double(a);
		for (size_t j = 1; j < NR_TEST_CASES; ++j) {
			b.setbits(j);
			hypot = sw::universal::hypot(a, b);
			// generate reference
			double db = double(b);
			ref = std::hypot(da, db);
			if (hypot != ref) {
				if (hypot.isnan() && ref.isnan()) return 0; // (s)nan != (s)nan, so the regular equivalance test fails
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoInputFunctionError("FAIL", "hypot", a, b, hypot, ref);
			}
			else {
				//if (reportTestCases) ReportTwoInputFunctionSuccess("PASS", "hypot", a, b, hypot, ref);
			}
		}
	}
	return nrOfFailedTests;
}

}} // namespace sw::universal

