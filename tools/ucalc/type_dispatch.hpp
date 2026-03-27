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
#include <any>
#include <functional>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdexcept>
#include <type_traits>

namespace sw { namespace ucalc {

// Value: type-erased arithmetic value
// Stores both a double approximation (for cross-type operations and display
// fallback) and a std::any holding the native type (for full-precision
// native arithmetic within the active type).
struct Value {
	double num;               // double approximation (lossy for >64-bit types)
	std::any native;          // native type value (lossless, for same-type arithmetic)
	std::string native_rep;   // native operator<< output (lossless for all types)
	std::string binary_rep;   // to_binary() output
	std::string color_rep;    // color_print() output (ANSI-colored bit fields)
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

// Extract native type T from a Value, falling back to double conversion
template<typename T>
T extract(const Value& v) {
	if (v.native.has_value()) {
		const T* p = std::any_cast<T>(&v.native);
		if (p) return *p;
	}
	return T(v.num);
}

// TypeOps: interface for type-specific operations
struct TypeOps {
	std::string name;
	std::string type_tag;
	int max_digits10;       // native precision: std::numeric_limits<T>::max_digits10
	int nbits;              // total bit width of the type

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
	std::function<Value(const Value&)>       fn_tan;
	std::function<Value(const Value&)>       fn_asin;
	std::function<Value(const Value&)>       fn_acos;
	std::function<Value(const Value&)>       fn_atan;
	std::function<Value(const Value&, const Value&)> fn_pow;

