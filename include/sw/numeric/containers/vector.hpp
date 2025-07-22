#pragma once
// vector.hpp: Universal vector class that composes std::vector<>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <initializer_list>
#include <cmath>

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

// a column vector
template<typename Scalar>
class vector {
public:
	typedef Scalar                            value_type;
	typedef const value_type&                 const_reference;
	typedef value_type&                       reference;
	typedef const value_type*                 const_pointer_type;
	typedef typename std::vector<Scalar>::size_type size_type;
	typedef typename std::vector<Scalar>::iterator     iterator;
	typedef typename std::vector<Scalar>::const_iterator const_iterator;
	typedef typename std::vector<Scalar>::reverse_iterator reverse_iterator;
	typedef typename std::vector<Scalar>::const_reverse_iterator const_reverse_iterator;
	static constexpr unsigned AggregationType = UNIVERSAL_AGGREGATE_VECTOR;

	vector() : data(0) {}
	vector(size_t N) : data(N) {}
	vector(size_t N, const Scalar& val) : data(N, val) {}
	vector(std::initializer_list<Scalar> iList) : data(iList) {}
	// Converting Constructor (SourceType A --> Scalar B)
	template<typename SourceType>
	vector(const vector<SourceType>& v) : data(v.size()) {
		for (size_t i = 0; i < size(); ++i){
			data[i] = Scalar(v(i));
		}
	}
	vector(const vector& v) = default;
	vector(vector&& v) = default;

	vector& operator=(const vector& v) = default;
	vector& operator=(vector&& v) = default;
	template<typename tgtScalar>
	vector& operator=(const vector<tgtScalar>& v) {
		for (size_t i = 0; i < size(); ++i) {
			data[i] = Scalar(v[i]); // conversion must be handled by number system
		}
		return *this;
	}

// operators
	vector& operator=(const Scalar& val) {
		for (auto& v : data) v = val;
		return *this;
	}
	value_type operator[](size_t index) const { return data[index]; }
	value_type& operator[](size_t index) { return data[index]; }
	value_type operator()(size_t index) const { return data[index]; }
	value_type& operator()(size_t index) { return data[index]; }

	// prefix operator
	vector operator-() {
		vector<value_type> n(*this);
		for (auto& v : n.data) v = -v;
		return n;
	}

	/// vector-wide operators
	// vector-wide add
	vector& operator+=(const Scalar& offset) {
		for (auto& e : data) e += offset;
		return *this;
	}
	// vector-wide subtract
	vector& operator-=(const Scalar& offset) {
		for (auto& e : data) e -= offset;
		return *this;
	}
	// vector-wide multiply
	vector& operator*=(const Scalar& scaler) {
		for (auto& e : data) e *= scaler;
		return *this;
	}
	// vector-wide divide
	vector& operator/=(const Scalar& normalizer) {
		for (auto& e : data) e /= normalizer;
		return *this;
	}

	// element-wise add
	vector& operator+=(const vector<Scalar>& offset) {
		for (size_t i = 0; i < size(); ++i) {
			data[i] += offset[i];
		}
		return *this;
	}
	// element-wise subtract
	vector& operator-=(const vector<Scalar>& offset) {
		for (size_t i = 0; i < size(); ++i) {
			data[i] -= offset[i];
		}
		return *this;
	}
	// element-wise multiply
	vector& operator*=(const vector<Scalar>& scaler) {
		for (size_t i = 0; i < size(); ++i) {
			data[i] *= scaler[i];
		}
		return *this;
	}
	// element-wise divide
	vector& operator/=(const vector<Scalar>& normalizer) {
		for (size_t i = 0; i < size(); ++i) {
			data[i] /= normalizer[i];
		}
		return *this;
	}

	// non-reproducible sum
	Scalar sum() const {
		Scalar sum(0);  // should we do this with a quire?
		for (auto v : data) sum += v;
		return sum;
	}
	// two-norm of a vector
	Scalar norm() const {  // default is 2-norm
		using std::sqrt;
		Scalar twoNorm = 0;
		for (auto v : data) twoNorm += v * v;
		return sqrt(twoNorm);
	}

	// inf-norm of a vector
	Scalar infnorm() const {  // default is 2-norm
		Scalar infNorm = 0;
		for (auto v : data) infNorm = (abs(v)>infNorm) ? abs(v) : infNorm;
		return infNorm;
	}

	// Print elements as a column (jquinlan)
	void disp(){
		for (auto v : data) {
			std::cout << v << '\n';
		}
	}

// modifiers
	vector& assign(const Scalar& val) {
		for (auto& v : data) v = val;
		return *this;
	}
	value_type& head(size_t index) { return data[index]; }
	value_type  tail(size_t index) const { return data[index]; }
	value_type& tail(size_t index) { return data[index]; }
	void push_back(const value_type& e) { data.push_back(e); }
	void resize(size_t N) {	data.resize(N); }

// selectors
	inline size_t size() const { return data.size(); }

