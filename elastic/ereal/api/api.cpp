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
#include <universal/number/ereal/ereal.hpp>
#include <math/polynomial/horners.hpp>
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

	std::cout << "+---------    ereal exceptions\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType            = ereal<nlimbs>;
		TestType a{1.0};
		TestType b{0.0};
#if EFLOAT_THROW_ARITHMETIC_EXCEPTION
		try {
			TestType c = a / b;
		} catch (const ereal_divide_by_zero& err) {
			std::cerr << "Caught an ereal_divide_by_zero exception: " << err.what() << '\n';
		} catch (const ereal_invalid_argument& err) {
			std::cerr << "Caught an ereal_invalid_argument exception: " << err.what() << '\n';
		} catch (const ereal_overflow& err) {
			std::cerr << "Caught an ereal_overflow exception: " << err.what() << '\n';
		} catch (const ereal_underflow& err) {
			std::cerr << "Caught an ereal_underflow exception: " << err.what() << '\n';
		} catch (... const std::exception& err) {
			std::cerr << "Caught an unexpected exception: " << err.what() << '\n';
		}

#else
		{
			TestType c = a / b;
			std::cerr << "a / b = " << c << " : " << to_triple(c) << " : " << c << '\n';
		}
#endif
	}

	// manipulators
	std::cout << "+---------    ereal manipulators\n";
	{
		constexpr unsigned nlimbs = 4;
		using FpType              = double;
		using TestType = ereal<nlimbs, FpType>;

		float       f;
		setFields(f, false, ieee754_parameter<float>::bias + 64, 0x7F'FF00u);
		std::cout << "floating point : " << color_print(f, true) << " : " << f << '\n';
		std::cout << "float triple   : " << to_triple(f, true) << " : " << f << '\n';

		TestType a{ f };
		std::cout << "ereal triple   : " << to_triple(a) << " : " << a << '\n';
		std::cout << "sign           : " << sign(a) << '\n';
		std::cout << "scale          : 2^" << scale(a) << '\n';
		std::cout << "significant    : " << significant<nlimbs, FpType>(a) << '\n';
	}

	// interacting with subnormals
	std::cout << "+---------    ereal has no subnormals\n";
	{
		constexpr unsigned nlimbs = 4;
		using FpType              = float;
		using TestType = ereal<nlimbs, FpType>;

		// create a subnormal
		float v;
		setFields(v, false, 0u, 0x00'0001u); // smallest subnormal single precision float
		std::cout << "subnormal      : " << to_binary(v) << " : " << v << '\n';
		
		TestType a{ v };

		std::cout << "ereal triple  : " << to_triple(a) << " : " << double(a) << '\n';
		std::cout << "sign           : " << sign(a) << '\n';
		std::cout << "scale          : 2^" << scale(a) << '\n';
		std::cout << "significant    : " << significant<nlimbs, float, FpType>(a) << "f\n";
	}

	// explicit configuration
	std::cout << "+---------    exceptional values of an ereal\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType = ereal<nlimbs>;

		TestType e;

		double d;
		d = std::numeric_limits<double>::infinity();
		e = d;
		std::cout << "+infinity       : " << e << '\n';
		e = -d;
		std::cout << "-infinity       : " << e << '\n';

		d = std::numeric_limits<double>::signaling_NaN();
		e = d;
		std::cout << "signaling NaN   : " << e << '\n';

		d = std::numeric_limits<double>::quiet_NaN();
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

	std::cout << "+------------ numeric limits of an ereal ----------+\n";
	{
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
		using EReal                   = ereal<4, float>;
		std::vector<EReal> polynomial = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };

		EReal a = sw::math::polynomial::polyeval(polynomial, 5, EReal(0.005f));
		std::cout << "polynomial(1.0) = " << a << '\n';
		for (const auto& limb : a.limbs()) {
			std::cout << "limb = " << color_print(limb) << " : " << limb << '\n';
		}
	}

	std::cout << "Basic ereal<16> operations test\n";
	std::cout << "================================\n\n";
	{
	    using Real = ereal<16, double>;

	    std::cout << "Creating x = 1.0...\n";
	    Real x(1.0);
	    std::cout << "x = " << x << "\n";

	    std::cout << "\nCreating y = 2.0...\n";
	    Real y(2.0);
	    std::cout << "y = " << y << "\n";

	    std::cout << "\nComputing x + y...\n";
	    Real sum = x + y;
	    std::cout << "x + y = " << sum << "\n";

	    std::cout << "\nComputing x - y...\n";
	    Real diff = x - y;
	    std::cout << "x - y = " << diff << "\n";

	    std::cout << "\nComputing abs(x - y)...\n";
	    Real abs_diff = abs(diff);
	    std::cout << "abs(x - y) = " << abs_diff << "\n";

	    std::cout << "\nComputing (x - y) / y...\n";
	    Real rel_err = diff / y;
	    std::cout << "(x - y) / y = " << rel_err << "\n";

	    std::cout << "\nConverting to double...\n";
	    double rel_err_d = double(rel_err);
	    std::cout << "as double: " << rel_err_d << "\n";
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
