#pragma once
// ereal_impl.hpp: implementation of an adaptive precision multi-component floating-point number system
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <regex>
#include <vector>
#include <map>

// supporting types and functions
#include <universal/native/ieee754.hpp>   // IEEE-754 decoders
#include <universal/number/shared/specific_value_encoding.hpp>
#include <universal/internal/expansion/expansion_ops.hpp>  // Shewchuk's expansion arithmetic

/*
The ereal arithmetic can be configured to:
- throw exceptions on invalid arguments and operations
- return a signaling NaN

Compile-time configuration flags are used to select the exception mode.

The exception types are defined, but you have the option to throw them
*/
#include <universal/number/ereal/exceptions.hpp>

namespace sw { namespace universal {


// ereal is a multi-component arbitrary-precision arithmetic type
template<unsigned maxlimbs = 1024>
class ereal {
public:
	static constexpr unsigned maxNrLimbs = maxlimbs;

	// constructor
	ereal() : _limb{ 0 } { }

	ereal(const ereal&) = default;
	ereal(ereal&&) = default;

	ereal& operator=(const ereal&) = default;
	ereal& operator=(ereal&&) = default;

	// initializers for native types
	ereal(signed char iv)                      noexcept { *this = iv; }
	ereal(short iv)                            noexcept { *this = iv; }
	ereal(int iv)                              noexcept { *this = iv; }
	ereal(long iv)                             noexcept { *this = iv; }
	ereal(long long iv)                        noexcept { *this = iv; }
	ereal(char iv)                             noexcept { *this = iv; }
	ereal(unsigned short iv)                   noexcept { *this = iv; }
	ereal(unsigned int iv)                     noexcept { *this = iv; }
	ereal(unsigned long iv)                    noexcept { *this = iv; }
	ereal(unsigned long long iv)               noexcept { *this = iv; }
	ereal(float iv)                            noexcept { *this = iv; }
	ereal(double iv)                           noexcept { *this = iv; }

