// api.cpp: API tests for sw::universal::complex<T>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <iostream>
#include <iomanip>
#include <string>

// Configure the posit library with arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>
#include <universal/math/complex.hpp>

#include <universal/verification/test_suite.hpp>

int main()
try {
	using namespace sw::universal;

	std::string test_suite  = "sw::universal::complex<T> API tests";
	std::string test_tag    = "api";
	bool reportTestCases    = true;
	int nrOfFailedTestCases = 0;

	ReportTestSuiteHeader(test_suite, reportTestCases);

	// Test with posit<32,2>
	{
		using Real = posit<32, 2>;
		using Complex = complex<Real>;

		std::cout << "\n--- Testing complex<posit<32,2>> ---\n";

		// Default constructor
		Complex z1;
		std::cout << "Default constructor: " << z1 << '\n';

		// Constructor from real
		Complex z2(Real(3.0));
		std::cout << "From real: " << z2 << '\n';

		// Constructor from real and imaginary
		Complex z3(Real(1.0), Real(2.0));
		std::cout << "From real,imag: " << z3 << '\n';

		// Copy constructor
		Complex z4(z3);
		std::cout << "Copy constructor: " << z4 << '\n';

		// Accessors
		std::cout << "real(z3) = " << real(z3) << '\n';
		std::cout << "imag(z3) = " << imag(z3) << '\n';

		// Basic arithmetic
		Complex a(Real(1.0), Real(2.0));
		Complex b(Real(3.0), Real(4.0));

		std::cout << "\na = " << a << '\n';
		std::cout << "b = " << b << '\n';
		std::cout << "a + b = " << (a + b) << '\n';
		std::cout << "a - b = " << (a - b) << '\n';
		std::cout << "a * b = " << (a * b) << '\n';
		std::cout << "a / b = " << (a / b) << '\n';

		// Conjugate
		std::cout << "\nconj(a) = " << conj(a) << '\n';

		// Magnitude and phase
		std::cout << "abs(a) = " << abs(a) << '\n';
		std::cout << "arg(a) = " << arg(a) << '\n';
		std::cout << "norm(a) = " << norm(a) << '\n';

		// Polar form
		Complex p = polar(Real(1.0), Real(0.785398));  // 45 degrees
		std::cout << "\npolar(1, pi/4) = " << p << '\n';

		// Transcendental functions
		std::cout << "\nexp(a) = " << exp(a) << '\n';
		std::cout << "log(a) = " << log(a) << '\n';
		std::cout << "sqrt(a) = " << sqrt(a) << '\n';

		// Trigonometric
		std::cout << "\nsin(a) = " << sin(a) << '\n';
		std::cout << "cos(a) = " << cos(a) << '\n';
		std::cout << "tan(a) = " << tan(a) << '\n';

		// Hyperbolic
		std::cout << "\nsinh(a) = " << sinh(a) << '\n';
		std::cout << "cosh(a) = " << cosh(a) << '\n';
		std::cout << "tanh(a) = " << tanh(a) << '\n';

		// Comparison
		std::cout << "\na == a: " << (a == a ? "true" : "false") << '\n';
		std::cout << "a != b: " << (a != b ? "true" : "false") << '\n';

		// Classification
		std::cout << "\nisnan(a): " << (isnan(a) ? "true" : "false") << '\n';
		std::cout << "isinf(a): " << (isinf(a) ? "true" : "false") << '\n';
		std::cout << "isfinite(a): " << (isfinite(a) ? "true" : "false") << '\n';
	}

	// Test interoperability with std::complex<double>
	{
		using Real = posit<32, 2>;
		using Complex = complex<Real>;

		std::cout << "\n--- Interoperability tests ---\n";

		Complex z(Real(1.0), Real(2.0));

		// Convert to std::complex<double>
		std::complex<double> std_z = static_cast<std::complex<double>>(z);
		std::cout << "To std::complex<double>: " << std_z << '\n';

		// Convert back
		Complex back(std_z);
		std::cout << "Back to complex<posit>: " << back << '\n';
	}

	// User-defined literals
	{
		using namespace sw::universal::complex_literals;

		std::cout << "\n--- User-defined literals ---\n";

		auto z = 3.0 + 4.0_ui;
		std::cout << "3.0 + 4.0_ui = " << z << '\n';
	}

	ReportTestSuiteResults(test_suite, nrOfFailedTestCases);
	return (nrOfFailedTestCases > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}
catch (char const* msg) {
	std::cerr << "Caught ad-hoc exception: " << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_arithmetic_exception& err) {
	std::cerr << "Caught universal arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::universal_internal_exception& err) {
	std::cerr << "Caught universal internal exception: " << err.what() << std::endl;
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
