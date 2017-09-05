#pragma once 

#include <limits>
#include "posit_scale_factors.hpp"

template<size_t nbits, size_t es>
std::string spec_to_string(posit<nbits, es> p) {
	std::stringstream ss;
	ss << "posit<" << std::setw(2) << nbits << "," << es << "> ";
	ss << "useed " << std::setw(4) << p.useed()    << "     ";
	ss << std::setprecision(12);
	ss << "minpos " << std::setw(20) << p.minpos() << "     ";
	ss << "maxpos " << std::setw(20) << p.maxpos();
	return ss.str();
}

template<size_t nbits, size_t es>
std::string components_to_string(posit<nbits,es> p) {
	std::stringstream ss;
	if (p.isZero()) {
		ss << " zero    " << std::setw(103) << "b" << p.get();
		return ss.str();
	}
	else if (p.isInfinite()) {
		ss << " infinite" << std::setw(103) << "b" << p.get();
		return ss.str();
	}

	ss << std::setw(14) << to_binary(p.get())
		<< " Sign : " << std::setw(2) << p.sign()
		<< " Regime : " << std::setw(3) << p.regime_k()
		<< " Exponent : " << std::setw(5) << p.exponent_int()
		<< " Fraction : " << std::setw(8) << std::setprecision(7) << 1.0 + p.fraction()
		<< " Value : " << std::setw(16) << p.to_double()
		<< std::setprecision(0);
	return ss.str();
}

template<size_t nbits, size_t es>
std::string component_values_to_string(posit<nbits,es> p) {
	std::stringstream ss;
	if (p.isZero()) {
		ss << " zero    " << std::setw(103) << "b" << p.get();
		return ss.str();
	}
	else if (p.isInfinite()) {
		ss << " infinite" << std::setw(103) << "b" << p.get();
		return ss.str();
	}

	ss << std::setw(14) << to_binary(p.get())
		<< " Sign : " << std::setw(2) << p.sign()
		<< " Regime : " << p.regime_int()
		<< " Exponent : " << p.exponent_int()
		<< std::hex
		<< " Fraction : " << p.fraction_int()
		<< " Value : " << p.to_int64()
		<< std::dec;
	return ss.str();
}

template<size_t nbits, size_t es>
inline posit<nbits,es> operator+(posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> sum = lhs;
	sum += rhs;
	return sum;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator-(posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> diff = lhs;
	diff -= rhs;
	return diff;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator*(posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> mul = lhs;
	mul *= rhs;
	return mul;
}

template<size_t nbits, size_t es>
inline posit<nbits, es> operator/(posit<nbits, es>& lhs, const posit<nbits, es>& rhs) {
	posit<nbits, es> ratio = lhs;
	ratio /= rhs;
	return ratio;
}

template<size_t nbits, size_t es>
inline std::ostream& operator<<(std::ostream& ostr, const posit<nbits, es>& p) {
	if (p.isZero()) {
		ostr << double(0.0);
		return ostr;
	}
	else if (p.isInfinite()) {
		ostr << std::numeric_limits<double>::infinity();
		return ostr;
	}	
	ostr << p.to_double();
	return ostr;
}

template<size_t nbits, size_t es>
inline std::istream& operator >> (std::istream& istr, const posit<nbits, es>& p) {
	istr >> p._Bits;
	return istr;
}

template<size_t nbits, size_t es>
inline bool operator==(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return lhs._Bits == rhs._Bits; }
template<size_t nbits, size_t es>
inline bool operator!=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator==(lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator< (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return lhs._Bits < rhs._Bits; }
template<size_t nbits, size_t es>
inline bool operator> (const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return  operator< (rhs, lhs); }
template<size_t nbits, size_t es>
inline bool operator<=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator> (lhs, rhs); }
template<size_t nbits, size_t es>
inline bool operator>=(const posit<nbits, es>& lhs, const posit<nbits, es>& rhs) { return !operator< (lhs, rhs); }
