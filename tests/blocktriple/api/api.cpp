// api.cpp: class interface usage patterns for blocktriple
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#if defined(_MSC_VER)
#pragma warning(disable : 4514)  // unreferenced function is removed
#pragma warning(disable : 4710)  // function is not inlined
#pragma warning(disable : 5045)  // Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif
#include <iostream>
#include <iomanip>
#include <fstream>
#include <typeinfo>
// minimum set of include files to reflect source code dependencies
#include <universal/internal/blocktriple/blocktriple.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>

#define MANUAL_TESTING 1
#define STRESS_TESTING 0

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	for (int i = 0; i < argc; ++i) std::cout << argv[0] << ' ';
	std::cout << std::endl;

	std::cout << "blocktriple<> class interface tests" << std::endl;

	int nrOfFailedTestCases = 0;

#if MANUAL_TESTING
/*
	{
		fixpnt<20, 9> a;
		a.set_raw_bits(0x15);
		cout << to_binary(a, true) << " " << a << endl;
	}
*/

	{
		CONSTEXPRESSION blocktriple<9, uint32_t> a = 511.5f;
		cout << to_binary(a) << " : " << to_triple(a) << " : " << a << '\n';
	}
	{
		constexpr double d = 511.5;
		//cout << to_binary(d, true) << '\n';
		CONSTEXPRESSION blocktriple<9, uint64_t> a = d;
		cout << to_binary(a) << " : " << to_triple(a) << " : " << a << '\n';
	}

#else // !MANUAL_TESTING



#endif // MANUAL_TESTING

	std::cout << "\nblocktriple API test suite           : " << (nrOfFailedTestCases == 0 ? "PASS\n" : "FAIL\n");

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