	// assignment operators for native types
	ereal& operator=(signed char rhs)          noexcept { return convert_signed(rhs); }
	ereal& operator=(short rhs)                noexcept { return convert_signed(rhs); }
	ereal& operator=(int rhs)                  noexcept { return convert_signed(rhs); }
	ereal& operator=(long rhs)                 noexcept { return convert_signed(rhs); }
	ereal& operator=(long long rhs)            noexcept { return convert_signed(rhs); }
	ereal& operator=(char rhs)                 noexcept { return convert_unsigned(rhs); }
	ereal& operator=(unsigned short rhs)       noexcept { return convert_unsigned(rhs); }
	ereal& operator=(unsigned int rhs)         noexcept { return convert_unsigned(rhs); }
	ereal& operator=(unsigned long rhs)        noexcept { return convert_unsigned(rhs); }
	ereal& operator=(unsigned long long rhs)   noexcept { return convert_unsigned(rhs); }
	ereal& operator=(float rhs)                noexcept { return convert_ieee754(rhs); }
	ereal& operator=(double rhs)               noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator float()             const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double()            const noexcept { return convert_to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	ereal(long double iv)                      noexcept { *this = iv; }
	ereal& operator=(long double rhs)          noexcept { return convert_ieee754(rhs); }
	explicit operator long double()       const noexcept { return convert_to_ieee754<long double>(); }
#endif 

	// prefix operators
	ereal operator-() const {
		ereal negated(*this);
		return negated;
	}

	// arithmetic operators
	ereal& operator+=(const ereal& rhs) {
		using namespace expansion_ops;
		_limb = linear_expansion_sum(_limb, rhs._limb);
		return *this;
	}
	ereal& operator+=(double rhs) {
		using namespace expansion_ops;
		ereal<maxlimbs> rhs_expansion(rhs);
		_limb = linear_expansion_sum(_limb, rhs_expansion._limb);
		return *this;
	}
	ereal& operator-=(const ereal& rhs) {
		using namespace expansion_ops;
		// Negate rhs components and add
		std::vector<double> neg_rhs = rhs._limb;
		for (auto& v : neg_rhs) v = -v;
		_limb = linear_expansion_sum(_limb, neg_rhs);
		return *this;
	}
	ereal& operator-=(double rhs) {
		return operator-=(ereal<maxlimbs>(rhs));
	}
	ereal& operator*=(const ereal& rhs) {
		using namespace expansion_ops;
		_limb = expansion_product(_limb, rhs._limb);
		return *this;
	}
	ereal& operator*=(double rhs) {
		using namespace expansion_ops;
		_limb = scale_expansion(_limb, rhs);
		return *this;
	}
	ereal& operator/=(const ereal& rhs) {
		using namespace expansion_ops;
		_limb = expansion_quotient(_limb, rhs._limb);
		return *this;
	}
	ereal& operator/=(double rhs) {
		using namespace expansion_ops;
		ereal<maxlimbs> rhs_expansion(rhs);
		_limb = expansion_quotient(_limb, rhs_expansion._limb);
		return *this;
	}

	// modifiers
	void clear() { _limb.clear(); _limb.push_back(0.0); }
	void setzero() { clear(); }

	ereal& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	bool iszero() const noexcept { return _limb[0] == 0.0; }
	bool isone()  const noexcept { return _limb[0] == 1.0; }
	bool ispos()  const noexcept { return _limb[0] > 0.0; }
	bool isneg()  const noexcept { return _limb[0] < 0.0; }
	bool isinf()  const noexcept { return false; }
	bool isnan()  const noexcept { return false; }
	bool isqnan()  const noexcept { return false; }
	bool issnan()  const noexcept { return false; }


	// value information selectors
	int     sign()        const noexcept { return (isneg() ? -1 : 1); }
	int64_t scale()       const noexcept { return sw::universal::scale(_limb[0]); }
	double  significant() const noexcept { return _limb[0]; }
	const std::vector<double>& limbs() const noexcept { return _limb; }
	//std::vector<uint32_t> bits() const { return _limb; }

protected:
	std::vector<double> _limb;     // components of the real value

	// HELPER methods

	// convert arithmetic types into an elastic floating-point
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	ereal& convert_signed(SignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {

		}
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	ereal& convert_unsigned(UnsignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {

		}
		return *this;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	ereal& convert_ieee754(Real rhs) noexcept {
		clear();
		_limb[0] = rhs;
		return *this;
	}


	// convert elastic floating-point to native ieee-754
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	Real convert_to_ieee754() const noexcept {
		// Sum all components to get the full value
		Real sum = 0.0;
		for (const auto& component : _limb) {
			sum += static_cast<Real>(component);
		}
		return sum;
	}

private:

	// find the most significant bit set
	friend signed findMsb(const ereal& v);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////    ereal functions   /////////////////////////////////

template<unsigned nlimbs>
inline ereal<nlimbs> abs(const ereal<nlimbs>& a) {
	return a; // (a < 0 ? -a : a);
}

////////////////////////////////////////////////////////////////////////////////
/// stream operators

// read a ereal ASCII format and make a binary ereal out of it
template<unsigned nlimbs>
bool parse(const std::string& txt, ereal<nlimbs>& value) {
	bool bSuccess = false;
	value.clear();
	return bSuccess;
}

// generate an ereal format ASCII format
template<unsigned nlimbs>
inline std::ostream& operator<<(std::ostream& ostr, const ereal<nlimbs>& rhs) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the ereal into a string
	std::stringstream ss;

	if (rhs.isinf()) {
		ss << (rhs.sign() == -1 ? "-inf" : "+inf");
	}
	else if (rhs.isqnan()) {
		ss << "nan(qnan)";
	}
	else if (rhs.issnan()) {
		ss << "nan(snan)";
	}
	else {
		std::streamsize prec = ostr.precision();
		std::streamsize width = ostr.width();
		std::ios_base::fmtflags ff;
		ff = ostr.flags();
		ss.flags(ff);
		ss << std::setw(width) << std::setprecision(prec) << "TBD";
	}

	return ostr << ss.str();
}

// read an ASCII ereal format
template<unsigned nlimbs>
inline std::istream& operator>>(std::istream& istr, ereal<nlimbs>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a floating-point value\n";
	}
	return istr;
}

////////////////// string operators


//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - ereal binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<unsigned nlimbs>
inline bool operator==(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	using namespace expansion_ops;
	return compare_adaptive(lhs.limbs(), rhs.limbs()) == 0;
}
template<unsigned nlimbs>
inline bool operator!=(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	using namespace expansion_ops;
	return compare_adaptive(lhs.limbs(), rhs.limbs()) < 0;
}
template<unsigned nlimbs>
inline bool operator> (const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs>
inline bool operator<=(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nlimbs>
inline bool operator==(const ereal<nlimbs>& lhs, double rhs) {
	return operator==(lhs, ereal<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline bool operator!=(const ereal<nlimbs>& lhs, double rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (const ereal<nlimbs>& lhs, double rhs) {
	return operator<(lhs, ereal<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline bool operator> (const ereal<nlimbs>& lhs, double rhs) {
	return operator< (ereal<nlimbs>(rhs), lhs);
}
template<unsigned nlimbs>
inline bool operator<=(const ereal<nlimbs>& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(const ereal<nlimbs>& lhs, double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - ereal binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<unsigned nlimbs>
inline bool operator==(double lhs, const ereal<nlimbs>& rhs) {
	return operator==(ereal<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline bool operator!=(double lhs, const ereal<nlimbs>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (double lhs, const ereal<nlimbs>& rhs) {
	return operator<(ereal<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline bool operator> (double lhs, const ereal<nlimbs>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs>
inline bool operator<=(double lhs, const ereal<nlimbs>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(double lhs, const ereal<nlimbs>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - ereal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline ereal<nlimbs> operator+(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	ereal<nlimbs> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline ereal<nlimbs> operator-(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	ereal<nlimbs> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline ereal<nlimbs> operator*(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	ereal<nlimbs> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nlimbs>
inline ereal<nlimbs> operator/(const ereal<nlimbs>& lhs, const ereal<nlimbs>& rhs) {
	ereal<nlimbs> ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// ereal - literal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline ereal<nlimbs> operator+(const ereal<nlimbs>& lhs, double rhs) {
	return operator+(lhs, ereal<nlimbs>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline ereal<nlimbs> operator-(const ereal<nlimbs>& lhs, double rhs) {
	return operator-(lhs, ereal<nlimbs>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline ereal<nlimbs> operator*(const ereal<nlimbs>& lhs, double rhs) {
	return operator*(lhs, ereal<nlimbs>(rhs));
}
// BINARY DIVISION
template<unsigned nlimbs>
inline ereal<nlimbs> operator/(const ereal<nlimbs>& lhs, double rhs) {
	return operator/(lhs, ereal<nlimbs>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - ereal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline ereal<nlimbs> operator+(double lhs, const ereal<nlimbs>& rhs) {
	return operator+(ereal<nlimbs>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline ereal<nlimbs> operator-(double lhs, const ereal<nlimbs>& rhs) {
	return operator-(ereal<nlimbs>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline ereal<nlimbs> operator*(double lhs, const ereal<nlimbs>& rhs) {
	return operator*(ereal<nlimbs>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nlimbs>
inline ereal<nlimbs> operator/(double lhs, const ereal<nlimbs>& rhs) {
	return operator/(ereal<nlimbs>(lhs), rhs);
}

}} // namespace sw::universal
