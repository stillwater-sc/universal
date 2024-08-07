// rounding_error_addition.cpp: rounding error comparision for addition
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/posit_test_suite.hpp>

// enumerate all addition cases for a posit configuration: is within 10sec till about nbits = 14
template<size_t nbits, size_t es>
int GenerateAdditionError(const std::string& tag, bool bReportIndividualTestCases) {
	const size_t NR_POSITS = (size_t(1) << nbits);
	int nrOfFailedTests = 0;
	using Posit = sw::universal::posit<nbits, es>;
	Posit pa, pb, psum, pref;
	//pair<Posit, Posit> s_and_r;
	double da, db;
	for (size_t i = 0; i < NR_POSITS; i++) {
		pa.setbits(i);
		da = double(pa);
		for (size_t j = 0; j < NR_POSITS; j++) {
			pb.setbits(j);
			db = double(pb);
			pref = da - db;
			psum = pa + pb;

			if (psum != pref) {
				nrOfFailedTests++;
				if (bReportIndividualTestCases)	ReportBinaryArithmeticError("FAIL", "+", pa, pb, pref, psum);
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
	using namespace sw::universal;

	//constexpr size_t nbits = 32;
	//constexpr size_t es = 2;
	//using Posit = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;
	bool bReportIndividualTestCases = false;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

#ifdef MANUAL_TEST

	GenerateAdditionError<8, 0>("error", bReportIndividualTestCases);

#else

	GenerateAdditionError<8, 0>("error", bReportIndividualTestCases);
	GenerateAdditionError<8, 1>("error", bReportIndividualTestCases);
	GenerateAdditionError<8, 2>("error", bReportIndividualTestCases);
	GenerateAdditionError<8, 3>("error", bReportIndividualTestCases);


#endif // MANUAL_TEST

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

	return EXIT_SUCCESS;
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
