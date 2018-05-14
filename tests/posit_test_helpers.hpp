#pragma once

//  posit_test_helpers.hpp : functions to aid in testing and test reporting on posit types.
// Needs to be included after posit type is declared.
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>

#include "../math/sqrt.hpp"
#include "../math/pow.hpp"
#include "../math/logarithm.hpp"
#include "../math/exponent.hpp"
#include "../math/trigonometric.hpp"
#include "../math/hyperbolic.hpp"

namespace sw {
	namespace unum {


		static constexpr unsigned FLOAT_TABLE_WIDTH = 15;

		template<size_t nbits, size_t es>
		void ReportConversionError(std::string test_case, std::string op, double input, double reference, const posit<nbits, es>& presult) {
			unsigned width = 15;
			std::cerr << test_case
				<< " " << op << " "
				<< std::setw(FLOAT_TABLE_WIDTH) << input
				<< " did not convert to "
				<< std::setw(FLOAT_TABLE_WIDTH) << reference << " instead it yielded "
				<< std::setw(FLOAT_TABLE_WIDTH) << double(presult)
				<< "  raw " << std::setw(nbits) << presult.get()
				<< "   scale= " << std::setw(3) << presult.scale() << "   k= " << std::setw(3) << presult.regime_k() << "   exp= " << std::setw(3) << presult.exp()
				<< std::endl;
		}

		template<size_t nbits, size_t es>
		void ReportConversionSuccess(std::string test_case, std::string op, double input, double reference, const posit<nbits, es>& presult) {
			std::cerr << test_case
				<< " " << op << " "
				<< std::setw(FLOAT_TABLE_WIDTH) << input
				<< " did     convert to "
				<< std::setw(FLOAT_TABLE_WIDTH) << double(presult) << " reference value is "
				<< std::setw(FLOAT_TABLE_WIDTH) << reference
				<< "  raw " << std::setw(nbits) << presult.get()
				<< "   scale= " << std::setw(3) << presult.scale() << "   k= " << std::setw(3) << presult.regime_k() << "   exp= " << std::setw(3) << presult.exp()
				<< std::endl;
		}

		template<size_t nbits, size_t es>
		void ReportUnaryArithmeticError(std::string test_case, std::string op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
			std::cerr << test_case
				<< " " << op << " "
				<< std::setw(FLOAT_TABLE_WIDTH) << rhs
				<< " != "
				<< std::setw(FLOAT_TABLE_WIDTH) << pref << " instead it yielded "
				<< std::setw(FLOAT_TABLE_WIDTH) << presult
				<< " " << pref.get() << " vs " << presult.get() << std::endl;
		}

		template<size_t nbits, size_t es>
		void ReportUnaryArithmeticSuccess(std::string test_case, std::string op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
			std::cerr << test_case
				<< " " << op << " "
				<< std::setw(FLOAT_TABLE_WIDTH) << rhs
				<< " == "
				<< std::setw(FLOAT_TABLE_WIDTH) << presult << " reference value is "
				<< std::setw(FLOAT_TABLE_WIDTH) << pref
				<< " " << components_to_string(presult) << std::endl;
		}

		template<size_t nbits, size_t es>
		void ReportBinaryArithmeticError(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
			std::cerr << test_case << " " 
				<< std::setprecision(20)
				<< std::setw(FLOAT_TABLE_WIDTH) << lhs
				<< " " << op << " "
				<< std::setw(FLOAT_TABLE_WIDTH) << rhs
				<< " != "
				<< std::setw(FLOAT_TABLE_WIDTH) << pref << " instead it yielded "
				<< std::setw(FLOAT_TABLE_WIDTH) << presult
				<< " " << pref.get() << " vs " << presult.get() 
				<< std::setprecision(5)
				<< std::endl;
		}

		template<size_t nbits, size_t es>
		void ReportBinaryArithmeticErrorInBinary(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
			std::cerr << test_case << " "
				<< std::setw(nbits) << lhs.get()
				<< " " << op << " "
				<< std::setw(nbits) << rhs.get()
				<< " != "
				<< std::setw(nbits) << pref.get() << " instead it yielded "
				<< std::setw(nbits) << presult.get()
				<< " " << pretty_print(presult,20) << std::endl;
		}

