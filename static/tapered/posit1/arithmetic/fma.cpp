// arithmetic_fma.cpp: test suite runner for fused-multiply-add
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <cstdint>	// uint8_t, etc.
#include <cmath>	// for frexp/frexpf and std::fma
#include <cfenv>	// feclearexcept/fetestexcept

// enable/disable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/posit1/posit1.hpp>
#include <universal/verification/posit_test_suite.hpp>
#include <universal/verification/posit_test_suite_mathlib.hpp>

// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_sub
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b, Ty c) {
	Ty ref;
	sw::universal::posit<nbits, es> pa, pb, pc, pref, pfma;
	pa = a;
	pb = b;
    pc = c;
	ref = std::fma(a,b,c);
	pref = ref;
	pfma = sw::universal::fma(pa,pb,pc);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " * " << std::setw(nbits) << b << " + " << std::setw(nbits) << c << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << std::setw(nbits) << pa << " * " << std::setw(nbits) << pb << " + " << std::setw(nbits) << pc << " = " << std::setw(nbits) << pref << std::endl;
	std::cout << pa.get() << " * " << pb.get() << " + " << pc.get() << " = " << pfma.get() << " (reference: " << pref.get() << ")  ";
	std::cout << (pref == pfma ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
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

// forward references
void ReportSizeof();
void ReportFmaResults();
void ReportErrors();

// TODO
int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "posit fma verification";
	std::string test_tag    = "fma";
	bool reportTestCases    = false;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

#if MANUAL_TESTING

	ReportSizeof();
	ReportFmaResults();
	ReportErrors();

	{
		double da(0.25), db(0.0), dc(0.0);
		posit<64, 3> pa, pb, pc, pfma;
		pa = da;
		pb = db;
		pc = dc;
		pfma = sw::universal::fma(pa, pb, pc);
		if (da*db + dc != 0.0)  std::cout << "Incorrect:  ";
		std::cout << pfma << " : " << (long double)(pfma) << '\n';
	}

	{
		double da(0.25), db(0.0), dc(1.0);
		posit<64, 3> pa, pb, pc, pfma;
		pa = da;
		pb = db;
		pc = dc;
		pfma = sw::universal::fma(pa, pb, pc);
		if (da*db + dc != 1.0)  std::cout << "Incorrect:  ";
		std::cout << pfma << " : " << (long double)(pfma) << '\n';
	}

	{
		// this is not a good test case, because 0.1 is not representable in binary so you get round-off in the conversion
		GenerateTestCase<16, 1, double>(0.1, 10, -1);
		GenerateTestCase<32, 2, double>(0.1, 10, -1);
		GenerateTestCase<64, 3, double>(0.1, 10, -1);
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return EXIT_SUCCESS;
#else

#if REGRESSION_LEVEL_1
	nrOfFailedTestCases += ReportTestResult(ValidateFMA<2, 0>(tag, reportTestCases), "posit<2,0>", "fused multiply-accumulate");
#endif

#if REGRESSION_LEVEL_2
#endif

#if REGRESSION_LEVEL_3
#endif

#if REGRESSION_LEVEL_4
#endif

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);

#endif  // MANUAL_TESTING
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
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


void ReportSizeof() 
{
	using namespace sw::universal;
	using namespace sw::universal::internal;

	posit< 8, 0> p8_0;
	positRegime<8, 0> r8_0;
	positExponent<8, 0> e8_0;
	positFraction<8> f8_0;
	internal::value<8> v8;
	posit<16, 1> p16_1;
	internal::value<16> v16;
	posit<32, 2> p32_2;
	internal::value<32> v32;
	positRegime<32, 2> r32_2;
	positExponent<32, 2> e32_2;
	positFraction<32> f32_2;
	posit<64, 3> p64_3;
	internal::value<64> v64;

	std::cout << "sizeof(posit< 8,0>)    = " << sizeof(p8_0)  << " bytes\n";
	std::cout << "sizeof(posit<16,1>)    = " << sizeof(p16_1) << " bytes\n";
	std::cout << "sizeof(posit<32,2>)    = " << sizeof(p32_2) << " bytes\n";
	std::cout << "sizeof(posit<64,3>)    = " << sizeof(p64_3) << " bytes\n";

	std::cout << "sizeof(regime< 8,0>)   = " << sizeof(r8_0) << " bytes\n";
	std::cout << "sizeof(exponent< 8,0>) = " << sizeof(e8_0) << " bytes\n";
	std::cout << "sizeof(fraction< 8,0>) = " << sizeof(f8_0) << " bytes\n";

	std::cout << "sizeof(regime<32,2>)   = " << sizeof(r32_2) << " bytes\n";
	std::cout << "sizeof(exponent<32,2>) = " << sizeof(e32_2) << " bytes\n";
	std::cout << "sizeof(fraction<32,2>) = " << sizeof(f32_2) << " bytes\n";

	std::cout << "sizeof(value<8 >)      = " << sizeof(v8)  << " bytes\n";
	std::cout << "sizeof(value<16>)      = " << sizeof(v16) << " bytes\n";
	std::cout << "sizeof(value<32>)      = " << sizeof(v32) << " bytes\n";
	std::cout << "sizeof(value<64>)      = " << sizeof(v64) << " bytes\n";

//	std::cout << "sizeof(bitset<8 >)     = " << sizeof(std::bitset<8>) << " bytes\n";
//	std::cout << "sizeof(bitset<16>)     = " << sizeof(std::bitset<16>) << " bytes\n";
//	std::cout << "sizeof(bitset<32>)     = " << sizeof(std::bitset<32>) << " bytes\n";
//	std::cout << "sizeof(bitset<64>)     = " << sizeof(std::bitset<64>) << " bytes\n";

	std::cout << "sizeof(bitblock< 4>)   = " << sizeof(bitblock<4>) << " bytes\n";
	std::cout << "sizeof(bitblock< 8>)   = " << sizeof(bitblock<8>) << " bytes\n";
	std::cout << "sizeof(bitblock<16>)   = " << sizeof(bitblock<16>) << " bytes\n";
	std::cout << "sizeof(bitblock<32>)   = " << sizeof(bitblock<32>) << " bytes\n";
	std::cout << "sizeof(bitblock<48>)   = " << sizeof(bitblock<48>) << " bytes\n";
	std::cout << "sizeof(bitblock<64>)   = " << sizeof(bitblock<64>) << " bytes\n";
	std::cout << "sizeof(bitblock<80>)   = " << sizeof(bitblock<80>) << " bytes\n";
	std::cout << "sizeof(bitblock<96>)   = " << sizeof(bitblock<96>) << " bytes\n";
	std::cout << "sizeof(bitblock<112>)  = " << sizeof(bitblock<112>) << " bytes\n";
	std::cout << "sizeof(bitblock<128>)  = " << sizeof(bitblock<128>) << " bytes\n";

	std::cout << "sizeof(posit< 4,0>)    = " << sizeof(posit<4,0>) << " bytes\n";
	std::cout << "sizeof(posit< 8,0>)    = " << sizeof(posit<8,0>) << " bytes\n";
	std::cout << "sizeof(posit<16,1>)    = " << sizeof(posit<16,1>) << " bytes\n";
	std::cout << "sizeof(posit<32,2>)    = " << sizeof(posit<32,2>) << " bytes\n";
	std::cout << "sizeof(posit<48,2>)    = " << sizeof(posit<48,2>) << " bytes\n";
	std::cout << "sizeof(posit<64,3>)    = " << sizeof(posit<64,3>) << " bytes\n";
	std::cout << "sizeof(posit<80,3>)    = " << sizeof(posit<80,3>) << " bytes\n";
	std::cout << "sizeof(posit<96,3>)    = " << sizeof(posit<96,3>) << " bytes\n";
	std::cout << "sizeof(posit<112,4>)   = " << sizeof(posit<112,4>) << " bytes\n";
	std::cout << "sizeof(posit<128,4>)   = " << sizeof(posit<128,4>) << " bytes\n";

	std::cout << "sizeof(bool)           = " << sizeof(bool) << " bytes\n";
	std::cout << "sizeof(uint8_t)        = " << sizeof(uint8_t) << " bytes\n";
	std::cout << "sizeof(uint16_t)       = " << sizeof(uint16_t) << " bytes\n";
	std::cout << "sizeof(uint32_t)       = " << sizeof(uint32_t) << " bytes\n";
	std::cout << "sizeof(uint64_t)       = " << sizeof(uint64_t) << " bytes\n";
}

void ReportFmaResults()
{
	// measure the difference between fma and built-in operators
	double in = 0.1;
	std::cout << "0.1 double is " << std::setprecision(23) << in
		<< " (" << std::hexfloat << in << std::defaultfloat << ")\n"
		<< "0.1*10 is 1.0000000000000000555112 (0x8.0000000000002p-3), "
		<< "or 1.0 if rounded to double\n";
	double expr_result = 0.1 * 10 - 1;
	double fma_result = std::fma(0.1, 10, -1);
	std::cout << "0.1 * 10 - 1 = " << expr_result
		<< " : 1 subtracted after intermediate rounding\n"
		<< "fma(0.1, 10, -1) = " << std::setprecision(6) << fma_result << " ("
		<< std::hexfloat << fma_result << std::defaultfloat << ")\n\n";
}

void ReportErrors()
{
	// fma is used in double-double arithmetic
	double high = 0.1 * 10;
	double low = std::fma(0.1, 10, -high);
	std::cout << "in double-double arithmetic, 0.1 * 10 is representable as "
		<< high << " + " << low << "\n\n";

	// error handling 
	std::feclearexcept(FE_ALL_EXCEPT);
	std::cout << "fma(+Inf, 10, -Inf) = " << std::fma(INFINITY, 10, -INFINITY) << '\n';
	if (std::fetestexcept(FE_INVALID))
		std::cout << "    FE_INVALID raised\n";
}
