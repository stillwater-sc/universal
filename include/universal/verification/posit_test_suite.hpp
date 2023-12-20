#pragma once
// posit_test_suite.hpp : posit number system verification test suite
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>
#include <universal/verification/test_status.hpp> // ReportTestResult used by test suite runner
#include <universal/verification/test_reporters.hpp> 

namespace sw { namespace universal {

	template<unsigned nbits, unsigned es>
	void ReportDecodeError(const std::string& test_case, const posit<nbits, es>& actual, double golden_value) {
		std::cerr << test_case << " actual " << actual << " required " << golden_value << std::endl;
	}

	/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

	template<unsigned nbits, unsigned es>
	int Compare(double input, const posit<nbits, es>& presult, double reference, bool reportTestCases) {
		int fail = 0;
		double result = double(presult);
		if (std::fabs(result - reference) > 0.000000001) {
			fail++;
			if (reportTestCases)	ReportConversionError("FAIL", "=", input, presult, reference);
		}
		else {
			// if (reportTestCases) ReportConversionSuccess("PASS", "=", input, reference, presult);
		}
		return fail;
	}

	// logic operator consistency check
	template<unsigned nbits, unsigned es>
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
	template<unsigned nbits, unsigned es>
	int VerifyConversion(bool reportTestCases) {
		// we are going to generate a test set that consists of all posit configs and their midpoints
		// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
		// These larger posits will be at the mid-point between the smaller posit sample values
		// and we'll enumerate the exact value, and a perturbation smaller and a perturbation larger
		// to test the rounding logic of the conversion.
		constexpr unsigned max = nbits > 14 ? 14 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << (max + 1));
		unsigned HALF = (unsigned(1) << max);

		if (nbits > 20) {
			std::cout << "VerifyConversion<" << nbits << "," << es << ">: NR_TEST_CASES = " << NR_TEST_CASES << " constrained due to nbits > 20" << std::endl;
		}

		double halfMinpos = double(posit<nbits + 1, es>(SpecificValue::minpos)) / 2.0;
		// execute the test
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits + 1, es> pref, pprev, pnext;