		template<size_t nbits, size_t es>
		void ReportBinaryArithmeticSuccess(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
			std::cerr << test_case << " "
				<< std::setprecision(20)
				<< std::setw(FLOAT_TABLE_WIDTH) << lhs
				<< " " << op << " "
				<< std::setw(FLOAT_TABLE_WIDTH) << rhs
				<< " == "
				<< std::setw(FLOAT_TABLE_WIDTH) << presult << " reference value is "
				<< std::setw(FLOAT_TABLE_WIDTH) << pref
				<< " " << pref.get() << " vs " << presult.get() 
				<< std::setprecision(5)
				<< std::endl;
		}

		template<size_t nbits, size_t es>
		void ReportBinaryArithmeticSuccessInBinary(std::string test_case, std::string op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
			std::cerr << test_case << " "
				<< std::setw(nbits) << lhs.get()
				<< " " << op << " "
				<< std::setw(nbits) << rhs.get()
				<< " == "
				<< std::setw(nbits) << presult.get() << " reference value is "
				<< std::setw(nbits) << pref.get()
				<< " " << pretty_print(presult,20) << std::endl;
		}
		template<size_t nbits, size_t es>
		void ReportDecodeError(std::string test_case, const posit<nbits, es>& actual, double golden_value) {
			std::cerr << test_case << " actual " << actual << " required " << golden_value << std::endl;
		}

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

		/////////////////////////////// VALIDATION TEST SUITES ////////////////////////////////

		template<size_t nbits, size_t es>
		int Compare(double input, const posit<nbits, es>& presult, double reference, bool bReportIndividualTestCases) {
			int fail = 0;
			double result = double(presult);
			if (std::fabs(result - reference) > 0.000000001) {
				fail++;
				if (bReportIndividualTestCases)	ReportConversionError("FAIL", "=", input, reference, presult);
			}
			else {
				// if (bReportIndividualTestCases) ReportConversionSuccess("PASS", "=", input, reference, presult);
			}
			return fail;
		}

		// enumerate all conversion cases for a posit configuration
		template<size_t nbits, size_t es>
		int ValidateConversion(std::string tag, bool bReportIndividualTestCases) {
			// we are going to generate a test set that consists of all posit configs and their midpoints
			// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
			const int NR_TEST_CASES = (1 << (nbits + 1));
			const int HALF = (1 << nbits);
			posit<nbits + 1, es> pref, pprev, pnext;

			// execute the test
			int nrOfFailedTests = 0;
			double minpos = minpos_value<nbits+1, es>();
			double eps;
			double da, input;
			posit<nbits, es> pa;
			for (int i = 0; i < NR_TEST_CASES; i++) {
				pref.set_raw_bits(i);
				da = double(pref);
				if (i == 0) {
					eps = minpos / 2.0;
				}
				else {
					eps = da > 0 ? da * 1.0e-6 : da * -1.0e-6;
				}
				if (i % 2) {
					if (i == 1) {
						// special case of projecting to +minpos
						// even the -delta goes to +minpos
						input = da - eps;
						pa = input;
						pnext.set_raw_bits(i + 1);
						nrOfFailedTests += Compare(input, pa, (double)pnext, bReportIndividualTestCases);
						input = da + eps;
						pa = input;
						nrOfFailedTests += Compare(input, pa, (double)pnext, bReportIndividualTestCases);

					}
					else if (i == HALF - 1) {
						// special case of projecting to +maxpos
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(HALF - 2);
						nrOfFailedTests += Compare(input, pa, (double)pprev, bReportIndividualTestCases);
					}
					else if (i == HALF + 1) {
						// special case of projecting to -maxpos
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(HALF + 2);
						nrOfFailedTests += Compare(input, pa, (double)pprev, bReportIndividualTestCases);
					}
					else if (i == NR_TEST_CASES - 1) {
						// special case of projecting to -minpos
						// even the +delta goes to -minpos
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(i - 1);
						nrOfFailedTests += Compare(input, pa, (double)pprev, bReportIndividualTestCases);
						input = da + eps;
						pa = input;
						nrOfFailedTests += Compare(input, pa, (double)pprev, bReportIndividualTestCases);
					}
					else {
						// for odd values, we are between posit values, so we create the round-up and round-down cases
						// round-down
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(i - 1);
						nrOfFailedTests += Compare(input, pa, (double)pprev, bReportIndividualTestCases);
						// round-up
						input = da + eps;
						pa = input;
						pnext.set_raw_bits(i + 1);
						nrOfFailedTests += Compare(input, pa, (double)pnext, bReportIndividualTestCases);
					}
				}
				else {
					// for the even values, we generate the round-to-actual cases
					if (i == 0) {
						// special case of projecting to +minpos
						input = da + eps;
						pa = input;
						pnext.set_raw_bits(i + 2);
						nrOfFailedTests += Compare(input, pa, (double)pnext, bReportIndividualTestCases);
					}
					else if (i == NR_TEST_CASES - 2) {
						// special case of projecting to -minpos
						input = da - eps;
						pa = input;
						pprev.set_raw_bits(NR_TEST_CASES - 2);
						nrOfFailedTests += Compare(input, pa, (double)pprev, bReportIndividualTestCases);
					}
					else {
						// round-up
						input = da - eps;
						pa = input;
						nrOfFailedTests += Compare(input, pa, da, bReportIndividualTestCases);
						// round-down
						input = da + eps;
						pa = input;
						nrOfFailedTests += Compare(input, pa, da, bReportIndividualTestCases);
					}
				}
			}
			return nrOfFailedTests;
		}

