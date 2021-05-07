// algo.cpp: tests to explore different implementations of the arithmetic operators
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>

// minimum set of include files to reflect source code dependencies
#include <universal/native/ieee754.hpp>
#define VALUE_THROW_ARITHMETIC_EXCEPTION 0
#define BITBLOCK_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/internal/value/value.hpp>
#define BLOCKTRIPLE_VERBOSE_OUTPUT 1
#define BLOCKTRIPLE_TRACE_ADD 1
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
// #include <universal/verification/test_reporters.hpp>


// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);
	
//	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING

	// generate individual testcases to hand trace/debug

	{
		constexpr size_t fbits = 7;
		constexpr size_t fhbits = fbits + 1;
		constexpr size_t abits = fhbits + 3;
		constexpr size_t sumbits = abits + 1;
		internal::value<fbits> a,b;
		a = 1.0f;
		b = 1.0f;
		cout << to_triple(a) << " : " << a << '\n';
		cout << to_triple(b) << " : " << b << '\n';
		// add is adding 3 bits to the mantissa to 
		// have all rounding bits available after alignment
		internal::value<sumbits> c;
		internal::module_add<fbits, abits>(a, b, c);  // this API is too confusing: caused by the <abits + 1> argument
		cout << to_triple(c) << " : " << c << '\n';
	}

	// blocktriple stores the significant as you need the hidden bit in any
	// arithmetic operators.

	// to support the quire (Kulisch superaccumulator):
	// - operators add/sub/mul need to produce unrounded results
	// - oprarators div/sqrt are rounded as part of the conversion iteration
	
	// for a significant of nbits, the add/sub input size is nbits + 3
	// The extra 3 bits, are the guard, round, and stick bits that need
	// to come into play to correctly round add/sub as operand alignment
	// shifts information into these bits.
	// The output of the add/sub is nbits + 3 + 1 representing the unrounded result.
	{
		constexpr size_t nbits = 8;  // hidden + fraction bits
		constexpr size_t abits = nbits + 3;
		constexpr size_t sumbits = abits + 1;
		{
			blockfraction<nbits, uint32_t> bba;
			bba.set_raw_bits(0x80);
			blockfraction<sumbits, uint32_t> bbb;
			bbb.assignWithoutSignExtend(bba);
			cout << to_binary(bbb, true) << '\n';
		}
		blocktriple<nbits> a,b;
		a = 1.0f;
		b = 1.0f;
		cout << to_triple(a) << " : " << a << '\n';
		cout << to_triple(b) << " : " << b << '\n';


		blockfraction<sumbits, uint32_t> bb = a.alignSignificant<sumbits>(3);
		cout << to_binary(bb, true) << '\n';
		// blocktriple presents an unrounded external interface for add/sub
		blocktriple<sumbits> c;
//		module_add(a, b, c);
		cout << to_triple(c) << " : " << c << '\n';
	}

#if STRESS_TESTING

#endif

#else

	cout << "block addition validation" << endl;


#if STRESS_TESTING



#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
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
