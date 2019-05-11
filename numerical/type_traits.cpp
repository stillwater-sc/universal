// type_traits.cpp: experiments with type traits of posit number types
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#define ALIASING_ALLOWED
#include "common.hpp"

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::unum;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	bool verbose = false;

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	static_assert( !is_integral< sw::unum::posit<nbits,es> >(), "a posit<nbits,es> is not an integral type");
	static_assert( is_floating_point< sw::unum::posit<nbits, es> >(), "a posit<nbits,es> is a floating point type");
	static_assert( is_arithmetic< sw::unum::posit<nbits, es> >(), "a posit<nbits,es> is an arithmetic type");

	static_assert( is_arithmetic< Scalar >(), "This Scalar is an arithmetic type");

	// restore the previous ostream precision
	cout << setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (std::runtime_error& err) {
	std::cerr << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
