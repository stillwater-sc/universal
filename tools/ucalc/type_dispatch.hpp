#pragma once
// type_dispatch.hpp: runtime type dispatch registry for ucalc REPL calculator
//
// Maps string type names to pre-instantiated Universal number types.
// Uses type-erased Value wrapper with double as interchange format.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <type_traits>

namespace sw { namespace ucalc {

// Value: type-erased arithmetic value using double as interchange
struct Value {
	double num;               // the numeric value (double interchange, lossy for >64-bit types)
	std::string native_rep;   // native operator<< output (lossless for all types)
	std::string binary_rep;   // to_binary() output
	std::string components_rep; // components() output
	std::string type_name;    // type_tag() output

	Value() : num(0.0) {}
	explicit Value(double v) : num(v) {}
	// 4-arg: native float/double (native_rep auto-generated from num)
	Value(double v, const std::string& bin, const std::string& comp, const std::string& tag)
		: num(v), binary_rep(bin), components_rep(comp), type_name(tag) {
		std::ostringstream ss;
		ss << std::setprecision(17) << v;
		native_rep = ss.str();
	}
	// 5-arg: full (native_rep provided by type's operator<<)
	Value(double v, const std::string& nat, const std::string& bin,
	      const std::string& comp, const std::string& tag)
		: num(v), native_rep(nat), binary_rep(bin), components_rep(comp), type_name(tag) {}
};

// TypeOps: interface for type-specific operations
struct TypeOps {
	std::string name;
	std::string type_tag;

	std::function<Value(double)>             from_double;
	std::function<Value(const Value&, const Value&)> add;
	std::function<Value(const Value&, const Value&)> sub;
	std::function<Value(const Value&, const Value&)> mul;
	std::function<Value(const Value&, const Value&)> div;
	std::function<Value(const Value&)>       negate;

	std::function<Value(const Value&)>       fn_sqrt;
	std::function<Value(const Value&)>       fn_abs;
	std::function<Value(const Value&)>       fn_log;
	std::function<Value(const Value&)>       fn_exp;
	std::function<Value(const Value&)>       fn_sin;
	std::function<Value(const Value&)>       fn_cos;
	std::function<Value(const Value&, const Value&)> fn_pow;

