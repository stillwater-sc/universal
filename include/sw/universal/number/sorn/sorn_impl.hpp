#pragma once
// sorn_impl.hpp: implementation of SORN arithmetic number format, based on unum type-II format
//
// Copyright (C) 2022 ITEM, University of Bremen.
//
// This file is aimed to provide an addition and to be included in the universal numbers project.
#include <cassert>
#include <cstdint>
#include <cmath>
#include <iosfwd>     // std::ostream, named by the operator<< friend declaration
#include <limits>
#include <string>     // std::string, returned by getInt/getConfig/getDT (defined in manipulators.hpp)
#include <vector>
#include <bitset>
#include <type_traits>

#include <universal/number/shared/specific_value_encoding.hpp>

// BEHAVIORAL COMPILATION SWITCHES: default here rather than in the sorn.hpp umbrella, so
// that a translation unit which includes core.hpp directly gets the same defaults (#1334,
// #1436). Defining either before any sorn header still wins. Neither is read yet.
#if !defined(SORN_ENABLE_LITERALS)
#define SORN_ENABLE_LITERALS 1
#endif
#if !defined(SORN_THROW_ARITHMETIC_EXCEPTION)
#define SORN_THROW_ARITHMETIC_EXCEPTION 0
#endif
#include <universal/utility/exceptions_check.hpp>  // reject a throw switch without exceptions (#1925)

namespace sw { namespace universal {

// struct sornInterval: a struct defining a SORN interval with two interval bound values and open/closed conditions.
template<typename Real> 
struct sornInterval {
	Real lowerBound;
	Real upperBound;
	bool lowerIsOpen;
	bool upperIsOpen;

	// the interval as text; defined in manipulators.hpp -- it builds it with a stringstream (#1334)
	std::string getInt() const;

