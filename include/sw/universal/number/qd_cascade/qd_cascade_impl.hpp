#pragma once
// qd_cascade_impl.hpp: implementation of quad-double using floatcascade<4>
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

#include <array>
#include <bit>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <vector>
#include <universal/utility/bit_cast.hpp>
#include <universal/internal/floatcascade/floatcascade.hpp>

namespace sw::universal {

    // Forward declarations
inline bool signbit(const class qd_cascade&);
constexpr qd_cascade operator-(const qd_cascade&, const qd_cascade&);
constexpr qd_cascade operator*(const qd_cascade&, const qd_cascade&);
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

private:
    // Helper templates used by the conversion operators below.  Defined before
    // the operators so the constexpr call chain is fully resolved at the point
    // of operator declaration (clang requires this; gcc is more permissive).
    //
    // For quad-double, collapse the lower three limbs (mid_hi + mid_lo + lo)
    // into a single representative double via a two_sum chain (sub-ULP error
    // discarded -- cannot affect integer truncation), then apply the dd-style
    // limb-separated truncation algorithm from #797.
    template<typename Unsigned>
    constexpr Unsigned convert_to_unsigned_impl() const noexcept {
        const double hi = cascade[0];
        // Collapse the lower three limbs into a single representative double.
        double s1 = 0.0, e1 = 0.0;
        expansion_ops::two_sum(cascade[2], cascade[3], s1, e1);
        double lo_combined = 0.0, lo_err = 0.0;
        expansion_ops::two_sum(cascade[1], s1, lo_combined, lo_err);
        (void)e1; (void)lo_err;
        const double lo = lo_combined;

        bool hi_finite;
        if (std::is_constant_evaluated()) {
            constexpr double inf = std::numeric_limits<double>::infinity();
            hi_finite = !(hi != hi) && hi != inf && hi != -inf;
        }
        else {
            hi_finite = std::isfinite(hi);
        }
        if (!hi_finite) {
            return hi < 0.0 ? Unsigned(0) : (std::numeric_limits<Unsigned>::max)();
        }
        if (hi < 0.0) return Unsigned(0);

        constexpr double unsigned_max_d = static_cast<double>((std::numeric_limits<Unsigned>::max)());
        if (hi > unsigned_max_d) return (std::numeric_limits<Unsigned>::max)();
        if (hi == unsigned_max_d) {
            if (lo >= 0.0) return (std::numeric_limits<Unsigned>::max)();
            double abs_lo = -lo;
            if (abs_lo >= unsigned_max_d) return Unsigned(0);
            Unsigned abs_lo_int = static_cast<Unsigned>(abs_lo);
            double abs_lo_frac = abs_lo - static_cast<double>(abs_lo_int);
            Unsigned result = (std::numeric_limits<Unsigned>::max)();
            if (abs_lo_frac == 0.0) return result - (abs_lo_int - 1);
            return result - abs_lo_int;
        }

        Unsigned hi_int = static_cast<Unsigned>(hi);
        double hi_frac = hi - static_cast<double>(hi_int);

        if (lo >= 0.0) {
            if (lo >= unsigned_max_d) return (std::numeric_limits<Unsigned>::max)();
            Unsigned lo_int = static_cast<Unsigned>(lo);
            double lo_frac = lo - static_cast<double>(lo_int);
            double frac_sum = hi_frac + lo_frac;
            if (lo_int > (std::numeric_limits<Unsigned>::max)() - hi_int) {
                return (std::numeric_limits<Unsigned>::max)();
            }
            Unsigned sum = hi_int + lo_int;
            if (frac_sum >= 1.0) {
                if (sum == (std::numeric_limits<Unsigned>::max)()) return sum;
                return sum + 1;
            }
            return sum;
        }
        else {
            double abs_lo = -lo;
            if (abs_lo >= unsigned_max_d) return Unsigned(0);
            Unsigned abs_lo_int = static_cast<Unsigned>(abs_lo);
            double abs_lo_frac = abs_lo - static_cast<double>(abs_lo_int);
            if (hi_int < abs_lo_int) return Unsigned(0);
            if (hi_int == abs_lo_int) return Unsigned(0);
            Unsigned diff = hi_int - abs_lo_int;
            if (hi_frac < abs_lo_frac) return diff - 1;
            return diff;
        }
    }

