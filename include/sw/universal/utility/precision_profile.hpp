#pragma once
// precision_profile.hpp: precision as a function of magnitude -- the "decimals of accuracy" profile
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Gustafson (The End of Error, 2015) compares number systems by how many decimal digits they
// keep at each magnitude.  For an encoding x whose next larger encoding is x+, ulp(x) = x+ - x
// and the worst-case relative error of rounding to the nearest encoding is ulp / (2 |x|), so
//     decimals(x)      = -log10(ulp(x) / (2 |x|))
//     fraction_bits(x) =  log2(|x| / ulp(x))
// Averaged over the ulp instead, the relative error is ulp / (4 |x|): the curve shifts up by
// exactly log10(2) = 0.30 decimals and keeps its shape, so only the worst case is computed.
// A type has 0 decimals at magnitudes it cannot represent, below minpos or above maxpos.
//
// The profile covers the positive half of the encoding (every type here is symmetric):
//   - nbits <= 16: every encoding is decoded with setbits() and the positive finite values are
//     sorted, so x+ is simply the next value.  This holds for any encoding order, including
//     lns, whose ++ is not monotone across the sign of its exponent.
//   - wider types are sampled: a few points per binade, each converted to its nearest
//     encoding, with the spacing taken from ++ (std::nextafter for native floats) or, where
//     ++ does not land above x, from the encoding below.  Precision is constant, or a simple
//     sawtooth, within a binade for every format here, so a few points per binade suffice.
// Values are handled in double, which is exact for every type whose significand fits in 53
// bits; for wider types the spacing falls back to T's own subtraction of neighbours.
//
// Usage:
//   auto p = precision_profile_of<cfloat<16, 5, uint16_t, true, false, false>>("fp16");
//   write_precision_csv(p, "fp16.csv");
//   print_precision_table(std::cout, { p, q }, -24, 16, 2);
// and tools/notebooks/plot_precision_profiles.py overlays the CSV files.
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <limits>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>
#include <universal/native/ieee754_type_tag.hpp>
#include <universal/native/integer_type_tag.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal {

struct precision_point {
	double magnitude;
	double log2_magnitude;
	double decimals;        // -log10(ulp / (2 |x|))
	double fraction_bits;   // log2(|x| / ulp)
};

struct precision_profile {
	std::string label;      // a short name for tables and plots
	std::string type;       // type_tag of the type
	double minpos{ 0.0 };
	double maxpos{ 0.0 };
	bool exhaustive{ false };
	std::vector<precision_point> points;   // ascending magnitude
};

namespace detail {

inline precision_point make_precision_point(double x, double ulp) {
	const double r = x / ulp;
	return { x, std::log2(x), std::log10(2.0 * r), std::log2(r) };
}

template<typename T>
constexpr unsigned precision_nbits() {
	if constexpr (std::is_arithmetic_v<T>) return static_cast<unsigned>(sizeof(T) * 8u);
	else return static_cast<unsigned>(T::nbits);
}

template<typename T>
double precision_minpos() {
	if constexpr (std::is_integral_v<T>) return 1.0;
	else if constexpr (std::is_floating_point_v<T>) return static_cast<double>(std::numeric_limits<T>::denorm_min());
	else return double(T(SpecificValue::minpos));
}

template<typename T>
double precision_maxpos() {
	if constexpr (std::is_arithmetic_v<T>) return static_cast<double>(std::numeric_limits<T>::max());
	else return double(T(SpecificValue::maxpos));
}

// the spacing at x: to the next encoding up, or to the one below when there is none above
template<typename T>
bool precision_spacing(const T& x, double maxpos, double& ulp) {
	const double dx = double(x);
	if constexpr (std::is_integral_v<T>) {
		ulp = 1.0;
		return true;
	}
	else if constexpr (std::is_floating_point_v<T>) {
		const T up = std::nextafter(x, std::numeric_limits<T>::infinity());
		if (std::isfinite(up)) { ulp = double(up) - dx; return ulp > 0.0; }
		ulp = dx - double(std::nextafter(x, T(0)));
		return ulp > 0.0;
	}
	else {
		T up = x;
		++up;
		const double du = double(up);
		if (du == dx && !(up == x)) {                 // neighbours that a double cannot tell apart: subtract in T
			T d = up;
			d -= x;
			ulp = double(d);
			return ulp > 0.0;
		}
		if (du > dx && du <= maxpos) {
			ulp = du - dx;
			return ulp > 0.0;
		}
		T down = x;
		--down;
		const double dd = double(down);
		if (dd < dx && dd > 0.0) { ulp = dx - dd; return ulp > 0.0; }
		return false;
	}
}

}  // namespace detail