		// Generate ordered set in ascending order from [-NaR, -maxpos, ..., +maxpos] for a particular posit config <nbits, es>
		template<size_t nbits, size_t es>
		void GenerateOrderedPositSet(std::vector<posit<nbits, es>>& set) {
			const size_t NR_OF_REALS = (unsigned(1) << nbits);		// don't do this for state spaces larger than 4G
			std::vector< posit<nbits, es> > s(NR_OF_REALS);
			posit<nbits, es> p;
			// generate raw set, which will sort later
			for (int i = 0; i < NR_OF_REALS; i++) {
				p.set_raw_bits(i);
				s[i] = p;
			}
			// sort the set
			std::sort(s.begin(), s.end());
			set = s;
		}

		// validate the increment operator++
		template<size_t nbits, size_t es>
		int ValidateIncrement(std::string tag, bool bReportIndividualTestCases)
		{
			const size_t NrOfReals = (unsigned(1) << nbits);
			std::vector< posit<nbits, es> > set;
			GenerateOrderedPositSet(set); // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

			int nrOfFailedTestCases = 0;

			posit<nbits, es> p, ref;
			// starting from NaR iterating from -maxpos to maxpos through zero
			for (typename std::vector < posit<nbits, es> >::iterator it = set.begin(); it != set.end() - 1; it++) {
				p = *it;
				p++;
				ref = *(it + 1);
				if (p != ref) {
					if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << ref << std::endl;
					nrOfFailedTestCases++;
				}
			}

			return nrOfFailedTestCases;
		}

		// validate the decrement operator--
		template<size_t nbits, size_t es>
		int ValidateDecrement(std::string tag, bool bReportIndividualTestCases)
		{
			const size_t NrOfReals = (unsigned(1) << nbits);
			std::vector< posit<nbits, es> > set;
			GenerateOrderedPositSet(set); // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

			int nrOfFailedTestCases = 0;

			posit<nbits, es> p, ref;
			// starting from maxpos iterating to -maxpos, and finally NaR via zero
			for (typename std::vector < posit<nbits, es> >::iterator it = set.end() - 1; it != set.begin(); it--) {
				p = *it;
				p--;
				ref = *(it - 1);
				if (p != ref) {
					if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << ref << std::endl;
					nrOfFailedTestCases++;
				}
			}

			return nrOfFailedTestCases;
		}


