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
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <universal/internal/floatcascade/floatcascade.hpp>

namespace sw::universal {

    // Forward declarations
inline bool signbit(const class dd_cascade&);
inline dd_cascade operator-(const dd_cascade&, const dd_cascade&);
inline dd_cascade operator*(const dd_cascade&, const dd_cascade&);
inline bool       operator==(const dd_cascade&, const dd_cascade&);
inline bool       operator<(const dd_cascade&, const dd_cascade&);
inline bool       operator>=(const dd_cascade&, const dd_cascade&);
inline dd_cascade abs(const dd_cascade&);
inline dd_cascade pow(const dd_cascade&, const dd_cascade&);
inline dd_cascade pown(const dd_cascade&, int);
inline dd_cascade frexp(const dd_cascade&, int*);
inline dd_cascade ldexp(const dd_cascade&, int);
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

    // this is debug infrastructure: TODO: remove when decimal conversion is solved reliably
	static constexpr bool bTraceDecimalConversion = false;
	static constexpr bool bTraceDecimalRounding   = false;

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

    	// convert to string containing digits number of digits
	std::string to_string(std::streamsize precision = 7, std::streamsize width = 15, bool fixed = false,
	                      bool scientific = true, bool internal = false, bool left = false, bool showpos = false,
	                      bool uppercase = false, char fill = ' ') const {
		std::string s;
		bool        negative = sign() ? true : false;
		int         e{0};
		if (fixed && scientific)
			fixed = false;  // scientific format takes precedence
		if (isnan()) {
			s        = uppercase ? "NAN" : "nan";
			negative = false;
		} else {
			if (negative) {
				s += '-';
			} else {
				if (showpos)
					s += '+';
			}

			if (isinf()) {
				s += uppercase ? "INF" : "inf";
			} else if (iszero()) {
				s += '0';
				if (precision > 0) {
					s += '.';
					s.append(static_cast<unsigned int>(precision), '0');
				}
			} else {
				int powerOfTenScale = static_cast<int>(std::log10(std::fabs(cascade[0])));
				int integerDigits   = (fixed ? (powerOfTenScale + 1) : 1);
				int nrDigits        = integerDigits + static_cast<int>(precision);

				int nrDigitsForFixedFormat = nrDigits;
				if (fixed)
					nrDigitsForFixedFormat =
					    std::max(60, nrDigits);  // can be much longer than the max accuracy for double-double

				if constexpr (bTraceDecimalConversion) {
					std::cout << "powerOfTenScale  : " << powerOfTenScale << '\n';
					std::cout << "integerDigits    : " << integerDigits << '\n';
					std::cout << "nrDigits         : " << nrDigits << '\n';
					std::cout << "nrDigitsForFixedFormat  : " << nrDigitsForFixedFormat << '\n';
				}


				// a number in the range of [0.5, 1.0) to be printed with zero precision
				// must be rounded up to 1 to print correctly
				if (fixed && (precision == 0) && (std::abs(cascade[0]) < 1.0)) {
					s += (std::abs(cascade[0]) >= 0.5) ? '1' : '0';
					return s;
				}

				if (fixed && nrDigits <= 0) {
					// process values that are near zero
					s += '0';
					if (precision > 0) {
						s += '.';
						s.append(static_cast<unsigned int>(precision), '0');
					}
				} else {
					std::vector<char> t;

					if (fixed) {
						t.resize(static_cast<size_t>(nrDigitsForFixedFormat + 1));
						to_digits(t, e, nrDigitsForFixedFormat);
					} else {
						t.resize(static_cast<size_t>(nrDigits + 1));
						to_digits(t, e, nrDigits);
					}

					if (fixed) {
						// round the decimal string
						round_string(t, nrDigits + 1, &integerDigits);

						if (integerDigits > 0) {
							int i;
							for (i = 0; i < integerDigits; ++i)
								s += t[static_cast<unsigned>(i)];
							if (precision > 0) {
								s += '.';
								for (int j = 0; j < precision; ++j, ++i)
									s += t[static_cast<unsigned>(i)];
							}
						} else {
							s += "0.";
							if (integerDigits < 0)
								s.append(static_cast<size_t>(-integerDigits), '0');
							for (int i = 0; i < nrDigits; ++i)
								s += t[static_cast<unsigned>(i)];
						}
					} else {
						s += t[0ull];
						if (precision > 0)
							s += '.';

						for (int i = 1; i <= precision; ++i)
							s += t[static_cast<unsigned>(i)];
					}
				}
			}

			// TBD: this is seriously broken and needs a redesign
			//
			// fix for improper offset with large values and small values
			// without this trap, output of values of the for 10^j - 1 fail for j > 28
			// and are output with the point in the wrong place, leading to a significant error
			if (fixed && (precision > 0)) {
				// make sure that the value isn't dramatically larger
				double from_string = atof(s.c_str());

				// if this ratio is large, then we've got problems
				if (std::fabs(from_string / cascade[0]) > 3.0) {
					// loop on the string, find the point, move it up one
					// don't act on the first character
					for (std::string::size_type i = 1; i < s.length(); ++i) {
						if (s[i] == '.') {
							s[i]     = s[i - 1];
							s[i - 1] = '.';  // this will destroy the leading 0 when s[i==1] == '.';
							break;
						}
					}
					// BUG: the loop above, in particular s[i-1] = '.', destroys the leading 0
					// in the fixed point representation if the point is located at i = 1;
					// it also breaks the precision request as it adds a new digit to the fixed representation

					from_string = atof(s.c_str());
					// if this ratio is large, then the string has not been fixed
					if (std::fabs(from_string / cascade[0]) > 3.0) {
						std::cerr << "re-rounding unsuccessful in fixed point fix\n";
					}
				}
			}

			if (!fixed && !isinf()) {
				// construct the exponent
				s += uppercase ? 'E' : 'e';
				append_exponent(s, e);
			}
		}

		// process any fill
		size_t strLength = s.length();
		if (strLength < static_cast<size_t>(width)) {
			size_t nrCharsToFill = (width - strLength);
			if (internal) {
				if (negative)
					s.insert(static_cast<std::string::size_type>(1), nrCharsToFill, fill);
				else
					s.insert(static_cast<std::string::size_type>(0), nrCharsToFill, fill);
			} else if (left) {
				s.append(nrCharsToFill, fill);
			} else {
				s.insert(static_cast<std::string::size_type>(0), nrCharsToFill, fill);
			}
		}

		return s;
	}

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

