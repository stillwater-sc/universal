// arithmetic_fma.cpp: functional tests for fused-multiply-add
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include "common.hpp"

// when you define POSIT_VERBOSE_OUTPUT executing an SUB the code will print intermediate results
//#define POSIT_VERBOSE_OUTPUT
#define POSIT_TRACE_SUB

// minimum set of include files to reflect source code dependencies
#include "../../posit/posit.hpp"
#include "../../posit/posit_manipulators.hpp"
#include "../tests/test_helpers.hpp"
#include "../tests/posit_test_helpers.hpp"


// generate specific test case that you can trace with the trace conditions in posit.h
// for most bugs they are traceable with _trace_conversion and _trace_sub
template<size_t nbits, size_t es, typename Ty>
void GenerateTestCase(Ty a, Ty b, Ty c) {
	Ty ref;
	sw::unum::posit<nbits, es> pa, pb, pc, pref, pfma;
	pa = a;
	pb = b;
    pc = c;
	ref = std::fma(a,b,c);
	pref = ref;
	pfma = sw::unum::fma(pa,pb,pc);
	std::cout << std::setprecision(nbits - 2);
	std::cout << std::setw(nbits) << a << " * " << std::setw(nbits) << b << " + " << std::setw(nbits) << c << " = " << std::setw(nbits) << ref << std::endl;
	std::cout << std::setw(nbits) << pa << " * " << std::setw(nbits) << pb << " + " << std::setw(nbits) << pc << " = " << std::setw(nbits) << pref << std::endl;
	std::cout << pa.get() << " * " << pb.get() << " + " << pc.get() << " = " << pfma.get() << " (reference: " << pref.get() << ")  ";
	std::cout << (pref == pfma ? "PASS" : "FAIL") << std::endl << std::endl;
	std::cout << std::setprecision(5);
}

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

// forward references
void ReportSizeof();
void ReportFmaResults();
void ReportErrors();

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	//bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "Fused Multiply-Accumulate failed: ";

#if MANUAL_TESTING

	//ReportSizeof();
	//ReportFmaResults();
	//ReportErrors():

	GenerateTestCase<16, 1, double>(0.1, 10, -1);
	GenerateTestCase<32, 2, double>(0.1, 10, -1);
	//GenerateTestCase<64, 3, double>(0.1, 10, -1);

#else

	nrOfFailedTestCases += ReportTestResult(ValidateFMA<2, 0>(tag, bReportIndividualTestCases), "posit<2,0>", "fused multiply-accumulate");


#if STRESS_TESTING

#endif

#endif

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}


void ReportSizeof() 
{
	using namespace std;
	using namespace sw::unum;

	posit< 8, 0> p8_0;
	regime<8, 0> r8_0;
	exponent<8, 0> e8_0;
	fraction<8> f8_0;
	value<8> v8;
	posit<16, 1> p16_1;
	value<16> v16;
	posit<32, 2> p32_2;
	value<32> v32;
	regime<32, 2> r32_2;
	exponent<32, 2> e32_2;
	fraction<32> f32_2;
	posit<64, 3> p64_3;
	value<64> v64;

	cout << "sizeof(posit< 8,0>)    = " << sizeof(p8_0) << " bytes" << endl;
	cout << "sizeof(posit<16,1>)    = " << sizeof(p16_1) << " bytes" << endl;
	cout << "sizeof(posit<32,2>)    = " << sizeof(p32_2) << " bytes" << endl;
	cout << "sizeof(posit<64,3>)    = " << sizeof(p64_3) << " bytes" << endl;

	cout << "sizeof(regime< 8,0>)   = " << sizeof(r8_0) << " bytes" << endl;
	cout << "sizeof(exponent< 8,0>) = " << sizeof(e8_0) << " bytes" << endl;
	cout << "sizeof(fraction< 8,0>) = " << sizeof(f8_0) << " bytes" << endl;

	cout << "sizeof(regime<32,2>)   = " << sizeof(r32_2) << " bytes" << endl;
	cout << "sizeof(exponent<32,2>) = " << sizeof(e32_2) << " bytes" << endl;
	cout << "sizeof(fraction<32,2>) = " << sizeof(f32_2) << " bytes" << endl;

	cout << "sizeof(value<8 >)      = " << sizeof(value<8>) << " bytes" << endl;
	cout << "sizeof(value<16>)      = " << sizeof(value<16>) << " bytes" << endl;
	cout << "sizeof(value<32>)      = " << sizeof(value<32>) << " bytes" << endl;
	cout << "sizeof(value<64>)      = " << sizeof(value<64>) << " bytes" << endl;

//	cout << "sizeof(bitset<8 >)     = " << sizeof(std::bitset<8>) << " bytes" << endl;
//	cout << "sizeof(bitset<16>)     = " << sizeof(std::bitset<16>) << " bytes" << endl;
//	cout << "sizeof(bitset<32>)     = " << sizeof(std::bitset<32>) << " bytes" << endl;
//	cout << "sizeof(bitset<64>)     = " << sizeof(std::bitset<64>) << " bytes" << endl;

	cout << "sizeof(bitblock<8 >)   = " << sizeof(bitblock<8>) << " bytes" << endl;
	cout << "sizeof(bitblock<16>)   = " << sizeof(bitblock<16>) << " bytes" << endl;
	cout << "sizeof(bitblock<32>)   = " << sizeof(bitblock<32>) << " bytes" << endl;
	cout << "sizeof(bitblock<64>)   = " << sizeof(bitblock<64>) << " bytes" << endl;

	cout << "sizeof(bool)           = " << sizeof(bool) << " bytes" << endl;
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
