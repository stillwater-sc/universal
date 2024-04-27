#pragma once
// matrix.hpp: super-simple dense matrix class implementation
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <vector>
#include <initializer_list>
#include <map>
#include <universal/blas/exceptions.hpp>

#if defined(__clang__)
/* Clang/LLVM. ---------------------------------------------- */
#define _HAS_NODISCARD 1

#elif defined(__ICC) || defined(__INTEL_COMPILER)
/* Intel ICC/ICPC. ------------------------------------------ */
#define _HAS_NODISCARD 1

#elif defined(__GNUC__) || defined(__GNUG__)
/* GNU GCC/G++. --------------------------------------------- */
#define _HAS_NODISCARD 1

#elif defined(__HP_cc) || defined(__HP_aCC)
/* Hewlett-Packard C/aC++. ---------------------------------- */
#define _HAS_NODISCARD 1

#elif defined(__IBMC__) || defined(__IBMCPP__)
/* IBM XL C/C++. -------------------------------------------- */
#define _HAS_NODISCARD 1

#elif defined(_MSC_VER)
/* Microsoft Visual Studio. --------------------------------- */
// already defines _NODISCARD

#elif defined(__PGI)
/* Portland Group PGCC/PGCPP. ------------------------------- */
#define _HAS_NODISCARD 1

#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
/* Oracle Solaris Studio. ----------------------------------- */
#define _HAS_NODISCARD 1

#endif

#if _HAS_NODISCARD
#define _NODISCARD [[nodiscard]]
#else // ^^^ CAN HAZ [[nodiscard]] / NO CAN HAZ [[nodiscard]] vvv
#define _NODISCARD
#endif // _HAS_NODISCARD

namespace sw { namespace universal { namespace blas { 

template<typename Scalar> class matrix;
template<typename Scalar>
class ConstRowProxy {
public:
	typedef typename std::vector<Scalar>::size_type              size_type;
	ConstRowProxy(typename std::vector<Scalar>::const_iterator iter) : _iter(iter) {}
	Scalar operator[](size_type col) const { return *(_iter + static_cast<int64_t>(col)); }

private:
	typename std::vector<Scalar>::const_iterator _iter;
};
template<typename Scalar>
class RowProxy {
public:
	typedef typename std::vector<Scalar>::size_type              size_type;
	RowProxy(typename std::vector<Scalar>::iterator iter) : _iter(iter) {}
	Scalar& operator[](size_type col) { return *(_iter + static_cast<int64_t>(col)); }

private:
	typename std::vector<Scalar>::iterator _iter;
};

template<typename Scalar>
class matrix {
public:
	typedef Scalar									             value_type;
	typedef const value_type&						             const_reference;
	typedef value_type&								             reference;
	typedef const value_type*						             const_pointer_type;
	typedef typename std::vector<Scalar>::size_type              size_type;
	typedef typename std::vector<Scalar>::iterator               iterator;
	typedef typename std::vector<Scalar>::const_iterator         const_iterator;
	typedef typename std::vector<Scalar>::reverse_iterator       reverse_iterator;
	typedef typename std::vector<Scalar>::const_reverse_iterator const_reverse_iterator;
	static constexpr unsigned AggregationType = UNIVERSAL_AGGREGATE_MATRIX;