	// precondition: string s must be all digits
	void round_string(std::vector<char>& s, int precision, int* decimalPoint) const {
		if constexpr (bTraceDecimalRounding) {
			std::string str(s.begin(), s.end());
			std::cout << "string       : " << str << '\n';
			std::cout << "precision    : " << precision << '\n';
			std::cout << "decimalPoint : " << *decimalPoint << '\n';
		}

		int nrDigits = precision;
		// round decimal string and propagate carry
		int lastDigit = nrDigits - 1;
		if (s[static_cast<unsigned>(lastDigit)] >= '5') {
			if constexpr (bTraceDecimalRounding)
				std::cout << "need to round\n";
			int i = nrDigits - 2;
			s[static_cast<unsigned>(i)]++;
			while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
				s[static_cast<unsigned>(i)] -= 10;
				s[static_cast<unsigned>(--i)]++;
			}
		}

		// if first digit is 10, shift everything.
		if (s[0] > '9') {
			if constexpr (bTraceDecimalRounding)
				std::cout << "shift right to handle overflow\n";
			for (int i = precision; i >= 2; --i)
				s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
			s[0u] = '1';
			s[1u] = '0';

			(*decimalPoint)++;  // increment decimal point
			++precision;
		}
	}

	void append_exponent(std::string& str, int e) const {
		str += (e < 0 ? '-' : '+');
		e = std::abs(e);
		int k;
		if (e >= 100) {
			k = (e / 100);
			str += static_cast<char>('0' + k);
			e -= 100 * k;
		}

		k = (e / 10);
		str += static_cast<char>('0' + k);
		e -= 10 * k;

		str += static_cast<char>('0' + e);
	}

    /// <summary>
	/// to_digits generates the decimal digits representing
	/// </summary>
	/// <param name="s"></param>
	/// <param name="exponent"></param>
	/// <param name="precision"></param>
	// void to_digits(char* s, int& exponent, int precision) const {
	void to_digits(std::vector<char>& s, int& exponent, int precision) const {
		constexpr dd_cascade _one(1.0), _ten(10.0);
		constexpr double     _log2(0.301029995663981);

		if (iszero()) {
			exponent = 0;
			for (int i = 0; i < precision; ++i)
				s[static_cast<unsigned>(i)] = '0';
			return;
		}

		// First determine the (approximate) exponent.
		// std::frexp(*this, &e);   // e is appropriate for 0.5 <= x < 1
		int e;
		(void) std::frexp(cascade[0], &e);            // Only need exponent, not mantissa
		--e;                                 // adjust e as frexp gives a binary e that is 1 too big
		e    = static_cast<int>(_log2 * e);  // estimate the power of ten exponent
		dd_cascade r = abs(*this);
		if (e < 0) {
			if (e < -300) {
				r = dd_cascade(std::ldexp(r.high(), 53), std::ldexp(r.low(), 53));
				r *= pown(_ten, -e);
				r = dd_cascade(std::ldexp(r.high(), -53), std::ldexp(r.low(), -53));
			} else {
				r *= pown(_ten, -e);
			}
		} else {
			if (e > 0) {
				if (e > 300) {
					r = dd_cascade(std::ldexp(r.high(), -53), std::ldexp(r.low(), -53));
					r /= pown(_ten, e);
					r = dd_cascade(std::ldexp(r.high(), 53), std::ldexp(r.low(), 53));
				} else {
					r /= pown(_ten, e);
				}
			}
		}

		// Fix exponent if we have gone too far
		if (r >= _ten) {
			r /= _ten;
			++e;
		} else {
			if (r < 1.0) {
				r *= _ten;
				--e;
			}
		}

		if ((r >= _ten) || (r < _one)) {
			std::cerr << "to_digits() failed to compute exponent\n";
			return;
		}

		// at this point the value is normalized to a decimal value between (0, 10)
		// generate the digits
		int nrDigits = precision + 1;
		for (int i = 0; i < nrDigits; ++i) {
			int mostSignificantDigit = static_cast<int>(r[0]);
			r -= mostSignificantDigit;
			r *= 10.0;

			s[static_cast<unsigned>(i)] = static_cast<char>(mostSignificantDigit + '0');
			if constexpr (bTraceDecimalConversion) {
				std::string str(s.begin(), s.end());
				std::cout << "to_digits  digit[" << i << "] : " << str << '\n';
			}
		}

		// Fix out of range digits
		for (int i = nrDigits - 1; i > 0; --i) {
			if (s[static_cast<unsigned>(i)] < '0') {
				s[static_cast<unsigned>(i - 1)]--;
				s[static_cast<unsigned>(i)] += 10;
			} else {
				if (s[static_cast<unsigned>(i)] > '9') {
					s[static_cast<unsigned>(i - 1)]++;
					s[static_cast<unsigned>(i)] -= 10;
				}
			}
		}

		if (s[0] <= '0') {
			std::cerr << "to_digits() non-positive leading digit\n";
			return;
		}

		// Round and propagate carry
		int lastDigit = nrDigits - 1;
		if (s[static_cast<unsigned>(lastDigit)] >= '5') {
			int i = nrDigits - 2;
			s[static_cast<unsigned>(i)]++;
			while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
				s[static_cast<unsigned>(i)] -= 10;
				s[static_cast<unsigned>(--i)]++;
			}
		}

		// If first digit is 10, shift left and increment exponent
		if (s[0] > '9') {
			++e;
			for (int i = precision; i >= 2; --i) {
				s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
			}
			s[0] = '1';
			s[1] = '0';
		}

		s[static_cast<unsigned>(precision)] = 0;  // termination null
		exponent                            = e;
	}
};

