// ieee754.cpp : native IEEE-754 operations
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <string>
#include <limits>
#include <universal/native/ieee754.hpp>

template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
void NativeEnvironment(Real r) {
	using namespace sw::universal;

	std::cout << "scale of " << r << " is 2^" << scale(r) << " ~ 10^" << int(scale(r) / 3.3) << '\n';
	std::cout << to_binary(r, true) << " " << r << '\n';
	std::cout << color_print(r) << " " << r << '\n';
}

template<typename Real,
	typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
void DescendingScales() {
	std::string size("unknown");

	switch (sizeof(Real)) {
	case 4:
		size = "single";
		break;
	case 8:
		size = "double";
		break;
	case 16:
		size = "quadruple";
		break;
	}
	std::cout << "IEEE-754 " << size << " precision scales:             in descending order\n";

	auto oldPrecision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Real>::digits10);

	long long largestScale = std::numeric_limits<Real>::max_exponent - 1;
	Real r = sw::universal::ipow<double>(largestScale);
	for (long long i = 0; i < largestScale + 1; ++i) {
		std::cout << std::setw(4) << largestScale - i << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		r /= 2.0;
	}
	// this gets us to 1.0, next enumerate the negative scaled normals
	int smallestScale = std::numeric_limits<Real>::min_exponent - 1;
	for (int i = 0; i > smallestScale; --i) {
		std::cout << std::setw(4) << i << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		r /= 2.0;
	}
	// this gets us to the smallest normal, next enumerate the subnormals
	for (int i = 0; i < sw::universal::ieee754_parameter<Real>::fbits; ++i) {
		std::cout << std::setw(4) << (smallestScale - i) << " : " << sw::universal::to_binary(r) << " : " << r << '\n';
		r /= 2.0;
	}
	std::cout << std::setprecision(oldPrecision);
}

void InfinityAdditions() {
	std::cout << "IEEE-754 addition with infinites\n";
	constexpr float fa = std::numeric_limits<float>::infinity();
	constexpr float fb = -fa;
	std::cout << fa << " + " << fa << " = " << (fa + fa) << " : " << sw::universal::to_binary(fa + fa) << '\n';
	std::cout << fa << " + " << fb << " = " << (fa + fb) << " : " << sw::universal::to_binary(fa + fb) << '\n';
	std::cout << fb << " + " << fa << " = " << (fb + fa) << " : " << sw::universal::to_binary(fb + fa) << '\n';
	std::cout << fb << " + " << fb << " = " << (fb + fb) << " : " << sw::universal::to_binary(fb + fb) << '\n';
}

int main()
try {
	using namespace sw::universal;

	// compare bits of different real number representations
	
	float f         = 1.0e1;
	double d        = 1.0e10;
#if LONG_DOUBLE_SUPPORT
	long double ld  = 1.0e100;
#else
	std::cout << "This environment does not support a native long double format\n";
#endif

	NativeEnvironment(f);
	NativeEnvironment(d);
#if LONG_DOUBLE_SUPPORT
	NativeEnvironment(ld);
#endif

	// show all the different presentations for the different IEEE-754 native formats
	valueRepresentations(f);
	valueRepresentations(d);
#if LONG_DOUBLE_SUPPORT
	valueRepresentations(ld);
#endif

	// show the scales that an IEEE-754 type contains
	DescendingScales<float>();

	// show the results of addition with infinites
	InfinityAdditions();

	std::cout << std::endl; // flush the stream

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
