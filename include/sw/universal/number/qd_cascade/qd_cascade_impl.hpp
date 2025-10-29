#pragma once
// qd_cascade_impl.hpp: implementation of quad-double using floatcascade<4>
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
inline bool signbit(const class qd_cascade&);
inline qd_cascade operator-(const qd_cascade&, const qd_cascade&);
inline qd_cascade operator*(const qd_cascade&, const qd_cascade&);
inline qd_cascade pow(const qd_cascade&, const qd_cascade&);
inline qd_cascade frexp(const qd_cascade&, int*);
inline qd_cascade ldexp(const qd_cascade&, int);
inline bool parse(const std::string&, qd_cascade&);

// Quad-Double (qd_cascade) number system using floatcascade<4>
//
// This is a modernized implementation using the floatcascade framework.
// It provides the same functionality as the classic qd type but with:
// - Unified implementation with dd_cascade/td via floatcascade
// - Fortified error-free transformations with volatile modifiers
// - Compatible API with classic qd (component accessors)
//
// TODO: Port sophisticated features from classic qd:
// - Full to_string() with formatting support
// - Robust parse() for decimal strings
// - Advanced mathematical functions (sqrt, exp, log, trig)
// - Optimized special cases
class qd_cascade {
private:
    floatcascade<4> cascade;

public:
    static constexpr unsigned nbits = 256;
    static constexpr unsigned es = 11;
    static constexpr unsigned fbits = 212; // number of fraction digits (4 * 53)
    // exponent characteristics are the same as native double precision floating-point
    static constexpr int      EXP_BIAS = ((1 << (es - 1u)) - 1l);
    static constexpr int      MAX_EXP = (es == 1) ? 1 : ((1 << es) - EXP_BIAS - 1);
    static constexpr int      MIN_EXP_NORMAL = 1 - EXP_BIAS;
    static constexpr int      MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP

    // Constructors

    /// trivial constructor
    qd_cascade() = default;

    qd_cascade(const qd_cascade&) = default;
    qd_cascade(qd_cascade&&) = default;

    // decorated constructors
    explicit constexpr qd_cascade(const floatcascade<4>& fc) : cascade(fc) {}

    // converting constructors
    qd_cascade(const std::string& stringRep) : cascade{} { assign(stringRep); }

