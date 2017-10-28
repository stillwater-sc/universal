#pragma once
// posit_manipulators.hpp: definitions of helper functions for posit type manipulation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <cmath>  // for frexp/frexpf

// This file contains functions that use the posit type.
// If you have helper functions that the posit type could use, but does not depend on 
// the posit type, you can add them to the file posit_helpers.hpp.


// DEBUG/REPORTING HELPERS
template<size_t nbits, size_t es>
std::string spec_to_string(posit<nbits, es> p) {
	std::stringstream ss;
	ss << " posit<" << std::setw(2) << nbits << "," << es << "> ";
	ss << "useed scale " << std::setw(4) << p.useed_scale() << "     ";
	ss << "minpos scale " << std::setw(10) << p.minpos_scale() << "     ";
	ss << "maxpos scale " << std::setw(10) << p.maxpos_scale();
	return ss.str();
}

template<size_t nbits, size_t es>
std::string components_to_string(const posit<nbits, es>& p) {
	std::stringstream ss;
	if (p.isZero()) {
		ss << " zero    " << std::setw(103) << "b" << p.get();
		return ss.str();
	}
	else if (p.isInfinite()) {
		ss << " infinite" << std::setw(103) << "b" << p.get();
		return ss.str();
	}

	ss << std::setw(14) << to_binary(p.get()) << std::setw(14) << to_binary(p.get_decoded())
		<< " Sign : " << std::setw(2) << p.sign_value()
		<< " Regime : " << std::setw(3) << p.regime_k()
		<< " Exponent : " << std::setw(5) << p.get_exponent().value()
		<< " Fraction : " << std::setw(8) << std::setprecision(21) << 1.0 + p.fraction_value()
		<< " Value : " << std::setw(16) << p.to_double()
		<< std::setprecision(0);
	return ss.str();
}

template<size_t nbits, size_t es>
std::string component_values_to_string(posit<nbits, es> p) {
	std::stringstream ss;
	if (p.isZero()) {
		ss << " zero    " << std::setw(103) << "b" << p.get();
		return ss.str();
	}
	else if (p.isInfinite()) {
		ss << " infinite" << std::setw(103) << "b" << p.get();
		return ss.str();
	}

	ss << std::setw(14) << to_binary(p.get())
		<< " Sign : " << std::setw(2) << p.sign()
		<< " Regime : " << p.regime_int()
		<< " Exponent : " << p.exponent_int()
		<< std::hex
		<< " Fraction : " << p.fraction_int()
		<< " Value : " << p.to_int64()
		<< std::dec;
	return ss.str();
}


// generate a full binary representation table for a given posit configuration
template<size_t nbits, size_t es>
void GeneratePositTable(std::ostream& ostr) 
{
	ostr << "Generate Posit Lookup table for a POSIT<" << nbits << "," << es << ">" << std::endl;

	const size_t size = (1 << nbits);
	posit<nbits, es>	myPosit;

	const size_t index_column = 5;
	const size_t bin_column = 16;
	const size_t k_column = 16;
	const size_t sign_column = 16;
	const size_t regime_column = 30;
	const size_t exponent_column = 16;
	const size_t fraction_column = 16;
	const size_t value_column = 30;

	ostr << std::setw(index_column) << " # "
		<< std::setw(bin_column) << " Binary"
		<< std::setw(bin_column) << " Decoded"
		<< std::setw(k_column) << " k-value"
		<< std::setw(sign_column) << "sign"
		<< std::setw(regime_column) << " regime"
		<< std::setw(exponent_column) << " exponent"
		<< std::setw(fraction_column) << " fraction"
		<< std::setw(value_column) << " value" << std::endl;
	for (int i = 0; i < size; i++) {
		myPosit.set_raw_bits(i);
		regime<nbits,es>   r = myPosit.get_regime();
		exponent<nbits,es> e = myPosit.get_exponent();
		fraction<nbits-2> f = myPosit.get_fraction();
		ostr << std::setw(4) << i << ": "
			<< std::setw(bin_column) << myPosit.get()
			<< std::setw(bin_column) << myPosit.get_decoded()
			<< std::setw(k_column) << myPosit.regime_k()
			<< std::setw(sign_column) << myPosit.sign_value()
			<< std::setw(regime_column) << std::setprecision(22) << r.value() << std::setprecision(0)
			<< std::setw(exponent_column) << std::right << e 
			<< std::setw(fraction_column) << std::right << f
			<< std::setw(value_column) << std::setprecision(22) << myPosit.to_double() << std::setprecision(0)
			<< std::endl;
	}
}

