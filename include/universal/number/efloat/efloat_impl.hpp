#pragma once
// efloat_impl.hpp: implementation of an adaptive precision binary floating-point number system
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

/*
The efloat arithmetic can be configured to:
- throw exceptions on invalid arguments and operations
- return a signaling NaN

Compile-time configuration flags are used to select the exception mode.

The exception types are defined, but you have the option to throw them
*/
#include <universal/number/efloat/exceptions.hpp>

namespace sw { namespace universal {

enum class FloatingPointState {
	Zero,
	Normal,
	SignalingNaN,             // let's use the US English spelling
	QuietNaN,
	Infinite
};

// efloat is an adaptive precision linear floating-point type
template<unsigned nlimbs = 1024>
class efloat {
public:
	static constexpr unsigned maxNrLimbs = nlimbs;

	// constructor
	efloat() : _state{ FloatingPointState::Zero }, _sign{ false }, _exponent{ 0 }, _limb{ 0 } { }

	efloat(const efloat&) = default;
	efloat(efloat&&) = default;

	efloat& operator=(const efloat&) = default;
	efloat& operator=(efloat&&) = default;

	// initializers for native types
	efloat(signed char iv)                      noexcept { *this = iv; }
	efloat(short iv)                            noexcept { *this = iv; }
	efloat(int iv)                              noexcept { *this = iv; }
	efloat(long iv)                             noexcept { *this = iv; }
	efloat(long long iv)                        noexcept { *this = iv; }
	efloat(char iv)                             noexcept { *this = iv; }
	efloat(unsigned short iv)                   noexcept { *this = iv; }
	efloat(unsigned int iv)                     noexcept { *this = iv; }
	efloat(unsigned long iv)                    noexcept { *this = iv; }
	efloat(unsigned long long iv)               noexcept { *this = iv; }
	efloat(float iv)                            noexcept { *this = iv; }
	efloat(double iv)                           noexcept { *this = iv; }

	// assignment operators for native types
	efloat& operator=(signed char rhs)          noexcept { return convert_signed(rhs); }
	efloat& operator=(short rhs)                noexcept { return convert_signed(rhs); }
	efloat& operator=(int rhs)                  noexcept { return convert_signed(rhs); }
	efloat& operator=(long rhs)                 noexcept { return convert_signed(rhs); }
	efloat& operator=(long long rhs)            noexcept { return convert_signed(rhs); }
	efloat& operator=(char rhs)                 noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned short rhs)       noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned int rhs)         noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned long rhs)        noexcept { return convert_unsigned(rhs); }
	efloat& operator=(unsigned long long rhs)   noexcept { return convert_unsigned(rhs); }
	efloat& operator=(float rhs)                noexcept { return convert_ieee754(rhs); }
	efloat& operator=(double rhs)               noexcept { return convert_ieee754(rhs); }

	// conversion operators
	explicit operator float()             const noexcept { return convert_to_ieee754<float>(); }
	explicit operator double()            const noexcept { return convert_to_ieee754<double>(); }

#if LONG_DOUBLE_SUPPORT
	efloat(long double iv)                      noexcept { *this = iv; }
	efloat& operator=(long double rhs)          noexcept { return convert_ieee754(rhs); }
	explicit operator long double()       const noexcept { return convert_to_ieee754<long double>(); }