	// High-precision constant lookup: returns a Value for a named constant
	// using the type's own high-precision definition when available
	std::function<Value(const std::string&)> constant;

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

template<typename T, typename = void>
struct has_color_print : std::false_type {};
template<typename T>
struct has_color_print<T, std::void_t<decltype(color_print(std::declval<const T&>()))>> : std::true_type {};

// Detect T::nbits member for Universal types
template<typename T, typename = void>
struct has_nbits : std::false_type {};
template<typename T>
struct has_nbits<T, std::void_t<decltype(T::nbits)>> : std::true_type {};

template<typename T>
constexpr int get_nbits() {
	if constexpr (has_nbits<T>::value) { return static_cast<int>(T::nbits); }
	else { return static_cast<int>(sizeof(T) * 8); }
}

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
UCALC_DETECT_MATH_FN(tan)
UCALC_DETECT_MATH_FN(asin)
UCALC_DETECT_MATH_FN(acos)
UCALC_DETECT_MATH_FN(atan)

template<typename T, typename = void>
struct has_pow : std::false_type {};
template<typename T>
struct has_pow<T, std::void_t<decltype(pow(std::declval<T>(), std::declval<T>()))>> : std::true_type {};

#undef UCALC_DETECT_MATH_FN

// make_value: create a Value from a Universal type instance
// Uses qualified calls to sw::universal:: to handle types whose numeric_limits
// return native types (e.g., integer<8>::denorm_min() returns float)
template<typename T>
Value make_value(const T& v) {
	using sw::universal::to_binary;
	using sw::universal::type_tag;
	std::ostringstream nat_ss, bin_ss, comp_ss;
	constexpr int prec = std::numeric_limits<T>::max_digits10 > 0
	                   ? std::numeric_limits<T>::max_digits10 : 17;
	nat_ss << std::setprecision(prec) << v;
	bin_ss << to_binary(v);
	if constexpr (has_components<T>::value) {
		using sw::universal::components;
		comp_ss << components(v);
	} else {
		comp_ss << type_tag(v) << ": " << double(v);
	}
	Value val(double(v), nat_ss.str(), bin_ss.str(), comp_ss.str(), type_tag(v));
	val.native = v;  // store the native type for full-precision arithmetic
	if constexpr (has_color_print<T>::value) {
		using sw::universal::color_print;
		val.color_rep = color_print(v);
	}
	return val;
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
T math_tan(const T& x) {
	if constexpr (has_tan<T>::value) { using sw::universal::tan; return tan(x); }
	else { return T(std::tan(double(x))); }
}
template<typename T>
T math_asin(const T& x) {
	if constexpr (has_asin<T>::value) { using sw::universal::asin; return asin(x); }
	else { return T(std::asin(double(x))); }
}
template<typename T>
T math_acos(const T& x) {
	if constexpr (has_acos<T>::value) { using sw::universal::acos; return acos(x); }
	else { return T(std::acos(double(x))); }
}
template<typename T>
T math_atan(const T& x) {
	if constexpr (has_atan<T>::value) { using sw::universal::atan; return atan(x); }
	else { return T(std::atan(double(x))); }
}
template<typename T>
T math_pow(const T& x, const T& y) {
	if constexpr (has_pow<T>::value) { using sw::universal::pow; return pow(x, y); }
	else { return T(std::pow(double(x), double(y))); }
}

// High-precision constant table: qd values for mathematical constants
// Used by the constant() callback to provide full precision for all types
struct HighPrecisionConstants {
	// Returns a qd value for the named constant, or a signaling NaN if unknown
	static sw::universal::qd lookup(const std::string& name) {
		using namespace sw::universal;
		if (name == "pi")  return qd_pi;
		if (name == "e")   return qd_e;
		if (name == "phi") return qd_phi;
		if (name == "ln2") return qd_ln2;
		if (name == "ln10") return qd_ln10;
		if (name == "sqrt2") return qd_sqrt2;
		if (name == "sqrt3") return qd_sqrt3;
		if (name == "sqrt5") return qd_sqrt5;
		return qd(std::numeric_limits<double>::quiet_NaN());
	}
};

// Default constant handler: convert qd constant to T via the highest
// available precision path. Multi-limb types get their full limbs;
// others convert qd -> double -> T.
template<typename T>
Value constant_via_qd(const std::string& name) {
	using namespace sw::universal;
	qd val = HighPrecisionConstants::lookup(name);
	if constexpr (std::is_same_v<T, qd>) {
		return make_value(val);
	}
	else if constexpr (std::is_same_v<T, qd_cascade>) {
		return make_value(qd_cascade(val[0], val[1], val[2], val[3]));
	}
	else if constexpr (std::is_same_v<T, dd>) {
		return make_value(dd(val[0], val[1]));
	}
	else if constexpr (std::is_same_v<T, dd_cascade>) {
		return make_value(dd_cascade(val[0], val[1]));
	}
	else if constexpr (std::is_same_v<T, td_cascade>) {
		return make_value(td_cascade(val[0], val[1], val[2]));
	}
	else {
		// For all other types: convert qd -> double -> T
		// This gives double precision (~16 digits) which is the best
		// we can do without type-specific string parsing
		T x(static_cast<double>(val));
		return make_value(x);
	}
}

// register_type: create a TypeOps for a specific Universal type T
template<typename T>
TypeOps register_type(const std::string& name) {
	using sw::universal::type_tag;
	TypeOps ops;
	ops.name = name;
	ops.type_tag = type_tag(T{});
	ops.max_digits10 = std::numeric_limits<T>::max_digits10;
	ops.nbits = get_nbits<T>();

	ops.from_double = [](double v) -> Value {
		T x(v);
		return make_value(x);
	};

	ops.constant = [](const std::string& cname) -> Value {
		return constant_via_qd<T>(cname);
	};

	ops.add = [](const Value& a, const Value& b) -> Value {
		return make_value(T(extract<T>(a) + extract<T>(b)));
	};
	ops.sub = [](const Value& a, const Value& b) -> Value {
		return make_value(T(extract<T>(a) - extract<T>(b)));
	};
	ops.mul = [](const Value& a, const Value& b) -> Value {
		return make_value(T(extract<T>(a) * extract<T>(b)));
	};
	ops.div = [](const Value& a, const Value& b) -> Value {
		return make_value(T(extract<T>(a) / extract<T>(b)));
	};
	ops.negate = [](const Value& a) -> Value {
		return make_value(T(-extract<T>(a)));
	};

	ops.fn_sqrt = [](const Value& a) -> Value { return make_value(math_sqrt(extract<T>(a))); };
	ops.fn_abs  = [](const Value& a) -> Value { return make_value(math_abs(extract<T>(a))); };
	ops.fn_log  = [](const Value& a) -> Value { return make_value(math_log(extract<T>(a))); };
	ops.fn_exp  = [](const Value& a) -> Value { return make_value(math_exp(extract<T>(a))); };
	ops.fn_sin  = [](const Value& a) -> Value { return make_value(math_sin(extract<T>(a))); };
	ops.fn_cos  = [](const Value& a) -> Value { return make_value(math_cos(extract<T>(a))); };
	ops.fn_tan  = [](const Value& a) -> Value { return make_value(math_tan(extract<T>(a))); };
	ops.fn_asin = [](const Value& a) -> Value { return make_value(math_asin(extract<T>(a))); };
	ops.fn_acos = [](const Value& a) -> Value { return make_value(math_acos(extract<T>(a))); };
	ops.fn_atan = [](const Value& a) -> Value { return make_value(math_atan(extract<T>(a))); };
	ops.fn_pow  = [](const Value& a, const Value& b) -> Value {
		return make_value(math_pow(extract<T>(a), extract<T>(b)));
	};

	// Type properties via numeric_limits
	// Use denorm_min() when the type supports subnormals, otherwise min()
	ops.maxpos  = []() -> Value { return make_value(std::numeric_limits<T>::max()); };
	ops.minpos  = []() -> Value {
		if constexpr (std::numeric_limits<T>::has_denorm == std::denorm_present) {
			return make_value(std::numeric_limits<T>::denorm_min());
		} else {
			return make_value(std::numeric_limits<T>::min());
		}
	};
	ops.maxneg  = []() -> Value { return make_value(std::numeric_limits<T>::lowest()); };
	ops.minneg  = []() -> Value {
		if constexpr (std::numeric_limits<T>::has_denorm == std::denorm_present) {
			return make_value(T(-std::numeric_limits<T>::denorm_min()));
		} else {
			return make_value(T(-std::numeric_limits<T>::min()));
		}
	};
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