			pref.setbits(i);
			double da = double(pref);
			double eps = double(i == 0 ? halfMinpos : (da > 0 ? da * 1.0e-6 : da * -1.0e-6));
			double input;
			posit<nbits, es> pa;
			if (i % 2) {
				if (i == 1) {
					// special case of projecting to +minpos
					// even the -delta goes to +minpos
					input = da - eps;
					pa = input;
					pnext.setbits(i + 1);
					nrOfFailedTests += Compare(input, pa, (double)pnext, reportTestCases);
					input = da + eps;
					pa = input;
					nrOfFailedTests += Compare(input, pa, (double)pnext, reportTestCases);
				}
				else if (i == HALF - 1) {
					// special case of projecting to +maxpos
					input = da - eps;
					pa = input;
					pprev.setbits(HALF - 2);
					nrOfFailedTests += Compare(input, pa, (double)pprev, reportTestCases);
				}
				else if (i == HALF + 1) {
					// special case of projecting to -maxpos
					input = da - eps;
					pa = input;
					pprev.setbits(HALF + 2);
					nrOfFailedTests += Compare(input, pa, (double)pprev, reportTestCases);
				}
				else if (i == NR_TEST_CASES - 1) {
					// special case of projecting to -minpos
					// even the +delta goes to -minpos
					input = da - eps;
					pa = input;
					pprev.setbits(i - 1);
					nrOfFailedTests += Compare(input, pa, (double)pprev, reportTestCases);
					input = da + eps;
					pa = input;
					nrOfFailedTests += Compare(input, pa, (double)pprev, reportTestCases);
				}
				else {
					// for odd values, we are between posit values, so we create the round-up and round-down cases
					// round-down
					input = da - eps;
					pa = input;
					pprev.setbits(i - 1);
					nrOfFailedTests += Compare(input, pa, (double)pprev, reportTestCases);
					// round-up
					input = da + eps;
					pa = input;
					pnext.setbits(i + 1);
					nrOfFailedTests += Compare(input, pa, (double)pnext, reportTestCases);
				}
			}
			else {
				// for the even values, we generate the round-to-actual cases
				if (i == 0) {
					// special case of assigning to 0
					input = 0.0;
					pa = input;
					nrOfFailedTests += Compare(input, pa, da, reportTestCases);
					// special case of projecting to +minpos
					input = da + eps;
					pa = input;
					pnext.setbits(i + 2);
					nrOfFailedTests += Compare(input, pa, (double)pnext, reportTestCases);
				}
				else if (i == NR_TEST_CASES - 2) {
					// special case of projecting to -minpos
					input = da - eps;
					pa = input;
					pprev.setbits(NR_TEST_CASES - 2);
					nrOfFailedTests += Compare(input, pa, (double)pprev, reportTestCases);
				}
				else {
					// round-up
					input = da - eps;
					pa = input;
					nrOfFailedTests += Compare(input, pa, da, reportTestCases);
					// round-down
					input = da + eps;
					pa = input;
					nrOfFailedTests += Compare(input, pa, da, reportTestCases);
				}
			}
		}
		return nrOfFailedTests;
	}

	template<>
	int VerifyConversion<NBITS_IS_2, ES_IS_0>(bool reportTestCases) {
		int nrOfFailedTestCases = 0;
		// special case
		posit<NBITS_IS_2, ES_IS_0> p = -INFINITY;
		if (!isnar(p)) nrOfFailedTestCases++;
		// test vector
		std::vector<double> in  = { -4.0, -2.0, -1.0, -0.5, -0.25, 0.0, 0.25, 0.5, 1.0, 2.0, 4.0 };
		std::vector<double> ref = { -1.0, -1.0, -1.0, -1.0, -1.00, 0.0, 1.00, 1.0, 1.0, 1.0, 1.0 };
		unsigned ref_index = 0;
		for (auto v : in) {
			p = v;
			double refv = ref[ref_index++];
			if (double(p) != refv) {
				if (reportTestCases) std::cout << " FAIL " << p << " != " << refv << std::endl;
				nrOfFailedTestCases++;
			}
			else {
				//if (reportTestCases) std::cout << " PASS " << p << " == " << refv << std::endl;
			}
		}
		return nrOfFailedTestCases;
	}

	// enumerate all conversion cases for integers
	template<unsigned nbits, unsigned es>
	int VerifyIntegerConversion(bool reportTestCases) {
		// we generate numbers from 1 via NaR to -1 and through the special case of 0 back to 1
		constexpr unsigned max = nbits > 20 ? 20 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << (max - 1)) + 1;  
		int nrOfFailedTestCases = 0;		
		// special cases in case we are clipped by the nbits > 20
		long ref = 0x80000000;  // -2147483648
		posit<nbits, es> presult(ref);
		if (ref != presult) {
			std::cout << " FAIL long(" << ref << ") != long(" << presult << ") : reference = -2147483648" << std::endl;
			nrOfFailedTestCases++;
		}
		posit<nbits, es> p(1);
		for (unsigned i = 0; i < NR_TEST_CASES; ++i) {
			if (!p.isnar()) {
				ref = (long)p; // obtain the integer cast of this posit
				presult = ref;		// assign this integer to a posit				
				if (ref != presult) { // compare the integer cast to the reference posit
					if (reportTestCases) std::cout << " FAIL long(" << p << ") != long(" << presult << ") : reference = " << ref << std::endl;
					nrOfFailedTestCases++;
				}
				else {
					//if (reportTestCases) std::cout << " PASS " << p << " casts to " << presult << " : reference = " << ref << std::endl;
				}
			}
			++p;
		}
		return nrOfFailedTestCases;
	}
