//  api.cpp : test suite runner for the class interface of the simplified floating-point type
//
// Copyright (C) 2017-2022 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/utility/long_double.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/internal/gfp/gfp.hpp>
#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "gfp API validation";
	std::string test_tag    = "API";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	/////////////////         construction


	{
		gfp<uint32_t> a, b, c;
		a = 1.0e0f;
		std::cout << to_binary(a) << '\n';
		b = 1.0e0f;
		c = a + b;
		std::cout << a << " * " << b << " = " << c << '\n';
	}

	{
		gfp<uint64_t> a, b, c;
		a = 1.0e0;
		std::cout << to_binary(a) << '\n';
		b = 1.0e0;
		c = a + b;
		std::cout << a << " * " << b << " = " << c << '\n';
	}

	{
		gfp<uint64_t> a, b, c;
		a.set(false, 0, 0xf'ffff'ffffull, 52); // emulate a double
		b.set(false, 0, 0x1'ffff'ffffull, 52); // emulate a double);
		std::cout << to_hex(b.significant()) << '\n';
		c = a * b;
		std::cout << a << " * " << b << " = " << c << '\n';
		int alpha = 0;
		std::cout << "alpha : " << alpha << "  k : " << c.calculate_k(alpha) << '\n';
	}

	{
		int alpha = 0;
		for (int binaryScale = -10; binaryScale <= 64; ++binaryScale) {
			std::cout << "binaryScale : " << binaryScale << " vs decimalScale : " << decimalScale(binaryScale, 64, alpha) << '\n';
		}
	}

	{
		using Scalar = std::uint32_t;
		gfp<Scalar> a;
		a = 1.0f;
		std::cout << to_binary(a) << " : " << float(a) << '\n';

		a = 0.03125f;
		std::cout << to_binary(a) << " : " << float(a) << '\n';
	}

	{
		using Scalar = std::uint64_t;
		gfp<Scalar> a;
		a = 1.0;
		std::cout << to_binary(a) << " : " << double(a) << '\n';

		a = 0.03125;
		std::cout << to_binary(a) << " : " << double(a) << '\n';
	}

	{
		half f{ 0.03125 };
		std::cout << "floating-point value : " << to_binary(f) << " : " << f << " : " << to_triple(f) << '\n';

		duble d{ 0.0312 };
		std::cout << "floating-point value : " << to_binary(d) << " : " << d << " : " << to_triple(d) << '\n';
	}

	{
		gfp<uint64_t> a;
		unsigned q = 64;
		// print out the cached powers
		for (int mk = 0; mk < 88; ++mk) {
			CachedPower cp = CachedPowers[mk];
			std::cout << mk << " : " << to_binary(cp.significand, 64, true) << " : ";
			a.set(false, cp.binary_exponent, cp.significand, q);
			std::cout << double(a) << '\n';

		}
	}

	{
		std::cout << grisu<std::uint64_t>(1.0) << '\n';
	}
	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << msg << '\n';
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << '\n';
	return EXIT_FAILURE;
}
