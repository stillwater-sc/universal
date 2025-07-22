// serialization.cpp: test suite for serialization functions for data exchange
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <universal/number_systems.hpp>
#include <blas/blas.hpp>
#include <blas/generators.hpp>
#include <blas/serialization/datafile.hpp>
#include <universal/verification/test_suite.hpp>
#include <math/constants/double_constants.hpp>

void ReportNativeHexFormats() {
	using namespace sw::universal;

	float f;
	double d;
	single b;
	b.setbits(0x23456789);
	f = float(b);
	d = double(b);
	std::cout << to_hex(f, true, true) << '\n';
	std::cout << to_hex(d, true, true) << '\n';
	std::cout << to_binary(f) << " : " << to_hex(f) << " : " << f << '\n';
	std::cout << to_binary(d) << " : " << to_hex(d) << " : " << d << '\n';
	std::cout << to_binary(b) << " : " << to_hex(b) << " : " << b << '\n';
}

void ReportNumberSystemFormats() {
	using namespace sw::universal;

	{
		constexpr unsigned nbits = 40;
		integer<nbits, uint32_t, IntegerNumberType::NaturalNumber> a{123456789};
		integer<nbits, uint32_t, IntegerNumberType::WholeNumber> b{123456789};
		integer<nbits, uint32_t, IntegerNumberType::IntegerNumber> c{-123456789};
		ReportFormats(a);
		ReportFormats(b);
		ReportFormats(c);
	}
	{
		fixpnt<40, 32, Modulo, uint32_t> a(d_pi);
		ReportFormats(a);
		ReportFormats(-a);
		fixpnt<40, 32, Saturate, uint32_t> b(d_pi);
		ReportFormats(b);

	}
	{
		cfloat<64, 11, uint32_t, true, false, false> a(d_pi);
		ReportFormats(a);
	}
	{
		posit<64, 5> a(d_pi);
		ReportFormats(a);
	}
	{
		lns<64, 32> a(d_pi);
		ReportFormats(a);
	}
	{
		dbns<8,3,uint8_t> a(d_pi);
		ReportFormats(a);
		dbns<10, 4, uint8_t> b(d_pi);
		ReportFormats(b);
		dbns<12, 5, uint8_t> c(d_pi);
		ReportFormats(c);
	}
}

void TestSaveTypeId() {
	using namespace sw::universal;

	blas::saveTypeId<char>(std::cout);
	blas::saveTypeId<short>(std::cout);
	blas::saveTypeId<int>(std::cout);
	blas::saveTypeId<long>(std::cout);
	blas::saveTypeId<long long>(std::cout);
	blas::saveTypeId<float>(std::cout);
	blas::saveTypeId<double>(std::cout);
	blas::saveTypeId<long double>(std::cout);
	blas::saveTypeId<integer<  8, uint8_t, IntegerNumberType::IntegerNumber>>(std::cout);
	blas::saveTypeId<integer< 16, uint16_t, IntegerNumberType::IntegerNumber>>(std::cout);
	blas::saveTypeId<integer< 32, uint32_t, IntegerNumberType::IntegerNumber>>(std::cout);
	blas::saveTypeId<integer< 64, uint32_t, IntegerNumberType::IntegerNumber>>(std::cout);
	blas::saveTypeId<integer<128, uint32_t, IntegerNumberType::IntegerNumber>>(std::cout);

	blas::saveTypeId<fixpnt<32, 16, Modulo>>(std::cout);
	blas::saveTypeId<fixpnt<64, 32, Saturate>>(std::cout);

	blas::saveTypeId<cfloat<12, 8, uint16_t, true, true, false>>(std::cout);
	blas::saveTypeId<quarter>(std::cout);
	blas::saveTypeId<half>(std::cout);
	blas::saveTypeId<single>(std::cout);
	blas::saveTypeId<duble>(std::cout);
	blas::saveTypeId<quad>(std::cout);

	blas::saveTypeId<posit<  8, 2>>(std::cout);
	blas::saveTypeId<posit< 16, 2>>(std::cout);
	blas::saveTypeId<posit< 32, 2>>(std::cout);
	blas::saveTypeId<posit< 64, 2>>(std::cout);
	blas::saveTypeId<posit<128, 2>>(std::cout);
	blas::saveTypeId<posit<256, 2>>(std::cout);

	blas::saveTypeId<lns<16, 8, uint16_t>>(std::cout);
	blas::saveTypeId<dbns<8, 3, uint8_t>>(std::cout);

	// or this ADL format
	half h;
	blas::saveTypeId(std::cout, h);
}

template<typename Scalar>
void TestVectorSerialization() {
	using namespace sw::universal;
	sw::universal::blas::vector<Scalar> v(5);
	gaussian_random(v, 0.0, 0.1);
	blas::datafile<blas::TextFormat> df;
	df.add(v, "testVector");
	std::cout << "datafile with a single vector(5) serialized using decimal format\n";
	df.save(std::cout, false);  // decimal format
	std::cout << "+--------------- end of df serialization using decimal ----\n";

	std::stringstream s;
	df.save(s, false);  // decimal format

	df.clear();
	df.restore(s);

	std::cout << "same datafile with a single vector(5) serialized using hex format\n";
	df.save(std::cout, true);
	std::cout << "+--------------- end of TestVectorSerialization -------------+\n";
}