/*
	// specialized template for fast posit<2,0>
	template<>
	int VerifyIntegerConversion<NBITS_IS_2, ES_IS_0>(const std::string& tag, bool reportTestCases) {
		std::vector<int> in = { -4, -3, -2, -1, 0, 1, 2, 3, 4 };
		std::vector<int> ref = { -1, -1, -1, -1, 0, 1, 1, 1, 1 };
		int nrOfFailedTestCases = 0;
		unsigned ref_index = 0;
		for (auto v : in) {
			posit<NBITS_IS_2, ES_IS_0> p;
			p = v;
			int refv = ref[ref_index++];
			if (p != refv) {
				if (reportTestCases) std::cout << tag << " FAIL " << p << " != " << refv << std::endl;
				nrOfFailedTestCases++;
			}
			else {
				//if (reportTestCases) std::cout << tag << " PASS " << p << " == " << refv << std::endl;
			}
		}
		return nrOfFailedTestCases;
	}
	// specialized template for fast posit<3,0>
	template<>
	int VerifyIntegerConversion<NBITS_IS_3, ES_IS_0>(const std::string& tag, bool reportTestCases) {
		std::vector<int> in = { -4, -3, -2, -1, 0, 1, 2, 3, 4 };
		std::vector<int> ref = { -2, -2, -2, -1, 0, 1, 2, 2, 2 };
		int nrOfFailedTestCases = 0;
		unsigned ref_index = 0;
		for (auto v : in) {
			posit<NBITS_IS_3, ES_IS_0> p;
			p = v;
			int refv = ref[ref_index++];
			if (p != refv) {
				if (reportTestCases) std::cout << tag << " FAIL " << p << " != " << refv << std::endl;
				nrOfFailedTestCases++;
			}
			else {
				//if (reportTestCases) std::cout << tag << " PASS " << p << " == " << refv << std::endl;
			}
		}
		return nrOfFailedTestCases;
	}
*/

