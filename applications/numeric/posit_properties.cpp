// posit_properties.cpp example program comparing epsilon and minpos across posit configurations
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

// select the number systems we would like to compare
#include <universal/integer/integer>
#include <universal/fixpnt/fixpnt>
#include <universal/areal/areal>
#include <universal/posit/posit>
#include <universal/lns/lns>
#include <universal/valid/valid>

//constexpr long double pi     = 3.14159265358979323846;
//constexpr long double e      = 2.71828182845904523536;
//constexpr long double log_2e = 1.44269504088896340736;

// variadic template to generate a range of nbits
template<size_t ... nbits, size_t es>
void eps_impl(std::index_sequence<nbits...>, const size_t index) }
    std::array<size_t, sizeof...(nbits)> eps = std::numeric_limits<sw::unum::posit<nbits,es>>::epsilon();
    return eps[index];
}

template<size_t es = 2>
const long double eps(const size_t nbits) {
	return eps_impl(std::make_index_sequence<64>(), nbits);
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	cout << "epsilon/minpos for different number systems " << endl;

	// report on smallest number, precision and dynamic range of the number system

	streamsize precision = cout.precision();

	

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
