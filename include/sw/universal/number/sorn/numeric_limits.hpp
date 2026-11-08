#pragma once
// numeric_limits.hpp: definition of numeric_limits for SORN types
//
// Copyright (C) 2017-2026 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <limits>

#include <universal/number/sorn/sorn_fwd.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>

// A sorn value is an interval of a fixed lattice, not a point, so the members below are the
// lattice's analogues of the IEEE quantities (#1986):
//   min(), max()       the positive interval nearest zero, and the outermost finite positive interval
//   lowest()           the lowest interval with a finite lower bound: maxneg with negatives, else
//                      the exact zero, else minpos (as lowest() is 0 for an unsigned integer)
//   epsilon()          the interval holding w, the width of the lattice interval just above 1;
//                      for float that interval is [1, 1+2^-23) and w is float's epsilon
//   round_error()      the interval holding w/2, the furthest a point of that interval lies from
//                      the midpoint that float(sorn) returns
//   denorm_min()       min(), as for any type without denormals
//   infinity()         the interval reaching +inf; a lattice without one saturates to its top
//   digits             the largest d with w <= 2^(1-d), so a d-digit binary format is no finer at 1;
//                      0 when nothing finite lies above 1
//   min_exponent ...   from the smallest positive and the largest finite lattice points, p and P:
//                      2^(min_exponent-1) <= p < 2^min_exponent and 2^(max_exponent-1) <= P < 2^max_exponent
// The lattice is a function-local static vector, so the functions are not constexpr. The constants
// are, and are computed from the template parameters the way setSornDT() builds the lattice.

namespace sw { namespace universal { namespace sorn_limits {

constexpr float kInf = std::numeric_limits<float>::infinity();

constexpr float pow2(int e) {
	float v = 1.0f;
	for (; e > 0; --e) v *= 2.0f;
	for (; e < 0; ++e) v /= 2.0f;
	return v;
}

// floor(log2(v)) for finite v > 0
constexpr int floor_log2(float v) {
	int e = 0;
	while (v >= 2.0f) { v /= 2.0f; ++e; }
	while (v < 1.0f) { v *= 2.0f; --e; }
	return e;
}

// ceil(log2(v)) for finite v > 0
constexpr int ceil_log2(float v) {
	const int e = floor_log2(v);
	return (pow2(e) == v ? e : e + 1);
}

// floor(log10(v)) and ceil(log10(v)) for finite v > 0
constexpr int floor_log10(double v) {
	int e = 0;
	while (v >= 10.0) { v /= 10.0; ++e; }
	while (v < 1.0) { v *= 10.0; --e; }
	return e;
}
constexpr int ceil_log10(double v) {
	int e = 0;
	double p = 1.0;
	while (p < v) { p *= 10.0; ++e; }
	while (p / 10.0 >= v) { p /= 10.0; --e; }
	return e;
}

// the positive part of the halfopen lattice, as setSornDT() builds it:
//   linear:      (b*step, (b+1)*step] for b = 0..steps-1, step = (stop-start)/steps
//   logarithmic: (0, 2^start], then (2^(b-1), 2^b] for b = start+1..stop
template<int start, int stop, unsigned steps, bool lin>
struct lattice {
	static constexpr float step = (float)(stop - start) / (float)steps;

	// smallest positive and largest finite lattice points
	static constexpr float smallest() { return lin ? 1 * step : pow2(start); }
	static constexpr float largest() { return lin ? (int)steps * step : pow2(stop); }

	// width of the lattice interval holding the smallest float above 1, inf when that float
	// lies above the largest finite point
	static constexpr float width_above_one() {
		if (!(largest() > 1.0f)) return kInf;
		if constexpr (lin) {
			for (int b = 0; b < (int)steps; ++b) {
				if ((b + 1) * step > 1.0f) return (b + 1) * step - b * step;
			}
			return kInf;
		}
		else {
			return (start >= 1 ? pow2(start) : 1.0f);
		}
	}

	static constexpr int digits() {
		const float w = width_above_one();
		if (w == kInf) return 0;
		const int d = 1 - ceil_log2(w);
		return (d > 0 ? d : 0);
	}
};

}}} // namespace sw::universal::sorn_limits

namespace std {

template<signed int _start, signed int _stop, unsigned int _steps, bool _lin, bool _halfopen, bool _neg, bool _inf, bool _zero>
class numeric_limits< sw::universal::sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero> > {
	using Lattice = sw::universal::sorn_limits::lattice<_start, _stop, _steps, _lin>;
	using SpecificValue = sw::universal::SpecificValue;
public:
	using SORN = sw::universal::sorn<_start, _stop, _steps, _lin, _halfopen, _neg, _inf, _zero>;
	static constexpr bool is_specialized = true;
	static SORN min()           { return SORN(SpecificValue::minpos); }   // the positive interval nearest zero
	static SORN max()           { return SORN(SpecificValue::maxpos); }   // the outermost finite positive interval
	static SORN lowest() {                                                 // the lowest finite interval
		if constexpr (_neg)  return SORN(SpecificValue::maxneg);
		else if constexpr (_zero) return SORN(SpecificValue::zero);
		else return SORN(SpecificValue::minpos);
	}
	static SORN epsilon()       { return SORN(Lattice::width_above_one()); }
	static SORN round_error()   { return SORN(Lattice::width_above_one() / 2.0f); }
	static SORN denorm_min()    { return min(); }
	static SORN infinity()      { return SORN(SpecificValue::infpos); }
	static SORN quiet_NaN()     { return SORN(SpecificValue::qnan); }      // the empty set
	static SORN signaling_NaN() { return SORN(SpecificValue::snan); }      // the same empty set

	static constexpr int digits       = Lattice::digits();
	static constexpr int digits10     = (digits > 1 ? (digits - 1) * 301 / 1000 : 0);   // floor((digits-1) * log10(2))
	static constexpr int max_digits10 = (digits > 0 ? 2 + digits * 301 / 1000 : 1);   // ceil(1 + digits * log10(2))
	static constexpr bool is_signed   = _neg;
	static constexpr bool is_integer  = false;
	static constexpr bool is_exact    = false;
	static constexpr int radix        = 2;

	static constexpr int min_exponent   = sw::universal::sorn_limits::floor_log2(Lattice::smallest()) + 1;
	static constexpr int min_exponent10 = sw::universal::sorn_limits::ceil_log10(Lattice::smallest());
	static constexpr int max_exponent   = sw::universal::sorn_limits::floor_log2(Lattice::largest()) + 1;
	static constexpr int max_exponent10 = sw::universal::sorn_limits::floor_log10(Lattice::largest());
	static constexpr bool has_infinity      = _inf;
	static constexpr bool has_quiet_NaN     = true;
	static constexpr bool has_signaling_NaN = false;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;

	static constexpr bool is_iec559 = false;
	static constexpr bool is_bounded = true;
	static constexpr bool is_modulo = false;
	static constexpr bool traps = false;
	static constexpr bool tinyness_before = false;
	static constexpr float_round_style round_style = round_indeterminate;   // conversion encloses, it does not round
};

}
