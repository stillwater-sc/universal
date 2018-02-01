//  quires.cpp : test suite for IEEE float quires
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include "stdafx.h"

#include "../../posit/bit_functions.hpp"
#include "../../float/float_functions.hpp"
#include "../../float/exceptions.hpp"
#include "../../posit/value.hpp"
#include "../../float/quire.hpp"

using namespace std;
using namespace sw::ieee;

int TestQuireAccumulationResult(int nrOfFailedTests, string descriptor)
{
	if (nrOfFailedTests > 0) {
		std::cout << descriptor << " quire accumulation FAIL" << std::endl;
	}
	else {
		std::cout << descriptor << " quire accumulation PASS" << std::endl;
	}
	return nrOfFailedTests;
}

template<typename Ty, size_t capacity>
int ValidateQuireAccumulation() {
	const size_t NR_TEST_CASES = size_t(1) << nbits;

	int nrOfFailedTests = 0;

	return nrOfFailedTests;
}

template<typename Ty, size_t capacity>
void GenerateTestCase(int input, const quire<Ty, capacity>& reference, const quire<Ty, capacity>& qresult) {

	std::cout << std::endl;
}

template<typename Ty, size_t capacity, size_t fbits = 1>
void GenerateValueAssignments() {
	quire<Ty, capacity> q;

	// report some parameters about the posit and quire configuration
	int max_scale = q.max_scale();
	int min_scale = q.min_scale();
	std::cout << "Maximum scale  = " << max_scale << " Minimum scale  = " << min_scale << " Dynamic range = " << q.dynamic_range() << std::endl;
	std::cout << "Maxpos Squared = " << maxpos_scale<Ty>() * 2 << " Minpos Squared = " << minpos_scale<Ty>() * 2 << std::endl;

	// cover the scales with one order outside of the dynamic range of the quire configuration (minpos^2 and maxpos^2)
	for (int scale = max_scale + 1; scale >= min_scale - 1; scale--) {  // extend by 1 max and min scale to test edge of the quire
		sw::unum::value<fbits> v = pow(2.0, scale);
		try {
			q = v;
			std::cout << setw(10) << v << q << std::endl;
			sw::unum::value<q.qbits> r = q.to_value();
			double in = (double)v;
			double out = (double)r;
			if (std::abs(in - out) > 0.0000001) { 
				std::cerr << "quire value conversion failed: " << components(v) << " != " << components(r) << std::endl; 
			}
		}
		catch (char const* msg) {
			std::cerr << "Caught the exception: " << msg << ". RHS was " << v << " " << components(v) << std::endl;
		}
	}
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main()
try {
	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Quire Accumulation failed";

#if MANUAL_TESTING
	const size_t capacity = 2; // for testing the accumulation capacity of the quire can be small
	const size_t fbits = 5;

	//GenerateUnsignedIntAssignments<nbits, es, capacity>();
	//GenerateSignedIntAssignments<nbits, es, capacity>();
	//GenerateUnsignedIntAssignments<8, 2, capacity>();

	GenerateValueAssignments<float, capacity, fbits>();

	std::cout << endl;
	std::cout << "Creating quires for float and double arithmetic" << std::endl;
	float f = 1.555555555555e-10f;
	sw::ieee::quire<float, 2> fquire(f);
	std::cout << "float:  " << setw(15) << f << " " << fquire << std::endl;

	double d = 1.555555555555e16;
	sw::ieee::quire<double, 2> dquire(d);
	std::cout << "double: " << setw(15) << d << " " << dquire << std::endl;

	std::cout << std::endl;
	// nbits = 4, es = 1, capacity = 2
	//  17 16   15 14 13 12 11 10  9  8    7  6  5  4  3  2  1  0
	// [ 0  0    0  0  0  0  0  0  0  0    0  0  0  0  0  0  0  0 ]
	quire<float, capacity> q;
	sw::unum::value<5> maxpos, maxpos_squared, minpos, minpos_squared;
	float dmax = (float)sw::ieee::maxpos_value<float>();
	maxpos = dmax;
	maxpos_squared = dmax*dmax;
	std::cout << "maxpos * maxpos = " << sw::unum::components(maxpos_squared) << std::endl;
	float dmin = (float)sw::ieee::minpos_value<float>();
	minpos = dmin;
	minpos_squared = dmin*dmin;
	std::cout << "minpos * minpos = " << sw::unum::components(minpos_squared) << std::endl;
	sw::unum::value<5> c(maxpos_squared);

	std::cout << "Add/Subtract propagating carry/borrows to and from capacity segment" << std::endl;
	q.clear();
	sw::unum::value<5> v(64);
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << " <- entering capacity bits" << std::endl;
	q += c;		std::cout << q << " <- adding maxpos^2" << std::endl;
	q += c;     std::cout << q << " <- flipping another capacity bit" << std::endl;
	q += -c;	std::cout << q << " <- subtracting maxpos^2" << std::endl;
	q += -c;	std::cout << q << " <- subtracting maxpos^2" << std::endl;
	q += -v;	std::cout << q << " <- removing the capacity bit" << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << " <- should be zero" << std::endl;

	std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << std::endl;
	q = 0;
	v = 0.5;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << " <- should be zero" << std::endl;

	std::cout << "Add/Subtract propagating carry/borrows across lower/upper accumulators" << std::endl;
	q.clear();  // equivalent to q = 0, but more articulate/informative
	v = 3.875 + 0.0625; std::cout << "v " << components(v) << std::endl; // the input value is 11.1111 so hidden + 5 fraction bits
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += v;		std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << std::endl;
	q += -v;	std::cout << q << " <- should be zero" << std::endl;


#else

	std::cout << "Quire validation" << std::endl;
	TestQuireAccumulationResult(ValidateQuireAccumulation<8,0,5>(), "quire<8,0,5>");

#ifdef STRESS_TESTING


#endif // STRESS_TESTING


#endif // MANUAL_TESTING
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	cerr << msg << endl;
	return EXIT_FAILURE;
}
catch (...) {
	cerr << "Caught unknown exception" << endl;
	return EXIT_FAILURE;
}
