// amd_fp24.cpp: test suite runner for standard AMD fp24 format, which is equivalent to cfloat<24,7>
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
// minimum set of include files to reflect source code dependencies
#include <universal/number/cfloat/cfloat_impl.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	print_cmd_line(argc, argv);

	// map the AMD FP24 onto the classic cfloats
	constexpr size_t nbits = 24;
	constexpr size_t ebits = 7;
	using amd_fp24 = cfloat<nbits, ebits>;

	int nrOfFailedTestCases = 0;
	std::string tag = " cfloat<24,7>";

	cout << "Standard AMD fp24 format, which is equivalent to a cfloat<24,7> configuration tests" << endl;

	amd_fp24 r;
	r = 1.2345;
	cout << r << endl;

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_arithmetic_exception& err) {
	std::cerr << "Uncaught real arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::cfloat_internal_exception& err) {
	std::cerr << "Uncaught real internal exception: " << err.what() << std::endl;
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
