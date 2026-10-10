#pragma once
// tiles.hpp: ucalc support for the ubit tile number systems, areal and poxel
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// A tile is an exact lattice point (ubit = 0) or the open interval to the next lattice
// point (ubit = 1).  ucalc registers two families for each tile type:
//
//   areal32, poxel32, ...     a single tile.  Arithmetic has the library's sticky-flag
//                             semantics: the ubit says "inexact", but once an operand is
//                             open the result tile need not contain the true result.
//   areal32i, poxel32i, ...   a tile_interval: a contiguous run of tiles that is
//                             guaranteed to contain the true result.  Containment is the
//                             invariant: a function without an enclosing implementation
//                             returns the entire line, never a rounded point.
//
// Literals are read from their decimal text, not through a double, so 0.1 lands in the
// tile that contains one tenth.  Values carry their tile metadata (ubit, tile count, sign
// verdict) in Value::tile_kind and friends; native_rep shows the set itself.
//
// IMPORTANT: include after type_dispatch.hpp and after the areal and poxel headers.
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

#include <universal/utility/tile_interval.hpp>

namespace sw { namespace ucalc {

namespace tiles {

using sw::universal::tile_interval;
using sw::universal::tile_traits;

template<typename Tile> struct is_areal : std::false_type {};
template<unsigned nbits, unsigned es, typename bt>
struct is_areal<sw::universal::areal<nbits, es, bt>> : std::true_type {};

// significant digits that keep neighbouring lattice points apart
template<typename Tile>
constexpr int digits() {
	return std::numeric_limits<Tile>::max_digits10 > 0 ? std::numeric_limits<Tile>::max_digits10 : 17;
}

///////////////////////////////////////////////////////////////////////////
// decimal text -> the tile that contains it

// poxel has no text parser of its own: convert the decimal exactly, load fbits + 3
// significand bits into a blocktriple, and put the dropped tail in the lowest bit as
// a sticky bit.  poxel::assign() then truncates the encoding and records the dropped bits,
// which places the value in its unique tile, on the correct side for negative values too.
template<unsigned BigBits, unsigned nbits, unsigned es, typename bt>
bool poxel_from_binary(const sw::universal::decimal_to_binary::basic_result<BigBits>& d,
                       sw::universal::poxel<nbits, es, bt>& v) {
	using P  = sw::universal::poxel<nbits, es, bt>;
	using BT = sw::universal::blocktriple<P::fbits, sw::universal::BlockTripleOperator::DIV, bt>;   // the widest radix: room below for the sticky bit
	constexpr unsigned radix  = static_cast<unsigned>(BT::radix);
	constexpr unsigned target = P::fbits + 3u;   // the hidden bit and the fbits + 2 bits assign() reads
	static_assert(radix >= target, "poxel_from_binary: no room below the significand for the sticky bit");
	if (!d.valid) return false;
	if (d.is_zero) { v = P(0); return true; }
	constexpr std::int64_t scale_limit = std::int64_t(1) << 30;   // far outside any posit range
	const std::int64_t scale = std::clamp(d.binary_scale, -scale_limit, scale_limit);
	BT t;
	t.setnormal();
	t.setsign(d.negative);
	t.setscale(static_cast<int>(scale));
	constexpr unsigned offset = radix + 1u - target;
	for (unsigned i = 0; i < target; ++i) {
		if (d.mantissa.at(i)) t.setbit(offset + i, true);
	}
	if (d.guard_bit || d.sticky_bit) t.setbit(0, true);
	v.assign(t, false);
	return true;
}

template<unsigned nbits, unsigned es, typename bt>
bool parse_tile(std::string_view s, sw::universal::poxel<nbits, es, bt>& v) {
	namespace d2b = sw::universal::decimal_to_binary;
	using P = sw::universal::poxel<nbits, es, bt>;
	constexpr unsigned target = P::fbits + 3u;
	const auto scan = sw::universal::string_parse::scan_decimal_float(s);
	if (!scan.valid) return false;
	const std::uint64_t need = d2b::detail::required_working_bits(scan, target);
	if (need <= 2048u)  return poxel_from_binary(d2b::convert<2048u>(scan, target), v);
	if (need <= 8192u)  return poxel_from_binary(d2b::convert<8192u>(scan, target), v);
	if (need <= 32768u) return poxel_from_binary(d2b::convert<32768u>(scan, target), v);
	return false;
}

template<unsigned nbits, unsigned es, typename bt>
bool parse_tile(std::string_view s, sw::universal::areal<nbits, es, bt>& v) {
	return sw::universal::areal_parse::faithful(s, v);
}

// the open-or-closed run of tiles strictly between the neighbours of d: any real within
// half an ulp of the double d.  The fallback for text the exact parsers do not read.
template<typename Tile>
tile_interval<Tile> bracket(double d) {
	using I = tile_interval<Tile>;
	using T = tile_traits<Tile>;
	if (std::isnan(d)) return I::nan();
	auto key = [](double x) -> std::int64_t {
		if (std::isinf(x)) return x > 0 ? T::kmax : -T::kmax;
		return std::clamp(T::key(Tile(x)), -T::kmax, T::kmax);
	};
	const double below = std::nextafter(d, -std::numeric_limits<double>::infinity());
	const double above = std::nextafter(d, std::numeric_limits<double>::infinity());
	std::int64_t lo = key(below), hi = key(above);
	if ((lo & 1) == 0 && lo < T::kmax) ++lo;   // the neighbours themselves are excluded
	if ((hi & 1) == 0 && hi > -T::kmax) --hi;
	return I::from_keys(lo, std::max(lo, hi));
}

// the box of a literal: the single tile that contains the decimal value
template<typename Tile>
tile_interval<Tile> literal_box(const std::string& text) {
	Tile t;
	if (parse_tile(text, t)) return tile_interval<Tile>(t);
	if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {   // a hex integer
		return tile_interval<Tile>(Tile(std::stoull(text, nullptr, 16)));
	}
	return bracket<Tile>(std::strtod(text.c_str(), nullptr));
}

// the box of a named constant: qd holds ~212 bits, so the constant lies strictly between
// q - |q| 2^-180 and q + |q| 2^-180, and the tiles of those two decimals enclose it
template<typename Tile>
tile_interval<Tile> constant_box(const std::string& name) {
	using I = tile_interval<Tile>;
	using T = tile_traits<Tile>;
	const sw::universal::qd q = HighPrecisionConstants::lookup(name);
	if (q.isnan()) return I::nan();
	const sw::universal::qd margin = sw::universal::abs(q) * std::ldexp(1.0, -180);
	auto key_of = [](const sw::universal::qd& x) {
		std::ostringstream ss;
		ss << std::setprecision(70) << std::scientific << x;
		Tile t;
		if (!parse_tile(ss.str(), t)) t = Tile(double(x));
		return std::clamp(T::key(t), -T::kmax, T::kmax);
	};
	return I::from_keys(key_of(q - margin), key_of(q + margin));
}

// a box narrowed to one tile, for the single-tile family: the box itself when it is one
// tile, otherwise its lowest tile with the ubit set (inexact, as the flag semantics say)
template<typename Tile>
Tile single(const tile_interval<Tile>& x) {
	using T = tile_traits<Tile>;
	if (x.isnan()) return T::nan();
	return T::tile(x.lo_key() == x.hi_key() ? x.lo_key() : (x.lo_key() | 1));
}

///////////////////////////////////////////////////////////////////////////
// text

// a lattice point as text, or the infinity on that side
template<typename Tile>
std::string point_text(long double v) {
	std::ostringstream ss;
	ss << std::setprecision(digits<Tile>()) << v;
	return ss.str();
}

// "3" for an exact point, "(0.0999, 0.1001)" for an open tile, "[a, b)" for a run of tiles
template<typename Tile>
std::string box_text(const tile_interval<Tile>& x) {
	if (x.isnan()) return "nan";
	const std::string lo = point_text<Tile>(x.template lower<long double>());
	if (x.isexact()) return lo;
	const std::string hi = point_text<Tile>(x.template upper<long double>());
	return std::string(x.lower_open() ? "(" : "[") + lo + ", " + hi + (x.upper_open() ? ")" : "]");
}

// hi - lo + 1 in unsigned arithmetic: a 64-bit box can hold more tiles than int64 counts
template<typename Tile>
std::uint64_t count(const tile_interval<Tile>& x) {
	return x.isnan() ? 0u : static_cast<std::uint64_t>(x.hi_key()) - static_cast<std::uint64_t>(x.lo_key()) + 1u;
}

// a finite stand-in for the set, for the commands that need one number: the midpoint of
// a bounded box, the finite end of a half-bounded one
template<typename Tile>
double midpoint(const tile_interval<Tile>& x) {
	if (x.isnan()) return std::numeric_limits<double>::quiet_NaN();
	const long double l = x.template lower<long double>(), u = x.template upper<long double>();
	if (std::isinf(l) && std::isinf(u)) return std::numeric_limits<double>::quiet_NaN();
	if (std::isinf(l)) return static_cast<double>(u);
	if (std::isinf(u)) return static_cast<double>(l);
	return static_cast<double>((l + u) / 2);
}

///////////////////////////////////////////////////////////////////////////
// Values

template<typename Tile>
Value make_tile_value(const Tile& t) {
	using sw::universal::to_binary;
	using sw::universal::type_tag;
	using T = tile_traits<Tile>;
	const tile_interval<Tile> box(t);
	Value val;
	val.num = midpoint(box);
	val.native = t;
	val.native_rep = box_text(box);
	val.binary_rep = to_binary(t);
	val.native_enc = val.binary_rep;
	val.type_name = type_tag(t);
	val.tile_kind = 1;
	val.ubit = !T::isnan(t) && (T::key(t) & 1) != 0;
	val.tile_count = count(box);
	val.tile_sign = to_string(box.sign());
	std::ostringstream comp;
	if (T::isnan(t))   comp << "nan";
	else if (val.ubit) comp << "open tile (ubit = 1): inexact; a flag, not an enclosure";
	else               comp << "exact tile (ubit = 0)";
	val.components_rep = comp.str();
	if constexpr (is_areal<Tile>::value) val.color_rep = sw::universal::color_print(t);
	return val;
}

template<typename Tile>
Value make_box_value(const tile_interval<Tile>& x) {
	using sw::universal::to_binary;
	using sw::universal::type_tag;
	Value val;
	val.num = midpoint(x);
	val.native = x;
	val.native_rep = box_text(x);
	val.binary_rep = x.isnan() ? std::string("nan") : to_binary(x.lo_tile()) + " .. " + to_binary(x.hi_tile());
	val.native_enc = val.binary_rep;
	val.type_name = "tile_interval<" + type_tag(Tile{}) + ">";
	val.tile_kind = 2;
	val.ubit = !x.isnan() && !x.isexact();
	val.tile_count = count(x);
	val.tile_sign = to_string(x.sign());
	std::ostringstream comp;
	if (x.isnan()) comp << "nan";
	else comp << "enclosure: lower end " << (x.lower_open() ? "open" : "closed") << ", upper end " << (x.upper_open() ? "open" : "closed");
	val.components_rep = comp.str();
	return val;
}

template<typename Tile>
tile_interval<Tile> box_of(const Value& v) {
	if (v.native.has_value()) {
		if (const auto* p = std::any_cast<tile_interval<Tile>>(&v.native)) return *p;
		if (const auto* t = std::any_cast<Tile>(&v.native)) return tile_interval<Tile>(*t);
	}
	return bracket<Tile>(v.num);
}

template<typename Tile>
Tile tile_of(const Value& v) {
	if (v.native.has_value()) {
		if (const auto* t = std::any_cast<Tile>(&v.native)) return *t;
		if (const auto* p = std::any_cast<tile_interval<Tile>>(&v.native)) return single(*p);
	}
	return single(bracket<Tile>(v.num));
}

// x^n for an integer n: squaring for the even steps, which an interval needs to stay
// non-negative and tight around zero
template<typename X, typename Mul, typename Sq>
X pow_int(X x, long long n, const X& one, Mul mul, Sq sq) {
	const bool invert = n < 0;
	unsigned long long m = static_cast<unsigned long long>(invert ? -n : n);
	X r = one;
	bool first = true;
	while (m != 0) {
		if (m & 1ull) { r = first ? x : mul(r, x); first = false; }
		m >>= 1;
		if (m != 0) x = sq(x);
	}
	return invert ? X(one / r) : r;
}

// the exponent as a small integer, when the box is one exact integer point
template<typename Tile>
bool integer_exponent(const tile_interval<Tile>& y, long long& n) {
	if (!y.isexact()) return false;
	const long double v = y.template lower<long double>();
	if (v != std::floor(v) || std::fabs(v) > 1024.0L) return false;
	n = static_cast<long long>(v);
	return true;
}

// the open tile just above an exact tile; an open tile is already uncertain and stays
template<typename Tile>
std::int64_t above_key(std::int64_t k) {
	return ((k & 1) == 0 && k < tile_traits<Tile>::kmax) ? k + 1 : k;
}

} // namespace tiles

///////////////////////////////////////////////////////////////////////////
// registration

// a single tile, with the library's sticky-flag arithmetic
template<typename Tile>
TypeOps register_tile_type(const std::string& name) {
	using namespace tiles;
	using T = tile_traits<Tile>;
	using I = tile_interval<Tile>;
	TypeOps ops;
	ops.name = name;
	ops.type_tag = sw::universal::type_tag(Tile{});
	ops.max_digits10 = digits<Tile>();
	ops.nbits = static_cast<int>(Tile::nbits);
	ops.family = "tile";

	auto mk = [](const Tile& t) { return make_tile_value(t); };
	auto x  = [](const Value& v) { return tile_of<Tile>(v); };
	// a value obtained through double: the tile that holds it, marked inexact
	auto via_double = [](double d) {
		const Tile t(d);
		if (T::isnan(t)) return make_tile_value(t);
		return make_tile_value(T::tile(above_key<Tile>(std::clamp(T::key(t), -T::kmax, T::kmax))));
	};

	ops.from_double  = [mk](double v) { return mk(Tile(v)); };
	ops.from_literal = [mk](const std::string& text) { return mk(single(literal_box<Tile>(text))); };
	ops.constant     = [mk](const std::string& cname) { return mk(single(constant_box<Tile>(cname))); };
	ops.above        = [mk, x](const Value& a) { const Tile t = x(a); return T::isnan(t) ? mk(t) : mk(T::tile(above_key<Tile>(T::key(t)))); };

	ops.add    = [mk, x](const Value& a, const Value& b) { return mk(x(a) + x(b)); };
	ops.sub    = [mk, x](const Value& a, const Value& b) { return mk(x(a) - x(b)); };
	ops.mul    = [mk, x](const Value& a, const Value& b) { return mk(x(a) * x(b)); };
	ops.div    = [mk, x](const Value& a, const Value& b) { return mk(x(a) / x(b)); };
	ops.negate = [mk, x](const Value& a) { return mk(-x(a)); };

	ops.fn_sqrt = [mk, x](const Value& a) { return mk(sw::universal::tile_sqrt(x(a))); };
	ops.fn_abs  = [mk, x](const Value& a) { const Tile t = x(a); return mk(T::key(t) < 0 ? Tile(-t) : t); };
	ops.fn_log  = [via_double, x](const Value& a) { return via_double(std::log(double(x(a)))); };
	ops.fn_exp  = [via_double, x](const Value& a) { return via_double(std::exp(double(x(a)))); };
	ops.fn_sin  = [via_double, x](const Value& a) { return via_double(std::sin(double(x(a)))); };
	ops.fn_cos  = [via_double, x](const Value& a) { return via_double(std::cos(double(x(a)))); };
	ops.fn_tan  = [via_double, x](const Value& a) { return via_double(std::tan(double(x(a)))); };
	ops.fn_asin = [via_double, x](const Value& a) { return via_double(std::asin(double(x(a)))); };
	ops.fn_acos = [via_double, x](const Value& a) { return via_double(std::acos(double(x(a)))); };
	ops.fn_atan = [via_double, x](const Value& a) { return via_double(std::atan(double(x(a)))); };
	ops.fn_pow  = [mk, x, via_double](const Value& a, const Value& b) {
		long long n = 0;
		if (integer_exponent(I(x(b)), n)) {
			auto mul = [](const Tile& p, const Tile& q) { return Tile(p * q); };
			return mk(pow_int(x(a), n, Tile(1), mul, [mul](const Tile& p) { return mul(p, p); }));
		}
		return via_double(std::pow(double(x(a)), double(x(b))));
	};

	ops.maxpos  = [mk]() { return mk(std::numeric_limits<Tile>::max()); };
	ops.minpos  = [mk]() { return mk(std::numeric_limits<Tile>::min()); };
	ops.maxneg  = [mk]() { return mk(std::numeric_limits<Tile>::lowest()); };
	ops.minneg  = [mk]() { return mk(Tile(-std::numeric_limits<Tile>::min())); };
	ops.epsilon = [mk]() { return mk(std::numeric_limits<Tile>::epsilon()); };
	ops.next    = [mk, x](const Value& a) { Tile t = x(a); return mk(++t); };
	ops.prev    = [mk, x](const Value& a) { Tile t = x(a); return mk(--t); };
	return ops;
}

// a tile_interval: a guaranteed enclosure
template<typename Tile>
TypeOps register_tile_interval_type(const std::string& name) {
	using namespace tiles;
	using T = tile_traits<Tile>;
	using I = tile_interval<Tile>;
	TypeOps ops;
	ops.name = name;
	ops.type_tag = "tile_interval<" + sw::universal::type_tag(Tile{}) + ">";
	ops.max_digits10 = digits<Tile>();
	ops.nbits = 2 * static_cast<int>(Tile::nbits);   // a pair of tiles
	ops.family = "tile interval";

	auto mk = [](const I& v) { return make_box_value(v); };
	auto x  = [](const Value& v) { return box_of<Tile>(v); };
	// no enclosing implementation: the whole line, which contains any result
	auto entire = [mk](const Value& a) { const I v = box_of<Tile>(a); return mk(v.isnan() ? v : I::entire()); };

	ops.from_double  = [mk](double v) { return mk(I(Tile(v))); };
	ops.from_literal = [mk](const std::string& text) { return mk(literal_box<Tile>(text)); };
	ops.constant     = [mk](const std::string& cname) { return mk(constant_box<Tile>(cname)); };
	ops.hull         = [mk, x](const Value& a, const Value& b) { return mk(sw::universal::hull(x(a), x(b))); };
	ops.above        = [mk, x](const Value& a) {
		const I v = x(a);
		if (!v.isexact()) return mk(v);
		const std::int64_t k = above_key<Tile>(v.lo_key());
		return mk(I::from_keys(k, k));
	};

	ops.add    = [mk, x](const Value& a, const Value& b) { return mk(x(a) + x(b)); };
	ops.sub    = [mk, x](const Value& a, const Value& b) { return mk(x(a) - x(b)); };
	ops.mul    = [mk, x](const Value& a, const Value& b) { return mk(x(a) * x(b)); };
	ops.div    = [mk, x](const Value& a, const Value& b) { return mk(x(a) / x(b)); };
	ops.negate = [mk, x](const Value& a) { return mk(-x(a)); };

	ops.fn_sqrt = [mk, x](const Value& a) { return mk(sw::universal::sqrt(x(a))); };
	ops.fn_abs  = [mk, x](const Value& a) {
		const I v = x(a);
		if (v.isnan() || v.lo_key() >= 0) return mk(v);
		if (v.hi_key() <= 0) return mk(-v);
		return mk(I::from_keys(0, std::max(-v.lo_key(), v.hi_key())));
	};
	ops.fn_cos  = [mk, x](const Value& a) { return mk(sw::universal::cos(x(a))); };
	ops.fn_sin  = [mk, x](const Value& a) {
		const I v = x(a);
		return mk(v.isnan() ? v : I(Tile(-1), Tile(1)));
	};
	ops.fn_log  = entire;
	ops.fn_exp  = entire;
	ops.fn_tan  = entire;
	ops.fn_asin = entire;
	ops.fn_acos = entire;
	ops.fn_atan = entire;
	ops.fn_pow  = [mk, x, entire](const Value& a, const Value& b) {
		long long n = 0;
		if (integer_exponent(x(b), n)) {
			auto mul = [](const I& p, const I& q) { return I(p * q); };
			auto sq  = [](const I& p) { return sw::universal::square(p); };
			return mk(pow_int(x(a), n, I(1), mul, sq));
		}
		return entire(a);
	};

	auto point = [mk](const Tile& t) { return mk(I(t)); };
	ops.maxpos  = [point]() { return point(std::numeric_limits<Tile>::max()); };
	ops.minpos  = [point]() { return point(std::numeric_limits<Tile>::min()); };
	ops.maxneg  = [point]() { return point(std::numeric_limits<Tile>::lowest()); };
	ops.minneg  = [point]() { return point(Tile(-std::numeric_limits<Tile>::min())); };
	ops.epsilon = [point]() { return point(std::numeric_limits<Tile>::epsilon()); };
	// the tile just past either end
	ops.next    = [mk, x](const Value& a) { const I v = x(a); const std::int64_t k = std::min(v.hi_key() + 1, T::kmax); return mk(I::from_keys(k, k)); };
	ops.prev    = [mk, x](const Value& a) { const I v = x(a); const std::int64_t k = std::max(v.lo_key() - 1, -T::kmax); return mk(I::from_keys(k, k)); };
	return ops;
}

}} // namespace sw::ucalc