	matrix() : _m{ 0 }, _n{ 0 }, data(0) {}
	matrix(size_type m, size_type n) : _m{ m }, _n{ n }, data(m*n, Scalar(0.0)) { }
	matrix(std::initializer_list< std::initializer_list<Scalar> > values) {
		auto nrows = values.size();
		auto ncols = values.begin()->size();
		data.resize(nrows * ncols);
		unsigned r = 0;
		for (auto l : values) {
			if (l.size() == ncols) {
				unsigned c = 0;
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
	matrix(const matrix<SourceType>& A) : _m{ A.rows() }, _n{A.cols() } {
		data.resize(_m*_n);
		for (size_type i = 0; i < _m; ++i) {
			for (size_type j = 0; j < _n; ++j) {
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
		size_type smallestDimension = (_m < _n ? _m : _n);
		for (size_type i = 0; i < smallestDimension; ++i) data[i*_n + i] = one;
		return *this;
	}

	Scalar operator()(size_type i, size_type j) const { return data[i*_n + j]; }
	Scalar& operator()(size_type i, size_type j) { return data[i*_n + j]; }
	RowProxy<Scalar> operator[](size_type i) {
		typename std::vector<Scalar>::iterator it = data.begin() + static_cast<int64_t>(i * _n);
		RowProxy<Scalar> proxy(it);
		return proxy;
	}
	ConstRowProxy<Scalar> operator[](size_type i) const {
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
		for (size_type e = 0; e < _m*_n; ++e) {
			data[e] *= a;
		}
		return *this;
	}
	// divide all matrix elements
	matrix& operator/=(const Scalar& a) {
		for (size_type e = 0; e < _m * _n; ++e) {
			data[e] /= a;
		}
		return *this;
	}

	// modifiers
	void push_back(const Scalar& v) { data.push_back(v); }
	void setzero() { for (auto& elem : data) elem = Scalar(0); }
	void resize(size_type m, size_type n) { _m = m; _n = n; data.resize(m * n); }
	// selectors
	size_type rows() const { return _m; }
	size_type cols() const { return _n; }
//	std::pair<unsigned, unsigned> size() const { return std::make_pair(_m, _n); }
	unsigned size() const { return static_cast<unsigned>(data.size()); }

	// in-place transpose
	matrix& transpose() {
		size_type size = _m * _n - 1;
		std::map<size_type, bool> b; // mark visits
		b[0] = true; // A(0,0) is stationary
		b[size] = true; // A(m-1,n-1) is stationary
		size_type index = 1;
		while (index < size) {
			size_type cycleStart = index; // holds start of cycle
			Scalar e = data[index]; // holds value of the element to be swapped
			do {
				size_type next = (index * _m) % size; // index of e
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
	matrix Zero(size_type m, size_type n) {
		matrix z(m, n);
		return z;
	}

	// iterators
	_NODISCARD iterator begin() noexcept {
		return data.begin();
	}

	_NODISCARD const_iterator begin() const noexcept {
		return data.begin();
	}

	_NODISCARD iterator end() noexcept {
		return data.end();
	}

	_NODISCARD const_iterator end() const noexcept {
		return data.end();
	}

	_NODISCARD reverse_iterator rbegin() noexcept {
		return reverse_iterator(end());
	}

	_NODISCARD const_reverse_iterator rbegin() const noexcept {
		return const_reverse_iterator(end());
	}

	_NODISCARD reverse_iterator rend() noexcept {
		return reverse_iterator(begin());
	}

	_NODISCARD const_reverse_iterator rend() const noexcept {
		return const_reverse_iterator(begin());
	}

private:
	size_type _m, _n; // m rows and n columns
	std::vector<Scalar> data;

};

template<typename Scalar>
inline typename matrix<Scalar>::size_type num_rows(const matrix<Scalar>& A) { return A.rows(); }
template<typename Scalar>
inline typename matrix<Scalar>::size_type num_cols(const matrix<Scalar>& A) { return A.cols(); }
template<typename Scalar>
inline std::pair<typename matrix<Scalar>::size_type, typename matrix<Scalar>::size_type> size(const matrix<Scalar>& A) { return std::make_pair(A.rows(), A.cols()); }

// ostream operator: no need to declare as friend as it only uses public interfaces
template<typename Scalar>
std::ostream& operator<<(std::ostream& ostr, const matrix<Scalar>& A) {
	using size_type = typename matrix<Scalar>::size_type;
	auto width = ostr.width();
	size_type m = A.rows();
	size_type n = A.cols();
	ostr << m << ' ' << n << '\n';
	for (size_type i = 0; i < m; ++i) {
		for (size_type j = 0; j < n; ++j) {
			if (j > 0) ostr << ' ';
			ostr << std::setw(width) << A(i, j);
		}
		ostr << '\n';
	}
	return ostr;
}

template<typename Scalar>
std::istream& operator>>(std::istream& istr, matrix<Scalar>& A) {
	constexpr bool trace = false;
	using size_type = typename matrix<Scalar>::size_type;
	size_type m, n;
	istr >> m >> n;
	if constexpr (trace) std::cout << m << ' ' << n << '\n';
	A.resize(m, n);
	for (size_type i = 0; i < m; ++i) {
		double item;
		for (size_type j = 0; j < n; ++j) {
			istr >> item;
			if constexpr (trace) std::cout << ' ' << item;
			A(i, j) = Scalar(item);
		}
		if constexpr (trace) std::cout << '\n';
	}
	return istr;
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
	using size_type = typename matrix<Scalar>::size_type;
	vector<Scalar> b(A.rows());
	for (size_type i = 0; i < A.rows(); ++i) {
		b[i] = Scalar(0);
		for (size_type j = 0; j < A.cols(); ++j) {
			b[i] += A(i, j) * x[j];
		}
	}
	return b;
}


template<typename Scalar>
matrix<Scalar> operator*(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	using size_type = typename matrix<Scalar>::size_type;
	if (A.cols() != B.rows()) throw matmul_incompatible_matrices(incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "*").what());
	size_type rows = A.rows();
	size_type cols = B.cols();
	size_type dots = A.cols();
	matrix<Scalar> C(rows, cols);
	for (size_type i = 0; i < rows; ++i) {
		for (size_type j = 0; j < cols; ++j) {
			Scalar e = Scalar(0);
			for (size_type k = 0; k < dots; ++k) {
				e += A(i, k) * B(k, j);
			}
			C(i, j) = e;
		}
	}
	return C;
}




template<typename Scalar>
matrix<Scalar> operator%(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	using size_type = typename matrix<Scalar>::size_type;
	// Hadamard Product A.*B.  Element-wise multiplication.
	if (A.size() != B.size()) throw matmul_incompatible_matrices(incompatible_matrices(A.rows(), A.cols(), B.rows(), B.cols(), "%").what());
	size_type rows = A.rows();
	size_type cols = A.cols();
	matrix<Scalar> C(rows, cols);
	for (size_type i = 0; i < rows; ++i) {
		for (size_type j = 0; j < cols; ++j) {
			C(i, j) = A(i, j) * B(i, j);
		}
	}
	return C;
}

// matrix equivalence tests
template<typename Scalar>
bool operator==(const matrix<Scalar>& A, const matrix<Scalar>& B) {
	using size_type = typename matrix<Scalar>::size_type;
	if (num_rows(A) != num_rows(B) ||
		num_cols(A) != num_cols(B)) return false;
	bool equal = true;
	for (size_type i = 0; i < num_rows(A); ++i) {
		for (size_type j = 0; j < num_cols(A); ++j) {
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
	using size_type = typename matrix<Scalar>::size_type;
	matrix<Scalar> B(A.cols(), A.rows());
	for (size_type i = 0; i < num_rows(A); ++i) {
		for (size_type j = 0; j < num_cols(A); ++j) {
			B(i,j) = (A(i, j) > x) ? 1 : 0;
		}
	}
	return B;
}
 

// maxelement (jq 2022-10-15)
template<typename Scalar>
Scalar maxelement(const matrix<Scalar>&A) {
	using size_type = typename matrix<Scalar>::size_type;
	auto x = abs(A(0,0));
	for (size_type i = 0; i < num_rows(A); ++i) {
		for (size_type j = 0; j < num_cols(A); ++j) {
			x = (abs(A(i, j)) > x) ? abs(A(i, j)) : x;
		}
	}
	return x;
}

// minelement (jq 2022-10-15)
template<typename Scalar>
Scalar minelement(const matrix<Scalar>&A) {
	using size_type = typename matrix<Scalar>::size_type;
	auto x = maxelement(A);
	for (size_type i = 0; i < num_rows(A); ++i) {
		for (size_type j = 0; j < num_cols(A); ++j) {
			x = ((abs(A(i, j)) < x) && (A(i,j)!=0)) ? abs(A(i, j)) : x;
		}
	}
	return x;
}


// Gets the ith row of matrix A
template<typename Scalar>
vector<Scalar> getRow(unsigned i, const matrix<Scalar>&A) {
	using size_type = typename matrix<Scalar>::size_type;
	vector<Scalar> x(num_cols(A));
	for (size_type j = 0; j < num_cols(A); ++j) {
		x(j) = A(i,j);
		}
	return x;
}

// Gets the jth column of matrix A
template<typename Scalar>
vector<Scalar> getCol(unsigned j, const matrix<Scalar>&A) {
	using size_type = typename matrix<Scalar>::size_type;
	vector<Scalar> x(num_rows(A));
	for (size_type i = 0; i < num_rows(A); ++i) {
		x(i) = A(i,j);
		}
	return x;
}


// Display Matrix
template<typename Scalar>
void disp(const matrix<Scalar>& A, const size_t COLWIDTH = 10) {
	using size_type = typename matrix<Scalar>::size_type;
    for (size_type i = 0; i < num_rows(A); ++i){
        for (size_type j = 0; j < num_cols(A); ++j){
            // std::cout <<std::setw(COLWIDTH) << A(i,j) << std::setw(COLWIDTH) << "\t" << std::fixed;
			std::cout << "\t" << A(i,j) << "\t"; // << std::fixed;
        }
        std::cout << "\n";
    }
    std::cout << "\n" << std::endl;
}


}}} // namespace sw::universal::blas