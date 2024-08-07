// quadratic.cpp: demonstration of catastrophic cancellation in the quadratic formula
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#include <numbers>    // high-precision constants

/* 
	Background on the poor numerical performance of the quadratic solution
	https://people.eecs.berkeley.edu/~wkahan/Qdrtcs.pdf
	https://news.ycombinator.com/item?id=16949156
	https://pavpanchekha.com/blog/accurate-quadratic.html
 */
#include <universal/number/integer/integer.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/posit/posit.hpp>

#include <cstddef>
#include <stdexcept>
#include <cstring>
#include <ostream>

using Float16 = sw::universal::cfloat<16, 5, uint16_t>; // , true, true, false > ;
using Float32 = sw::universal::cfloat<32, 8, uint32_t>; // , true, true, false > ;
using Float48 = sw::universal::cfloat<48, 8, uint32_t>; // , true, true, false > ;
using Float64 = sw::universal::cfloat<64, 11, uint32_t>; // , true, true, false > ;
using FloatSP = float;
using FloatDP = double;
using Posit32 = sw::universal::posit<32, 2>;
using Posit48 = sw::universal::posit<48, 2>;
using Posit64 = sw::universal::posit<64, 2>;
using Fixed64 = sw::universal::fixpnt<64, 23>;  // nbits = 64 is necessary to cover dynamic range of b^2 - 4ac, and rbits = 23 is necessary to capture the the difference between b and SQRT(b^2 - 4ac)

template<typename Scalar>
std::pair<Scalar, Scalar> Quadratic(const Scalar& a, const Scalar& b, const Scalar& c) {
	std::pair<Scalar, Scalar> roots;

	Scalar b2_minus_4ac = b * b - 4 * a * c;
	Scalar sqrt_b2_minus_4ac = sqrt(b2_minus_4ac);
	roots.first  = (-b - sqrt_b2_minus_4ac) / (2 * a);
	roots.second = (-b + sqrt_b2_minus_4ac) / (2 * a);
	return roots;
}

void CompareBigTerms(float a, float b, float c) {
	using namespace sw::universal;
	integer<64> inta(a);
	integer<64> intb(b);
	integer<64> intc(c);
	integer<64> difference = intb * intb - 4 * inta * intc;
	std::cout << "    (b^2 - 4ac)      : " << sw::universal::to_binary(difference) << " : " << difference << '\n';

	{
		Fixed64 v;
		v = 100000.0f;
		std::cout << "a   : " << to_binary(v) << " : " << v << '\n';
		v *= v;
		std::cout << "a^2 : " << to_binary(v) << " : " << v << '\n';
	}
}

template<typename Real>
void CompareTerms(Real a, Real b, Real c) {
    std::cout << "a                    : " << sw::universal::to_binary(a) << " : " << a << '\n';
    std::cout << "b                    : " << sw::universal::to_binary(b) << " : " << b << '\n';
    std::cout << "c                    : " << sw::universal::to_binary(c) << " : " << c << '\n';
	Real b_square = b * b;
	Real fourac = 4 * a * c;
	Real difference = b_square - fourac;
	Real sqrt_b_square_minus_fourac = sqrt(difference);
    std::cout << "b^2                  : " << sw::universal::to_binary(b_square) << " : " << (b_square) << '\n';
    std::cout << "4ac                  : " << sw::universal::to_binary(fourac) << " : " << (fourac) << '\n';
	std::cout << "    (b^2 - 4ac)      : " << sw::universal::to_binary(difference) << " : " << difference << '\n';
	std::cout << "sqrt(b^2 - 4ac)      : " << sw::universal::to_binary(sqrt_b_square_minus_fourac) << " : " << sqrt_b_square_minus_fourac << '\n';
	std::cout << "-b                   : " << sw::universal::to_binary(-b) << " : " << (-b) << '\n';
	Real numerator = -b + sqrt_b_square_minus_fourac;
	Real denominator = 2 * a;
	std::cout << "-b + sqrt(b^2 - 4ac) : " << sw::universal::to_binary(numerator) << " : " << numerator << '\n';
	std::cout << "2a                   : " << sw::universal::to_binary(denominator) << " : " << denominator << '\n';
	Real root = numerator / denominator;
	std::cout << "root                 : " << sw::universal::to_binary(root) << " : " << root << '\n';
}