#endif 

	// prefix operators
	efloat operator-() const {
		efloat negated(*this);
		return negated;
	}

	// arithmetic operators
	efloat& operator+=(const efloat& rhs) {
		return *this;
	}
	efloat& operator+=(double rhs) {
		return *this;
	}
	efloat& operator-=(const efloat& rhs) {
		return *this;
	}
	efloat& operator-=(double rhs) {
		return *this;
	}
	efloat& operator*=(const efloat& rhs) {
		return *this;
	}
	efloat& operator*=(double rhs) {
		return *this;
	}
	efloat& operator/=(const efloat& rhs) {
		return *this;
	}
	efloat& operator/=(double rhs) {
		return *this;
	}

	// modifiers
	void clear() { _state = FloatingPointState::Normal;  _sign = false; _exponent = 0; _limb.clear(); }
	void setzero() { clear(); }

	efloat& assign(const std::string& txt) {
		return *this;
	}

	// selectors
	bool iszero() const noexcept { return _state == FloatingPointState::Zero; }
	bool isone()  const noexcept { return (_state == FloatingPointState::Normal && !_sign && _exponent == 0 && _limb.size() == 1 && _limb[0] == 0x8000'000); }
	bool isodd()  const noexcept { return false; }
	bool iseven() const noexcept { return !isodd(); }
	bool ispos()  const noexcept { return (_state == FloatingPointState::Normal && !_sign); }
	bool isneg()  const noexcept { return (_state == FloatingPointState::Normal && _sign); }
	bool isinf()  const noexcept { return (_state == FloatingPointState::Infinite); }
	bool isnan()  const noexcept { return (_state == FloatingPointState::QuietNaN || _state == FloatingPointState::SignalingNaN); }
	bool isqnan()  const noexcept { return (_state == FloatingPointState::QuietNaN); }
	bool issnan()  const noexcept { return (_state == FloatingPointState::SignalingNaN); }


	// value information selectors
	int     sign()        const noexcept { return (_sign ? -1 : 1); }
	int64_t scale()       const noexcept { return _exponent; }
	double  significant() const noexcept {
		// efloat is a normalized floating-point, thus the significant falls in the range [1.0, 2.0)
		double v{ 0.0 };
		if (_state == FloatingPointState::Normal) {
			// build a 64-bit bit representation
			uint64_t raw{ 0 };
			switch (_limb.size()) {
			case 0:
				break;
			case 1:
				raw = _limb[0];
				raw <<= 32;
				break;
			case 2:
			default:
				raw = _limb[0];
				raw <<= 32;
				raw |= _limb[1];
				break;
			}
			raw &= 0x7FFF'FFFF'FFFF'FFFF; // remove hidden bit
			if (raw > 0) {
				v = double(raw)/ 9223372036854775808.0;
			}
			v += 1.0;
		}
		// else {
			// Zero, NaN or Infinity will return a significant value of 0.0
		// }

		return v;
	}
	std::vector<uint32_t> bits() const { return _limb; }

protected:
	FloatingPointState    _state;    // exceptional state
	bool                  _sign;     // sign of the number: -1 if true, +1 if false, zero is positive
	int64_t               _exponent; // exponent of the number
	std::vector<uint32_t> _limb;     // limbs of the representation

	// HELPER methods

	// convert arithmetic types into an elastic floating-point
	template<typename SignedInt,
		typename = typename std::enable_if< std::is_integral<SignedInt>::value, SignedInt >::type>
	efloat& convert_signed(SignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {

		}
		return *this;
	}

	template<typename UnsignedInt,
		typename = typename std::enable_if< std::is_integral<UnsignedInt>::value, UnsignedInt >::type>
	efloat& convert_unsigned(UnsignedInt v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {

		}
		return *this;
	}

	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	efloat& convert_ieee754(Real rhs) noexcept {
		clear();
		bool isSubnormal{ false };
		int nan_type{ NAN_TYPE_NEITHER };
		switch (std::fpclassify(rhs)) {
		case FP_ZERO:
			_state = FloatingPointState::Zero;
			_sign = false;
			_exponent = 0;
			// stay limbless
			return *this;
		case FP_NAN:
			_sign = sw::universal::sign(rhs);
			// x86 specific: the top bit of the significand = 1 for quiet, 0 for signaling
			checkNaN(rhs, nan_type);
			if (nan_type == NAN_TYPE_QUIET) {
				_state = FloatingPointState::QuietNaN;
			}
			else {
				_state = FloatingPointState::SignalingNaN;
			}
			_exponent = 0;
			// stay limbless
			return *this;
		case FP_INFINITE:
			_state = FloatingPointState::Infinite;
			_sign = sw::universal::sign(rhs);
			_exponent = 0;
			// stay limbless
			return *this;
		case FP_SUBNORMAL:
			isSubnormal = true;
			break;
		case FP_NORMAL:
		default:
			break;
		}

		_sign = sw::universal::sign(rhs);
		_exponent = sw::universal::scale(rhs); // scale already deals with subnormal numbers
		if constexpr (sizeof(Real) == 4) {
			uint32_t bits{ 0 };
			if (isSubnormal) { // subnormal number
				bits = sw::universal::_extractFraction<uint32_t, Real>(rhs);
				bits <<= 8; // 31 - 23 = 8 bits to get the hidden bit to land on bit 31
				uint32_t mask = 0x8000'0000;
				while ((mask & bits) == 0) {
					bits <<= 1;
				}
			}
			else {
				bits = sw::universal::_extractSignificant<uint32_t, Real>(rhs);
				bits <<= 8; // 31 - 23 = 8 bits to get the hidden bit to land on bit 31
			}
			_limb.push_back(bits);
		}
		else if constexpr (sizeof(Real) == 8) {
			uint64_t bits{ 0 };
			if (isSubnormal) { // subnormal number
				bits = sw::universal::_extractFraction<uint64_t, Real>(rhs);
				bits <<= 11; // 63 - 52 = 11 bits to get the hidden bit to land on bit 63
				uint64_t mask = 0x8000'0000'0000'0000;
				while ((mask & bits) == 0) {
					bits <<= 1;
				}
			}
			else {
				bits = sw::universal::_extractSignificant<uint64_t, Real>(rhs);
				bits <<= 11; // 63 - 52 = 11 bits to get the hidden bit to land on bit 63
			}
			_limb.push_back(static_cast<uint32_t>(bits >> 32));
			_limb.push_back(static_cast<uint32_t>(bits & 0xFFFF'FFFF));
		}
		else {
			static_assert(true);
		}
		return *this;
	}


	// convert elastic floating-point to native ieee-754
	template<typename Real,
		typename = typename std::enable_if< std::is_floating_point<Real>::value, Real >::type>
	Real convert_to_ieee754() const noexcept {
		Real v{ 0.0 };
		switch (_state) {
		case FloatingPointState::Zero:
			break;
		case FloatingPointState::QuietNaN:
			v = std::numeric_limits<Real>::quiet_NaN();
			break;
		case FloatingPointState::SignalingNaN:
			v = std::numeric_limits<Real>::signaling_NaN();
			break;
		case FloatingPointState::Infinite:
			v = (_sign ? -std::numeric_limits<Real>::infinity() : +std::numeric_limits<Real>::infinity());
			break;
		case FloatingPointState::Normal:
			v = Real(sign()) * std::pow(Real(2.0), Real(scale())) * Real(significant());
		}
		return v;
	}

private:

	// efloat - efloat logic comparisons
	friend bool operator==(const efloat& lhs, const efloat& rhs);

	// efloat - literal logic comparisons
	friend bool operator==(const efloat& lhs, const long long rhs);

	// literal - efloat logic comparisons
	friend bool operator==(const long long lhs, const efloat& rhs);

	// find the most significant bit set
	friend signed findMsb(const efloat& v);
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////    efloat functions   /////////////////////////////////

template<unsigned nlimbs>
inline efloat<nlimbs> abs(const efloat<nlimbs>& a) {
	return a; // (a < 0 ? -a : a);
}

////////////////////////////////////////////////////////////////////////////////
/// stream operators

// read a efloat ASCII format and make a binary efloat out of it
template<unsigned nlimbs>
bool parse(const std::string& txt, efloat<nlimbs>& value) {
	bool bSuccess = false;
	value.clear();
	return bSuccess;
}

// generate an efloat format ASCII format
template<unsigned nlimbs>
inline std::ostream& operator<<(std::ostream& ostr, const efloat<nlimbs>& rhs) {
	// to make certain that setw and left/right operators work properly
	// we need to transform the efloat into a string
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

// read an ASCII efloat format
template<unsigned nlimbs>
inline std::istream& operator>>(std::istream& istr, efloat<nlimbs>& p) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, p)) {
		std::cerr << "unable to parse -" << txt << "- into a floating-point value\n";
	}
	return istr;
}

////////////////// string operators


//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - efloat binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
template<unsigned nlimbs>
inline bool operator==(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	return true;
}
template<unsigned nlimbs>
inline bool operator!=(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	return false; // lhs and rhs are the same
}
template<unsigned nlimbs>
inline bool operator> (const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs>
inline bool operator<=(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
template<unsigned nlimbs>
inline bool operator==(const efloat<nlimbs>& lhs, double rhs) {
	return operator==(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline bool operator!=(const efloat<nlimbs>& lhs, double rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (const efloat<nlimbs>& lhs, double rhs) {
	return operator<(lhs, efloat<nlimbs>(rhs));
}
template<unsigned nlimbs>
inline bool operator> (const efloat<nlimbs>& lhs, double rhs) {
	return operator< (efloat<nlimbs>(rhs), lhs);
}
template<unsigned nlimbs>
inline bool operator<=(const efloat<nlimbs>& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(const efloat<nlimbs>& lhs, double rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

template<unsigned nlimbs>
inline bool operator==(double lhs, const efloat<nlimbs>& rhs) {
	return operator==(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline bool operator!=(double lhs, const efloat<nlimbs>& rhs) {
	return !operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator< (double lhs, const efloat<nlimbs>& rhs) {
	return operator<(efloat<nlimbs>(lhs), rhs);
}
template<unsigned nlimbs>
inline bool operator> (double lhs, const efloat<nlimbs>& rhs) {
	return operator< (rhs, lhs);
}
template<unsigned nlimbs>
inline bool operator<=(double lhs, const efloat<nlimbs>& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}
template<unsigned nlimbs>
inline bool operator>=(double lhs, const efloat<nlimbs>& rhs) {
	return !operator< (lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - efloat binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline efloat<nlimbs> operator+(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	efloat<nlimbs> sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline efloat<nlimbs> operator-(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	efloat<nlimbs> diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline efloat<nlimbs> operator*(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	efloat<nlimbs> mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
template<unsigned nlimbs>
inline efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, const efloat<nlimbs>& rhs) {
	efloat<nlimbs> ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// efloat - literal binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline efloat<nlimbs> operator+(const efloat<nlimbs>& lhs, double rhs) {
	return operator+(lhs, efloat<nlimbs>(rhs));
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline efloat<nlimbs> operator-(const efloat<nlimbs>& lhs, double rhs) {
	return operator-(lhs, efloat<nlimbs>(rhs));
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline efloat<nlimbs> operator*(const efloat<nlimbs>& lhs, double rhs) {
	return operator*(lhs, efloat<nlimbs>(rhs));
}
// BINARY DIVISION
template<unsigned nlimbs>
inline efloat<nlimbs> operator/(const efloat<nlimbs>& lhs, double rhs) {
	return operator/(lhs, efloat<nlimbs>(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - efloat binary arithmetic operators
// BINARY ADDITION
template<unsigned nlimbs>
inline efloat<nlimbs> operator+(double lhs, const efloat<nlimbs>& rhs) {
	return operator+(efloat<nlimbs>(lhs), rhs);
}
// BINARY SUBTRACTION
template<unsigned nlimbs>
inline efloat<nlimbs> operator-(double lhs, const efloat<nlimbs>& rhs) {
	return operator-(efloat<nlimbs>(lhs), rhs);
}
// BINARY MULTIPLICATION
template<unsigned nlimbs>
inline efloat<nlimbs> operator*(double lhs, const efloat<nlimbs>& rhs) {
	return operator*(efloat<nlimbs>(lhs), rhs);
}
// BINARY DIVISION
template<unsigned nlimbs>
inline efloat<nlimbs> operator/(double lhs, const efloat<nlimbs>& rhs) {
	return operator/(efloat<nlimbs>(lhs), rhs);
}

}} // namespace sw::universal
