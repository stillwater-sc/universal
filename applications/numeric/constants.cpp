// numbers.cpp: example program to use C++20 <numbers> high precision constants
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#if (__cplusplus == 202003L) || (_MSVC_LANG == 202003L)
#include <numbers>    // high-precision numbers
#endif
#include <universal/utility/number_system_properties.hpp> //minmax_range etc. for native types
#include <universal/verification/performance_runner.hpp>

// select the number systems we would like to compare
#define FIXPNT_NATIVE_SQRT 1
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <ostream>

template<typename Scalar>
void SqrtWorkload(uint64_t NR_OPS) {
	Scalar a{ 0 }, c{ 0 };
	size_t maxpos = (1ull << (Scalar::nbits - Scalar::rbits - 1));
	for (uint64_t i = 0; i < NR_OPS; ++i) {
		a = (i % maxpos);
		c = sw::universal::sqrt(a);
	}
	if (a == c) std::cout << "amazing\n";
}

template<typename Float>
std::string type_tag(Float v) {
	return typeid(v).name();
}

template<typename Scalar>
void Sqrt(double v) {
	Scalar s{ v };
	s = sqrt(s);  // sqrt in template type
	std::cout << std::setw(20) << type_tag(s) << " : " << s << '\n';
}

template<typename Scalar>
void Compare(double v) {
	std::cout << "sqrt(" << v << ")\n";
	Sqrt<Scalar>(v);
	Scalar a(v);
	auto b = BabylonianMethod(a);
	std::cout << b * b << '\n';
	b = BabylonianMethod2(a);
	std::cout << b * b << '\n';
	b = BabylonianMethod3(a);
	std::cout << b * b << '\n';
}

int main()
try {
	using namespace sw::universal;

	std::cout << "high-precision constants\n";

	using Longd = long double;
	using Fixed = fixpnt<80,75>;
	using Posit = posit<64,2>;
	using Float = cfloat<128, 15, uint32_t>;
	using Areal = areal<128, 15,uint32_t>;
	using Lns   = lns<128, uint32_t>;


	std::streamsize precision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Longd>::max_digits10);
	std::cout << "long double digits of precision : " << std::numeric_limits<Longd>::max_digits10 << '\n';

	constexpr size_t NR_OPS = 1024;
	PerformanceRunner(type_tag(Fixed()) + "::sqrt ", SqrtWorkload< Fixed >, NR_OPS);

//	Compare<Fixed>(2.0);

	{
		std::cout << "sqrt(2)\n";
		float f = 2.0f;
		std::cout << sqrt(Longd(f)) << " : " << type_tag(Longd()) << '\n';
		std::cout << sqrt(Fixed(f)) << " : " << type_tag(Fixed()) << '\n';
		std::cout << sqrt(Posit(f)) << " : " << type_tag(Posit()) << '\n';
	}

	{
		std::cout << "sqrt(3)\n";
		float f = 3.0f;
		std::cout << sqrt(Longd(f)) << " : " << type_tag(Longd()) << '\n';
		std::cout << sqrt(Fixed(f)) << " : " << type_tag(Fixed()) << '\n';
		std::cout << sqrt(Posit(f)) << " : " << type_tag(Posit()) << '\n';
	}

	{
		std::cout << "sqrt(5)\n";
		float f = 5.0f;
		std::cout << sqrt(Longd(f)) << " : " << type_tag(Longd()) << '\n';
		std::cout << sqrt(Fixed(f)) << " : " << type_tag(Fixed()) << '\n';
		std::cout << sqrt(Posit(f)) << " : " << type_tag(Posit()) << '\n';
	}

	{
		std::cout << "sqrt(7)\n";
		float f = 7.0f;
		std::cout << sqrt(Longd(f)) << " : " << type_tag(Longd()) << '\n';
		std::cout << sqrt(Fixed(f)) << " : " << type_tag(Fixed()) << '\n';
		std::cout << sqrt(Posit(f)) << " : " << type_tag(Posit()) << '\n';
	}

	std::cout << std::setprecision(precision);
	std::cout << std::endl;
	
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