// enumerate all conversion cases for integers
	template<unsigned nbits, unsigned es>
	int VerifyUintConversion(bool reportTestCases) {
		// we generate numbers from 1 via NaR to -1 and through the special case of 0 back to 1
		constexpr unsigned max = nbits > 20 ? 20 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << (max - 1)) + 1;
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
					if (reportTestCases) std::cout << " FAIL uint32(" << p << ") != uint32(" << presult << ") : reference = " << ref << std::endl;
					nrOfFailedTestCases++;
				}
				else {
					//if (reportTestCases) std::cout << tag << " PASS " << p << " == " << presult << " : reference = " << ref << std::endl;
				}
				--p;
			}
		}
		else {
			p = 1;
			if (!p.isone()) {
				if (reportTestCases) std::cout << " FAIL " << p << " != " << 1 << std::endl;
				nrOfFailedTestCases++;
			}
			for (unsigned i = 0; i < NR_TEST_CASES; ++i) {
				if (!p.isnar()) {
					unsigned long ref = (unsigned long)p;   // obtain the integer cast of this posit
					presult = ref;		  // assign this integer to a reference posit
					if (presult != ref) { // compare the integer cast to the reference posit
						if (reportTestCases) std::cout << " FAIL uint32(" << p << ") != uint32(" << presult << ") : reference = " << ref << std::endl;
						nrOfFailedTestCases++;
					}
					else {
						//if (reportTestCases) std::cout << " PASS " << p << " == " << presult << " : reference = " << ref << std::endl;
					}
				}
				++p;
			}
		}
		return nrOfFailedTestCases;
	}

	// Generate ordered set in ascending order from [-NaR, -maxpos, ..., +maxpos] for a particular posit config <nbits, es>
	template<unsigned nbits, unsigned es>
	void GenerateOrderedPositSet(std::vector<posit<nbits, es>>& set) {
		constexpr unsigned NR_OF_REALS = (unsigned(1) << nbits);		// don't do this for state spaces larger than 4G
		std::vector< posit<nbits, es> > s(NR_OF_REALS);
		posit<nbits, es> p;
		// generate raw set, which will sort later
		for (unsigned i = 0; i < NR_OF_REALS; ++i) {
			p.setbits(i);
			s[i] = p;
		}
		// sort the set
		std::sort(s.begin(), s.end());
		set = s;
	}

	// Verify the increment operator++
	template<unsigned nbits, unsigned es>
	int VerifyIncrement(bool reportTestCases)	{
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
				if (reportTestCases) std::cout << " FAIL " << p << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

	// Verify the decrement operator--
	template<unsigned nbits, unsigned es>
	int VerifyDecrement(bool reportTestCases)
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
				if (reportTestCases) std::cout << " FAIL " << p << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

	// Verify the postfix operator++
	template<unsigned nbits, unsigned es>
	int VerifyPostfix(bool reportTestCases)
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
				if (reportTestCases) std::cout << " FAIL " << p << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

	// Verify the prefix operator++
	template<unsigned nbits, unsigned es>
	int VerifyPrefix(bool reportTestCases)
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
				if (reportTestCases) std::cout << " FAIL " << p << " != " << ref << std::endl;
				nrOfFailedTestCases++;
			}
		}

		return nrOfFailedTestCases;
	}

	// enumerate all negation cases for a posit configuration: executes within 10 sec till about nbits = 14
	template<unsigned nbits, unsigned es>
	int VerifyNegation(bool reportTestCases) {
		constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		posit<nbits, es> pa(0), pneg(0), pref(0);

		double da;
		for (unsigned i = 1; i < NR_TEST_CASES; i++) {
			pa.setbits(i);
			pneg = -pa;
			// generate reference
			da = double(pa);
			pref = -da;
			if (pneg != pref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "-", pa, pref, pneg);
			}
			else {
				//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "-", pa, pref, pneg);
			}
		}
		return nrOfFailedTests;
	}

	// enumerate all SQRT cases for a posit configuration: executes within 10 sec till about nbits = 14
	template<unsigned nbits, unsigned es>
	int VerifySqrt(bool reportTestCases) {
		constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;

		for (unsigned i = 1; i < NR_TEST_CASES; i++) {
			posit<nbits, es> pa, psqrt, pref;
			pa.setbits(i);
			psqrt = sw::universal::sqrt(pa);
			// generate reference
			double da = double(pa);
			pref = std::sqrt(da);
			if (psqrt != pref) {
				nrOfFailedTests++;
				std::cout << psqrt << " != " << pref << std::endl;
				if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "sqrt", pa, pref, psqrt);
				if (nrOfFailedTests > 24) return nrOfFailedTests;
			}
			else {
				//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "sqrt", pa, pref, psqrt);
			}
		}
		return nrOfFailedTests;
	}

	// enumerate all addition cases for a posit configuration
	template<unsigned nbits, unsigned es>
	int VerifyAddition(bool reportTestCases) {
		const unsigned NR_POSITS = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		posit<nbits, es> pa, pb, psum, pref;

		double da, db;
		for (unsigned i = 0; i < NR_POSITS; i++) {
			pa.setbits(i);
			da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				pb.setbits(j);
				db = double(pb);
				pref = da + db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					psum = pa + pb;
				}
				catch (const posit_operand_is_nar& err) {
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
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", pa, pb, psum, pref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, psum, pref);
				}
				if (nrOfFailedTests > 9) return nrOfFailedTests;
			}
		}

		return nrOfFailedTests;
	}

	// enumerate all addition cases for a posit configuration
	template<unsigned nbits, unsigned es>
	int VerifyInPlaceAddition(bool reportTestCases) {
		const unsigned NR_POSITS = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.setbits(i);
			double da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, psum, pref;
				pb.setbits(j);
				double db = double(pb);
				pref = da + db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					psum = pa;
					psum += pb;
				}
				catch (const posit_operand_is_nar& err) {
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						psum.setnar();
					}
					else {
						throw err; // rethrow
					}
				}

#else
				psum = pa;
				psum += pb;
