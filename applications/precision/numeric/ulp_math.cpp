// ulp_math.cpp: example program to show operations on Unit in Last Position
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <ostream>
#include <limits>
#include <numeric>   // nextafter

// select the number systems we would like to compare
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <universal/number/posit/numeric_limits.hpp>

template<typename Scalar>
void ULP(const Scalar& s) {
	using namespace sw::universal;
	int maxDigits = std::numeric_limits<Scalar>::max_digits10;
	std::cout << "scalar type: " << std::setw(50) << typeid(s).name() << " max digits: " << std::setw(5) << maxDigits << '\n';
	// needs C++20 to become constexpr for generic universal types
	Scalar zero     = 0;
	Scalar infinity = std::numeric_limits<Scalar>::infinity();
	auto precision = std::cout.precision();
	std::cout << std::setprecision(maxDigits);
	std::cout << "prior  : " << nextafter(s, zero) << '\n'
		 << "value  : " << s << "                 " << std::hexfloat << s << std::dec << '\n'   // <--- need to overload hexfloat for posit hex_format
		 << "post   : " << nextafter(s, infinity) << '\n';
	std::cout << std::setprecision(precision);
}

// TODO: since we haven't implemented std::hexfloat for posits yet, we need this extra function
template<size_t nbits, size_t es>
void ULP(const sw::universal::posit<nbits,es>& s) { 
	using namespace sw::universal;
	using Scalar = sw::universal::posit<nbits, es>;
	int maxDigits = std::numeric_limits<Scalar>::max_digits10;
	std::cout << "scalar type: " << std::setw(50) << typeid(s).name() << " max digits: " << std::setw(5) << maxDigits << '\n';
	// needs C++20 to become constexpr for generic universal types
	Scalar zero = 0;
	Scalar infinity = std::numeric_limits<Scalar>::infinity();
	auto precision = std::cout.precision();
	std::cout << std::setprecision(maxDigits);
	std::cout << "prior  : " << nextafter(s, zero) << '\n'
		 << "value  : " << s << "                 " << hex_format(s) << '\n'
		 << "post   : " << nextafter(s, infinity) << '\n';
	std::cout << std::setprecision(precision) << std::dec;
}

template<typename Scalar>
void smallest_value() {
	using std::nexttoward; // when called with a native Real type
	std::cout << "first representable value greater than zero: " << nexttoward(Scalar(0.0), 1.0L) << '\n';
	std::cout << "first representable value less than zero   : " << nexttoward(Scalar(0.0), -1.0L) << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::cout << "ULP math\n";

	// operations on the unit in last position

	std::streamsize precision = std::cout.precision();

	ULP(1.25e-10f);
	ULP(1.25e-20);
	ULP(1.25e-40l);

	ULP(posit< 32, 2>(1.25e-10f));
	ULP(posit< 64, 3>(1.25e-20));
	ULP(posit<128, 4>(1.25e-40l));

	smallest_value<float>();
	smallest_value<double>();
	smallest_value<long double>();
	smallest_value< posit< 32, 2> >();
	smallest_value< posit< 64, 3> >();
	smallest_value< posit<128, 4> >();

	std::cout << std::setprecision(precision) << std::endl;
	
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