    // specific value constructor
    constexpr qd_cascade(const SpecificValue code) noexcept : cascade{} {
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
        case SpecificValue::nar: // approximation as qds don't have a NaR
        case SpecificValue::qnan:
            setnan(NAN_TYPE_QUIET);
            break;
        case SpecificValue::snan:
            setnan(NAN_TYPE_SIGNALLING);
            break;
        }
    }

    // raw limb constructor: no argument checking, arguments need to be properly aligned
    constexpr qd_cascade(float h)                                  noexcept : cascade{} { cascade[0] = h; }
    constexpr qd_cascade(double h)                                 noexcept : cascade{} { cascade[0] = h; }
    constexpr qd_cascade(double h, double mh, double ml, double l) noexcept : cascade{} {
        cascade[0] = h;
        cascade[1] = mh;
        cascade[2] = ml;
        cascade[3] = l;
    }

    // initializers for native types
    constexpr qd_cascade(signed char iv)         noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr qd_cascade(short iv)               noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr qd_cascade(int iv)                 noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr qd_cascade(long iv)                noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr qd_cascade(long long iv)           noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr qd_cascade(char iv)                noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr qd_cascade(unsigned short iv)      noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr qd_cascade(unsigned int iv)        noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr qd_cascade(unsigned long iv)       noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr qd_cascade(unsigned long long iv)  noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }

    // assignment operators for native types
    constexpr qd_cascade& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
    constexpr qd_cascade& operator=(short rhs)              noexcept { return convert_signed(rhs); }
    constexpr qd_cascade& operator=(int rhs)                noexcept { return convert_signed(rhs); }
    constexpr qd_cascade& operator=(long rhs)               noexcept { return convert_signed(rhs); }
    constexpr qd_cascade& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
    constexpr qd_cascade& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
    constexpr qd_cascade& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
    constexpr qd_cascade& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
    constexpr qd_cascade& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
    constexpr qd_cascade& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
    constexpr qd_cascade& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
    constexpr qd_cascade& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

    // conversion operators
    explicit operator int()                   const noexcept { return convert_to_signed<int>(); }
    explicit operator long()                  const noexcept { return convert_to_signed<long>(); }
    explicit operator long long()             const noexcept { return convert_to_signed<long long>(); }
    explicit operator unsigned int()          const noexcept { return convert_to_unsigned<unsigned int>(); }
    explicit operator unsigned long()         const noexcept { return convert_to_unsigned<unsigned long>(); }
    explicit operator unsigned long long()    const noexcept { return convert_to_unsigned<unsigned long long>(); }
    explicit operator float()                 const noexcept { return convert_to_ieee754<float>(); }
    explicit operator double()                const noexcept { return convert_to_ieee754<double>(); }

    qd_cascade& operator=(const qd_cascade&) = default;
    qd_cascade& operator=(qd_cascade&&) = default;

    // Assignment from floatcascade
    qd_cascade& operator=(const floatcascade<4>& fc) {
        cascade = fc;
        return *this;
    }

    // Extract floatcascade
    const floatcascade<4>& get_cascade() const { return cascade; }
    operator floatcascade<4>() const { return cascade; }

    // Arithmetic operations

	constexpr qd_cascade operator-() const noexcept {
		floatcascade<4> neg;
		neg[0] = -cascade[0];
		neg[1] = -cascade[1];
		neg[2] = -cascade[2];
		neg[3] = -cascade[3];
		return qd_cascade(neg);
	}

    // Compound assignment operators
    qd_cascade& operator+=(const qd_cascade& rhs) noexcept {
		auto result = expansion_ops::add_cascades(cascade, rhs.cascade);  // 8 components
		// Compress to 4 components using proven QD algorithm
		cascade = expansion_ops::compress_8to4(result);
        return *this;
    }

    qd_cascade& operator-=(const qd_cascade& rhs) noexcept {
		floatcascade<4> neg_rhs;
		neg_rhs[0] = -rhs.cascade[0];
		neg_rhs[1] = -rhs.cascade[1];
		neg_rhs[2] = -rhs.cascade[2];
		neg_rhs[3] = -rhs.cascade[3];

		auto result = expansion_ops::add_cascades(cascade, neg_rhs);  // 8 components
		// Compress to 4 components using proven QD algorithm
		cascade = expansion_ops::compress_8to4(result);
		return *this;
    }

    qd_cascade& operator*=(const qd_cascade& rhs) noexcept {
		*this = expansion_ops::multiply_cascades(cascade, rhs.cascade);
        return *this;
    }

    qd_cascade& operator/=(const qd_cascade& rhs) noexcept {
		if (isnan())
			return *this;
		if (rhs.isnan())
			return *this = rhs;
		if (rhs.iszero()) {
			if (iszero()) {
				*this = qd_cascade(SpecificValue::qnan);
			} else {
				*this = qd_cascade(sign() == rhs.sign() ? SpecificValue::infpos : SpecificValue::infneg);
			}
			return *this;
		}

		// Newton-Raphson division: compute reciprocal then multiply
		// For quad-double, we need more refinement iterations

		// Initial approximation q0 = a/b using highest component
		double q0 = cascade[0] / rhs.cascade[0];

		// Compute residual: *this - q0 * other
		qd_cascade q0_times_other = q0 * rhs;
		qd_cascade residual = *this - q0_times_other;

		// Refine: q1 = q0 + residual/other
		double q1 = residual.cascade[0] / rhs.cascade[0];
		qd_cascade q1_times_other = qd_cascade(q1) * rhs;
		residual = residual - q1_times_other;

		// Refine again: q2 = q1 + residual/other
		double q2 = residual.cascade[0] / rhs.cascade[0];
		qd_cascade q2_times_other = qd_cascade(q2) * rhs;
		residual = residual - q2_times_other;

		// Final refinement: q3
		double q3 = residual.cascade[0] / rhs.cascade[0];

		// Combine quotients
		floatcascade<4> result_cascade;
		result_cascade[0] = q0;
		result_cascade[1] = q1;
		result_cascade[2] = q2;
		result_cascade[3] = q3;

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
            cascade[2] = -cascade[2];
            cascade[3] = -cascade[3];
        }
    }
    constexpr void set(double c0, double c1, double c2, double c3) noexcept {
        cascade[0] = c0;
        cascade[1] = c1;
        cascade[2] = c2;
        cascade[3] = c3;
    }
	constexpr void setbit(unsigned index, bool b = true) noexcept {
		if (index < 64) {  // set bit in lowest limb
			sw::universal::setbit(cascade[3], index, b);
		} else if (index < 128) {  // set bit in mid-low limb
			sw::universal::setbit(cascade[2], index - 64, b);
		} else if (index < 192) {  // set bit in mid-high limb
			sw::universal::setbit(cascade[1], index - 128, b);
		} else if (index < 256) {  // set bit in highest limb
			sw::universal::setbit(cascade[0], index - 192, b);
		} else {
			// NOP if index out of bounds
		}
	}
	constexpr void setbits(uint64_t value) noexcept {
		cascade[0] = static_cast<double>(value);
		cascade[1] = 0.0;
		cascade[2] = 0.0;
		cascade[3] = 0.0;
	}

    // argument is not protected for speed
    double operator[](int index) const { return cascade[index]; }
    double& operator[](int index) { return cascade[index]; }

    // create specific number system values of interest
    constexpr qd_cascade& maxpos() noexcept {
        cascade[0] = 1.79769313486231570814527423731704357e+308;
        cascade[1] = 9.97920154767359795037289025843547926e+291;
        cascade[2] = 5.53956966280111259858119742279688267e+275;
        cascade[3] = 3.07507899888268538886654502482441665e+259;
        return *this;
    }
    constexpr qd_cascade& minpos() noexcept {
        cascade[0] = std::numeric_limits<double>::min();
        cascade[1] = 0.0;
        cascade[2] = 0.0;
        cascade[3] = 0.0;
        return *this;
    }
    constexpr qd_cascade& zero() noexcept {
        // the zero value
        clear();
        return *this;
    }
    constexpr qd_cascade& minneg() noexcept {
        cascade[0] = -std::numeric_limits<double>::min();
        cascade[1] = 0.0;
        cascade[2] = 0.0;
        cascade[3] = 0.0;
        return *this;
    }
    constexpr qd_cascade& maxneg() noexcept {
        cascade[0] = -1.79769313486231570814527423731704357e+308;
        cascade[1] = -9.97920154767359795037289025843547926e+291;
        cascade[2] = -5.53956966280111259858119742279688267e+275;
        cascade[3] = -3.07507899888268538886654502482441665e+259;
        return *this;
    }

    qd_cascade& assign(const std::string& txt) {
        qd_cascade v;
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

    constexpr bool sign()          const noexcept { return cascade.sign(); }
    constexpr int  scale()         const noexcept { return cascade.scale(); }
    constexpr int  exponent()      const noexcept { return cascade.scale(); }


protected:
    // HELPER methods

    constexpr qd_cascade& convert_signed(int64_t v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }

    constexpr qd_cascade& convert_unsigned(uint64_t v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }

    // no need to SFINAE this as it is an internal method that we ONLY call when we know the argument type is a native float
    constexpr qd_cascade& convert_ieee754(float v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }
    constexpr qd_cascade& convert_ieee754(double v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }

    // convert to native unsigned integer, use C++ conversion rules to cast down to float and double
    template<typename Unsigned>
    Unsigned convert_to_unsigned() const noexcept {
        int64_t h = static_cast<int64_t>(cascade[0]);
        int64_t l = static_cast<int64_t>(cascade[1]);
        return Unsigned(h + l);
    }

    // convert to native signed integer, use C++ conversion rules to cast down to float and double
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

    // Stream output - TODO: Port sophisticated formatting from classic qd
    friend std::ostream& operator<<(std::ostream& os, const qd_cascade& q) {
        os << "qd_cascade(" << q.cascade << ")";
        return os;
    }
};

