// two_sum.cpp: TwoSum evaluation of posit number systems
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#define ALIASING_ALLOWED
#include "common.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"

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

namespace sw {
	namespace unum {
		template<typename Scalar>
		std::pair<Scalar, Scalar> twoSum2(const Scalar& a, const Scalar& b) {
			Scalar s = a + b;
			Scalar aApprox = s - b;
			Scalar bApprox = s - aApprox;
			Scalar aDiff = a - aApprox;
			Scalar bDiff = b - bApprox;
			Scalar r = aDiff + bDiff;
			return std::pair<Scalar, Scalar>(s, r);
		}

		template<size_t nbits, size_t es>
		std::pair< posit<nbits, es>, posit<nbits, es> > twoSum(const posit<nbits, es>& a, const posit<nbits, es>& b) {
			if ((minpos<nbits, es>() == a && minpos<nbits, es>() == b) || (maxpos<nbits, es>() == a && maxpos<nbits, es>() == b)) {
				return std::pair< posit<nbits, es>, posit<nbits, es> >(a, b);
			}
			using Scalar = posit<nbits, es>;
			Scalar s = a + b;
			Scalar aApprox = s - b;
			Scalar bApprox = s - aApprox;
			Scalar aDiff = a - aApprox;
			Scalar bDiff = b - bApprox;
			Scalar r = aDiff + bDiff;
			return std::pair<Scalar, Scalar>(s, r);
		}
	}
}

template<size_t nbits, size_t es>
void ReportTwoSumError(std::string test_case, std::string op, const sw::unum::posit<nbits, es>& a, const sw::unum::posit<nbits, es>& b, const sw::unum::posit<nbits, es>& s, const sw::unum::posit<nbits, es>& r) {
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
		<< " : " << sw::unum::pretty_print(s + r)
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
	return a_and_b == s_and_r;
}

// enumerate all addition cases for a posit configuration: is within 10sec till about nbits = 14
template<size_t nbits, size_t es>
int ValidateTwoSum(std::string tag, bool bReportIndividualTestCases) {
	using namespace std;
	const size_t NR_POSITS = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	using Posit = sw::unum::posit<nbits, es>;
	Posit pa, pb, ps, pr, psum, pref;
	pair<Posit, Posit> s_and_r;
	double da, db;
	for (size_t i = 0; i < NR_POSITS; i++) {
		pa.set_raw_bits(i);
		da = double(pa);
		for (size_t j = 0; j < NR_POSITS; j++) {
			pb.set_raw_bits(j);
			db = double(pb);

			s_and_r = sw::unum::twoSum(pa, pb);
			ps = s_and_r.first;
			pr = s_and_r.second;
			pref = ps + pr;
			psum = pa + pb;

			if (psum != pref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportTwoSumError("FAIL", "+", pa, pb, ps, pr);
			}
			else {
				//if (bReportIndividualTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, pref, psum);
			}
		}
	}

	return nrOfFailedTests;
}

#define MANUAL_TEST 1

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	// print detailed bit-level computational intermediate results
	bool verbose = false;

	int nrOfFailedTestCases = 0;
	bool bReportIndividualTestCases = true;
	std::string tag = "TwoSum failed: ";

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	cout << "Posit TwoSum validation" << endl;

#if MANUAL_TEST
	/*
	constexpr size_t nbits = 8;
	constexpr size_t es = 1;
	using Posit = posit<nbits, es>;
	Posit a, b;
	a = b = minpos<nbits, es>();
	cout << "geometric rounding region\n";
	cout << (GenerateTwoSumTestCase(a, b) ? "PASS\n" : "FAIL\n");
	cout << (GenerateTwoSumTestCase(a, ++b) ? "PASS\n" : "FAIL\n");
	cout << (GenerateTwoSumTestCase(++a, b) ? "PASS\n" : "FAIL\n");
	cout << endl << "arithmetic rounding region\n";
	a = 1.0;
	b = 1.0;
	++b;
	cout << (GenerateTwoSumTestCase(a, b) ? "PASS\n" : "FAIL\n");
	*/
	
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 0>(tag, bReportIndividualTestCases), "posit<8,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 1>(tag, bReportIndividualTestCases), "posit<8,1>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 2>(tag, bReportIndividualTestCases), "posit<8,2>", "twoSum");
//	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 3>(tag, bReportIndividualTestCases), "posit<8,3>", "twoSum");
//	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 4>(tag, bReportIndividualTestCases), "posit<8,4>", "twoSum");
//	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 5>(tag, bReportIndividualTestCases), "posit<8,5>", "twoSum");

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
#endif MANUAL_TEST

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