void CompareTypes(float a, float b, float c) {
	std::cout << "16-bit floating-point\n";
	CompareTerms<Float16>(a, b, c);
	std::cout << '\n';

	std::cout << "32-bit floating-point\n";
	CompareTerms<Float32>(a, b, c);
	std::cout << '\n';

	std::cout << "native single precision floating-point\n";
	CompareTerms<FloatSP>(a, b, c);
	std::cout << '\n';

	std::cout << "48-bit floating-point\n";
	CompareTerms<Float48>(a, b, c);
	std::cout << '\n';

	std::cout << "64-bit floating-point\n";
	CompareTerms<Float64>(a, b, c);
	std::cout << '\n';

	std::cout << "native double precision floating-point\n";
	CompareTerms<FloatDP>(a, b, c);
	std::cout << '\n';

	std::cout << "single precision posit<32, 2>\n";
	CompareTerms<Posit32>(a, b, c);
	std::cout << '\n';

	std::cout << "custom precision posit<48, 2>\n";
	CompareTerms<Posit48>(a, b, c);
	std::cout << '\n';

	std::cout << "double precision posit<64, 2>\n";
	CompareTerms<Posit64>(a, b, c);
	std::cout << '\n';

	std::cout << "fixed-point fixpnt<64, 16>\n";
	CompareTerms<Fixed64>(a, b, c);
	std::cout << '\n';
}

void CompareRoots(float fa, float fb, float fc) {
	std::cout << "a*x^2 + b*x + c = 0 : " << fa << ", " << fb << ", " << fc << '\n';
	constexpr unsigned TAG_WIDTH = 80;
	{
		using Scalar = Float32;
		Scalar a{ fa }, b{ fb }, c{ fc };
		std::pair<Scalar, Scalar> roots = Quadratic(a, b, c);
		std::cout << std::setw(TAG_WIDTH) << std::left << type_tag(Scalar()) << " roots: " << roots.first << ", " << roots.second << std::endl;
	}
	{
		using Scalar = Posit32;
		Scalar a{ fa }, b{ fb }, c{ fc };
		std::pair<Scalar, Scalar> roots = Quadratic(a, b, c);
		std::cout << std::setw(TAG_WIDTH) << std::left << type_tag(Scalar()) << " roots: " << roots.first << ", " << roots.second << std::endl;
	}
	{
		using Scalar = Fixed64; 
		//using Scalar = sw::universal::fixpnt<32, 16, sw::universal::Saturate>;  // divide is TBD
		Scalar a{ fa }, b{ fb }, c{ fc };
		try {
			std::pair<Scalar, Scalar> roots = Quadratic(a, b, c);
			std::cout << std::setw(TAG_WIDTH) << type_tag(Scalar()) << " roots: " << roots.first << ", " << roots.second << std::endl;
		} 
		catch (const sw::universal::universal_arithmetic_exception& err) {
			std::cerr << "Caught unexpected universal arithmetic exception: " << err.what() << '\n';
			std::cerr << "Likely culprit is that the dynamic range of the fixpnt is insufficient to capture the b^2 - 4ac term\n";
			std::cerr << "b    : " << to_binary(b) << " : " << b << '\n';
			std::cerr << "b^ 2 : " << b * b << '\n';
			std::cerr << "4ac  : " << 4 * a * c << '\n';
		}
	}
	{
		using Scalar = Float64;
		Scalar a{ fa }, b{ fb }, c{ fc };
		std::pair<Scalar, Scalar> roots = Quadratic(a, b, c);
		std::cout << std::setw(TAG_WIDTH) << type_tag(Scalar()) << " roots: " << roots.first << ", " << roots.second << std::endl;
	}
	{
		using Scalar = Posit64;
		Scalar a{ fa }, b{ fb }, c{ fc };
		std::pair<Scalar, Scalar> roots = Quadratic(a, b, c);
		std::cout << std::setw(TAG_WIDTH) << type_tag(Scalar()) << " roots: " << roots.first << ", " << roots.second << std::endl;
	}
}

template<typename Real>
Real MyKernel(const Real& a, const Real& b) {
	return a * b;  // replace this with your kernel computation
}

constexpr double pi = 3.14159265358979323846;

void Test() {
	using Real = sw::universal::half; // half-precision IEEE-754 floating-point  

	Real a = sqrt(2);
	Real b = pi;
	std::cout << "Result: " << MyKernel(a, b) << std::endl;
}

int main()
try {
	using namespace sw::universal;

	std::cout << "catastrophic cancellation in the quadratic formula\n";

	auto precision = std::cout.precision();
	std::cout << std::setprecision(15);

	CompareRoots(1.0f, 1.0e5f, 1.0f);

	std::cout << "\n\n\n";

	CompareRoots(3.0f, 5.0f, -7.0f);

	std::cout << "\n\n\n";

	Test();

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
