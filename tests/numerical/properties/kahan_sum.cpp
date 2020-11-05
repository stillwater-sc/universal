// kahan_sum.cpp: Kahan summation evaluation of posit number systems
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include "common.hpp"
// pull in the posit number system
#include <universal/posit/posit>
// test helpers, such as, ReportTestResults
#include "../tests/utils/test_helpers.hpp"
#include "../tests/utils/posit_test_helpers.hpp"

/*
floating point arithmetic:
 - integers are represented exactly
 - float(x - y) = x - y when x/2 <= y <= 2x: 
        difference is represented exactly when two numbers are less than 2x of each other
 - float(2x)    = 2x barring overflow
 - float(x/2)   = x/2 barring underflow

TwoSum denotes an algorithm introduced by Knuth 
in "The Art of Computer Programming", vol 2, Seminumerical Algorithms.

Given two floating point values a and b, 
generate a rounded sum s and a remainder r, such that
    s = RoundToNearest(a + b), and
    a + b = s + r

*/


template<typename Vector>
typename Vector::value_type KahanSummation(const Vector& data) {
	using Scalar = Vector::value_type;
	Scalar sum{ 0 };
	Scalar c{ 0 };
	for (auto v : data) {
		Scalar y = v - c;
		Scalar t = sum + y;
		c = (t - sum) - y;
		sum = t;
	}
	return sum;
}

template<typename Vector>
void GenerateData(Vector& data, const typename Vector::value_type& nrElements) {
	using Scalar = typename Vector::value_type;
	Scalar v = 1.0 / nrElements;
	data.clear();
	for (size_t i = 0; i < 10; ++i) {
		data.push_back(v);
	}
}

#define MANUAL_TEST 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = true;
	std::string tag = "TwoSum failed: ";

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	cout << "Kahan summation comparison" << endl;

#if MANUAL_TEST

	std::vector<float> data;
	GenerateData(data, 10);
	cout << KahanSummation(data) << endl;

#else
	// nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "twoSum");

#endif // MANUAL_TEST

	// restore the previous ostream precision
	cout << setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
