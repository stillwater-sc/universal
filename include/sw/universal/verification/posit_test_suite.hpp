#pragma once
// posit_test_suite.hpp : posit number system verification test suite
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <vector>
#include <iostream>
#include <typeinfo>
#include <random>
#include <limits>
#include <universal/verification/test_status.hpp>
#include <universal/verification/test_case.hpp>
#include <universal/verification/test_reporters.hpp>

namespace sw { namespace universal {

	/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

	template<typename TestType>
	int Compare(double input, const TestType& presult, double reference, bool reportTestCases) {
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
	template<typename TestType>
	void testLogicOperators(const TestType& a, const TestType& b) {
		using namespace std;
		cout << a << " vs " << b << endl;
		if (a == b) cout << "a == b\n"; else cout << "a != b\n";
		if (a != b) cout << "a != b\n"; else cout << "a == b\n";
		if (a < b)  cout << "a <  b\n"; else cout << "a >= b\n";
		if (a <= b) cout << "a <= b\n"; else cout << "a >  b\n";
		if (a > b)  cout << "a >  b\n"; else cout << "a <= b\n";
		if (a >= b) cout << "a >= b\n"; else cout << "a <  b\n";
	}

	// 

	/// <summary>
	/// verify all conversion conditions by enumerate all conversion cases for a posit configuration
	/// </summary>
	/// <typeparam name="TestType">the posit type under test</typeparam>
	/// <typeparam name="EnvelopeType">the posit type that is one bit bigger</typeparam>
	/// <typeparam name="MarshalingType">native IEEE floating-point type to marshal the conversion through</typeparam>
	/// <param name="reportTestCases">if true report the pass/fail of each test case</param>
	/// <returns></returns>
	template<typename TestType, typename EnvelopeType, typename MarshalingType>
	int VerifyConversion(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		static_assert(nbits + 1 == EnvelopeType::nbits, "The EnvelopeType is not one bit larger than the TestType");
		static_assert(std::numeric_limits<MarshalingType>::is_iec559, "MarshalingType is not an IEEE floating-point type");
		static_assert(std::numeric_limits<MarshalingType>::radix == 2, "MarshalingType is not a binary floating-point type");
		static_assert(nbits < 20, "Conversion test suite is limited to nbits < 20");
		// constexpr unsigned es = TestType::es;
		// we are going to generate a test set that consists of all posit configs and their midpoints
		// we do this by enumerating a posit that is 1-bit larger than the test posit configuration
		// These larger posits will be at the mid-point between the smaller posit sample values
		// and we'll enumerate the exact value, and a perturbation smaller and a perturbation larger
		// to test the rounding logic of the conversion.
		unsigned NR_TEST_CASES = (unsigned(1) << (nbits + 1));
		unsigned HALF = (unsigned(1) << nbits);

		MarshalingType halfMinpos = MarshalingType(EnvelopeType(SpecificValue::minpos)) / MarshalingType(2.0);
		// execute the test
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			EnvelopeType pref, pprev, pnext;

			pref.setbits(i);
			MarshalingType da = MarshalingType(pref);
			MarshalingType eps = MarshalingType(i == 0 ? halfMinpos : (da > 0 ? da * 1.0e-6 : da * -1.0e-6));
			MarshalingType input;
			TestType pa;
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
	int VerifyConversion<posit<NBITS_IS_2, ES_IS_0>, posit<NBITS_IS_3, ES_IS_0>, float>(bool reportTestCases) {
		using SrcType = float;
		int nrOfFailedTestCases = 0;
		// special case
		posit<NBITS_IS_2, ES_IS_0> p = -INFINITY;
		if (!isnar(p)) nrOfFailedTestCases++;
		// test vector
		std::vector<SrcType> in  = { -4.0, -2.0, -1.0, -0.5, -0.25, 0.0, 0.25, 0.5, 1.0, 2.0, 4.0 };
		std::vector<SrcType> ref = { -1.0, -1.0, -1.0, -1.0, -1.00, 0.0, 1.00, 1.0, 1.0, 1.0, 1.0 };
		unsigned ref_index = 0;
		for (auto v : in) {
			p = v;
			SrcType refv = ref[ref_index++];
			if (SrcType(p) != refv) {
				if (reportTestCases) std::cout << " FAIL " << p << " != " << refv << std::endl;
				nrOfFailedTestCases++;
			}
			else {
				//if (reportTestCases) std::cout << " PASS " << p << " == " << refv << std::endl;
			}
		}
		return nrOfFailedTestCases;
	}
	template<>
	int VerifyConversion<posit<NBITS_IS_2, ES_IS_0>, posit<NBITS_IS_3, ES_IS_0>, double>(bool reportTestCases) {
		using SrcType = double;
		int nrOfFailedTestCases = 0;
		// special case
		posit<NBITS_IS_2, ES_IS_0> p = -INFINITY;
		if (!isnar(p)) nrOfFailedTestCases++;
		// test vector
		std::vector<SrcType> in = { -4.0, -2.0, -1.0, -0.5, -0.25, 0.0, 0.25, 0.5, 1.0, 2.0, 4.0 };
		std::vector<SrcType> ref = { -1.0, -1.0, -1.0, -1.0, -1.00, 0.0, 1.00, 1.0, 1.0, 1.0, 1.0 };
		unsigned ref_index = 0;
		for (auto v : in) {
			p = v;
			SrcType refv = ref[ref_index++];
			if (SrcType(p) != refv) {
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
	template<typename TestType>
	int VerifyIntegerConversion(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		//constexpr unsigned es = TestType::es;
		// we generate numbers from 1 via NaR to -1 and through the special case of 0 back to 1
		constexpr unsigned max = nbits > 20 ? 20 : nbits;
		unsigned NR_TEST_CASES = (unsigned(1) << (max - 1)) + 1;  
		int nrOfFailedTestCases = 0;		
		TestType p(1);
		for (unsigned i = 0; i < NR_TEST_CASES; ++i) {
			//std::cout << to_binary(p) << " : " << p << '\n';
			if (!p.isnar()) {
				long long ref = (long long)(p);  // obtain the integer cast of this posit
				TestType presult = ref;  // assign this integer to a posit				
				if (ref != (long long)presult) { // compare the integer cast to the reference posit
					if (reportTestCases) std::cout << " FAIL " << p << " != " << presult << " : reference = " << ref << std::endl;
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
	template<typename TestType>
	int VerifyUintConversion(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	void GenerateOrderedPositSet(std::vector<TestType>& set) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyIncrement(bool reportTestCases)	{
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyDecrement(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyPostfix(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyPrefix(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyNegation(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifySqrt(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyAddition(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyInPlaceAddition(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifySubtraction(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
		const unsigned NR_POSITS = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.setbits(i);
			double da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pref, pdiff;

				pb.setbits(j);
				double db = double(pb);
				pref = da - db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pdiff = pa - pb;
				}
				catch (const posit_operand_is_nar&) {
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						pdiff.setnar();
					}
					else {
						throw; // rethrow
					}
				}
#else
				pdiff = pa - pb;
#endif
				if (pdiff != pref) {
					nrOfFailedTests++;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-", pa, pb, pdiff, pref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", pa, pb, pdiff, pref);
				}
			}
		}

		return nrOfFailedTests;
	}

	// enumerate all subtraction cases for a posit configuration: is within 10sec till about nbits = 14
	template<typename TestType>
	int VerifyInPlaceSubtraction(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
		const unsigned NR_POSITS = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_POSITS; i++) {
			posit<nbits, es> pa;
			pa.setbits(i);
			double da = double(pa);
			for (unsigned j = 0; j < NR_POSITS; j++) {
				posit<nbits, es> pb, pref, pdiff;
				pb.setbits(j);
				double db = double(pb);
				pref = da - db;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
				try {
					pdiff = pa;
					pdiff -= pb;
				}
				catch (const posit_operand_is_nar& err) {
					if (pa.isnar() || pb.isnar()) {
						// correctly caught the exception
						pdiff.setnar();
					}
					else {
						throw err;  // rethrow
					}
				}
#else
				pdiff = pa;
				pdiff -= pb;
#endif
				if (pdiff != pref) {
					nrOfFailedTests++;
					if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-=", pa, pb, pdiff, pref);
				}
				else {
					//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-=", pa, pb, pdiff, pref);
				}
			}
		}

		return nrOfFailedTests;
	}

	// enumerate all multiplication cases for a posit configuration: is within 10sec till about nbits = 14
	template<typename TestType>
	int VerifyMultiplication(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyInPlaceMultiplication(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyReciprocation(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
		const unsigned NR_TEST_CASES = (unsigned(1) << nbits);
		int nrOfFailedTests = 0;
		for (unsigned i = 0; i < NR_TEST_CASES; i++) {
			posit<nbits, es> pa, preciprocal, preference;
			pa.setbits(i);
			// generate reference
			double da;
#if POSIT_THROW_ARITHMETIC_EXCEPTION
			try {
				preciprocal = pa.reciprocal();
				if (pa.isnar()) {
					preference.setnar();
				}
				else {
					da = double(pa);
					preference = 1.0 / da;
				}
			}
			catch (const posit_divide_by_zero& err) {
				if (pa.iszero()) {
					// correctly caught the exception
				}
				else {
					throw err;
				}
			}
			catch (const posit_divide_by_nar& err) {
				if (pa.isnar()) {
					// correctly caught the exception
					preference.setnar();
				}
				else {
					throw err;
				}
			}
#else
			preciprocal = pa.reciprocal();  // this will err when pa == 0
			da = double(pa);
			preference = 1.0 / da;
#endif
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
	template<typename TestType>
	int VerifyDivision(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyInPlaceDivision(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
	template<typename TestType>
	int VerifyLogicEqual(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
					if (reportTestCases) std::cout << a << " == " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit not-equal diverges from IEEE float in dealing with INFINITY/NAN
	// Posit NaR can be checked for equality/inequality
	template<typename TestType>
	int VerifyLogicNotEqual(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
					if (reportTestCases) std::cout << a << " != " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit less-than diverges from IEEE float in dealing with INFINITY/NAN
	// Posit NaR is smaller than any other value
	template<typename TestType>
	int VerifyLogicLessThan(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
					if (reportTestCases) std::cout << a << " < " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit greater-than diverges from IEEE float in dealing with INFINITY/NAN
	// Any number is greater-than posit NaR
	template<typename TestType>
	int VerifyLogicGreaterThan(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
					if (reportTestCases) std::cout << a << " > " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit less-or-equal-than diverges from IEEE float in dealing with INFINITY/NAN
	// Posit NaR is smaller or equal than any other value
	template<typename TestType>
	int VerifyLogicLessOrEqualThan(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
					if (reportTestCases) std::cout << a << " <= " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

	// Posit greater-or-equal-than diverges from IEEE float in dealing with INFINITY/NAN
	// Any number is greater-or-equal-than posit NaR
	template<typename TestType>
	int VerifyLogicGreaterOrEqualThan(bool reportTestCases) {
		constexpr unsigned nbits = TestType::nbits;
		constexpr unsigned es = TestType::es;
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
					if (reportTestCases) std::cout << a << " >= " << b << " fails: reference is " << ref << " actual is " << presult << std::endl;
				}
			}
		}
		return nrOfFailedTestCases;
	}

}} // namespace sw::universal

