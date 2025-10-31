#pragma once
// td_cascade_impl.hpp: Triple-double cascade implementation using floatcascade<3>
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
inline bool signbit(const class td_cascade&);
inline td_cascade operator-(const td_cascade&, const td_cascade&);
inline td_cascade operator*(const td_cascade&, const td_cascade&);
inline td_cascade pow(const td_cascade&, const td_cascade&);
inline td_cascade frexp(const td_cascade&, int*);
inline td_cascade ldexp(const td_cascade&, int);
inline bool parse(const std::string&, td_cascade&);

// Triple-Double (td_cascade) number system using floatcascade<3>
class td_cascade {
private:
    floatcascade<3> cascade;

public:
    static constexpr unsigned nbits = 192;
    static constexpr unsigned es = 11;
    static constexpr unsigned fbits = 159; // number of fraction digits
    // exponent characteristics are the same as native double precision floating-point
    static constexpr int      EXP_BIAS = ((1 << (es - 1u)) - 1l);
    static constexpr int      MAX_EXP = (es == 1) ? 1 : ((1 << es) - EXP_BIAS - 1);
    static constexpr int      MIN_EXP_NORMAL = 1 - EXP_BIAS;
    static constexpr int      MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP

    // Constructors

    /// trivial constructor
    td_cascade() = default;

    td_cascade(const td_cascade&) = default;
    td_cascade(td_cascade&&) = default;

    // decorated constructors
    explicit constexpr td_cascade(const floatcascade<3>& fc) : cascade(fc) {}

    // Constructor from dd_cascade (zero-extends to 3 components)
    explicit constexpr td_cascade(const floatcascade<2>& dd_cascade) : cascade() {
        cascade[0] = dd_cascade[0];
        cascade[1] = dd_cascade[1];
        cascade[2] = 0.0;
    }

    // converting constructors
    td_cascade(const std::string& stringRep) : cascade{} { assign(stringRep); }

