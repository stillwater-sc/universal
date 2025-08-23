#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include <universal/internal/floatcascade/floatcascade.hpp>

namespace sw::universal {

// Triple-Double (td) number system using floatcascade<3>
class td {
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
    td() = default;

    td(const td&) = default;
    td(td&&) = default;

    // decorated constructors
    //explicit td(double x) : cascade(x) {}  // managed with the other native type constructors
    explicit constexpr td(const floatcascade<3>& fc) : cascade(fc) {}

#ifdef CONNECTED_FLOATCASCADE_TYPES
    // Constructor from dd (zero-extends to 3 components)
    explicit constexpr td(const dd& d) : cascade() {
        floatcascade<2> dd_cascade = d.get_cascade();
        cascade[0] = dd_cascade[0];
        cascade[1] = dd_cascade[1];
        cascade[2] = 0.0;
    }
#else
    explicit constexpr td(const floatcascade<2>& dd_cascade) : cascade() {
        cascade[0] = dd_cascade[0];
        cascade[1] = dd_cascade[1];
        cascade[2] = 0.0;
    }
#endif

    // converting constructors
    td(const std::string& stringRep) : cascade{} { assign(stringRep); }

    // specific value constructor
    constexpr td(const SpecificValue code) noexcept : cascade{} {
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
    constexpr td(float h)                           noexcept : cascade{} { cascade[0] = h; }
    constexpr td(double h)                          noexcept : cascade{} { cascade[0] = h; }
    constexpr td(double h, double m)                noexcept : cascade{} { cascade[0] = h; cascade[1] = m; }
    constexpr td(double h, double m, double l)      noexcept : cascade{} { cascade[0] = h; cascade[1] = m; cascade[2] = l; }

    // initializers for native types
    constexpr td(signed char iv)                    noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td(short iv)                          noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td(int iv)                            noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td(long iv)                           noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td(long long iv)                      noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td(char iv)                           noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td(unsigned short iv)                 noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td(unsigned int iv)                   noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td(unsigned long iv)                  noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }
    constexpr td(unsigned long long iv)             noexcept : cascade{} { cascade[0] = static_cast<double>(iv); }

    // assignment operators for native types
    constexpr td& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
    constexpr td& operator=(short rhs)              noexcept { return convert_signed(rhs); }
    constexpr td& operator=(int rhs)                noexcept { return convert_signed(rhs); }
    constexpr td& operator=(long rhs)               noexcept { return convert_signed(rhs); }
    constexpr td& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
    constexpr td& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
    constexpr td& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
    constexpr td& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
    constexpr td& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
    constexpr td& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
    constexpr td& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
    constexpr td& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

    // conversion operators
    explicit operator int()                   const noexcept { return convert_to_signed<int>(); }
    explicit operator long()                  const noexcept { return convert_to_signed<long>(); }
    explicit operator long long()             const noexcept { return convert_to_signed<long long>(); }
    explicit operator unsigned int()          const noexcept { return convert_to_unsigned<unsigned int>(); }
    explicit operator unsigned long()         const noexcept { return convert_to_unsigned<unsigned long>(); }
    explicit operator unsigned long long()    const noexcept { return convert_to_unsigned<unsigned long long>(); }
    explicit operator float()                 const noexcept { return convert_to_ieee754<float>(); }
    explicit operator double()                const noexcept { return convert_to_ieee754<double>(); }

    td& operator=(const td&) = default;
    td& operator=(td&&) = default;

    // Assignment from floatcascade
    td& operator=(const floatcascade<3>& fc) {
        cascade = fc;
        return *this;
    }
    
    // Extract floatcascade
    const floatcascade<3>& get_cascade() const { return cascade; }
    operator floatcascade<3>() const { return cascade; }

#ifdef CONNECTED_FLOATCASCADE_TYPES 
    // Assignment from dd
    td& operator=(const dd& other) {
        floatcascade<2> other_cascade = other.get_cascade();
        cascade[0] = other_cascade[0];
        cascade[1] = other_cascade[1];
        cascade[2] = 0.0;
        return *this;
    }
#else
    td& operator=(const floatcascade<2>& dd_cascade) {
        cascade[0] = dd_cascade[0];
        cascade[1] = dd_cascade[1];
        cascade[2] = 0.0;
        return *this;
    }
#endif

