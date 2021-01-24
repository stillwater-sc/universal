#pragma once
// posit_test_suite.hpp : posit number system verification test suite
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>
#include <universal/verification/test_status.hpp> // ReportTestResult used by test suite runner
#include <universal/verification/test_reporters.hpp> 

namespace sw::universal {

#if 0
	static constexpr unsigned FLOAT_TABLE_WIDTH = 15;

	template<size_t nbits, size_t es>
	void ReportConversionError(const std::string& test_case, const std::string& op, double input, double reference, const posit<nbits, es>& presult) {
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

	template<>
	void ReportConversionError<2,0>(const std::string& test_case, const std::string& op, double input, double reference, const posit<2, 0>& presult) {
		constexpr size_t nbits = 2;
		//constexpr size_t es = 0;
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(FLOAT_TABLE_WIDTH) << input
			<< " did not convert to "
			<< std::setw(FLOAT_TABLE_WIDTH) << reference << " instead it yielded "
			<< std::setw(FLOAT_TABLE_WIDTH) << double(presult)
			<< "  raw " << std::setw(nbits) << presult.get()
//						<< "   scale= " << std::setw(3) << _scale << "   k= " << std::setw(3) << _regime.regime_k() << "   exp= " << std::setw(3) << _exponent.scale()
			<< std::endl;

	}
	template<>
	void ReportConversionError<3, 1>(const std::string& test_case, const std::string& op, double input, double reference, const posit<3, 1>& presult) {
		constexpr size_t nbits = 3;
		//constexpr size_t es = 1; 
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(FLOAT_TABLE_WIDTH) << input
			<< " did not convert to "
			<< std::setw(FLOAT_TABLE_WIDTH) << reference << " instead it yielded "
			<< std::setw(FLOAT_TABLE_WIDTH) << double(presult)
			<< "  raw " << std::setw(nbits) << presult.get()
			//						<< "   scale= " << std::setw(3) << _scale << "   k= " << std::setw(3) << _regime.regime_k() << "   exp= " << std::setw(3) << _exponent.scale()
			<< std::endl;

	}

	template<size_t nbits, size_t es>
	void ReportConversionSuccess(const std::string& test_case, const std::string& op, double input, double reference, const posit<nbits, es>& presult) {
		static_assert(nbits > 1, "component_to_string requires nbits >= 2");
		if (nbits > 2) {
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
		else {
			if (nbits == 2) {
				std::cerr << test_case
					<< " " << op << " "
					<< std::setw(FLOAT_TABLE_WIDTH) << input
					<< " did     convert to "
					<< std::setw(FLOAT_TABLE_WIDTH) << double(presult) << " reference value is "
					<< std::setw(FLOAT_TABLE_WIDTH) << reference
					<< "  raw " << std::setw(nbits) << presult.get()
//						<< "   scale= " << std::setw(3) << _scale << "   k= " << std::setw(3) << _regime.regime_k() << "   exp= " << std::setw(3) << _exponent.scale()
					<< std::endl;
			}
		}
	}

	template<size_t nbits, size_t es>
	void ReportUnaryArithmeticError(const std::string& test_case, const std::string& op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(FLOAT_TABLE_WIDTH) << rhs
			<< " != "
			<< std::setw(FLOAT_TABLE_WIDTH) << pref << " instead it yielded "
			<< std::setw(FLOAT_TABLE_WIDTH) << presult
			<< " " << pref.get() << " vs " << presult.get() << std::endl;
	}

	template<size_t nbits, size_t es>
	void ReportUnaryArithmeticSuccess(const std::string& test_case, const std::string& op, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
		std::cerr << test_case
			<< " " << op << " "
			<< std::setw(FLOAT_TABLE_WIDTH) << rhs
			<< " == "
			<< std::setw(FLOAT_TABLE_WIDTH) << presult << " reference value is "
			<< std::setw(FLOAT_TABLE_WIDTH) << pref
			<< " " << pretty_print(presult) << std::endl;
	}

	template<size_t nbits, size_t es>
	void ReportBinaryArithmeticError(const std::string& test_case, const std::string& op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
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
	void ReportBinaryArithmeticErrorInBinary(const std::string& test_case, const std::string& op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
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
	void ReportBinaryArithmeticSuccess(const std::string& test_case, const std::string& op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
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
	void ReportBinaryArithmeticSuccessInBinary(const std::string& test_case, const std::string& op, const posit<nbits, es>& lhs, const posit<nbits, es>& rhs, const posit<nbits, es>& pref, const posit<nbits, es>& presult) {
		std::cerr << test_case << " "
			<< std::setw(nbits) << lhs.get()
			<< " " << op << " "
			<< std::setw(nbits) << rhs.get()
			<< " == "
			<< std::setw(nbits) << presult.get() << " reference value is "
			<< std::setw(nbits) << pref.get()
			<< " " << pretty_print(presult,20) << std::endl;
	}
#endif

	template<size_t nbits, size_t es>
	void ReportDecodeError(const std::string& test_case, const posit<nbits, es>& actual, double golden_value) {
		std::cerr << test_case << " actual " << actual << " required " << golden_value << std::endl;
	}

	/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

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

	// logic operator consistency check
	template<size_t nbits, size_t es>
	void testLogicOperators(const posit<nbits, es>& a, const posit<nbits, es>& b) {
		using namespace std;
		cout << a << " vs " << b << endl;
		if (a == b) cout << "a == b\n"; else cout << "a != b\n";
		if (a != b) cout << "a != b\n"; else cout << "a == b\n";
		if (a < b)  cout << "a <  b\n"; else cout << "a >= b\n";
		if (a <= b) cout << "a <= b\n"; else cout << "a >  b\n";
		if (a > b)  cout << "a >  b\n"; else cout << "a <= b\n";
		if (a >= b) cout << "a >= b\n"; else cout << "a <  b\n";
	}

	// enumerate all conversion cases for a posit configuration
	template<size_t nbits, size_t es>
	int VerifyConversion(const std::string& tag, bool bReportIndividualTestCases) {
		// we are going to generate a test set that consists of all posit configs and their midpoints
		// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
		// These larger posits will be at the mid-point between the smaller posit sample values
		// and we'll enumerate the exact value, and a perturbation smaller and a perturbation larger
		// to test the rounding logic of the conversion.
		constexpr size_t max = nbits > 20 ? 20 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << (max + 1));
		size_t HALF = (size_t(1) << max);

		if (nbits > 20) {
			std::cout << "VerifyConversion<" << nbits << "," << es << ">: NR_TEST_CASES = " << NR_TEST_CASES << " constrained due to nbits > 20" << std::endl;
		}

		// execute the test
		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits + 1, es> pref, pprev, pnext;

			pref.set_raw_bits(i);
			double da = double(pref);
			double eps = (i == 0) ? minpos_value<nbits + 1, es>() / 2.0 : (da > 0 ? da * 1.0e-6 : da * -1.0e-6);
			double input;
			posit<nbits, es> pa;
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
	template<>
	int VerifyConversion<NBITS_IS_2, ES_IS_0>(const std::string& tag, bool bReportIndividualTestCases) {
		int nrOfFailedTestCases = 0;
		// special case
		posit<NBITS_IS_2, ES_IS_0> p = -INFINITY;
		if (!isnar(p)) nrOfFailedTestCases++;
		// test vector
		std::vector<double> in  = { -4.0, -2.0, -1.0, -0.5, -0.25, 0.0, 0.25, 0.5, 1.0, 2.0, 4.0 };
		std::vector<double> ref = { -1.0, -1.0, -1.0, -1.0, -1.00, 0.0, 1.00, 1.0, 1.0, 1.0, 1.0 };
		size_t ref_index = 0;
		for (auto v : in) {
			p = v;
			double refv = ref[ref_index++];
			if (double(p) != refv) {
				if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << refv << std::endl;
				nrOfFailedTestCases++;
			}
			else {
				//if (bReportIndividualTestCases) std::cout << tag << " PASS " << p << " == " << refv << std::endl;
			}
		}
		return nrOfFailedTestCases;
	}

	// enumerate all conversion cases for integers
	template<size_t nbits, size_t es>
	int VerifyIntegerConversion(const std::string& tag, bool bReportIndividualTestCases) {
		// we generate numbers from 1 via NaR to -1 and through the special case of 0 back to 1
		constexpr unsigned max = nbits > 20 ? 20 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << (max - 1)) + 1;  
		int nrOfFailedTestCases = 0;		
		// special cases in case we are clipped by the nbits > 20
		long ref = 0x80000000;  // -2147483648
		posit<nbits, es> presult(ref);
		if (ref != presult) {
			std::cout << tag << " FAIL long(" << ref << ") != long(" << presult << ") : reference = -2147483648" << std::endl;
			nrOfFailedTestCases++;
		}
		posit<nbits, es> p(1);
		for (size_t i = 0; i < NR_TEST_CASES; ++i) {
			if (!p.isnar()) {
				long ref = (long)p; // obtain the integer cast of this posit
				presult = ref;		// assign this integer to a posit				
				if (ref != presult) { // compare the integer cast to the reference posit
					if (bReportIndividualTestCases) std::cout << tag << " FAIL long(" << p << ") != long(" << presult << ") : reference = " << ref << std::endl;
					nrOfFailedTestCases++;
				}
				else {
					//if (bReportIndividualTestCases) std::cout << tag << " PASS " << p << " casts to " << presult << " : reference = " << ref << std::endl;
				}
			}
			++p;
		}
		return nrOfFailedTestCases;
	}
/*
	// specialized template for fast posit<2,0>
	template<>
	int VerifyIntegerConversion<NBITS_IS_2, ES_IS_0>(const std::string& tag, bool bReportIndividualTestCases) {
		std::vector<int> in = { -4, -3, -2, -1, 0, 1, 2, 3, 4 };
		std::vector<int> ref = { -1, -1, -1, -1, 0, 1, 1, 1, 1 };
		int nrOfFailedTestCases = 0;
		size_t ref_index = 0;
		for (auto v : in) {
			posit<NBITS_IS_2, ES_IS_0> p;
			p = v;
			int refv = ref[ref_index++];
			if (p != refv) {
				if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << refv << std::endl;
				nrOfFailedTestCases++;
			}
			else {
				//if (bReportIndividualTestCases) std::cout << tag << " PASS " << p << " == " << refv << std::endl;
			}
		}
		return nrOfFailedTestCases;
	}
	// specialized template for fast posit<3,0>
	template<>
	int VerifyIntegerConversion<NBITS_IS_3, ES_IS_0>(const std::string& tag, bool bReportIndividualTestCases) {
		std::vector<int> in = { -4, -3, -2, -1, 0, 1, 2, 3, 4 };
		std::vector<int> ref = { -2, -2, -2, -1, 0, 1, 2, 2, 2 };
		int nrOfFailedTestCases = 0;
		size_t ref_index = 0;
		for (auto v : in) {
			posit<NBITS_IS_3, ES_IS_0> p;
			p = v;
			int refv = ref[ref_index++];
			if (p != refv) {
				if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << refv << std::endl;
				nrOfFailedTestCases++;
			}
			else {
				//if (bReportIndividualTestCases) std::cout << tag << " PASS " << p << " == " << refv << std::endl;
			}
		}
		return nrOfFailedTestCases;
	}
*/

// enumerate all conversion cases for integers
	template<size_t nbits, size_t es>
	int VerifyUintConversion(std::string& tag, bool bReportIndividualTestCases) {
		// we generate numbers from 1 via NaR to -1 and through the special case of 0 back to 1
		constexpr unsigned max = nbits > 20 ? 20 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << (max - 1)) + 1;
		int nrOfFailedTestCases = 0;

		posit<nbits, es> p, presult;

		if (nbits > 24) {
			// cycle from largest value down to 0 via positive regime
			constexpr unsigned long upper_bound = 0xFFFFFFFF;
			p = upper_bound;
			for (unsigned long i = upper_bound; i > upper_bound - (unsigned long)(NR_TEST_CASES); --i) {
				unsigned long ref = (unsigned long)p;   // obtain the integer cast of this posit
				presult = ref;		  // assign this integer to a reference posit
				if (presult != ref) { // compare the integer cast to the reference posit
					if (bReportIndividualTestCases) std::cout << tag << " FAIL uint32(" << p << ") != uint32(" << presult << ") : reference = " << ref << std::endl;
					nrOfFailedTestCases++;
				}
				else {
					//if (bReportIndividualTestCases) std::cout << tag << " PASS " << p << " == " << presult << " : reference = " << ref << std::endl;
				}
				--p;
			}
		}
		else {
			p = 1;
			if (!p.isone()) {
				if (bReportIndividualTestCases) std::cout << tag << " FAIL " << p << " != " << 1 << std::endl;
				nrOfFailedTestCases++;
			}
			for (size_t i = 0; i < NR_TEST_CASES; ++i) {
				if (!p.isnar()) {
					unsigned long ref = (unsigned long)p;   // obtain the integer cast of this posit
					presult = ref;		  // assign this integer to a reference posit
					if (presult != ref) { // compare the integer cast to the reference posit
						if (bReportIndividualTestCases) std::cout << tag << " FAIL uint32(" << p << ") != uint32(" << presult << ") : reference = " << ref << std::endl;
						nrOfFailedTestCases++;
					}
					else {
						//if (bReportIndividualTestCases) std::cout << tag << " PASS " << p << " == " << presult << " : reference = " << ref << std::endl;
					}
				}
				++p;
			}
		}
		return nrOfFailedTestCases;
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

	// Verify the increment operator++
	template<size_t nbits, size_t es>
	int VerifyIncrement(const std::string& tag, bool bReportIndividualTestCases)	{
		std::vector< posit<nbits, es> > set;
		GenerateOrderedPositSet(set); // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

		int nrOfFailedTestCases = 0;

		posit<nbits, es> p, ref;
		// starting from NaR iterating from -maxpos to maxpos through zero
		for (typename std::vector < posit<nbits, es> >::iterator it = set.begin(); it != set.end() - 1; ++it) {
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

	// Verify the decrement operator--
	template<size_t nbits, size_t es>
	int VerifyDecrement(const std::string& tag, bool bReportIndividualTestCases)
	{
		std::vector< posit<nbits, es> > set;
		GenerateOrderedPositSet(set); // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

		int nrOfFailedTestCases = 0;

		posit<nbits, es> p, ref;
		// starting from maxpos iterating to -maxpos, and finally NaR via zero
		for (typename std::vector < posit<nbits, es> >::iterator it = set.end() - 1; it != set.begin(); --it) {
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

	// Verify the postfix operator++
	template<size_t nbits, size_t es>
	int VerifyPostfix(const std::string& tag, bool bReportIndividualTestCases)
	{
		std::vector< posit<nbits, es> > set;
		GenerateOrderedPositSet(set);  // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

		int nrOfFailedTestCases = 0;

		posit<nbits, es> p, ref;
		// from -maxpos to maxpos through zero
		for (typename std::vector < posit<nbits, es> >::iterator it = set.begin(); it != set.end() - 1; ++it) {
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

	// Verify the prefix operator++
	template<size_t nbits, size_t es>
	int VerifyPrefix(const std::string& tag, bool bReportIndividualTestCases)
	{
		std::vector< posit<nbits, es> > set;
		GenerateOrderedPositSet(set);  // [NaR, -maxpos, ..., -minpos, 0, minpos, ..., maxpos]

		int nrOfFailedTestCases = 0;

		posit<nbits, es> p, ref;
		// from -maxpos to maxpos through zero
		for (typename std::vector < posit<nbits, es> >::iterator it = set.begin(); it != set.end() - 1; ++it) {
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
	int VerifyNegation(const std::string& tag, bool bReportIndividualTestCases) {
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
	int VerifySqrt(const std::string& tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_TEST_CASES = (size_t(1) << nbits);
		int nrOfFailedTests = 0;

		for (size_t i = 1; i < NR_TEST_CASES; i++) {
			posit<nbits, es> pa, psqrt, pref;
			pa.set_raw_bits(i);
			psqrt = sw::universal::sqrt(pa);
			// generate reference
			double da = double(pa);
			pref = std::sqrt(da);
			if (psqrt != pref) {
				nrOfFailedTests++;
				std::cout << psqrt << " != " << pref << std::endl;
				if (bReportIndividualTestCases)	ReportUnaryArithmeticError("FAIL", "sqrt", pa, pref, psqrt);
				if (nrOfFailedTests > 24) return nrOfFailedTests;
			}
			else {
				//if (bReportIndividualTestCases) ReportUnaryArithmeticSuccess("PASS", "sqrt", pa, pref, psqrt);
			}
		}
		return nrOfFailedTests;
	}

	// enumerate all addition cases for a posit configuration
	template<size_t nbits, size_t es>
	int VerifyAddition(const std::string& tag, bool bReportIndividualTestCases) {
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
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						psum.setnar();
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

	// enumerate all addition cases for a posit configuration
	template<size_t nbits, size_t es>
	int VerifyInPlaceAddition(const std::string& tag, bool bReportIndividualTestCases) {
		const size_t NR_POSITS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.set_raw_bits(i);
			double da = double(pa);
			for (size_t j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, psum, pref;
				pb.set_raw_bits(j);
				double db = double(pb);
				pref = da + db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					psum = pa;
					psum += pb;
				}
				catch (const operand_is_nar& err) {
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						psum.setnar();
					}
					else {
						throw; // rethrow
					}
				}

#else
				psum = pa;
				psum += pb;
#endif
				if (psum != pref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+=", pa, pb, pref, psum);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+=", pa, pb, pref, psum);
				}
			}
		}

		return nrOfFailedTests;
	}

	// enumerate all subtraction cases for a posit configuration: is within 10sec till about nbits = 14
	template<size_t nbits, size_t es>
	int VerifySubtraction(const std::string& tag, bool bReportIndividualTestCases) {
		const size_t NR_POSITS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.set_raw_bits(i);
			double da = double(pa);
			for (size_t j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pref, pdif;

				pb.set_raw_bits(j);
				double db = double(pb);
				pref = da - db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pdif = pa - pb;
				}
				catch (const operand_is_nar&) {
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						pdif.setnar();
					}
					else {
						throw; // rethrow
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

	// enumerate all subtraction cases for a posit configuration: is within 10sec till about nbits = 14
	template<size_t nbits, size_t es>
	int VerifyInPlaceSubtraction(const std::string& tag, bool bReportIndividualTestCases) {
		const size_t NR_POSITS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.set_raw_bits(i);
			double da = double(pa);
			for (size_t j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pref, pdif;
				pb.set_raw_bits(j);
				double db = double(pb);
				pref = da - db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pdif = pa;
					pdif -= pb;
				}
				catch (const operand_is_nar& err) {
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						pdif.setnar();
					}
					else {
						throw;  // rethrow
					}
				}
#else
				pdif = pa;
				pdif -= pb;
#endif
				if (pdif != pref) {
					nrOfFailedTests++;
					if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "-=", pa, pb, pref, pdif);
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "-=", pa, pb, pref, pdif);
				}
			}
		}

		return nrOfFailedTests;
	}

	// enumerate all multiplication cases for a posit configuration: is within 10sec till about nbits = 14
	template<size_t nbits, size_t es>
	int VerifyMultiplication(const std::string& tag, bool bReportIndividualTestCases) {
		int nrOfFailedTests = 0;
		const size_t NR_POSITS = (size_t(1) << nbits);
		for (size_t i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.set_raw_bits(i);
			double da = double(pa);
			for (size_t j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pmul, pref;
				pb.set_raw_bits(j);
				double db = double(pb);
				pref = da * db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pmul = pa * pb;
				}
				catch (const operand_is_nar&) {
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						pmul.setnar();
					}
					else {
						throw;  // rethrow
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

	// enumerate all multiplication cases for a posit configuration: is within 10sec till about nbits = 14
	template<size_t nbits, size_t es>
	int VerifyInPlaceMultiplication(const std::string& tag, bool bReportIndividualTestCases) {
		int nrOfFailedTests = 0;
		const size_t NR_POSITS = (size_t(1) << nbits);
		for (size_t i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.set_raw_bits(i);
			double da = double(pa);
			for (size_t j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pmul, pref;
				pb.set_raw_bits(j);
				double db = double(pb);
				pref = da * db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pmul = pa;
					pmul *= pb;
				}
				catch (const operand_is_nar& err) {
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						pmul.setnar();
					}
					else {
						throw err;
					}
				}
#else
				pmul = pa;
				pmul *= pb;
#endif
				if (pmul != pref) {
					if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "*=", pa, pb, pref, pmul);
					nrOfFailedTests++;
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*=", pa, pb, pref, pmul);
				}
			}
		}
		return nrOfFailedTests;
	}

	// enerate all reciprocation cases for a posit configuration: executes within 10 sec till about nbits = 14
	template<size_t nbits, size_t es>
	int VerifyReciprocation(const std::string& tag, bool bReportIndividualTestCases) {
		const size_t NR_TEST_CASES = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> pa, preciprocal, preference;
			pa.set_raw_bits(i);
			// generate reference
			double da;
			if (pa.isnar()) {
				preference.setnar();
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
	int VerifyDivision(const std::string& tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_POSITS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.set_raw_bits(i);
			double da = double(pa);
			for (size_t j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pdiv, pref;
				pb.set_raw_bits(j);
				double db = double(pb);
				if (pb.isnar()) {
					pref.setnar();
				}
				else {
					pref = da / db;
				}
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pdiv = pa / pb;
				}
				catch (const divide_by_zero&) {
					if (pb.iszero()) {
						// correctly caught the divide by zero condition
						continue;
						//pdiv.setnar();
					}
					else {
						if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
						throw; // rethrow
					}
				}
				catch (const divide_by_nar&) {
					if (pb.isnar()) {
						// correctly caught the divide by nar condition
						continue;
						//pdiv = 0.0f;
					}
					else {
						if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
						throw; // rethrow
					}
				}
				catch (const numerator_is_nar&) {
					if (pa.isnar()) {
						// correctly caught the numerator is nar condition
						continue;
						//pdiv.setnar();
					}
					else {
						if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
						throw; // rethrow
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

	// enumerate all division cases for a posit configuration: is within 10sec till about nbits = 14
	template<size_t nbits, size_t es>
	int VerifyInPlaceDivision(const std::string& tag, bool bReportIndividualTestCases) {
		constexpr size_t NR_POSITS = (size_t(1) << nbits);
		int nrOfFailedTests = 0;
		for (size_t i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.set_raw_bits(i);
			double da = double(pa);
			for (size_t j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pdiv, pref;
				pb.set_raw_bits(j);
				double db = double(pb);
				if (pb.isnar()) {
					pref.setnar();
				}
				else {
					pref = da / db;
				}
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pdiv = pa;
					pdiv /= pb;
				}
				catch (const divide_by_zero& err) {
					if (pb.iszero()) {
						// correctly caught the divide by zero condition
						continue;
						//pdiv.setnar();
					}
					else {
						if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
						throw err; // rethrow
					}
				}
				catch (const divide_by_nar& err) {
					if (pb.isnar()) {
						// correctly caught the divide by nar condition
						continue;
						//pdiv = 0.0f;
					}
					else {
						if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pref, pdiv);
						throw; // rethrow
					}
				}
				catch (const numerator_is_nar& err) {
					if (pa.isnar()) {
						// correctly caught the numerator is nar condition
						continue;
						//pdiv.setnar();
					}
					else {
						if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/=", pa, pb, pref, pdiv);
						throw; // rethrow
					}
				}
#else
				pdiv = pa;
				pdiv /= pb;
#endif
				// check against the IEEE reference
				if (pdiv != pref) {
					if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "/=", pa, pb, pref, pdiv);
					nrOfFailedTests++;
				}
				else {
					//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "/=", pa, pb, pref, pdiv);
				}

			}
		}
		return nrOfFailedTests;
	}

	// Posit equal diverges from IEEE float in dealing with INFINITY/NAN
	// Posit NaR can be checked for equality/inequality
	template<size_t nbits, size_t es>
	int VerifyPositLogicEqual() {
		constexpr size_t max = nbits > 12 ? 12 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.set_raw_bits(j);
				// set the golden reference
				bool ref;
				if (a.isnar() && b.isnar()) {
					// special case of posit equality
					ref = true;
				}
				else {
					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Posit = true
					//	NaN == real : IEEE = true    Posit = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Posit = true
					//	NaN == real : IEEE = false    Posit = false
					// and thus we can't relay on IEEE float as reference

					// instead, use the bit pattern as reference
					ref = (i == j);
				}

				bool presult = (a == b);
				if (ref != presult) {
					nrOfFailedTestCases++;
					std::cout << a << " == " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit not-equal diverges from IEEE float in dealing with INFINITY/NAN
	// Posit NaR can be checked for equality/inequality
	template<size_t nbits, size_t es>
	int VerifyPositLogicNotEqual() {
		constexpr size_t max = nbits > 12 ? 12 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.set_raw_bits(j);

				// set the golden reference
				bool ref;
				if (a.isnar() && b.isnar()) {
					// special case of posit equality
					ref = false;
				}
				else {
					// initially, we thought this would be the same behavior as IEEE floats
					// ref = double(a) == double(b);
					// but we have found that some compilers (MSVC) take liberty with NaN
					// \fp:fast		floating point model set to fast
					//	NaN == NaN  : IEEE = true    Posit = true
					//	NaN == real : IEEE = true    Posit = false
					// \fp:strict	floating point model set to strict
					//	NaN == NaN  : IEEE = false    Posit = true
					//	NaN == real : IEEE = false    Posit = false
					// and thus we can't rely on IEEE float as reference

					// instead, use the bit pattern as reference
					ref = (i != j);
				}

				bool presult = (a != b);
				if (ref != presult) {
					nrOfFailedTestCases++;
					std::cout << a << " != " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit less-than diverges from IEEE float in dealing with INFINITY/NAN
	// Posit NaR is smaller than any other value
	template<size_t nbits, size_t es>
	int VerifyPositLogicLessThan() {
		constexpr size_t max = nbits > 10 ? 10 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.set_raw_bits(j);

				// generate the golden reference
				bool ref;
				if (a.isnar() && !b.isnar()) {
					// special case of posit NaR
					ref = true;
				}
				else if (b.isnar()) {
					ref = false;  // everything is less than NaR
				}
				else {
					// same behavior as IEEE floats
					ref = (double(a) < double(b));
				}
				bool presult = (a < b);
				if (ref != presult) {
					nrOfFailedTestCases++;
					std::cout << a << " < " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit greater-than diverges from IEEE float in dealing with INFINITY/NAN
	// Any number is greater-than posit NaR
	template<size_t nbits, size_t es>
	int VerifyPositLogicGreaterThan() {
		constexpr size_t max = nbits > 10 ? 10 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.set_raw_bits(j);

				// generate the golden reference
				bool ref = (double(a) > double(b)); // same behavior as IEEE floats 
				if (!a.isnar() && b.isnar()) {					
					ref = true; // special case of posit NaR
				}
				bool presult = (a > b);
				if (ref != presult) {
					nrOfFailedTestCases++;
					std::cout << a << " > " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit less-or-equal-than diverges from IEEE float in dealing with INFINITY/NAN
	// Posit NaR is smaller or equal than any other value
	template<size_t nbits, size_t es>
	int VerifyPositLogicLessOrEqualThan() {
		constexpr size_t max = nbits > 10 ? 10 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.set_raw_bits(j);

				// set the golden reference			
				bool ref = (double(a) <= double(b));// same behavior as IEEE floats
				if (a.isnar()) {
					// special case of posit <= for NaR
					ref = true;
				}
				bool presult = (a <= b);
				if (ref != presult) {
					nrOfFailedTestCases++;
					std::cout << a << " <= " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit greater-or-equal-than diverges from IEEE float in dealing with INFINITY/NAN
	// Any number is greater-or-equal-than posit NaR
	template<size_t nbits, size_t es>
	int VerifyPositLogicGreaterOrEqualThan() {
		constexpr size_t max = nbits > 10 ? 10 : nbits;
		size_t NR_TEST_CASES = (size_t(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.set_raw_bits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.set_raw_bits(j);

				// set the golden reference			
				bool ref = (double(a) >= double(b));// same behavior as IEEE floats
				if (b.isnar()) {
					// special case of posit >= for NaR
					ref = true;
				}
				bool presult = (a >= b);
				if (ref != presult) {
					nrOfFailedTestCases++;
					std::cout << a << " >= " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

} // namespace sw::universal

