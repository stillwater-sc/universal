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
#include <universal/number/efloat/efloat.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite = "efloat<> Application Programming Interface demonstration";
	int nrOfFailedTestCases = 0;

	// important behavioral traits
	{
		constexpr unsigned nlimbs = 4;
		using TestType = efloat<nlimbs>;
		
		bool isTrivial = bool(std::is_trivial<TestType>());
		std::string testType = sw::universal::type_tag(TestType());
		std::cout << (isTrivial ? testType + std::string(" is trivial") 
								: testType + std::string(" is not trivial")) << '\n';


		bool isTriviallyConstructible = bool(std::is_trivially_constructible<TestType>());
		std::cout << (isTriviallyConstructible ? testType + std::string(" is trivially constructible")
		                                       : testType + std::string(" is not trivially constructible")) << '\n';


		bool isTriviallyCopyable = bool(std::is_trivially_copyable<TestType>());
		std::cout << (isTriviallyCopyable ? testType + std::string(" is trivially copyable")
		                                  : testType + std::string(" is not trivially copyable")) << '\n';

		bool isTriviallyCopyAssignable = bool(std::is_trivially_copy_assignable<TestType>());
		std::cout << (isTriviallyCopyAssignable ? testType + std::string(" is trivially copy-assignable")
		                                        : testType + std::string(" is not trivially copy-assignable")) << '\n';

	}

	// construction, initialization, and copy construction
	std::cout << "+---------    efloat construction, initialization, and copy construction\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType = efloat<nlimbs>;

		TestType a{1.5f}, b(2.5);
		TestType an_eps(pow(2.0, -64));

#if LONG_DOUBLE_SUPPORT
		TestType c{ 4.5l };
#else
		TestType c{ 4.5 };
#endif

		a += an_eps;
		b += an_eps;
		c += an_eps;
		TestType d(c);

		std::cout << "a : " << to_triple(a) << " : " << a.significant() << " : " << a << '\n';
		std::cout << "b : " << to_triple(b) << " : " << b.significant() << " : " << b << '\n';
		std::cout << "c : " << to_triple(c) << " : " << c.significant() << " : " << c << '\n';
		std::cout << "d : " << to_triple(d) << " : " << d.significant() << " : " << c << '\n';
	}

	// manipulators
	std::cout << "+---------    efloat manipulators\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType = efloat<nlimbs>;

		TestType a{ 1.5f };
		std::cout << "efloat triple  : " << to_triple(a) << " : " << a.significant() << " : " << a << '\n';
		std::cout << "sign           : " << sign(a) << '\n';
		std::cout << "scale          : 2^" << scale(a) << '\n';
		std::cout << "significant    : " << significant<nlimbs, float>(a) << "f\n";
		std::cout << "significant    : " << significant<nlimbs, double>(a) << '\n';

		std::cout << "efloat bits    : " << to_binary(a) << '\n';
		std::cout << "efloat bits    : " << color_print(a, true) << '\n';
	}

	// interacting with subnormals
	std::cout << "+---------    efloat has no subnormals\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType = efloat<nlimbs>;

		// create a subnormal
		float v;
		setFields(v, false, 0u, 0x00'0001u); // smallest subnormal single precision float
		std::cout << "subnormal float  : " << to_binary(v, true) << " : " << v << '\n';
		
		TestType a{ v };

		std::cout << "efloat triple    : " << to_triple(a) << " : " << a.significant() << " : " << float(a) << '\n';
		std::cout << "sign             : " << sign(a) << '\n';
		std::cout << "scale            : 2^" << scale(a) << '\n';
		std::cout << "significant      : " << significant<nlimbs, float>(a) << "f\n";

		double dv;
		setFields(dv, true, 0ull, 0x1ull);  // smallest subnormal double precision float
		std::cout << "subnormal double : " << to_binary(dv, true) << " : " << dv << '\n';
		a = dv;

		std::cout << "efloat triple    : " << to_triple(a) << " : " << a.significant() << " : " << double(a) << '\n';
		std::cout << "sign             : " << sign(a) << '\n';
		std::cout << "scale            : 2^" << scale(a) << '\n';
		std::cout << "significant      : " << significant<nlimbs, double>(a) << "f\n";
	}

	// explicit configuration
	std::cout << "+---------    exceptional values of an efloat\n";
	{
		constexpr unsigned nlimbs = 4;
		using TestType = efloat<nlimbs>;

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
	std::cout << "+---------    explicit configuration of a efloat\n";
	auto old_precision = std::cout.precision();
	{
		using TestType = efloat<10>;  // 320 bits of precision

		double two64 = pow(2.0, 64);
		TestType a(two64);
		TestType b(1ull);
		TestType c(1.0 / static_cast<double>(two64));

		TestType d = a + b + c;

		std::cout << std::setprecision(40);
		std::cout << "a + b + c : " << to_binary(d) << " : " << d << '\n';

	}

	std::cout << "+---------    human-readable output for large efloats   --------+\n";
	{
		using TestType = efloat<10>;  // 320 bits of precision
		TestType a(pow(2.0, 300));
		std::cout << "a : " << to_binary(a) << " : " << std::setprecision(100) << a << '\n';
	}
	std::cout.precision(old_precision);

	std::cout << "+------------ numeric limits of a Cfloat ----------+\n";
	{
		// using efloat = sw::universal::efloat<32, 8, uint32_t, true, false, false>;

		std::cout << "efloat(INFINITY): " << efloat(INFINITY) << "\n";
		std::cout << "efloat(-INFINITY): " << efloat(-INFINITY) << "\n";

		std::cout << "efloat(std::numeric_limits<float>::infinity())  : " << efloat(std::numeric_limits<float>::infinity()) << "\n";
		std::cout << "efloat(-std::numeric_limits<float>::infinity()) : " << efloat(-std::numeric_limits<float>::infinity()) << "\n";

		std::cout << " 2 * std::numeric_limits<float>::infinity()  : " << 2 * std::numeric_limits<float>::infinity() << "\n";
		std::cout << " 2 * std::numeric_limits<efloat>::infinity() : " << 2 * std::numeric_limits<efloat<4>>::infinity() << "\n";
		std::cout << "-2 * std::numeric_limits<efloat>::infinity() : " << -2 * std::numeric_limits<efloat<4>>::infinity() << "\n";

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

	// runtime precision: a value rounds at its own precision, and a native operand takes its
	// partner's rather than widening the result to the type's nlimbs * 32 bits (#1757)
	std::cout << "+------------ runtime precision of an efloat ----------+\n";
	{
		using TestType = efloat<100>;   // 3200 bits
		TestType x(1);
		x /= 3;
		x.set_precision(640);
		TestType y = x * 3 / TestType(7) + 0.5;   // literals carry no precision: 640 bits
		TestType z = exp(x);                       // as does the math library
		TestType w = TestType(1) / TestType(3);    // literals alone: the type's 3200 bits
		std::cout << "x                : " << x.get_precision() << " bits\n";
		std::cout << "x * 3 / 7 + 0.5  : " << y.get_precision() << " bits\n";
		std::cout << "exp(x)           : " << z.get_precision() << " bits\n";
		std::cout << "1 / 3 (literals) : " << w.get_precision() << " bits\n";
		if (y.get_precision() != 640 || z.get_precision() != 640 || w.get_precision() != 3200) ++nrOfFailedTestCases;
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
