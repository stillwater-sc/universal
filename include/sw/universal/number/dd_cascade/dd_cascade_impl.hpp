#pragma once
// dd_cascade_impl.hpp: implementation of double-double using floatcascade<2>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include <universal/internal/floatcascade/floatcascade.hpp>

namespace sw::universal {

    // Forward declarations
inline bool signbit(const class dd_cascade&);
inline dd_cascade operator-(const dd_cascade&, const dd_cascade&);
inline dd_cascade operator*(const dd_cascade&, const dd_cascade&);
inline dd_cascade pow(const dd_cascade&, const dd_cascade&);
inline bool parse(const std::string&, dd_cascade&);

// Double-Double (dd_cascade) number system using floatcascade<2>
//
// This is a modernized implementation using the floatcascade framework.
// It provides the same functionality as the classic dd type but with:
// - Unified implementation with td/qd via floatcascade
// - Fortified error-free transformations with volatile modifiers
// - Compatible API with classic dd (high(), low() accessors)
//
// TODO: Port sophisticated features from classic dd:
// - Full to_string() with formatting support
// - Robust parse() for decimal strings
// - Advanced mathematical functions (sqrt, exp, log, trig)
// - Optimized special cases
class dd_cascade {
private:
    floatcascade<2> cascade;

public:
    static constexpr unsigned nbits = 128;
    static constexpr unsigned es = 11;
    static constexpr unsigned fbits = 106; // number of fraction digits
    // exponent characteristics are the same as native double precision floating-point
    static constexpr int      EXP_BIAS = ((1 << (es - 1u)) - 1l);
    static constexpr int      MAX_EXP = (es == 1) ? 1 : ((1 << es) - EXP_BIAS - 1);
    static constexpr int      MIN_EXP_NORMAL = 1 - EXP_BIAS;
    static constexpr int      MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP

    // Constructors

    /// trivial constructor
    dd_cascade() = default;

    dd_cascade(const dd_cascade&) = default;
    dd_cascade(dd_cascade&&) = default;

    // decorated constructors
    explicit constexpr dd_cascade(const floatcascade<2>& fc) : cascade(fc) {}

    // converting constructors
    dd_cascade(const std::string& stringRep) : cascade{} { assign(stringRep); }

    // specific value constructor
    constexpr dd_cascade(const SpecificValue code) noexcept : cascade{} {
        switch (code) {
        case SpecificValue::maxpos:
            maxpos();
            break;
        case SpecificValue::minpos:
            minpos();
            break;
        case SpecificValue::zero:
        default:
            zero();
            break;
        case SpecificValue::minneg:
            minneg();
            break;
        case SpecificValue::maxneg:
            maxneg();
            break;
        case SpecificValue::infpos:
            setinf(false);
            break;
        case SpecificValue::infneg:
            setinf(true);
            break;
        case SpecificValue::nar: // approximation as dds don't have a NaR
        case SpecificValue::qnan:
            setnan(NAN_TYPE_QUIET);
            break;
        case SpecificValue::snan:
            setnan(NAN_TYPE_SIGNALLING);
            break;
        }
    }

    // raw limb constructor: no argument checking, arguments need to be properly aligned
    constexpr dd_cascade(float h)                noexcept : cascade{} { cascade[0] = h; }
    constexpr dd_cascade(double h)               noexcept : cascade{} { cascade[0] = h; }
    constexpr dd_cascade(double h, double l)     noexcept : cascade{} { cascade[0] = h; cascade[1] = l; }

    // initializers for native types
    constexpr dd_cascade(signed char iv)         noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr dd_cascade(short iv)               noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr dd_cascade(int iv)                 noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr dd_cascade(long iv)                noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr dd_cascade(long long iv)           noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr dd_cascade(char iv)                noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr dd_cascade(unsigned short iv)      noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr dd_cascade(unsigned int iv)        noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr dd_cascade(unsigned long iv)       noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr dd_cascade(unsigned long long iv)  noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }

    // assignment operators for native types
    constexpr dd_cascade& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
    constexpr dd_cascade& operator=(short rhs)              noexcept { return convert_signed(rhs); }
    constexpr dd_cascade& operator=(int rhs)                noexcept { return convert_signed(rhs); }
    constexpr dd_cascade& operator=(long rhs)               noexcept { return convert_signed(rhs); }
    constexpr dd_cascade& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
    constexpr dd_cascade& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
    constexpr dd_cascade& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
    constexpr dd_cascade& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
    constexpr dd_cascade& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
    constexpr dd_cascade& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
    constexpr dd_cascade& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
    constexpr dd_cascade& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

