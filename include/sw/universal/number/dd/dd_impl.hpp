#pragma once
// dd_impl.hpp: implementation of the double-double floating-point number system described in
// 
// Sherry Li, David Bailey, LBNL, "Library for Double-Double and Quad-Double Arithmetic", 2008
// https://www.researchgate.net/publication/228570156_Library_for_Double-Double_and_Quad-Double_Arithmetic
// 
// Adapted core subroutines from QD library by Yozo Hida
// 
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <cstdint>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <vector>
#include <bit>
#include <type_traits>

// supporting types and functions
#include <universal/utility/bit_cast.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/numerics/error_free_ops.hpp>
#include <universal/number/shared/nan_encoding.hpp>
#include <universal/number/shared/infinite_encoding.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>
// dd exception structure
#include <universal/number/dd/exceptions.hpp>
#include <universal/number/dd/dd_fwd.hpp>

namespace sw { namespace universal {

	inline std::ostream& operator<<(std::ostream& ostr, const std::vector<char>& s) {
		for (auto c : s) {
			ostr << c;
		}
		return ostr;
	}

// fwd references to free functions
constexpr dd operator-(const dd&, const dd&);
constexpr dd operator*(const dd&, const dd&);
constexpr dd operator/(const dd&, const dd&);
constexpr dd fma(const dd&, const dd&, const dd&);
inline std::ostream& operator<<(std::ostream&, const dd&);
inline bool signbit(const dd&);
inline dd pown(const dd&, int);
inline dd frexp(const dd&, int*);
inline dd ldexp(const dd&, int);

// dd is an unevaluated pair of IEEE-754 doubles that provides a (1,11,106) floating-point triple
class dd {
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

	/// trivial constructor
	dd() = default;

	dd(const dd&) = default;
	dd(dd&&) = default;

	dd& operator=(const dd&) = default;
	dd& operator=(dd&&) = default;

	// converting constructors
	dd(const std::string& stringRep) : hi{0.0}, lo{0.0} { assign(stringRep); }

