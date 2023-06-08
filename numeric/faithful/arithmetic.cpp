// arithmetic.cpp: faithfully rounded floating-point arithmetic
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
#include <universal/analysis/twosum.hpp>

template<typename Scalar>
void traceTwoSum(const Scalar& a, const Scalar& b, Scalar& s, Scalar& r) {
	std::cout << "twosum\n";
	std::cout << "a     " << a << '\n';
	std::cout << "b     " << b << '\n';
	s = a + b;
	std::cout << "s     " << s << '\n';
	Scalar bdiff = s - a;
	Scalar adiff = s - bdiff;
	std::cout << "adiff " << adiff << '\n';
	std::cout << "bdiff " << bdiff << '\n';
	volatile Scalar aerr = a - adiff;
	volatile Scalar berr = b - bdiff;
	std::cout << "aerr " << aerr << '\n';
	std::cout << "berr " << berr << '\n';
	r = aerr + berr;
}

template<typename Scalar>
void traceCascadingSum(const std::vector<Scalar>& v, Scalar& s, Scalar& r) {
	Scalar a, p, q;
	size_t N = v.size();
	p = v[0];
	r = 0;
	for (size_t i = 1; i < N; ++i) {
		a = p;
		twoSum(a, v[i], p, q);
		r += q;
		std::cout << "stage " << i << " : " << a << " + " << v[i] << " = " << p << " + " << q << " cumulative err: " << r << '\n';
	}
	s = p;
}


int main()
try {
	using namespace sw::universal;

	// preserve the existing ostream precision
	auto precision = std::cout.precision();
	std::cout << std::setprecision(12);

	// float
	{
		using Real = float;
		Real a, b, s, r;

		constexpr Real eps = std::numeric_limits<Real>::epsilon();
		constexpr Real epsHalf = std::numeric_limits<Real>::epsilon() / 2.0f;
		a = 0.5f + epsHalf;
		b = 1.0f;
		std::cout << std::fixed;
		std::cout << to_binary(eps) << " : " << eps << '\n';
		std::cout << to_binary(epsHalf) << " : " << epsHalf << '\n';
		std::cout << to_binary(a) << " : " << a << '\n';
		std::cout << to_binary(b) << " : " << b << '\n';
		twoSum(a, b, s, r);
		std::cout << a << " + " << b << " = " << s << " + " << r << '\n';
		// validation using a double
		double da(a), db(b), ds(s), dr(r);
		double sum = da + db;
		std::cout << "sum " << sum << " vs " << (ds + dr) << '\n';
		std::cout << std::defaultfloat;
	}

	{
		faithful<float> fa, fb, fs, fr;
		fa = 1;
		fb = 2.0;
		int scale = fb.scale();
		
		std::cout << "scale of fb : " << scale << '\n';
		std::cout << fb << '\n';

		using Real = float;
		Real a, b;
		constexpr Real eps = std::numeric_limits<Real>::epsilon();
		constexpr Real epsHalf = std::numeric_limits<Real>::epsilon() / 2.0f;
		a = 0.5f + epsHalf;
		b = 1.0f;
		fa = a;
		fb = b;
		fs = fa + fb;
		std::cout << "compensated sum : " << fs << '\n';

	}

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