    // conversion operators
    explicit operator int()                   const noexcept { return convert_to_signed<int>(); }
    explicit operator long()                  const noexcept { return convert_to_signed<long>(); }
    explicit operator long long()             const noexcept { return convert_to_signed<long long>(); }
    explicit operator unsigned int()          const noexcept { return convert_to_unsigned<unsigned int>(); }
    explicit operator unsigned long()         const noexcept { return convert_to_unsigned<unsigned long>(); }
    explicit operator unsigned long long()    const noexcept { return convert_to_unsigned<unsigned long long>(); }
    explicit operator float()                 const noexcept { return convert_to_ieee754<float>(); }
    explicit operator double()                const noexcept { return convert_to_ieee754<double>(); }

    dd_cascade& operator=(const dd_cascade&) = default;
    dd_cascade& operator=(dd_cascade&&) = default;

    // Assignment from floatcascade
    dd_cascade& operator=(const floatcascade<2>& fc) {
        cascade = fc;
        return *this;
    }

    // Extract floatcascade
    const floatcascade<2>& get_cascade() const { return cascade; }
    operator floatcascade<2>() const { return cascade; }

    // Classic dd API compatibility: high() and low() accessors
    double high() const { return cascade[0]; }
    double low() const { return cascade[1]; }
    double& high() { return cascade[0]; }
    double& low() { return cascade[1]; }

    // Arithmetic operations

	constexpr dd_cascade operator-() const noexcept {
		floatcascade<2> neg;
		neg[0] = -cascade[0];
		neg[1] = -cascade[1];
		return dd_cascade(neg);
	}

    // Compound assignment operators
    dd_cascade& operator+=(const dd_cascade& rhs) noexcept {
		auto result = expansion_ops::add_cascades(cascade, rhs.cascade);  // 4 components
		// Compress to 2 components using proven QD algorithm
		cascade = expansion_ops::compress_4to2(result);
        return *this;
    }

    dd_cascade& operator-=(const dd_cascade& rhs) noexcept {
		floatcascade<2> neg_rhs;
		neg_rhs[0] = -rhs.cascade[0];
		neg_rhs[1] = -rhs.cascade[1];

		auto result = expansion_ops::add_cascades(cascade, neg_rhs);  // 4 components
		// Compress to 2 components using proven QD algorithm
		cascade = expansion_ops::compress_4to2(result);
		return *this;
    }

    dd_cascade& operator*=(const dd_cascade& rhs) noexcept {
		*this = expansion_ops::multiply_cascades(cascade, rhs.cascade);
        return *this;
    }

    dd_cascade& operator/=(const dd_cascade& rhs) noexcept {
		if (isnan())
			return *this;
		if (rhs.isnan())
			return *this = rhs;
		if (rhs.iszero()) {
			if (iszero()) {
				*this = dd_cascade(SpecificValue::qnan);
			} else {
				*this = dd_cascade(sign() == rhs.sign() ? SpecificValue::infpos : SpecificValue::infneg);
			}
			return *this;
		}

		// Newton-Raphson division: compute reciprocal then multiply
		// x / y ~ x * (1/y) where 1/y is computed iteratively

		// Initial approximation q0 = a/b using highest component
		double q0 = cascade[0] / rhs.cascade[0];

		// Compute residual: *this - q0 * other
		dd_cascade q0_times_other = q0 * rhs;
		dd_cascade residual       = *this - q0_times_other;

		// Refine: q1 = q0 + residual/other
		double q1 = residual.cascade[0] / rhs.cascade[0];

		// Combine quotients
		floatcascade<2> result_cascade;
		result_cascade[0] = q0;
		result_cascade[1] = q1;

		*this = expansion_ops::renormalize(result_cascade);
        return *this;
    }

