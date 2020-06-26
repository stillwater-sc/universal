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

template<typename Matrix>
void BakersMap(Matrix& S) {
	using Real = typename Matrix::value_type;
	unsigned n = S.rows();
	unsigned m = S.cols();
	assert(n == m);
	for (unsigned i = 0; i < n; ++i) {
		for (unsigned j = 0; j < n; ++j) {
			S(i, j) = Real(rand()) / Real(RAND_MAX);
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
	for (unsigned i = 0; i < n; ++i) {
		Real x = i / n;
		for (unsigned j = 0; j < n; ++j) {
			Real y = j / n;
			if (x < Real(0.5)) {
				Snext(i, j) = S(2 * i, j / 2);
			}
			else {
				Snext(i, j) = S(2 - 2 * i, 1 - j / 2);
			}
		}
	}
}

int main()
try {
	using namespace std;

	cout << "Baker's Map\n";

	matrix<float> square(5, 5);
	float v = square(1, 1);
	cout << v << endl;

	matrix<float> S(5, 5);
	BakersMap(S);
	cout << S << endl;

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
