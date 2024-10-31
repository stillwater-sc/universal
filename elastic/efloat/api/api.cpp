// api.cpp: application programming interface tests for efloat: a multi-digit adaptive precision floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the efloat template environment
// : enable/disable arithmetic exceptions
#define EFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// : enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "efloat<> Application Programming Interface demonstration";
	int nrOfFailedTestCases = 0;

	// important behavioral traits
	{
		using TestType = efloat;
		//ReportTrivialityOfType<TestType>();
		bool isTrivial = bool(std::is_trivial<TestType>());
		std::string testType = sw::universal::type_tag(TestType());
		std::cout << (isTrivial ? testType + std::string(" is trivial") : testType + std::string(" is not trivial")) << '\n';
	}

	// construction, initialization, and copy construction
	std::cout << "+---------    efloat construction, initialization, and copy construction\n";
	{
		using TestType = efloat;

		TestType a{ 1.5f }, b(2.5);
#if LONG_DOUBLE_SUPPORT
		TestType c{ 4.5l };
#else
		TestType c{ 4.5 };
#endif
		TestType d(c);

		std::cout << "a : " << to_triple(a) << " : " << a.significant() << " : " << double(a) << '\n';
		std::cout << "b : " << to_triple(b) << " : " << b.significant() << " : " << double(b) << '\n';
		std::cout << "c : " << to_triple(c) << " : " << c.significant() << " : " << double(c) << '\n';
		std::cout << "d : " << to_triple(d) << " : " << d.significant() << " : " << double(c) << '\n';
	}

	// manipulators
	std::cout << "+---------    efloat manipulators\n";
	{
		using TestType = efloat;

		float_decoder d;
		d.parts.sign = false;
		d.parts.exponent = ieee754_parameter<float>::bias + 64;
		d.parts.fraction = 0x7FFFFu << 8;   // these are just the fraction bits, no hidden bit
		std::cout << "fraction bits  : " << to_binary(d.parts.fraction, true) << '\n';
		float f = d.f;
		std::cout << "floating point : " << to_binary(f, true) << " : " << f << '\n';

		TestType a{ f };
		std::cout << "efloat triple  : " << to_triple(a) << " : " << a.significant() << " : " << double(a) << '\n';
		std::cout << "sign           : " << sign(a) << '\n';
		std::cout << "scale          : 2^" << scale(a) << '\n';
		std::cout << "significant    : " << significant<float>(a) << "f\n";
		std::cout << "significant    : " << significant<double>(a) << '\n';
	}

	// default behavior
	std::cout << "+---------    Default efloat has no subnormals, no supernormals and is not saturating\n";
	{
		
	}

	// explicit configuration
	std::cout << "+---------    Explicit configuration of a efloat\n";
	{
		
	}

	std::cout << "+---------    human-readable output for large efloats   --------+\n";
	{
		
	}

	std::cout << "+------------ numeric limits of a Cfloat ----------+\n";
	{
		// using efloat = sw::universal::efloat<32, 8, uint32_t, true, false, false>;

		std::cout << "efloat(INFINITY): " << efloat(INFINITY) << "\n";
		std::cout << "efloat(-INFINITY): " << efloat(-INFINITY) << "\n";

		std::cout << "efloat(std::numeric_limits<float>::infinity())  : " << efloat(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "efloat(-std::numeric_limits<float>::infinity()) : " << efloat(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<efloat>::infinity() : " << 2 * std::numeric_limits<efloat>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<efloat>::infinity() : " << -2 * std::numeric_limits<efloat>::infinity() << "\n";

//		std::cout << "sw::universal::nextafter(efloat(0), std::numeric_limits<efloat>::infinity())  : " << sw::universal::nextafter(efloat(-0), std::numeric_limits<efloat>::infinity()) << "\n";
//		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())              : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
//		std::cout << "sw::universal::nextafter(efloat(0), -std::numeric_limits<efloat>::infinity()) : " << sw::universal::nextafter(efloat(0), -std::numeric_limits<efloat>::infinity()) << "\n";
//		std::cout << "std::nextafter(float(0), -std::numeric_limits<float>::infinity())             : " << std::nextafter(float(0), -std::numeric_limits<float>::infinity()) << "\n";

//		std::cout << "efloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)      : " << efloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
//		std::cout << "efloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << efloat(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
	}

	std::cout << "+------------ Serialization of a efloat ----------+\n";
	{

	}

	std::cout << "+------------ Horner's Rule ----------+\n";
	{
//		std::vector<efloat> polynomial = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

//		std::cout << "polynomial(1.0) = " << polyeval(polynomial, 5, efloat(1.0f)) << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
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
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
