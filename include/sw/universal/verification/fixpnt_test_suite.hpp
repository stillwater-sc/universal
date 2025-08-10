#pragma once
//  fixpnt_test_suite.hpp : arithmetic/logic test suite for arbitrary fixed point number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <typeinfo>

// We want the test suite to be used with different configurations of the fixed-point number system
// so the calling environment needs to set the configuration
#include <universal/number/fixpnt/fixpnt_impl.hpp>
#include <universal/number/fixpnt/attributes.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult used by test suite runner
#include <universal/verification/test_reporters.hpp> 

namespace sw { namespace universal {

/////////////////////////////// VERIFICATION TEST SUITES ////////////////////////////////

template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
int Compare(double testValue, const fixpnt<nbits, rbits, arithmetic, BlockType>& presult, double reference, bool reportTestCases) {
	int fail = 0;
	double result = double(presult);
	if (std::fabs(result - reference) > 0.000000001) {
		fail++;
		if (reportTestCases)	ReportConversionError("FAIL", "=", testValue, presult, reference);
	}
	else {
		// if (reportTestCases) ReportConversionSuccess("PASS", "=", testValue, reference, presult);
	}
	return fail;
}

template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType, typename Ty>
int VerifyAssignment(bool reportTestCases) {
	const unsigned NR_NUMBERS = (unsigned(1) << nbits);
	int nrOfFailedTestCases = 0;

	// use only valid fixed-point values
	// fixpnt_raw -> to value in Ty -> assign to fixpnt -> compare fixpnts
	fixpnt<nbits, rbits, arithmetic, BlockType> p, assigned;
	for (unsigned i = 0; i < NR_NUMBERS; i++) {
		p.setbits(i); 
		//std::cout << to_binary(p) << std::endl;
		Ty value = (Ty)(p);
		assigned = value;
		//std::cout << p << " " << value << " " << assigned << std::endl;
		if (p != assigned) {
			nrOfFailedTestCases++;
			if (reportTestCases) ReportAssignmentError("FAIL", "=", p, assigned, value);
		}
		else {
			//if (reportTestCases) ReportAssignmentSuccess("PASS", "=", p, assigned, value);
		}
	}
	return nrOfFailedTestCases;
}

/*
	relationship between fixpnt<nbits,rbits> and fixpnt<nbits+1,rbits+1>

 */
// enumerate all conversion cases for a fixed-point configuration
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
int VerifyConversion(bool reportTestCases) {
	// we are going to generate a test set that consists of all fixed-point configs and their midpoints
	// we do this by enumerating a fixed-point that is 1-bit larger than the test configuration
	// with the extra bit allocated to the fraction => rbits+1
	// These larger configuration fixpnt valuess will be at the mid-point between the smaller 
	// configuration fixpnt values thus creating a full cover test set for value conversions.
	// The precondition for this type of test is that the value conversion is verified.
	// To generate the three test cases, we'll enumerate the exact value, and a perturbation slightly
	// smaller from the midpoint that will round down, and one slightly larger that will round up,
	// to test the rounding logic of the conversion.
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << (nbits + 1));
	constexpr unsigned HALF = (unsigned(1) << nbits);
	fixpnt<nbits + 1, rbits + 1, arithmetic, BlockType> ref, prev, next;

	const unsigned max = nbits > 20 ? 20 : nbits + 1;
	unsigned max_tests = (unsigned(1) << max);
	if (max_tests < NR_TEST_CASES) {
		std::cout << "VerifyConversion<" << nbits << "," << rbits << ">: NR_TEST_CASES = " << NR_TEST_CASES << " clipped by " << max_tests << std::endl;
	}

	// execute the test
	int nrOfFailedTests = 0;
	fixpnt<nbits + 1, rbits + 1, arithmetic, BlockType> minpos(SpecificValue::minpos);
	double dminpos = double(minpos);
	fixpnt<nbits, rbits, arithmetic, BlockType> maxneg(SpecificValue::maxneg);
	double dmaxneg = double(maxneg);