	// specific value constructor
	constexpr dd(const SpecificValue code) noexcept : hi{0.0}, lo{0.0} {
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
	constexpr dd(double h, double l)                noexcept : hi{ h }, lo{ l } {}

	// initializers for native types
	constexpr dd(signed char iv)                    noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(short iv)                          noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(int iv)                            noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(long iv)                           noexcept { *this = iv; }
	constexpr dd(long long iv)                      noexcept { *this = iv; }
	constexpr dd(char iv)                           noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(unsigned short iv)                 noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(unsigned int iv)                   noexcept : hi{ static_cast<double>(iv) }, lo{ 0.0 } {}
	constexpr dd(unsigned long iv)                  noexcept { *this = iv; }
	constexpr dd(unsigned long long iv)             noexcept { *this = iv; }
	constexpr dd(float iv)                          noexcept : hi{ iv }, lo{ 0.0 } {}
	constexpr dd(double iv)                         noexcept : hi{ iv }, lo{ 0.0 } {}

	// assignment operators for native types
	constexpr dd& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr dd& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr dd& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr dd& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

private:
	// Helper templates used by the conversion operators below.  Defined
	// before the operators so the constexpr call chain is fully resolved at
	// the point of operator declaration (clang requires this; gcc is more
	// permissive).
	// Portable limb-separated truncation toward zero for integer conversion.
	//
	// dd represents (hi + lo) where |lo| <= ulp(hi)/2 (non-overlapping).
	// Naive truncation of each limb (`static_cast<Signed>(hi) +
	// static_cast<Signed>(lo)`) is wrong when the limbs' fractional parts
	// combine to cross an integer boundary -- e.g. hi=101.0, lo=-0.3
	// represents 100.7 (truncates to 100), but hi_int + lo_int yields
	// 101 + 0 = 101.
	//
	// Correct algorithm: compute the limb-separated naive sum, then
	// adjust by the carry/borrow implied by the sum of fractional residuals.
	// This uses only double-precision and integer arithmetic (no
	// std::isfinite, no long double summation), so it gives the same exact
	// result on all platforms regardless of long double width.
	//
	// Limb-separated finiteness: a normalized dd has a finite hi iff the
	// dd value is finite, so the NaN/Inf check reduces to a check on hi.
	template<typename Signed>
	constexpr Signed convert_to_signed_impl() const noexcept {
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

		// Range check on hi.  For Signed = long long, max = 2^63 - 1 rounds
		// up to 2^63 as a double (since 2^63 - 1 is not representable in
		// binary64), so signed_max_d is one ulp ABOVE max in long long
		// terms. signed_min_d == min exactly (powers of 2 are exact).
		//
		// At hi == signed_max_d the dd may still fit in Signed if lo brings
		// it back into range -- e.g. dd(0x1p63, -2.0) represents 2^63 - 2,
		// which is a valid long long.  Handle that case via offset
		// arithmetic from max instead of saturating naively.
		constexpr double signed_max_d = static_cast<double>((std::numeric_limits<Signed>::max)());
		constexpr double signed_min_d = static_cast<double>((std::numeric_limits<Signed>::min)());

		if (hi > signed_max_d) return (std::numeric_limits<Signed>::max)();
		if (hi == signed_max_d) {
			if (lo >= 0.0) return (std::numeric_limits<Signed>::max)();
			// lo < 0.  dd = signed_max_d + lo, which fits in Signed.
			// Compute as max - (|lo| with fractional adjustment for trunc).
			//   |lo| in (0, ulp(signed_max_d)/2), purely an in-Signed-range value.
			double abs_lo = -lo;
			Signed abs_lo_int = static_cast<Signed>(abs_lo);
			double abs_lo_frac = abs_lo - static_cast<double>(abs_lo_int);
			Signed result = (std::numeric_limits<Signed>::max)();
			if (abs_lo_frac == 0.0) {
				// Integer lo:  signed_max_d - abs_lo_int = max + 1 - abs_lo_int
				return result - (abs_lo_int - 1);
			}
			// Fractional lo: trunc((max + 1) - abs_lo_int - abs_lo_frac) = max - abs_lo_int.
			return result - abs_lo_int;
		}
		if (hi < signed_min_d) return (std::numeric_limits<Signed>::min)();
		// hi == signed_min_d is OK: signed_min_d is exactly representable
		// as Signed (powers of two are exact in IEEE-754).

		// Limb-separated truncation toward zero.  The range check above
		// guarantees hi is in (signed_min_d, signed_max_d) (or == signed_min_d),
		// and for normalized dd |lo| <= ulp(hi)/2 so the limb sums stay in range.
		Signed hi_int = static_cast<Signed>(hi);
		Signed lo_int = static_cast<Signed>(lo);

		// Saturate on signed addition overflow (rare: only when hi is
		// within ulp(hi)/2 of the Signed limit and lo points the same way).
		if (lo_int > 0 && hi_int > (std::numeric_limits<Signed>::max)() - lo_int) {
			return (std::numeric_limits<Signed>::max)();
		}
		if (lo_int < 0 && hi_int < (std::numeric_limits<Signed>::min)() - lo_int) {
			return (std::numeric_limits<Signed>::min)();
		}
		Signed sum = hi_int + lo_int;

		// Fractional residuals after truncation toward zero.
		double hi_frac = hi - static_cast<double>(hi_int);  // in (-1, 1)
		double lo_frac = lo - static_cast<double>(lo_int);  // in (-1, 1)
		double frac_sum = hi_frac + lo_frac;                // in (-2, 2)

		// Adjust for fractional contribution crossing an integer boundary.
		if (frac_sum >= 1.0) {
			if (sum == (std::numeric_limits<Signed>::max)()) return sum;
			return sum + 1;
		}
		if (frac_sum <= -1.0) {
			if (sum == (std::numeric_limits<Signed>::min)()) return sum;
			return sum - 1;
		}

		// frac_sum in (-1, 1).  Adjust when the limb-wise truncation toward
		// zero overshoots the combined value's true integer bin.
		if (sum > 0 && frac_sum < 0.0) return sum - 1;
		if (sum < 0 && frac_sum > 0.0) return sum + 1;

		return sum;
	}

	template<typename Unsigned>
	constexpr Unsigned convert_to_unsigned_impl() const noexcept {
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

		// Negative dd values clamp to 0 (unsigned cannot represent them).
		// Normalized dd has |lo| < |hi| (within ulp), so hi < 0 implies
		// the dd value is negative.  hi == 0 with lo < 0 also clamps to 0.
		if (hi < 0.0) return Unsigned(0);

		// At hi == unsigned_max_d the dd may still fit in Unsigned if lo
		// brings it into range -- e.g. dd(0x1p64, -2.0) represents 2^64 - 2,
		// a valid unsigned long long. Handle via offset arithmetic from max.
		constexpr double unsigned_max_d = static_cast<double>((std::numeric_limits<Unsigned>::max)());

		if (hi > unsigned_max_d) return (std::numeric_limits<Unsigned>::max)();
		if (hi == unsigned_max_d) {
			if (lo >= 0.0) return (std::numeric_limits<Unsigned>::max)();
			double abs_lo = -lo;
			Unsigned abs_lo_int = static_cast<Unsigned>(abs_lo);
			double abs_lo_frac = abs_lo - static_cast<double>(abs_lo_int);
			Unsigned result = (std::numeric_limits<Unsigned>::max)();
			if (abs_lo_frac == 0.0) {
				return result - (abs_lo_int - 1);
			}
			return result - abs_lo_int;
		}

		// hi >= 0 and in range. Cast each limb to Unsigned (well-defined
		// since hi >= 0 and bounded; lo handled below).
		Unsigned hi_int = static_cast<Unsigned>(hi);
		double hi_frac = hi - static_cast<double>(hi_int);  // [0, 1)

		if (lo >= 0.0) {
			// Both non-negative: integer add with overflow saturation,
			// fractional carry adjustment.
			Unsigned lo_int = static_cast<Unsigned>(lo);
			double lo_frac = lo - static_cast<double>(lo_int);  // [0, 1)
			double frac_sum = hi_frac + lo_frac;                // [0, 2)

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
			// lo < 0: subtract its magnitude.  Casting a negative double
			// directly to Unsigned is UB, so cast |lo| instead.
			double abs_lo = -lo;
			Unsigned abs_lo_int = static_cast<Unsigned>(abs_lo);
			double abs_lo_frac = abs_lo - static_cast<double>(abs_lo_int);  // [0, 1)

			// dd = (hi_int + hi_frac) - (abs_lo_int + abs_lo_frac)
			if (hi_int < abs_lo_int) return Unsigned(0);
			if (hi_int == abs_lo_int) {
				// Difference is purely fractional, in (-1, 1) -- truncates to 0.
				return Unsigned(0);
			}
			Unsigned diff = hi_int - abs_lo_int;
			if (hi_frac < abs_lo_frac) {
				// Fractional borrow: result is one less than diff.
				return diff - 1;  // safe: diff >= 1
			}
			return diff;
		}
	}

public:
	// conversion operators
	explicit constexpr operator int()                   const noexcept { return convert_to_signed_impl<int>(); }
	explicit constexpr operator long()                  const noexcept { return convert_to_signed_impl<long>(); }
	explicit constexpr operator long long()             const noexcept { return convert_to_signed_impl<long long>(); }
	explicit constexpr operator unsigned int()          const noexcept { return convert_to_unsigned_impl<unsigned int>(); }
	explicit constexpr operator unsigned long()         const noexcept { return convert_to_unsigned_impl<unsigned long>(); }
	explicit constexpr operator unsigned long long()    const noexcept { return convert_to_unsigned_impl<unsigned long long>(); }
	explicit constexpr operator float()                 const noexcept { return static_cast<float>(hi + lo); }
	explicit constexpr operator double()                const noexcept { return hi + lo; }


#if LONG_DOUBLE_SUPPORT
	// long double constructor and assignment cannot be constexpr because the
	// remainder calculation requires volatile designation; conversion-out is
	// constexpr-clean (just hi + lo).
			  dd(long double iv)                    noexcept { *this = iv; }
			  dd& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }
	explicit constexpr operator long double() const noexcept { return static_cast<long double>(hi) + static_cast<long double>(lo); }
#endif