	// Type properties for range/precision display
	std::function<Value()>                  maxpos;    // largest positive
	std::function<Value()>                  minpos;    // smallest positive (denorm_min)
	std::function<Value()>                  maxneg;    // most negative (lowest)
	std::function<Value()>                  minneg;    // largest negative (closest to zero)
	std::function<Value()>                  epsilon;   // machine epsilon
};

// SFINAE helpers for detecting available free functions
template<typename T, typename = void>
struct has_components : std::false_type {};
template<typename T>
struct has_components<T, std::void_t<decltype(components(std::declval<const T&>()))>> : std::true_type {};

// Detect math function availability via ADL
#define UCALC_DETECT_MATH_FN(fn_name) \
	template<typename T, typename = void> \
	struct has_##fn_name : std::false_type {}; \
	template<typename T> \
	struct has_##fn_name<T, std::void_t<decltype(fn_name(std::declval<T>()))>> : std::true_type {};

UCALC_DETECT_MATH_FN(sqrt)
UCALC_DETECT_MATH_FN(abs)
UCALC_DETECT_MATH_FN(log)
UCALC_DETECT_MATH_FN(exp)
UCALC_DETECT_MATH_FN(sin)
UCALC_DETECT_MATH_FN(cos)

template<typename T, typename = void>
struct has_pow : std::false_type {};
template<typename T>
struct has_pow<T, std::void_t<decltype(pow(std::declval<T>(), std::declval<T>()))>> : std::true_type {};

#undef UCALC_DETECT_MATH_FN

// make_value: create a Value from a Universal type instance
// Uses qualified calls to sw::universal:: to handle types whose numeric_limits
// return native types (e.g., integer<8>::denorm_min() returns float)
// SFINAE: detect if T is a cfloat (has ::fbits and ::EXP_BIAS members)
template<typename T, typename = void>
struct is_wide_cfloat : std::false_type {};
template<typename T>
struct is_wide_cfloat<T, std::void_t<decltype(T::fbits), decltype(T::EXP_BIAS)>>
	: std::bool_constant<(T::nbits > 64)> {};

template<typename T>
Value make_value(const T& v) {
	using sw::universal::to_binary;
	using sw::universal::type_tag;
	std::ostringstream nat_ss, bin_ss, comp_ss;
	if constexpr (is_wide_cfloat<T>::value) {
		// For wide cfloats (>64 bits), use fixed format which invokes
		// to_decimal_fixpnt_string instead of going through double
		nat_ss << std::setprecision(40) << std::fixed << v;
	} else {
		nat_ss << std::setprecision(17) << v;
	}
	bin_ss << to_binary(v);
	if constexpr (has_components<T>::value) {
		using sw::universal::components;
		comp_ss << components(v);
	} else {
		comp_ss << type_tag(v) << ": " << double(v);
	}
	return Value(double(v), nat_ss.str(), bin_ss.str(), comp_ss.str(), type_tag(v));
}

// Math function wrappers: use type's own function if available, else fall back to std:: via double
template<typename T>
T math_sqrt(const T& x) {
	if constexpr (has_sqrt<T>::value) { using sw::universal::sqrt; return sqrt(x); }
	else { return T(std::sqrt(double(x))); }
}
template<typename T>
T math_abs(const T& x) {
	if constexpr (has_abs<T>::value) { using sw::universal::abs; return abs(x); }
	else { return T(std::abs(double(x))); }
}
template<typename T>
T math_log(const T& x) {
	if constexpr (has_log<T>::value) { using sw::universal::log; return log(x); }
	else { return T(std::log(double(x))); }
}
template<typename T>
T math_exp(const T& x) {
	if constexpr (has_exp<T>::value) { using sw::universal::exp; return exp(x); }
	else { return T(std::exp(double(x))); }
}
template<typename T>
T math_sin(const T& x) {
	if constexpr (has_sin<T>::value) { using sw::universal::sin; return sin(x); }
	else { return T(std::sin(double(x))); }
}
template<typename T>
T math_cos(const T& x) {
	if constexpr (has_cos<T>::value) { using sw::universal::cos; return cos(x); }
	else { return T(std::cos(double(x))); }
}
template<typename T>
T math_pow(const T& x, const T& y) {
	if constexpr (has_pow<T>::value) { using sw::universal::pow; return pow(x, y); }
	else { return T(std::pow(double(x), double(y))); }
}

// register_type: create a TypeOps for a specific Universal type T
template<typename T>
TypeOps register_type(const std::string& name) {
	TypeOps ops;
	ops.name = name;
	ops.type_tag = type_tag(T{});

	ops.from_double = [](double v) -> Value {
		T x(v);
		return make_value(x);
	};

	ops.add = [](const Value& a, const Value& b) -> Value {
		T xa(a.num), xb(b.num);
		return make_value(T(xa + xb));
	};
	ops.sub = [](const Value& a, const Value& b) -> Value {
		T xa(a.num), xb(b.num);
		return make_value(T(xa - xb));
	};
	ops.mul = [](const Value& a, const Value& b) -> Value {
		T xa(a.num), xb(b.num);
		return make_value(T(xa * xb));
	};
	ops.div = [](const Value& a, const Value& b) -> Value {
		T xa(a.num), xb(b.num);
		return make_value(T(xa / xb));
	};
	ops.negate = [](const Value& a) -> Value {
		T xa(a.num);
		return make_value(T(-xa));
	};

	ops.fn_sqrt = [](const Value& a) -> Value { T xa(a.num); return make_value(math_sqrt(xa)); };
	ops.fn_abs  = [](const Value& a) -> Value { T xa(a.num); return make_value(math_abs(xa)); };
	ops.fn_log  = [](const Value& a) -> Value { T xa(a.num); return make_value(math_log(xa)); };
	ops.fn_exp  = [](const Value& a) -> Value { T xa(a.num); return make_value(math_exp(xa)); };
	ops.fn_sin  = [](const Value& a) -> Value { T xa(a.num); return make_value(math_sin(xa)); };
	ops.fn_cos  = [](const Value& a) -> Value { T xa(a.num); return make_value(math_cos(xa)); };
	ops.fn_pow  = [](const Value& a, const Value& b) -> Value {
		T xa(a.num), xb(b.num);
		return make_value(math_pow(xa, xb));
	};

	// Type properties via numeric_limits
	ops.maxpos  = []() -> Value { return make_value(std::numeric_limits<T>::max()); };
	ops.minpos  = []() -> Value { return make_value(std::numeric_limits<T>::denorm_min()); };
	ops.maxneg  = []() -> Value { return make_value(std::numeric_limits<T>::lowest()); };
	ops.minneg  = []() -> Value { return make_value(T(-std::numeric_limits<T>::denorm_min())); };
	ops.epsilon = []() -> Value { return make_value(std::numeric_limits<T>::epsilon()); };

	return ops;
}

// TypeRegistry: maps string aliases to TypeOps
class TypeRegistry {
public:
	void add(const std::string& alias, TypeOps ops) {
		aliases_.push_back(alias);
		registry_[alias] = std::move(ops);
	}

	const TypeOps* find(const std::string& name) const {
		auto it = registry_.find(name);
		if (it == registry_.end()) return nullptr;
		return &it->second;
	}

	const std::vector<std::string>& aliases() const { return aliases_; }

	const TypeOps& get(const std::string& name) const {
		auto it = registry_.find(name);
		if (it == registry_.end()) {
			throw std::runtime_error("unknown type: " + name);
		}
		return it->second;
	}

private:
	std::vector<std::string> aliases_;  // insertion order
	std::map<std::string, TypeOps> registry_;
};

}} // namespace sw::ucalc