	// NUT: number under test
	fixpnt<nbits, rbits, arithmetic, BlockType> nut;
	double eps = dminpos / 2.0;  // the test value between 0 and minpos
	for (unsigned i = 0; i < NR_TEST_CASES && i < max_tests; ++i) {
		double testValue{ 0.0 };
		ref.setbits(i);
		double da = double(ref);
		if (i > 0) {
			eps = da > 0 ? da * 1.0e-6 : da * -1.0e-6;
		}
		if (i % 2) {
			if (i == 1) {
				// special case of a tie that needs to round to even -> 0
				testValue = da;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, 0.0, reportTestCases);

				// this rounds up
				testValue = da + eps;
				nut = testValue;
				next.setbits(i + 1);
				nrOfFailedTests += Compare(testValue, nut, (double)next, reportTestCases);

			}
			else if (i == HALF - 1) {
				// special case of projecting to maxpos
				testValue = da - eps;
				nut = testValue;
				prev.setbits(HALF - 2);
				nrOfFailedTests += Compare(testValue, nut, (double)prev, reportTestCases);
			}
			else if (i == HALF + 1) {
				// special case of projecting to maxneg
				testValue = da - eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, dmaxneg, reportTestCases);
			}
			else if (i == NR_TEST_CASES - 1) {
				// special case of projecting to minneg
				testValue = da - eps;
				nut = testValue;
				prev.setbits(i - 1);
				nrOfFailedTests += Compare(testValue, nut, (double)prev, reportTestCases);
				// but the +delta goes to 0
				testValue = da + eps;
				nut = testValue;
//				nrOfFailedTests += Compare(testValue, nut, (double)prev, reportTestCases);
				nrOfFailedTests += Compare(testValue, nut, 0.0, reportTestCases);
			}
			else {
				// for odd values, we are between fixed point values, so we create the round-up and round-down cases
				// round-down
				testValue = da - eps;
				nut = testValue;
				prev.setbits(i - 1);
				nrOfFailedTests += Compare(testValue, nut, (double)prev, reportTestCases);
				// round-up
				testValue = da + eps;
				nut = testValue;
				next.setbits(i + 1);
				nrOfFailedTests += Compare(testValue, nut, (double)next, reportTestCases);
			}
		}
		else {
			// for the even values, we generate the round-to-actual cases
			if (i == 0) {
				// pref = 0
				// 0                 -> value = 0
				// half of next     -> value = 0
				// special case of assigning to 0
				testValue = da;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, da, reportTestCases);

				testValue = da + eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, da, reportTestCases);
			}
			else if (i == NR_TEST_CASES - 2) {
				// special case of projecting to minneg
				testValue = da - eps;
				nut = testValue;
				prev.setbits(NR_TEST_CASES - 2);
				nrOfFailedTests += Compare(testValue, nut, (double)prev, reportTestCases);
			}
			else {
				// for even values, we are on actual fixed point values, so we create the round-up and round-down cases
				// round-up
				testValue = da - eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, da, reportTestCases);
				// round-down
				testValue = da + eps;
				nut = testValue;
				nrOfFailedTests += Compare(testValue, nut, da, reportTestCases);
			}
		}
	}
	return nrOfFailedTests;
}

// enumerate all addition cases for an fixpnt<nbits,rbits> configuration
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
int VerifyAddition(bool reportTestCases) {
	constexpr unsigned NR_VALUES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits, arithmetic, BlockType> a, b, result, cref;
	double ref;

	// set the saturation clamps
	fixpnt<nbits, rbits, arithmetic, BlockType> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	double da, db;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		da = double(a);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			db = double(b);
			ref = da + db;
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				result = a + b;
			}
			catch (...) {
				if (ref < double(maxneg) || ref > double(maxpos)) {
					// correctly caught the overflow exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a + b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "+", a, b, cref, result);
			}
			else {
				//if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", a, b, cref, result);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) std::cout << '.';
	}
//	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all subtraction cases for an fixpnt<nbits,rbits> configuration
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
int VerifySubtraction(bool reportTestCases) {
	constexpr unsigned NR_VALUES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits, arithmetic, BlockType> a, b, result, cref;
	double ref;

	// set the saturation clamps
	fixpnt<nbits, rbits, arithmetic, BlockType> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	double da, db;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		da = double(a);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			db = double(b);
			ref = da - db;
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				result = a - b;
			}
			catch (...) {
				if (ref < double(maxneg) || ref > double(maxpos)) {
					// correctly caught the overflow exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a - b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "-", a, b, cref, result);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "-", a, b, cref, result);
			}
			if (nrOfFailedTests > 100) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) std::cout << '.';
	}
