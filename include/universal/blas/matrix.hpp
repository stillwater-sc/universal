#pragma once
// matrix.hpp: super-simple dense matrix class implementation
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <vector>
#include <initializer_list>
#include <map>
#include <universal/blas/exceptions.hpp>
#include <universal/number/posit/posit_fwd.hpp>
#include <universal/number/cfloat/cfloat_fwd.hpp>

namespace sw { namespace universal { namespace blas { 

template<typename Scalar> class matrix;
template<typename Scalar>
class ConstRowProxy {
public:
	ConstRowProxy(typename std::vector<Scalar>::const_iterator iter) : _iter(iter) {}
	Scalar operator[](size_t col) const { return *(_iter + int64_t(col)); }

private:
	typename std::vector<Scalar>::const_iterator _iter;
};
template<typename Scalar>
class RowProxy {
public:
	RowProxy(typename std::vector<Scalar>::iterator iter) : _iter(iter) {}
	Scalar& operator[](size_t col) { return *(_iter + int64_t(col)); }

private:
	typename std::vector<Scalar>::iterator _iter;
};

template<typename Scalar>
class matrix {
public:
	typedef Scalar									value_type;
	typedef const value_type&						const_reference;
	typedef value_type&								reference;
	typedef const value_type*						const_pointer_type;
	typedef typename std::vector<Scalar>::size_type size_type;

	matrix() : _m{ 0 }, _n{ 0 }, data(0) {}
	matrix(size_t m, size_t n) : _m{ m }, _n{ n }, data(m*n, Scalar(0.0)) { }
	matrix(std::initializer_list< std::initializer_list<Scalar> > values) {
		size_t nrows = values.size();
		size_t ncols = values.begin()->size();
		data.resize(nrows * ncols);
		size_t r = 0;
		for (auto l : values) {
			if (l.size() == ncols) {
				size_t c = 0;
				for (auto v : l) {
					data[r*ncols + c] = v;
					++c;
				}
				++r;
			}
		}
		_m = nrows;
		_n = ncols;
	}
	matrix(const matrix& A) : _m{ A._m }, _n{ A._n }, data(A.data) {}

	// Converting Constructor (SourceType A --> Scalar B)
	template<typename SourceType>
	matrix(const matrix<SourceType>& A) : _m{ A.rows() }, _n{A.cols() }{
		data.resize(_m*_n);
		for (size_t i = 0; i < _m; ++i){
			for (size_t j = 0; j < _n; ++j){
				data[i*_n + j] = Scalar(A(i,j));
			}
		}
	}


	/* Operators
	/ 	Binary and unitary operators, =,>,!=,...
	*/
	matrix& operator=(const matrix& M) = default;
	matrix& operator=(matrix&& M) = default;

	// Identity matrix operator
	matrix& operator=(const Scalar& one) {
		setzero();
		size_t smallestDimension = (_m < _n ? _m : _n);
		for (size_t i = 0; i < smallestDimension; ++i) data[i*_n + i] = one;
		return *this;
	}

	Scalar operator()(size_t i, size_t j) const { return data[i*_n + j]; }
	Scalar& operator()(size_t i, size_t j) { return data[i*_n + j]; }
	RowProxy<Scalar> operator[](size_t i) {
		typename std::vector<Scalar>::iterator it = data.begin() + int64_t(i) * int64_t(_n);
		RowProxy<Scalar> proxy(it);
		return proxy;
	}
	ConstRowProxy<Scalar> operator[](size_t i) const {
		typename std::vector<Scalar>::const_iterator it = data.begin() + static_cast<int64_t>(i * _n);
		ConstRowProxy<Scalar> proxy(it);
		return proxy;
	}

	// matrix element-wise sum
	matrix& operator+=(const matrix& rhs) {
		// check if the matrices are compatible
		if (_m != rhs._m || _n != rhs._n) {
			std::cerr << "Element-wise matrix sum received incompatible matrices ("
				<< _m << ", " << _n << ") += (" << rhs._m << ", " << rhs._n << ")\n";
			return *this; // return without changing
		}
		for (size_type e = 0; e < _m * _n; ++e) {
			data[e] += rhs.data[e];
		}
		return *this;
	}
	// matrix element-wise difference
	matrix& operator-=(const matrix& rhs) {
		// check if the matrices are compatible
		if (_m != rhs._m || _n != rhs._n) {
			std::cerr << "Element-wise matrix difference received incompatible matrices ("
				<< _m << ", " << _n << ") -= (" << rhs._m << ", " << rhs._n << ")\n";
			return *this; // return without changing
		}
		for (size_type e = 0; e < _m*_n; ++e) {
			data[e] -= rhs.data[e];
		}
		return *this;
	}