    // specific value constructor
    constexpr td_cascade(const SpecificValue code) noexcept : cascade{} {
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
        case SpecificValue::nar: // approximation as td_cascades don't have a NaR
        case SpecificValue::qnan:
            setnan(NAN_TYPE_QUIET);
            break;
        case SpecificValue::snan:
            setnan(NAN_TYPE_SIGNALLING);
            break;
        }
    }

    // raw limb constructor: no argument checking, arguments need to be properly aligned
    constexpr td_cascade(float h)                           noexcept : cascade{} { cascade[0] = h; }
    constexpr td_cascade(double h)                          noexcept : cascade{} { cascade[0] = h; }
    constexpr td_cascade(double h, double m)                noexcept : cascade{} { cascade[0] = h; cascade[1] = m; }
    constexpr td_cascade(double h, double m, double l)      noexcept : cascade{} { cascade[0] = h; cascade[1] = m; cascade[2] = l; }

    // initializers for native types
    constexpr td_cascade(signed char iv)                    noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td_cascade(short iv)                          noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td_cascade(int iv)                            noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td_cascade(long iv)                           noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td_cascade(long long iv)                      noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td_cascade(char iv)                           noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td_cascade(unsigned short iv)                 noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td_cascade(unsigned int iv)                   noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td_cascade(unsigned long iv)                  noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td_cascade(unsigned long long iv)             noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }

    // assignment operators for native types
    constexpr td_cascade& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
    constexpr td_cascade& operator=(short rhs)              noexcept { return convert_signed(rhs); }
    constexpr td_cascade& operator=(int rhs)                noexcept { return convert_signed(rhs); }
    constexpr td_cascade& operator=(long rhs)               noexcept { return convert_signed(rhs); }
    constexpr td_cascade& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
    constexpr td_cascade& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
    constexpr td_cascade& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
    constexpr td_cascade& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
    constexpr td_cascade& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
    constexpr td_cascade& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
    constexpr td_cascade& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
    constexpr td_cascade& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

    // conversion operators
    explicit operator int()                   const noexcept { return convert_to_signed<int>(); }
    explicit operator long()                  const noexcept { return convert_to_signed<long>(); }
    explicit operator long long()             const noexcept { return convert_to_signed<long long>(); }
    explicit operator unsigned int()          const noexcept { return convert_to_unsigned<unsigned int>(); }
    explicit operator unsigned long()         const noexcept { return convert_to_unsigned<unsigned long>(); }
    explicit operator unsigned long long()    const noexcept { return convert_to_unsigned<unsigned long long>(); }
    explicit operator float()                 const noexcept { return convert_to_ieee754<float>(); }
    explicit operator double()                const noexcept { return convert_to_ieee754<double>(); }

    td_cascade& operator=(const td_cascade&) = default;
    td_cascade& operator=(td_cascade&&) = default;

    // Assignment from floatcascade
    td_cascade& operator=(const floatcascade<3>& fc) {
        cascade = fc;
        return *this;
    }

    // Extract floatcascade
    const floatcascade<3>& get_cascade() const { return cascade; }
    operator floatcascade<3>() const { return cascade; }

    // Assignment from dd_cascade
    td_cascade& operator=(const floatcascade<2>& dd_cascade) {
        cascade[0] = dd_cascade[0];
        cascade[1] = dd_cascade[1];
        cascade[2] = 0.0;
        return *this;
    }

    // Arithmetic operations

    constexpr td_cascade operator-() const noexcept {
        floatcascade<3> neg;
        neg[0] = -cascade[0];
        neg[1] = -cascade[1];
        neg[2] = -cascade[2];
        return td_cascade(neg);
    }

    // Compound assignment operators
    td_cascade& operator+=(const td_cascade& rhs) noexcept {
        auto result = expansion_ops::add_cascades(cascade, rhs.cascade);  // 6 components
        // Compress to 3 components using proven QD algorithm
        cascade = expansion_ops::compress_6to3(result);
        return *this;
    }

    td_cascade& operator-=(const td_cascade& rhs) noexcept {
        floatcascade<3> neg_rhs;
        neg_rhs[0]   = -rhs.cascade[0];
        neg_rhs[1]   = -rhs.cascade[1];
        neg_rhs[2]   = -rhs.cascade[2];

        auto result = expansion_ops::add_cascades(cascade, neg_rhs);  // 6 components
        // Compress to 3 components using proven QD algorithm
        cascade = expansion_ops::compress_6to3(result);
        return *this;
    }

    td_cascade& operator*=(const td_cascade& rhs) noexcept {
        *this = expansion_ops::multiply_cascades(cascade, rhs.cascade);
        return *this;
    }

    td_cascade& operator/=(const td_cascade& rhs) noexcept {
        if (isnan())
            return *this;
        if (rhs.isnan())
            return *this = rhs;
        if (rhs.iszero()) {
            if (iszero()) {
                *this = td_cascade(SpecificValue::qnan);
            } else {
                *this = td_cascade(sign() == rhs.sign() ? SpecificValue::infpos : SpecificValue::infneg);
            }
            return *this;
        }

        // Newton-Raphson division: 3 refinement iterations for triple-double precision
        // x / y ~ x * (1/y) where 1/y is computed iteratively

        // Initial approximation q0 = a/b using highest component
        double q0 = cascade[0] / rhs.cascade[0];

        // Compute residual: *this - q0 * other
        td_cascade q0_times_other = q0 * rhs;
        td_cascade residual       = *this - q0_times_other;

        // Refine: q1 = q0 + residual/other
        double q1             = residual.cascade[0] / rhs.cascade[0];
        td_cascade q1_times_other = td_cascade(q1) * rhs;
        residual              = residual - q1_times_other;

        // Refine again: q2 = q1 + residual/other
        double q2 = residual.cascade[0] / rhs.cascade[0];

        // Combine quotients
        floatcascade<3> result_cascade;
        result_cascade[0] = q0;
        result_cascade[1] = q1;
        result_cascade[2] = q2;

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
        }
    }
    constexpr void set(double high, double mid, double low)        noexcept { cascade[0] = high, cascade[1] = mid, cascade[2] = low; }
    constexpr void setbit(unsigned index, bool b = true) noexcept {
        if (index < 64) {  // set bit in lower limb
            sw::universal::setbit(cascade[2], index, b);
        } else if (index < 128) {  // set bit in middle limb
            sw::universal::setbit(cascade[1], index - 64, b);
        } else if (index < 192) {  // set bit in upper limb
            sw::universal::setbit(cascade[0], index - 128, b);
        } else {
            // NOP if index out of bounds
        }
    }
    constexpr void setbits(uint64_t value) noexcept {
        cascade[0] = static_cast<double>(value);
        cascade[1] = 0.0;
        cascade[2] = 0.0;
    }

    // argument is not protected for speed
    double operator[](size_t index) const { return cascade[index]; }
    double& operator[](size_t index) { return cascade[index]; }

    // create specific number system values of interest
    constexpr td_cascade& maxpos() noexcept {
        cascade[0] = 1.7976931348623157e+308;
        cascade[1] = 1.9958403095347196e+292;
        cascade[2] = 1.9958403095347196e+292;
        return *this;
    }
    constexpr td_cascade& minpos() noexcept {
        cascade[0] = std::numeric_limits<double>::min();
        cascade[1] = cascade[2] = 0.0;
        return *this;
    }
    constexpr td_cascade& zero() noexcept {
        // the zero value
        clear();
        return *this;
    }
    constexpr td_cascade& minneg() noexcept {
        cascade[0] = -std::numeric_limits<double>::min();
        cascade[1] = cascade[2] = 0.0;
        return *this;
    }
    constexpr td_cascade& maxneg() noexcept {
        cascade[0] = -1.7976931348623157e+308;
        cascade[1] = -1.9958403095347196e+292;
        cascade[2] = -1.9958403095347196e+292;
        return *this;
    }

    td_cascade& assign(const std::string& txt) {
        td_cascade v;
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
        //return std::isfinite(hi); with C++23 std::isfinite is constexpr and can replace the code below
        return (!isnan() && !isinf());
    }

    constexpr int  sign()          const noexcept { return cascade.sign(); }
    constexpr int  scale()         const noexcept { return cascade.scale(); }
    constexpr int  exponent()      const noexcept { return cascade.scale(); }


