//  bakers_map.cpp : The baker's map is a chaotic map from the unit squre into itself.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/number/posit/posit.hpp>
// Stillwater BLAS library
#include <blas/blas.hpp>

/*
In dynamical systems theory, the baker's map is a chaotic map from the unit square into itself.
It is named after a kneading operation that bakers apply to dough: the dough is cut in half,
and the two halves are stacked on one another, and compressed.

The baker's map can be understood as the bilateral shift operator of a bi-infinite two-state
lattice model. The baker's map is topologically conjugate to the horseshoe map.
In physics, a chain of coupled baker's maps can be used to model deterministic diffusion.

As with many deterministic dynamical systems, the baker's map is studied by its action
on the space of functions defined on the unit square. The baker's map defines an operator
on the space of functions, known as the transfer operator of the map. The baker's map is
an exactly solvable model of deterministic chaos, in that the eigenfunctions and eigenvalues
of the transfer operator can be explicitly determined.

Keywords: deterministic chaos, float precision
 */


/*
  Folded baker's map acts on the unit square as

  S_baker-folded(x, y) = { (2x, y/2)         for 0.0 <= x < 0.5
						 { (2 - 2x, 1 - y/2) for 0.5 <= x < 1.0
 */
template<typename Real>
std::pair<Real, Real> BakersMap(const std::pair<Real, Real>& xy) {
	std::pair<Real, Real> map;
	if (xy.first < Real(0.5)) {
		map.first = 2 * xy.first;  // <- this requires POSIT_ENABLE_LITERALS if you want to use posits for Real
		map.second = xy.second / 2;// <- this line too of course
	}
	else {
		map.first = 2 - 2 * xy.first;
		map.second = 1 - xy.second / 2;
	}
	return map;
}


template<typename Real>
std::ostream& operator<<(std::ostream& ostr, std::pair<Real, Real>& xy) {
	return ostr << '(' << sw::universal::color_print(xy.first) << ", " << sw::universal::color_print(xy.second) << ") : (" << xy.first << ", " << xy.second << ')';
}

// iterate a set of (x,y) values through the Baker's map
template<typename Real>
void TraceBakersMap(const Real& x, const Real& y, unsigned nrIterations) {
	std::cout << typeid(Real).name() << '\n';
	std::pair<Real, Real> xy(x, y);
//	xy.first = x;
//	xy.second = y;
	std::cout << std::setw(5) << 0 << " : " << xy << std::endl;
	for (unsigned i = 1; i < nrIterations; ++i) {
		xy = BakersMap(xy);
		std::cout << std::setw(5) << i << " : " << xy << std::endl;
	}
}

template<typename Matrix>
void InitializeTwoBands(Matrix& S) {
	using Real = typename Matrix::value_type;
	unsigned n = S.rows();
	unsigned m = S.cols();
	assert(n == m);
	for (unsigned i = 0; i < n; ++i) {
		for (unsigned j = 0; j < m / 2; ++j) {
			S(i, j) = Real(0.25);
		}
		for (unsigned j = m / 2; j < m; ++j) {
			S(i, j) = Real(0.75);
		}
	}
}

/*
  Folded baker's map acts on the unit square as

  S_baker-folded(x, y) = { (2x, y/2)         for 0.0 <= x < 0.5
                         { (2 - 2x, 1 - y/2) for 0.5 <= x < 1.0
 */
