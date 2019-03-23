#pragma once
//  posit_math_helpers.hpp : functions to aid in testing and test reporting of function evaluation on posit types.
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

// mathematical function definitions and implementations
#include "../posit/math_functions.hpp"

// include the base test helpers
#include "posit_test_helpers.hpp"

namespace sw {
	namespace unum {

//		static constexpr unsigned FLOAT_TABLE_WIDTH = 15;

		template<size_t nbits, size_t es>
		void ReportTwoInputFunctionError(std::string test_case, std::string op, const posit<nbits, es>& a, const posit<nbits, es>& b, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
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
		void ReportTwoInputFunctionSuccess(std::string test_case, std::string op, const posit<nbits, es>& a, const posit<nbits, es>& b, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
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
		void ReportOneInputFunctionError(std::string test_case, std::string op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
			std::cerr << test_case
				<< " " << op << " "
				<< std::setw(FLOAT_TABLE_WIDTH) << rhs
				<< " != "
				<< std::setw(FLOAT_TABLE_WIDTH) << pref << " instead it yielded "
				<< std::setw(FLOAT_TABLE_WIDTH) << presult
				<< " " << pref.get() << " vs " << presult.get() << std::endl;
		}

		template<size_t nbits, size_t es>
		void ReportOneInputFunctionSuccess(std::string test_case, std::string op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
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
		int ValidateLog(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, plog, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				plog = sw::unum::log(pa);
				// generate reference
				da = double(pa);
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
		int ValidateLog2(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, plog2, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				plog2 = sw::unum::log2(pa);
				// generate reference
				da = double(pa);
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
		int ValidateLog10(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, plog10, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				plog10 = sw::unum::log10(pa);
				// generate reference
				da = double(pa);
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
		int ValidateExp(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pexp, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				pexp = sw::unum::exp(pa);
				// generate reference
				da = double(pa);
				pref = std::exp(da);
				if (pexp != pref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "exp", pa, pref, pexp);
				}
				else {
					//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "exp", pa, pref, pexp);
				}
			}
			return nrOfFailedTests;
		}

		// enumerate all base-2 exponent cases for a posit configuration
		template<size_t nbits, size_t es>
		int ValidateExp2(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pexp2, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				pexp2 = sw::unum::exp2(pa);
				// generate reference
				da = double(pa);
				pref = std::exp2(da);
				if (pexp2 != pref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportOneInputFunctionError("FAIL", "exp2", pa, pref, pexp2);
				}
				else {
					//if (bReportIndividualTestCases) ReportOneInputFunctionSuccess("PASS", "exp2", pa, pref, pexp2);
				}
			}
			return nrOfFailedTests;
		}

		// enumerate all power method cases for a posit configuration
		template<size_t nbits, size_t es>
		int ValidatePowerFunction(std::string tag, bool bReportIndividualTestCases, unsigned int maxSamples = 10000) {
			const int NR_POSITS = (unsigned(1) << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pb, ppow, pref;

			uint32_t testNr = 0;
			double da, db;
			for (int i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = double(pa);
				for (int j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					db = double(pb);
					ppow = pow(pa, pb);
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
						std::cerr << "ValidatePower has been truncated\n";
						i = j = NR_POSITS;
					}
				}
			}

			return nrOfFailedTests;
		}

		// enumerate all trigonometric sine cases for a posit configuration
		template<size_t nbits, size_t es>
		int ValidateSine(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, psin, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				psin = sw::unum::sin(pa);
				// generate reference
				da = double(pa);
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
		int ValidateCosine(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pcos, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				pcos = sw::unum::cos(pa);
				// generate reference
				da = double(pa);
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
		int ValidateTangent(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, ptan, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				ptan = sw::unum::tan(pa);
				// generate reference
				da = double(pa);
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
		int ValidateAtan(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, patan, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				patan = sw::unum::atan(pa);
				// generate reference
				da = double(pa);
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
		int ValidateAsin(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pasin, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				pasin = sw::unum::asin(pa);
				// generate reference
				da = double(pa);
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
		int ValidateAcos(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pacos, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				pacos = sw::unum::acos(pa);
				// generate reference
				da = double(pa);
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
		int ValidateSinh(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, psinh, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				psinh = sw::unum::sinh(pa);
				// generate reference
				da = double(pa);
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
		int ValidateCosh(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pcosh, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				pcosh = sw::unum::cosh(pa);
				// generate reference
				da = double(pa);
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
		int ValidateTanh(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, ptanh, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				ptanh = sw::unum::tanh(pa);
				// generate reference
				da = double(pa);
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
		int ValidateAtanh(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, patanh, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				patanh = sw::unum::atanh(pa);
				// generate reference
				da = double(pa);
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
		int ValidateAsinh(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pasinh, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				pasinh = sw::unum::asinh(pa);
				// generate reference
				da = double(pa);
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
		int ValidateAcosh(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pacosh, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				pacosh = sw::unum::acosh(pa);
				// generate reference
				da = double(pa);
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



	} // namespace unum

} // namespace sw

