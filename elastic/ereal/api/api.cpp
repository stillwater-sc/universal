// api.cpp: application programming interface tests for ereal: a multi-component adaptive precision floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>

// minimum set of include files to reflect source code dependencies
// Configure the ereal template environment
// : enable/disable arithmetic exceptions
#define EFLOAT_THROW_ARITHMETIC_EXCEPTION 0
// : enable trace conversion
#define TRACE_CONVERSION 0
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "ereal<> Application Programming Interface demonstration";
	int nrOfFailedTestCases = 0;

	// important behavioral traits
	{
		constexpr unsigned nlimbs = 4;
		using TestType = ereal<nlimbs>;
		//ReportTrivialityOfType<TestType>();
		bool isTrivial = bool(std::is_trivial<TestType>());
		std::string testType = sw::universal::type_tag(TestType());
		std::cout << (isTrivial ? testType + std::string(" is trivial") : testType + std::string(" is not trivial")) << '\n';
	}

	// construction, initialization, and copy construction
	std::cout << "+---------    ereal construction, initialization, and copy construction\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType = ereal<nlimbs>;

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
	std::cout << "+---------    ereal manipulators\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType = ereal<nlimbs>;

		float_decoder d;
		d.parts.sign = false;
		d.parts.exponent = ieee754_parameter<float>::bias + 64;
		d.parts.fraction = 0x7F'FF00u;   // these are just the 23 fraction bits, no hidden bit
		std::cout << "fraction bits  : " << to_binary(d.parts.fraction, true) << '\n';
		float f = d.f;
		std::cout << "floating point : " << to_binary(f, true) << " : " << f << '\n';

		TestType a{ f };
		std::cout << "ereal triple  : " << to_triple(a) << " : " << a.significant() << " : " << double(a) << '\n';
		std::cout << "sign           : " << sign(a) << '\n';
		std::cout << "scale          : 2^" << scale(a) << '\n';
		std::cout << "significant    : " << significant<nlimbs, float>(a) << "f\n";
		std::cout << "significant    : " << significant<nlimbs, double>(a) << '\n';
	}

	// interacting with subnormals
	std::cout << "+---------    ereal has no subnormals\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType = ereal<nlimbs>;

		// create a subnormal
		float v;
		setFields(v, false, 0u, 0x00'0001u); // smallest subnormal single precision float
//		bool s{ false };
//		uint32_t e{ 0 };
//		uint32_t f{ 0 };
//		uint32_t bits{ 0 };
//		extractFields(v, s, e, f, bits);
		std::cout << "subnormal      : " << to_binary(v) << " : " << v << '\n';
		
		TestType a{ v };

		std::cout << "ereal triple  : " << to_triple(a) << " : " << a.significant() << " : " << float(a) << '\n';
		std::cout << "sign           : " << sign(a) << '\n';
		std::cout << "scale          : 2^" << scale(a) << '\n';
		std::cout << "significant    : " << significant<nlimbs, float>(a) << "f\n";

		double dv;
		setFields(dv, true, 0ull, 0x1ull);
		std::cout << "floating point : " << to_binary(dv, true) << " : " << dv << '\n';
		a = dv;

		std::cout << "ereal triple  : " << to_triple(a) << " : " << a.significant() << " : " << double(a) << '\n';
		std::cout << "sign           : " << sign(a) << '\n';
		std::cout << "scale          : 2^" << scale(a) << '\n';
		std::cout << "significant    : " << significant<nlimbs, double>(a) << "f\n";
	}

	// explicit configuration
	std::cout << "+---------    exceptional values of an ereal\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType = ereal<nlimbs>;

		TestType e;

		double d;
		d = std::numeric_limits<double>::infinity();
//		std::cout << d << '\n';
		e = d;
		std::cout << "+infinity       : " << e << '\n';
		e = -d;
		std::cout << "-infinity       : " << e << '\n';

		d = std::numeric_limits<double>::signaling_NaN();
//		std::cout << d << " : " << '\n';
		e = d;
		std::cout << "signaling NaN   : " << e << '\n';

		d = std::numeric_limits<double>::quiet_NaN();
//		std::cout << d << " : " << '\n';
		e = d;
		std::cout << "quiet NaN       : " << e << '\n';
	}

	// explicit configuration
	std::cout << "+---------    explicit configuration of a ereal\n";
	{
		
	}

	std::cout << "+---------    human-readable output for large ereals   --------+\n";
	{
		
	}

	std::cout << "+------------ numeric limits of a Cfloat ----------+\n";
	{
		// using ereal = sw::universal::ereal<32, 8, uint32_t, true, false, false>;

		std::cout << "ereal(INFINITY): " << ereal(INFINITY) << "\n";
		std::cout << "ereal(-INFINITY): " << ereal(-INFINITY) << "\n";

		std::cout << "ereal(std::numeric_limits<float>::infinity())  : " << ereal(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "ereal(-std::numeric_limits<float>::infinity()) : " << ereal(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<ereal>::infinity() : " << 2 * std::numeric_limits<ereal<4>>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<ereal>::infinity() : " << -2 * std::numeric_limits<ereal<4>>::infinity() << "\n";

//		std::cout << "sw::universal::nextafter(ereal(0), std::numeric_limits<ereal>::infinity())  : " << sw::universal::nextafter(ereal(-0), std::numeric_limits<ereal>::infinity()) << "\n";
//		std::cout << "std::nextafter(float(0), std::numeric_limits<float>::infinity())              : " << std::nextafter(float(-0), std::numeric_limits<float>::infinity()) << "\n";
//		std::cout << "sw::universal::nextafter(ereal(0), -std::numeric_limits<ereal>::infinity()) : " << sw::universal::nextafter(ereal(0), -std::numeric_limits<ereal>::infinity()) << "\n";
//		std::cout << "std::nextafter(float(0), -std::numeric_limits<float>::infinity())             : " << std::nextafter(float(0), -std::numeric_limits<float>::infinity()) << "\n";

//		std::cout << "ereal(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET)      : " << ereal(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_QUIET) << "\n";
//		std::cout << "ereal(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) : " << ereal(std::numeric_limits<float>::signaling_NaN()).isnan(sw::universal::NAN_TYPE_SIGNALLING) << "\n";
	}

	std::cout << "+------------ Serialization of a ereal ----------+\n";
	{

	}

	std::cout << "+------------ Horner's Rule ----------+\n";
	{
//		std::vector<ereal> polynomial = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

//		std::cout << "polynomial(1.0) = " << polyeval(polynomial, 5, ereal(1.0f)) << '\n';
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