	bool isZero() const noexcept {
		if (this->lowerBound == 0 && this->upperBound == 0 && not this->lowerIsOpen && not this->upperIsOpen) {
			return true;
		} 
		else {
			return false;
		}
	}

};

// setSornDT: function to create a SORN datatype
template<typename Real>
inline std::vector<sornInterval<Real>> setSornDT(signed int start, signed int stop, unsigned int steps, float stepSize,
									bool flagNeg, bool flagInf, bool flagZero, bool flagLin,
									bool flagLog, bool flagHalfopen, bool flagOpen, size_t sornBits ) {
	using SORN_INTERVAL = sornInterval<Real>;
	using std::pow;
	using std::fmax;

	std::vector<SORN_INTERVAL> sornDT;
	// 1. halfopen config
	if (flagHalfopen) {
		// 1.1. zero
		if (flagZero) {
			SORN_INTERVAL structInt;
			structInt.lowerBound = 0;
			structInt.upperBound = 0;
			structInt.lowerIsOpen = false;
			structInt.upperIsOpen = false;
			sornDT.push_back(structInt);
		}
		// 1.2. positive part
		if (flagLin) {
			// 1.2.1 linear config
			assert(start == 0 && "%% ERROR %% Start value has to be set to 0 for linear halfopen configuration.");
			for (int b = 0; b < (int)steps; b++) {
				SORN_INTERVAL structInt;
				structInt.lowerBound = b * stepSize;
				structInt.upperBound = (b + 1) * stepSize;
				structInt.lowerIsOpen = (b == 0 && not flagZero ? false : true);
				structInt.upperIsOpen = false;
				sornDT.push_back(structInt);
			}
		}
		else if (flagLog) {
			// 1.2.2 logarithmic config (Note: "steps" value is ignored for logarithmic configuration)
			for (int b = start; b < (stop + 1); b++) {
				SORN_INTERVAL structInt;
				structInt.lowerBound = (b == start ? 0 : (float)pow(2, (b - 1)));
				structInt.upperBound = (float)pow(2, b);
				structInt.lowerIsOpen = (b == 0 && not flagZero ? false : true);
				structInt.upperIsOpen = false;
				sornDT.push_back(structInt);
			}
		}
		// 1.3. infinity
		if (flagInf) {
			SORN_INTERVAL structInt;
			structInt.lowerBound = sornDT[sornDT.size() - 1].upperBound;
			structInt.upperBound = std::numeric_limits<float>::infinity();
			structInt.lowerIsOpen = true;
			structInt.upperIsOpen = false;
			sornDT.push_back(structInt);
		}
		// 1.4. negative intervals
		if (flagNeg) {
			std::vector<SORN_INTERVAL> negDT;
			for (size_t b = (flagZero ? 1 : 0); b < sornDT.size(); b++) {
				SORN_INTERVAL structInt;
				structInt.lowerBound = -sornDT[b].upperBound;
				structInt.upperBound = (sornDT[b].lowerBound == 0 ? sornDT[b].lowerBound : -sornDT[b].lowerBound);
				structInt.lowerIsOpen = false;
				structInt.upperIsOpen = true;
				negDT.insert(negDT.begin(), structInt);
			}
			sornDT.insert(sornDT.begin(), negDT.begin(), negDT.end());
		}
	}
	// 2. open config (TODO)
	else if (flagOpen) {
		assert(not flagOpen && "%% WARNING %% Open interval datatypes not implemented.");
	}
	// 3. check config
	assert(sornDT.size() == sornBits && "Something is wrong with the Datatype size. Check sornDT and sornBits.");
	// 4. return DT
	return sornDT;
}

// float2SORN: convert a single float to the SORN interval that contains it.
// A NaN converts to NaN. A value outside the lattice saturates to the outermost interval on
// its side, the same rule adaptToDT() applies to arithmetic results (#1972).
// side says where the value being converted lies relative to operand, its rounding to float:
// 0 when operand is the value itself, +1 when the value lies strictly between operand and the
// next float above it, -1 when it lies strictly between the next float below and operand.
// Every lattice bound is a float, so none lies strictly between the value and operand, and
// the interval found encloses the value; looking up the rounded float alone put a value just
// above a lattice point into the interval below it (#2010).
template<typename Real>
sornInterval<Real> float2SORN(float operand, const std::vector<sornInterval<Real>>& sornDT, sornInterval<Real>& sornIntVal, int side = 0) {
	if (std::isnan(operand) || sornDT.empty()) {
		sornIntVal = { std::numeric_limits<Real>::quiet_NaN(), std::numeric_limits<Real>::quiet_NaN(), false, false };
		return sornIntVal;
	}
	for (const auto& interval : sornDT) {
		const bool aboveLower = (operand > interval.lowerBound) ||
		                        (operand == interval.lowerBound && (side > 0 || (side == 0 && not interval.lowerIsOpen)));
		const bool belowUpper = (operand < interval.upperBound) ||
		                        (operand == interval.upperBound && (side < 0 || (side == 0 && not interval.upperIsOpen)));
		if (aboveLower && belowUpper) {
			sornIntVal = interval;
			return sornIntVal;
		}
	}
	sornIntVal = (operand <= sornDT.front().lowerBound ? sornDT.front() : sornDT.back());
	return sornIntVal;
}

// sornCompareToFloat: the sign of v - f, compared in the precision of v, where f is v rounded
// to float. A NaN compares as 0.
template<typename Scalar>
inline int sornCompareToFloat(Scalar v, float f) {
	if constexpr (std::is_integral_v<Scalar>) {
		// f is a whole number here; at or past 2^digits it exceeds every value of Scalar, and
		// below that it converts back to Scalar exactly
		if (f >= std::ldexp(1.0f, std::numeric_limits<Scalar>::digits)) return -1;
		const Scalar fi = static_cast<Scalar>(f);
		return (v > fi) - (v < fi);
	}
	else {
		const Scalar fs = static_cast<Scalar>(f);
		return (v > fs) - (v < fs);
	}
}

// sornEnclose: the floats lo <= v <= hi on either side of a native value v; exact when v is a
// float, and then lo == hi == v. When v is not a float, lo < v < hi.
struct sornScalarEnclosure { float lo; float hi; bool exact; };
template<typename Scalar>
inline sornScalarEnclosure sornEnclose(Scalar v) {
	constexpr float inf = std::numeric_limits<float>::infinity();
	const float f = static_cast<float>(v);
	const int c = sornCompareToFloat(v, f);
	if (c > 0) return { f, std::nextafter(f, inf), false };
	if (c < 0) return { std::nextafter(f, -inf), f, false };
	return { f, f, true };
}

// sornRoundOut: the float result f moved outward past the exact value: up to the float above
// it for an upper bound, down for a lower bound. c is the sign of exact - f; inexact reports c != 0.
inline float sornRoundOut(float f, int c, bool up, bool& inexact) {
	constexpr float inf = std::numeric_limits<float>::infinity();
	inexact = (c != 0);
	if (up && c > 0) return std::nextafter(f, inf);
	if (!up && c < 0) return std::nextafter(f, -inf);
	return f;
}
// a + b rounded outward; the TwoSum error is exact in float
inline float sornAddOut(float a, float b, bool up, bool& inexact) {
	const float s = a + b;
	int c = 0;
	if (std::isfinite(s)) {
		const float bb = s - a;
		const float err = (a - (s - bb)) + (b - bb);
		c = (err > 0) - (err < 0);
	}
	else if (std::isinf(s) && std::isfinite(a) && std::isfinite(b)) {
		c = (s > 0 ? -1 : 1);   // overflow: the exact sum is finite
	}
	return sornRoundOut(s, c, up, inexact);
}
// a * b rounded outward; the product of two floats is exact in double. A zero factor gives 0,
// also against an infinite bound, as the sorn * sorn corner products do.
inline float sornMulOut(float a, float b, bool up, bool& inexact) {
	inexact = false;
	if (a == 0.0f || b == 0.0f) return 0.0f;
	if (!std::isfinite(a) || !std::isfinite(b)) return a * b;
	const double d = static_cast<double>(a) * static_cast<double>(b);
	const float f = static_cast<float>(d);
	const double fd = static_cast<double>(f);
	return sornRoundOut(f, (d > fd) - (d < fd), up, inexact);
}
// a / b rounded outward, for b > 0 (a zero b stands for the limit from above)
inline float sornDivOut(float a, float b, bool up, bool& inexact) {
	inexact = false;
	if (a == 0.0f) return 0.0f;
	const float q = a / b;
	if (b == 0.0f || !std::isfinite(a) || !std::isfinite(b) || std::isnan(q)) return q;
	// q * b is exact in double; the exact quotient lies above q when a > q * b (b > 0)
	const double qb = static_cast<double>(q) * static_cast<double>(b);
	const double ad = static_cast<double>(a);
	return sornRoundOut(q, (ad > qb) - (ad < qb), up, inexact);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// class sorn: a class for defining a SORN format:	sorn<start,stop,steps,lin,halfopen,neg,inf,zero>
//
// -- Mandatory configuration parameters:
//		start:	lowest value in the SORN lattice // for "lin" start=0 // for "log" -inf<start<inf, lattice begins with 2^start
//		stop:	highest non-infinity value in the SORN lattice // for "lin" start<stop // for "log" start<stop, lattice ends with 2^stop
//		steps:	number of intervals/steps within the SORN representation between "start" and "stop" for "lin" (positive part),
//				not required for "log" distribution (any positve value allowed)
//
// -- Optional configuration parameters: (all "true" by default)
//		lin:		set the SORN interval distribution to "linear" (true) or "logarithmic" (false)
//		halfopen:	set the SORN interval distribution to "halfopen bounds, no exact values" (true) or "open bounds,
//					intermediate exact values" (false)
//		neg:		include negative values/intervals in the SORN datatype, symmetric to positive part
//		inf:		inlcude infinity value/interval bounds to the SORN datatype
//		zero:		inlcude the exact zero value in the SORN datatype
//
// A sorn value is one connected interval of the lattice: a single lattice interval, or a span of
// adjacent ones. NaN stands for the empty set (no lattice interval).
//////////////////////////////////////////////////////////////////////////////////////////////////
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin = 1, bool _halfopen = 1, bool _neg=1, bool _inf=1, bool _zero=1>
class sorn {
	// the open configuration, with exact values between the open intervals, is not implemented:
	// setSornDT() builds no lattice for it (#2010)
	static_assert(_halfopen, "sorn: only the halfopen lattice (_halfopen = true) is implemented");
public:
	using Real = float;
	using SORN_INTERVAL = sornInterval<Real>;
	typedef Real value_type;

private:

	// input configuration parameters
	static constexpr signed int start	= _start;	// lowest non-zero value in the SORN lattice
	static constexpr signed int stop	= _stop;	// highest non-infinity value in the SORN lattice
	static constexpr unsigned int steps	= _steps;	// number of intervals/steps within the SORN representation between start and stop (only for positive part, only for linear distribution)
	static constexpr float stepSize = (float)(stop - start) / (float)steps;

	// input configuration flags
	static constexpr bool flagNeg		= _neg;			// include negative values/intervals in the SORN datatype, symmetric to positive part	(default: true)
	static constexpr bool flagInf		= _inf;			// inlcude infinity bounds to the SORN datatype											(default: true)
	static constexpr bool flagZero		= _zero;		// inlcude the exact zero value in the SORN datatype									(default: true)
	static constexpr bool flagLin		= _lin;			// set the SORN interval distribution to "linear"										(default: true)
	static constexpr bool flagLog		= not _lin;		// set the SORN interval distribution to "logarithmic"									(default: false)
	static constexpr bool flagHalfopen	= _halfopen;	// set the SORN interval distribution to halfopen without exacts						(default: true)
	static constexpr bool flagOpen		= not _halfopen;// set the SORN interval distribution to open with intermediate exacts					(default: false)

	static constexpr Real kInf = std::numeric_limits<Real>::infinity();
	static constexpr Real kNaN = std::numeric_limits<Real>::quiet_NaN();

public:

	// SORN value with NaN initialisation
	SORN_INTERVAL sornIntVal = { kNaN , kNaN , false , false };

	// SORN bitwidth (redundant with sornDT.size())
	static constexpr size_t sornBits =	(
											( (flagLin ? steps : stop - start + 1) + (flagInf ? 1 : 0) ) *		// determine number of intervals (either halfopen or open)
											(flagOpen ? 2 : 1)													// double if open intervals with intermediate exact values are used
										) *
										(flagNeg ? 2 : 1) +														// double if negative values/intervals are included
										(flagZero ? 1 : 0) -													// consider the exact zero value
										((flagOpen & flagInf & flagNeg) ? 1 : 0);									// for open intervals only one value for +-inf is used
	static constexpr size_t nbits = sornBits;

	// datatype: the lattice of intervals this configuration represents. It is built once per
	// configuration and shared by every object; a per-object std::vector made every construction,
	// copy and conversion allocate (#1972).
	static const std::vector<SORN_INTERVAL>& datatype() {
		static const std::vector<SORN_INTERVAL> dt = setSornDT<Real>(start, stop, steps, stepSize, flagNeg, flagInf, flagZero, flagLin, flagLog, flagHalfopen, flagOpen, sornBits);
		return dt;
	}
	// sornDT: the lattice under its original name, for existing callers. Code that may run during
	// static initialization should call datatype() instead.
	static inline const std::vector<SORN_INTERVAL>& sornDT = datatype();

	// constructors
	sorn() {}

	// specific value constructor
	sorn(const SpecificValue code) {
		switch (code) {
		case SpecificValue::maxpos:
			maxpos();
			break;
		case SpecificValue::minpos:
			minpos();
			break;
		case SpecificValue::zero:
		default:
			setzero();
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
		case SpecificValue::nar: // a SORN has no NaR: the empty set stands in for it
		case SpecificValue::qnan:
		case SpecificValue::snan:
			setnan();
			break;
		}
	}

	// converting constructors
	sorn(signed char initial_value)			{ *this = initial_value; }
	sorn(short initial_value)				{ *this = initial_value; }
	sorn(int initial_value)					{ *this = initial_value; }
	sorn(long initial_value)				{ *this = initial_value; }
	sorn(long long initial_value)			{ *this = initial_value; }
	sorn(char initial_value)				{ *this = initial_value; }
	sorn(unsigned short initial_value)		{ *this = initial_value; }
	sorn(unsigned int initial_value)		{ *this = initial_value; }
	sorn(unsigned long initial_value)		{ *this = initial_value; }
	sorn(unsigned long long initial_value)	{ *this = initial_value; }
	sorn(float initial_value)				{ *this = initial_value; }
	sorn(double initial_value)				{ *this = initial_value; }
	sorn(long double initial_value)			{ *this = initial_value; }

	// assignment operators for native types: the lattice interval that encloses the value. A value
	// that is not a float is looked up on its side of its rounding to float, so a double just
	// above a lattice point, or a 64-bit integer past 2^24, is not put into the interval below (#2010).
	sorn& operator=(signed char rhs)		{ return assignNative(rhs); }
	sorn& operator=(short rhs)				{ return assignNative(rhs); }
	sorn& operator=(int rhs)				{ return assignNative(rhs); }
	sorn& operator=(long rhs)				{ return assignNative(rhs); }
	sorn& operator=(long long rhs)			{ return assignNative(rhs); }
	sorn& operator=(char rhs)				{ return assignNative(rhs); }
	sorn& operator=(unsigned short rhs)		{ return assignNative(rhs); }
	sorn& operator=(unsigned int rhs)		{ return assignNative(rhs); }
	sorn& operator=(unsigned long rhs)		{ return assignNative(rhs); }
	sorn& operator=(unsigned long long rhs)	{ return assignNative(rhs); }
	sorn& operator=(float rhs)				{ return assign(rhs); }
	sorn& operator=(double rhs)				{ return assignNative(rhs); }
	sorn& operator=(long double rhs)		{ return assignNative(rhs); }

	///////////////////////////////
	////////// operators //////////
	///////////////////////////////

	// write to output: operator<<(std::ostream&, const sorn&) is in iostream.hpp (#1334)

	// negation operator
	sorn operator-() const {
		if (iszero() || isnan()) {
			return *this;
		}
		else {
			sorn negated;
			negated.sornIntVal.lowerBound = (this->sornIntVal.upperBound == 0 ? this->sornIntVal.upperBound : -this->sornIntVal.upperBound);
			negated.sornIntVal.upperBound = (this->sornIntVal.lowerBound == 0 ? this->sornIntVal.lowerBound : -this->sornIntVal.lowerBound);
			negated.sornIntVal.lowerIsOpen = this->sornIntVal.upperIsOpen;
			negated.sornIntVal.upperIsOpen = this->sornIntVal.lowerIsOpen;
			return negated;
		}
	}

	// single operand addition

	// sorn + sorn
	sorn& operator+=(const sorn& rhs) {
		if (isnan() || rhs.isnan()) {
			return setnan();
		}
		if (rhs.iszero()) {
			return *this;
		}
		else if (this->iszero()) {
			*this = rhs;
			return *this;
		}
		else {
			// the bound sums are rounded outward, and a rounded bound is open (#2021)
			SORN_INTERVAL& v = this->sornIntVal;
			const SORN_INTERVAL& r = rhs.sornIntVal;
			bool lowerInexact{ false }, upperInexact{ false };
			const float lower = sornAddOut(v.lowerBound, r.lowerBound, false, lowerInexact);
			const float upper = sornAddOut(v.upperBound, r.upperBound, true, upperInexact);
			v = { lower, upper, v.lowerIsOpen || r.lowerIsOpen || lowerInexact, v.upperIsOpen || r.upperIsOpen || upperInexact };
			adaptToDT();
			return *this;
		}
	}
	// sorn + native scalar: the scalar is an exact value, not converted to a lattice interval first.
	// The scalar and the bound sums are rounded outward, so the result encloses the exact one (#2010).
	template<typename Scalar> requires std::is_arithmetic_v<Scalar>
	sorn& operator+=(Scalar rhs) {
		if (isnan()) return *this;
		const sornScalarEnclosure r = sornEnclose(rhs);
		shiftBounds(r.lo, r.hi, r.exact);
		adaptToDT();
		return *this;
	}

	// single operand subtraction

	// sorn - sorn
	sorn& operator-=(const sorn& rhs) {
		if (isnan() || rhs.isnan()) {
			return setnan();
		}
		if (rhs.iszero()) {
			return *this;
		}
		else if (this->iszero()) {
			*this = -rhs;
			return *this;
		}
		else {
			// the bound differences are rounded outward, and a rounded bound is open (#2021)
			SORN_INTERVAL& v = this->sornIntVal;
			const SORN_INTERVAL& r = rhs.sornIntVal;
			bool lowerInexact{ false }, upperInexact{ false };
			const float lower = sornAddOut(v.lowerBound, -r.upperBound, false, lowerInexact);
			const float upper = sornAddOut(v.upperBound, -r.lowerBound, true, upperInexact);
			v = { lower, upper, v.lowerIsOpen || r.upperIsOpen || lowerInexact, v.upperIsOpen || r.lowerIsOpen || upperInexact };
			adaptToDT();
			return *this;
		}
	}
	// sorn - native scalar, rounded outward as sorn + scalar is
	template<typename Scalar> requires std::is_arithmetic_v<Scalar>
	sorn& operator-=(Scalar rhs) {
		if (isnan()) return *this;
		const sornScalarEnclosure r = sornEnclose(rhs);
		shiftBounds(-r.hi, -r.lo, r.exact);
		adaptToDT();
		return *this;
	}

	// single operand multiplication

	// sorn * sorn
	sorn& operator*=(const sorn& rhs) {
		if (isnan() || rhs.isnan()) {
			return setnan();
		}
		if (rhs.iszero() || this->iszero()) {
			return setzero();
		}
		// interval product: the bounds are the smallest and largest of the four corner products.
		// A corner is attained (closed) when both of its factors are, or when one factor is a
		// closed zero, which makes the product zero whatever the other operand is. A bound
		// product with a zero factor is zero, also against an infinite bound, where IEEE
		// 0 * inf is NaN. The earlier case table ORed the open conditions, so it dropped the
		// attained zero of [-inf,-2) * [0,1] (#1972). Each corner is rounded down for the lower
		// bound and up for the upper one, and a rounded corner is open on that side (#2021).
		struct Corner { float down; float up; bool downOpen; bool upOpen; };
		auto corner = [](float a, bool aOpen, float b, bool bOpen) {
			const bool closedZero = (a == 0.0f && !aOpen) || (b == 0.0f && !bOpen);
			const bool open = (aOpen || bOpen) && !closedZero;
			bool downInexact{ false }, upInexact{ false };
			const float down = sornMulOut(a, b, false, downInexact);
			const float up   = sornMulOut(a, b, true, upInexact);
			return Corner{ down, up, open || downInexact, open || upInexact };
		};
		const SORN_INTERVAL& l = this->sornIntVal;
		const SORN_INTERVAL& r = rhs.sornIntVal;
		const Corner corners[4] = {
			corner(l.lowerBound, l.lowerIsOpen, r.lowerBound, r.lowerIsOpen),
			corner(l.lowerBound, l.lowerIsOpen, r.upperBound, r.upperIsOpen),
			corner(l.upperBound, l.upperIsOpen, r.lowerBound, r.lowerIsOpen),
			corner(l.upperBound, l.upperIsOpen, r.upperBound, r.upperIsOpen),
		};
		Corner lo = corners[0], hi = corners[0];
		for (const Corner& c : corners) {
			if (c.down < lo.down || (c.down == lo.down && !c.downOpen)) lo = c;
			if (c.up > hi.up || (c.up == hi.up && !c.upOpen)) hi = c;
		}
		this->sornIntVal = { lo.down, hi.up, lo.downOpen, hi.upOpen };
		adaptToDT();
		return *this;
	}
	// sorn * native scalar, with the scalar and the bound products rounded outward (#2010). A zero
	// factor makes the product zero whatever the other one is, as in sorn * sorn: the exact zero
	// times an infinite scalar is 0, and so is a zero scalar times an infinite bound.
	template<typename Scalar> requires std::is_arithmetic_v<Scalar>
	sorn& operator*=(Scalar rhs) {
		if (isnan()) return *this;
		const sornScalarEnclosure r = sornEnclose(rhs);
		if (std::isnan(r.lo)) return setnan();
		if (r.exact && r.lo == 0) return setzero();
		if (iszero()) return *this;
		// |rhs| lies in [mlo, mhi], mlo >= 0; a negative scalar negates the product afterwards
		const bool negative = (r.lo < 0);
		const float mlo = (negative ? -r.hi : r.lo);
		const float mhi = (negative ? -r.lo : r.hi);
		SORN_INTERVAL& v = this->sornIntVal;
		bool lowerInexact{ false }, upperInexact{ false };
		const float lower = sornMulOut(v.lowerBound, (v.lowerBound >= 0 ? mlo : mhi), false, lowerInexact);
		const float upper = sornMulOut(v.upperBound, (v.upperBound >= 0 ? mhi : mlo), true, upperInexact);
		v.lowerIsOpen = v.lowerIsOpen || lowerInexact || (!r.exact && v.lowerBound != 0 && std::isfinite(v.lowerBound));
		v.upperIsOpen = v.upperIsOpen || upperInexact || (!r.exact && v.upperBound != 0 && std::isfinite(v.upperBound));
		v.lowerBound = lower;
		v.upperBound = upper;
		if (negative) negateBounds();
		adaptToDT();
		return *this;
	}

	// single operand division

	// sorn / sorn: multiply by the reciprocal of the divisor. A divisor that straddles zero makes
	// the quotient of a nonzero dividend unbounded on both sides, so the result is the whole
	// lattice; dividing by the exact zero is NaN (the empty set). The exact zero divided by any
	// other divisor, one that straddles zero included, is 0: 0 / y = 0 for every nonzero y in the
	// divisor, and the zero in it is excluded as it is for the exact zero divisor (#2010).
	sorn& operator/=(const sorn& rhs) {
		if (isnan() || rhs.isnan() || rhs.iszero()) {
			return setnan();
		}
		if (iszero()) {
			return *this;
		}
		sorn recip;
		if (!reciprocal(rhs.sornIntVal, recip.sornIntVal)) {
			this->sornIntVal = { -kInf, kInf, false, false };
			adaptToDT();
			return *this;
		}
		return *this *= recip;
	}
	// sorn / native scalar, with the scalar and the bound quotients rounded outward (#2010)
	template<typename Scalar> requires std::is_arithmetic_v<Scalar>
	sorn& operator/=(Scalar rhs) {
		if (isnan()) return *this;
		const sornScalarEnclosure r = sornEnclose(rhs);
		if (std::isnan(r.lo) || (r.exact && r.lo == 0)) return setnan();
		// |rhs| lies in [mlo, mhi], mlo >= 0; a negative scalar negates the quotient afterwards
		const bool negative = (r.lo < 0);
		const float mlo = (negative ? -r.hi : r.lo);
		const float mhi = (negative ? -r.lo : r.hi);
		SORN_INTERVAL& v = this->sornIntVal;
		bool lowerInexact{ false }, upperInexact{ false };
		const float lower = sornDivOut(v.lowerBound, (v.lowerBound >= 0 ? mhi : mlo), false, lowerInexact);
		const float upper = sornDivOut(v.upperBound, (v.upperBound >= 0 ? mlo : mhi), true, upperInexact);
		v.lowerIsOpen = v.lowerIsOpen || lowerInexact || (!r.exact && v.lowerBound != 0 && std::isfinite(v.lowerBound));
		v.upperIsOpen = v.upperIsOpen || upperInexact || (!r.exact && v.upperBound != 0 && std::isfinite(v.upperBound));
		v.lowerBound = lower;
		v.upperBound = upper;
		if (negative) negateBounds();
		adaptToDT();
		return *this;
	}

	//////////////////////////////////////////
	////////// arithmetic functions //////////
	//////////////////////////////////////////

	// absolute value
	sorn abs() const {
		if (isnan()) return *this;
		sorn absVal;
		if (this->sornIntVal.upperBound <= 0) {
			absVal = -*this;
		}
		else if (this->sornIntVal.lowerBound < 0) {
			absVal.sornIntVal.lowerBound = 0;
			absVal.sornIntVal.lowerIsOpen = false;
			if (std::abs(this->sornIntVal.lowerBound) > std::abs(this->sornIntVal.upperBound)) {
				absVal.sornIntVal.upperBound = std::abs(this->sornIntVal.lowerBound);
				absVal.sornIntVal.upperIsOpen = this->sornIntVal.lowerIsOpen;
			}
			else {
				absVal.sornIntVal.upperBound = this->sornIntVal.upperBound;
				absVal.sornIntVal.upperIsOpen = this->sornIntVal.upperIsOpen;
			}
		}
		else {
			absVal = *this;
		};
		absVal.adaptToDT();
		return absVal;
	}

	// to_native: the midpoint of the interval. For a single lattice interval float2SORN maps it back
	// into that interval; a span maps back to the lattice interval holding its midpoint. When the
	// midpoint rounds outside the interval on its way back to float, as it does for (0, 2^-149], a
	// closed bound is reported instead. An interval with one infinite bound reports that bound; the
	// whole real line and NaN report NaN.
	template<typename NativeReal>
	NativeReal to_native() const noexcept {
		if (isnan()) return std::numeric_limits<NativeReal>::quiet_NaN();
		const NativeReal lo = static_cast<NativeReal>(sornIntVal.lowerBound);
		const NativeReal hi = static_cast<NativeReal>(sornIntVal.upperBound);
		if (std::isinf(lo) && std::isinf(hi)) return (lo == hi ? lo : std::numeric_limits<NativeReal>::quiet_NaN());
		if (std::isinf(lo)) return lo;
		if (std::isinf(hi)) return hi;
		// hi - lo overflows a float for a wide finite interval; halve first then (#2010)
		const NativeReal width = hi - lo;
		const NativeReal mid = (std::isfinite(width) ? lo + width / NativeReal(2) : lo / NativeReal(2) + hi / NativeReal(2));
		const Real back = static_cast<Real>(mid);
		const SORN_INTERVAL& i = sornIntVal;
		const bool inside = ((back > i.lowerBound) || (back == i.lowerBound && !i.lowerIsOpen)) &&
		                    ((back < i.upperBound) || (back == i.upperBound && !i.upperIsOpen));
		if (inside) return mid;
		if (!i.upperIsOpen) return hi;
		if (!i.lowerIsOpen) return lo;
		return mid;
	}
	// make conversions to native types explicit
	explicit operator float()       const noexcept { return to_native<float>(); }
	explicit operator double()      const noexcept { return to_native<double>(); }
	explicit operator long double() const noexcept { return to_native<long double>(); }

	///////////////////////////////////////
	////////// helper functions //////////
	///////////////////////////////////////

	// special value functions
	bool iszero() const noexcept { return this->sornIntVal.isZero(); }
	bool isnan()  const noexcept { return std::isnan(this->sornIntVal.lowerBound) || std::isnan(this->sornIntVal.upperBound); }

	sorn& setzero() noexcept { this->sornIntVal = { 0, 0, false, false }; return *this; }
	sorn& setnan()  noexcept { this->sornIntVal = { kNaN, kNaN, false, false }; return *this; }

	// specific lattice values; a configuration without negative intervals has no minneg or
	// maxneg, and those come out NaN
	sorn& maxpos() {	// the positive interval with the largest finite upper bound
		const auto& dt = datatype();
		for (auto it = dt.rbegin(); it != dt.rend(); ++it) {
			if (it->upperBound > 0 && std::isfinite(it->upperBound)) { this->sornIntVal = *it; return *this; }
		}
		return setnan();
	}
	sorn& minpos() {	// the positive interval closest to zero
		for (const auto& interval : datatype()) {
			if (interval.lowerBound >= 0 && interval.upperBound > 0) { this->sornIntVal = interval; return *this; }
		}
		return setnan();
	}
	sorn& minneg() {	// the negative interval closest to zero
		const auto& dt = datatype();
		for (auto it = dt.rbegin(); it != dt.rend(); ++it) {
			if (it->lowerBound < 0 && it->upperBound <= 0) { this->sornIntVal = *it; return *this; }
		}
		return setnan();
	}
	sorn& maxneg() {	// the negative interval with the most negative finite lower bound
		for (const auto& interval : datatype()) {
			if (interval.lowerBound < 0 && std::isfinite(interval.lowerBound)) { this->sornIntVal = interval; return *this; }
		}
		return setnan();
	}
	// setinf: the interval reaching infinity; without one, infinity saturates to the outermost
	// interval, as converting INFINITY does
	sorn& setinf(bool negative) {
		return assign(negative ? -kInf : kInf);
	}

	Real minVal() const noexcept { return datatype().front().lowerBound; }
	Real maxVal() const noexcept { return datatype().back().upperBound; }

	// adaptToDT: adapt the variable sornIntVal to the defined datatype
	SORN_INTERVAL& adaptToDT() {
		const auto& dt = datatype();
		// 0. an indeterminate bound (inf - inf, NaN input) is the empty set; drop a negative zero
		if (isnan()) {
			setnan();
			return this->sornIntVal;
		}
		if (this->sornIntVal.lowerBound == 0) this->sornIntVal.lowerBound = 0;
		if (this->sornIntVal.upperBound == 0) this->sornIntVal.upperBound = 0;
		// 1. lower bound
		bool lowerExists = false;
		// 1.1. check if lower bound already exisits in DT
		for (size_t b = 0; b < dt.size(); b++) {
			if (this->sornIntVal.lowerBound == dt[b].lowerBound && this->sornIntVal.lowerIsOpen == dt[b].lowerIsOpen) {
				lowerExists = true;
			}
		}
		// 1.2. adapt lower bound to DT
		if (not lowerExists) {
			// 1.2.1. lower bound < min value
			if (this->sornIntVal.lowerBound < minVal()) {
				this->sornIntVal.lowerBound = minVal();
				this->sornIntVal.lowerIsOpen = dt[0].lowerIsOpen;
			}
			// 1.2.1b. lower bound at or past the top of a lattice without infinity: the whole
			// result lies above the lattice and saturates to the outermost interval, as a
			// conversion does (left alone, the lower bound stayed above the clamped upper one)
			else if (this->sornIntVal.lowerBound >= maxVal()) {
				this->sornIntVal.lowerBound = dt[dt.size() - 1].lowerBound;
				this->sornIntVal.lowerIsOpen = dt[dt.size() - 1].lowerIsOpen;
			}
			else {
				for (size_t b = 0; b < dt.size(); b++) {
					// 1.2.2. lower bound value between two possible values (from DT)
					if (this->sornIntVal.lowerBound > dt[b].lowerBound && this->sornIntVal.lowerBound < dt[b].upperBound) {
						this->sornIntVal.lowerBound = dt[b].lowerBound;
						this->sornIntVal.lowerIsOpen = dt[b].lowerIsOpen;
					} // 1.2.3. lower bound in DT but open/close condition does not match
					else if (this->sornIntVal.lowerBound == dt[b].lowerBound && this->sornIntVal.lowerIsOpen != dt[b].lowerIsOpen) {
						// cond. is "open" but has to be "closed"
						if (this->sornIntVal.lowerIsOpen) {
							this->sornIntVal.lowerBound = dt[b].lowerBound;
							this->sornIntVal.lowerIsOpen = dt[b].lowerIsOpen;
						}
						// cond. is "closed" but has to be "open" --> utilize prevoius interval
						else if (not this->sornIntVal.lowerIsOpen && b > 0) {
							this->sornIntVal.lowerBound = dt[b - 1].lowerBound;
							this->sornIntVal.lowerIsOpen = dt[b - 1].lowerIsOpen;
						}
					}
				}
			}
		}
		// 2. upper bound
		bool upperExists = false;
		// 2.1.check if upper bound already exisits in DT
		for (size_t b = 0; b < dt.size(); b++) {
			if (this->sornIntVal.upperBound == dt[b].upperBound && this->sornIntVal.upperIsOpen == dt[b].upperIsOpen) {
				upperExists = true;
			}
		}
		// 2.2. adapt upper bound to DT
		if (not upperExists) {
			// 2.2.1 upper bound > max value
			if (this->sornIntVal.upperBound > maxVal()) {
				this->sornIntVal.upperBound = maxVal();
				this->sornIntVal.upperIsOpen = dt[dt.size() - 1].upperIsOpen;
			}
			// 2.2.1b. upper bound at or below the bottom of the lattice: saturate to the
			// outermost interval on that side, the mirror of 1.2.1b
			else if (this->sornIntVal.upperBound <= minVal()) {
				this->sornIntVal.upperBound = dt[0].upperBound;
				this->sornIntVal.upperIsOpen = dt[0].upperIsOpen;
			}
			else {
				for (size_t b = 0; b < dt.size(); b++) {
					// 2.2.2. upper bound value between two possible values (from DT)
					if (this->sornIntVal.upperBound > dt[b].lowerBound && this->sornIntVal.upperBound < dt[b].upperBound) {
						this->sornIntVal.upperBound = dt[b].upperBound;
						this->sornIntVal.upperIsOpen = dt[b].upperIsOpen;
						break; // when the upper bound it set to open zero the next iteration would set it to closed zero
					} // 2.2.3. upper bound in DT but open/close condition does not match
					else if (this->sornIntVal.upperBound == dt[b].upperBound && this->sornIntVal.upperIsOpen != dt[b].upperIsOpen) {
						// cond. is "open" but has to be "closed"
						if (this->sornIntVal.upperIsOpen) {
							this->sornIntVal.upperBound = dt[b].upperBound;
							this->sornIntVal.upperIsOpen = dt[b].upperIsOpen;
						} // cond. is "closed" but has to be "open" --> utilize next interval
						else if (not this->sornIntVal.upperIsOpen && b + 1 < dt.size()) {
							this->sornIntVal.upperBound = dt[b+1].upperBound;
							this->sornIntVal.upperIsOpen = dt[b+1].upperIsOpen;
						}
						break;
					}
				}
			}
		}
		// 3. return value
		return this->sornIntVal;
	}

	// negateBounds: the interval of the negated values, -[a,b) = (-b,-a]
	SORN_INTERVAL& negateBounds() {
		switchBounds();
		this->sornIntVal.lowerBound = -this->sornIntVal.lowerBound;
		this->sornIntVal.upperBound = -this->sornIntVal.upperBound;
		return this->sornIntVal;
	}

	// switchBounds: switch Boundaries of sornIntVal
	SORN_INTERVAL& switchBounds() {
		float newLow  = this->sornIntVal.upperBound;
		float newUp   = this->sornIntVal.lowerBound;
		bool  newLowC = this->sornIntVal.upperIsOpen;
		bool  newUpC  = this->sornIntVal.lowerIsOpen;
		this->sornIntVal = { newLow,newUp,newLowC,newUpC };
		return this->sornIntVal;
	}

	// setbits: set the value from the low sornBits bits of an encoding, as setBits does
	void setbits(std::uint64_t v) {
		std::bitset<sornBits> bin;
		for (size_t b = 0; b < sornBits && b < 64; b++) {
			bin[b] = ((v >> b) & 1u) != 0;
		}
		setBits(bin);
	}

	// setBits: set the sornIntVal parameter via binary input (input type: bitset). The value spans
	// from the lowest to the highest set bit; no bit set is the empty set, NaN.
	SORN_INTERVAL& setBits(const std::bitset<sornBits>& bin) {
		const auto& dt = datatype();
		bool lowerSet = false;
		for (size_t b = 0; b < sornBits; b++) {
			if (bin[b] == 1 && not lowerSet) {
				this->sornIntVal = dt[b];
				lowerSet = true;
			}
			else if (bin[b] == 1 && lowerSet) {
				this->sornIntVal.upperBound = dt[b].upperBound;
				this->sornIntVal.upperIsOpen = dt[b].upperIsOpen;
			}
		}
		if (not lowerSet) setnan();
		return this->sornIntVal;
	}

	//////////////////////////////////////
	////////// getter functions //////////
	//////////////////////////////////////

	// getConfig: writes all configuration parameters and flags to a string
	std::string getConfig() const;   // defined in manipulators.hpp (#1334)

	// getDT: writes the SORN datatype configuration to a string
	std::string getDT() const;       // defined in manipulators.hpp (#1334)

	// getBits: the binary representation of a SORN value using bitset class, one bit for every
	// lattice interval the value covers (note: displayed from max downto 0 when using << operator)
	std::bitset<sornBits> getBits() const {
		const auto& dt = datatype();
		const SORN_INTERVAL& v = this->sornIntVal;
		std::bitset<sornBits> raw_bits;
		for (size_t b = 0; b < sornBits; b++) {
			const SORN_INTERVAL& interval = dt[b];
			const bool lowerInside = (interval.lowerBound > v.lowerBound) ||
			                         (interval.lowerBound == v.lowerBound && (not v.lowerIsOpen || interval.lowerIsOpen));
			const bool upperInside = (interval.upperBound < v.upperBound) ||
			                         (interval.upperBound == v.upperBound && (not v.upperIsOpen || interval.upperIsOpen));
			raw_bits[b] = lowerInside && upperInside;
		}
		return raw_bits;
	}

private:
	sorn& assign(float rhs) {
		float2SORN(rhs, datatype(), this->sornIntVal);
		return *this;
	}
	// assignNative: a native value that may not be a float, looked up on its side of its float rounding
	template<typename Scalar>
	sorn& assignNative(Scalar rhs) {
		const float f = static_cast<float>(rhs);
		float2SORN(f, datatype(), this->sornIntVal, sornCompareToFloat(rhs, f));
		return *this;
	}
	// shiftBounds: add the exact value enclosed by [rlo, rhi] to the bounds, rounding outward; a
	// rounded bound, or one moved by an inexact scalar, excludes itself and becomes open
	void shiftBounds(float rlo, float rhi, bool exact) {
		SORN_INTERVAL& v = this->sornIntVal;
		bool lowerInexact{ false }, upperInexact{ false };
		const float lower = sornAddOut(v.lowerBound, rlo, false, lowerInexact);
		const float upper = sornAddOut(v.upperBound, rhi, true, upperInexact);
		v.lowerIsOpen = v.lowerIsOpen || lowerInexact || (!exact && std::isfinite(v.lowerBound));
		v.upperIsOpen = v.upperIsOpen || upperInexact || (!exact && std::isfinite(v.upperBound));
		v.lowerBound = lower;
		v.upperBound = upper;
	}

	// reciprocal: 1/y for an interval y that excludes zero, with the open/closed conditions
	// carried to the swapped bounds. Each bound is rounded outward, and a rounded bound is open
	// (#2021). Returns false when y contains zero.
	static bool reciprocal(const SORN_INTERVAL& y, SORN_INTERVAL& r) {
		const bool positive = (y.lowerBound > 0) || (y.lowerBound == 0 && y.lowerIsOpen && y.upperBound > 0);
		const bool negative = (y.upperBound < 0) || (y.upperBound == 0 && y.upperIsOpen && y.lowerBound < 0);
		// 1/b rounded outward, for b != 0: 1/b = -(1/|b|) for a negative b, which turns the
		// direction of the rounding around
		auto recip = [](float b, bool up, bool& inexact) {
			return (b > 0) ? sornDivOut(1.0f, b, up, inexact) : -sornDivOut(1.0f, -b, !up, inexact);
		};
		bool lowerInexact{ false }, upperInexact{ false };
		if (positive) {
			r.lowerBound = recip(y.upperBound, false, lowerInexact);                                // 1/inf = 0
			r.upperBound = (y.lowerBound == 0 ? kInf : recip(y.lowerBound, true, upperInexact));
		}
		else if (negative) {
			r.lowerBound = (y.upperBound == 0 ? -kInf : recip(y.upperBound, false, lowerInexact));  // stored zeros are +0
			r.upperBound = recip(y.lowerBound, true, upperInexact);
		}
		else {
			return false;
		}
		r.lowerIsOpen = y.upperIsOpen || lowerInexact;
		r.upperIsOpen = y.lowerIsOpen || upperInexact;
		return true;
	}

	template<signed int _sstart, signed int _sstop, unsigned int _ssteps, bool _llin, bool _hhalfopen, bool _nneg, bool _iinf, bool _zzero>
	friend std::ostream& operator<<(std::ostream&, const sorn< _sstart, _sstop, _ssteps, _llin, _hhalfopen, _nneg, _iinf, _zzero>&);

}; // end class sorn

//////////////////////////////////////////////////////////////////////////////////////////////////

#define SORN_TEMPLATE_PARAMS template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
#define SORN_TYPE sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>

// equality: the same interval, bounds and open/closed conditions alike. NaN, the empty set,
// compares unequal to everything, itself included, as an IEEE NaN does.
SORN_TEMPLATE_PARAMS
inline bool operator==(const SORN_TYPE& lhs, const SORN_TYPE& rhs) {
	const auto& l = lhs.sornIntVal;
	const auto& r = rhs.sornIntVal;
	return l.lowerBound == r.lowerBound && l.upperBound == r.upperBound &&
	       l.lowerIsOpen == r.lowerIsOpen && l.upperIsOpen == r.upperIsOpen;
}
SORN_TEMPLATE_PARAMS
inline bool operator!=(const SORN_TYPE& lhs, const SORN_TYPE& rhs) {
	return !(lhs == rhs);
}

// operator<<(std::ostream&, const sorn&) is declared a friend above and defined in iostream.hpp (#1334)

// two operand arithmetic: sorn op sorn, sorn op scalar, scalar op sorn. A scalar operand is an
// exact value; the operators take const references so temporaries and const operands work (#1972).

SORN_TEMPLATE_PARAMS
inline SORN_TYPE operator+(const SORN_TYPE& lhs, const SORN_TYPE& rhs) { SORN_TYPE sum = lhs; return sum += rhs; }
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero, typename Scalar> requires std::is_arithmetic_v<Scalar>
inline SORN_TYPE operator+(const SORN_TYPE& lhs, Scalar rhs) { SORN_TYPE sum = lhs; return sum += rhs; }
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero, typename Scalar> requires std::is_arithmetic_v<Scalar>
inline SORN_TYPE operator+(Scalar lhs, const SORN_TYPE& rhs) { SORN_TYPE sum = rhs; return sum += lhs; }

SORN_TEMPLATE_PARAMS
inline SORN_TYPE operator-(const SORN_TYPE& lhs, const SORN_TYPE& rhs) { SORN_TYPE dif = lhs; return dif -= rhs; }
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero, typename Scalar> requires std::is_arithmetic_v<Scalar>
inline SORN_TYPE operator-(const SORN_TYPE& lhs, Scalar rhs) { SORN_TYPE dif = lhs; return dif -= rhs; }
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero, typename Scalar> requires std::is_arithmetic_v<Scalar>
inline SORN_TYPE operator-(Scalar lhs, const SORN_TYPE& rhs) { SORN_TYPE dif = -rhs; return dif += lhs; }

SORN_TEMPLATE_PARAMS
inline SORN_TYPE operator*(const SORN_TYPE& lhs, const SORN_TYPE& rhs) { SORN_TYPE prod = lhs; return prod *= rhs; }
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero, typename Scalar> requires std::is_arithmetic_v<Scalar>
inline SORN_TYPE operator*(const SORN_TYPE& lhs, Scalar rhs) { SORN_TYPE prod = lhs; return prod *= rhs; }
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero, typename Scalar> requires std::is_arithmetic_v<Scalar>
inline SORN_TYPE operator*(Scalar lhs, const SORN_TYPE& rhs) { SORN_TYPE prod = rhs; return prod *= lhs; }

SORN_TEMPLATE_PARAMS
inline SORN_TYPE operator/(const SORN_TYPE& lhs, const SORN_TYPE& rhs) { SORN_TYPE quot = lhs; return quot /= rhs; }
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero, typename Scalar> requires std::is_arithmetic_v<Scalar>
inline SORN_TYPE operator/(const SORN_TYPE& lhs, Scalar rhs) { SORN_TYPE quot = lhs; return quot /= rhs; }
// scalar / sorn: the scalar as an exact point divided by the interval
template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero, typename Scalar> requires std::is_arithmetic_v<Scalar>
inline SORN_TYPE operator/(Scalar lhs, const SORN_TYPE& rhs) {
	SORN_TYPE quot;
	// a scalar that is not a float is the open interval between its float neighbours (#2010)
	const sornScalarEnclosure c = sornEnclose(lhs);
	quot.sornIntVal = { c.lo, c.hi, !c.exact, !c.exact };
	return quot /= rhs;
}

///////////////////////////////////////////
////////// arithmetic operations //////////
///////////////////////////////////////////

// absolute value
SORN_TEMPLATE_PARAMS
inline SORN_TYPE abs(SORN_TYPE op) {
	return op.abs();
}

// hypot(sorn,sorn)
SORN_TEMPLATE_PARAMS
inline SORN_TYPE hypot(SORN_TYPE lhs, SORN_TYPE rhs) {
	using std::sqrt;
	SORN_TYPE res, lhsAbs, rhsAbs;
	if (lhs.isnan() || rhs.isnan()) return res.setnan();
	// take abs value of inputs
	lhsAbs = lhs.abs();
	rhsAbs = rhs.abs();
	// carry out hypot on the abs values of the inputs
	res.sornIntVal.lowerBound = sqrt(lhsAbs.sornIntVal.lowerBound * lhsAbs.sornIntVal.lowerBound + rhsAbs.sornIntVal.lowerBound * rhsAbs.sornIntVal.lowerBound);
	res.sornIntVal.upperBound = sqrt(lhsAbs.sornIntVal.upperBound * lhsAbs.sornIntVal.upperBound + rhsAbs.sornIntVal.upperBound * rhsAbs.sornIntVal.upperBound);
	res.sornIntVal.lowerIsOpen = lhsAbs.sornIntVal.lowerIsOpen || rhsAbs.sornIntVal.lowerIsOpen;
	res.sornIntVal.upperIsOpen = lhsAbs.sornIntVal.upperIsOpen || rhsAbs.sornIntVal.upperIsOpen;
	res.adaptToDT();
	return res;
}

#undef SORN_TYPE
#undef SORN_TEMPLATE_PARAMS

}} // end namespace sw::universal