	// prefix operators
	constexpr dd operator-() const noexcept {
		return dd(-hi, -lo);
	}

	// arithmetic operators
	CONSTEXPRESSION dd& operator+=(const dd& rhs) {
		double s2;
		hi = two_sum(hi, rhs.hi, s2);
		if (is_finite_cx(hi)) {
			double t2, t1 = two_sum(lo, rhs.lo, t2);
			lo = two_sum(s2, t1, t1);
			t1 += t2;
			three_sum(hi, lo, t1);
		}
		else {
			lo = 0.0;
		}
		return *this;
	}
	CONSTEXPRESSION dd& operator+=(double rhs) {
		return operator+=(dd(rhs));
	}
	CONSTEXPRESSION dd& operator-=(const dd& rhs) {
		double s2;
		hi = two_sum(hi, -rhs.hi, s2);
		if (is_finite_cx(hi)) {
			double t2, t1 = two_sum(lo, -rhs.lo, t2);
			lo = two_sum(s2, t1, t1);
			t1 += t2;
			three_sum(hi, lo, t1);
		}
		else {
			lo = 0.0;
		}
		return *this;
	}
	CONSTEXPRESSION dd& operator-=(double rhs) {
		return operator-=(dd(rhs));
	}
	CONSTEXPRESSION dd& operator*=(const dd& rhs) {
		double p[7]{};
		//	e powers in p = 0, 1, 1, 1, 2, 2, 2
		p[0] = two_prod(hi, rhs.hi, p[1]);
		if (is_finite_cx(p[0])) {
			p[2] = two_prod(hi, rhs.lo, p[4]);
			p[3] = two_prod(lo, rhs.hi, p[5]);
			p[6] = lo * rhs.lo;

			//	e powers in p = 0, 1, 2, 3, 2, 2, 2
			three_sum(p[1], p[2], p[3]);

			//	e powers in p = 0, 1, 2, 3, 2, 3, 4
			p[2] += p[4] + p[5] + p[6];

			three_sum(p[0], p[1], p[2]);

			hi = p[0];
			lo = p[1];
		}
		else {
			hi = p[0];
			lo = 0.0;
		}
		return *this;
	}
	CONSTEXPRESSION dd& operator*=(double rhs) {
		return operator*=(dd(rhs));
	}
	CONSTEXPRESSION dd& operator/=(const dd& rhs) {
		if (isnan()) return *this;

		if (rhs.isnan()) {
			*this = rhs;
			return *this;
		}

		if (rhs.iszero()) {
			if (iszero()) {
				*this = dd(SpecificValue::qnan);
			}
			else {
				// Determine sign of the result. At runtime use std::copysign,
				// which preserves the IEEE-754 sign bit of -0.0. During
				// constant evaluation use std::bit_cast to extract the sign
				// bit directly (an ordered comparison would treat -0.0 as
				// non-negative and produce the wrong-signed infinity).
				double signA, signB;
				if (std::is_constant_evaluated()) {
					signA = (std::bit_cast<std::uint64_t>(hi)     >> 63) ? -1.0 : 1.0;
					signB = (std::bit_cast<std::uint64_t>(rhs.hi) >> 63) ? -1.0 : 1.0;
				}
				else {
					signA = std::copysign(1.0, hi);
					signB = std::copysign(1.0, rhs.hi);
				}
				*this = (signA * signB > 0.0)
					? dd(SpecificValue::infpos)
					: dd(SpecificValue::infneg);
			}
			return *this;
		}

		double q1 = hi / rhs.hi;  // approximate quotient
		if (is_finite_cx(q1)) {
			dd r = fma(-q1, rhs, *this);

			double q2 = r.hi / rhs.hi;
			r = fma(-q2, rhs, r);

			double q3 = r.hi / rhs.hi;

			three_sum(q1, q2, q3);
			hi = q1;
			lo = q2;
		}
		else {
			hi = q1;
			lo = 0.0;
		}

		return *this;
	}
	CONSTEXPRESSION dd& operator/=(double rhs) {
		return operator/=(dd(rhs));
	}

