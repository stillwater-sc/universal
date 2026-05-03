#pragma once
// qd_impl.hpp: implementation of the double-double floating-point number system described in
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
// qd exception structure
#include <universal/number/qd/exceptions.hpp>
#include <universal/number/qd/qd_fwd.hpp>

namespace sw { namespace universal {

// fwd references to free functions
constexpr qd operator+(const qd&, const qd&);
constexpr qd operator-(const qd&, const qd&);
constexpr qd operator*(const qd&, const qd&);
constexpr qd operator/(const qd&, const qd&);
inline std::ostream& operator<<(std::ostream&, const qd&);
inline qd pown(const qd&, int);
inline qd frexp(const qd&, int*);
inline qd ldexp(const qd&, int);

// qd is an unevaluated quadruple of IEEE-754 doubles that provides a (1,11,212) floating-point triple
class qd {
public:
	static constexpr unsigned nbits = 256;
	static constexpr unsigned es = 11;
	static constexpr unsigned fbits = 212; // number of fraction digits
	// exponent characteristics are the same as native double precision floating-point
	static constexpr int      EXP_BIAS = ((1 << (es - 1u)) - 1l);
	static constexpr int      MAX_EXP = (es == 1) ? 1 : ((1 << es) - EXP_BIAS - 1);
	static constexpr int      MIN_EXP_NORMAL = 1 - EXP_BIAS;
	static constexpr int      MIN_EXP_SUBNORMAL = 1 - EXP_BIAS - int(fbits); // the scale of smallest ULP

	/// trivial constructor
	qd() = default;

	qd(const qd&) = default;
	qd(qd&&) = default;

	qd& operator=(const qd&) = default;
	qd& operator=(qd&&) = default;

	// converting constructors
	qd(const std::string& stringRep) : x{0} { assign(stringRep); }

