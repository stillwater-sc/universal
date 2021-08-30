// complex.cpp: test suite runner for complex (real, imag, conj) functions
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// use default number system library configuration
#include <universal/number/fixpnt/fixpnt.hpp>

#define MANUAL_TESTING 0
#define STRESS_TESTING 0

int main()
try {
	using namespace sw::universal;

	bool bReportIndividualTestCases = false;
	int nrOfFailedTestCases = 0;

	std::string tag = "fixpnt complex failed: ";

#if MANUAL_TESTING

	// manual exhaustive test

	{
	    constexpr size_t nbits = 8;
	    constexpr size_t rbits = 4;
	    constexpr bool arithmetic = Saturating;
	    typedef uint8_t bt;
	    using Real = fixpnt<nbits, rbits, arithmetic, bt>;
	    std::complex<FixedPoint> a, b, c;

	    a.real = 1.0f;
	    a.imag = 1.0f;
	}
#else

	std::cout << "fixpnt complex function validation\n";


#if STRESS_TESTING
	
#endif  // STRESS_TESTING

#endif  // MANUAL_TESTING

	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_arithmetic_exception& err) {
	std::cerr << "Uncaught fixpnt arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::fixpnt_internal_exception& err) {
	std::cerr << "Uncaught fixpnt internal exception: " << err.what() << std::endl;
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
