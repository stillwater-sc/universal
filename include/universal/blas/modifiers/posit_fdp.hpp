#pragma once

namespace sw { namespace universal { namespace blas {

// overload for posits to use fused dot products
template<unsigned nbits, unsigned es>
vector< posit<nbits, es> > operator*(const matrix< posit<nbits, es> >& A, const vector< posit<nbits, es> >& x) {
	constexpr unsigned capacity = 20; // FDP for vectors < 1,048,576 elements
	vector< posit<nbits, es> > b(A.rows());
	for (unsigned i = 0; i < A.rows(); ++i) {
		quire<nbits, es, capacity> q;
		for (unsigned j = 0; j < A.cols(); ++j) {
			q += quire_mul(A(i, j), x[j]);
		}
		convert(q.to_value(), b[i]); // one and only rounding step of the fused-dot product
	}
	return b;
}

// overload for posits uses fused dot products
template<unsigned nbits, unsigned es>
matrix< posit<nbits, es> > operator*(const matrix< posit<nbits, es> >& A, const matrix< posit<nbits, es> >& B) {
	constexpr unsigned capacity = 20; // FDP for vectors < 1,048,576 elements
	if (A.cols() != B.rows()) throw matmul_incompatible_matrices(incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());
	unsigned rows = A.rows();
	unsigned cols = B.cols();
	unsigned dots = A.cols();
	matrix< posit<nbits, es> > C(rows, cols);
	for (unsigned i = 0; i < rows; ++i) {
		for (unsigned j = 0; j < cols; ++j) {
			quire<nbits, es, capacity> q;
			for (unsigned k = 0; k < dots; ++k) {
				q += quire_mul(A(i, k), B(k, j));
			}
			convert(q.to_value(), C(i, j)); // one and only rounding step of the fused-dot product
		}
	}
	return C;
}

}}} // namespace sw::universal::blas
