#pragma once 
// quire_operators.hpp: definition of quire operators
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <limits>

#define NOW 0

#if NOW
// QUIRE BINARY ARITHMETIC OPERATORS
template<size_t nbits, size_t es, size_t capacity>
inline quire<nbits,es,capacity> operator+(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) {
	quire<nbits, es> sum = lhs;
	sum += rhs;
	return sum;
}

template<size_t nbits, size_t es, size_t capacity>
inline quire<nbits, es, capacity> operator-(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) {
	quire<nbits, es, capacity> diff = lhs;
	diff -= rhs;
	return diff;
}

template<size_t nbits, size_t es, size_t capacity>
inline quire<nbits, es, capacity> operator*(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) {
	quire<nbits, es, capacity> mul = lhs;
	mul *= rhs;
	return mul;
}

template<size_t nbits, size_t es, size_t capacity>
inline quire<nbits, es, capacity> operator/(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) {
	quire<nbits, es, capacity> ratio = lhs;
	ratio /= rhs;
	return ratio;
}
#endif

////////////////// QUIRE
template<size_t nbits, size_t es, size_t capacity>
inline std::ostream& operator<<(std::ostream& ostr, const quire<nbits, es, capacity>& q) {
	ostr << q.sign_value << ": " << q._accu;
	return ostr;
}

template<size_t nbits, size_t es, size_t capacity>
inline std::istream& operator>> (std::istream& istr, const quire<nbits, es, capacity>& q) {
	istr >> q._accu;
	return istr;
}

template<size_t nbits, size_t es, size_t capacity>
inline bool operator==(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return lhs._accu == rhs._accu; }
template<size_t nbits, size_t es, size_t capacity>
inline bool operator!=(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es, size_t capacity>
inline bool operator< (const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return lhs._accu < rhs._accu; }
template<size_t nbits, size_t es, size_t capacity>
inline bool operator> (const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, size_t es, size_t capacity>
inline bool operator<=(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, size_t es, size_t capacity>
inline bool operator>=(const quire<nbits, es, capacity>& lhs, const quire<nbits, es, capacity>& rhs) { return !operator< (lhs, rhs); }
