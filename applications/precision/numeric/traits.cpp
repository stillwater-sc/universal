// traits.cpp example program comparing number trait of different number systems
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>

// select the number systems we would like to compare
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit1/posit1.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/valid/valid.hpp>

//constexpr long double pi     = 3.14159265358979323846;
//constexpr long double e      = 2.71828182845904523536;
//constexpr long double log_2e = 1.44269504088896340736;

int main()
try {
	using namespace sw::universal;

	std::cout << "numeric_limits for different number systems\n";

	using int32    = integer<32, std::uint32_t>;
	using fixpnt32 = fixpnt<32, 16, Modulo, std::uint32_t>;
	using cfloat32 = cfloat<32, 8, std::uint32_t, true, false, false>;
	using posit32  = posit<32, 2>;
	using areal32  = areal<32, 8, std::uint32_t>;
	using lns32    = lns<32, 8, std::uint32_t>;

	// report on precision and dynamic range of the number system

	std::streamsize precision = std::cout.precision();
	
	constexpr size_t columnWidth = 30;
	numberTraits<std::int32_t, columnWidth>(std::cout);
	numberTraits<int32, columnWidth>(std::cout);
	numberTraits<fixpnt32, columnWidth>(std::cout);
	numberTraits<float, columnWidth>(std::cout);
	numberTraits<areal32, columnWidth>(std::cout);
	numberTraits<cfloat32, columnWidth>(std::cout);
	numberTraits<posit32, columnWidth>(std::cout);
	numberTraits<lns32, columnWidth>(std::cout);

	std::cout << minmax_range<int>() << '\n';
	std::cout << minmax_range<float>() << '\n';
	std::cout << minmax_range<cfloat32>() << '\n';
	std::cout << minmax_range<posit32>() << '\n';
	std::cout << minmax_range<lns32>() << '\n';

	std::cout << dynamic_range<int>() << '\n';
	std::cout << dynamic_range<float>() << '\n';
	std::cout << dynamic_range<cfloat32>() << '\n';
	std::cout << dynamic_range<posit32>() << '\n';
	std::cout << dynamic_range<lns32>() << '\n';

	std::cout << symmetry_range<float>() << '\n';
	std::cout << symmetry_range<cfloat32>() << '\n';
	std::cout << symmetry_range<posit32>() << '\n';
	std::cout << symmetry_range<lns32>() << '\n';

	compareNumberTraits<float, areal32>(std::cout);
	compareNumberTraits<float, cfloat32>(std::cout);
	compareNumberTraits<float, posit32>(std::cout);
	compareNumberTraits<float, lns32>(std::cout);

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