	// specific value constructor
	constexpr qd(const SpecificValue code) noexcept : x{0.0} {
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

	// raw limb constructor: no argument checking
	constexpr qd(double x0)                         noexcept : x{ 0 } { x[0] = x0; }
	constexpr qd(double x0, double x1)              noexcept : x{ 0 } { x[0] = x0; x[1] = x1; }
	constexpr qd(double x0, double x1, double x2, double x3) noexcept : x{ 0 } { x[0] = x0; x[1] = x1; x[2] = x2; x[3] = x3; }

	// initializers for native types
	constexpr qd(signed char iv)                    noexcept : x{0} { x[0] = static_cast<double>(iv); }
	constexpr qd(short iv)                          noexcept : x{0} { x[0] = static_cast<double>(iv); }
	constexpr qd(int iv)                            noexcept : x{0} { x[0] = static_cast<double>(iv); }
	constexpr qd(long iv)                           noexcept { *this = iv; }
	constexpr qd(long long iv)                      noexcept { *this = iv; }
	constexpr qd(char iv)                           noexcept : x{0} { x[0] = static_cast<double>(iv); }
	constexpr qd(unsigned short iv)                 noexcept : x{0} { x[0] = static_cast<double>(iv); }
	constexpr qd(unsigned int iv)                   noexcept : x{0} { x[0] = static_cast<double>(iv); }
	constexpr qd(unsigned long iv)                  noexcept { *this = iv; }
	constexpr qd(unsigned long long iv)             noexcept { *this = iv; }
	constexpr qd(float iv)                          noexcept : x{0} { x[0] = iv; }

	// assignment operators for native types
	constexpr qd& operator=(signed char rhs)        noexcept { return convert_signed(rhs); }
	constexpr qd& operator=(short rhs)              noexcept { return convert_signed(rhs); }
	constexpr qd& operator=(int rhs)                noexcept { return convert_signed(rhs); }
	constexpr qd& operator=(long rhs)               noexcept { return convert_signed(rhs); }
	constexpr qd& operator=(long long rhs)          noexcept { return convert_signed(rhs); }
	constexpr qd& operator=(unsigned char rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr qd& operator=(unsigned short rhs)     noexcept { return convert_unsigned(rhs); }
	constexpr qd& operator=(unsigned int rhs)       noexcept { return convert_unsigned(rhs); }
	constexpr qd& operator=(unsigned long rhs)      noexcept { return convert_unsigned(rhs); }
	constexpr qd& operator=(unsigned long long rhs) noexcept { return convert_unsigned(rhs); }
	constexpr qd& operator=(float rhs)              noexcept { return convert_ieee754(rhs); }
	constexpr qd& operator=(double rhs)             noexcept { return convert_ieee754(rhs); }

private:
	// Helper templates used by the conversion operators below.  Defined before
	// the operators so the constexpr call chain is fully resolved at the point
	// of operator declaration (clang requires this; gcc is more permissive).
	//
	// For quad-double, collapse the lower three limbs (x[1] + x[2] + x[3])
	// into a single representative double via a two_sum chain (sub-ULP error
	// discarded -- cannot affect integer truncation), then apply the dd-style
	// limb-separated truncation algorithm from #796.  Extends the
	// limb-separated approach to the full 212-bit range.
	template<typename Unsigned>
	constexpr Unsigned convert_to_unsigned_impl() const noexcept {
		const double hi = x[0];
		double e1 = 0.0;
		double e23 = 0.0;
		// Collapse x[2] + x[3] first, then add x[1].  error_free_ops::two_sum
		// returns the rounded sum and writes the residual via the third arg.
		double s23 = two_sum(x[2], x[3], e23);
		double s1  = two_sum(x[1], s23, e1);
		(void)e1; (void)e23;
		const double lo = s1;

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
		const double hi = x[0];
		double e1 = 0.0;
		double e23 = 0.0;
		double s23 = two_sum(x[2], x[3], e23);
		double s1  = two_sum(x[1], s23, e1);
		(void)e1; (void)e23;
		const double lo = s1;

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
	explicit constexpr operator float()                 const noexcept { return static_cast<float>(x[0] + x[1] + x[2] + x[3]); }
	explicit constexpr operator double()                const noexcept { return x[0] + x[1] + x[2] + x[3]; }


#if LONG_DOUBLE_SUPPORT
	// long double constructor and assignment cannot be constexpr because the
	// remainder calculation requires volatile designation; conversion-out is
	// constexpr-clean.
			  qd(long double iv)                    noexcept { *this = iv; }
			  qd& operator=(long double rhs)        noexcept { return convert_ieee754(rhs); }
	explicit constexpr operator long double() const noexcept { return static_cast<long double>(x[0]) + static_cast<long double>(x[1]) + static_cast<long double>(x[2]) + static_cast<long double>(x[3]); }
#endif

	// prefix operators
	constexpr qd operator-() const noexcept {
		return qd(-x[0], -x[1], -x[2], -x[3]);
	}

	// arithmetic operators
#define IEEE_ERROR_BOUND 1
	CONSTEXPRESSION qd& operator+=(const qd& rhs) {
#if defined(IEEE_ERROR_BOUND)
		return *this = accurate_addition(*this, rhs);
#else // !IEEE_ERROR_BOUND -> CRAY_ERROR_BOUND
		return *this = approximate_addition(*this, rhs);
#endif
	}
	CONSTEXPRESSION qd& operator+=(double rhs) {
		return operator+=(qd(rhs));
	}
	CONSTEXPRESSION qd& operator-=(const qd& rhs) {
		return *this += -rhs;
	}
	CONSTEXPRESSION qd& operator-=(double rhs) {
		return *this += qd(-rhs);
	}
#define ACCURATE_MULTIPLICATION 1
	CONSTEXPRESSION qd& operator*=(const qd& rhs) {
#if defined(ACCURATE_MULTIPLICATION)
		return *this = accurate_multiplication(*this, rhs);
#else
		return *this = approximate_multiplication(*this, rhs);
#endif
	}
	CONSTEXPRESSION qd& operator*=(double rhs) {
		double q0, q1, q2;

		double p0 = two_prod(x[0], rhs, q0);
		double p1 = two_prod(x[1], rhs, q1);
		double p2 = two_prod(x[2], rhs, q2);
		double p3 = x[3] * rhs;

		double s0 = p0;
		double s2;
		double s1 = two_sum(q0, p1, s2);

		three_sum(s2, q1, p2);

		three_sum2(q1, q2, p3);
		double s3 = q1;

		double s4 = q2 + p2;

		sw::universal::renorm(s0, s1, s2, s3, s4);
		x[0] = s0;
		x[1] = s1;
		x[2] = s2;
		x[3] = s3;
		return *this;
	}
#define ACCURATE_DIVISION 1
	CONSTEXPRESSION qd& operator/=(const qd& rhs) {
#if QUADDOUBLE_THROW_ARITHMETIC_EXCEPTION
		if (isnan()) throw qd_not_a_number();
		if (rhs.isnan()) throw qd_divide_by_nan();
		if (rhs.iszero()) {
			if (iszero()) throw qd_not_a_number();
			throw qd_divide_by_zero();
		}
#else
		if (isnan()) return *this;

		if (rhs.isnan()) {
			*this = rhs;
			return *this;
		}

		if (rhs.iszero()) {
			if (iszero()) {
				*this = qd(SpecificValue::qnan);
			}
			else {
				// Determine sign of result.  Both branches must extract the
				// IEEE-754 sign bit so -0.0 carries through correctly: at
				// runtime use std::signbit (which inspects the bit), during
				// constant evaluation use std::bit_cast (since std::signbit
				// is not constexpr in C++20).  Per #727 / #797 / #798 lessons.
				int sA, sB;
				if (std::is_constant_evaluated()) {
					sA = (std::bit_cast<std::uint64_t>(x[0])     >> 63) ? -1 : 1;
					sB = (std::bit_cast<std::uint64_t>(rhs.x[0]) >> 63) ? -1 : 1;
				}
				else {
					sA = std::signbit(x[0])     ? -1 : 1;
					sB = std::signbit(rhs.x[0]) ? -1 : 1;
				}
				*this = ((sA == sB) ? qd(SpecificValue::infpos) : qd(SpecificValue::infneg));
			}
			return *this;
		}
#endif

#if ACCURATE_DIVISION
		return *this = accurate_division(*this, rhs);
#else
		return *this = approximate_division(*this, rhs);
#endif
	}
	CONSTEXPRESSION qd& operator/=(double rhs) {
		return operator/=(qd(rhs));
	}

	// overloaded unary operators

	
	/// <summary>
	/// overloaded increment operator that find the next valid quad so that x_next - x = ulp(x)
	/// A quad-double number is an unevaluated sum of four IEEE double numbers.
	/// The quad-double (a0 a1 a2 a3) represents the exact sum a = a0 + a1 + a2 + a3.
	/// Note that for any given representable number x, there can be many representations
	/// as an unevaluated sum of four doubles.
	/// Hence we require that the quadruple(a0 a1 a2 a3) to satisfy
	///  a_(i+1) leq ulp(a_i) / 2 
	/// for i=0, 1, 2, with equality only occuring when a_i = 0, or the last bit of a_i is 0
	/// Note that the first a0 is the double precision approximation of the quad-double number,
	/// accurate to almost half an ulp.
	/// </summary>
	/// <returns>a reference to *this</returns>
	qd& operator++() {
		if ((x[0] == 0.0 && x[1] == 0.0 && x[2] == 0.0 && x[3] == 0.0) || sw::universal::isdenorm(x[0])) {
			// move into or through the subnormal range of the high limb
			x[0] = std::nextafter(x[0], +INFINITY);
			x[1] = x[2] = x[3] = 0.0; // just in case something messes up the canonical form
		}
		else if (std::isfinite(x[0])) {
			if (x[1] == 0.0) {
				int highScale = sw::universal::scale(x[0]);
				// the second limb cannot be a denorm, so we need to jump to the first normal value
				// in the binade that is 2^-159 below that of the high limb
				x[1] = std::ldexp(1.0, highScale - 159);
				x[2] = x[3] = 0.0;
				// how do we enforce the constraint: a_(i+1) leq ulp(a_i) / 2 for i=0,1,2? TODO
			}
			else {
				// enforce that the leading double-double is the approximation of the quad-double
				int currentScale = sw::universal::scale(x[1]);
				x[1] = std::nextafter(x[1], +INFINITY);
				int nextScale = sw::universal::scale(x[1]);
				// check for overflow: could be transitioning into the next binade
				if (currentScale < nextScale) {
					x[0] = std::nextafter(x[0], +INFINITY);
					x[1] = 0.0;
				}
			}
		}
		else {
			// the quad-double is INF/NAN and will stay INF/NAN
		}
		return *this;
	}
	qd operator++(int) {
		qd tmp(*this);
		operator++();
		return tmp;
	}
	qd& operator--() {
		if ((x[0] == 0.0 && x[1] == 0.0 && x[2] == 0.0 && x[3] == 0.0) || sw::universal::isdenorm(x[0])) {
			// move into or through the subnormal range of the high limb
			x[0] = std::nextafter(x[0], -INFINITY);
		}
		else if (std::isfinite(x[0])) {
			if (x[1] == 0.0) {
				// we need to drop into a lower binade, thus we need to update the high limb first
				x[0] = std::nextafter(x[0], -INFINITY);
				int highScale = sw::universal::scale(x[0]);
				// next, the low limb needs to become the largest value 2^-159 below the new high limb
				x[1] = std::ldexp(0.9999999999999999, highScale - 52);  // 52 because we are all 1's and need to be one below the full shift
				x[2] = std::ldexp(0.9999999999999999, highScale - 105);
				x[3] = std::ldexp(0.9999999999999999, highScale - 158);
			}
			else {
				int currentScale = sw::universal::scale(x[1]);
				x[1] = std::nextafter(x[1], -INFINITY);
				int nextScale = sw::universal::scale(x[1]);
				// check for overflow
				if (currentScale < nextScale) {
					x[1] = 0.0;
					x[0] = std::nextafter(x[0], -INFINITY);
				}
			}
		}
		else {
			// the double-double is INF/NAN and will stay INF/NAN
		}
		return *this;
	}
	qd operator--(int) {
		qd tmp(*this);
		operator--();
		return tmp;
	}

	// modifiers
	constexpr void clear()                                         noexcept { x[0] = 0.0; x[1] = 0.0; x[2] = 0.0; x[3] = 0.0; }
	constexpr void setzero()                                       noexcept { x[0] = 0.0; x[1] = 0.0; x[2] = 0.0; x[3] = 0.0; }
	constexpr void setinf(bool sign = true)                        noexcept { x[0] = (sign ? -INFINITY : INFINITY); x[1] = 0.0; x[2] = 0.0; x[3] = 0.0; }
	constexpr void setnan(int NaNType = NAN_TYPE_SIGNALLING)       noexcept { x[0] = (NaNType == NAN_TYPE_SIGNALLING ? std::numeric_limits<double>::signaling_NaN() : std::numeric_limits<double>::quiet_NaN()); x[1] = 0.0; x[2] = 0.0; x[3] = 0.0; }
	constexpr void setsign(bool sign = true)                       noexcept { if (sign && x[0] > 0.0) x[0] = -x[0]; }

	constexpr void setbit(unsigned index, bool b = true)           noexcept {
		if (index < 64) { // set bit in lowest limb
			sw::universal::setbit(x[3], index, b);
		}
		else if (index < 128) { // set bit in second to lowest limb
			sw::universal::setbit(x[2], index - 64, b);
		}
		else if (index < 192) { // set bit in second to upper limb
			sw::universal::setbit(x[1], index - 128, b);
		}
		else if (index < 128) { // set bit in upper limb
			sw::universal::setbit(x[0], index - 192, b);
		}
		else {
			// NOP if index out of bounds
		}
	}
	constexpr void setbits(uint64_t value)                         noexcept {
		x[0] = static_cast<double>(value);
		x[1] = 0.0; x[2] = 0.0; x[3] = 0.0;
	}
	
	void renorm() noexcept {
		sw::universal::renorm(x[0], x[1], x[2], x[3]);
	}
	void renorm(double r) noexcept {
		sw::universal::renorm(x[0], x[1], x[2], x[3], r);
	}

	// argument is not protected for speed
	constexpr double operator[](int index) const { return x[index]; }
	constexpr double& operator[](int index)      { return x[index]; }

	// create specific number system values of interest
	constexpr qd& maxpos() noexcept {
		x[0] = std::numeric_limits<double>::max(); 
		x[1] = 0.0; 
		x[2] = 0.0; 
		x[3] = 0.0;
		return *this;
	}
	// smallest positive normal number
	constexpr qd& minpos() noexcept {
		x[0] = std::numeric_limits<double>::min();
		x[1] = 0.0;
		x[2] = 0.0;
		x[3] = 0.0;
		return *this;
	}
	constexpr qd& zero() noexcept {
		x[0] = 0.0;
		x[1] = 0.0;
		x[2] = 0.0;
		x[3] = 0.0;
		return *this;
	}
	// smallest negative normal number
	constexpr qd& minneg() noexcept {
		x[0] = -std::numeric_limits<double>::min();
		x[1] = 0.0;
		x[2] = 0.0;
		x[3] = 0.0;
		return *this;
	}
	constexpr qd& maxneg() noexcept {
		x[0] = std::numeric_limits<double>::lowest();
		x[1] = 0.0;
		x[2] = 0.0;
		x[3] = 0.0;
		return *this;
	}

	qd& assign(const std::string& txt) {
		qd v;
		if (parse(txt, v)) *this = v;
		return *this; // Is this what we want? when the string is not valid, keep the current value?
	}

	// selectors
	constexpr bool iszero()        const noexcept { return x[0] == 0.0; }
	constexpr bool isone()         const noexcept { return x[0] == 1.0 && x[1] == 0.0; }
	constexpr bool ispos()         const noexcept { return x[0] > 0.0; }
	constexpr bool isneg()         const noexcept { return x[0] < 0.0; }
	BIT_CAST_CONSTEXPR bool isnan(int NaNType = NAN_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		int nan_type;
		bool isNaN = checkNaN(x[0], nan_type);
		bool isNegNaN = isNaN && negative;
		bool isPosNaN = isNaN && !negative;
		return (NaNType == NAN_TYPE_EITHER ? (isNegNaN || isPosNaN) :
			(NaNType == NAN_TYPE_SIGNALLING ? isNegNaN :
				(NaNType == NAN_TYPE_QUIET ? isPosNaN : false)));
	}
	BIT_CAST_CONSTEXPR bool isinf(int InfType = INF_TYPE_EITHER)  const noexcept {
		bool negative = isneg();
		int inf_type;
		bool isInf = checkInf(x[0], inf_type);
		bool isNegInf = isInf && negative;
		bool isPosInf = isInf && !negative;
		return (InfType == INF_TYPE_EITHER ? (isNegInf || isPosInf) :
			(InfType == INF_TYPE_NEGATIVE ? isNegInf :
				(InfType == INF_TYPE_POSITIVE ? isPosInf : false)));
	}

	constexpr bool sign()          const noexcept { return (x[0] < 0.0); }
	constexpr int  scale()         const noexcept { return sw::universal::scale(x[0]); }
	constexpr int  exponent()      const noexcept { return sw::universal::scale(x[0]); }


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// arithmetic operator helpers

	constexpr qd accurate_addition(const qd& a, const qd& b) const {
		// std::abs(double) is not constexpr in C++20; use a ternary helper.
		auto absv = [](double v) constexpr -> double { return v < 0.0 ? -v : v; };

		double u, v;
		int i{ 0 }, j{ 0 }, k{ 0 };
		if (absv(a[i]) > absv(b[j])) {
			u = a[i++];
		}
		else {
			u = b[j++];
		}
		if (absv(a[i]) > absv(b[j])) {
			v = a[i++];
		}
		else {
			v = b[j++];
		}

		u = quick_two_sum(u, v, v);

		double c[4] = { 0.0, 0.0, 0.0, 0.0 };
		while (k < 4) {
			if (i >= 4 && j >= 4) {
				c[k] = u;
				if (k < 3) {
					c[++k] = v;
				}
				break;
			}
			double t;
			if (i >= 4) {
				t = b[j++];
			}
			else if (j >= 4) {
				t = a[i++];
			}
			else if (absv(a[i]) > absv(b[j])) {
				t = a[i++];
			}
			else {
				t = b[j++];
			}

			double s = quick_three_accumulation(u, v, t);

			if (s != 0.0) {
				c[k++] = s;
			}
		}

		// add the rest
		for (k = i; k < 4; k++) c[3] += a[k];
		for (k = j; k < 4; k++) c[3] += b[k];

		sw::universal::renorm(c[0], c[1], c[2], c[3]);
		return qd(c[0], c[1], c[2], c[3]);
	}

	constexpr qd approximate_addition(const qd& a, const qd& b) const {
		double s0, s1, s2, s3;
		double t0, t1, t2, t3;

		s0 = two_sum(a[0], b[0], t0);
		s1 = two_sum(a[1], b[1], t1);
		s2 = two_sum(a[2], b[2], t2);
		s3 = two_sum(a[3], b[3], t3);

		s1 = two_sum(s1, t0, t0);
		three_sum(s2, t0, t1);
		three_sum2(s3, t0, t2);
		t0 = t0 + t1 + t3;

		sw::universal::renorm(s0, s1, s2, s3, t0);
		return qd(s0, s1, s2, s3);
	}

	constexpr qd approximate_addition_explicit(const qd& a, const qd& b) const {
		// Same as approximate_addition, but addition re-organized to guide bad compilers

		double s0 = a[0] + b[0];
		double s1 = a[1] + b[1];
		double s2 = a[2] + b[2];
		double s3 = a[3] + b[3];

		double v0 = s0 - a[0];
		double v1 = s1 - a[1];
		double v2 = s2 - a[2];
		double v3 = s3 - a[3];

		double u0 = s0 - v0;
		double u1 = s1 - v1;
		double u2 = s2 - v2;
		double u3 = s3 - v3;

		double w0 = a[0] - u0;
		double w1 = a[1] - u1;
		double w2 = a[2] - u2;
		double w3 = a[3] - u3;

		u0 = b[0] - v0;
		u1 = b[1] - v1;
		u2 = b[2] - v2;
		u3 = b[3] - v3;

		double t0 = w0 + u0;
		double t1 = w1 + u1;
		double t2 = w2 + u2;
		double t3 = w3 + u3;

		s1 = two_sum(s1, t0, t0);
		three_sum(s2, t0, t1);
		three_sum2(s3, t0, t2);
		t0 = t0 + t1 + t3;

		sw::universal::renorm(s0, s1, s2, s3, t0);
		return qd(s0, s1, s2, s3);
	}

	/* quad-double * quad-double
	   a0 * b0                    0
			a0 * b1               1
			a1 * b0               2
				 a0 * b2          3
				 a1 * b1          4
				 a2 * b0          5
					  a0 * b3     6
					  a1 * b2     7
					  a2 * b1     8
					  a3 * b0     9  
	 */
	constexpr qd approximate_multiplication(const qd& a, const qd& b) const {
		double p0, p1, p2, p3, p4, p5;
		double q0, q1, q2, q3, q4, q5;
		double t0, t1;
		double s0, s1, s2;

		p0 = two_prod(a[0], b[0], q0);

		p1 = two_prod(a[0], b[1], q1);
		p2 = two_prod(a[1], b[0], q2);

		p3 = two_prod(a[0], b[2], q3);
		p4 = two_prod(a[1], b[1], q4);
		p5 = two_prod(a[2], b[0], q5);

		// Start accumulation of partials
		three_sum(p1, p2, q0);

		// Six-Three Sum  of p2, q1, q2, p3, p4, p5
		three_sum(p2, q1, q2);
		three_sum(p3, p4, p5);
		// compute (s0, s1, s2) = (p2, q1, q2) + (p3, p4, p5)
		s0 = two_sum(p2, p3, t0);
		s1 = two_sum(q1, p4, t1);
		s2 = q2 + p5;
		s1 = two_sum(s1, t0, t0);
		s2 += (t0 + t1);

		// O(eps^3) order terms
		s1 += a[0] * b[3] + a[1] * b[2] + a[2] * b[1] + a[3] * b[0] + q0 + q3 + q4 + q5;
		sw::universal::renorm(p0, p1, s0, s1, s2);
		return qd(p0, p1, s0, s1);
	}

	constexpr qd accurate_multiplication(const qd& a, const qd& b) const {
		double q0, q1, q2, q3, q4, q5;
		double p0 = two_prod(a[0], b[0], q0);
		
		double p1 = two_prod(a[0], b[1], q1);
		double p2 = two_prod(a[1], b[0], q2);

		double p3 = two_prod(a[0], b[2], q3);
		double p4 = two_prod(a[1], b[1], q4);
		double p5 = two_prod(a[2], b[0], q5);

		// Start Accumulation
		three_sum(p1, p2, q0);

		// Six-Three Sum  of p2, q1, q2, p3, p4, p5
		three_sum(p2, q1, q2);
		three_sum(p3, p4, p5);
		// compute (s0, s1, s2) = (p2, q1, q2) + (p3, p4, p5)
		double t0, t1;
		double s0 = two_sum(p2, p3, t0);
		double s1 = two_sum(q1, p4, t1);
		double s2 = q2 + p5;
		s1 = two_sum(s1, t0, t0);
		s2 += (t0 + t1);

		// O(eps^3) order terms
		double q6, q7, q8, q9;
		double p6 = two_prod(a[0], b[3], q6);
		double p7 = two_prod(a[1], b[2], q7);
		double p8 = two_prod(a[2], b[1], q8);
		double p9 = two_prod(a[3], b[0], q9);

		// Nine-Two-Sum of q0, s1, q3, q4, q5, p6, p7, p8, p9
		q0 = two_sum(q0, q3, q3);
		q4 = two_sum(q4, q5, q5);
		p6 = two_sum(p6, p7, p7);
		p8 = two_sum(p8, p9, p9);
		// Compute (t0, t1) = (q0, q3) + (q4, q5)
		t0 = two_sum(q0, q4, t1);
		t1 += (q3 + q5);
		// Compute (r0, r1) = (p6, p7) + (p8, p9)
		double r1;
		double r0 = two_sum(p6, p8, r1);
		r1 += (p7 + p9);
		// Compute (q3, q4) = (t0, t1) + (r0, r1)
		q3 = two_sum(t0, r0, q4);
		q4 += (t1 + r1);
		// Compute (t0, t1) = (q3, q4) + s1
		t0 = two_sum(q3, s1, t1);
		t1 += q4;

		// O(eps^4) terms -- Nine-One-Sum
		t1 += a[1] * b[3] + a[2] * b[2] + a[3] * b[1] + q6 + q7 + q8 + q9 + s2;

		sw::universal::renorm(p0, p1, s0, t0, t1);
		return qd(p0, p1, s0, t0);
	}
	
	constexpr qd approximate_division(const qd& a, const qd& b) const {
		qd r{};

		double q0 = a[0] / b[0];
		r = a - (b * q0);

		double q1 = r[0] / b[0];
		r -= (b * q1);

		double q2 = r[0] / b[0];
		r -= (b * q2);

		double q3 = r[0] / b[0];

		sw::universal::renorm(q0, q1, q2, q3);
		return qd(q0, q1, q2, q3);
	}

	constexpr qd accurate_division(const qd& a, const qd& b) const {
		qd r{};

		double q0 = a[0] / b[0];
		r = a - (b * q0);

		double q1 = r[0] / b[0];
		r -= (b * q1);

		double q2 = r[0] / b[0];
		r -= (b * q2);

		double q3 = r[0] / b[0];
		r -= (b * q3);

		double q4 = r[0] / b[0];

		sw::universal::renorm(q0, q1, q2, q3, q4);
		return qd(q0, q1, q2, q3);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// decimal string converter helpers

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
				int powerOfTenScale = static_cast<int>(std::log10(std::fabs(x[0])));
				int integerDigits = (fixed ? (powerOfTenScale + 1) : 1);
				int nrDigits = integerDigits + static_cast<int>(precision);

				int nrDigitsForFixedFormat = nrDigits;
				if (fixed)
					nrDigitsForFixedFormat = std::max(120, nrDigits); // can be much longer than the max accuracy for quad-double

				// a number in the range of [0.5, 1.0) to be printed with zero precision 
				// must be rounded up to 1 to print correctly
				if (fixed && (precision == 0) && (std::abs(x[0]) < 1.0)) {
					s += (std::abs(x[0]) >= 0.5) ? '1' : '0';
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
						t.resize(static_cast<unsigned>(nrDigitsForFixedFormat + 1));
						to_digits(t, e, nrDigitsForFixedFormat);
					}
					else {
						t.resize(static_cast<unsigned>(nrDigits + 1));
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
						s += t[0];
						if (precision > 0) s += '.';

						for (int i = 1; i <= precision; ++i) {
							s += t[static_cast<unsigned>(i)];
						}
					}
				}
			}

			// trap for improper offset with large values
			// without this trap, output of values of the for 10^j - 1 fail for j > 28
			// and are output with the point in the wrong place, leading to a dramatically off value
			if (fixed && (precision > 0)) {
				// make sure that the value isn't dramatically larger
				double from_string = atof(s.c_str());

				// if this ratio is large, then we've got problems
				if (std::fabs(from_string / x[0]) > 3.0) {

					// loop on the string, find the point, move it up one
					// don't act on the first character
					for (std::string::size_type i = 1; i < s.length(); ++i) {
						if (s[i] == '.') {
							s[i] = s[i - 1];
							s[i - 1] = '.';
							break;
						}
					}

					from_string = atof(s.c_str());
					// if this ratio is large, then the string has not been fixed
					if (std::fabs(from_string / x[0]) > 3.0) {
						//error("Re-rounding unsuccessful in large number fixed point trap.");
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
	double x[4];  // fixed four (4) limbs, x[0] is highest order limb, x[3] is lowest order limb

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// private helper methods

	constexpr qd& convert_signed(int64_t v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			x[0] = static_cast<double>(v);
			x[1] = static_cast<double>(v - static_cast<int64_t>(x[0]));
		}
		return *this;
	}

	constexpr qd& convert_unsigned(uint64_t v) noexcept {
		if (0 == v) {
			setzero();
		}
		else {
			x[0] = static_cast<double>(v);
			x[1] = static_cast<double>(v - static_cast<uint64_t>(x[0]));  // difference is always positive
		}
		return *this;
	}

	// no need to SFINAE this as it is an internal method that we ONLY call when we know the argument type is a native float
	constexpr qd& convert_ieee754(float rhs) noexcept {
		x[0] = double(rhs);
		x[1] = 0.0;
		x[2] = 0.0;
		x[3] = 0.0;
		return *this;
	}
	constexpr qd& convert_ieee754(double rhs) noexcept {
		x[0] = double(rhs);
		x[1] = 0.0;
		x[2] = 0.0;
		x[3] = 0.0;
		return *this;
	}
#if LONG_DOUBLE_SUPPORT
	qd& convert_ieee754(long double rhs) {
		volatile long double truncated = static_cast<long double>(double(rhs));
		volatile double remainder = static_cast<double>(rhs - truncated);
		x[0] = static_cast<double>(truncated);
		x[1] = remainder;
		x[2] = 0.0;
		x[3] = 0.0;
		return *this;
	}
#endif

	// (convert_to_unsigned_impl / convert_to_signed_impl moved up next to the
	// conversion operators that call them, so the constexpr call chain is
	// fully resolved at the point of declaration.  The old int64_t-cast
	// helpers had two bugs: they ignored x[2] and x[3], and could trigger
	// UB per C++20 [conv.fpint] for unnormalized inputs.)


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// functional helpers

	// precondition: string s must be all digits
	void round_string(std::vector<char>& s, int precision, int* decimalPoint) const {
		int nrDigits = precision;
		// round decimal string and propagate carry
		int lastDigit = nrDigits - 1;
		if (s[static_cast<unsigned>(lastDigit)] >= '5') {
			int i = nrDigits - 2;
			s[static_cast<unsigned>(i)]++;
			while (i > 0 && s[static_cast<unsigned>(i)] > '9') {
				s[static_cast<unsigned>(i)] -= 10;
				s[static_cast<unsigned>(--i)]++;
			}
		}

		// if first digit is 10, shift everything.
		if (s[0] > '9') {
			for (int i = precision; i >= 2; i--) s[static_cast<unsigned>(i)] = s[static_cast<unsigned>(i - 1)];
			s[0] = '1';
			s[1] = '0';

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


	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// decimal string converter helper


	/// <summary>
	/// to_digits generates the decimal digits representing
	/// </summary>
	/// <param name="s"></param>
	/// <param name="exponent"></param>
	/// <param name="precision"></param>
	void to_digits(std::vector<char>& s, int& exponent, int precision) const {
		constexpr qd _one(1.0), _ten(10.0);
		constexpr double _log2(0.301029995663981);
		double hi = x[0];
		//double lo = x[1];

		if (iszero()) {
			exponent = 0;
			for (int i = 0; i < precision; ++i) s[static_cast<unsigned>(i)] = '0';
			return;
		}

		// First determine the (approximate) exponent.
		// std::frexp(*this, &e);   // e is appropriate for 0.5 <= x < 1
		int e;
		std::frexp(hi, &e);	
		--e; // adjust e as frexp gives a binary e that is 1 too big
		e = static_cast<int>(_log2 * e); // estimate the power of ten exponent 
		qd r = abs(*this);
		if (e < 0) {
			if (e < -300) {
				//r = qd(std::ldexp(r[0], 53), std::ldexp(r[1], 53));
				r *= pown(_ten, -e);
				//r = qd(std::ldexp(r[0], -53), std::ldexp(r[1], -53));
			}
			else {
				r *= pown(_ten, -e);
			}
		}
		else {
			if (e > 0) {
				if (e > 300) {
					//r = qd(std::ldexp(r[0], -53), std::ldexp(r[1], -53));
					r /= pown(_ten, e);
					//r = qd(std::ldexp(r[0], 53), std::ldexp(r[1], 53));
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

		// at this point the value is normalized to a decimal value between (0, 10)
		// generate the digits
		int nrDigits = precision + 1;
		for (int i = 0; i < nrDigits; ++i) {
			// Defensive NaN guard: if arithmetic drift in earlier iterations
			// pushed r[0] to NaN, casting to int is C++20 [conv.fpint] UB.
			// Coerce NaN to 0 so the cast is always safe; the resulting
			// digit string will be incorrect, but the program does not
			// trigger UB.  (The algorithm is supposed to keep r[0] in
			// [0, 10) at each iteration; this is a backstop.)
			double v = r[0];
			int mostSignificantDigit = (v != v) ? 0 : static_cast<int>(v);
			r -= mostSignificantDigit;
			r *= 10.0;

			s[static_cast<unsigned>(i)] = static_cast<char>(mostSignificantDigit + '0');
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

	// qd - qd logic comparisons
	friend constexpr bool operator==(const qd& lhs, const qd& rhs);
	friend constexpr bool operator!=(const qd& lhs, const qd& rhs);
	friend constexpr bool operator<=(const qd& lhs, const qd& rhs);
	friend constexpr bool operator>=(const qd& lhs, const qd& rhs);
	friend constexpr bool operator<(const qd& lhs, const qd& rhs);
	friend constexpr bool operator>(const qd& lhs, const qd& rhs);

	// qd - literal logic comparisons
	friend constexpr bool operator==(const qd& lhs, const double rhs);

	// literal - qd logic comparisons
	friend constexpr bool operator==(const double lhs, const qd& rhs);

};

////////////////////////  precomputed constants of note  /////////////////////////////////

// precomputed quad-double constants 

constexpr qd qd_max(1.79769313486231570815e+308, 9.97920154767359795037e+291);

constexpr double qd_eps = 4.93038065763132e-32;  // 2^-104
constexpr double qd_min_normalized = 2.0041683600089728e-292;  // = 2^(-1022 + 53)

////////////////////////    helper functions   /////////////////////////////////

inline qd ulp(const qd& a) {
	int scaleOf = scale(a[0]);
	return ldexp(qd(1.0), scaleOf - 159);
}

inline std::string to_quad(const qd& v, int precision = 17) {
	std::stringstream s;
	s << std::setprecision(precision) << "( " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ')';
	return s.str();
}

inline std::string to_triple(const qd& v, int precision = 17) {
	std::stringstream s;
	bool isneg = v.isneg();
	int scale = v.scale();
	int exponent;
	qd fraction = frexp(v, &exponent);
	s << '(' << (isneg ? '1' : '0') << ", " << scale << ", " << std::setprecision(precision) << fraction << ')';
	return s.str();
}

inline std::string to_binary(const qd& number, bool nibbleMarker = false) {
	std::stringstream s;
	double_decoder decoder;
	decoder.d = number[0];	

	s << "0b";
	// print sign bit
	s << (decoder.parts.sign ? '1' : '0') << '.';

	// print exponent bits
	{
		uint64_t mask = 0x400;
		for (int bit = 10; bit >= 0; --bit) {
			s << ((decoder.parts.exponent & mask) ? '1' : '0');
			if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

	s << '.';

	// print first limb's fraction bits
	{
		uint64_t mask = (uint64_t(1) << 51);
		for (int bit = 51; bit >= 0; --bit) {
			s << ((decoder.parts.fraction & mask) ? '1' : '0');
			if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
			mask >>= 1;
		}
	}

// remove debugging statements when validated
//	auto defaultPrec = std::cout.precision();
//	std::cout << std::setprecision(7);
	// print the extension fraction bits
	// this is bit of a trick as there can be many different ways in which the limbs represent
	// more precise fraction bits

	// For quad-double we need to enumerate in the qd bit space, 
	// since we know the scale of the bits in this space, set by the scale of the first limb
	int limb{ 0 };
	int scaleOfBit = scale(number[limb++]) - 53;  // this is the scale of the first extension bit
	double bitValue = std::ldexp(1.0, scaleOfBit-1);
	constexpr int firstExtensionBit = 212 - 53;
	double segment = number[limb];
	// when do you know to switch to a new limb?
	for (int bit = firstExtensionBit; bit > 0; --bit) {
		if (bit == firstExtensionBit || bit == 106 || bit == 53) s << '|';
		double diff = segment - bitValue;
//		std::cout << "segment    : " << to_binary(segment) << " : " << segment << '\n';
//		std::cout << "bitValue   : " << to_binary(bitValue) << " : " << bitValue << '\n';
//		std::cout << "difference : " << diff << '\n';
		if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
		if (diff >= 0.0) {
			// segment > bitValue
			segment -= bitValue;
			s << '1';
		}
		else {
			s << '0';
		}
		bitValue /= 2;
		if (segment == 0.0) {
			// configurations where there are segments that are 0.0 have these segments
			// after non-zero segments. This logic is consistent, as the conditional
			// will avoid stepping out the segment array.
			if (limb < 3) segment = number[++limb];
		}
	}
//	std::cout << std::setprecision(defaultPrec);

	return s.str();
}

// native semantic representation: radix-2, delegates to to_binary
inline std::string to_native(const qd& number, bool nibbleMarker = false) {
	return to_binary(number, nibbleMarker);
}

inline std::string to_components(const qd& number, bool nibbleMarker = false) {
	std::stringstream s;
	constexpr int nrLimbs = 4;
	for (int i = 0; i < nrLimbs; ++i) {
		double_decoder decoder;
		decoder.d = number[i];

		std::string label = "x[" + std::to_string(i) + "]";
		s << label << " : ";
		s << "0b";
		// print sign bit
		s << (decoder.parts.sign ? '1' : '0') << '.';

		// print exponent bits
		{
			uint64_t mask = 0x400;
			for (int bit = 10; bit >= 0; --bit) {
				s << ((decoder.parts.exponent & mask) ? '1' : '0');
				if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
				mask >>= 1;
			}
		}

		s << '.';

		// print hi fraction bits
		uint64_t mask = (uint64_t(1) << 51);
		for (int bit = 51; bit >= 0; --bit) {
			s << ((decoder.parts.fraction & mask) ? '1' : '0');
			if (nibbleMarker && bit != 0 && (bit % 4) == 0) s << '\'';
			mask >>= 1;
		}

		s << std::scientific << std::showpos << std::setprecision(15); // we are printing a double
		s << " : " << number[i] << " : binary scale " << scale(number[i]) << '\n';
	}

	return s.str();
}

////////////////////////    math functions   /////////////////////////////////

inline qd reciprocal(const qd& a) {
	return qd(1.0) / a;
}

inline qd abs(const qd& a) {
	return (a[0] < 0.0) ? -a : a;
}

inline qd ceil(const qd& a) {
	double x0{ 0.0 }, x1{ 0.0 }, x2{ 0.0 }, x3{ 0.0 };
	x0 = std::ceil(a[0]);

	if (x0 == a[0]) {
		x1 = std::ceil(a[1]);

		if (x1 == a[1]) {
			x2 = std::ceil(a[2]);

			if (x2 == a[2]) {
				x3 = std::ceil(a[3]);
			}
		}

		renorm(x0, x1, x2, x3);
		return qd(x0, x1, x2, x3);
	}

	return qd(x0, x1, x2, x3);
}

inline qd floor(const qd& a) {
	double x0{ 0.0 }, x1{ 0.0 }, x2{ 0.0 }, x3{ 0.0 };
	x0 = std::floor(a[0]);

	if (x0 == a[0]) {
		x1 = std::floor(a[1]);

		if (x1 == a[1]) {
			x2 = std::floor(a[2]);

			if (x2 == a[2]) {
				x3 = std::floor(a[3]);
			}
		}

		renorm(x0, x1, x2, x3);
		return qd(x0, x1, x2, x3);
	}

	return qd(x0, x1, x2, x3);
}

// Round to Nearest integer
inline qd nint(const qd& a) {
	double x0{ 0.0 }, x1{ 0.0 }, x2{ 0.0 }, x3{ 0.0 };
	x0 = nint(a[0]);

	if (x0 == a[0]) {
		// First double is already an integer
		x1 = nint(a[1]);

		if (x1 == a[1]) {
			// Second double is already an integer
			x2 = nint(a[2]);

			if (x2 == a[2]) {
				// Third double is already an integer
				x3 = nint(a[3]);
			}
			else {
				if (std::abs(x2 - a[2]) == 0.5 && a[3] < 0.0) {
					x2 -= 1.0;
				}
			}

		}
		else {
			if (std::abs(x1 - a[1]) == 0.5 && a[2] < 0.0) {
				x1 -= 1.0;
			}
		}

	}
	else {
		/* First double is not an integer. */
		if (std::abs(x0 - a[0]) == 0.5 && a[1] < 0.0) {
			x0 -= 1.0;
		}
	}

	renorm(x0, x1, x2, x3);
	return qd(x0, x1, x2, x3);
}

// Round to Nearest integer quick version.  May be off by one when qd is very close to the middle of two integers.
inline qd quick_nint(const qd& a) {
	qd r = qd(nint(a[0]), nint(a[1]), nint(a[2]), nint(a[3]));
	r.renorm();
	return r;
}

// quad-double * double, where double is a power of 2
inline qd mul_pwr2(const qd& a, double b) {
	return qd(a[0] * b, a[1] * b, a[2] * b, a[3] * b);
}

/* quad-double ^ 2  = (x0 + x1 + x2 + x3) ^ 2
					= x0 ^ 2 + 2 x0 * x1 + (2 x0 * x2 + x1 ^ 2)
							   + (2 x0 * x3 + 2 x1 * x2)           */
inline qd sqr(const qd& a) {
	double q0, q1, q2, q3;
	double p0 = two_sqr(a[0], q0);
	double p1 = two_prod(2.0 * a[0], a[1], q1);
	double p2 = two_prod(2.0 * a[0], a[2], q2);
	double p3 = two_sqr(a[1], q3);

	p1 = two_sum(q0, p1, q0);

	q0 = two_sum(q0, q1, q1);
	p2 = two_sum(p2, p3, p3);

	double t0, t1;
	double s0 = two_sum(q0, p2, t0);
	double s1 = two_sum(q1, p3, t1);

	s1 = two_sum(s1, t0, t0);
	t0 += t1;

	s1 = quick_two_sum(s1, t0, t0);
	p2 = quick_two_sum(s0, s1, t1);
	p3 = quick_two_sum(t1, t0, q0);

	double p4 = 2.0 * a[0] * a[3];
	double p5 = 2.0 * a[1] * a[2];

	p4 = two_sum(p4, p5, p5);
	q2 = two_sum(q2, q3, q3);

	t0 = two_sum(p4, q2, t1);
	t1 = t1 + p5 + q3;

	p3 = two_sum(p3, t0, p4);
	p4 = p4 + q0 + t1;

	renorm(p0, p1, p2, p3, p4);
	return qd(p0, p1, p2, p3);
}

// Computes pow(qd, n), where n is an integer
inline qd pown(const qd& a, int n) {
	if (n == 0)
		return 1.0;

	qd r{ a };   // odd-case multiplier
	qd s{ 1.0 };
	int N = std::abs(n);

	if (N > 1) {
		while (N > 0) {
			if (N % 2 == 1) {
				s *= r;
			}
			N /= 2;
			if (N > 0) r = sqr(r);
		}
	}
	else {
		s = r;
	}

	if (n < 0)
		return (qd(1.0) / s);

	return s;
}

////////////////////////  stream operators   /////////////////////////////////

// stream out a decimal floating-point representation of the quad-double
inline std::ostream& operator<<(std::ostream& ostr, const qd& v) {
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

// stream in an ASCII decimal floating-point format and assign it to a quad-double
inline std::istream& operator>>(std::istream& istr, qd& v) {
	std::string txt;
	istr >> txt;
	if (!parse(txt, v)) {
		std::cerr << "unable to parse -" << txt << "- into a quad-double value\n";
	}
	return istr;
}

////////////////// string operators

// parse a decimal ASCII floating-point format and make a quad-double (qd) out of it
inline bool parse(const std::string& number, qd& value) {
	char const* p = number.c_str();

	// Skip any leading spaces
	while (std::isspace(*p)) ++p;

	qd r{ 0.0 };
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
	qd _ten(10.0, 0.0);
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
// qd - qd binary logic operators

// equal: precondition is that the storage is properly nulled in all arithmetic paths
constexpr bool operator==(const qd& lhs, const qd& rhs) {
	return (lhs[0] == rhs[0]) && (lhs[1] == rhs[1] && lhs[2] == rhs[2]) && (lhs[3] == rhs[3]);
}

constexpr bool operator!=(const qd& lhs, const qd& rhs) {
	return !operator==(lhs, rhs);
}

constexpr bool operator< (const qd& lhs, const qd& rhs) {
	return (lhs[0] < rhs[0] ||
		(lhs[0] == rhs[0] && (lhs[1] < rhs[1] ||
			(lhs[1] == rhs[1] && (lhs[2] < rhs[2] ||
				(lhs[2] == rhs[2] && lhs[3] < rhs[3]))))));
}

constexpr bool operator> (const qd& lhs, const qd& rhs) {
	return operator< (rhs, lhs);
}

constexpr bool operator<=(const qd& lhs, const qd& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

constexpr bool operator>=(const qd& lhs, const qd& rhs) {
	// NOT !operator<: that would be true for unordered (NaN) operands. Use
	// operator> || operator== so NaN comparisons stay unordered (per #797).
	return operator>(lhs, rhs) || operator==(lhs, rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// qd - literal binary logic operators
// 
// equal: precondition is that the storage is properly nulled in all arithmetic paths
constexpr bool operator==(const qd& lhs, double rhs) {
	return operator==(lhs, qd(rhs));
}

constexpr bool operator!=(const qd& lhs, double rhs) {
	return !operator==(lhs, rhs);
}

constexpr bool operator< (const qd& lhs, double rhs) {
	return operator<(lhs, qd(rhs));
}

constexpr bool operator> (const qd& lhs, double rhs) {
	return operator< (qd(rhs), lhs);
}

constexpr bool operator<=(const qd& lhs, double rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

constexpr bool operator>=(const qd& lhs, double rhs) {
	return operator>(lhs, qd(rhs)) || operator==(lhs, qd(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - qd binary logic operators
// 
// equal: precondition is that the storage is properly nulled in all arithmetic paths
constexpr bool operator==(double lhs, const qd& rhs) {
	return operator==(qd(lhs), rhs);
}

constexpr bool operator!=(double lhs, const qd& rhs) {
	return !operator==(lhs, rhs);
}

constexpr bool operator< (double lhs, const qd& rhs) {
	return operator<(qd(lhs), rhs);
}

constexpr bool operator> (double lhs, const qd& rhs) {
	return operator< (rhs, lhs);
}

constexpr bool operator<=(double lhs, const qd& rhs) {
	return operator< (lhs, rhs) || operator==(lhs, rhs);
}

constexpr bool operator>=(double lhs, const qd& rhs) {
	return operator>(qd(lhs), rhs) || operator==(qd(lhs), rhs);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////
// qd - qd binary arithmetic operators
// BINARY ADDITION
constexpr qd operator+(const qd& lhs, const qd& rhs) {
	qd sum{ lhs };
	sum += rhs;
	return sum;
}
// BINARY SUBTRACTION
constexpr qd operator-(const qd& lhs, const qd& rhs) {
	qd diff{ lhs };
	diff -= rhs;
	return diff;
}
// BINARY MULTIPLICATION
constexpr qd operator*(const qd& lhs, const qd& rhs) {
	qd mul{ lhs };
	mul *= rhs;
	return mul;
}
// BINARY DIVISION
constexpr qd operator/(const qd& lhs, const qd& rhs) {
	qd ratio{ lhs };
	ratio /= rhs;
	return ratio;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// qd - literal binary arithmetic operators
// BINARY ADDITION
constexpr qd operator+(const qd& lhs, double rhs) {
	return operator+(lhs, qd(rhs));
}
// BINARY SUBTRACTION
constexpr qd operator-(const qd& lhs, double rhs) {
	return operator-(lhs, qd(rhs));
}
// BINARY MULTIPLICATION
constexpr qd operator*(const qd& lhs, double rhs) {
	return operator*(lhs, qd(rhs));
}
// BINARY DIVISION
constexpr qd operator/(const qd& lhs, double rhs) {
	return operator/(lhs, qd(rhs));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// literal - qd binary arithmetic operators
// BINARY ADDITION
constexpr qd operator+(double lhs, const qd& rhs) {
	return operator+(qd(lhs), rhs);
}
// BINARY SUBTRACTION
constexpr qd operator-(double lhs, const qd& rhs) {
	return operator-(qd(lhs), rhs);
}
// BINARY MULTIPLICATION
constexpr qd operator*(double lhs, const qd& rhs) {
	return operator*(qd(lhs), rhs);
}
// BINARY DIVISION
constexpr qd operator/(double lhs, const qd& rhs) {
	return operator/(qd(lhs), rhs);
}

}} // namespace sw::universal
