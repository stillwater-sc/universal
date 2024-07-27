// catastrophic_cancellation.cpp: examples of catastrophic cancellation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/verification/test_status.hpp> // ReportTestResult
#include <universal/verification/posit_test_suite.hpp>

// example of catastrophic cancellation if the number representation doesn't have enough precision bits
template<typename Scalar>
Scalar GenerateTestCase(Scalar e, Scalar x, Scalar origin) {
	Scalar y = origin + x;
	Scalar more = y + e;
	Scalar diff_e = more - y;
	Scalar diff_0 = diff_e - e;
	Scalar zero = diff_0 + diff_0;
	Scalar result = 2 * zero;
	using namespace sw::universal;
	std::cout << "e              : " << to_binary(e) << " : " << e << '\n';
	std::cout << "x              : " << to_binary(x) << " : " << x << '\n';
	std::cout << "origin         : " << to_binary(origin) << " : " << origin << '\n';
	std::cout << "y              : " << to_binary(y) << " : " << y << '\n';
	std::cout << "more           : " << to_binary(more) << " : " << more << '\n';
	std::cout << "diff_e         : " << to_binary(diff_e) << " : " << diff_e << '\n';
	std::cout << "diff_0         : " << to_binary(diff_0) << " : " << diff_0 << '\n';
	std::cout << "zero           : " << to_binary(zero) << " : " << zero << '\n';
	std::cout << "result         : " << to_binary(result) << " : " << result << '\n';

	return result;
}

int main()
try {
	using namespace sw::universal;

	std::string tag = "Catastrophic Cancellation: ";

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	std::cout << "Catastrophic Cancellation Experiment" << '\n';

	std::cout << "IEEE Float single precision  :\n" << GenerateTestCase(0.00000006f, 0.5f, 1.0f) << '\n';
	std::cout << "IEEE Float double precision  :\n" << GenerateTestCase<double>(0.00000006, 0.5, 1.0) << '\n';
	if constexpr (sizeof(long double) == 16) {
		std::cout << "IEEE Float quad precision  :\n" << GenerateTestCase(0.00000006l, 0.5l, 1.0l) << '\n';
	}

	{
		constexpr size_t nbits = 56;
		constexpr size_t es = 2;
		posit<nbits, es> peps, px, porigin;
		peps = 0.00000006f;
		px = 0.5;
		porigin = 1.0;
		std::cout << "posit<56,2>                  :\n" << GenerateTestCase(peps, px, porigin) << '\n';
	}

	{
		constexpr size_t nbits = 64;
		constexpr size_t es = 3;
		posit<nbits, es> peps, px, porigin;
		peps = 0.00000006f;
		px = 0.5;
		porigin = 1.0;
		std::cout << "posit<64,3>                  :\n" << GenerateTestCase(peps, px, porigin) << '\n';
	}

	{
		constexpr size_t nbits = 80;
		constexpr size_t es = 3;
		posit<nbits, es> peps, px, porigin;
		peps = 0.00000006f;
		px = 0.5;
		porigin = 1.0;
		std::cout << "posit<80,3>                  :\n" << GenerateTestCase(peps, px, porigin) << '\n';
	}

	{
		constexpr size_t nbits = 88;
		constexpr size_t es = 3;
		posit<nbits, es> peps, px, porigin;
		peps = 0.00000006f;
		px = 0.5;
		porigin = 1.0;
		std::cout << "posit<88,3>                  :\n" << GenerateTestCase(peps, px, porigin) << '\n';
	}

	{
		constexpr size_t nbits = 96;
		constexpr size_t es = 3;
		posit<nbits, es> peps, px, porigin;
		peps = 0.00000006;
		px = 0.5;
		porigin = 1.0;
		std::cout << "posit<96,3>                  :\n" << GenerateTestCase(peps, px, porigin) << '\n';
	}

	{
		constexpr size_t nbits = 100;
		constexpr size_t es = 3;
		posit<nbits, es> peps, px, porigin;
		peps = 0.00000006;
		px = 0.5;
		porigin = 1.0;
		std::cout << "posit<100,3>                 :\n" << GenerateTestCase(peps, px, porigin) << '\n';
	}

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught unexpected universal internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime error: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
