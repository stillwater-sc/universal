// quadratic.cpp: demonstration of catastrophic cancellation in the quadratic formula
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <limits>
#if (__cplusplus == 202003L) || (_MSVC_LANG == 202003L)
#include <numbers>    // high-precision numbers
#endif

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

template<typename Scalar>
std::pair<Scalar, Scalar> Quadratic(const Scalar& a, const Scalar& b, const Scalar& c) {
	std::pair<Scalar, Scalar> roots;

	Scalar b2_minus_4ac = b * b - 4 * a * c;
	Scalar sqrt_b2_minus_4ac = sqrt(b2_minus_4ac);
	roots.first  = (-b - sqrt_b2_minus_4ac) / (2 * a);
	roots.second = (-b + sqrt_b2_minus_4ac) / (2 * a);
	return roots;
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

int main()
try {
	using namespace sw::universal;

	std::cout << "catastrophic cancellation in the quadratic formula\n";

	auto precision = std::cout.precision();
	std::cout << std::setprecision(15);

	using Float16 = cfloat<16,  5, uint16_t, true, true, false>;
	using Float32 = cfloat<32,  8, uint16_t, true, true, false>;
	using Float64 = cfloat<64, 11, uint16_t, true, true, false>;
	using FloatSP = float;
	using FloatDP = double;
	using Posit32 = sw::universal::posit<32, 2>;
	using Posit64 = sw::universal::posit<64, 2>;
	using Fixed64 = sw::universal::fixpnt<64, 16>;

	float a = 1.0f;
	float b = 1.0e5f;
	float c = 1.0f;

	std::cout << "half precision floating-point\n";
	CompareTerms<Float16>(a, b, c);
	std::cout << '\n';

	std::cout << "single precision floating-point\n";
	CompareTerms<Float32>(a, b, c);
	std::cout << '\n';

	std::cout << "native single precision floating-point\n";
	CompareTerms<FloatSP>(a, b, c);
	std::cout << '\n';

	std::cout << "double precision floating-point\n";
	CompareTerms<Float64>(a, b, c);
	std::cout << '\n';

	std::cout << "native double precision floating-point\n";
	CompareTerms<FloatDP>(a, b, c);
	std::cout << '\n';

	std::cout << "single precision posit<32, 2>\n";
	CompareTerms<Posit32>(a, b, c);
	std::cout << '\n';

	std::cout << "custom precision posit<40, 2>\n";
	CompareTerms<posit<40,2>>(a, b, c);
	std::cout << '\n';

	std::cout << "double precision posit<64, 2>\n";
	CompareTerms<Posit64>(a, b, c);
	std::cout << '\n';

	std::cout << "fixed-point fixpnt<64, 16>\n";
	CompareTerms<Fixed64>(a, b, c);
	std::cout << '\n';

	integer<64> inta(a);
	integer<64> intb(b);
	integer<64> intc(c);
	integer<64> difference = intb * intb - 4 * inta * intc;
	std::cout << "    (b^2 - 4ac)      : " << sw::universal::to_binary(difference) << " : " << difference << '\n';;

	{
		using Scalar = Posit32;
		Scalar a{ 3.0 }, b{ 5.0 }, c{ -7.0 };
		std::pair<Scalar, Scalar> roots = Quadratic(a, b, c);
		std::cout << "roots: " << roots.first << ", " << roots.second << std::endl;
	}
	{
		using Scalar = sw::universal::fixpnt<16,8>;
		Scalar a{ 3.0 }, b{ 5.0 }, c{ -7.0 };
		std::pair<Scalar, Scalar> roots = Quadratic(a, b, c);
		std::cout << "roots: " << roots.first << ", " << roots.second << std::endl;
	}

	{
		Fixed64 a;
		a = 100000.0f;
		std::cout << "a   : " << to_binary(a) << " : " << a << '\n';
		a *= a;
		std::cout << "a^2 : " << to_binary(a) << " : " << a << '\n';
	}

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
