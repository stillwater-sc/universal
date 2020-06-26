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
class matrix : public std::vector<Real> {
public:
	matrix() {}
	matrix(unsigned _n, unsigned _m) : n{ _n }, m{ _m }, std::vector<Real>(n*m, Real(0.0)) { }

	Real operator()(int i, int j) { return this->operator[](i*m + j); }
	unsigned rows() { return n; }
	unsigned cols() { return m; }

private:
	unsigned n, m;
};

template<typename Matrix>
void BakersMap(const Matrix& previous, Matrix& next) {
	for (unsigned i = 0; i < previous.rows(); ++i) {
		for (unsigned j = 0; m < previous.cols(); ++j) {

		}
	}
}
int main()
try {
	using namespace std;

	cout << "Baker's Map\n";

	matrix<float> square(5, 5);
	cout << "line 58\n";
	float v = square(1, 1);
	cout << v << endl;
	cout << "line 61\n";

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
