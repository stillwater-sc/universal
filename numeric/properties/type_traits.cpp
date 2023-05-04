// type_traits.cpp: experiments with type traits of native floats, integers, fixpnts, and posit number types
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/native/native.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/posit/posit.hpp>

template<typename Scalar>
bool TestPosit() {
	bool isPosit = false;
	if (sw::universal::is_posit<Scalar>) {
		std::cout << "type is a posit: " << typeid(Scalar).name() << "  ";
		isPosit = true;
	}
	else {
		std::cout << "type is not a posit: " << typeid(Scalar).name() << "  ";
	}
	return isPosit;
}

template<typename Scalar>
bool TestFixpnt() {
	bool isFixpnt = false;
	if (sw::universal::is_fixpnt<Scalar>) {
		std::cout << "type is a fixed-point: " << typeid(Scalar).name() << "  ";
		isFixpnt = true;
	}
	else {
		std::cout << "type is not a fixed-point: " << typeid(Scalar).name() << "  ";
	}
	return isFixpnt;
}

int main(int argc, char** argv)
try {
	using namespace sw::universal;

	constexpr size_t nbits = 32;
	constexpr size_t es = 2;
	using Scalar = posit<nbits,es>;

	// print detailed bit-level computational intermediate results
	// bool verbose = false;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	static_assert( !std::is_integral< sw::universal::posit<nbits,es> >(), "a posit<nbits,es> is not an integral type");
	static_assert( !std::is_floating_point< sw::universal::posit<nbits, es> >(), "a posit<nbits,es> is a floating point type");
	static_assert( !std::is_arithmetic< sw::universal::posit<nbits, es> >(), "a posit<nbits,es> is not an arithmetic type");
	static_assert( !std::is_arithmetic< Scalar >(), "This Scalar is not an arithmetic type");

	static_assert( is_posit< sw::universal::posit<nbits, es> >, "a posit<nbits,es> is a posit type");
	static_assert( is_posit_trait< Scalar >(), "This Scalar is a posit type");

	constexpr size_t rbits = nbits / 2;
	using Fixpnt = fixpnt<nbits, rbits>;
	static_assert( is_fixpnt< fixpnt<nbits, rbits> >, "a fixpnt<nbits,rbits> is a fixed-point type");
	static_assert( is_fixpnt_trait< Fixpnt >(), "This Scalar is a fixed-point type");

	std::cout << (TestPosit<long double>() ?  "FAIL\n"  : "PASS\n");
	std::cout << (TestPosit<posit<1024, 7>>() ? "PASS\n" : "FAIL\n");
	std::cout << (TestPosit<fixpnt<32, 16>>() ? "FAIL\n" : "PASS\n");
	std::cout << (TestFixpnt<long double>() ? "FAIL\n" : "PASS\n");
	std::cout << (TestFixpnt<posit<1024, 7>>() ? "FAIL\n" : "PASS\n");
	std::cout << (TestFixpnt<fixpnt<32, 16>>() ? "PASS\n" : "FAIL\n");

	// restore the previous ostream precision
	std::cout << std::setprecision(precision);

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