protected:
    // HELPER methods

    constexpr td_cascade& convert_signed(int64_t v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }

    constexpr td_cascade& convert_unsigned(uint64_t v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }

    // no need to SFINAE this as it is an internal method that we ONLY call when we know the argument type is a native float
    constexpr td_cascade& convert_ieee754(float v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }
    constexpr td_cascade& convert_ieee754(double v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }
#if LONG_DOUBLE_SUPPORT
    td_cascade& convert_ieee754(long double v) {
        volatile long double truncated = static_cast<long double>(double(v));
        volatile double remainder = static_cast<double>(v - truncated);
        cascade[0] = static_cast<double>(truncated);
        cascade[1] = remainder;
        cascade[2] = 0.0;
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

public:
    // Decimal conversion - delegates to floatcascade base class
    std::string to_string(
        std::streamsize precision = 7,
        std::streamsize width = 15,
        bool fixed = false,
        bool scientific = true,
        bool internal = false,
        bool left = false,
        bool showpos = false,
        bool uppercase = false,
        char fill = ' '
    ) const {
        return cascade.to_string(precision, width, fixed, scientific, internal, left, showpos, uppercase, fill);
    }

private:
    // Stream output - uses to_string with formatting extraction
    friend std::ostream& operator<<(std::ostream& ostr, const td_cascade& v) {
        std::ios_base::fmtflags fmt = ostr.flags();
        std::streamsize precision = ostr.precision();
        std::streamsize width = ostr.width();
        char fillChar = ostr.fill();
        bool showpos = fmt & std::ios_base::showpos;
        bool uppercase = fmt & std::ios_base::uppercase;
        bool fixed = fmt & std::ios_base::fixed;
        bool scientific = fmt & std::ios_base::scientific;
        bool internal = fmt & std::ios_base::internal;
        bool left = fmt & std::ios_base::left;
        return ostr << v.to_string(precision, width, fixed, scientific, internal, left, showpos, uppercase, fillChar);
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// td_cascade - td_cascade binary arithmetic operators

inline td_cascade operator+(const td_cascade& lhs, const td_cascade& rhs) {
    td_cascade sum = lhs;
    sum += rhs;
    return sum;
}

inline td_cascade operator-(const td_cascade& lhs, const td_cascade& rhs) {
    td_cascade diff = lhs;
    diff -= rhs;
    return diff;
}

inline td_cascade operator*(const td_cascade& lhs, const td_cascade& rhs) {
    td_cascade mul = lhs;
    mul *= rhs;
    return mul;
}

inline td_cascade operator/(const td_cascade& lhs, const td_cascade& rhs) {
    td_cascade div = lhs;
    div /= rhs;
    return div;
}

// td_cascade-double mixed operations
inline td_cascade operator+(const td_cascade& lhs, double rhs) { return operator+(lhs, td_cascade(rhs)); }
inline td_cascade operator-(const td_cascade& lhs, double rhs) { return operator-(lhs, td_cascade(rhs)); }
inline td_cascade operator*(const td_cascade& lhs, double rhs) { return operator*(lhs, td_cascade(rhs)); }
inline td_cascade operator/(const td_cascade& lhs, double rhs) { return operator/(lhs, td_cascade(rhs)); }

// double-td_cascade mixed operations
inline td_cascade operator+(double lhs, const td_cascade& rhs) { return operator+(td_cascade(lhs), rhs); }
inline td_cascade operator-(double lhs, const td_cascade& rhs) { return operator-(td_cascade(lhs), rhs); }
inline td_cascade operator*(double lhs, const td_cascade& rhs) { return operator*(td_cascade(lhs), rhs); }
inline td_cascade operator/(double lhs, const td_cascade& rhs) { return operator/(td_cascade(lhs), rhs); }

// Comparison operators
inline bool operator==(const td_cascade& lhs, const td_cascade& rhs) {
    return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2];
}

inline bool operator!=(const td_cascade& lhs, const td_cascade& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(const td_cascade& lhs, const td_cascade& rhs) {
    if (lhs[0] < rhs[0]) return true;
    if (lhs[0] > rhs[0]) return false;
    if (lhs[1] < rhs[1]) return true;
    if (lhs[1] > rhs[1]) return false;
    return lhs[2] < rhs[2];
}

inline bool operator>(const td_cascade& lhs, const td_cascade& rhs) {
    return rhs < lhs;
}

inline bool operator<=(const td_cascade& lhs, const td_cascade& rhs) {
    return !(rhs < lhs);
}

inline bool operator>=(const td_cascade& lhs, const td_cascade& rhs) {
    return !(lhs < rhs);
}

// Comparison with double
inline bool operator==(const td_cascade& lhs, double rhs) { return lhs == td_cascade(rhs); }
inline bool operator!=(const td_cascade& lhs, double rhs) { return lhs != td_cascade(rhs); }
inline bool operator<(const td_cascade& lhs, double rhs) { return lhs < td_cascade(rhs); }
inline bool operator>(const td_cascade& lhs, double rhs) { return lhs > td_cascade(rhs); }
inline bool operator<=(const td_cascade& lhs, double rhs) { return lhs <= td_cascade(rhs); }
inline bool operator>=(const td_cascade& lhs, double rhs) { return lhs >= td_cascade(rhs); }

inline bool operator==(double lhs, const td_cascade& rhs) { return td_cascade(lhs) == rhs; }
inline bool operator!=(double lhs, const td_cascade& rhs) { return td_cascade(lhs) != rhs; }
inline bool operator<(double lhs, const td_cascade& rhs) { return td_cascade(lhs) < rhs; }
inline bool operator>(double lhs, const td_cascade& rhs) { return td_cascade(lhs) > rhs; }
inline bool operator<=(double lhs, const td_cascade& rhs) { return td_cascade(lhs) <= rhs; }
inline bool operator>=(double lhs, const td_cascade& rhs) { return td_cascade(lhs) >= rhs; }


// standard attribute function overloads

inline bool signbit(const td_cascade& a) {
    return std::signbit(a[0]);
}

inline td_cascade pow(const td_cascade& base, const td_cascade& exp) {
    // use double pow on the highest component as an approximation
    return td_cascade(std::pow(base[0], exp[0]));
}

inline td_cascade reciprocal(const td_cascade& a) {
    return td_cascade(1.0) / a;
}

inline td_cascade sqrt(td_cascade a) {
    // use double sqrt on the highest component as an approximation
    return td_cascade(std::sqrt(a[0]));
}

// Decimal string parsing - delegates to floatcascade base class for full precision
inline bool parse(const std::string& number, td_cascade& value) {
    // Delegates to floatcascade base class for full precision parsing
    floatcascade<3> temp_cascade;
    if (temp_cascade.parse(number)) {
        value = td_cascade(temp_cascade);
        return true;
    }
    return false;
}

} // namespace sw::universal