	// multiply all matrix elements
	matrix& operator*=(const Scalar& a) {
		using size_type = typename matrix<Scalar>::size_type;
		for (size_type e = 0; e < _m*_n; ++e) {
			data[e] *= a;
		}
		return *this;
	}
	// divide all matrix elements
	matrix& operator/=(const Scalar& a) {
		using size_type = typename matrix<Scalar>::size_type;
		for (size_type e = 0; e < _m * _n; ++e) {
			data[e] /= a;
		}
		return *this;
	}

	


	// modifiers
	inline void setzero() { for (auto& elem : data) elem = Scalar(0); }
	inline void resize(size_t m, size_t n) { _m = m; _n = n; data.resize(m * n); }
	// selectors
	inline size_t rows() const { return _m; }
	inline size_t cols() const { return _n; }
	inline std::pair<size_t, size_t> size() const { return std::make_pair(_m, _n); }

	// in-place transpose
	matrix& transpose() {
		size_t size = _m * _n - 1;
		std::map<size_t, bool> b; // mark visits
		b[0] = true; // A(0,0) is stationary
		b[size] = true; // A(m-1,n-1) is stationary
		size_t index = 1;
		while (index < size) {
			size_t cycleStart = index; // holds start of cycle
			Scalar e = data[index]; // holds value of the element to be swapped
			do {
				size_t next = (index * _m) % size; // index of e
				std::swap(data[next], e);
				b[index] = true;
				index = next;
			} while (index != cycleStart);
			// get the starting point of the next cycle
			for (index = 1; index < size && b[index]; ++index) {}
		}
		std::swap(_m, _n);
		return *this;
	}

	// Eigen operators I need to reverse engineer
	matrix Zero(size_t m, size_t n) {
		matrix z(m, n);
		return z;
	}

private:
	size_t _m, _n; // m rows and n columns
	std::vector<Scalar> data;

};

template<typename Scalar>
inline size_t num_rows(const matrix<Scalar>& A) { return A.rows(); }
template<typename Scalar>
inline size_t num_cols(const matrix<Scalar>& A) { return A.cols(); }
template<typename Scalar>
inline std::pair<size_t, size_t> size(const matrix<Scalar>& A) { return A.size(); }

// ostream operator: no need to declare as friend as it only uses public interfaces
template<typename Scalar>
std::ostream& operator<<(std::ostream& ostr, const matrix<Scalar>& A) {
	auto width = ostr.width();
	size_t m = A.rows();
	size_t n = A.cols();
	for (size_t i = 0; i < m; ++i) {
		for (size_t j = 0; j < n; ++j) {
			ostr << std::setw(width) << A(i, j) << " ";
		}
		ostr << '\n';
	}
	return ostr;
}

// generate a posit format ASCII format nbits.esxNN...NNp
template<size_t nbits, size_t es>
inline std::string hex_format(const matrix< sw::universal::posit<nbits, es> >& A) {
	// we need to transform the posit into a string
	std::stringstream ostr;
	size_t m = A.rows();
	size_t n = A.cols();
	for (size_t i = 0; i < m; ++i) {
		for (size_t j = 0; j < n; ++j) {
			ostr << hex_format(A(i,j)) << " ";
		}
		ostr << '\n';
	}
	return ostr.str();
}

template<typename Scalar>
std::ostream& operator<<(std::ostream& ostr, const std::pair<Scalar, Scalar>& p) {
	return ostr << '(' << p.first << " by " << p.second << ')';
}



// matrix element-wise sum
template<typename Scalar>
matrix<Scalar> operator+(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	matrix<Scalar> Sum(A);
	return Sum += B;
}

// matrix element-wise difference
template<typename Scalar>
matrix<Scalar> operator-(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	matrix<Scalar> Diff(A);
	return Diff -= B;
}

// matrix scaling through Scalar multiply
template<typename Scalar>
matrix<Scalar> operator*(const Scalar& a, const matrix<Scalar>& B) {
	matrix<Scalar> A(B);
	return A *= a;
}

// matrix scaling through Scalar divide
template<typename Scalar>
matrix<Scalar> operator/(const matrix<Scalar>& A, const Scalar& b) {
	matrix<Scalar> B(A);
	return B /= b;
}

// matrix-vector multiply
template<typename Scalar>
vector<Scalar> operator*(const matrix<Scalar>& A, const vector<Scalar>& x) {
	vector<Scalar> b(A.rows());
	for (size_t i = 0; i < A.rows(); ++i) {
		b[i] = Scalar(0);
		for (size_t j = 0; j < A.cols(); ++j) {
			b[i] += A(i, j) * x[j];
		}
	}
	return b;
}

// overload for posits to use fused dot products
template<size_t nbits, size_t es>
vector< posit<nbits, es> > operator*(const matrix< posit<nbits, es> >& A, const vector< posit<nbits, es> >& x) {
	constexpr size_t capacity = 20; // FDP for vectors < 1,048,576 elements
	vector< posit<nbits, es> > b(A.rows());
	for (size_t i = 0; i < A.rows(); ++i) {
		quire<nbits, es, capacity> q;
		for (size_t j = 0; j < A.cols(); ++j) {
			q += quire_mul(A(i, j), x[j]);
		}
		convert(q.to_value(), b[i]); // one and only rounding step of the fused-dot product
	}
	return b;
}

template<typename Scalar>
matrix<Scalar> operator*(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	if (A.cols() != B.rows()) throw matmul_incompatible_matrices(incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());
	size_t rows = A.rows();
	size_t cols = B.cols();
	size_t dots = A.cols();
	matrix<Scalar> C(rows, cols);
	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < cols; ++j) {
			Scalar e = Scalar(0);
			for (size_t k = 0; k < dots; ++k) {
				e += A(i, k) * B(k, j);
			}
			C(i, j) = e;
		}
	}
	return C;
}




template<typename Scalar>
matrix<Scalar> operator%(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	// Hadamard Product A.*B.  Element-wise multiplication.
	if (A.size() != B.size()) throw matmul_incompatible_matrices(incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());
	size_t rows = A.rows();
	size_t cols = A.cols();
	 
	matrix<Scalar> C(rows, cols);
	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < cols; ++j) {
			C(i, j) = A(i, j) * B(i, j);
		}
	}
	return C;
}



