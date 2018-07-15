#pragma once
// valid.hpp: definition of arbitrary valid number configurations
//
// Copyright (C) 2017-2018 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <limits>

namespace sw {

	namespace unum {

template<size_t nbits, size_t es>
class valid {

	static_assert(es + 3 <= nbits, "Value for 'es' is too large for this 'nbits' value");
	//static_assert(sizeof(long double) == 16, "Valid library requires compiler support for 128 bit long double.");

	template <typename T>
	valid<nbits, es>& _assign(const T& rhs) {
		constexpr int fbits = std::numeric_limits<T>::digits - 1;
		value<fbits> v((T)rhs);

		// special case processing
		if (v.isZero()) {
			setToZero();
			return *this;
		}
		if (v.isInfinite() || v.isNaN()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
			setToNaR();
			return *this;
		}

		convert(v);
		return *this;
	}

public:
	static constexpr size_t somebits = 10;

	valid<nbits, es>() { setToZero(); }

	valid(const valid&) = default;
	valid(valid&&) = default;

	valid& operator=(const valid&) = default;
	valid& operator=(valid&&) = default;

	valid(int initial_value)                { *this = initial_value; }
	valid(unsigned long long initial_value) { *this = initial_value; }
	valid(double initial_value)             { *this = initial_value; }
	valid(long double initial_value)        { *this = initial_value; }

	valid& operator=(int rhs) { return _assign(rhs); }
	valid& operator=(unsigned long long rhs) { return _assign(rhs); }
	valid& operator=(double rhs) { return _assign(rhs); }
	valid& operator=(long double rhs) { return _assign(rhs); }

	valid& operator+=(const valid& rhs) {
		return *this;
	}
	valid& operator-=(const valid& rhs) {
		return *this;
	}
	valid& operator*=(const valid& rhs) {
		return *this;
	}
	valid& operator/=(const valid& rhs) {
		return *this;
	}

	// conversion operators

	// Maybe remove explicit, MTL compiles, but we have lots of double computation then
	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }
	explicit operator long long() const { return to_long_long(); }
	explicit operator long() const { return to_long(); }
	explicit operator int() const { return to_int(); }

	// selectors
	bool isNaR() const {
		return false;
	}
	bool isZero() const {
		return false;
	}

	// modifiers
	inline void clear() {

	}
	inline void setToZero() {

	}
	inline void setToNaR() {

	}

private:
	// member variables



	// helper methods
	
	int         to_int() const {
		if (isZero()) return 0;
		if (isNaR()) throw "NaR (Not a Real)";
		return int(to_float());
	}
	long        to_long() const {
		if (isZero()) return 0;
		if (isNaR()) throw "NaR (Not a Real)";
		return long(to_double());
	}
	long long   to_long_long() const {
		if (isZero()) return 0;
		if (isNaR()) throw "NaR (Not a Real)";
		return long(to_long_double());
	}
	float       to_float() const {
		return (float)to_double();
	}
	double      to_double() const {
		if (isZero())	return 0.0;
		if (isNaR())	return NAN;
		return 0.0;
	}
	long double to_long_double() const {
		if (isZero())  return 0.0;
		if (isNaR())   return NAN;

		return 0.0;
	}

	// friends
	// template parameters need names different from class template parameters (for gcc and clang)
	template<size_t nnbits, size_t ees>
	friend std::ostream& operator<< (std::ostream& ostr, const valid<nnbits, ees>& p);
	template<size_t nnbits, size_t ees>
	friend std::istream& operator>> (std::istream& istr, valid<nnbits, ees>& p);

	template<size_t nnbits, size_t ees>
	friend bool operator==(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator!=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator< (const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator> (const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator<=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
	template<size_t nnbits, size_t ees>
	friend bool operator>=(const valid<nnbits, ees>& lhs, const valid<nnbits, ees>& rhs);
};


// VALID operators
template<size_t nbits, size_t es>
inline std::ostream& operator<<(std::ostream& ostr, const valid<nbits, es>& p) {
	if (p.isZero()) {
		ostr << double(0.0);
		return ostr;
	}
	else if (p.isNaR()) {
		ostr << "NaR";
		return ostr;
	}
	ostr << p.to_double();
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator>> (std::istream& istr, const valid<nbits, es>& p) {
	istr >> p._Bits;
	return istr;
}

	}  // namespace unum

} // namespace sw