template<typename Matrix>
void KneadAndFold(const Matrix& S, Matrix& Snext) {
	using Real = typename Matrix::value_type;
	unsigned n = S.rows();
	unsigned m = S.cols();
	assert(n == m);
	auto oldprecision = std::cout.precision();
	std::cout << std::setprecision(15);
	for (unsigned i = 0; i < n; ++i) {
		Real x = Real(i) / n;
		for (unsigned j = 0; j < m; ++j) {
			Real y = Real(j) / m;
			Real xnext, ynext;
			if (x <= Real(0.5)) {
				xnext = 2 * x;
				ynext = y / 2;
				Snext(i, j) = S(unsigned(xnext * n), unsigned(ynext * m));
			}
			else {
				xnext = 2 - 2 * x;
				ynext = 1 - y / 2;
				Snext(i, j) = S(unsigned(xnext * n), unsigned(ynext * m));
			}
			// trace the accuracy of (x,y) of a point
			if (i == (n - 1) && j == (m - 1)) {
				std::cout << '(' << std::setw(15) << sw::universal::to_binary(xnext * n) << ',';
				std::cout << std::setw(15) << sw::universal::to_binary(ynext * m) << ")\n";
				std::cout << '(' << x << ',' << y << ") maps to ";
				std::cout << '(' << xnext << ',' << ynext << ") truncates to ";
				std::cout << '(' << unsigned(xnext * n) << ',' << unsigned(ynext * m) << ")\n";
			}
			//std::cout << Snext << std::endl;
		}
	}
	std::cout << std::setprecision(oldprecision);
}

template<typename Real>
void Knead(int nrOfFolds) {
	constexpr int N = 100;
	sw::numeric::containers::matrix<Real> S1(N, N), S2(N, N);
	InitializeTwoBands(S1);
	//cout << S1 << endl;

	for (unsigned nrOfFolds = 0; nrOfFolds < 10; ++nrOfFolds) {
		if (nrOfFolds % 2) {
			KneadAndFold(S2, S1); // fold back
		}
		else {
			KneadAndFold(S1, S2); // fold forward
		}
	}
}

int main()
try {
	using namespace sw::blas;

	std::cout << "Baker's Map\n";

	{
		using Real = float;
		Real x = 0.125 * 0.125 * 0.125 * 0.125 * 0.125 * 0.125;
		Real y = 0.75;
		TraceBakersMap(x, y, 25);
	}

	{
		using Real = sw::universal::posit<32,2>;
		Real x = 0.125 * 0.125 * 0.125 * 0.125 * 0.125 * 0.125;
		Real y = 0.75;
		TraceBakersMap(x, y, 25);
	}

	std::cout << "Baker's Map\n";
	double random_value = double(rand()) / double(RAND_MAX);
	{
		using Real = float;
		Real x = Real(random_value);
		Real y = 0.75;
		TraceBakersMap(x, y, 25);
	}
	{
		using Real = sw::universal::posit<32, 2>;
		Real x = random_value;
		Real y = 0.75;
		TraceBakersMap(x, y, 25);
	}

	// setting x to minpos fails as 2 * minpos still rounds to minpos

	std::cout << "Baker's Map: minpos fail:\n";
	{
		using Real = sw::universal::posit<32, 2>;
		std::cout << std::setprecision(17);
		Real x(sw::universal::SpecificValue::minpos);
		std::cout << "minpos<32,2> : " << x << '\n';
		Real y = 0.75;
		TraceBakersMap(x, y, 5);
	}
	std::cout << "Baker's Map: region k-1:\n";
	{
		using Real = sw::universal::posit<32, 2>;
		std::cout << std::setprecision(17);
		Real x(sw::universal::SpecificValue::minpos);
		x *= sw::universal::useed<32, 2>();
		std::cout << "minpos<32,2> * useed : " << x << '\n';
		Real y = 0.75;
		TraceBakersMap(x, y, 5);
	}
	std::cout << "Baker's Map: region k-2:\n";
	{
		using Real = sw::universal::posit<32, 2>;
		std::cout << std::setprecision(17);
		Real x(sw::universal::SpecificValue::minpos);
		x *= sw::universal::useed<32, 2>() * sw::universal::useed<32,2>();
		std::cout << "minpos<32,2> * useed^2 : " << x << '\n';
		Real y = 0.75;
		TraceBakersMap(x, y, 125);
	}

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