#endif
				if (psum != pref) {
					nrOfFailedTests++;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+=", pa, pb, psum, pref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+=", pa, pb, psum, pref);
				}
			}
		}

		return nrOfFailedTests;
	}

	// enumerate all subtraction cases for a posit configuration: is within 10sec till about nbits = 14
	template<unsigned nbits, unsigned es>
	int VerifySubtraction(bool reportTestCases) {
		const unsigned NR_POSITS = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.setbits(i);
			double da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pref, pdif;

				pb.setbits(j);
				double db = double(pb);
				pref = da - db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pdif = pa - pb;
				}
				catch (const posit_operand_is_nar&) {
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
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-", pa, pb, pdif, pref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", pa, pb, pdif, pref);
				}
			}
		}

		return nrOfFailedTests;
	}

	// enumerate all subtraction cases for a posit configuration: is within 10sec till about nbits = 14
	template<unsigned nbits, unsigned es>
	int VerifyInPlaceSubtraction(bool reportTestCases) {
		const unsigned NR_POSITS = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.setbits(i);
			double da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pref, pdif;
				pb.setbits(j);
				double db = double(pb);
				pref = da - db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pdif = pa;
					pdif -= pb;
				}
				catch (const posit_operand_is_nar& err) {
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						pdif.setnar();
					}
					else {
						throw err;  // rethrow
					}
				}
#else
				pdif = pa;
				pdif -= pb;
