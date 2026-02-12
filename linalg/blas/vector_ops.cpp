// vector_ops.cpp: example program to show sw::universal::blas::vector operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// enable the following define to show the intermediate steps in the fused-dot product
// #define ALGORITHM_VERBOSE_OUTPUT
//#define ALGORITHM_TRACE_MUL
//#define QUIRE_TRACE_ADD
// configure posit environment using fast posits
#define POSIT_FAST_POSIT_8_0 1
#define POSIT_FAST_POSIT_16_1 1
#define POSIT_FAST_POSIT_32_2 1
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/lns/lns.hpp>
#include <blas/blas.hpp>
#include <universal/verification/test_suite.hpp>

template<unsigned nbits, unsigned es>
void PrintProducts(const sw::numeric::containers::vector<sw::universal::posit<nbits,es>>& a,
		           const sw::numeric::containers::vector<sw::universal::posit<nbits,es>>& b)
{
	sw::universal::quire<nbits, es> q(0);
	for (unsigned i = 0; i < a.size(); ++i) {
		q += sw::universal::quire_mul(a[i], b[i]);
		std::cout << a[i] << " * " << b[i] << " = " << a[i] * b[i] << std::endl << "quire " << q << std::endl;
	}
	sw::universal::posit<nbits,es> sum;
	sw::universal::convert(q.to_value(), sum);     // one and only rounding step of the fused-dot product
	std::cout << "fdp result " << sum << std::endl;
}

template<typename Scalar>
int VerifyErrorFreeFusedDotProduct(Scalar maxpos) {
	using namespace sw::universal;
	using namespace sw::numeric::containers;

	// Setting up a dot product with catastrophic cancellation
	// 	   a:   maxpos     1       1    ...    1     maxpos
	// 	   b:    -1     epsilon epsilon ... epsilon    1
	// 	The two maxpos values will cancel out leaving the 32k epsilon's accumulated
	// 	The dot product will experience catastrophic cancellation, 
	//  fdp will calculate the sum of products correctly
	using namespace sw::blas;
	constexpr unsigned vectorSize = SIZE_32K + 2;
	vector<Scalar> a(vectorSize), b(vectorSize);
	Scalar epsilon = std::numeric_limits<Scalar>::epsilon();
	for (unsigned i = 1; i < vectorSize - 1; ++i) {
		a[i] = 1;
		b[i] = epsilon;
	}
	a[0] = a[vectorSize - 1] = maxpos;
	b[0] = -1;  b[vectorSize - 1] = 1;
	std::cout << "a:   maxpos     1       1    ...    1     maxpos\n";
	std::cout << "b:    -1     epsilon epsilon ... epsilon    1\n";
	ReportValue(a[0], "a[0]");
	ReportValue(b[0], "b[0]");
	ReportValue(a[1], "a[1]");
	ReportValue(b[1], "b[1]");

	// dot: 0
	// fdp: 0.000244141
	Scalar errorFullDot = dot(a, b);
	Scalar errorFreeFDP = fdp(a, b);
	std::cout << "\naccumulation of 32k epsilons (" << epsilon << ") for a " << type_tag(Scalar()) << " yields:\n";
	std::cout << "dot            : " << errorFullDot << " : " << to_binary(errorFullDot) << '\n';
	std::cout << "fdp            : " << errorFreeFDP << " : " << to_binary(errorFreeFDP) << '\n';
	Scalar validation = (vectorSize - 2) * epsilon;
	std::cout << "32k * epsilon  : " << validation << " : " << to_binary(validation) << '\n';

	return (validation != errorFreeFDP) ? 1 : 0;
}

template<typename Scalar>
int VerifyVectorScale(unsigned vectorSize ) {
	// scale a vector
	using namespace sw::numeric::containers;

	vector<Scalar> a(vectorSize), b(vectorSize);
	Scalar epsilon = std::numeric_limits<Scalar>::epsilon();

	for (unsigned i = 0; i < vectorSize; ++i) {
		a[i] = 1;
		b[i] = epsilon;
	}
	a *= epsilon; // a * epsilon -> b
	bool success = true;
	for (unsigned i = 0; i < size(a); ++i) {
		if (a[i] != b[i]) {
			std::cout << a[i] << " != " << b[i] << '\n';
			success = false;
			break;
		}
	}
	return (success ? 0 : 1);
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 0
// REGRESSION_LEVEL_OVERRIDE is set by the cmake file to drive a specific regression intensity
// It is the responsibility of the regression test to organize the tests in a quartile progression.
//#undef REGRESSION_LEVEL_OVERRIDE
#ifndef REGRESSION_LEVEL_OVERRIDE
#undef REGRESSION_LEVEL_1
#undef REGRESSION_LEVEL_2
#undef REGRESSION_LEVEL_3
#undef REGRESSION_LEVEL_4
#define REGRESSION_LEVEL_1 1
#define REGRESSION_LEVEL_2 1
#define REGRESSION_LEVEL_3 1
#define REGRESSION_LEVEL_4 1
#endif

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "error free FDP";
	std::string test_tag    = "fdp";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	std::cout << "error full and error free dot products\n";
	// posit<8,0> is failing on 32k sums of epsilon
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFreeFusedDotProduct(std::numeric_limits<posit<8, 2> >::max()), test_tag, "error free posit<8,2> dot");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFreeFusedDotProduct(std::numeric_limits<posit<16, 2> >::max()), test_tag, "error free posit<16,2> dot");
	nrOfFailedTestCases += ReportTestResult(VerifyErrorFreeFusedDotProduct(std::numeric_limits<posit<32, 2> >::max()), test_tag, "error free posit<32,2> dot");
	// TBD: no fdp yet for cfloat or lns
	// nrOfFailedTestCases += ReportTestResult(VerifyErrorFreeFusedDotProduct(std::numeric_limits< bfloat_t >::max()), test_tag, "error free bfloat16 dot");
	// nrOfFailedTestCases += ReportTestResult(VerifyErrorFreeFusedDotProduct(std::numeric_limits< lns<16, 8> >::max()), test_tag, "error free lns dot");

	std::cout << "Verify Vector scaling for different arithmetic types\n";
	nrOfFailedTestCases += ReportTestResult(VerifyVectorScale< posit<32, 2> >(100), "vector scale", "scale posit vector");
	nrOfFailedTestCases += ReportTestResult(VerifyVectorScale< bfloat_t >(100), "vector scale", "scale bfloat16 vector");
	nrOfFailedTestCases += ReportTestResult(VerifyVectorScale< lns<16, 8> >(100), "vector scale", "scale lns vector");

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
// LCOV_EXCL_START
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
// LCOV_EXCL_STOP