// print scales of different posit configurations
// useed = 2^(2^es) and thus is just a function of the exponent configuration
// maxpos = useed^(nbits-2)
// minpos = useed^(2-nbits)
void ReportPositScales() {
	posit<3, 0> p3_0;
	posit<4, 0> p4_0;
	posit<4, 1> p4_1;
	posit<5, 0> p5_0;
	posit<5, 1> p5_1;
	posit<5, 2> p5_2;
	posit<6, 0> p6_0;
	posit<6, 1> p6_1;
	posit<6, 2> p6_2;
	posit<6, 3> p6_3;
	posit<7, 0> p7_0;
	posit<7, 1> p7_1;
	posit<7, 2> p7_2;
	posit<7, 3> p7_3;
	posit<7, 4> p7_4;
	posit<8, 0> p8_0;
	posit<16, 1> p16_1;
	posit<32, 2> p32_2;
	posit<64, 3> p64_3;

	std::cout << "Posit specificiation examples and their ranges:" << std::endl;
	std::cout << "Small, specialized posit configurations" << std::endl;
	std::cout << "nbits = 3" << std::endl;
	std::cout << spec_to_string(p3_0) << std::endl;
	std::cout << "nbits = 4" << std::endl;
	std::cout << spec_to_string(p4_0) << std::endl;
	std::cout << spec_to_string(p4_1) << std::endl;
	std::cout << "nbits = 5" << std::endl;
	std::cout << spec_to_string(p5_0) << std::endl;
	std::cout << spec_to_string(p5_1) << std::endl;
	std::cout << spec_to_string(p5_2) << std::endl;
	std::cout << "nbits = 6" << std::endl;
	std::cout << spec_to_string(p6_0) << std::endl;
	std::cout << spec_to_string(p6_1) << std::endl;
	std::cout << spec_to_string(p6_2) << std::endl;
	std::cout << spec_to_string(p6_3) << std::endl;
	std::cout << "nbits = 7" << std::endl;
	std::cout << spec_to_string(p7_0) << std::endl;
	std::cout << spec_to_string(p7_1) << std::endl;
	std::cout << spec_to_string(p7_2) << std::endl;
	std::cout << spec_to_string(p7_3) << std::endl;
	std::cout << spec_to_string(p7_4) << std::endl;
	std::cout << "Standard posit configurations" << std::endl;
	std::cout << spec_to_string(p8_0) << std::endl;
	std::cout << spec_to_string(p16_1) << std::endl;
	std::cout << spec_to_string(p32_2) << std::endl;
	std::cout << spec_to_string(p64_3) << std::endl;
	std::cout << std::endl;
}


// enumerate all addition cases for a posit configuration
template<size_t nbits, size_t es>
int ValidateAddition(std::string error_tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pb, psum, pref;

	double input_values[NR_TEST_CASES];
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pref.set_raw_bits(i);
		input_values[i] = pref.to_double();
	}
	double fa, fb;
	for (int i = 1; i < NR_TEST_CASES; i++) {
		fa = input_values[i];
		pa = fa;
		for (int j = 2; j < NR_TEST_CASES; j++) {
			fb = input_values[j];
			pb = fb;
			psum = pa + pb;
			pref = fa + fb;
			if (fabs(psum.to_double() - pref.to_double()) > 0.0001) {
// 				std::cout << "pa " << pa << " pb " << pb << " psum " << psum << " pref " << pref << std::endl;
// 				std::cout << "fa " << fa << " fb " << fb << "  sum " << fa + fb << " pref " << pref << std::endl;
				if (bReportIndividualTestCases) 
                                    ReportBinaryArithmeticError("FAIL", "+", pa, pb, pref, psum);
				nrOfFailedTests++;
// 				throw "Falsch.";
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, pref, psum);
			}
		}
	}

	return nrOfFailedTests;
}

// enumerate all subtraction cases for a posit configuration
template<size_t nbits, size_t es>
int ValidateSubtraction(std::string error_tag, bool bReportIndividualTestCases) {
	const int NR_TEST_CASES = (1 << nbits);
	int nrOfFailedTests = 0;
	posit<nbits, es> pa, pb, pref, pdif;

	double input_values[NR_TEST_CASES];
	for (int i = 0; i < NR_TEST_CASES; i++) {
		pref.set_raw_bits(i);
		input_values[i] = pref.to_double();
	}
	double fa, fb;
	for (int i = 1; i < NR_TEST_CASES; i++) {
		fa = input_values[i];
		pa = fa;
		for (int j = 2; j < NR_TEST_CASES; j++) {
			fb = input_values[j];
			pb = fb;
			pdif = pa - pb;
			pref = fa - fb;
			if (fabs(pdif.to_double() - pref.to_double()) > 0.0001) {
				// 				std::cout << "pa " << pa << " pb " << pb << " psum " << psum << " pref " << pref << std::endl;
				// 				std::cout << "fa " << fa << " fb " << fb << "  sum " << fa + fb << " pref " << pref << std::endl;
				if (bReportIndividualTestCases)
					ReportBinaryArithmeticError("FAIL", "-", pa, pb, pref, pdif);
				nrOfFailedTests++;
				// 				throw "Falsch.";
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, pref, pdif);
			}
		}
	}

	return nrOfFailedTests;
}


template<size_t nbits, size_t es>
int ValidateMultiplication(std::string tag, bool bReportIndividualTestCases) {
	int nrOfFailedTests = 0;
	const size_t NR_OF_TESTCASES = (unsigned(1) << nbits);

	posit<nbits, es> pa, pb, pmul, pref;
	double da, db, dref;
	for (int i = 0; i < NR_OF_TESTCASES; i++) {
		pa.set_raw_bits(i);
		da = pa.to_double();
		for (int j = 0; j < NR_OF_TESTCASES; j++) {
			pb.set_raw_bits(j);
			db = pb.to_double();
			pmul = pa * pb;
			dref = da * db;
			pref = dref;
			if (fabs(pmul.to_double() - pref.to_double()) > 0.000000001) {
				if (bReportIndividualTestCases) ReportBinaryArithmeticError("FAIL", "*", pa, pb, pref, pmul);
				nrOfFailedTests++;
			}
			else {
				if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "*", pa, pb, pref, pmul);
			}
		}
	}
	return nrOfFailedTests;
}


