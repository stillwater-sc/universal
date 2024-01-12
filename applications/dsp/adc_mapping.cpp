// adc_mapping.cpp: example program showing how to map ADC values to posit values 
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
// Configure the posit library with arithmetic exceptions
// enable posit arithmetic exceptions
#define POSIT_THROW_ARITHMETIC_EXCEPTION 1
#include <universal/number/posit/posit.hpp>

/*

Mathematical 	C++ Symbol	Decimal Representation
Expression
pi				M_PI		3.14159265358979323846
pi/2			M_PI_2		1.57079632679489661923
pi/4			M_PI_4		0.785398163397448309616
1/pi			M_1_PI		0.318309886183790671538
2/pi			M_2_PI		0.636619772367581343076
2/sqrt(pi)		M_2_SQRTPI	1.12837916709551257390
sqrt(2)			M_SQRT2		1.41421356237309504880
1/sqrt(2)		M_SQRT1_2	0.707106781186547524401
e               M_E			2.71828182845904523536
log_2(e)		M_LOG2E		1.44269504088896340736
log_10(e)		M_LOG10E	0.434294481903251827651
log_e(2)		M_LN2		0.693147180559945309417
log_e(10)		M_LN10		2.30258509299404568402

*/

// constexpr double pi = 3.14159265358979323846;  // best practice for C++

template<size_t nbits, size_t es>
void GenerateSample() {
	using namespace sw::universal;

	posit<nbits, es> a, b;

	// posit<16,1> can represent 14-bits worth of equal spaced samples
	// -1, -8191/8192, ... -1/8192, 0, 1/8192, ... , 8191/8192, 1
	b = 8192; // 2^13
	a = 8191; // 2^13 - 1
	std::cout << a << " / " << b << " = " << a / b << '\n';
}

int main()
try {
	using namespace sw::universal;

	GenerateSample<16, 1>();
	GenerateSample<32, 2>();

	posit<16, 1> p = 1, q, diff;
	q = p--;
	diff = q - p;
	std::cout << q << " " << color_print(q) << " - " << p << " " << color_print(p) << " diff " << diff << " " << color_print(diff) << '\n';

	posit<16, 1> a, b, zero = 0;
	b = 8192;
	a = 1 / b;
	std::cout << "   1 / 8192 =  " << a << " " << color_print(a) << '\n';
	std::cout << "   0 / 8192 =  0.00000000 " << color_print(zero) << '\n';
	std::cout << "  -1 / 8192 = " << -a << " " << color_print(-a) << '\n';

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
