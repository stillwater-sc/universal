#pragma once
// unum2.hpp: definition of the flexible configuration universal number system
//
// Copyright (C) 2017-2021 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cassert>
#include <limits>

#include <universal/native/ieee754.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal {
		
// Forward definitions
template<unsigned esizesize, unsigned fsizesize, typename bt> class unum2;
template<unsigned esizesize, unsigned fsizesize, typename bt> unum2<esizesize,fsizesize,bt> abs(const unum2<esizesize,fsizesize,bt>& v);

// template class reprfsizesizeenting a value in scientific notation, using a template size for the number of fraction bits
template<unsigned esizesize, unsigned fsizesize, typename bt = uint8_t>
class unum2 {
public:
	static constexpr unsigned UTAGSIZE   = 1 + esizesize + fsizesize;
	static constexpr unsigned UTAGMASK   = unsigned(~(int64_t(-1) << UTAGSIZE));
	static constexpr unsigned EBITSMASK  = 1;
	static constexpr unsigned FBITSMASK  = 2;

	unum2() {}

	// specific value constructor
	constexpr unum2(const SpecificValue code) {
		switch (code) {
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		default:
			zero();
			break;
		case SpecificValue::minneg:
			minneg();
			break;
		case SpecificValue::maxneg:
			maxneg();
			break;
		}
	}

	unum2(signed char initial_value)        { *this = initial_value; }
	unum2(short initial_value)              { *this = initial_value; }
	unum2(int initial_value)                { *this = initial_value; }
	unum2(long long initial_value)          { *this = initial_value; }
	unum2(unsigned long long initial_value) { *this = initial_value; }
	unum2(float initial_value)              { *this = initial_value; }
	unum2(double initial_value)             { *this = initial_value; }
	unum2(long double initial_value)        { *this = initial_value; }
	unum2(const unum2& rhs)                  { *this = rhs; }

	// assignment operators
	unum2& operator=(signed char rhs) {
		return *this = (long long)(rhs);
	}
	unum2& operator=(short rhs) {
		return *this = (long long)(rhs);
	}
	unum2& operator=(int rhs) {
		return *this = (long long)(rhs);
	}
	unum2& operator=(long long rhs) {
		return *this;
	}
	unum2& operator=(unsigned long long rhs) {
		return *this;
	}
	unum2& operator=(float rhs) {

		return *this;
	}
	unum2& operator=(double rhs) {

		return *this;
	}
	unum2& operator=(long double rhs) {

		return *this;
	}

	// arithmetic operators
	// prefix operator
	unum2 operator-() const {				
		return *this;
	}

	unum2& operator+=(const unum2& rhs) {
		return *this;
	}
	unum2& operator+=(double rhs) {
		return *this += unum2(rhs);
	}
	unum2& operator-=(const unum2& rhs) {

		return *this;
	}
	unum2& operator-=(double rhs) {
		return *this -= unum2<esizesize, fsizesize>(rhs);
	}
	unum2& operator*=(const unum2& rhs) {

		return *this;
	}
	unum2& operator*=(double rhs) {
		return *this *= unum2<esizesize, fsizesize>(rhs);
	}
	unum2& operator/=(const unum2& rhs) {

		return *this;
	}
	unum2& operator/=(double rhs) {
		return *this /= unum2<esizesize, fsizesize>(rhs);
	}
	unum2& operator++() {
		return *this;
	}
	unum2 operator++(int) {
		unum2 tmp(*this);
		operator++();
		return tmp;
	}
	unum2& operator--() {
		return *this;
	}
	unum2 operator--(int) {
		unum2 tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers

	/// <summary>
	/// clear the content of this bfloat to zero
	/// </summary>
	/// <returns>void</returns>
	inline constexpr void clear() noexcept {

	}
	/// <summary>
	/// set the number to +0
	/// </summary>
	/// <returns>void</returns>
	inline constexpr void setzero() noexcept { clear(); }
	/// <summary>
	/// set the number to +inf
	/// </summary>
	/// <param name="sign">boolean to make it + or - infinity, default is -inf</param>
	/// <returns>void</returns> 
	inline constexpr void setinf(bool sign = true) noexcept {

	}
	/// <summary>
	/// set the number to a quiet NaN (+nan) or a signalling NaN (-nan, default)
	/// </summary>
	/// <param name="sign">boolean to make it + or - infinity, default is -inf</param>
	/// <returns>void</returns> 
	inline constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING) noexcept {
	}
	// specific number system values of interest
	inline constexpr unum2& maxpos() noexcept {

		return *this;
	}
	inline constexpr unum2& minpos() noexcept {

		return *this;
	}
	inline constexpr unum2& zero() noexcept {

		return *this;
	}
	inline constexpr unum2& minneg() noexcept {

		return *this;
	}
	inline constexpr unum2& maxneg() noexcept {

		return *this;
	}

