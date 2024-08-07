// midpoint.cpp: example program to use C++20 <cmath> lerp and midpoint functions
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <cmath>     // lerp
#include <numeric>   // midpoint
#include <numbers>   // high-precision constants

// select the number systems we would like to compare
#include <universal/native/ieee754.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/areal/areal.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/lns/lns.hpp>

#include <universal/verification/test_case.hpp>


void example() {
	using std::midpoint;
#if (__cplusplus >= 202002L)

	std::cout << std::setprecision(50);
	std::cout << "midpoint          " << std::midpoint(5, 7) << '\n';
	std::cout << "lerp              " << std::lerp(5, 7, 0.5f) << '\n';

	using Real = float;

	Real a(1.0), b(2.0);
	Real mp = midpoint(a, b);
	std::cout << "midpoint(1.0, 2.0) = " << mp << '\n';

	std::cout << "a=" << a << '\n'
		<< "b=" << b << '\n'
		<< "mid point=" << std::lerp(a, b, 0.5f) << '\n'
		<< std::boolalpha << (a == std::lerp(a, b, 0.0f)) << '\n';
#else
	std::cerr << "midpoint program requires C++20\n";
#endif
}

template<typename Real>
std::pair<Real, Real> GenerateRange(Real lb, Real nr_ulps = Real(7)) {
	using namespace sw::universal;

	Real n = nextafter(lb, Real(2.0) * lb);
	Real lb_ulp = n - lb;
	std::cout << "ULP    " << to_binary(lb_ulp) << " : " << lb_ulp << '\n';

	Real ub = lb + nr_ulps * lb_ulp;

	return std::pair<Real, Real>(lb, ub);
}

template<typename Real>
void Midpoint(const std::pair<Real, Real> p) {
	using namespace sw::universal;
	using std::midpoint;   // in case Real is a native type supported by std

	Real mp = std::midpoint(p.first, p.second);

	std::cout << type_tag(mp) << '\n';
	ReportValue(p.first);
	ReportValue(mp);
	ReportValue(p.second);
}

template<typename Real>
void GenerateMidpointTestCase(Real lb, Real multiple_ulps) {
	Midpoint<Real>(GenerateRange<Real>(lb, multiple_ulps));
}

int main()
try {
	using namespace sw::universal;
	using std::midpoint;

	std::cout << "lerp and midpoint operators\n";

	using int32    = integer<32>;
	using fixpnt32 = fixpnt<32,16>;
	using posit32  = posit<32,2>;
	using areal32  = areal<32,8>;
	using lns32    = lns<32,23>;

	// check difficult midpoint and lerp operations on different number systems

	std::streamsize precision = std::cout.precision();

	std::cout << std::setprecision(23);

	GenerateMidpointTestCase<float>(1, 7);
//	GenerateMidpointTestCase<int32>(1, 6);
//	GenerateMidpointTestCase<fixpnt32>(1, 6);
//	GenerateMidpointTestCase<posit32>(1, 6);
//	GenerateMidpointTestCase<areal32>(1, 6);
//	GenerateMidpointTestCase<lns32>(1, 6);

	std::cout << std::setprecision(precision);
	std::cout << std::endl;
	
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
