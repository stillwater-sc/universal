// ulp_math.cpp: example program to show operations on Unit in Last Position
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <ostream>
#include <limits>
#include <numeric>   // nextafter

// select the number systems we would like to compare
#include <universal/number/integer/integer>
#include <universal/number/fixpnt/fixpnt>
#include <universal/number/areal/areal>
#include <universal/number/posit/posit>
#include <universal/number/lns/lns>

#include <universal/number/posit/numeric_limits.hpp>

template<typename Scalar>
void ULP(std::ostream& ostr, const Scalar& s) {
	using namespace sw::universal;
	int maxDigits = std::numeric_limits<Scalar>::max_digits10;
	ostr << "scalar type: " << std::setw(50) << typeid(s).name() << " max digits: " << std::setw(5) << maxDigits << '\n';
	// needs C++20 to become constexpr for generic universal types
	Scalar zero     = 0;
	Scalar infinity = std::numeric_limits<Scalar>::infinity();
	auto precision = ostr.precision();
	ostr << std::setprecision(maxDigits);    
	ostr << "prior  : " << nextafter(s, zero) << '\n'
		 << "value  : " << s << "                 " << std::hexfloat << s << std::dec << '\n'   // <--- need to overload hexfloat for posit hex_format
		 << "post   : " << nextafter(s, infinity) << '\n';
	ostr << std::setprecision(precision);
}

template<size_t nbits, size_t es>
void ULP(std::ostream& ostr, const sw::universal::posit<nbits,es>& s) {
	using namespace sw::universal;
	using Scalar = sw::universal::posit<nbits, es>;
	int maxDigits = std::numeric_limits<Scalar>::max_digits10;
	ostr << "scalar type: " << std::setw(50) << typeid(s).name() << " max digits: " << std::setw(5) << maxDigits << '\n';
	// needs C++20 to become constexpr for generic universal types
	Scalar zero = 0;
	Scalar infinity = std::numeric_limits<Scalar>::infinity();
	auto precision = ostr.precision();
	ostr << std::setprecision(maxDigits);
	ostr << "prior  : " << nextafter(s, zero) << '\n'
		 << "value  : " << s << "                 " << hex_format(s) << '\n'
		 << "post   : " << nextafter(s, infinity) << '\n';
	ostr << std::setprecision(precision) << std::dec;
}

template<typename Scalar>
void smallest_value(std::ostream& ostr) {
	ostr << "first representable value greater than zero: " << nexttoward(Scalar(0.0), 1.0L) << '\n';
	ostr << "first representable value less than zero   : " << nexttoward(Scalar(0.0), -1.0L) << '\n';
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	cout << "ULP math " << endl;

	// operations on the unit in last position

	streamsize precision = cout.precision();

	ULP(cout, 1.25e-10f);
	ULP(cout, 1.25e-20);
	ULP(cout, 1.25e-40l);

	ULP(cout, posit< 32, 2>(1.25e-10f));
	ULP(cout, posit< 64, 3>(1.25e-20));
	ULP(cout, posit<128, 4>(1.25e-40l));

	smallest_value<float>(cout);
	smallest_value<double>(cout);
	smallest_value<long double>(cout);
	smallest_value< posit< 32, 2> >(cout);
	smallest_value< posit< 64, 3> >(cout);
	smallest_value< posit<128, 4> >(cout);

	cout << setprecision(precision);
	cout << endl;
	
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << "Caught exception: " << msg << std::endl;
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