#endif
				if (pdif != pref) {
					nrOfFailedTests++;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-=", pa, pb, pdif, pref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-=", pa, pb, pdif, pref);
				}
			}
		}

		return nrOfFailedTests;
	}

	// enumerate all multiplication cases for a posit configuration: is within 10sec till about nbits = 14
	template<unsigned nbits, unsigned es>
	int VerifyMultiplication(bool reportTestCases) {
		int nrOfFailedTests = 0;
		const unsigned NR_POSITS = (unsigned(1) << nbits);
		for (unsigned i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.setbits(i);
			double da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pmul, pref;
				pb.setbits(j);
				double db = double(pb);
				pref = da * db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pmul = pa * pb;
				}
				catch (const posit_operand_is_nar&) {
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
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*", pa, pb, pmul, pref);
					nrOfFailedTests++;
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", pa, pb, pmul, pref);
				}
			}
		}
		return nrOfFailedTests;
	}

	// enumerate all multiplication cases for a posit configuration: is within 10sec till about nbits = 14
	template<unsigned nbits, unsigned es>
	int VerifyInPlaceMultiplication(bool reportTestCases) {
		int nrOfFailedTests = 0;
		const unsigned NR_POSITS = (unsigned(1) << nbits);
		for (unsigned i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.setbits(i);
			double da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pmul, pref;
				pb.setbits(j);
				double db = double(pb);
				pref = da * db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pmul = pa;
					pmul *= pb;
				}
				catch (const posit_operand_is_nar& err) {
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
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "*=", pa, pb, pmul, pref);
					nrOfFailedTests++;
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*=", pa, pb, pmul, pref);
				}
			}
		}
		return nrOfFailedTests;
	}

	// enerate all reciprocation cases for a posit configuration: executes within 10 sec till about nbits = 14
	template<unsigned nbits, unsigned es>
	int VerifyReciprocation(bool reportTestCases) {
		const unsigned NR_TEST_CASES = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> pa, preciprocal, preference;
			pa.setbits(i);
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
				if (reportTestCases)	ReportUnaryArithmeticError("FAIL", "reciprocate", pa, preciprocal, preference);
			}
			else {
				//if (reportTestCases) ReportUnaryArithmeticSuccess("PASS", "reciprocate", pa, preciprocal, preference);
			}
		}
		return nrOfFailedTests;
	}

	// enumerate all division cases for a posit configuration: is within 10sec till about nbits = 14
	template<unsigned nbits, unsigned es>
	int VerifyDivision(bool reportTestCases) {
		constexpr unsigned NR_POSITS = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.setbits(i);
			double da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pdiv, pref;
				pb.setbits(j);
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
				catch (const posit_divide_by_zero&) {
					if (pb.iszero()) {
						// correctly caught the divide by zero condition
						continue;
						//pdiv.setnar();
					}
					else {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
						throw; // rethrow
					}
				}
				catch (const posit_divide_by_nar&) {
					if (pb.isnar()) {
						// correctly caught the divide by nar condition
						continue;
						//pdiv = 0.0f;
					}
					else {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
						throw; // rethrow
					}
				}
				catch (const posit_numerator_is_nar&) {
					if (pa.isnar()) {
						// correctly caught the numerator is nar condition
						continue;
						//pdiv.setnar();
					}
					else {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
						throw; // rethrow
					}
				}
#else
				pdiv = pa / pb;
#endif
				// check against the IEEE reference
				if (pdiv != pref) {
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
					nrOfFailedTests++;
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", pa, pb, pdiv, pref);
				}

			}
		}
		return nrOfFailedTests;
	}

	// enumerate all division cases for a posit configuration: is within 10sec till about nbits = 14
	template<unsigned nbits, unsigned es>
	int VerifyInPlaceDivision(bool reportTestCases) {
		constexpr unsigned NR_POSITS = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.setbits(i);
			double da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pdiv, pref;
				pb.setbits(j);
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
				catch (const posit_divide_by_zero& err) {
					if (pb.iszero()) {
						// correctly caught the divide by zero condition
						continue;
						//pdiv.setnar();
					}
					else {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
						throw err; // rethrow
					}
				}
				catch (const posit_divide_by_nar& err) {
					if (pb.isnar()) {
						// correctly caught the divide by nar condition
						continue;
						//pdiv = 0.0f;
					}
					else {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/", pa, pb, pdiv, pref);
						throw err; // rethrow
					}
				}
				catch (const posit_numerator_is_nar& err) {
					if (pa.isnar()) {
						// correctly caught the numerator is nar condition
						continue;
						//pdiv.setnar();
					}
					else {
						if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/=", pa, pb, pdiv, pref);
						throw err; // rethrow
					}
				}
#else
				pdiv = pa;
				pdiv /= pb;
#endif
				// check against the IEEE reference
				if (pdiv != pref) {
					if (reportTestCases) ReportBinaryArithmeticError("FAIL", "/=", pa, pb, pdiv, pref);
					nrOfFailedTests++;
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/=", pa, pb, pdiv, pref);
				}

			}
		}
		return nrOfFailedTests;
	}

	// Posit equal diverges from IEEE float in dealing with INFINITY/NAN
	// Posit NaR can be checked for equality/inequality
	template<unsigned nbits, unsigned es>
	int VerifyPositLogicEqual() {
		constexpr unsigned max = nbits > 10 ? 10 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.setbits(j);
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
	template<unsigned nbits, unsigned es>
	int VerifyPositLogicNotEqual() {
		constexpr unsigned max = nbits > 10 ? 10 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.setbits(j);

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
	template<unsigned nbits, unsigned es>
	int VerifyPositLogicLessThan() {
		constexpr unsigned max = nbits > 10 ? 10 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.setbits(j);

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
	template<unsigned nbits, unsigned es>
	int VerifyPositLogicGreaterThan() {
		constexpr unsigned max = nbits > 10 ? 10 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.setbits(j);

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
	template<unsigned nbits, unsigned es>
	int VerifyPositLogicLessOrEqualThan() {
		constexpr unsigned max = nbits > 10 ? 10 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.setbits(j);

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
	template<unsigned nbits, unsigned es>
	int VerifyPositLogicGreaterOrEqualThan() {
		constexpr unsigned max = nbits > 10 ? 10 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << max);
		int nrOfFailedTestCases = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> a;
			a.setbits(i);
			for (unsigned j = 0; j < NR_TEST_CASES; j++) {
				posit<nbits, es> b;
				b.setbits(j);

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

}} // namespace sw::universal