	// overloaded unary operators
	dd& operator++() {
		if ((hi == 0.0 && lo == 0.0) || sw::universal::isdenorm(hi)) {
			// move into or through the subnormal range of the high limb
			hi = std::nextafter(hi, +INFINITY);
		}
		else if (std::isfinite(hi)) {
			if (lo == 0.0) {
				int highScale = sw::universal::scale(hi);
				// the second limb cannot be a denorm, so we need to jump to the first normal value
				// in the binade that is 2^-53 below that of the high limb
				lo = std::ldexp(1.0, highScale - 53);
			}
			else {
				int lowScale = sw::universal::scale(lo);
				lo = std::nextafter(lo, +INFINITY);
				int newLowScale = sw::universal::scale(lo);
				// check for overflow: could be transitioning into the next binade, or INF in last binade
				if (lowScale < newLowScale || std::isinf(lo)) {
					lo = 0.0;
					hi = std::nextafter(hi, +INFINITY);
				}
			}
		}
		else {
			// the double-double is INF/NAN and will stay INF/NAN
		}
		return *this;
	}
	dd operator++(int) {
		dd tmp(*this);
		operator++();
		return tmp;
	}
	dd& operator--() {
		if ((hi == 0.0 && lo == 0.0) || sw::universal::isdenorm(hi)) {
			// move into or through the subnormal range of the high limb
			hi = std::nextafter(hi, -INFINITY);
		}
		else if (std::isfinite(hi)) {
			if (lo == 0.0) {
				// we need to drop into a lower binade, thus we need to update the high limb first
				hi = std::nextafter(hi, -INFINITY);
				int highScale = sw::universal::scale(hi);
				// next, the low limb needs to become the largest value 2^-53 below the new high limb
				lo = std::ldexp(0.9999999999999999, highScale - 52);  // 52 because we are all 1's and need to be one below the full shift
			}
			else {
				int lowScale = sw::universal::scale(lo);
				lo = std::nextafter(lo, -INFINITY);
				int newLowScale = sw::universal::scale(lo);
				// check for overflow
				if (lowScale < newLowScale || std::isinf(lo)) {
					lo = 0.0;
					hi = std::nextafter(hi, -INFINITY);
				}
			}
		}
		else {
			// the double-double is INF/NAN and will stay INF/NAN
		}
		return *this;
	}
	dd operator--(int) {
		dd tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	constexpr void clear()                                         noexcept { hi = 0.0; lo = 0.0; }
	constexpr void setzero()                                       noexcept { hi = 0.0; lo = 0.0; }
	constexpr void setinf(bool sign = true)                        noexcept { hi = (sign ? -INFINITY : INFINITY); lo = 0.0; }
	constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING)       noexcept { hi = (NaNType == NAN_TYPE_SIGNALLING ? std::numeric_limits<double>::signaling_NaN() : std::numeric_limits<double>::quiet_NaN()); lo = 0.0; }
	constexpr void setsign(bool sign = true)                       noexcept { if (sign && hi > 0.0) hi = -hi; }
	constexpr void set(double high, double low)                    noexcept { hi = high; lo = low; }
	// Re-establish the canonical (hi, lo) limb layout, where |lo| <= ulp(hi)/2.
	// Used by I/O / digit-extraction paths to defend against non-canonical
	// values constructed via the raw-limb constructor `dd(double, double)` or
	// via `setbits()`, both of which bypass the EFT renormalization that
	// arithmetic primitives apply.  See issue #801.
	constexpr void renorm() noexcept {
		// Skip non-finite hi -- already broken or infinity, no canonical
		// form to recover.
		if (sw::universal::is_inf_cx(hi) || hi != hi) return;
		// Skip the maxpos / maxneg boundary: when hi is at DBL_MAX and lo
		// pushes the canonical sum out of the representable range,
		// two_sum's `hi + lo` overflows to +/-infinity.  The dd value at
		// that boundary is canonical by construction (the maxpos / maxneg
		// encoders satisfy the magnitude invariant), so leaving it
		// untouched is the right behavior.
		if (sw::universal::is_inf_cx(hi + lo)) return;
		// two_sum (NOT quick_two_sum) so the canonicalization is correct
		// for arbitrary raw-limb inputs -- including |lo| > |hi| cases like
		// dd(0.0, 1e100) which the public raw-limb constructor accepts.
		// quick_two_sum's exact-residual contract requires |hi| >= |lo|.
		hi = two_sum(hi, lo, lo);
	}
	constexpr void setbit(unsigned index, bool b = true)           noexcept {
		if (index < 64) { // set bit in lower limb
			sw::universal::setbit(lo, index, b);
		}
		else if (index < 128) { // set bit in upper limb
			sw::universal::setbit(hi, index-64, b);
		}
		else {
			// NOP if index out of bounds
		}
	}
	constexpr void setbits(uint64_t value)                         noexcept {
		hi = static_cast<double>(value);
		lo = 0.0;
	}
	
	// argument is not protected for speed
	double operator[](int index) const { return (index == 0 ? hi : lo); }
	double& operator[](int index) { return (index == 0 ? hi : lo); }

	// create specific number system values of interest
	constexpr dd& maxpos() noexcept {
		hi = 1.7976931348623157e+308;
		lo = 1.9958403095347196e+292;
		return *this;
	}
	constexpr dd& minpos() noexcept {
		hi = std::numeric_limits<double>::min();
		lo = 0.0f;
		return *this;
	}
	constexpr dd& zero() noexcept {
		// the zero value
		clear();
		return *this;
	}
	constexpr dd& minneg() noexcept {
		hi = -std::numeric_limits<double>::min();
		lo = 0.0f;
		return *this;
	}
	constexpr dd& maxneg() noexcept {
		hi = -1.7976931348623157e+308;
		lo = -1.9958403095347196e+292;
		return *this;
	}

	dd& assign(const std::string& txt) {
		dd v;
		if (parse(txt, v)) *this = v;
		return *this; // Is this what we want? when the string is not valid, keep the current value?
	}

