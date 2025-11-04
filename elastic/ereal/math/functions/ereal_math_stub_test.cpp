// ereal_math_stub_test.cpp: test suite for ereal mathlib stub implementations
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <iostream>
#include <iomanip>
#include <string>

// Configure ereal
#include <universal/number/ereal/ereal.hpp>

// Test helpers
constexpr unsigned COLWIDTH = 20;

int main()
try {
	using namespace sw::universal;

	std::cout << "ereal mathlib stub function validation\n";
	std::cout << "========================================\n\n";

	// Use default maxlimbs parameter
	using Real = ereal<>;

	int nrOfFailures = 0;

	// Test values
	Real x(2.0);
	Real y(3.0);
	Real z;

	std::cout << std::setw(COLWIDTH) << "Function" << " : " << "Result\n";
	std::cout << std::string(COLWIDTH + 20, '-') << '\n';

	// Classification functions
	std::cout << "\nClassification Functions:\n";
	std::cout << std::setw(COLWIDTH) << "isfinite(2.0)" << " : " << (isfinite(x) ? "true" : "false") << '\n';
	std::cout << std::setw(COLWIDTH) << "isnan(2.0)" << " : " << (isnan(x) ? "true" : "false") << '\n';
	std::cout << std::setw(COLWIDTH) << "isinf(2.0)" << " : " << (isinf(x) ? "true" : "false") << '\n';
	std::cout << std::setw(COLWIDTH) << "isnormal(2.0)" << " : " << (isnormal(x) ? "true" : "false") << '\n';
	std::cout << std::setw(COLWIDTH) << "signbit(2.0)" << " : " << (signbit(x) ? "true" : "false") << '\n';

	// Numeric operations
	std::cout << "\nNumeric Operations:\n";
	int exponent;
	z = frexp(x, &exponent);
	std::cout << std::setw(COLWIDTH) << "frexp(2.0)" << " : " << z << " * 2^" << exponent << '\n';
	z = ldexp(x, 3);
	std::cout << std::setw(COLWIDTH) << "ldexp(2.0, 3)" << " : " << z << '\n';
	z = copysign(x, Real(-1.0));
	std::cout << std::setw(COLWIDTH) << "copysign(2.0, -1)" << " : " << z << '\n';
	z = abs(Real(-2.0));
	std::cout << std::setw(COLWIDTH) << "abs(-2.0)" << " : " << z << '\n';

	// Truncation functions
	std::cout << "\nTruncation Functions:\n";
	z = floor(Real(2.7));
	std::cout << std::setw(COLWIDTH) << "floor(2.7)" << " : " << z << '\n';
	z = ceil(Real(2.3));
	std::cout << std::setw(COLWIDTH) << "ceil(2.3)" << " : " << z << '\n';
	z = trunc(Real(2.7));
	std::cout << std::setw(COLWIDTH) << "trunc(2.7)" << " : " << z << '\n';
	z = round(Real(2.5));
	std::cout << std::setw(COLWIDTH) << "round(2.5)" << " : " << z << '\n';

	// Min/Max functions
	std::cout << "\nMin/Max Functions:\n";
	z = min(x, y);
	std::cout << std::setw(COLWIDTH) << "min(2.0, 3.0)" << " : " << z << '\n';
	z = max(x, y);
	std::cout << std::setw(COLWIDTH) << "max(2.0, 3.0)" << " : " << z << '\n';

	// Fractional functions
	std::cout << "\nFractional Functions:\n";
	z = fmod(Real(7.0), Real(3.0));
	std::cout << std::setw(COLWIDTH) << "fmod(7.0, 3.0)" << " : " << z << '\n';
	z = remainder(Real(7.0), Real(3.0));
	std::cout << std::setw(COLWIDTH) << "remainder(7.0, 3.0)" << " : " << z << '\n';

	// Hypot function
	std::cout << "\nHypot Function:\n";
	z = hypot(x, y);
	std::cout << std::setw(COLWIDTH) << "hypot(2.0, 3.0)" << " : " << z << '\n';
	z = hypot(x, y, Real(4.0));
	std::cout << std::setw(COLWIDTH) << "hypot(2,3,4)" << " : " << z << '\n';

	// Root functions
	std::cout << "\nRoot Functions:\n";
	z = sqrt(x);
	std::cout << std::setw(COLWIDTH) << "sqrt(2.0)" << " : " << z << '\n';
	z = cbrt(Real(8.0));
	std::cout << std::setw(COLWIDTH) << "cbrt(8.0)" << " : " << z << '\n';

	// Exponential functions
	std::cout << "\nExponential Functions:\n";
	z = exp(x);
	std::cout << std::setw(COLWIDTH) << "exp(2.0)" << " : " << z << '\n';
	z = exp2(x);
	std::cout << std::setw(COLWIDTH) << "exp2(2.0)" << " : " << z << '\n';
	z = exp10(x);
	std::cout << std::setw(COLWIDTH) << "exp10(2.0)" << " : " << z << '\n';
	z = expm1(Real(0.1));
	std::cout << std::setw(COLWIDTH) << "expm1(0.1)" << " : " << z << '\n';

	// Logarithm functions
	std::cout << "\nLogarithm Functions:\n";
	z = log(x);
	std::cout << std::setw(COLWIDTH) << "log(2.0)" << " : " << z << '\n';
	z = log2(x);
	std::cout << std::setw(COLWIDTH) << "log2(2.0)" << " : " << z << '\n';
	z = log10(x);
	std::cout << std::setw(COLWIDTH) << "log10(2.0)" << " : " << z << '\n';
	z = log1p(Real(0.1));
	std::cout << std::setw(COLWIDTH) << "log1p(0.1)" << " : " << z << '\n';

	// Power functions
	std::cout << "\nPower Functions:\n";
	z = pow(x, y);
	std::cout << std::setw(COLWIDTH) << "pow(2.0, 3.0)" << " : " << z << '\n';
	z = pown(x, 3);
	std::cout << std::setw(COLWIDTH) << "pown(2.0, 3)" << " : " << z << '\n';

	// Trigonometric functions
	std::cout << "\nTrigonometric Functions:\n";
	z = sin(Real(1.0));
	std::cout << std::setw(COLWIDTH) << "sin(1.0)" << " : " << z << '\n';
	z = cos(Real(1.0));
	std::cout << std::setw(COLWIDTH) << "cos(1.0)" << " : " << z << '\n';
	z = tan(Real(1.0));
	std::cout << std::setw(COLWIDTH) << "tan(1.0)" << " : " << z << '\n';
	z = asin(Real(0.5));
	std::cout << std::setw(COLWIDTH) << "asin(0.5)" << " : " << z << '\n';
	z = acos(Real(0.5));
	std::cout << std::setw(COLWIDTH) << "acos(0.5)" << " : " << z << '\n';
	z = atan(Real(1.0));
	std::cout << std::setw(COLWIDTH) << "atan(1.0)" << " : " << z << '\n';
	z = atan2(y, x);
	std::cout << std::setw(COLWIDTH) << "atan2(3.0, 2.0)" << " : " << z << '\n';

	// Hyperbolic functions
	std::cout << "\nHyperbolic Functions:\n";
	z = sinh(x);
	std::cout << std::setw(COLWIDTH) << "sinh(2.0)" << " : " << z << '\n';
	z = cosh(x);
	std::cout << std::setw(COLWIDTH) << "cosh(2.0)" << " : " << z << '\n';
	z = tanh(x);
	std::cout << std::setw(COLWIDTH) << "tanh(2.0)" << " : " << z << '\n';
	z = asinh(x);
	std::cout << std::setw(COLWIDTH) << "asinh(2.0)" << " : " << z << '\n';
	z = acosh(x);
	std::cout << std::setw(COLWIDTH) << "acosh(2.0)" << " : " << z << '\n';
	z = atanh(Real(0.5));
	std::cout << std::setw(COLWIDTH) << "atanh(0.5)" << " : " << z << '\n';

	// Error and Gamma functions
	std::cout << "\nError and Gamma Functions:\n";
	z = erf(x);
	std::cout << std::setw(COLWIDTH) << "erf(2.0)" << " : " << z << '\n';
	z = erfc(x);
	std::cout << std::setw(COLWIDTH) << "erfc(2.0)" << " : " << z << '\n';
	z = tgamma(x);
	std::cout << std::setw(COLWIDTH) << "tgamma(2.0)" << " : " << z << '\n';
	z = lgamma(x);
	std::cout << std::setw(COLWIDTH) << "lgamma(2.0)" << " : " << z << '\n';

	// Next functions
	std::cout << "\nNext Functions:\n";
	z = nextafter(x, y);
	std::cout << std::setw(COLWIDTH) << "nextafter(2,3)" << " : " << z << '\n';

	std::cout << "\n========================================\n";
	if (nrOfFailures == 0) {
		std::cout << "All stub functions compiled and executed successfully.\n";
		std::cout << "Phase 0 infrastructure validation: PASS\n";
	}
	else {
		std::cout << "Phase 0 infrastructure validation: FAIL\n";
		std::cout << "Number of failures: " << nrOfFailures << '\n';
	}

	return (nrOfFailures > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
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
catch (const std::runtime_error& err) {
	std::cerr << "Caught unexpected runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
