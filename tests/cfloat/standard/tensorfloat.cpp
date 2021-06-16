// tensorfloat.cpp: test suite runner for NVIDIA's TensorFloat
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

	// map the NVIDIA TensorFloat onto the classic cfloats
	constexpr size_t nbits = 19;
	constexpr size_t es = 8;
	using tensorfloat = cfloat<nbits, es>;

	int nrOfFailedTestCases = 0;
	std::string tag = " cfloat<19,8>";

	cout << "Standard NVIDIA TensorFloat, which is equivalent to a cfloat<19,8> configuration tests" << endl;

	tensorfloat r;
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