////////////////////////  precomputed constants of note  /////////////////////////////////

constexpr int ddc_max_precision = 106;  // in bits

// simple constants
constexpr dd_cascade ddc_third(0.33333333333333331, 1.8503717077085941e-17);

constexpr double     ddc_eps            = 4.93038065763132e-32;     // 2^-104
constexpr double     ddc_min_normalized = 2.0041683600089728e-292;  // = 2^(-1022 + 53)
constexpr dd_cascade ddc_max(1.79769313486231570815e+308, 9.97920154767359795037e+291);
constexpr dd_cascade ddc_safe_max(1.7976931080746007281e+308, 9.97920154767359795037e+291);

// precomputed double-double constants courtesy of Universal constants generator
// Author: Theodore Omtzigt
constexpr dd_cascade ddc_one(1.0, 0.0);

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
inline bool operator==(const dd_cascade& lhs, const dd_cascade& rhs) { return lhs[0] == rhs[0] && lhs[1] == rhs[1]; }
inline bool operator!=(const dd_cascade& lhs, const dd_cascade& rhs) { return !(lhs == rhs); }
inline bool operator< (const dd_cascade& lhs, const dd_cascade& rhs) { 
    if (lhs[0] < rhs[0]) return true;
    if (lhs[0] > rhs[0]) return false;
    return lhs[1] < rhs[1];
}
inline bool operator> (const dd_cascade& lhs, const dd_cascade& rhs) { 
    if (lhs[0] > rhs[0]) return true;
	if (lhs[0] < rhs[0]) return false;
	return lhs[1] > rhs[1];
}
inline bool operator<=(const dd_cascade& lhs, const dd_cascade& rhs) { return !(rhs > lhs); }
inline bool operator>=(const dd_cascade& lhs, const dd_cascade& rhs) { return !(lhs < rhs); }