    // modifiers
    constexpr void clear()                                         noexcept { cascade.clear(); }
    constexpr void setzero()                                       noexcept { cascade.clear(); }
    constexpr void setinf(bool sign = true)                        noexcept { cascade.clear(); cascade[0] = (sign ? -INFINITY : INFINITY); }
    constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING)       noexcept { cascade.clear(); cascade[0] = (NaNType == NAN_TYPE_SIGNALLING ? std::numeric_limits<double>::signaling_NaN() : std::numeric_limits<double>::quiet_NaN()); }
    constexpr void setsign(bool sign = true)                       noexcept {
        if (sign && cascade[0] > 0.0) {
            cascade[0] = -cascade[0];
            cascade[1] = -cascade[1];
        }
    }
    constexpr void set(double high, double low)                    noexcept { cascade[0] = high; cascade[1] = low; }
	constexpr void setbit(unsigned index, bool b = true) noexcept {
		if (index < 64) {  // set bit in lower limb
			sw::universal::setbit(cascade[1], index, b);
		} else if (index < 128) {  // set bit in upper limb
			sw::universal::setbit(cascade[0], index - 64, b);
		} else {
			// NOP if index out of bounds
		}
	}
	constexpr void setbits(uint64_t value) noexcept {
		cascade[0] = static_cast<double>(value);
		cascade[1] = 0.0;
	}

    // argument is not protected for speed
    double operator[](int index) const { return cascade[index]; }
    double& operator[](int index) { return cascade[index]; }

    // create specific number system values of interest
    constexpr dd_cascade& maxpos() noexcept {
        cascade[0] = 1.7976931348623157e+308;
        cascade[1] = 9.9792015476735972e+291;
        return *this;
    }
    constexpr dd_cascade& minpos() noexcept {
        cascade[0] = std::numeric_limits<double>::min();
        cascade[1] = 0.0;
        return *this;
    }
    constexpr dd_cascade& zero() noexcept {
        // the zero value
        clear();
        return *this;
    }
    constexpr dd_cascade& minneg() noexcept {
        cascade[0] = -std::numeric_limits<double>::min();
        cascade[1] = 0.0;
        return *this;
    }
    constexpr dd_cascade& maxneg() noexcept {
        cascade[0] = -1.7976931348623157e+308;
        cascade[1] = -9.9792015476735972e+291;
        return *this;
    }

    dd_cascade& assign(const std::string& txt) {
        dd_cascade v;
        if (parse(txt, v)) *this = v;
        return *this; // Is this what we want? when the string is not valid, keep the current value?
    }

    // selectors
    constexpr bool iszero()   const noexcept { return cascade.iszero(); }
    constexpr bool isone()    const noexcept { return cascade.isone(); }
    constexpr bool ispos()    const noexcept { return cascade.ispos(); }
    constexpr bool isneg()    const noexcept { return cascade.isneg(); }
    BIT_CAST_CONSTEXPR bool isnan(int NaNType = NAN_TYPE_EITHER)  const noexcept {
        bool negative = isneg();
        int nan_type;
        bool isNaN = checkNaN(cascade[0], nan_type);
        bool isNegNaN = isNaN && negative;
        bool isPosNaN = isNaN && !negative;
        return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) :
            (NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
                (NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
    }
    BIT_CAST_CONSTEXPR bool isinf(int InfType = INF_TYPE_EITHER)  const noexcept {
        bool negative = isneg();
        int inf_type;
        bool isInf = checkInf(cascade[0], inf_type);
        bool isNegInf = isInf && negative;
        bool isPosInf = isInf && !negative;
        return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
            (InfType == INF_TYPE_NEGATIVE ? isNegInf :
                (InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
    }
    // normal, subnormal or zero, but not infinite or NaN:
    BIT_CAST_CONSTEXPR bool isfinite() const noexcept {
        return (!isnan() && !isinf());
    }

    constexpr bool sign()     const noexcept { return cascade.sign(); }
    constexpr int  scale()    const noexcept { return cascade.scale(); }
	constexpr int  exponent() const noexcept { return cascade.scale(); }  // alias for scale()

protected:
    // HELPER methods

    constexpr dd_cascade& convert_signed(int64_t v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }

    constexpr dd_cascade& convert_unsigned(uint64_t v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }

    // no need to SFINAE this as it is an internal method that we ONLY call when we know the argument type is a native float
    constexpr dd_cascade& convert_ieee754(float v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }
    constexpr dd_cascade& convert_ieee754(double v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }
#if LONG_DOUBLE_SUPPORT
    dd_cascade& convert_ieee754(long double v) {
        volatile long double truncated = static_cast<long double>(double(v));
        volatile double remainder = static_cast<double>(v - truncated);
        cascade[0] = static_cast<double>(truncated);
        cascade[1] = remainder;
        return *this;
    }
#endif

    // convert to native unsigned integer, use C++ conversion rules to cast down to float and double
    template<typename Unsigned>
    Unsigned convert_to_unsigned() const noexcept {
        int64_t h = static_cast<int64_t>(cascade[0]);
        int64_t l = static_cast<int64_t>(cascade[1]);
        return Unsigned(h + l);
    }

    // convert to native unsigned integer, use C++ conversion rules to cast down to float and double
    template<typename Signed>
    Signed convert_to_signed() const noexcept {
        int64_t h = static_cast<int64_t>(cascade[0]);
        int64_t l = static_cast<int64_t>(cascade[1]);
        return Signed(h + l);
    }

    // convert to native floating-point, use C++ conversion rules to cast down to float and double
    template<typename Real>
    Real convert_to_ieee754() const noexcept {
        return Real(cascade.to_double());
    }

    // Stream output - TODO: Port sophisticated formatting from classic dd
    friend std::ostream& operator<<(std::ostream& os, const dd_cascade& d) {
        os << "dd_cascade(" << d.cascade << ")";
        return os;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd_cascade - dd_cascade binary arithmetic operators

inline dd_cascade operator+(const dd_cascade& lhs, const dd_cascade& rhs) {
	dd_cascade sum = lhs;
	sum += rhs;
	return sum;
}

inline dd_cascade operator-(const dd_cascade& lhs, const dd_cascade& rhs) {
	dd_cascade diff = lhs;
	diff -= rhs;
	return diff;
}

inline dd_cascade operator*(const dd_cascade& lhs, const dd_cascade& rhs) {
	dd_cascade mul = lhs;
	mul *= rhs;
	return mul;
}

inline dd_cascade operator/(const dd_cascade& lhs, const dd_cascade& rhs) {
	dd_cascade div = lhs;
	div /= rhs;
	return div;
}

// dd_cascade-double mixed operations
inline dd_cascade operator+(const dd_cascade& lhs, double rhs) { return operator+(lhs, dd_cascade(rhs)); }
inline dd_cascade operator-(const dd_cascade& lhs, double rhs) { return operator-(lhs, dd_cascade(rhs)); }
inline dd_cascade operator*(const dd_cascade& lhs, double rhs) { return operator*(lhs, dd_cascade(rhs)); }
inline dd_cascade operator/(const dd_cascade& lhs, double rhs) { return operator/(lhs, dd_cascade(rhs)); }

// double-dd_cascade mixed operations
inline dd_cascade operator+(double lhs, const dd_cascade& rhs) { return operator+(dd_cascade(lhs), rhs); }
inline dd_cascade operator-(double lhs, const dd_cascade& rhs) { return operator-(dd_cascade(lhs), rhs); }
inline dd_cascade operator*(double lhs, const dd_cascade& rhs) { return operator*(dd_cascade(lhs), rhs); }
inline dd_cascade operator/(double lhs, const dd_cascade& rhs) { return operator/(dd_cascade(lhs), rhs); }

// Comparison operators
inline bool operator==(const dd_cascade& lhs, const dd_cascade& rhs) {
    return lhs[0] == rhs[0] && lhs[1] == rhs[1];
}

inline bool operator!=(const dd_cascade& lhs, const dd_cascade& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(const dd_cascade& lhs, const dd_cascade& rhs) {
    if (lhs[0] < rhs[0]) return true;
    if (lhs[0] > rhs[0]) return false;
    return lhs[1] < rhs[1];
}

inline bool operator>(const dd_cascade& lhs, const dd_cascade& rhs) {
    return rhs < lhs;
}

inline bool operator<=(const dd_cascade& lhs, const dd_cascade& rhs) {
    return !(rhs < lhs);
}

inline bool operator>=(const dd_cascade& lhs, const dd_cascade& rhs) {
    return !(lhs < rhs);
}

// Comparison with double
inline bool operator==(const dd_cascade& lhs, double rhs) { return lhs == dd_cascade(rhs); }
inline bool operator!=(const dd_cascade& lhs, double rhs) { return lhs != dd_cascade(rhs); }
inline bool operator<(const dd_cascade& lhs, double rhs) { return lhs < dd_cascade(rhs); }
inline bool operator>(const dd_cascade& lhs, double rhs) { return lhs > dd_cascade(rhs); }
inline bool operator<=(const dd_cascade& lhs, double rhs) { return lhs <= dd_cascade(rhs); }
inline bool operator>=(const dd_cascade& lhs, double rhs) { return lhs >= dd_cascade(rhs); }

inline bool operator==(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) == rhs; }
inline bool operator!=(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) != rhs; }
inline bool operator<(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) < rhs; }
inline bool operator>(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) > rhs; }
inline bool operator<=(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) <= rhs; }
inline bool operator>=(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) >= rhs; }

// standard attribute function overloads

inline bool signbit(const dd_cascade& a) {
	return std::signbit(a[0]);
}

inline dd_cascade pow(const dd_cascade& base, const dd_cascade& exp) {
	// use double pow on the highest component as an approximation
	// TODO: Port more accurate implementation from classic dd
	return dd_cascade(std::pow(base[0], exp[0]));
}

inline dd_cascade reciprocal(const dd_cascade& a) {
	return dd_cascade(1.0) / a;
}

inline dd_cascade sqrt(dd_cascade a) {
	// use double sqrt on the highest component as an approximation
	// TODO: Port more accurate sqrt implementation from classic dd
	return dd_cascade(std::sqrt(a[0]));
}

// TODO: Port parse() function from classic dd for decimal string parsing
inline bool parse(const std::string& number, dd_cascade& value) {
	// Placeholder implementation - just use double parsing for now
	// TODO: Implement proper decimal string parsing with full dd_cascade precision
	try {
		double d = std::stod(number);
		value = dd_cascade(d);
		return true;
	}
	catch (...) {
		return false;
	}
}

} // namespace sw::universal
