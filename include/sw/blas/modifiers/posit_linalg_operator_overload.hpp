#pragma once
// posit_linalg_operator_overload.hpp: type specific fused dot product overloads for the BLAS vector and matrix classes
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <universal/number/posit/posit_fwd.hpp>
#include <numeric/containers.hpp>

namespace sw { namespace blas {
	using namespace sw::numeric::containers;

	// generate a posit format ASCII format nbits.esxNN...NNp
	template<unsigned nbits, unsigned es>
	inline std::string hex_format(const vector< sw::universal::posit<nbits, es> >& v) {
		// we need to transform the posit into a string
		std::stringstream ss;
		for (size_t j = 0; j < size(v); ++j) ss << hex_format(v[j]) << " ";
		return ss.str();
	}

	// generate a posit format ASCII format nbits.esxNN...NNp
	template<unsigned nbits, unsigned es>
	inline std::string hex_format(const matrix< sw::universal::posit<nbits, es> >& A) {
		using Scalar = sw::universal::posit<nbits, es>;
		using size_type = typename matrix<Scalar>::size_type;
		std::stringstream ostr;
		size_type m = A.rows();
		size_type n = A.cols();
		for (size_type i = 0; i < m; ++i) {
			for (size_type j = 0; j < n; ++j) {
				ostr << hex_format(A(i, j)) << " ";
			}
			ostr << '\n';
		}
		return ostr.str();
	}

	// fused dot product operator*() overload for posit vectors
	template<unsigned nbits, unsigned es>
	sw::universal::posit<nbits, es> operator*(const vector< sw::universal::posit<nbits, es> >& a, const vector< sw::universal::posit<nbits, es> >& b) {
		using Scalar = posit<nbits, es>;
		//	std::cout << "fused dot product for " << typeid(Scalar).name() << std::endl;
		size_t N = size(a);
		if (size(a) != size(b)) {  // LCOV_EXCL_START
			std::cerr << "vector sizes are different: " << N << " vs " << size(b) << '\n';
			return Scalar{ 0 };
		}  // LCOV_EXCL_STOP
		constexpr unsigned capacity = 20;
		sw::universal::quire<nbits, es, capacity> sum{ 0 };
		for (size_t i = 0; i < N; ++i) {
			sum += sw::universal::quire_mul(a(i), b(i));
		}
		Scalar p;
		convert(sum.to_value(), p);
		return p;
	}

	// overload for posits to use fused dot products
	template<unsigned nbits, unsigned es>
	vector< sw::universal::posit<nbits, es> > operator*(const matrix< sw::universal::posit<nbits, es> >& A, const vector< sw::universal::posit<nbits, es> >& x) {
		constexpr unsigned capacity = 20; // FDP for vectors < 1,048,576 elements
		vector< sw::universal::posit<nbits, es> > b(A.rows());
		for (unsigned i = 0; i < A.rows(); ++i) {
			sw::universal::quire<nbits, es, capacity> q;
			for (unsigned j = 0; j < A.cols(); ++j) {
				q += quire_mul(A(i, j), x[j]);
			}
			convert(q.to_value(), b[i]); // one and only rounding step of the fused-dot product
		}
		return b;
	}

	// overload for posits uses fused dot products
	template<unsigned nbits, unsigned es>
	matrix< sw::universal::posit<nbits, es> > operator*(const matrix< sw::universal::posit<nbits, es> >& A, const matrix< sw::universal::posit<nbits, es> >& B) {
		using size_type = typename matrix< sw::universal::posit<nbits, es> >::size_type;
		constexpr unsigned capacity = 20; // FDP for vectors < 1,048,576 elements
		if (A.cols() != B.rows()) throw matmul_incompatible_matrices(incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());  // LCOV_EXCL_LINE
		auto rows = A.rows();
		auto cols = B.cols();
		auto dots = A.cols();
		matrix< posit<nbits, es> > C(rows, cols);
		for (size_type i = 0; i < rows; ++i) {
			for (size_type j = 0; j < cols; ++j) {
				sw::universal::quire<nbits, es, capacity> q;
				for (size_type k = 0; k < dots; ++k) {
					q += quire_mul(A(i, k), B(k, j));
				}
				convert(q.to_value(), C(i, j)); // one and only rounding step of the fused-dot product
			}
		}
		std::cout << "fused dot product matrix-matrix multiplication\n";
		return C;
	}

}} // namespace sw::blas
