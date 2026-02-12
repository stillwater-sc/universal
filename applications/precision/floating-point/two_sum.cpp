// two_sum.cpp: TwoSum evaluation of posit number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_suite.hpp>

/*
important properties of linear floating point arithmetic:
 - integers are represented exactly
 - float(x - y) = x - y when x/2 <= y <= 2x: 
        difference is represented exactly when two numbers are less than 2x of each other
 - float(2x)    = 2x barring overflow
 - float(x/2)   = x/2 barring underflow

These properties derive from multiply and divide by 2 are
equivalent to shifting left and right, respectively. 
Left shift introduces an extra 0 bit and thus is guaranteed
to be represented in the encoding.
Right shift removes a bit and is also guaranteed to be
representable. 
The difference with the conditions stated, also guarantees
that bits are only removed from the representation and
the result can be faithfully reproduced.

TwoSum denotes an algorithm introduced by Knuth 
in "The Art of Computer Programming", vol 2, Seminumerical Algorithms.
that takes advantage of this property to capture any
rounding error.

Given two floating point values a and b, 
generate a rounded sum s and a remainder r, such that
    s = RoundToNearest(a + b), and
    a + b = s + r

*/

template<unsigned nbits, unsigned es>
void ReportTwoSumError(const std::string& test_case, const std::string& op, const sw::universal::posit<nbits, es>& a, const sw::universal::posit<nbits, es>& b, const sw::universal::posit<nbits, es>& s, const sw::universal::posit<nbits, es>& r) {
	std::cerr << test_case << " "
		<< std::setw(nbits) << a.bits()
		<< " " << op << " "
		<< std::setw(nbits) << b.bits()
		<< " != "
		<< std::setw(nbits) << s.bits()
		<< " " << op << " "
		<< std::setw(nbits) << r.bits()
		<< " instead it yielded "
		<< std::setw(nbits) << (a + b).bits()
		<< " vs "
		<< std::setw(nbits) << (s + r).bits()
		<< std::endl;
}

template<typename Scalar>
bool GenerateTwoSumTestCase(const Scalar& a, const Scalar& b) {
	constexpr unsigned nbits = a.nbits;

	Scalar s = a + b;
	Scalar aApprox = s - b;
	Scalar bApprox = s - aApprox;
	Scalar aDiff = a - aApprox;
	Scalar bDiff = b - bApprox;
	Scalar r = aDiff + bDiff;
	std::cout << "a                      : " << std::setw(nbits) << a.bits()       << " : " << a << std::endl;
	std::cout << "b                      : " << std::setw(nbits) << b.bits()       << " : " << b << std::endl;
	std::cout << "s                      : " << std::setw(nbits) << s.bits()       << " : " << s << std::endl;
	std::cout << "aApprox = s - a        : " << std::setw(nbits) << aApprox.bits() << " : " << aApprox << std::endl;
	std::cout << "bApprox = s - aApprox  : " << std::setw(nbits) << bApprox.bits() << " : " << bApprox << std::endl;
	std::cout << "aDiff = a - aApprox    : " << std::setw(nbits) << aDiff.bits()   << " : " << aDiff << std::endl;
	std::cout << "bDiff = b - bApprox    : " << std::setw(nbits) << bDiff.bits()   << " : " << bDiff << std::endl;
	std::cout << "r = aDiff + bDiff      : " << std::setw(nbits) << r.bits()       << " : " << r << std::endl;
	std::cout << "s + r                  : " << std::setw(nbits) << (s + r).bits() << " : " << (s + r) << std::endl;
	std::cout << "a + b                  : " << std::setw(nbits) << (a + b).bits() << " : " << (a + b) << std::endl;
	Scalar a_and_b = a + b;
	Scalar s_and_r = s + r;
	bool equal = (a_and_b == s_and_r);
	std::cout << (equal ? " PASS\n" : " FAIL\n");
	return equal;
}

// enumerate all addition cases for a posit configuration: is within 10sec till about nbits = 14
template<unsigned nbits, unsigned es>
int ValidateTwoSum(bool reportTestCases) {
	const unsigned NR_POSITS = (unsigned(1) << nbits);
	int nrOfFailedTests = 0;
	using Posit = sw::universal::posit<nbits, es>;
	Posit pa, pb, ps, pr, psum, pref;
	std::pair<Posit, Posit> s_and_r;
	for (size_t i = 0; i < NR_POSITS; i++) {
		pa.setbits(i);
		for (size_t j = 0; j < NR_POSITS; j++) {
			pb.setbits(j);

			s_and_r = sw::universal::twoSum(pa, pb);
			ps = s_and_r.first;
			pr = s_and_r.second;
			pref = ps + pr;
			psum = pa + pb;

//			std::cout << pa << " + " << pb << " = " << ps << " + " << pr << "    " << psum << " vs " << pref << '\n';

			if (psum != pref) {
				nrOfFailedTests++;
				if (reportTestCases)	ReportTwoSumError("FAIL", "+", pa, pb, ps, pr);
			}
			else {
				// if (reportTestCases) ReportBinaryArithmeticSuccess("PASS", "+", pa, pb, pref, psum);
			}
		}
	}

	return nrOfFailedTests;
}


#define MANUAL_TEST 0

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "application two-sum";
	std::string test_tag    = "two-sum";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TEST

	constexpr unsigned nbits = 8;
	constexpr unsigned es = 1;
	using Posit = posit<nbits, es>;
	Posit a, b(SpecificValue::minpos);
	a = b;
	GenerateTwoSumTestCase(a, b);
	GenerateTwoSumTestCase(-a, -b);
	++a;
	GenerateTwoSumTestCase(a, b);
	++b;
	GenerateTwoSumTestCase(a, b);
	a.minpos();
	std::cout << a.bits() << " : " << a << " : sum(a,a) " << a + a << " : " << (a + a).bits() << '\n';
	++a;
	std::cout << a.bits() << " : " << a << " : sum(a,a) " << a + a << " : " << (a + a).bits() << '\n';
	++a;
	std::cout << a.bits() << " : " << a << " : sum(a,a) " << a + a << " : " << (a + a).bits() << '\n';

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS; // ignore failures
#else
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<2, 0>(reportTestCases), "posit<2,0>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<3, 0>(reportTestCases), "posit<3,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<3, 1>(reportTestCases), "posit<3,1>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<4, 0>(reportTestCases), "posit<4,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<4, 1>(reportTestCases), "posit<4,1>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<4, 2>(reportTestCases), "posit<4,2>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<5, 0>(reportTestCases), "posit<5,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<5, 1>(reportTestCases), "posit<5,1>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<5, 2>(reportTestCases), "posit<5,2>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<5, 3>(reportTestCases), "posit<5,3>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 0>(reportTestCases), "posit<6,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 1>(reportTestCases), "posit<6,1>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 2>(reportTestCases), "posit<6,2>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 3>(reportTestCases), "posit<6,3>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<6, 4>(reportTestCases), "posit<6,4>", "twoSum");

	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 0>(reportTestCases), "posit<8,0>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 1>(reportTestCases), "posit<8,1>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 2>(reportTestCases), "posit<8,2>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 3>(reportTestCases), "posit<8,3>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 4>(reportTestCases), "posit<8,4>", "twoSum");
	nrOfFailedTestCases += ReportTestResult(ValidateTwoSum<8, 5>(reportTestCases), "posit<8,5>", "twoSum");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

	//return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
	return EXIT_SUCCESS;  // standard posits fail the twoSum test of floating-point, so ignore failures
#endif // MANUAL_TEST
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