// the precision profile of T: exhaustive for nbits <= exhaustiveLimit, sampled otherwise
template<typename T>
precision_profile precision_profile_of(const std::string& label = "", unsigned samplesPerBinade = 4, unsigned exhaustiveLimit = 16) {
	precision_profile p;
	p.type = type_tag(T{});
	p.label = label.empty() ? p.type : label;
	p.minpos = detail::precision_minpos<T>();
	p.maxpos = detail::precision_maxpos<T>();
	constexpr unsigned nbits = detail::precision_nbits<T>();

	if constexpr (std::is_integral_v<T>) {
		if (nbits <= exhaustiveLimit) {
			p.exhaustive = true;
			for (double x = 1.0; x <= p.maxpos; x += 1.0) p.points.push_back(detail::make_precision_point(x, 1.0));
			return p;
		}
	}
	else if constexpr (!std::is_floating_point_v<T>) {
		if (nbits <= exhaustiveLimit) {
			p.exhaustive = true;
			std::vector<double> v;
			for (std::uint64_t bits = 0; bits < (std::uint64_t(1) << nbits); ++bits) {
				T x;
				x.setbits(bits);
				const double d = double(x);
				if (std::isfinite(d) && d > 0.0) v.push_back(d);
			}
			std::sort(v.begin(), v.end());
			v.erase(std::unique(v.begin(), v.end()), v.end());
			for (std::size_t i = 0; i + 1 < v.size(); ++i) p.points.push_back(detail::make_precision_point(v[i], v[i + 1] - v[i]));
			if (v.size() >= 2) p.points.push_back(detail::make_precision_point(v.back(), v.back() - v[v.size() - 2]));   // maxpos: the spacing below
			return p;
		}
	}

	// sampled: samplesPerBinade points in every binade between minpos and maxpos, plus both ends
	std::vector<double> targets{ p.minpos, p.maxpos };
	const int lo = static_cast<int>(std::floor(std::log2(p.minpos)));
	const int hi = static_cast<int>(std::floor(std::log2(p.maxpos)));
	for (int s = lo; s <= hi; ++s) {
		for (unsigned j = 0; j < samplesPerBinade; ++j) {
			const double t = std::ldexp(1.0 + double(j) / double(samplesPerBinade), s);
			if (t >= p.minpos && t <= p.maxpos) targets.push_back(t);
		}
	}
	for (double t : targets) {
		T x;
		if constexpr (std::is_integral_v<T>) x = static_cast<T>(t);
		else if constexpr (std::is_floating_point_v<T>) x = static_cast<T>(t);
		else x = t;
		const double dx = double(x);
		double ulp = 0.0;
		if (!(dx > 0.0) || !std::isfinite(dx) || !detail::precision_spacing(x, p.maxpos, ulp)) continue;
		p.points.push_back(detail::make_precision_point(dx, ulp));
	}
	std::sort(p.points.begin(), p.points.end(), [](const precision_point& a, const precision_point& b) { return a.magnitude < b.magnitude; });
	p.points.erase(std::unique(p.points.begin(), p.points.end(), [](const precision_point& a, const precision_point& b) { return a.magnitude == b.magnitude; }), p.points.end());
	return p;
}

// the decimals of accuracy at magnitude m: those of the largest profiled encoding <= m, and 0
// outside [minpos, maxpos]
inline double decimals_at(const precision_profile& p, double m) {
	if (!(m >= p.minpos) || m > p.maxpos || p.points.empty()) return 0.0;
	auto it = std::upper_bound(p.points.begin(), p.points.end(), m, [](double v, const precision_point& q) { return v < q.magnitude; });
	if (it == p.points.begin()) return p.points.front().decimals;
	return std::prev(it)->decimals;
}

// one CSV per profile: magnitude, log2_magnitude, decimals, fraction_bits
inline bool write_precision_csv(const precision_profile& p, const std::string& path) {
	std::ofstream out(path);
	if (!out) return false;
	out << "# label: " << p.label << '\n'
	    << "# type: " << p.type << '\n'
	    << "# minpos: " << std::setprecision(17) << p.minpos << '\n'
	    << "# maxpos: " << p.maxpos << '\n'
	    << "# sampling: " << (p.exhaustive ? "exhaustive" : "sampled") << '\n'
	    << "magnitude,log2_magnitude,decimals,fraction_bits\n";
	for (const precision_point& q : p.points) out << q.magnitude << ',' << q.log2_magnitude << ',' << q.decimals << ',' << q.fraction_bits << '\n';
	return bool(out);
}

// a text view for terminals and CI: decimals of accuracy at 2^k, k = log2Lo, log2Lo + step, ...
inline void print_precision_table(std::ostream& os, const std::vector<precision_profile>& profiles, int log2Lo, int log2Hi, int step = 1, int width = 14) {
	os << std::setw(10) << "magnitude";
	for (const precision_profile& p : profiles) os << std::setw(width) << p.label;
	os << '\n' << std::string(10 + static_cast<std::size_t>(width) * profiles.size(), '-') << '\n';
	const auto flags = os.flags();
	const auto prec = os.precision();
	for (int k = log2Lo; k <= log2Hi; k += step) {
		os << "    2^" << std::left << std::setw(4) << k << std::right;
		for (const precision_profile& p : profiles) os << std::setw(width) << std::fixed << std::setprecision(2) << decimals_at(p, std::ldexp(1.0, k));
		os << '\n';
	}
	os.flags(flags);
	os.precision(prec);
}

}}  // namespace sw::universal
