// tensorfloat.cpp: test suite runner for NVIDIA's TensorFloat
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/verification/test_suite_arithmetic.hpp>

int main()
try {
	using namespace sw::universal;

	// cast the NVIDIA TensorFloat onto the classic cfloats
	constexpr size_t nbits = 19;
	constexpr size_t es = 8;
	using tensorfloat = cfloat<nbits, es>;

	int nrOfFailedTestCases = 0;
	std::string tag = " cfloat<19,8>";

	std::cout << "Standard NVIDIA TensorFloat, which is equivalent to a cfloat<19,8> configuration tests\n";

	tensorfloat r; // uninitialized
	r = 1.2345;
	std::cout << r << '\n';

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception : " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Caught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
