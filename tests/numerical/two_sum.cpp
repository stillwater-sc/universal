// two_sum.cpp: TwoSum evaluation of posit number systems
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include "common.hpp"
// test helpers, such as, ReportTestResults
#include "../tests/utils/test_helpers.hpp"
#include "../tests/utils/posit_test_helpers.hpp"

/*
floating point arithmetic:
 - integers are represented exactly
 - float(x - y) = x - y when x/2 <= y <= 2x: difference is represented exactly when two numbers are less than 2x of each other
 - float(2x)    = 2x barring overflow
 - float(x/2)   = x/2 barring underflow

TwoSum denotes an algorithm introduced by Knuth in "The Art of Computer Programming", vol 2, Seminumerical Algorithms.

Given two floating point values a and b, generate a rounded sum s and a remainder r, such that
s = RoundToNearest(a + b), and
a + b = s + r

*/

template<size_t nbits, size_t es>
void ReportTwoSumError(const std::string& test_case, const std::string& op, const sw::unum::posit<nbits, es>& a, const sw::unum::posit<nbits, es>& b, const sw::unum::posit<nbits, es>& s, const sw::unum::posit<nbits, es>& r) {
	std::cerr << test_case << " "
		<< std::setw(nbits) << a.get()
		<< " " << op << " "
		<< std::setw(nbits) << b.get()
		<< " != "
		<< std::setw(nbits) << s.get()
		<< " " << op << " "
		<< std::setw(nbits) << r.get()
		<< " instead it yielded "
		<< std::setw(nbits) << (a + b).get()
		<< " vs "
		<< std::setw(nbits) << (s + r).get()
		<< std::endl;
}

template<typename Scalar>
bool GenerateTwoSumTestCase(const Scalar& a, const Scalar& b) {
	constexpr size_t nbits = a.nbits;

	Scalar s = a + b;
	Scalar aApprox = s - b;
	Scalar bApprox = s - aApprox;
	Scalar aDiff = a - aApprox;
	Scalar bDiff = b - bApprox;
	Scalar r = aDiff + bDiff;
	std::cout << "a                      : " << std::setw(nbits) << a.get()       << " : " << a << std::endl;
	std::cout << "b                      : " << std::setw(nbits) << b.get()       << " : " << b << std::endl;
	std::cout << "s                      : " << std::setw(nbits) << s.get()       << " : " << s << std::endl;
	std::cout << "aApprox = s - a        : " << std::setw(nbits) << aApprox.get() << " : " << aApprox << std::endl;
	std::cout << "bApprox = s - aApprox  : " << std::setw(nbits) << bApprox.get() << " : " << bApprox << std::endl;
	std::cout << "aDiff = a - aApprox    : " << std::setw(nbits) << aDiff.get()   << " : " << aDiff << std::endl;
	std::cout << "bDiff = b - bApprox    : " << std::setw(nbits) << bDiff.get()   << " : " << bDiff << std::endl;
	std::cout << "r = aDiff + bDiff      : " << std::setw(nbits) << r.get()       << " : " << r << std::endl;
	std::cout << "s + r                  : " << std::setw(nbits) << (s + r).get() << " : " << (s + r) << std::endl;
	std::cout << "a + b                  : " << std::setw(nbits) << (a + b).get() << " : " << (a + b) << std::endl;
	Scalar a_and_b = a + b;
	Scalar s_and_r = s + r;
	bool equal = (a_and_b == s_and_r);
	std::cout << (equal ? " PASS\n" : " FAIL\n");
	return equal;
}

// enumerate all addition cases for a posit configuration: is within 10sec till about nbits = 14
template<size_t nbits, size_t es>
int ValidateTwoSum(const std::string& tag, bool bReportIndividualTestCases) {
	using namespace std;
	const size_t NR_POSITS = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	using Posit = sw::unum::posit<nbits, es>;
	Posit pa, pb, ps, pr, psum, pref;
	pair<Posit, Posit> s_and_r;
	for (size_t i = 0; i < NR_POSITS; i++) {
		pa.set_raw_bits(i);
		for (size_t j = 0; j < NR_POSITS; j++) {
			pb.set_raw_bits(j);

			s_and_r = sw::unum::twoSum(pa, pb);
			ps = s_and_r.first;
			pr = s_and_r.second;
			pref = ps + pr;
			psum = pa + pb;

//			cout << pa << " + " << pb << " = " << ps << " + " << pr << "    " << psum << " vs " << pref << endl;

			if (psum != pref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportTwoSumError("FAIL", "+", pa, pb, ps, pr);
			}
			else {
				// if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, pref, psum);
			}
		}
	}

	return nrOfFailedTests;
}

#define MANUAL_TEST 0

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

	cout << "Posit TwoSum validation" << endl;

#if MANUAL_TEST

	constexpr size_t nbits = 8;
	constexpr size_t es = 1;
	using Posit = posit<nbits, es>;
	Posit a, b;
	minpos<nbits, es>(b);
	a = b;
	GenerateTwoSumTestCase(a, b);
	GenerateTwoSumTestCase(-a, -b);
	++a;
	GenerateTwoSumTestCase(a, b);
	++b;
	GenerateTwoSumTestCase(a, b);
	minpos<nbits, es>(a);
	cout << a.get() << " : " << a << " : sum(a,a) " << a + a << " : " << (a + a).get() << endl;
	++a;
	cout << a.get() << " : " << a << " : sum(a,a) " << a + a << " : " << (a + a).get() << endl;
	++a;
	cout << a.get() << " : " << a << " : sum(a,a) " << a + a << " : " << (a + a).get() << endl;


#else
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<3, 0>(tag, bReportIndividualTestCases), "posit<3,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<3, 1>(tag, bReportIndividualTestCases), "posit<3,1>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<4, 0>(tag, bReportIndividualTestCases), "posit<4,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<4, 1>(tag, bReportIndividualTestCases), "posit<4,1>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<4, 2>(tag, bReportIndividualTestCases), "posit<4,2>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<5, 0>(tag, bReportIndividualTestCases), "posit<5,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<5, 1>(tag, bReportIndividualTestCases), "posit<5,1>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<5, 2>(tag, bReportIndividualTestCases), "posit<5,2>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<5, 3>(tag, bReportIndividualTestCases), "posit<5,3>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 0>(tag, bReportIndividualTestCases), "posit<6,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 1>(tag, bReportIndividualTestCases), "posit<6,1>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 2>(tag, bReportIndividualTestCases), "posit<6,2>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 3>(tag, bReportIndividualTestCases), "posit<6,3>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 4>(tag, bReportIndividualTestCases), "posit<6,4>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "twoSum");

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
