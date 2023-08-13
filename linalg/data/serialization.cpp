// serialization.cpp: test suite for serialization functions for data exchange
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cmath>
#include <universal/native/ieee754.hpp>
#include <universal/native/integers.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/blas/blas.hpp>
#include <universal/blas/generators.hpp>
#include <universal/verification/test_suite.hpp>
#include <universal/math/math_constants.hpp>

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
		fixpnt<40, 32, Modulo, uint32_t> a(m_pi);
		ReportFormats(a);
		ReportFormats(-a);
		fixpnt<40, 32, Saturate, uint32_t> b(m_pi);
		ReportFormats(b);

	}
	{
		cfloat<64, 11, uint32_t, true, false, false> a(m_pi);
		ReportFormats(a);
	}
	{
		posit<64, 5> a(m_pi);
		ReportFormats(a);
	}
	{
		lns<64, 32> a(m_pi);
		ReportFormats(a);
	}
	{
		dbns<8,3,uint8_t> a(m_pi);
		ReportFormats(a);
		dbns<10, 4, uint8_t> b(m_pi);
		ReportFormats(b);
		dbns<12, 5, uint8_t> c(m_pi);
		ReportFormats(c);
	}
}

void TestSerialization() {
	using namespace sw::universal;

	// Create instances of different specialized collections
	sw::universal::blas::vector<float> xfp32(5), yfp32(5);
	sw::universal::blas::matrix<float> Afp32(5, 5);
	sw::universal::blas::tensor<float> Tfp32(5, 5); // TBD
	sw::universal::blas::matrix<float> dpfp32(1, 1);
	gaussian_random(xfp32, 0.0, 0.1);
	gaussian_random(yfp32, 0.0, 0.1);
	auto zfp32 = Afp32 * xfp32;
	dpfp32 = xfp32 * yfp32;
	sw::universal::blas::vector<half> x(5), y(5);
	sw::universal::blas::matrix<half> A(5, 5);
	sw::universal::blas::matrix<half> dotProduct(1, 1);
	x = xfp32;
	y = yfp32;
	A = Afp32;
	auto z = A * x;
	dotProduct = x * y;

	// Use the base class reference to aggregate the collections
	blas::datafile<blas::TextFormat> df;
	df.add(Tfp32);
	df.add(xfp32);
	df.add(yfp32);
	df.add(Afp32);
	df.add(dpfp32);
	df.add(x);
	df.add(y);
	df.add(z);
	df.add(dotProduct);
	df.save(std::cout, false);  // decimal format
	//		df.save(std::cout, true);   // hex format

	std::stringstream s;
	df.save(s, true);
	blas::datafile<blas::TextFormat> in;
	if (!in.restore(s)) {
		std::cerr << "Failed to load Universal Data File\n";
	}
	else {
		in.save(std::cout, true);
	}
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

	std::cout << (is_integer<integer<8>> ? "integer" : "not an integer") << '\n';
	std::cout << (is_integer<integer<16>> ? "integer" : "not an integer") << '\n';
	std::cout << (is_integer<integer<160>> ? "integer" : "not an integer") << '\n';

	blas::saveTypeId<char>(std::cout);
	blas::saveTypeId<short>(std::cout);
	blas::saveTypeId<int>(std::cout);
	blas::saveTypeId<long>(std::cout);
	blas::saveTypeId<long long>(std::cout);
	blas::saveTypeId<float>(std::cout);
	blas::saveTypeId<double>(std::cout);
	blas::saveTypeId<long double>(std::cout);
	blas::saveTypeId<integer<  8, uint8_t , IntegerNumberType::IntegerNumber>>(std::cout);
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

	TestSerialization();
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
	//nrOfFailedTestCases += ReportTestResult(VerifyCompress<lns<8,4>>(reportTestCases), "compress to lns<8,4>", "lns<8,4>");
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