template<typename Scalar>
void TestMatrixSerialization() {
	using namespace sw::universal;
	sw::universal::blas::matrix<Scalar> m(5,5);
	gaussian_random(m, 0.0, 0.1);
	blas::datafile<blas::TextFormat> df;
	df.add(m, "testMatrix");
	std::cout << "datafile with a single matrix(5,5) serialized using decimal format\n";
	df.save(std::cout, false);  // decimal format
	std::cout << "+--------------- end of df serialization using decimal ----\n";

	std::stringstream s;
	df.save(s, false);  // decimal format

	df.clear();
	df.restore(s);

	std::cout << "same datafile with a single matrix(5,5) serialized using hex format\n";
	df.save(std::cout, true);
	std::cout << "+--------------- end of TestMatrixSerialization -------------+\n";
}

void TestCollectionSerialization() {
	using namespace sw::universal;

	// Create instances of different specialized collections
	sw::universal::blas::vector<float> xfp32(7), yfp32(7);
	sw::universal::blas::matrix<float> Afp32(9, 5);
//	sw::universal::blas::tensor<float> Tfp32(5, 5); // TBD
	sw::universal::blas::matrix<float> dpfp32(1, 1);
	gaussian_random(xfp32, 0.0, 0.1);
	gaussian_random(yfp32, 0.0, 0.1);
	gaussian_random(Afp32, 0.0, 1.0);
	auto zfp32 = Afp32 * xfp32;
	dpfp32 = xfp32 * yfp32;
	sw::universal::blas::vector<half> x(7), y(7);
	sw::universal::blas::matrix<half> A(5, 7);
	gaussian_random(A, 0.0, 1.0);
	sw::universal::blas::matrix<half> dotProduct(1, 1);
	x = xfp32;
	y = yfp32;
	A = Afp32;
	auto z = A * x;
	dotProduct = x * y;

	// Use the base class reference to aggregate the collections
	blas::datafile<blas::TextFormat> df;
//	df.add(Tfp32);
	df.add(xfp32, "xfp32");
	df.add(yfp32, "yfp32");
	df.add(Afp32, "Afp32");
	df.add(dpfp32, "dpfp32");
	df.add(x, "xhalf");
	df.add(y, "yhalf");
	df.add(A, "Ahalf");
	df.add(z, "zhalf");
	df.add(dotProduct, "dotProduct_xy");
	df.save(std::cout, false);  // decimal format

	std::ofstream fo;
	fo.open("TestCollectionSerialization.txt");
	df.save(fo, false);
	fo.close();

	std::stringstream s;
	df.save(s, false);
	blas::datafile<blas::TextFormat> in;
	if (!in.restore(s)) {
		std::cerr << "Failed to load Universal Data File\n";
	}
	else {
		in.save(std::cout, false);
	}
	std::cout << "+--------------- end of TestCollectionSerialization -------------+\n";
}

// Regression testing guards: typically set by the cmake configuration, but MANUAL_TESTING is an override
#define MANUAL_TESTING 1
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

	std::string test_suite = "serialization";
	std::string test_tag = "save/restore";
	bool reportTestCases = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	// manual test cases
	//nrOfFailedTestCases += ReportTestResult(VerifyCompress<quarter>(reportTestCases), "compress to quarter precision", "quarter precision");
	
	// ReportNativeHexFormats();
	// ReportNumberSystemFormats();

	constexpr unsigned m = 9;
	constexpr unsigned n = 5;
	blas::matrix<double> A(m, n), B(m, n);
	blas::gaussian_random(A, 0.0, 1.0);
	std::cout << "Matrix A\n" << A << '\n';

	std::stringstream s;
	s << A;

	std::cout << '\n';
	s >> B;

	std::cout << "Matrix B\n" << B << '\n';

	return 0;

	// TODO: datafiles are not working yet: ETLO 3/25/2024
	TestCollectionSerialization();

	TestVectorSerialization<double>();
	TestVectorSerialization<dbns<8, 3>>();
	TestMatrixSerialization<float>();
	TestMatrixSerialization<half>();

//	TestSaveTypeId();


	
	return 0;

	unsigned N = 32;
	sw::universal::blas::vector<double> x(N), y(N);
	double zeroMean = 0.0;
	double variance = 0.1;
	gaussian_random(x, zeroMean, variance);



	{
		sw::universal::blas::vector<lns<8, 2, uint8_t>> v(N);
		v = x;
		save(std::cout, v);
	}
	{
		sw::universal::blas::vector<lns<12, 4, uint8_t>> v(N);
		v = x;
		save(std::cout, v);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1

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