		// validate the postfix operator++
		template<size_t nbits, size_t es>
		int ValidatePostfix(std::string tag, bool bReportIndividualTestCases)
		{
			const size_t NrOfReals = (unsigned(1) << nbits);
			std::vector< posit<nbits, es> > set;
			GenerateOrderedPositSet(set);  // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

			int nrOfFailedTestCases = 0;

			posit<nbits, es> p, ref;
			// from -maxpos to maxpos through zero
			for (typename std::vector < posit<nbits, es> >::iterator it = set.begin(); it != set.end() - 1; it++) {
				p = *it;
				p++;
				ref = *(it + 1);
				if (p != ref) {
					if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << ref << std::endl;
					nrOfFailedTestCases++;
				}
			}

			return nrOfFailedTestCases;
		}


		// validate the prefix operator++
		template<size_t nbits, size_t es>
		int ValidatePrefix(std::string tag, bool bReportIndividualTestCases)
		{
			const size_t NrOfReals = (unsigned(1) << nbits);
			std::vector< posit<nbits, es> > set;
			GenerateOrderedPositSet(set);  // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

			int nrOfFailedTestCases = 0;

			posit<nbits, es> p, ref;
			// from -maxpos to maxpos through zero
			for (typename std::vector < posit<nbits, es> >::iterator it = set.begin(); it != set.end() - 1; it++) {
				p = *it;
				++p;
				ref = *(it + 1);
				if (p != ref) {
					if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << ref << std::endl;
					nrOfFailedTestCases++;
				}
			}

			return nrOfFailedTestCases;
		}