// overload for posits uses fused dot products
template<size_t nbits, size_t es>
matrix< posit<nbits, es> > operator*(const matrix< posit<nbits, es> >& A, const matrix< posit<nbits, es> >& B) {
	constexpr size_t capacity = 20; // FDP for vectors < 1,048,576 elements
	if (A.cols() != B.rows()) throw matmul_incompatible_matrices(incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());
	size_t rows = A.rows();
	size_t cols = B.cols();
	size_t dots = A.cols();
	matrix< posit<nbits, es> > C(rows, cols);
	for (size_t i = 0; i < rows; ++i) {
		for (size_t j = 0; j < cols; ++j) {
			quire<nbits, es, capacity> q;
			for (size_t k = 0; k < dots; ++k) {
				q += quire_mul(A(i, k), B(k, j));
			}
			convert(q.to_value(), C(i, j)); // one and only rounding step of the fused-dot product
		}
	}
	return C;
}

// matrix equivalence tests
template<typename Scalar>
bool operator==(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	if (num_rows(A) != num_rows(B) ||
		num_cols(A) != num_cols(B)) return false;
	bool equal = true;
	for (size_t i = 0; i < num_rows(A); ++i) {
		for (size_t j = 0; j < num_cols(A); ++j) {
			// a tolerance is needed (equal up to eps)
			if (A(i, j) != B(i, j)) {
				equal = false;
				break;
			}
		}
		if (!equal) break;
	}
	return equal;
}



template<typename Scalar>
bool operator!=(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	return !(A == B);
}



// Matrix > x ==> Matrix with 1/0 representing True/False
template<typename Scalar>
matrix<Scalar> operator>(const matrix<Scalar>& A, const Scalar& x) {
	matrix<Scalar> B(A.cols(), A.rows());
	
	for (size_t i = 0; i < num_rows(A); ++i) {
		for (size_t j = 0; j < num_cols(A); ++j) {
			B(i,j) = (A(i, j) > x) ? 1 : 0;
		}
	}
	return B;
}
 

// maxelement (jq 2022-10-15)
template<typename Scalar>
Scalar maxelement(const matrix<Scalar>&A) {
	auto x = abs(A(0,0));
	for (size_t i = 0; i < num_rows(A); ++i) {
		for (size_t j = 0; j < num_cols(A); ++j) {
			x = (abs(A(i, j)) > x) ? abs(A(i, j)) : x;
		}
	}
	return x;
}

// minelement (jq 2022-10-15)
template<typename Scalar>
Scalar minelement(const matrix<Scalar>&A) {
	auto x = abs(A(0,0));
	for (size_t i = 0; i < num_rows(A); ++i) {
		for (size_t j = 0; j < num_cols(A); ++j) {
			x = (abs(A(i, j)) < x && A(i,j)!=0) ? abs(A(i, j)) : x;
		}
	}
	return x;
}



// getRow (jq 2022-11-19)
template<typename Scalar>
vector<Scalar> getRow(unsigned i, const matrix<Scalar>&A) {
	vector<Scalar> x(num_cols(A),0);
	for (size_t j = 0; j < num_cols(A); ++j) {
		x(j) = A(i,j);
		}
	return x;
}


// permat (jq 2022-11-19)
//template<typename Scalar>
//void permat(vector<Scalar>&P, matrix<Scalar>&A) {
//	vector<Scalar> x(num_cols(A),0);
//	for (size_t row = 0; row < num_rows(A); ++row) {
//		x = getRow(row,A);
//		something here
//		}
//}


}}} // namespace sw::universal::blas