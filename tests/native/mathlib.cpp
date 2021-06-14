//  mathlib.cpp : universal math library wrapper
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <universal/math/math>

#include <universal/number/posit/posit>

// conditional compile flags
#define MANUAL_TESTING 1
#define STRESS_TESTING 0

template<typename Scalar>
Scalar MathLib(Scalar a, Scalar b =  Scalar(1)) {
	using namespace sw::universal;
	Scalar result{0};

	std::cout << "abs               : " << abs(a) << '\n';

	std::cout << "fpclassify        : " << fpclassify(a) << '\n';
	std::cout << "isnormal          : " << isnormal(a) << '\n';
	std::cout << "isfinite          : " << isfinite(a) << '\n';
	std::cout << "isinf             : " << isinf(a) << '\n';
	std::cout << "isnan             : " << isnan(a) << '\n';
	std::cout << "isdenorm          : " << isdenorm(a) << '\n';

	std::cout << "erf               : " << erf(a) << '\n';
	std::cout << "erfc              : " << erfc(a) << '\n';

	std::cout << "exp               : " << exp(a) << '\n';
	std::cout << "exp2              : " << exp2(a) << '\n';
	std::cout << "exp10             : " << exp10(a) << '\n';
	std::cout << "expm1             : " << expm1(a) << '\n';

	std::cout << "log               : " << log(a) << '\n';
	std::cout << "log2              : " << log2(a) << '\n';
	std::cout << "log10             : " << log10(a) << '\n';
	std::cout << "log1p             : " << log1p(a) << '\n';

	std::cout << "fmod              : " << fmod(a, b) << '\n';
	std::cout << "remainder         : " << remainder(a, b) << '\n';
	std::cout << "frac              : " << frac(a) << '\n';

	std::cout << "sin               : " << sin(a) << '\n';
	std::cout << "cos               : " << cos(a) << '\n';
	std::cout << "tan               : " << tan(a) << '\n';
	std::cout << "atan              : " << atan(a) << '\n';
	std::cout << "acos              : " << acos(a) << '\n';
	std::cout << "asin              : " << asin(a) << '\n';

	std::cout << "sinh              : " << sinh(a) << '\n';
	std::cout << "cosh              : " << cosh(a) << '\n';
	std::cout << "tanh              : " << tanh(a) << '\n';
	std::cout << "atanh             : " << atanh(a) << '\n';
	std::cout << "acosh             : " << acosh(a) << '\n';
	std::cout << "asinh             : " << asinh(a) << '\n';

	std::cout << "hypot             : " << hypot(a, b) << '\n';

	std::cout << "min               : " << min(a, b) << '\n';
	std::cout << "max               : " << max(a, b) << '\n';

	std::cout << "pow               : " << pow(a, b) << '\n';

	std::cout << "sqrt              : " << sqrt(a) << '\n';

	std::cout << "trunc             : " << trunc(a) << '\n';
	std::cout << "round             : " << round(a) << '\n';
	std::cout << "floor             : " << floor(a) << '\n';
	std::cout << "ceil              : " << ceil(a) << '\n';

	return result;
}

int main()
try {
	using namespace std;
	using namespace sw::universal;

	// test Class Template Argument Deduction (CTAD) for elementary functions
	
	{
		float f         = 1.5e-1;
		MathLib(f);
	}

	{
		double d         = 1.5e-1;
		MathLib(d);
	}

	return EXIT_SUCCESS;
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
