// type_traits.cpp: experiments with type traits of posit number types
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include "common.hpp"
// pull in the native tools
#include <universal/native/native>
// pull in the posit number system
#include <universal/number/posit/posit.hpp>
// pull in the arbitrary integer number system
#include <universal/number/integer/integer.hpp>
// pull in the fixed-point systems
#include <universal/number/fixpnt/fixpnt.hpp>

template<typename Scalar>
bool TestPosit() {
	using namespace std;
	bool isPosit = false;
	if (sw::universal::is_posit<Scalar>) {
		cout << "type is a posit: " << typeid(Scalar).name() << "  ";
		isPosit = true;
	}
	else {
		cout << "type is not a posit: " << typeid(Scalar).name() << "  ";
	}
	return isPosit;
}

template<typename Scalar>
bool TestFixpnt() {
	using namespace std;
	bool isFixpnt = false;
	if (sw::universal::is_fixpnt<Scalar>) {
		cout << "type is a fixed-point: " << typeid(Scalar).name() << "  ";
		isFixpnt = true;
	}
	else {
		cout << "type is not a fixed-point: " << typeid(Scalar).name() << "  ";
	}
	return isFixpnt;
}

int main(int argc, char** argv)
try {
	using namespace std;
	using namespace sw::universal;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = cout.precision();
	cout << setprecision(12);

	static_assert( !is_integral< sw::universal::posit<nbits,es> >(), "a posit<nbits,es> is not an integral type");
	static_assert( !is_floating_point< sw::universal::posit<nbits, es> >(), "a posit<nbits,es> is a floating point type");
	static_assert( !is_arithmetic< sw::universal::posit<nbits, es> >(), "a posit<nbits,es> is not an arithmetic type");
	static_assert( !is_arithmetic< Scalar >(), "This Scalar is not an arithmetic type");

	static_assert( is_posit< sw::universal::posit<nbits, es> >, "a posit<nbits,es> is a posit type");
	static_assert( is_posit_trait< Scalar >(), "This Scalar is a posit type");

	constexpr size_t rbits = nbits / 2;
	using Fixpnt = fixpnt<nbits, rbits>;
	static_assert( is_fixpnt< fixpnt<nbits, rbits> >, "a fixpnt<nbits,rbits> is a fixed-point type");
	static_assert( is_fixpnt_trait< Fixpnt >(), "This Scalar is a fixed-point type");

	cout << (TestPosit<long double>() ?  "FAIL\n"  : "PASS\n");
	cout << (TestPosit<posit<1024, 7>>() ? "PASS\n" : "FAIL\n");
	cout << (TestPosit<fixpnt<32, 16>>() ? "FAIL\n" : "PASS\n");
	cout << (TestFixpnt<long double>() ? "FAIL\n" : "PASS\n");
	cout << (TestFixpnt<posit<1024, 7>>() ? "FAIL\n" : "PASS\n");
	cout << (TestFixpnt<fixpnt<32, 16>>() ? "PASS\n" : "FAIL\n");

	// restore the previous ostream precision
	cout << setprecision(precision);

	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const sw::universal::posit_internal_exception& err) {
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