	// selectors
	constexpr bool iszero()   const noexcept { return hi == 0.0; }
	constexpr bool isone()    const noexcept { return hi == 1.0 && lo == 0.0; }
	constexpr bool ispos()    const noexcept { return hi > 0.0; }
	constexpr bool isneg()    const noexcept { return hi < 0.0; }
	BIT_CAST_CONSTEXPR bool isnan(int NaNType = NAN_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		int nan_type;
		bool isNaN = checkNaN(hi, nan_type);
		bool isNegNaN = isNaN && negative;
		bool isPosNaN = isNaN && !negative;
		return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) :
			(NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
				(NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
	}
	BIT_CAST_CONSTEXPR bool isinf(int InfType = INF_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		int inf_type;
		bool isInf = checkInf(hi, inf_type);
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

	constexpr bool sign()          const noexcept { return (hi < 0.0); }
	constexpr int  scale()         const noexcept { return sw::universal::scale(hi); }
	constexpr int  exponent()      const noexcept { return sw::universal::scale(hi); }
	constexpr double high()        const noexcept { return hi; }
	constexpr double low()         const noexcept { return lo; }

	// convert to string containing digits number of digits
	std::string to_string(std::streamsize precision = 7, std::streamsize width = 15, bool fixed = false, bool scientific = true, bool internal = false, bool left = false, bool showpos = false, bool uppercase = false, char fill = ' ') const {
		std::string s;
		bool negative = sign() ? true : false;
		int  e{ 0 };
		if (fixed && scientific) fixed = false; // scientific format takes precedence
		if (isnan()) {
			s = uppercase ? "NAN" : "nan";
			negative = false;
		}
		else {
			if (negative) {	s += '-'; } else { if (showpos) s += '+'; }

			if (isinf()) {
				s += uppercase ? "INF" : "inf";
			}
			else if (iszero()) {
				s += '0';
				if (precision > 0) {
					s += '.';
					s.append(static_cast<unsigned int>(precision), '0');
				}
			}
			else {
				int powerOfTenScale = static_cast<int>(std::log10(std::fabs(hi)));
				int integerDigits = (fixed ? (powerOfTenScale + 1) : 1);
				int nrDigits = integerDigits + static_cast<int>(precision);

				int nrDigitsForFixedFormat = nrDigits;
				if (fixed)
					nrDigitsForFixedFormat = std::max(60, nrDigits); // can be much longer than the max accuracy for double-double

				if constexpr (bTraceDecimalConversion) {
					std::cout << "powerOfTenScale  : " << powerOfTenScale << '\n';
					std::cout << "integerDigits    : " << integerDigits   << '\n';
					std::cout << "nrDigits         : " << nrDigits        << '\n';
					std::cout << "nrDigitsForFixedFormat  : " << nrDigitsForFixedFormat << '\n';
				}


				// a number in the range of [0.5, 1.0) to be printed with zero precision 
				// must be rounded up to 1 to print correctly
				if (fixed && (precision == 0) && (std::abs(hi) < 1.0)) {
					s += (std::abs(hi) >= 0.5) ? '1' : '0';
					return s;
				}

				if (fixed && nrDigits <= 0) {
					// process values that are near zero
					s += '0';
					if (precision > 0) {
						s += '.';
						s.append(static_cast<unsigned int>(precision), '0');
					}
				}
				else {
					std::vector<char> t;

					if (fixed) {
						t.resize(static_cast<size_t>(nrDigitsForFixedFormat+1));
						to_digits(t, e, nrDigitsForFixedFormat);
					}
					else {
						t.resize(static_cast<size_t>(nrDigits+1));
						to_digits(t, e, nrDigits);
					}

					if (fixed) {
						// round the decimal string
						round_string(t, nrDigits+1, &integerDigits);

						if (integerDigits > 0) {
							int i;
							for (i = 0; i < integerDigits; ++i) s += t[static_cast<unsigned>(i)];
							if (precision > 0) {
								s += '.';
								for (int j = 0; j < precision; ++j, ++i) s += t[static_cast<unsigned>(i)];
							}
						}
						else {
							s += "0.";
							if (integerDigits < 0) s.append(static_cast<size_t>(-integerDigits), '0');
							for (int i = 0; i < nrDigits; ++i) s += t[static_cast<unsigned>(i)];
						}
					}
					else {
						s += t[0ull];
						if (precision > 0) s += '.';

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
				if (std::fabs(from_string / hi) > 3.0) {

					// loop on the string, find the point, move it up one
					// don't act on the first character
					for (std::string::size_type i = 1; i < s.length(); ++i) {
						if (s[i] == '.') {
							s[i] = s[i - 1];
							s[i - 1] = '.'; // this will destroy the leading 0 when s[i==1] == '.';
							break;
						}
					}
					// BUG: the loop above, in particular s[i-1] = '.', destroys the leading 0
					// in the fixed point representation if the point is located at i = 1;
					// it also breaks the precision request as it adds a new digit to the fixed representation

					from_string = atof(s.c_str());
					// if this ratio is large, then the string has not been fixed
					if (std::fabs(from_string / hi) > 3.0) {
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
			}
			else if (left) {
				s.append(nrCharsToFill, fill);
			}
			else {
				s.insert(static_cast<std::string::size_type>(0), nrCharsToFill, fill);
			}
		}

		return s;
	}

protected:
	double hi, lo;

	// HELPER methods

	constexpr dd& convert_signed(int64_t v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			hi = static_cast<double>(v);
			lo = 0.0;
		}
		return *this;
	}

	constexpr dd& convert_unsigned(uint64_t v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			hi = static_cast<double>(v);
			lo = 0.0;
		}
		return *this;
	}

	// no need to SFINAE this as it is an internal method that we ONLY call when we know the argument type is a native float
	constexpr dd& convert_ieee754(float rhs) noexcept {
		hi = double(rhs);
		lo = 0.0;
		return *this;
	}
	constexpr dd& convert_ieee754(double rhs) noexcept {
		hi = rhs;
		lo = 0.0;
		return *this;
	}
#if LONG_DOUBLE_SUPPORT
	dd& convert_ieee754(long double rhs) {
		volatile long double truncated = static_cast<long double>(double(rhs));
		volatile double remainder = static_cast<double>(rhs - truncated);
		hi = static_cast<double>(truncated);
		lo = remainder;
		return *this;
	}
#endif

	// (convert_to_unsigned_impl / convert_to_signed_impl moved up next to the
	// conversion operators that call them, so the constexpr call chain is
	// fully resolved at the point of declaration.)

	// precondition: string s must be all digits
	void round_string(std::vector<char>& s, int precision, int* decimalPoint) const {
		if constexpr(bTraceDecimalRounding) {
			std::cout << "string       : " << s << '\n';
			std::cout << "precision    : " << precision << '\n';
			std::cout << "decimalPoint : " << *decimalPoint << '\n';
		}

		int nrDigits = precision;
		// round decimal string and propagate carry
		int lastDigit = nrDigits - 1;
		if (s[static_cast<unsigned>(lastDigit)] >= '5') {
			if constexpr(bTraceDecimalRounding) std::cout << "need to round\n";
			int i = nrDigits - 2;
			s[static_cast<unsigned>(i)]++;
			while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
				s[static_cast<unsigned>(i)] -= 10;
				s[static_cast<unsigned>(--i)]++;
			}
		}

		// if first digit is 10, shift everything.
		if (s[0] > '9') {
			if constexpr(bTraceDecimalRounding) std::cout << "shift right to handle overflow\n";
			for (int i = precision; i >= 2; --i) s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
			s[0u] = '1';
			s[1u] = '0';

			(*decimalPoint)++; // increment decimal point
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
	//void to_digits(char* s, int& exponent, int precision) const {
	void to_digits(std::vector<char>& s, int& exponent, int precision) const {
		constexpr dd _one(1.0), _ten(10.0);
		constexpr double _log2(0.301029995663981);

		// Canonicalize before all magnitude-dependent checks.  iszero()
		// only inspects hi, so for raw-limb inputs like dd(0.0, 1e100)
		// the pre-renorm hi==0 would short-circuit to "0" output even
		// though the value is 1e100.  See issue #801.
		dd r = abs(*this);
		r.renorm();

		if (r.iszero()) {
			exponent = 0;
			for (int i = 0; i < precision; ++i) s[static_cast<unsigned>(i)] = '0';
			return;
		}
		// First determine the (approximate) exponent FROM THE RENORMALIZED
		// pair.  Computing it from this->hi pre-renorm misses the case
		// where renorm promotes a previously-non-leading limb into r.high()
		// (e.g. dd(0.0, 1e100) where the original hi is 0 but the canonical
		// representation has hi ~1e100).  The single-step `e++ / e--`
		// correction below cannot recover from a multi-decade shift.
		int e;
		(void)std::frexp(r.high(), &e);  // Only need exponent, not mantissa
		--e; // adjust e as frexp gives a binary e that is 1 too big
		e = static_cast<int>(_log2 * e); // estimate the power of ten exponent
		if (e < 0) {
			if (e < -300) {
				r = dd(std::ldexp(r.high(), 53), std::ldexp(r.low(), 53));
				r *= pown(_ten, -e);
				r = dd(std::ldexp(r.high(), -53), std::ldexp(r.low(), -53));
			}
			else {
				r *= pown(_ten, -e);
			}
		}
		else {
			if (e > 0) {
				if (e > 300) {
					r = dd(std::ldexp(r.high(), -53), std::ldexp(r.low(), -53));
					r /= pown(_ten, e);
					r = dd(std::ldexp(r.high(), 53), std::ldexp(r.low(), 53));
				}
				else {
					r /= pown(_ten, e);
				}
			}
		}

		// Fix exponent if we have gone too far
		if (r >= _ten) {
			r /= _ten;
			++e;
		}
		else {
			if (r < 1.0) {
				r *= _ten;
				--e;
			}
		}

		if ((r >= _ten) || (r < _one)) {
			std::cerr << "to_digits() failed to compute exponent\n";
			return;
		}

		// at this point the value is normalized to a decimal value between (0, 10).
		// Defensive NaN guard: even after r.renorm() at entry, the iterative
		// subtraction / multiplication in this loop can drift r.hi to NaN
		// for extreme input magnitudes.  Casting NaN to int is C++20
		// [conv.fpint] UB; coerce NaN to 0 to keep the cast well-defined.
		// See issue #801 / mirror of the qd guard at qd_impl.hpp:1228.
		int nrDigits = precision + 1;
		for (int i = 0; i < nrDigits; ++i) {
			double v = r.hi;
			int mostSignificantDigit = (v != v) ? 0 : static_cast<int>(v);
			r -= mostSignificantDigit;
			r *= 10.0;

			s[static_cast<unsigned>(i)] = static_cast<char>(mostSignificantDigit + '0');
			if constexpr (bTraceDecimalConversion) std::cout << "to_digits  digit[" << i << "] : " << s << '\n';
		}

		// Fix out of range digits
		for (int i = nrDigits - 1; i > 0; --i) {
			if (s[static_cast<unsigned>(i)] < '0') {
				s[static_cast<unsigned>(i - 1)]--;
				s[static_cast<unsigned>(i)] += 10;
			}
			else {
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
		exponent = e;
	}

private:

	// dd - dd logic comparisons
	friend constexpr bool operator==(const dd& lhs, const dd& rhs);
	friend constexpr bool operator!=(const dd& lhs, const dd& rhs);
	friend constexpr bool operator<=(const dd& lhs, const dd& rhs);
	friend constexpr bool operator>=(const dd& lhs, const dd& rhs);
	friend constexpr bool operator<(const dd& lhs, const dd& rhs);
	friend constexpr bool operator>(const dd& lhs, const dd& rhs);

	// dd - literal logic comparisons
	friend constexpr bool operator==(const dd& lhs, const double rhs);

	// literal - dd logic comparisons
	friend constexpr bool operator==(const double lhs, const dd& rhs);

};

////////////////////////  precomputed constants of note  /////////////////////////////////

constexpr int dd_max_precision = 106; // in bits

// simple constants
constexpr dd dd_third(0.33333333333333331, 1.8503717077085941e-17);

constexpr double dd_eps = 4.93038065763132e-32;  // 2^-104
constexpr double dd_min_normalized = 2.0041683600089728e-292;  // = 2^(-1022 + 53)
constexpr dd dd_max     (1.79769313486231570815e+308, 9.97920154767359795037e+291);
constexpr dd dd_safe_max(1.7976931080746007281e+308, 9.97920154767359795037e+291);

////////////////////////    math functions   /////////////////////////////////

inline dd ulp(const dd& a) {
	double hi{ a.high() };
	double lo{ a.low() };
	double nlo;
	if (lo == 0.0) {
		nlo = std::numeric_limits<double>::epsilon() / 2.0;
		int binaryExponent = scale(hi) - 53;
		nlo /= std::pow(2.0, -binaryExponent);
	}
	else {
		nlo = (hi < 0.0 ? std::nextafter(lo, -INFINITY) : std::nextafter(lo, +INFINITY));
	}
	dd n(hi, nlo);

	return n - a;
}

inline dd abs(dd a) {
	double hi = a.high();
	double lo = a.low();
	if (hi < 0) { // flip the pair with respect to 0
		hi = -hi;
		lo = -lo;
	}
	return dd(hi, lo);
}

inline dd ceil(const dd& a)
{
	if (a.isnan()) return a;

	double hi = std::ceil(a.high());
	double lo = 0.0;

	if (hi == a.high())	{ // High segment was already an integer, thus just round the low segment
		lo = std::ceil(a.low());
		hi = quick_two_sum(hi, lo, lo);
	}

	return dd(hi, lo);
}

inline dd floor(const dd& a) {
	if (a.isnan()) return a;

	double hi = std::floor(a.high());
	double lo = 0.0;

	if (hi == a.high()) {
		// High word is integer already.  Round the low word.
		//
		lo = std::floor(a.low());
		hi = quick_two_sum(hi, lo, lo);
	}

	return dd(hi, lo);
}

// Round to Nearest integer
inline dd nint(const dd& a) {
	double hi = nint(a.high());
	double lo;

	if (hi == a.high()) {
		/* High word is an integer already.  Round the low word.*/
		lo = nint(a.low());

		/* Renormalize. This is needed if x[0] = some integer, x[1] = 1/2.*/
		hi = quick_two_sum(hi, lo, lo);
	}
	else {
		/* High word is not an integer. */
		lo = 0.0;
		if (std::abs(hi - a.high()) == 0.5 && a.low() < 0.0) {
			/* There is a tie in the high word, consult the low word
			   to break the tie. */
			hi -= 1.0;      /* NOTE: This does not cause INEXACT. */
		}
	}

	return dd(hi, lo);
}

// double plus double yielding a double-double
inline dd add(double a, double b) {
	if (std::isnan(a) || std::isnan(b)) return dd(SpecificValue::snan);
	double s, e;
	s = two_sum(a, b, e);
	return dd(s, e);
}

// double minus double yielding a double-double
inline dd sub(double a, double b) {
	if (std::isnan(a) || std::isnan(b)) return dd(SpecificValue::snan);
	double s, e;
	s = two_sum(a, -b, e);
	return dd(s, e);
}

// double times double yielding a double-double
inline dd mul(double a, double b) {
	if (std::isnan(a) || std::isnan(b)) return dd(SpecificValue::snan);
	double p, e;
	p = two_prod(a, b, e);
	return dd(p, e);
}

// double divide by double yielding a double-double
inline dd div(double a, double b) {
	if (std::isnan(a) || std::isnan(b)) return dd(SpecificValue::snan);

	if (b == 0.0) return (sign(a) ? dd(SpecificValue::infneg) : dd(SpecificValue::infpos));

	double q1 = a / b; // initial approximation

	// Compute residual: a - q1 * b
	double p2;
	double p1 = two_prod(q1, b, p2);
	double e;
	double s = two_diff(a, p1, e);
	e -= p2;

	// get next approximation
	double q2 = (s + e) / b;

	//	normalize
	s = quick_two_sum(q1, q2, e);
	return dd(s, e);
}

// double-double * double, where double is a power of 2
constexpr inline dd mul_pwr2(const dd& a, double b) {
	return dd(a.high() * b, a.low() * b);
}

// quad-double operators

// quad-double + double-double
constexpr inline void qd_add(double const a[4], const dd& b, double s[4]) {
	double t[5]{};
	s[0] = two_sum(a[0], b.high(), t[0]);		//	s0 - O( 1 ); t0 - O( e )
	s[1] = two_sum(a[1], b.low(), t[1]);		//	s1 - O( e ); t1 - O( e^2 )

	s[1] = two_sum(s[1], t[0], t[0]);		//	s1 - O( e ); t0 - O( e^2 )

	s[2] = a[2];									//	s2 - O( e^2 )
	three_sum(s[2], t[0], t[1]);		//	s2 - O( e^2 ); t0 - O( e^3 ); t1 = O( e^4 )

	s[3] = two_sum(a[3], t[0], t[0]);			//	s3 - O( e^3 ); t0 - O( e^4 )
	t[0] += t[1];									//	fl( t0 + t1 ) - accuracy less important

	renorm(s[0], s[1], s[2], s[3], t[0]);
}

// quad-double = double-double * double-double
constexpr inline void qd_mul(const dd& a, const dd& b, double p[4]) {
	double p4{}, p5{}, p6{}, p7{};

	//	powers of e - 0, 1, 1, 1, 2, 2, 2, 3
	p[0] = two_prod(a.high(), b.high(), p[1]);
	if (is_finite_cx(p[0])) {
		p[2] = two_prod(a.high(), b.low(), p4);
		p[3] = two_prod(a.low(), b.high(), p5);
		p6 = two_prod(a.low(), b.low(), p7);

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
	}
	else {
		p[1] = p[2] = p[3] = 0.0;
	}
}

constexpr inline dd fma(const dd& a, const dd& b, const dd& c) {
	double p[4]{};
	qd_mul(a, b, p);
	qd_add(p, c, p);
	p[0] = two_sum(p[0], p[1] + p[2] + p[3], p[1]);
	return dd(p[0], p[1]);
}

constexpr inline dd sqr(const dd& a) {
	if (a.isnan()) return a;

	double p2, p1 = two_sqr(a.high(), p2);
	p2 += 2.0 * a.high() * a.low();
	p2 += a.low() * a.low();

	double s2{ 0 }, s1 = quick_two_sum(p1, p2, s2);
	return dd(s1, s2);
}

constexpr inline dd reciprocal(const dd& a) {
	if (a.iszero()) return dd(SpecificValue::infpos);

	if (a.isinf()) return dd(0.0);

	double q1 = 1.0 / a.high();  /* approximate quotient */
	if (is_finite_cx(q1)) {
		dd r = fma(-q1, a, 1.0);

		double q2 = r.high() / a.high();
		r = fma(-q2, a, r);

		double q3 = r.high() / a.high();
		three_sum(q1, q2, q3);
		return dd(q1, q2);
	}
	else {
		return dd(q1, 0.0);
	}
}

/////////////////////////////////////////////////////////////////////////////
//	power functions

inline dd pown(const dd& a, int n) {
	if (a.isnan()) return a;

	int N = (n < 0) ? -n : n;
	dd s;

	switch (N) {
	case 0:
		if (a.iszero()) {
			std::cerr << "pown: invalid argument\n";
			errno = EDOM;
			return dd(SpecificValue::qnan);
		}
		return dd(1.0);

	case 1:
		s = a;
		break;

	case 2:
		s = sqr(a);
		break;

	default: // Use binary exponentiation
	{
		dd r{ a };

		s = 1.0;
		while (N > 0) {
			if (N % 2 == 1) {
				s *= r;
			}
			N /= 2;
			if (N > 0) r = sqr(r);
		}
	}
	break;
	}

	// Compute the reciprocal if n is negative.
	return n < 0 ? reciprocal(s) : s;
}

////////////////////////  stream operators   /////////////////////////////////

// stream out a decimal floating-point representation of the double-double
inline std::ostream& operator<<(std::ostream& ostr, const dd& v) {
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

// stream in an ASCII decimal floating-point format and assign it to a double-double
inline std::istream& operator>>(std::istream& istr, dd& v) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a double-double value\n";
	}
	return istr;
}

////////////////// string operators

// parse a decimal ASCII floating-point format and make a doubledouble (dd) out of it
inline bool parse(const std::string& number, dd& value) {
	char const* p = number.c_str();

	// Skip any leading spaces
	while (std::isspace(*p)) ++p;

	dd r{ 0.0 };
	int nrDigits{ 0 };
	int decimalPoint{ -1 };
	int sign{ 0 }, eSign{ 1 };
	int e{ 0 };
	bool done{ false }, parsingMantissa{ true };
	char ch;
	while (!done && (ch = *p) != '\0') {
		if (std::isdigit(ch)) {
			if (parsingMantissa) {
				int digit = ch - '0';
				r *= 10.0;
				r += static_cast<double>(digit);
				++nrDigits;
			}
			else { // parsing exponent section
				int digit = ch - '0';
				e *= 10;
				e += digit;
			}
		}
		else {
			switch (ch) {
			case '.':
				if (decimalPoint >= 0) return false;
				decimalPoint = nrDigits;
				break;

			case '-':
			case '+':
				if (parsingMantissa) {
					if (sign != 0 || nrDigits > 0) return false;
					sign = (ch == '-' ? -1 : 1);
				}
				else {
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

	if (decimalPoint >= 0) e -= (nrDigits - decimalPoint);
	dd _ten(10.0, 0.0);
	if (e > 0) {
		r *= pown(_ten, e);
	}
	else {
		if (e < 0) r /= pown(_ten, -e);
	}
	value = (sign == -1) ? -r : r;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - dd binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
constexpr bool operator==(const dd& lhs, const dd& rhs) {
	return (lhs.hi == rhs.hi) && (lhs.lo == rhs.lo);
}

constexpr bool operator!=(const dd& lhs, const dd& rhs) {
	return !operator==(lhs, rhs);
}

constexpr bool operator< (const dd& lhs, const dd& rhs) {
	if (lhs.hi < rhs.hi) {
		return true;
	}
	else if (lhs.hi > rhs.hi) {
		return false;
	}
	else {
		// hi limbs are the same
		if (lhs.lo < rhs.lo) {
			return true;
		}
		else if (lhs.lo > rhs.lo) {
			return false;
		}
		else {
			// lhs and rhs are the same
			return false;
		}
	}
}

constexpr bool operator> (const dd& lhs, const dd& rhs) {
	return operator< (rhs, lhs);
}

constexpr bool operator<=(const dd& lhs, const dd& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

constexpr bool operator>=(const dd& lhs, const dd& rhs) {
	// NOT !operator<: that would be true for unordered (NaN) operands,
	// violating IEEE-754. Use operator> || operator== so NaN stays unordered.
	return operator>(lhs, rhs) || operator==(lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - literal binary logic operators
// equal: precondition is that the byte-storage is properly nulled in all arithmetic paths
constexpr bool operator==(const dd& lhs, double rhs) {
	return operator==(lhs, dd(rhs));
}

constexpr bool operator!=(const dd& lhs, double rhs) {
	return !operator==(lhs, rhs);
}

constexpr bool operator< (const dd& lhs, double rhs) {
	return operator<(lhs, dd(rhs));
}

constexpr bool operator> (const dd& lhs, double rhs) {
	return operator< (dd(rhs), lhs);
}

constexpr bool operator<=(const dd& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

constexpr bool operator>=(const dd& lhs, double rhs) {
	return operator>(lhs, rhs) || operator==(lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dd binary logic operators
// precondition is that the byte-storage is properly nulled in all arithmetic paths

constexpr bool operator==(double lhs, const dd& rhs) {
	return operator==(dd(lhs), rhs);
}

constexpr bool operator!=(double lhs, const dd& rhs) {
	return !operator==(lhs, rhs);
}

constexpr bool operator< (double lhs, const dd& rhs) {
	return operator<(dd(lhs), rhs);
}

constexpr bool operator> (double lhs, const dd& rhs) {
	return operator< (rhs, lhs);
}

constexpr bool operator<=(double lhs, const dd& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

constexpr bool operator>=(double lhs, const dd& rhs) {
	return operator>(lhs, rhs) || operator==(lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - dd binary arithmetic operators
// BINARY ADDITION
constexpr dd operator+(const dd& lhs, const dd& rhs) {
	dd sum = lhs;
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
constexpr dd operator-(const dd& lhs, const dd& rhs) {
	dd diff = lhs;
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
constexpr dd operator*(const dd& lhs, const dd& rhs) {
	dd mul = lhs;
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
constexpr dd operator/(const dd& lhs, const dd& rhs) {
	dd ratio = lhs;
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// dd - literal binary arithmetic operators
// BINARY ADDITION
constexpr dd operator+(const dd& lhs, double rhs) {
	return operator+(lhs, dd(rhs));
}
// BINARY SUBTRACTION
constexpr dd operator-(const dd& lhs, double rhs) {
	return operator-(lhs, dd(rhs));
}
// BINARY MULTIPLICATION
constexpr dd operator*(const dd& lhs, double rhs) {
	return operator*(lhs, dd(rhs));
}
// BINARY DIVISION
constexpr dd operator/(const dd& lhs, double rhs) {
	return operator/(lhs, dd(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - dd binary arithmetic operators
// BINARY ADDITION
constexpr dd operator+(double lhs, const dd& rhs) {
	return operator+(dd(lhs), rhs);
}
// BINARY SUBTRACTION
constexpr dd operator-(double lhs, const dd& rhs) {
	return operator-(dd(lhs), rhs);
}
// BINARY MULTIPLICATION
constexpr dd operator*(double lhs, const dd& rhs) {
	return operator*(dd(lhs), rhs);
}
// BINARY DIVISION
constexpr dd operator/(double lhs, const dd& rhs) {
	return operator/(dd(lhs), rhs);
}

}} // namespace sw::universal