	// Eigen operators I need to reverse engineer
	vector& array() {
		return *this;
	}
	vector& log() {
		return *this;
	}
	vector& matrix() {
		return *this;
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
	std::vector<Scalar> data;
};

// ostream operators
template<typename Scalar>
std::ostream& operator<<(std::ostream& ostr, const vector<Scalar>& v) {
	auto width = ostr.width();
	ostr << "[ ";
	for (size_t j = 0; j < size(v); ++j) ostr << std::setw(width) << v[j] << " ";
	ostr << " ]";
	return ostr;
}

// serialization operators

template<typename Scalar>
void save(std::ostream& ostr, const sw::universal::blas::vector<Scalar>& v) {
//	ostr << sw::universal::type_tag(Scalar()) << '\n';
//	ostr << sw::universal::type_field(Scalar()) << '\n';
	ostr << "shape(" << v.size() << ", 1)\n";
	unsigned i = 0;
	for (auto e : v) {
		ostr << type_tag(e) << ' ';
		// ostr << sw::universal::to_hex(e) << ' ';  // TODO: need to find which type does not have a to_hex() manipulator
		if ((++i % 16) == 0) ostr << '\n';
	}
	ostr << std::endl;
}

template<typename Scalar>
void restore(std::istream& istr, const sw::universal::blas::vector<Scalar>& v) {
	//TypeTag<<Scalar> tt;
	//istr << tt;
	//TypeField<Scalar> tf; 
	//istr << strTypeField;

	// parse "shape(" << v.size() << ", 1)\n";
	unsigned VectorSize{ 0 };
	std::string token{};
	istr >> token >> VectorSize >> token;
	for (unsigned i = 0; i < VectorSize; ++i) {
		std::string valueInHex;
		istr >> valueInHex;
		std::cout << valueInHex << '\n';
	}
}

template<typename Scalar>
vector<Scalar> operator+(const vector<Scalar>& lhs, const vector<Scalar>& rhs) {
	vector<Scalar> sum(lhs);
	return sum += rhs;
}

template<typename Scalar>
vector<Scalar> operator-(const vector<Scalar>& lhs, const vector<Scalar>& rhs) {
	vector<Scalar> difference(lhs);
	return difference -= rhs;
}

// scale a vector through operator* overload
template<typename Scalar>
vector<Scalar> operator*(const Scalar& alpha, const vector<Scalar>& x) {
	vector<Scalar> scaled(x);
	return scaled *= alpha;
}
 

// scale a vector through operator* overload
template<typename Scalar>
vector<Scalar> operator*(const vector<Scalar>& x, const Scalar& alpha) {
	vector<Scalar> scaled(x);
	return scaled *= alpha;
}

// scale a vector through operator/ overload
template<typename Scalar>
vector<Scalar> operator/(const vector<Scalar>& v, const Scalar& normalizer) {
	vector<Scalar> normalized(v);
	return normalized /= normalizer;
}

// TODO: this next overload will create an ambiguous overload if Scalar is an int as it will be the same as the function above

// scale a vector through operator/ overload
template<typename Scalar>
vector<Scalar> operator/(const vector<Scalar>& v, const int normalizer) {
	vector<Scalar> normalized(v);
	return normalized /= Scalar(normalizer);
}

template<typename Scalar> auto size(const vector<Scalar>& v) { return v.size(); }

// this design does not work well for universal as we would need to create
// enable_if() configurations for all possible type combinations
//template<typename Scalar>
// typename enable_if_posit<Scalar, Scalar> operator*(const vector<Scalar>& a, const vector<Scalar>& b) { // doesn't compile with gcc
// typename std::enable_if<sw::universal::is_posit<Scalar>, Scalar>::type operator*(const vector<Scalar>& a, const vector<Scalar>& b) {

#ifdef LATER
// regular dot product for non-posits
template<typename Scalar>
typename std::enable_if<std::is_floating_point<Scalar>::value,Scalar>::type operator*(const vector<Scalar>& a, const vector<Scalar>& b) {
//	std::cout << "dot product for " << typeid(Scalar).name() << std::endl;
	size_t N = size(a);
	if (size(a) != size(b)) {
		std::cerr << "vector sizes are different: " << N << " vs " << size(b) << '\n';
		return Scalar{ 0 };
	}
	Scalar sum{ 0 };
	for (size_t i = 0; i < N; ++i) {
		sum += a(i) * b(i);
	}
	return sum;
}

// regular dot product for integers
template<typename Scalar>
typename std::enable_if<std::is_integral<Scalar>::value, Scalar>::type operator*(const vector<Scalar>& a, const vector<Scalar>& b) {
//	std::cout << "dot product for " << typeid(Scalar).name() << std::endl;
	size_t N = size(a);
	if (size(a) != size(b)) {
		std::cerr << "vector sizes are different: " << N << " vs " << size(b) << '\n';
		return Scalar{ 0 };
	}
	Scalar sum{ 0 };
	for (size_t i = 0; i < N; ++i) {
		sum += a(i) * b(i);
	}
	return sum;
}
#endif

template<typename Scalar>
Scalar operator*(const vector<Scalar>& a, const vector<Scalar>& b) {
	//	std::cout << "dot product for " << typeid(Scalar).name() << std::endl;
	size_t N = size(a);
	if (size(a) != size(b)) {
		std::cerr << "vector sizes are different: " << N << " vs " << size(b) << '\n';
		return Scalar{ 0 };
	}
	Scalar sum{ 0 };
	for (size_t i = 0; i < N; ++i) {
		sum += a(i) * b(i);
//		std::cout << std::setw(15) << double(a(i)) << " * " << std::setw(15) << double(b(i)) << " cumulative sum: " << std::setw(15) << double(sum) << '\n';
	}
	return sum;
}

template<typename Scalar>
bool operator==(const vector<Scalar>& lhs, const vector<Scalar>& rhs) {
	if (lhs.size() != rhs.size()) return false;
	for (unsigned i = 0; i < lhs.size(); ++i) {
		if (lhs[i] != rhs[i]) return false;
	}
	return true;
}

template<typename Scalar>
bool operator!=(const vector<Scalar>& lhs, const vector<Scalar>& rhs) {
	return !(lhs == rhs);
}

}}} // namespace sw::universal::blas