    // Arithmetic operations
    td operator+(const td& other) const {
        auto result = expansion_ops::add_cascades(cascade, other.cascade);
        // Compress to 3 components
        floatcascade<3> compressed;
        compressed[0] = result[0];
        compressed[1] = result[1];
        compressed[2] = result[2] + result[3] + result[4] + result[5]; // Simple compression
        return td(compressed);
    }

    td operator-(const td& other) const {
        floatcascade<3> neg_other;
        neg_other[0] = -other.cascade[0];
        neg_other[1] = -other.cascade[1];
        neg_other[2] = -other.cascade[2];
        
        auto result = expansion_ops::add_cascades(cascade, neg_other);
        floatcascade<3> compressed;
        compressed[0] = result[0];
        compressed[1] = result[1];
        compressed[2] = result[2] + result[3] + result[4] + result[5];
        return td(compressed);
    }

    constexpr td operator-() const {
        floatcascade<3> neg;
        neg[0] = -cascade[0];
        neg[1] = -cascade[1];
        neg[2] = -cascade[2];
        return td(neg);
    }

    // modifiers
    constexpr void clear()                                         noexcept { cascade.clear(); }
    constexpr void setzero()                                       noexcept { cascade.clear(); }
    constexpr void setinf(bool sign = true)                        noexcept { cascade.clear(); cascade[0] = (sign ? -INFINITY : INFINITY); }
    constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING)       noexcept { cascade.clear(); cascade[0] = (NaNType == NAN_TYPE_SIGNALLING ? std::numeric_limits<double>::signaling_NaN() : std::numeric_limits<double>::quiet_NaN()); }
    constexpr void setsign(bool sign = true)                       noexcept { if (sign && cascade[0] > 0.0) cascade[0] = -cascade[0]; }
    constexpr void set(double high, double mid, double low)        noexcept { cascade[0] = high, cascade[1] = mid, cascade[2] = low; }

    // argument is not protected for speed
    double operator[](int index) const { return cascade[index]; }
    double& operator[](int index) { return cascade[index]; }

    // create specific number system values of interest
    constexpr td& maxpos() noexcept {
        cascade[0] = 1.7976931348623157e+308;
        cascade[1] = 1.9958403095347196e+292;
        cascade[2] = 1.9958403095347196e+292;
        return *this;
    }
    constexpr td& minpos() noexcept {
        cascade[0] = std::numeric_limits<double>::min();
        cascade[1] = cascade[2] = 0.0f;
        return *this;
    }
    constexpr td& zero() noexcept {
        // the zero value
        clear();
        return *this;
    }
    constexpr td& minneg() noexcept {
        cascade[0] = -std::numeric_limits<double>::min();
        cascade[1] = cascade[2] = 0.0f;
        return *this;
    }
    constexpr td& maxneg() noexcept {
        cascade[0] = -1.7976931348623157e+308;
        cascade[1] = -1.9958403095347196e+292;
        cascade[2] = -1.9958403095347196e+292;
        return *this;
    }

    td& assign(const std::string& txt) {
        td v;
        if (parse(txt, v)) *this = v;
        return *this; // Is this what we want? when the string is not valid, keep the current value?
    }

    // selectors
    constexpr bool iszero()   const noexcept { return cascade.iszero(); }
    constexpr bool isone()    const noexcept { return cascade[0] == 1.0 && cascade[1] == 0.0 && cascade[2] == 0.0; }
    constexpr bool ispos()    const noexcept { return cascade[0] > 0.0; }
    constexpr bool isneg()    const noexcept { return cascade[1] < 0.0; }
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

    constexpr bool sign()          const noexcept { return cascade.sign(); }
    constexpr int  scale()         const noexcept { return _extractExponent<std::uint64_t, double>(cascade[0]); }
    constexpr int  exponent()      const noexcept { return _extractExponent<std::uint64_t, double>(cascade[0]); }


protected:
    // HELPER methods

    constexpr td& convert_signed(int64_t v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }

    constexpr td& convert_unsigned(uint64_t v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }

    // no need to SFINAE this as it is an internal method that we ONLY call when we know the argument type is a native float
    constexpr td& convert_ieee754(float v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }
    constexpr td& convert_ieee754(double v) noexcept {
        cascade.set(static_cast<double>(v));
        return *this;
    }
#if LONG_DOUBLE_SUPPORT
    td& convert_ieee754(long double v) {
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

    // Stream output
    friend std::ostream& operator<<(std::ostream& os, const td& t) {
        os << "td(" << t.cascade << ")";
        return os;
    }
};

} // namespace sw::universal
