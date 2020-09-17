// ulp_math.cpp: example program to show operations on Unit in Last Position
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <ostream>
#include <limits>
#include <numeric>   // nextafter

// select the number systems we would like to compare
#include <universal/integer/integer>
#include <universal/fixpnt/fixpnt>
#include <universal/areal/areal>
#include <universal/posit/posit>
#include <universal/lns/lns>

template<typename Scalar>
void ULP(std::ostream& ostr, const Scalar& s) {
	using namespace sw::unum;
	int maxDigits = std::numeric_limits<Scalar>::max_digits10;
	ostr << "scalar type: " << std::setw(50) << typeid(s).name() << " max digits: " << std::setw(5) << maxDigits << '\n';
	// needs C++20 to become constexpr for generic universal types
	Scalar zero     = 0;
	Scalar infinity = std::numeric_limits<Scalar>::infinity();
	auto precision = ostr.precision();
	ostr << std::setprecision(maxDigits) << std::hexfloat;    // <--- need to overload hexfloat for posit hex_format
	ostr << "prior  : " << nextafter(s, zero) << '\n'
		 << "value  : " << s << '\n'
		 << "post   : " << nextafter(s, infinity) << '\n';
	ostr << std::setprecision(precision) << std::dec;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	cout << "ULP math " << endl;

	// operations on the unit in last position

	streamsize precision = cout.precision();

	ULP(cout, 0.125e-10f);
	ULP(cout, 0.125e-10);
	ULP(cout, 0.125e-10l);

	ULP(cout, posit< 32, 2>(0.125e-10f));
	ULP(cout, posit< 64, 3>(0.125e-10));
	ULP(cout, posit<128, 4>(0.125e-10l));

	cout << setprecision(precision);
	cout << endl;
	
	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
