﻿// arithmetic.cpp: faithfully rounded floating-point arithmetic
//
// Copyright (C) 2017-2023 Stillwater Supercomputing, Inc.
//
// This file is part of the UNIVERSAL project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <universal/number/faithful/faithful.hpp>
#include <universal/number/cfloat/cfloat.hpp>
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
	Scalar aerr = a - adiff;
	Scalar berr = b - bdiff;
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

template<typename Real>
void CompensatedEvaluation() {
	using namespace sw::universal;

	std::cout << "+-------\nCompensated Evaluation for type : " << type_tag(Real()) << '\n';

	Real a, b;
	Real eps = std::numeric_limits<Real>::epsilon();
	Real epsHalf = std::numeric_limits<Real>::epsilon() / 2.0f;
	a = 0.5f + epsHalf;
	b = 1.0f + eps;
	std::cout << "a = 0.5 + half epsilon : " << to_binary(a) << " : " << a << '\n';
	std::cout << "b = 1.0 + epsilon      : " << to_binary(b) << " : " << b << '\n';
	// validation using a double
	double da(a), db(b);

	faithful<Real> fa, fb, fsum;
	fa = a;
	fb = b;
	fsum = fa + fb;
	std::cout << "compensated sum        : " << fsum << " : " << Real(fsum) << '\n';
	double dsum = da + db;
	std::cout << "reference   sum        : " << dsum << '\n';

	faithful<Real> fdiff;
	fdiff = fa - fb;
	std::cout << "compensated difference : " << fdiff << " : " << Real(fdiff) << '\n';
	double ddiff = da - db;
	std::cout << "reference   difference : " << ddiff << '\n';

	faithful<Real> fprod;
	fprod = fa * fb;
	std::cout << "compensated product    : " << fprod << " : " << Real(fprod) << '\n';
	double dprod = da * db;
	std::cout << "reference   product    : " << dprod << '\n';

	faithful<Real> fdiv;
	fdiv = fa / fb;
	std::cout << "compensated ratio      : " << fdiv << " : " << Real(fdiv) << '\n';
	double ddiv = da / db;
	std::cout << "reference   ratio      : " << ddiv << '\n';

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
		std::cout << "epsilon      : " << to_binary(eps) << " : " << eps << '\n';
		std::cout << "half epsilon : " << to_binary(epsHalf) << " : " << epsHalf << '\n';
		std::cout << "operand a    : " << to_binary(a) << " : " << a << '\n';
		std::cout << "operand b    : " << to_binary(b) << " : " << b << '\n';
		twoSum(a, b, s, r);
		std::cout << a << " + " << b << " = " << s << " + " << r << '\n';
		// validation using a double
		double da(a), db(b), ds(s), dr(r);
		double sum = da + db;
		std::cout << "sum " << sum << " vs " << (ds + dr) << '\n';
		std::cout << std::defaultfloat;
	}

	std::cout << "\n\n";

	CompensatedEvaluation<quarter>();
	CompensatedEvaluation<half>();
	CompensatedEvaluation<single>();
	CompensatedEvaluation<float>();
//	CompensatedEvaluation<double>();

	CompensatedEvaluation<cfloat<8, 5, uint8_t, true, true, false> >();

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
