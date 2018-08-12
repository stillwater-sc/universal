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

namespace sw {
	namespace unum {

		static constexpr unsigned FLOAT_TABLE_WIDTH = 15;

		template<size_t nbits, size_t es>
		void ReportConversionError(std::string test_case, std::string op, double input, double reference, const posit<nbits, es>& presult) {
			static_assert(nbits > 2, "component_to_string requires nbits > 2");
			constexpr size_t fbits = nbits - 3 - es;

			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(presult.get(), _sign, _regime, _exponent, _fraction);
			int                  _scale = _regime.scale() + _exponent.scale();

			std::cerr << test_case
				<< " " << op << " "
				<< std::setw(FLOAT_TABLE_WIDTH) << input
				<< " did not convert to "
				<< std::setw(FLOAT_TABLE_WIDTH) << reference << " instead it yielded "
				<< std::setw(FLOAT_TABLE_WIDTH) << double(presult)
				<< "  raw " << std::setw(nbits) << presult.get()
				<< "   scale= " << std::setw(3) << _scale << "   k= " << std::setw(3) << _regime.regime_k() << "   exp= " << std::setw(3) << _exponent.scale()
				<< std::endl;
		}

		template<size_t nbits, size_t es>
		void ReportConversionSuccess(std::string test_case, std::string op, double input, double reference, const posit<nbits, es>& presult) {
			static_assert(nbits > 2, "component_to_string requires nbits > 2");
			constexpr size_t fbits = nbits - 3 - es;

			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			decode(presult.get(), _sign, _regime, _exponent, _fraction);
			int                  _scale = _regime.scale() + _exponent.scale();

			std::cerr << test_case
				<< " " << op << " "
				<< std::setw(FLOAT_TABLE_WIDTH) << input
				<< " did     convert to "
				<< std::setw(FLOAT_TABLE_WIDTH) << double(presult) << " reference value is "
				<< std::setw(FLOAT_TABLE_WIDTH) << reference
				<< "  raw " << std::setw(nbits) << presult.get()
				<< "   scale= " << std::setw(3) << _scale << "   k= " << std::setw(3) << _regime.regime_k() << "   exp= " << std::setw(3) << _exponent.scale()
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
			// These larger posits will be at the mid-point between the smaller posit sample values
			// and we'll enumerate the exact value, and a perturbation smaller and a perturbation larger
			// to test the rounding logic of the conversion.
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
						// special case of assigning to 0
						input = 0.0;
						pa = input;
						nrOfFailedTests += Compare(input, pa, da, bReportIndividualTestCases);
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
			for (size_t i = 0; i < NR_OF_REALS; i++) {
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
			constexpr size_t NR_TEST_CASES = (size_t(1) << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa(0), pneg(0), pref(0);

			double da;
			for (size_t i = 1; i < NR_TEST_CASES; i++) {
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
			constexpr size_t NR_TEST_CASES = (size_t(1) << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, psqrt, pref;

			double da;
			for (size_t i = 1; i < NR_TEST_CASES; i++) {
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
			const size_t NR_POSITS = (size_t(1) << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pb, psum, pref;

			double da, db;
			for (size_t i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = double(pa);
				for (size_t j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					db = double(pb);
					pref = da + db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
					try {
						psum = pa + pb;
					}
					catch (const operand_is_nar& err) {
						if (pa.isNaR() || pb.isNaR()) {
							// correctly caught the exception
							psum.setToNaR();
						}
						else {
							throw err;
						}
					}

#else
					psum = pa + pb;
#endif
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
			const size_t NR_POSITS = (size_t(1) << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, pb, pref, pdif;

			double da, db;
			for (size_t i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = double(pa);
				for (size_t j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					db = double(pb);
					pref = da - db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
					try {
						pdif = pa - pb;
					}
					catch (const operand_is_nar& err) {
						if (pa.isNaR() || pb.isNaR()) {
							// correctly caught the exception
							pdif.setToNaR();
						}
						else {
							throw err;
						}
					}
#else
					pdif = pa - pb;
#endif
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
			const size_t NR_POSITS = (size_t(1) << nbits);

			posit<nbits, es> pa, pb, pmul, pref;
			double da, db;
			for (size_t i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = double(pa);
				for (size_t j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					db = double(pb);
					pref = da * db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
					try {
						pmul = pa * pb;
					}
					catch (const operand_is_nar& err) {
						if (pa.isNaR() || pb.isNaR()) {
							// correctly caught the exception
							pmul.setToNaR();
						}
						else {
							throw err;
						}
					}
#else
					pmul = pa * pb;
#endif
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
			const size_t NR_TEST_CASES = (size_t(1) << nbits);
			int nrOfFailedTests = 0;
			posit<nbits, es> pa, preciprocal, preference;

			double da;
			for (size_t i = 0; i < NR_TEST_CASES; i++) {
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
			const size_t NR_POSITS = (size_t(1) << nbits);

			posit<nbits, es> pa, pb, pdiv, pref;
			double da, db;
			for (size_t i = 0; i < NR_POSITS; i++) {
				pa.set_raw_bits(i);
				da = double(pa);
				for (size_t j = 0; j < NR_POSITS; j++) {
					pb.set_raw_bits(j);
					db = double(pb);
					if (pb.isNaR()) {
						pref.setToNaR();
					}
					else {
						pref = da / db;
					}
#if POSIT_THROW_ARITHMETIC_EXCEPTION
					try {
						pdiv = pa / pb;
					}
					catch (const divide_by_zero& err) {
						if (pb.isZero()) {
							// correctly caught the divide by zero condition
							continue;
							//pdiv.setToNaR();
						}
						else {
							if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
							throw err; // rethrow
						}
					}
					catch (const divide_by_nar& err) {
						if (pb.isNaR()) {
							// correctly caught the divide by nar condition
							continue;
							//pdiv = 0.0f;
						}
						else {
							if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
							throw err; // rethrow
						}
					}
					catch (const numerator_is_nar& err) {
						if (pa.isNaR()) {
							// correctly caught the numerator is nar condition
							continue;
							//pdiv.setToNaR();
						}
						else {
							if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
							throw err; // rethrow
						}
					}
#else
					pdiv = pa / pb;
#endif
					// check against the IEEE reference
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
			double reference = 0.0;
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
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					execute(opcode, da, db, pa, pb, preference, presult);
				}
				catch (const posit_arithmetic_exception& err) {
					if (pa.isNaR() || pb.isNaR() || (opcode == OPCODE_DIV && pb.isZero())) {
						std::cerr << "Correctly caught arithmetic exception: " << err.what() << std::endl;
					}
					else {
						throw err;
					}
				}
#else
				execute(opcode, da, db, pa, pb, preference, presult);
#endif
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


	} // namespace unum

} // namespace sw

