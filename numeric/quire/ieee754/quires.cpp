//  quires.cpp : test suite for IEEE float quires
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>

// till we figure out how to derive sizes from types
#define TEMPLATIZED_TYPE 0
// enable/disable quire exceptions
#define QUIRE_THROW_EXCEPTION 0
#include <universal/utility/find_msb.hpp>
#include <universal/number/float/float_functions.hpp>
#include <universal/number/float/quire.hpp>

int TestQuireAccumulationResult(int nrOfFailedTests, const std::string& descriptor)
{
	if (nrOfFailedTests > 0) {
		std::cout << descriptor << " quire accumulation FAIL" << std::endl;
	}
	else {
		std::cout << descriptor << " quire accumulation PASS" << std::endl;
	}
	return nrOfFailedTests;
}

template<size_t nbits, size_t es, size_t capacity>
int ValidateQuireAccumulation() {

	int nrOfFailedTests = 0;

	return nrOfFailedTests;
}

template<size_t nbits, size_t es, size_t capacity>
void GenerateTestCase(int input, const sw::ieee::quire<nbits, es, capacity>& reference, const sw::ieee::quire<nbits, es, capacity>& qresult) {

	std::cout << std::endl;
}

template<size_t nbits, size_t es, size_t capacity, size_t fbits = 1>
void GenerateValueAssignments() {
	sw::ieee::quire<nbits, es, capacity> q;

	// report some parameters about the posit and quire configuration
	int max_scale = q.max_scale();
	int min_scale = q.min_scale();
	std::cout << "Maximum scale  = " << max_scale << " Minimum scale  = " << min_scale << " Dynamic range = " << q.dynamic_range() << std::endl;
	std::cout << "Maxpos Squared = " << sw::ieee::maxpos_scale<nbits,es>() * 2 << " Minpos Squared = " << sw::ieee::minpos_scale<nbits,es>() * 2 << std::endl;

	// cover the scales with one order outside of the dynamic range of the quire configuration (minpos^2 and maxpos^2)
	for (int scale = max_scale + 1; scale >= min_scale - 1; scale--) {  // extend by 1 max and min scale to test edge of the quire
		sw::universal::internal::value<fbits> v = pow(2.0, scale);
		try {
			q = v;
			std::cout << std::setw(10) << v << q << std::endl;
			sw::universal::internal::value<q.qbits> r = q.to_value();
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
	using namespace sw::ieee;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Quire Accumulation failed";

#if MANUAL_TESTING
	// float
	constexpr size_t nbits = 32;
	constexpr size_t es = 8;
	constexpr size_t capacity = 2; // for testing the accumulation capacity of the quire can be small
	//constexpr size_t fbits = 5;

	//GenerateUnsignedIntAssignments<nbits, es, capacity>();
	//GenerateSignedIntAssignments<nbits, es, capacity>();
	//GenerateUnsignedIntAssignments<8, 2, capacity>();

	//GenerateValueAssignments<nbits, es, capacity, fbits>();

	typedef sw::ieee::quire<nbits, es, 2> QuireFloat;
	typedef sw::ieee::quire<64, 11, 2> QuireDouble;

	std::cout << std::endl;
	std::cout << "Creating quires for float and double arithmetic" << std::endl;
	float f = 1.555555555555e-10f;
	QuireFloat fquire(f);
	std::cout << "quire<32, 8, 2>: qbits: " << QuireFloat::qbits << " dynamic range: " << QuireFloat::escale << " lower range: " << QuireFloat::half_range << " upper range: " << QuireFloat::upper_range << std::endl;
	std::cout << "float:  " << std::setw(15) << f << " " << fquire << std::endl;

	double d = 1.555555555555e16;
	QuireDouble dquire(d);
	std::cout << "quire<64, 11, 2>: qbits: " << QuireDouble::qbits << " dynamic range: " << QuireDouble::escale << " lower range: " << QuireDouble::half_range << " upper range: " << QuireDouble::upper_range << std::endl;
	std::cout << "double: " << std::setw(15) << d << " " << dquire << std::endl;

	std::cout << std::endl;
	// quire for float nbits= 32 es = 8
	quire<32, 8, capacity> q;
	sw::universal::internal::value<54> maxpos, maxpos_squared, minpos, minpos_squared;
	constexpr double dmax = std::numeric_limits<float>::max();
	maxpos = dmax;
	maxpos_squared = dmax*dmax;
	std::cout << "maxpos * maxpos = " << sw::universal::internal::to_triple(maxpos_squared) << std::endl;
	constexpr double dmin = std::numeric_limits<float>::min();
	minpos = dmin;
	minpos_squared = dmin*dmin;
	std::cout << "minpos * minpos = " << sw::universal::internal::to_triple(minpos_squared) << std::endl;
	sw::universal::internal::value<54> c(maxpos_squared);

	std::cout << "Add/Subtract propagating carry/borrows to and from capacity segment" << std::endl;
	q.clear();
	sw::universal::internal::value<54> v = maxpos;
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
	v = 3.875 + 0.0625; std::cout << "v " << to_triple(v) << std::endl; // the input value is 11.1111 so hidden + 5 fraction bits
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
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
/* TODO: How to unify this with posit quires so they can co-exist in the same code
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
*/
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