////////////////////////  precomputed constants of note  /////////////////////////////////

// precomputed quad-double constants

constexpr qd_cascade qdc_max(1.79769313486231570815e+308, 9.97920154767359795037e+291, 9.97920154767359795037e+274, 9.97920154767359795037e+247);

constexpr double qdc_eps            = 4.93038065763132e-32;     // 2^-104
constexpr double qdc_min_normalized = 2.0041683600089728e-292;  // = 2^(-1022 + 53)

////////////////////////    helper functions   /////////////////////////////////

inline qd_cascade ulp(const qd_cascade& a) {
	int scaleOf = scale(a[0]);
	return ldexp(qd_cascade(1.0), scaleOf - 159);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// qd_cascade - qd_cascade binary arithmetic operators

inline qd_cascade operator+(const qd_cascade& lhs, const qd_cascade& rhs) {
	qd_cascade sum = lhs;
	sum += rhs;
	return sum;
}

inline qd_cascade operator-(const qd_cascade& lhs, const qd_cascade& rhs) {
	qd_cascade diff = lhs;
	diff -= rhs;
	return diff;
}

inline qd_cascade operator*(const qd_cascade& lhs, const qd_cascade& rhs) {
	qd_cascade mul = lhs;
	mul *= rhs;
	return mul;
}

inline qd_cascade operator/(const qd_cascade& lhs, const qd_cascade& rhs) {
	qd_cascade div = lhs;
	div /= rhs;
	return div;
}

// qd_cascade-double mixed operations
inline qd_cascade operator+(const qd_cascade& lhs, double rhs) { return operator+(lhs, qd_cascade(rhs)); }
inline qd_cascade operator-(const qd_cascade& lhs, double rhs) { return operator-(lhs, qd_cascade(rhs)); }
inline qd_cascade operator*(const qd_cascade& lhs, double rhs) { return operator*(lhs, qd_cascade(rhs)); }
inline qd_cascade operator/(const qd_cascade& lhs, double rhs) { return operator/(lhs, qd_cascade(rhs)); }

// double-qd_cascade mixed operations
inline qd_cascade operator+(double lhs, const qd_cascade& rhs) { return operator+(qd_cascade(lhs), rhs); }
inline qd_cascade operator-(double lhs, const qd_cascade& rhs) { return operator-(qd_cascade(lhs), rhs); }
inline qd_cascade operator*(double lhs, const qd_cascade& rhs) { return operator*(qd_cascade(lhs), rhs); }
inline qd_cascade operator/(double lhs, const qd_cascade& rhs) { return operator/(qd_cascade(lhs), rhs); }

// Comparison operators
inline bool operator==(const qd_cascade& lhs, const qd_cascade& rhs) {
    return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

inline bool operator!=(const qd_cascade& lhs, const qd_cascade& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(const qd_cascade& lhs, const qd_cascade& rhs) {
    if (lhs[0] < rhs[0]) return true;
    if (lhs[0] > rhs[0]) return false;
    if (lhs[1] < rhs[1]) return true;
    if (lhs[1] > rhs[1]) return false;
    if (lhs[2] < rhs[2]) return true;
    if (lhs[2] > rhs[2]) return false;
    return lhs[3] < rhs[3];
}

inline bool operator>(const qd_cascade& lhs, const qd_cascade& rhs) {
    return rhs < lhs;
}

inline bool operator<=(const qd_cascade& lhs, const qd_cascade& rhs) {
    return !(rhs < lhs);
}

inline bool operator>=(const qd_cascade& lhs, const qd_cascade& rhs) {
    return !(lhs < rhs);
}

// Comparison with double
inline bool operator==(const qd_cascade& lhs, double rhs) { return lhs == qd_cascade(rhs); }
inline bool operator!=(const qd_cascade& lhs, double rhs) { return lhs != qd_cascade(rhs); }
inline bool operator<(const qd_cascade& lhs, double rhs) { return lhs < qd_cascade(rhs); }
inline bool operator>(const qd_cascade& lhs, double rhs) { return lhs > qd_cascade(rhs); }
inline bool operator<=(const qd_cascade& lhs, double rhs) { return lhs <= qd_cascade(rhs); }
inline bool operator>=(const qd_cascade& lhs, double rhs) { return lhs >= qd_cascade(rhs); }

inline bool operator==(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) == rhs; }
inline bool operator!=(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) != rhs; }
inline bool operator<(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) < rhs; }
inline bool operator>(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) > rhs; }
inline bool operator<=(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) <= rhs; }
inline bool operator>=(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) >= rhs; }

// standard attribute function overloads

inline bool signbit(const qd_cascade& a) {
	return std::signbit(a[0]);
}

inline qd_cascade pow(const qd_cascade& base, const qd_cascade& exp) {
	// use double pow on the highest component as an approximation
	// TODO: Port more accurate implementation from classic qd
	return qd_cascade(std::pow(base[0], exp[0]));
}

inline qd_cascade reciprocal(const qd_cascade& a) {
	return qd_cascade(1.0) / a;
}

inline qd_cascade sqrt(qd_cascade a) {
	// use double sqrt on the highest component as an approximation
	// TODO: Port more accurate sqrt implementation from classic qd
	return qd_cascade(std::sqrt(a[0]));
}

// TODO: Port parse() function from classic qd for decimal string parsing
inline bool parse(const std::string& number, qd_cascade& value) {
	// Placeholder implementation - just use double parsing for now
	// TODO: Implement proper decimal string parsing with full qd_cascade precision
	try {
		double d = std::stod(number);
		value = qd_cascade(d);
		return true;
	}
	catch (...) {
		return false;
	}
}

} // namespace sw::universal
