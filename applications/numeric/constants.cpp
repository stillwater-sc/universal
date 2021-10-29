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

// select the number systems we would like to compare
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <ostream>

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

int main()
try {
	using namespace sw::universal;

	std::cout << "high-precision constants\n";

	using Longd = long double;
	using Fixed = fixpnt<128,120>;
	using Posit = posit<128,2>;
	using Float = cfloat<128, 15, uint32_t>;
	using Areal = areal<128, 15,uint32_t>;
	using Lns   = lns<128, uint32_t>;


	std::streamsize precision = std::cout.precision();
	std::cout << std::setprecision(std::numeric_limits<Longd>::max_digits10);
	std::cout << "long double digits of precision : " << std::numeric_limits<Longd>::max_digits10 << '\n';

	// sqrt(2)
	{
		std::cout << "sqrt(2)\n";
		Sqrt<Longd>(2.0);
		Sqrt<Fixed>(2.0);
		Sqrt<Posit>(2.0);
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
