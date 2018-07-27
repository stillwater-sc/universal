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

		return *this;
	}

public:
	static constexpr size_t somebits = 10;

	valid<nbits, es>() { clear(); }

	valid(const valid&) = default;
	valid(valid&&) = default;

	valid& operator=(const valid&) = default;
	valid& operator=(valid&&) = default;

	valid(long initial_value)               { *this = initial_value; }
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
	inline bool isOpen() const {
		return !isClosed();
	}
	inline bool isClosed() const {
		return lubit && uubit;
	}
	inline bool isOpenLower() const {
		return lubit;
	}
	inline bool isOpenUpper() const {
		return uubit;
	}

	// modifiers

	// TODO: do we clear to exact 0, or [-inf, inf]?
	inline void clear() {
		lb.clear();
		ub.clear();
		lubit = true;
		uubit = true;
	}
	inline void setToInclusive() {
		lb.setToNaR();
		ub.setToNaR();
		lubit = true;
		uubit = true;
	}
	inline void setLowerBound(sw::unum::posit<nbits, es>& _lb, bool ubit) {
		lb = _lb;
		lubit = ubit;
	}
	inline void setUpperBound(sw::unum::posit<nbits, es>& _ub, bool ubit) {
		ub = _ub;
		uubit = ubit;
	}

private:
	// member variables
	sw::unum::posit<nbits, es> lb, ub;  // lower_bound and upper_bound of the tile
	bool lubit, uubit; // lower ubit, upper ubit


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
inline std::ostream& operator<<(std::ostream& ostr, const valid<nbits, es>& v) {
	// determine lower bound
	if (v.lubit == true) {  // exact lower bound
		ostr << '[' << v.lb << ", ";
	}
	else {					// inexact lower bound
		ostr << '(' << v.lb << ", ";
	}
	if (v.uubit == true) { // exact upper bound
		ostr << v.ub << ']';
	}
	else {					// inexact upper bound
		ostr << v.ub << ')';
	}
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator>> (std::istream& istr, const valid<nbits, es>& v) {
	istr >> p._Bits;
	return istr;
}

	}  // namespace unum

} // namespace sw