// Comparison with double
inline bool operator==(const dd_cascade& lhs, double rhs) { return lhs == dd_cascade(rhs); }
inline bool operator!=(const dd_cascade& lhs, double rhs) { return lhs != dd_cascade(rhs); }
inline bool operator< (const dd_cascade& lhs, double rhs) { return lhs < dd_cascade(rhs); }
inline bool operator> (const dd_cascade& lhs, double rhs) { return lhs > dd_cascade(rhs); }
inline bool operator<=(const dd_cascade& lhs, double rhs) { return lhs <= dd_cascade(rhs); }
inline bool operator>=(const dd_cascade& lhs, double rhs) { return lhs >= dd_cascade(rhs); }

inline bool operator==(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) == rhs; }
inline bool operator!=(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) != rhs; }
inline bool operator< (double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) < rhs; }
inline bool operator> (double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) > rhs; }
inline bool operator<=(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) <= rhs; }
inline bool operator>=(double lhs, const dd_cascade& rhs) { return dd_cascade(lhs) >= rhs; }

// standard attribute function overloads

inline bool signbit(const dd_cascade& a) {
	return std::signbit(a[0]);
}

////////////////////////    math functions   /////////////////////////////////

inline dd_cascade ulp(const dd_cascade& a) {
	double hi{a.high()};
	double lo{a.low()};
	double nlo;
	if (lo == 0.0) {
		nlo                = std::numeric_limits<double>::epsilon() / 2.0;
		int binaryExponent = scale(hi) - 53;
		nlo /= std::pow(2.0, -binaryExponent);
	} else {
		nlo = (hi < 0.0 ? std::nextafter(lo, -INFINITY) : std::nextafter(lo, +INFINITY));
	}
	dd_cascade n(hi, nlo);

	return n - a;
}

inline dd_cascade abs(const dd_cascade& a) {
	double hi = a.high();
	double lo = a.low();
	if (hi < 0) {  // flip the pair with respect to 0
		hi = -hi;
		lo = -lo;
	}
	return dd_cascade(hi, lo);
}

// Round to Nearest integer
inline dd_cascade nint(const dd_cascade& a) {
	double hi = nint(a.high());
	double lo;

	if (hi == a.high()) {
		/* High word is an integer already.  Round the low word.*/
		lo = nint(a.low());

		/* Renormalize. This is needed if x[0] = some integer, x[1] = 1/2.*/
		hi = quick_two_sum(hi, lo, lo);
	} else {
		/* High word is not an integer. */
		lo = 0.0;
		if (std::abs(hi - a.high()) == 0.5 && a.low() < 0.0) {
			/* There is a tie in the high word, consult the low word
			   to break the tie. */
			hi -= 1.0; /* NOTE: This does not cause INEXACT. */
		}
	}

	return dd_cascade(hi, lo);
}

// double plus double yielding a double-double
inline dd_cascade add(double a, double b) {
	if (std::isnan(a) || std::isnan(b))
		return dd_cascade(SpecificValue::snan);
	double s, e;
	s = two_sum(a, b, e);
	return dd_cascade(s, e);
}

// double minus double yielding a double-double
inline dd_cascade sub(double a, double b) {
	if (std::isnan(a) || std::isnan(b))
		return dd_cascade(SpecificValue::snan);
	double s, e;
	s = two_sum(a, -b, e);
	return dd_cascade(s, e);
}