    template<typename Signed>
    constexpr Signed convert_to_signed_impl() const noexcept {
        const double hi = cascade[0];
        double s1 = 0.0, e1 = 0.0;
        expansion_ops::two_sum(cascade[2], cascade[3], s1, e1);
        double lo_combined = 0.0, lo_err = 0.0;
        expansion_ops::two_sum(cascade[1], s1, lo_combined, lo_err);
        (void)e1; (void)lo_err;
        const double lo = lo_combined;

        bool hi_finite;
        if (std::is_constant_evaluated()) {
            constexpr double inf = std::numeric_limits<double>::infinity();
            hi_finite = !(hi != hi) && hi != inf && hi != -inf;
        }
        else {
            hi_finite = std::isfinite(hi);
        }
        if (!hi_finite) {
            return hi < 0.0 ? (std::numeric_limits<Signed>::min)() : (std::numeric_limits<Signed>::max)();
        }

        constexpr double signed_max_d = static_cast<double>((std::numeric_limits<Signed>::max)());
        constexpr double signed_min_d = static_cast<double>((std::numeric_limits<Signed>::min)());

        if (hi > signed_max_d) return (std::numeric_limits<Signed>::max)();
        if (hi == signed_max_d) {
            if (lo >= 0.0) return (std::numeric_limits<Signed>::max)();
            double abs_lo = -lo;
            if (abs_lo >= signed_max_d) return (std::numeric_limits<Signed>::min)();
            Signed abs_lo_int = static_cast<Signed>(abs_lo);
            double abs_lo_frac = abs_lo - static_cast<double>(abs_lo_int);
            Signed result = (std::numeric_limits<Signed>::max)();
            if (abs_lo_frac == 0.0) return result - (abs_lo_int - 1);
            return result - abs_lo_int;
        }
        if (hi < signed_min_d) return (std::numeric_limits<Signed>::min)();

        Signed hi_int = static_cast<Signed>(hi);
        if (lo >= signed_max_d) return (std::numeric_limits<Signed>::max)();
        if (lo < signed_min_d) return (std::numeric_limits<Signed>::min)();
        Signed lo_int = static_cast<Signed>(lo);
        if (lo_int > 0 && hi_int > (std::numeric_limits<Signed>::max)() - lo_int) {
            return (std::numeric_limits<Signed>::max)();
        }
        if (lo_int < 0 && hi_int < (std::numeric_limits<Signed>::min)() - lo_int) {
            return (std::numeric_limits<Signed>::min)();
        }
        Signed sum = hi_int + lo_int;

        double hi_frac = hi - static_cast<double>(hi_int);
        double lo_frac = lo - static_cast<double>(lo_int);
        double frac_sum = hi_frac + lo_frac;

        if (frac_sum >= 1.0) {
            if (sum == (std::numeric_limits<Signed>::max)()) return sum;
            return sum + 1;
        }
        if (frac_sum <= -1.0) {
            if (sum == (std::numeric_limits<Signed>::min)()) return sum;
            return sum - 1;
        }
        if (sum > 0 && frac_sum < 0.0) return sum - 1;
        if (sum < 0 && frac_sum > 0.0) return sum + 1;
        return sum;
    }

public:
    // conversion operators
    explicit constexpr operator int()                   const noexcept { return convert_to_signed_impl<int>(); }
    explicit constexpr operator long()                  const noexcept { return convert_to_signed_impl<long>(); }
    explicit constexpr operator long long()             const noexcept { return convert_to_signed_impl<long long>(); }
    explicit constexpr operator unsigned int()          const noexcept { return convert_to_unsigned_impl<unsigned int>(); }
    explicit constexpr operator unsigned long()         const noexcept { return convert_to_unsigned_impl<unsigned long>(); }
    explicit constexpr operator unsigned long long()    const noexcept { return convert_to_unsigned_impl<unsigned long long>(); }
    explicit constexpr operator float()                 const noexcept { return static_cast<float>(cascade.to_double()); }
    explicit constexpr operator double()                const noexcept { return cascade.to_double(); }