	// selectors
	inline bool isneg() const { return false; }
	inline bool iszero() const { return false; }
	inline bool isinf() const { return false; }
	inline bool isnan() const { return false; }
	inline bool issnan() const { return false; }
	inline bool isqnan() const { return false; }
	inline bool sign() const { return false; }
	inline int32_t scale() const { return false; } // 2^+-2^31 should be enough to capture empirical use cases
	inline std::string get() const { return std::string("tbd"); }


	long double to_long_double() const {
		return 0.0l;
	}
	double to_double() const {
		return 0.0;
	}
	float to_float() const {
		return 0.0f;
	}
	// Maybe remove explicit
	explicit operator long double() const { return to_long_double(); }
	explicit operator double() const { return to_double(); }
	explicit operator float() const { return to_float(); }

private:

	// template parameters need namfsizesize different from class template parameters (for gcc and clang)
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend std::ostream& operator<< (std::ostream& ostr, const unum2<nesizesize,nfsizesize,nbt>& r);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend std::istream& operator>> (std::istream& istr, unum2<nesizesize,nfsizesize,nbt>& r);

	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator==(const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator!=(const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator< (const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator> (const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator<=(const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs);
	template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
	friend bool operator>=(const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs);
};

////////////////////// operators
template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
inline std::ostream& operator<<(std::ostream& ostr, const unum2<nesizesize,nfsizesize,nbt>& v) {

	return ostr;
}

template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
inline std::istream& operator>>(std::istream& istr, const unum2<nesizesize,nfsizesize,nbt>& v) {
	istr >> v._fraction;
	return istr;
}

template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
inline bool operator==(const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs) { return false; }
template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
inline bool operator!=(const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs) { return !operator==(lhs, rhs); }
template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
inline bool operator< (const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs) { return false; }
template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
inline bool operator> (const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs) { return  operator< (rhs, lhs); }
template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
inline bool operator<=(const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs) { return !operator> (lhs, rhs); }
template<unsigned nesizesize, unsigned nfsizesize, typename nbt>
inline bool operator>=(const unum2<nesizesize,nfsizesize,nbt>& lhs, const unum2<nesizesize,nfsizesize,nbt>& rhs) { return !operator< (lhs, rhs); }

// posit - posit binary arithmetic operators
// BINARY ADDITION
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline unum2<esizesize, fsizesize, bt> operator+(const unum2<esizesize, fsizesize, bt>& lhs, const unum2<esizesize, fsizesize, bt>& rhs) {
	unum2<esizesize, fsizesize> sum(lhs);
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline unum2<esizesize, fsizesize, bt> operator-(const unum2<esizesize, fsizesize, bt>& lhs, const unum2<esizesize, fsizesize, bt>& rhs) {
	unum2<esizesize, fsizesize> diff(lhs);
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline unum2<esizesize, fsizesize, bt> operator*(const unum2<esizesize, fsizesize, bt>& lhs, const unum2<esizesize, fsizesize, bt>& rhs) {
	unum2<esizesize, fsizesize> mul(lhs);
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned esizesize, unsigned fsizesize, typename bt>
inline unum2<esizesize, fsizesize, bt> operator/(const unum2<esizesize, fsizesize, bt>& lhs, const unum2<esizesize, fsizesize, bt>& rhs) {
	unum2<esizesize, fsizesize> ratio(lhs);
	ratio /= rhs;
	return ratio;
}


template<unsigned esizesize, unsigned fsizesize, typename bt>
inline std::string components(const unum2<esizesize,fsizesize,bt>& v) {
	std::stringstream s;
	if (v.iszero()) {
		s << " zero b" << std::setw(esizesize) << v.fraction();
		return s.str();
	}
	else if (v.isinf()) {
		s << " infinite b" << std::setw(esizesize) << v.fraction();
		return s.str();
	}
	s << "(" << (v.sign() ? "-" : "+") << "," << v.scale() << "," << v.fraction() << ")";
	return s.str();
}

/// Magnitude of a scientific notation value (equivalent to turning the sign bit off).
template<unsigned esizesize, unsigned fsizesize, typename bt>
unum2<esizesize,fsizesize> abs(const unum2<esizesize,fsizesize,bt>& v) {
	return unum2<esizesize,fsizesize>(false, v.scale(), v.fraction(), v.isZero());
}


}} // namespace sw::universal