// double times double yielding a double-double
inline dd_cascade mul(double a, double b) {
	if (std::isnan(a) || std::isnan(b))
		return dd_cascade(SpecificValue::snan);
	double p, e;
	p = two_prod(a, b, e);
	return dd_cascade(p, e);
}

// double divide by double yielding a double-double
inline dd_cascade div(double a, double b) {
	if (std::isnan(a) || std::isnan(b))
		return dd_cascade(SpecificValue::snan);

	if (b == 0.0)
		return (sign(a) ? dd_cascade(SpecificValue::infneg) : dd_cascade(SpecificValue::infpos));

	double q1 = a / b;  // initial approximation

	// Compute residual: a - q1 * b
	volatile double p2;
	double          p1 = two_prod(q1, b, p2);
	volatile double e;
	double          s = two_diff(a, p1, e);
	e -= p2;

	// get next approximation
	double q2 = (s + e) / b;

	//	normalize
	s = quick_two_sum(q1, q2, e);
	return dd_cascade(s, e);
}

// double-double * double, where double is a power of 2
inline dd_cascade mul_pwr2(const dd_cascade& a, double b) {
	return dd_cascade(a.high() * b, a.low() * b);
}

/////////////////////////////////////////////////////////////////////////////
// quad-double operators

// quad-double + double-double
inline void qd_add(double const a[4], const dd_cascade& b, double s[4]) {
	double t[5];
	s[0] = two_sum(a[0], b.high(), t[0]);  //	s0 - O( 1 ); t0 - O( e )
	s[1] = two_sum(a[1], b.low(), t[1]);   //	s1 - O( e ); t1 - O( e^2 )

	s[1] = two_sum(s[1], t[0], t[0]);  //	s1 - O( e ); t0 - O( e^2 )

	s[2] = a[2];                  //	s2 - O( e^2 )
	three_sum(s[2], t[0], t[1]);  //	s2 - O( e^2 ); t0 - O( e^3 ); t1 = O( e^4 )

	s[3] = two_sum(a[3], t[0], t[0]);  //	s3 - O( e^3 ); t0 - O( e^4 )
	t[0] += t[1];                      //	fl( t0 + t1 ) - accuracy less important

	renorm(s[0], s[1], s[2], s[3], t[0]);
}

// quad-double = double-double * double-double
inline void qd_mul(const dd_cascade& a, const dd_cascade& b, double p[4]) {
	double p4, p5, p6, p7;

	//	powers of e - 0, 1, 1, 1, 2, 2, 2, 3
	p[0] = two_prod(a.high(), b.high(), p[1]);
	if (std::isfinite(p[0])) {
		p[2] = two_prod(a.high(), b.low(), p4);
		p[3] = two_prod(a.low(), b.high(), p5);
		p6   = two_prod(a.low(), b.low(), p7);

		//	powers of e - 0, 1, 2, 3, 2, 2, 2, 3
		three_sum(p[1], p[2], p[3]);

		//	powers of e - 0, 1, 2, 3, 2, 3, 4, 3
		three_sum(p4, p5, p6);

		//	powers of e - 0, 1, 2, 3, 3, 3, 4, 3
		p[2] = two_sum(p[2], p4, p4);

		//	powers of e - 0, 1, 2, 3, 4, 5, 4, 3
		three_sum(p[3], p4, p5);

		//	powers of e - 0, 1, 2, 3, 4, 5, 4, 4
		p[3] = two_sum(p[3], p7, p7);

		p4 += (p6 + p7);

		renorm(p[0], p[1], p[2], p[3], p4);
	} else {
		p[1] = p[2] = p[3] = 0.0;
	}
}

inline dd_cascade fma(const dd_cascade& a, const dd_cascade& b, const dd_cascade& c) {
	double p[4];
	qd_mul(a, b, p);
	qd_add(p, c, p);
	p[0] = two_sum(p[0], p[1] + p[2] + p[3], p[1]);
	return dd_cascade(p[0], p[1]);
}

inline dd_cascade sqr(const dd_cascade& a) {
	if (a.isnan())
		return a;

	double p2, p1 = two_sqr(a.high(), p2);
	p2 += 2.0 * a.high() * a.low();
	p2 += a.low() * a.low();

	double s2{0}, s1 = quick_two_sum(p1, p2, s2);
	return dd_cascade(s1, s2);
}

