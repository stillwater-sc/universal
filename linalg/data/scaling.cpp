// scaling.cpp: test suite for scaling functions for data preprocessing
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

// Stillwater BLAS library
#include <blas/blas.hpp>
#include <universal/verification/test_suite.hpp>

//constexpr double pi = 3.14159265358979323846;  // best practice for C++

template<typename Scalar>
int VerifyRange(bool reportTestCases = false) {
	using namespace sw::universal;
	using namespace sw::blas;

	std::cerr << "VerifyRange\n" << minmax_range<Scalar>() << '\n';
	int nrFailedTests{ 0 };
	Scalar maxneg = std::numeric_limits<Scalar>::lowest();
	Scalar maxpos = std::numeric_limits<Scalar>::max();
	vector<Scalar> v;
	v.push_back(maxneg);
	v.push_back(-std::numeric_limits<Scalar>::min());
	v.push_back(0);
	v.push_back(std::numeric_limits<Scalar>::min());
	v.push_back(maxpos);

	auto minmax = range(v);
	if (minmax.first != maxneg && minmax.second != maxpos) {
		++nrFailedTests;
		if (reportTestCases) std::cerr << minmax.first << ", " << minmax.second << " != " << maxneg << ", " << maxpos << '\n';
	}
	else {
		std::cerr << symmetry_range<Scalar>() << '\n';
		std::cerr << std::left << std::setw(WIDTH_TYPE_TAG - 6) << type_tag(maxpos) << " range : [ " << maxneg << " ... " << maxpos << " ]\n";
	}
	return nrFailedTests;
}

/*
 * compress takes a vector of normal distributed double values and compresses
 * it into the range of a target arithmetic type.
 */

template<typename Scalar>
int VerifyCompress(bool reportTestCases = false) {
	using namespace sw::universal;
	using namespace sw::numeric::containers;
	using namespace sw::blas;

	std::cerr << "VerifyCompress\n" << minmax_range<Scalar>() << '\n';
	int nrFailedTests{ 0 };

	// to validate that compress() works, we are going to create a
	// vector in the target arithmetic type and convert it to double
	// and then scale it up. When we compress that scaled version
	// we should get back the original reference vector.

	// we are going to assume that the target arithmetic can represent
	// normal distributed data with zero mean and stddev of 1.0
	unsigned N{ 20 };
	using SrcType = double;
	SrcType mean{ 0.0 };
	SrcType stddev{ 1.0 };
	vector<SrcType> v = gaussian_random_vector<SrcType>(N, mean, stddev);
	if (N < 20) std::cout << "original vector   : " << v << '\n';

	auto maxpos = SrcType(std::numeric_limits<Scalar>::max());
	auto vminmax = arange(v);
	auto maxValue = vminmax.second;
	auto scale = sqrt(maxpos) / maxValue;
	// scale the original to 'fill' 75% of the dynamic range of the target scale
	if (N < 20) std::cout << "scale up          : " << scale << '\n';
	v *= scale;
	// assign it to the target type
	vector<Scalar> ref(v);
	v = ref; // convert the double vector to the target reference
	if (N < 20) std::cout << "converted vector  : " << v << '\n';

	vector<Scalar> compressed = compress<SrcType, Scalar>(v);
	if (N < 20) {
		std::cout << "compressed vector : " << compressed << '\n';
		for (auto e : compressed) std::cout << to_binary(e) << " : " << e << '\n';

		for (unsigned i = 0; i < N; ++i) {
			auto factor = double(compressed[i]) / v[i];
			std::cout << i << " : " << factor << '\n';
		}
	}

	return nrFailedTests;
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
	using namespace sw::blas;

	std::string test_suite  = "data preprocessing";
	std::string test_tag    = "data prop";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	std::cout << minmax_range<float>() << '\n';
	std::cout << minmax_range<half>() << '\n';  // has subnormals
	std::cout << minmax_range<cfloat<16, 5, uint16_t, false, false, false> >() << '\n'; // no subnormals
	std::cout << minmax_range<quarter>() << '\n'; // has subnormals
	std::cout << minmax_range<cfloat<8, 2, uint8_t, false, false, false> >() << '\n'; // no subnormals

	// manual test cases
	nrOfFailedTestCases += ReportTestResult(VerifyCompress<half>(reportTestCases), "compress to half precision", "half precision");
	nrOfFailedTestCases += ReportTestResult(VerifyCompress<quarter>(reportTestCases), "compress to quarter precision", "quarter precision");
	nrOfFailedTestCases += ReportTestResult(VerifyCompress<lns<8, 4>>(reportTestCases), "compress to lns<8,4>", "lns<8,4>");


	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(VerifyRange< integer<12> >(reportTestCases), "range", "range");
	nrOfFailedTestCases += ReportTestResult(VerifyRange< fixpnt<12,4> >(reportTestCases), "range", "range");
	nrOfFailedTestCases += ReportTestResult(VerifyRange< float >(reportTestCases), "range", "range");
	nrOfFailedTestCases += ReportTestResult(VerifyRange< half >(reportTestCases), "range", "range");
	nrOfFailedTestCases += ReportTestResult(VerifyRange< posit<16, 1> >(reportTestCases), "range", "range");
	nrOfFailedTestCases += ReportTestResult(VerifyRange < lns<8, 4> > (reportTestCases), "range", "range");

	nrOfFailedTestCases += ReportTestResult(VerifyCompress<half>(reportTestCases), "compress to half precision", "half precision");
	nrOfFailedTestCases += ReportTestResult(VerifyCompress<quarter>(reportTestCases), "compress to quarter precision", "quarter precision");
	nrOfFailedTestCases += ReportTestResult(VerifyCompress<lns<8,4>>(reportTestCases), "compress to lns<8,4>", "lns<8,4>");
#endif

#if REGRESSION_LEVEL_2

#endif

#if REGRESSION_LEVEL_3

#endif

#if REGRESSION_LEVEL_4

#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
#endif
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Uncaught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Uncaught universal internal exception: " << err.what() << std::endl;
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