//	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all multiplication cases for an fixpnt<nbits,rbits> configuration
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
int VerifyMultiplication(bool reportTestCases) {
	constexpr unsigned NR_VALUES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits, arithmetic, BlockType> a, b, result, cref;
	double ref;

	// set the saturation clamps
	fixpnt<nbits, rbits, arithmetic, BlockType> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	double da, db;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		da = double(a);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			db = double(b);
			ref = da * db;
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			// catching overflow
			try {
				result = a * b;
			}
			catch (...) {
				if (ref < double(maxneg) || ref > double(maxpos)) {
					// correctly caught the overflow exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a * b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "*", a, b, cref, result);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "*", a, b, cref, result);
			}
			if (nrOfFailedTests > 24) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) std::cout << '.';
	}
//	std::cout << std::endl;
	return nrOfFailedTests;
}

// enumerate all division cases for a fixpnt<nbits,rbits> configuration
template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
int VerifyDivision(bool reportTestCases) {
	constexpr unsigned NR_VALUES = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	fixpnt<nbits, rbits, arithmetic, BlockType> a, b, result, cref;
	double ref;

	// set the saturation clamps
	fixpnt<nbits, rbits, arithmetic, BlockType> maxpos(SpecificValue::maxpos), maxneg(SpecificValue::maxneg);
	double da, db;
	for (unsigned i = 0; i < NR_VALUES; i++) {
		a.setbits(i);
		da = double(a);
		for (unsigned j = 0; j < NR_VALUES; j++) {
			b.setbits(j);
			db = double(b);
			if (j != 0) {
				ref = da / db;
			}
			else {
				ref = 0;
			}
#if FIXPNT_THROW_ARITHMETIC_EXCEPTION
			try {
				result = a / b;
			}
			catch (...) {
				if (ref < double(maxneg) || b == 0 || ref > double(maxpos)) {
					// correctly caught the overflow and divide by zero exception
					continue;
				}
				else {
					nrOfFailedTests++;
				}
			}

#else
			result = a / b;
#endif // FIXPNT_THROW_ARITHMETIC_EXCEPTION
			cref = ref;
			if (result != cref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportBinaryArithmeticError("FAIL", "/", a, b, result, cref);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "/", a, b, result, cref);
			}
//			if (nrOfFailedTests > 1) return nrOfFailedTests;
		}
//		if (i % 1024 == 0) std::cout << '.';
	}
//	std::cout << std::endl;
	return nrOfFailedTests;
}

//////////////////////////////////////////////////////////////////////////
// enumeration utility functions

template<unsigned nbits, unsigned rbits, bool arithmetic, typename BlockType>
void GenerateFixedPointValues(std::ostream& ostr, const fixpnt<nbits, rbits, arithmetic, BlockType>& v) {
	constexpr unsigned NR_TEST_CASES = (unsigned(1) << nbits);
	fixpnt<nbits, rbits, arithmetic, BlockType> a;
	ostr << type_tag(v) << '\n';
	for (unsigned i = 0; i < NR_TEST_CASES; ++i) {
		a.setbits(i);
		float f = float(a);
		ostr << to_binary(a) << " | " << to_triple(a) << " | " << std::setw(15) << a << " | " << std::setw(15) << f << '\n';
	}
}

}} // namespace sw::universal
