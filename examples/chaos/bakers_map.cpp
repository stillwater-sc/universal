//  bakers_map.cpp : The baker's map is a chaotic map from the unit squre into itself.
//
// Copyright (C) 2017-2020 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <universal/posit/posit>

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

template<typename Real>
class matrix {
public:
	typedef Real                              value_type;
	typedef const value_type&                 const_reference;
	typedef value_type&                       reference;
	typedef const value_type*                 const_pointer_type;

	matrix() {}
	matrix(unsigned _n, unsigned _m) : n{ _n }, m{ _m }, data(n*m, Real(0.0)) { }

	Real operator()(int i, int j) const { return data[i*m + j]; }
	Real& operator()(int i, int j) { return data[i*m + j]; }

	unsigned rows() const { return n; }
	unsigned cols() const { return m; }

private:
	unsigned n, m;
	std::vector<Real> data;
};

template<typename Real>
std::ostream& operator<<(std::ostream& ostr, const matrix<Real>& A) {
	unsigned n = A.rows();
	unsigned m = A.cols();
	for (unsigned i = 0; i < n; ++i) {
		for (unsigned j = 0; j < n; ++j) {
			ostr << A(i, j) << " ";
		}
		ostr << '\n';
	}
	return ostr;
}

/*
  Folded baker's map acts on the unit square as

  S_baker-folded(x, y) = { (2x, y/2)         for 0.0 <= x < 0.5
						 { (2 - 2x, 1 - y/2) for 0.5 <= x < 1.0
 */
template<typename Real>
std::pair<Real, Real> BakersMap(const std::pair<Real, Real>& xy) {
	std::pair<Real, Real> map;
	if (xy.first < Real(0.5)) {
		map.first = 2 * xy.first;
		map.second = xy.second / 2;
	}
	else {
		map.first = 2 - 2 * xy.first;
		map.second = 1 - xy.second / 2;
	}
	return map;
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
				std::cout << '(' << std::setw(15) << sw::unum::to_binary(xnext * n) << ',';
				std::cout << std::setw(15) << sw::unum::to_binary(ynext * m) << ")\n";
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
	matrix<Real> S1(N, N), S2(N, N);
	InitializeTwoBands(S1);
	//cout << S1 << endl;

	for (unsigned nrOfFolds = 0; nrOfFolds < 10; ++nrOfFolds) {
		if (nrOfFolds % 2) {
			KneadAndFold(S2, S1); // fold back
			//cout << S1 << endl;
		}
		else {
			KneadAndFold(S1, S2); // fold forward
			//cout << S2 << endl;
		}
	}
}

template<typename Real>
std::ostream& operator<<(std::ostream& ostr, std::pair<Real, Real>& xy) {
	return ostr << '(' << sw::unum::color_print(xy.first) << ", " << sw::unum::color_print(xy.second) << ") : (" << xy.first << ", " << xy.second << ')';
}

template<typename Real>
void TraceBakersMap(const Real& x, const Real& y, unsigned nrIterations) {
	std::cout << typeid(Real).name() << '\n';
	std::pair<Real, Real> xy;
	xy.first = x;
	xy.second = y;
	for (unsigned i = 0; i < nrIterations; ++i) {
		xy = BakersMap(xy);
		std::cout << std::setw(5) << i << " : " << xy << std::endl;
	}
}


int main()
try {
	using namespace std;

	cout << "Baker's Map\n";

	{
		using Real = float;
		Real x = 0.125 * 0.125 * 0.125 * 0.125 * 0.125 * 0.125;
		Real y = 0.75;
		TraceBakersMap(x, y, 25);
	}

	{
		using Real = sw::unum::posit<32,2>;
		Real x = 0.125 * 0.125 * 0.125 * 0.125 * 0.125 * 0.125;
		Real y = 0.75;
		TraceBakersMap(x, y, 25);
	}

	cout << "Baker's Map\n";
	double random_value = double(rand()) / double(RAND_MAX);
	{
		using Real = float;
		Real x = Real(random_value);
		Real y = 0.75;
		TraceBakersMap(x, y, 25);
	}
	{
		using Real = sw::unum::posit<32, 2>;
		Real x = random_value;
		Real y = 0.75;
		TraceBakersMap(x, y, 25);
	}


	return EXIT_SUCCESS;
}
catch (char const* msg) {
	std::cerr << msg << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_arithmetic_exception& err) {
	std::cerr << "Uncaught posit arithmetic exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const quire_exception& err) {
	std::cerr << "Uncaught quire exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const posit_internal_exception& err) {
	std::cerr << "Uncaught posit internal exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (const std::runtime_error& err) {
	std::cerr << "Uncaught runtime exception: " << err.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