    qd_cascade& operator=(const qd_cascade&) = default;
    qd_cascade& operator=(qd_cascade&&) = default;

    // Assignment from floatcascade
    constexpr qd_cascade& operator=(const floatcascade<4>& fc) {
        cascade = fc;
        return *this;
    }

    // Extract floatcascade
    constexpr const floatcascade<4>& get_cascade() const { return cascade; }
    constexpr operator floatcascade<4>() const { return cascade; }

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
    CONSTEXPRESSION qd_cascade& operator+=(const qd_cascade& rhs) noexcept {
		auto result = expansion_ops::add_cascades(cascade, rhs.cascade);  // 8 components
		// Compress to 4 components using proven QD algorithm
		cascade = expansion_ops::compress_8to4(result);
        return *this;
    }

    CONSTEXPRESSION qd_cascade& operator-=(const qd_cascade& rhs) noexcept {
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

    CONSTEXPRESSION qd_cascade& operator*=(const qd_cascade& rhs) noexcept {
		*this = expansion_ops::multiply_cascades(cascade, rhs.cascade);
        return *this;
    }

    CONSTEXPRESSION qd_cascade& operator/=(const qd_cascade& rhs) noexcept {
		if (isnan())
			return *this;
		if (rhs.isnan())
			return *this = rhs;
		if (rhs.iszero()) {
			if (iszero()) {
				*this = qd_cascade(SpecificValue::qnan);
			} else {
				// Determine sign of result.  At runtime use std::copysign;
				// during constant evaluation use std::bit_cast to extract
				// the IEEE-754 sign bit so -0.0 carries through correctly
				// (per #727 / #797 / #798 lessons).
				int sA, sB;
				if (std::is_constant_evaluated()) {
					sA = (std::bit_cast<std::uint64_t>(cascade[0])     >> 63) ? -1 : 1;
					sB = (std::bit_cast<std::uint64_t>(rhs.cascade[0]) >> 63) ? -1 : 1;
				}
				else {
					sA = (std::copysign(1.0, cascade[0])     < 0.0) ? -1 : 1;
					sB = (std::copysign(1.0, rhs.cascade[0]) < 0.0) ? -1 : 1;
				}
				*this = qd_cascade((sA == sB) ? SpecificValue::infpos : SpecificValue::infneg);
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

    // Compound assignment with double
    CONSTEXPRESSION qd_cascade& operator+=(double rhs) noexcept { return *this += qd_cascade(rhs); }
    CONSTEXPRESSION qd_cascade& operator-=(double rhs) noexcept { return *this -= qd_cascade(rhs); }
    CONSTEXPRESSION qd_cascade& operator*=(double rhs) noexcept { return *this *= qd_cascade(rhs); }
    CONSTEXPRESSION qd_cascade& operator/=(double rhs) noexcept { return *this /= qd_cascade(rhs); }

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
    constexpr double operator[](size_t index) const { return cascade[index]; }
    constexpr double& operator[](size_t index) { return cascade[index]; }

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

    constexpr int  sign()          const noexcept { return cascade.sign(); }
    constexpr bool signbit()       const noexcept { return cascade.sign() < 0; }
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

    // (convert_to_unsigned_impl / convert_to_signed_impl moved up next to the
    // conversion operators that call them, so the constexpr call chain is
    // fully resolved at the point of declaration.  The old int64_t-cast
    // helpers had two bugs: they ignored cascade[2] and cascade[3], and
    // could trigger UB per C++20 [conv.fpint] for unnormalized inputs.)

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
    friend std::ostream& operator<<(std::ostream& ostr, const qd_cascade& v) {
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

constexpr qd_cascade operator+(const qd_cascade& lhs, const qd_cascade& rhs) {
	qd_cascade sum = lhs;
	sum += rhs;
	return sum;
}

constexpr qd_cascade operator-(const qd_cascade& lhs, const qd_cascade& rhs) {
	qd_cascade diff = lhs;
	diff -= rhs;
	return diff;
}

constexpr qd_cascade operator*(const qd_cascade& lhs, const qd_cascade& rhs) {
	qd_cascade mul = lhs;
	mul *= rhs;
	return mul;
}

constexpr qd_cascade operator/(const qd_cascade& lhs, const qd_cascade& rhs) {
	qd_cascade div = lhs;
	div /= rhs;
	return div;
}

// qd_cascade-double mixed operations
constexpr qd_cascade operator+(const qd_cascade& lhs, double rhs) { return operator+(lhs, qd_cascade(rhs)); }
constexpr qd_cascade operator-(const qd_cascade& lhs, double rhs) { return operator-(lhs, qd_cascade(rhs)); }
constexpr qd_cascade operator*(const qd_cascade& lhs, double rhs) { return operator*(lhs, qd_cascade(rhs)); }
constexpr qd_cascade operator/(const qd_cascade& lhs, double rhs) { return operator/(lhs, qd_cascade(rhs)); }

// double-qd_cascade mixed operations
constexpr qd_cascade operator+(double lhs, const qd_cascade& rhs) { return operator+(qd_cascade(lhs), rhs); }
constexpr qd_cascade operator-(double lhs, const qd_cascade& rhs) { return operator-(qd_cascade(lhs), rhs); }
constexpr qd_cascade operator*(double lhs, const qd_cascade& rhs) { return operator*(qd_cascade(lhs), rhs); }
constexpr qd_cascade operator/(double lhs, const qd_cascade& rhs) { return operator/(qd_cascade(lhs), rhs); }

// Comparison operators
constexpr bool operator==(const qd_cascade& lhs, const qd_cascade& rhs) {
    return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2] && lhs[3] == rhs[3];
}

constexpr bool operator!=(const qd_cascade& lhs, const qd_cascade& rhs) {
    return !(lhs == rhs);
}

constexpr bool operator<(const qd_cascade& lhs, const qd_cascade& rhs) {
    if (lhs[0] < rhs[0]) return true;
    if (lhs[0] > rhs[0]) return false;
    if (lhs[1] < rhs[1]) return true;
    if (lhs[1] > rhs[1]) return false;
    if (lhs[2] < rhs[2]) return true;
    if (lhs[2] > rhs[2]) return false;
    return lhs[3] < rhs[3];
}

constexpr bool operator>(const qd_cascade& lhs, const qd_cascade& rhs) {
    return rhs < lhs;
}

constexpr bool operator<=(const qd_cascade& lhs, const qd_cascade& rhs) {
    // NOT !operator>: that returns true for unordered (NaN) operands. Use
    // operator< || operator== so NaN comparisons stay unordered (per #797).
    return operator<(lhs, rhs) || operator==(lhs, rhs);
}

constexpr bool operator>=(const qd_cascade& lhs, const qd_cascade& rhs) {
    return operator>(lhs, rhs) || operator==(lhs, rhs);
}

// Comparison with double
constexpr bool operator==(const qd_cascade& lhs, double rhs) { return lhs == qd_cascade(rhs); }
constexpr bool operator!=(const qd_cascade& lhs, double rhs) { return lhs != qd_cascade(rhs); }
constexpr bool operator<(const qd_cascade& lhs, double rhs) { return lhs < qd_cascade(rhs); }
constexpr bool operator>(const qd_cascade& lhs, double rhs) { return lhs > qd_cascade(rhs); }
constexpr bool operator<=(const qd_cascade& lhs, double rhs) { return operator<(lhs, qd_cascade(rhs)) || operator==(lhs, qd_cascade(rhs)); }
constexpr bool operator>=(const qd_cascade& lhs, double rhs) { return operator>(lhs, qd_cascade(rhs)) || operator==(lhs, qd_cascade(rhs)); }

constexpr bool operator==(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) == rhs; }
constexpr bool operator!=(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) != rhs; }
constexpr bool operator<(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) < rhs; }
constexpr bool operator>(double lhs, const qd_cascade& rhs) { return qd_cascade(lhs) > rhs; }
constexpr bool operator<=(double lhs, const qd_cascade& rhs) { return operator<(qd_cascade(lhs), rhs) || operator==(qd_cascade(lhs), rhs); }
constexpr bool operator>=(double lhs, const qd_cascade& rhs) { return operator>(qd_cascade(lhs), rhs) || operator==(qd_cascade(lhs), rhs); }

// standard attribute function overloads

inline bool signbit(const qd_cascade& a) {
	return std::signbit(a[0]);
}

// pow() is defined in math/functions/pow.hpp

inline qd_cascade reciprocal(const qd_cascade& a) {
	return qd_cascade(1.0) / a;
}

// Square function - delegates to floatcascade implementation
inline qd_cascade sqr(const qd_cascade& a) {
	floatcascade<4> fc = a;
	return qd_cascade(sqr(fc));
}

// Round to Nearest integer
inline qd_cascade nint(const qd_cascade& a) {
	double x0 = nint(a[0]);
	double x1, x2, x3;

	if (x0 == a[0]) {
		// x[0] is an integer already. Round x[1].
		x1 = nint(a[1]);

		if (x1 == a[1]) {
			// x[1] is also an integer. Round x[2].
			x2 = nint(a[2]);

			if (x2 == a[2]) {
				// x[2] is also an integer. Round x[3].
				x3 = nint(a[3]);
				// Renormalize
				double t;
				x0 = quick_two_sum(x0, x1, t);
				x1 = quick_two_sum(t, x2, t);
				x2 = quick_two_sum(t, x3, x3);
				x0 = quick_two_sum(x0, x1, t);
				x1 = quick_two_sum(t, x2, x2);
			} else {
				// x[2] is not an integer
				x3 = 0.0;
				if (std::abs(x2 - a[2]) == 0.5 && a[3] < 0.0) {
					x2 -= 1.0;  // Break tie using x[3]
				}
				double t;
				x0 = quick_two_sum(x0, x1, t);
				x1 = quick_two_sum(t, x2, x2);
			}
		} else {
			// x[1] is not an integer
			x2 = 0.0;
			x3 = 0.0;
			if (std::abs(x1 - a[1]) == 0.5 && a[2] < 0.0) {
				x1 -= 1.0;  // Break tie using x[2]
			}
			x0 = quick_two_sum(x0, x1, x1);
		}
	} else {
		// x[0] is not an integer
		x1 = 0.0;
		x2 = 0.0;
		x3 = 0.0;
		if (std::abs(x0 - a[0]) == 0.5 && a[1] < 0.0) {
			x0 -= 1.0;  // Break tie using x[1]
		}
	}

	return qd_cascade(x0, x1, x2, x3);
}

// Note: add/sub/mul/div helper functions with (double, double) signatures
// have been removed to avoid namespace pollution when multiple cascade types
// are included together. Use operators or constructors instead:
//   qd_cascade(a) + qd_cascade(b)  instead of  add(a, b)

// sqrt() is defined in math/functions/sqrt.hpp

// Decimal string parsing - delegates to floatcascade base class for full precision
inline bool parse(const std::string& number, qd_cascade& value) {
	// Delegates to floatcascade base class for full precision parsing
	floatcascade<4> temp_cascade;
	if (temp_cascade.parse(number)) {
		value = qd_cascade(temp_cascade);
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// qd_cascade constants

constexpr int qdc_max_precision = 212;  // in bits (53*4)

// simple constants
constexpr qd_cascade qdc_third(0.33333333333333331, 1.8503717077085941e-17, 0.0, 0.0);

} // namespace sw::universal