inline dd_cascade reciprocal(const dd_cascade& a) {
	if (a.iszero())
		return dd_cascade(SpecificValue::infpos);

	if (a.isinf())
		return dd_cascade(0.0);

	double q1 = 1.0 / a.high(); /* approximate quotient */
	if (std::isfinite(q1)) {
		dd_cascade r = fma(-q1, a, 1.0);

		double q2 = r.high() / a.high();
		r         = fma(-q2, a, r);

		double q3 = r.high() / a.high();
		three_sum(q1, q2, q3);
		return dd_cascade(q1, q2);
	} else {
		return dd_cascade(q1, 0.0);
	}
}

/////////////////////////////////////////////////////////////////////////////
//	power functions

inline dd_cascade pown(const dd_cascade& a, int n) {
	if (a.isnan())
		return a;

	int        N = (n < 0) ? -n : n;
	dd_cascade s;

	switch (N) {
	case 0:
		if (a.iszero()) {
			std::cerr << "pown: invalid argument\n";
			errno = EDOM;
			return dd_cascade(SpecificValue::qnan);
		}
		return dd_cascade(1.0);

	case 1:
		s = a;
		break;

	case 2:
		s = sqr(a);
		break;

	default:  // Use binary exponentiation
	{
		dd_cascade r{a};

		s = 1.0;
		while (N > 0) {
			if (N % 2 == 1) {
				s *= r;
			}
			N /= 2;
			if (N > 0)
				r = sqr(r);
		}
	} break;
	}

	// Compute the reciprocal if n is negative.
	return n < 0 ? reciprocal(s) : s;
}

////////////////////////  stream operators   /////////////////////////////////

// stream out a decimal floating-point representation of the double-double
inline std::ostream& operator<<(std::ostream& ostr, const dd_cascade& v) {
	std::ios_base::fmtflags fmt        = ostr.flags();
	std::streamsize         precision  = ostr.precision();
	std::streamsize         width      = ostr.width();
	char                    fillChar   = ostr.fill();
	bool                    showpos    = fmt & std::ios_base::showpos;
	bool                    uppercase  = fmt & std::ios_base::uppercase;
	bool                    fixed      = fmt & std::ios_base::fixed;
	bool                    scientific = fmt & std::ios_base::scientific;
	bool                    internal   = fmt & std::ios_base::internal;
	bool                    left       = fmt & std::ios_base::left;
	return ostr << v.to_string(precision, width, fixed, scientific, internal, left, showpos, uppercase, fillChar);
}

// stream in an ASCII decimal floating-point format and assign it to a double-double
inline std::istream& operator>>(std::istream& istr, dd_cascade& v) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a double-double value\n";
	}
	return istr;
}

////////////////// string operators

// parse a decimal ASCII floating-point format and make a doubledouble (dd) out of it
inline bool parse(const std::string& number, dd_cascade& value) {
	char const* p = number.c_str();

	// Skip any leading spaces
	while (std::isspace(*p))
		++p;

	dd_cascade r{0.0};
	int  nrDigits{0};
	int  decimalPoint{-1};
	int  sign{0}, eSign{1};
	int  e{0};
	bool done{false}, parsingMantissa{true};
	char ch;
	while (!done && (ch = *p) != '\0') {
		if (std::isdigit(ch)) {
			if (parsingMantissa) {
				int digit = ch - '0';
				r *= 10.0;
				r += static_cast<double>(digit);
				++nrDigits;
			} else {  // parsing exponent section
				int digit = ch - '0';
				e *= 10;
				e += digit;
			}
		} else {
			switch (ch) {
			case '.':
				if (decimalPoint >= 0)
					return false;
				decimalPoint = nrDigits;
				break;

			case '-':
			case '+':
				if (parsingMantissa) {
					if (sign != 0 || nrDigits > 0)
						return false;
					sign = (ch == '-' ? -1 : 1);
				} else {
					eSign = (ch == '-' ? -1 : 1);
				}
				break;

			case 'E':
			case 'e':
				parsingMantissa = false;
				break;

			default:
				return false;
			}
		}

		++p;
	}
	e *= eSign;

	if (decimalPoint >= 0)
		e -= (nrDigits - decimalPoint);
	dd_cascade _ten(10.0, 0.0);
	if (e > 0) {
		r *= pown(_ten, e);
	} else {
		if (e < 0)
			r /= pown(_ten, -e);
	}
	value = (sign == -1) ? -r : r;
	return true;
}


} // namespace sw::universal