		// enumerate all negation cases for a posit configuration: executes within 10 sec till about nbits = 14
		template<size_t nbits, size_t es>
		int ValidateNegation(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pneg, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				pneg = -pa;
				// generate reference
				da = double(pa);
				pref = -da;
				if (pneg != pref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "-", pa, pref, pneg);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "-", pa, pref, pneg);
				}
			}
			return nrOfFailedTests;
		}

		// enumerate all SQRT cases for a posit configuration: executes within 10 sec till about nbits = 14
		template<size_t nbits, size_t es>
		int ValidateSqrt(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, psqrt, pref;

			double da;
			for (int i = 1; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				psqrt = sw::unum::sqrt(pa);
				// generate reference
				da = double(pa);
				pref = std::sqrt(da);
				if (psqrt != pref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "sqrt", pa, pref, psqrt);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "sqrt", pa, pref, psqrt);
				}
			}
			return nrOfFailedTests;
		}

		// enumerate all addition cases for a posit configuration: is within 10sec till about nbits = 14
		template<size_t nbits, size_t es>
		int ValidateAddition(std::string tag, bool bReportIndividualTestCases) {
			const int NR_POSITS = (unsigned(1) << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pb, psum, pref;

			double da, db;
			for (int i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = double(pa);
				for (int j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					db = double(pb);
					psum = pa + pb;
					pref = da + db;
					if (psum != pref) {
						nrOfFailedTests++;
						if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", pa, pb, pref, psum);
					}
					else {
						//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, pref, psum);
					}
				}
			}

			return nrOfFailedTests;
		}

		// enumerate all subtraction cases for a posit configuration: is within 10sec till about nbits = 14
		template<size_t nbits, size_t es>
		int ValidateSubtraction(std::string tag, bool bReportIndividualTestCases) {
			const int NR_POSITS = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pb, pref, pdif;

			double da, db;
			for (int i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = double(pa);
				for (int j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					db = double(pb);
					pdif = pa - pb;
					pref = da - db;
					if (pdif != pref) {
						nrOfFailedTests++;
						if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-", pa, pb, pref, pdif);
					}
					else {
						//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-", pa, pb, pref, pdif);
					}
				}
			}

			return nrOfFailedTests;
		}

		// enumerate all multiplication cases for a posit configuration: is within 10sec till about nbits = 14
		template<size_t nbits, size_t es>
		int ValidateMultiplication(std::string tag, bool bReportIndividualTestCases) {
			int nrOfFailedTests = 0;
			const size_t NR_POSITS = (unsigned(1) << nbits);

			posit<nbits, es> pa, pb, pmul, pref;
			double da, db;
			for (int i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = double(pa);
				for (int j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					db = double(pb);
					pmul = pa * pb;
					pref = da * db;
					if (pmul != pref) {
						if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "*", pa, pb, pref, pmul);
						nrOfFailedTests++;
					}
					else {
						//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", pa, pb, pref, pmul);
					}
				}
			}
			return nrOfFailedTests;
		}

		// enerate all reciprocation cases for a posit configuration: executes within 10 sec till about nbits = 14
		template<size_t nbits, size_t es>
		int ValidateReciprocation(std::string tag, bool bReportIndividualTestCases) {
			const int NR_TEST_CASES = (1 << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, preciprocal, preference;

			double da;
			for (int i = 0; i < NR_TEST_CASES; i++) {
				pa.set_raw_bits(i);
				// generate reference
				if (pa.isNaR()) {
					preference.setToNaR();
				}
				else {
					da = double(pa);
					preference = 1.0 / da;
				}
				preciprocal = pa.reciprocate();

				if (preciprocal != preference) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "reciprocate", pa, preference, preciprocal);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "reciprocate", pa, preference, preciprocal);
				}
			}
			return nrOfFailedTests;
		}

		// enumerate all division cases for a posit configuration: is within 10sec till about nbits = 14
		template<size_t nbits, size_t es>
		int ValidateDivision(std::string tag, bool bReportIndividualTestCases) {
			int nrOfFailedTests = 0;
			const size_t NR_POSITS = (unsigned(1) << nbits);

			posit<nbits, es> pa, pb, pdiv, pref;
			double da, db;
			for (int i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = double(pa);
				for (int j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					if (pb.isNaR()) {
						pref.setToNaR();
					}
					else {
						db = double(pb);
						pref = da / db;
					}
					pdiv = pa / pb;

					if (pdiv != pref) {
						if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
						nrOfFailedTests++;
					}
					else {
						//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/", pa, pb, pref, pdiv);
					}
				}
			}
			return nrOfFailedTests;
		}

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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "log", pa, pref, plog);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "log", pa, pref, plog);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "log2", pa, pref, plog2);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "log2", pa, pref, plog2);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "log10", pa, pref, plog10);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "log10", pa, pref, plog10);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "exp", pa, pref, pexp);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "exp", pa, pref, pexp);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "exp2", pa, pref, pexp2);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "exp2", pa, pref, pexp2);
				}
			}
			return nrOfFailedTests;
		}

		// enumerate all power method cases for a posit configuration
		template<size_t nbits, size_t es>
		int ValidatePowerFunction(std::string tag, bool bReportIndividualTestCases) {
			const int NR_POSITS = (unsigned(1) << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pb, ppow, pref;

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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "sin", pa, pref, psin);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "sin", pa, pref, psin);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "cos", pa, pref, pcos);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "cos", pa, pref, pcos);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "tan", pa, pref, ptan);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "tan", pa, pref, ptan);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "atan", pa, pref, patan);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "atan", pa, pref, patan);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "asin", pa, pref, pasin);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "asin", pa, pref, pasin);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "acos", pa, pref, pacos);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "acos", pa, pref, pacos);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "sinh", pa, pref, psinh);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "sinh", pa, pref, psinh);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "cosh", pa, pref, pcosh);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "cosh", pa, pref, pcosh);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "tanh", pa, pref, ptanh);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "tanh", pa, pref, ptanh);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "atanh", pa, pref, patanh);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "atanh", pa, pref, patanh);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "asinh", pa, pref, pasinh);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "asinh", pa, pref, pasinh);
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
					if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "acosh", pa, pref, pacosh);
				}
				else {
					//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "acosh", pa, pref, pacosh);
				}
			}
			return nrOfFailedTests;
		}

		//////////////////////////////////// RANDOMIZED TEST SUITE FOR BINARY OPERATORS ////////////////////////

		// for testing posit configs that are > 14-15, we need a more efficient approach.
		// One simple, brute force approach is to generate randoms.
		// A more white box approach is to focus on the testcases 
		// where something special happens in the posit arithmetic, such as rounding.

		// operation opcodes
		const int OPCODE_NOP = 0;
		const int OPCODE_ADD = 1;
		const int OPCODE_SUB = 2;
		const int OPCODE_MUL = 3;
		const int OPCODE_DIV = 4;
		const int OPCODE_RAN = 5;

		template<size_t nbits, size_t es>
		void execute(int opcode, double da, double db, const posit<nbits, es>& pa, const posit<nbits, es>& pb, posit<nbits, es>& preference, posit<nbits, es>& presult) {
			double reference;
			switch (opcode) {
			default:
			case OPCODE_NOP:
				preference.setToZero();
				presult.setToZero();
				return;
			case OPCODE_ADD:
				presult = pa + pb;
				reference = da + db;
				break;
			case OPCODE_SUB:
				presult = pa - pb;
				reference = da - db;
				break;
			case OPCODE_MUL:
				presult = pa * pb;
				reference = da * db;
				break;
			case OPCODE_DIV:
				presult = pa / pb;
				reference = da / db;
				break;
			}
			preference = reference;
		}

		// generate a random set of operands to test the binary operators for a posit configuration
		// Basic design is that we generate nrOfRandom posit values and store them in an operand array.
		// We will then execute the binary operator nrOfRandom combinations.
		template<size_t nbits, size_t es>
		int ValidateThroughRandoms(std::string tag, bool bReportIndividualTestCases, int opcode, uint32_t nrOfRandoms) {
			const size_t SIZE_STATE_SPACE = nrOfRandoms;
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pb, presult, preference;

			std::string operation_string;
			switch (opcode) {
			default:
			case OPCODE_NOP:
				operation_string = "nop";
				break;
			case OPCODE_ADD:
				operation_string = "+";
				break;
			case OPCODE_SUB:
				operation_string = "-";
				break;
			case OPCODE_MUL:
				operation_string = "*";
				break;
			case OPCODE_DIV:
				operation_string = "/";
				break;
			}
			// generate the full state space set of valid posit values
			std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
			std::mt19937_64 eng(rd()); //Use the 64-bit Mersenne Twister 19937 generator and seed it with entropy.
									   //Define the distribution, by default it goes from 0 to MAX(unsigned long long)
			std::uniform_int_distribution<unsigned long long> distr;
#ifdef POSIT_USE_LONG_DOUBLE
			std::vector<long double> operand_values(SIZE_STATE_SPACE);
			for (uint32_t i = 0; i < SIZE_STATE_SPACE; i++) {
				presult.set_raw_bits(distr(eng));  // take the bottom nbits bits as posit encoding
				operand_values[i] = (long double)(presult);
			}
			long double da, db;
#else // USE DOUBLE
			std::vector<double> operand_values(SIZE_STATE_SPACE);
			for (uint32_t i = 0; i < SIZE_STATE_SPACE; i++) {
				presult.set_raw_bits(distr(eng));  // take the bottom nbits bits as posit encoding
				operand_values[i] = double(presult);
			}
			double da, db;
#endif // POSIT_USE_LONG_DOUBLE
			unsigned ia, ib;  // random indices for picking operands to test
			for (unsigned i = 1; i < nrOfRandoms; i++) {
				ia = std::rand() % SIZE_STATE_SPACE;
				da = operand_values[ia];
				pa = da;
				ib = std::rand() % SIZE_STATE_SPACE;
				db = operand_values[ib];
				pb = db;
				// in case you have numeric_limits<long double>::digits trouble... this will show that
				//std::cout << "sizeof da: " << sizeof(da) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value da " << da << " at index " << ia << " pa " << pa << std::endl;
				//std::cout << "sizeof db: " << sizeof(db) << " bits in significant " << (std::numeric_limits<long double>::digits - 1) << " value db " << db << " at index " << ia << " pa " << pb << std::endl;
				execute(opcode, da, db, pa, pb, preference, presult);
				if (presult != preference) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases) ReportBinaryArithmeticErrorInBinary("FAIL", operation_string, pa, pb, preference, presult);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccessInBinary("PASS", operation_string, pa, pb, preference, presult);
				}
			}

			return nrOfFailedTests;
		}


	}; // namespace unum

}; // namespace sw

