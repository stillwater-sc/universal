#pragma once
// registry.hpp: default type registry for ucalc
//
// Shared between ucalc.cpp (the REPL) and regression.cpp (tests).
// Contains native float/double specializations of register_type<T>
// and the build_default_registry() function.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
//
// IMPORTANT: this header must be included AFTER all number system
// headers and AFTER type_dispatch.hpp / expression.hpp, since it
// references types and functions defined there.

#include <cmath>
#include <limits>
#include <sstream>

namespace sw { namespace ucalc {

// --- Native float helpers -----------------------------------------

inline Value make_float_value(float f) {
	using namespace sw::universal;
	std::ostringstream nat_ss, bin_ss, comp_ss;
	nat_ss << std::setprecision(std::numeric_limits<float>::max_digits10) << f;
	bin_ss << to_binary(f);
	comp_ss << components(f);
	Value val(double(f), nat_ss.str(), bin_ss.str(), comp_ss.str(), "float (IEEE-754 binary32)");
	val.color_rep = color_print(f);
	return val;
}

template<>
inline TypeOps register_type<float>(const std::string& name) {
	TypeOps ops;
	ops.name = name;
	ops.type_tag = "float (IEEE-754 binary32)";
	ops.max_digits10 = std::numeric_limits<float>::max_digits10;
	ops.nbits = 32;

	ops.from_double = [](double v) -> Value { return make_float_value(static_cast<float>(v)); };
	ops.constant = [](const std::string& cname) -> Value {
		return make_float_value(static_cast<float>(HighPrecisionConstants::lookup(cname)));
	};
	ops.add    = [](const Value& a, const Value& b) -> Value { return make_float_value(float(a.num) + float(b.num)); };
	ops.sub    = [](const Value& a, const Value& b) -> Value { return make_float_value(float(a.num) - float(b.num)); };
	ops.mul    = [](const Value& a, const Value& b) -> Value { return make_float_value(float(a.num) * float(b.num)); };
	ops.div    = [](const Value& a, const Value& b) -> Value { return make_float_value(float(a.num) / float(b.num)); };
	ops.negate = [](const Value& a) -> Value { return make_float_value(-float(a.num)); };
	ops.fn_sqrt = [](const Value& a) -> Value { return make_float_value(std::sqrt(float(a.num))); };
	ops.fn_abs  = [](const Value& a) -> Value { return make_float_value(std::abs(float(a.num))); };
	ops.fn_log  = [](const Value& a) -> Value { return make_float_value(std::log(float(a.num))); };
	ops.fn_exp  = [](const Value& a) -> Value { return make_float_value(std::exp(float(a.num))); };
	ops.fn_sin  = [](const Value& a) -> Value { return make_float_value(std::sin(float(a.num))); };
	ops.fn_cos  = [](const Value& a) -> Value { return make_float_value(std::cos(float(a.num))); };
	ops.fn_pow  = [](const Value& a, const Value& b) -> Value { return make_float_value(std::pow(float(a.num), float(b.num))); };
	ops.maxpos  = []() -> Value { return make_float_value(std::numeric_limits<float>::max()); };
	ops.minpos  = []() -> Value { return make_float_value(std::numeric_limits<float>::denorm_min()); };
	ops.maxneg  = []() -> Value { return make_float_value(std::numeric_limits<float>::lowest()); };
	ops.minneg  = []() -> Value { return make_float_value(-std::numeric_limits<float>::denorm_min()); };
	ops.epsilon = []() -> Value { return make_float_value(std::numeric_limits<float>::epsilon()); };
	return ops;
}

// --- Native double helpers ----------------------------------------

inline Value make_double_value(double d) {
	using namespace sw::universal;
	std::ostringstream nat_ss, bin_ss, comp_ss;
	nat_ss << std::setprecision(std::numeric_limits<double>::max_digits10) << d;
	bin_ss << to_binary(d);
	comp_ss << components(d);
	Value val(double(d), nat_ss.str(), bin_ss.str(), comp_ss.str(), "double (IEEE-754 binary64)");
	val.color_rep = color_print(d);
	return val;
}

template<>
inline TypeOps register_type<double>(const std::string& name) {
	TypeOps ops;
	ops.name = name;
	ops.type_tag = "double (IEEE-754 binary64)";
	ops.max_digits10 = std::numeric_limits<double>::max_digits10;
	ops.nbits = 64;

	ops.from_double = [](double v) -> Value { return make_double_value(v); };
	ops.constant = [](const std::string& cname) -> Value {
		return make_double_value(static_cast<double>(HighPrecisionConstants::lookup(cname)));
	};
	ops.add    = [](const Value& a, const Value& b) -> Value { return make_double_value(a.num + b.num); };
	ops.sub    = [](const Value& a, const Value& b) -> Value { return make_double_value(a.num - b.num); };
	ops.mul    = [](const Value& a, const Value& b) -> Value { return make_double_value(a.num * b.num); };
	ops.div    = [](const Value& a, const Value& b) -> Value { return make_double_value(a.num / b.num); };
	ops.negate = [](const Value& a) -> Value { return make_double_value(-a.num); };
	ops.fn_sqrt = [](const Value& a) -> Value { return make_double_value(std::sqrt(a.num)); };
	ops.fn_abs  = [](const Value& a) -> Value { return make_double_value(std::abs(a.num)); };
	ops.fn_log  = [](const Value& a) -> Value { return make_double_value(std::log(a.num)); };
	ops.fn_exp  = [](const Value& a) -> Value { return make_double_value(std::exp(a.num)); };
	ops.fn_sin  = [](const Value& a) -> Value { return make_double_value(std::sin(a.num)); };
	ops.fn_cos  = [](const Value& a) -> Value { return make_double_value(std::cos(a.num)); };
	ops.fn_pow  = [](const Value& a, const Value& b) -> Value { return make_double_value(std::pow(a.num, b.num)); };
	ops.maxpos  = []() -> Value { return make_double_value(std::numeric_limits<double>::max()); };
	ops.minpos  = []() -> Value { return make_double_value(std::numeric_limits<double>::denorm_min()); };
	ops.maxneg  = []() -> Value { return make_double_value(std::numeric_limits<double>::lowest()); };
	ops.minneg  = []() -> Value { return make_double_value(-std::numeric_limits<double>::denorm_min()); };
	ops.epsilon = []() -> Value { return make_double_value(std::numeric_limits<double>::epsilon()); };
	return ops;
}

// --- Default registry ---------------------------------------------

inline TypeRegistry build_default_registry() {
	using namespace sw::universal;

	TypeRegistry reg;

	// Native IEEE types
	reg.add("float",    register_type<float>("float"));
	reg.add("double",   register_type<double>("double"));

	// Posit Standard defines es=2 for all standard sizes
	reg.add("posit8",   register_type<posit<8, 2, uint8_t>>("posit8"));
	reg.add("posit16",  register_type<posit<16, 2, uint16_t>>("posit16"));
	reg.add("posit32",  register_type<posit<32, 2, uint32_t>>("posit32"));
	reg.add("posit64",  register_type<posit<64, 2, uint64_t>>("posit64"));

	// Google Brain float
	reg.add("bfloat16", register_type<bfloat16>("bfloat16"));

	// Classic floating-point types (IEEE-754)
	reg.add("fp16",     register_type<fp16>("fp16"));
	reg.add("fp32",     register_type<fp32>("fp32"));
	reg.add("fp64",     register_type<fp64>("fp64"));
	reg.add("fp128",    register_type<fp128>("fp128"));

	// FP8 formats for Deep Learning
	reg.add("fp8e2m5",  register_type<fp8e2m5>("fp8e2m5"));
	reg.add("fp8e3m4",  register_type<fp8e3m4>("fp8e3m4"));
	reg.add("fp8e4m3",  register_type<fp8e4m3>("fp8e4m3"));
	reg.add("fp8e5m2",  register_type<fp8e5m2>("fp8e5m2"));

	// Fixed-point types
	reg.add("fixpnt16", register_type<fixpnt<16, 8, Modulo, uint16_t>>("fixpnt16"));
	reg.add("fixpnt32", register_type<fixpnt<32, 16, Modulo, uint32_t>>("fixpnt32"));

	// Logarithmic number system types
	reg.add("lns8",     register_type<lns<8, 2, uint8_t>>("lns8"));
	reg.add("lns16",    register_type<lns<16, 8, uint16_t>>("lns16"));
	reg.add("lns32",    register_type<lns<32, 16, uint32_t>>("lns32"));

	// Integer types
	reg.add("int8",     register_type<integer<8, uint8_t>>("int8"));
	reg.add("int16",    register_type<integer<16, uint16_t>>("int16"));
	reg.add("int32",    register_type<integer<32, uint32_t>>("int32"));
	reg.add("int64",    register_type<integer<64, uint64_t>>("int64"));

	// Note: dbns and takum omitted -- math function stubs cause link errors

	// Hexadecimal floating-point
	reg.add("hfloat32", register_type<hfloat<6, 7>>("hfloat32"));
	reg.add("hfloat64", register_type<hfloat<14, 7>>("hfloat64"));

	// Decimal floating-point
	reg.add("decimal32",  register_type<dfloat<7, 6>>("decimal32"));
	reg.add("decimal64",  register_type<dfloat<16, 8>>("decimal64"));

	// Rational types (exact fractions, base-2 representation)
	reg.add("rational8",  register_type<rational<8, base2, uint8_t>>("rational8"));
	reg.add("rational16", register_type<rational<16, base2, uint16_t>>("rational16"));
	reg.add("rational32", register_type<rational<32, base2, uint32_t>>("rational32"));

	// High-precision floating-point: Dekker and Priest variants
	reg.add("dd",         register_type<dd>("dd"));
	reg.add("dd_cascade", register_type<dd_cascade>("dd_cascade"));
	reg.add("td_cascade", register_type<td_cascade>("td_cascade"));
	reg.add("qd",         register_type<qd>("qd"));
	reg.add("qd_cascade", register_type<qd_cascade>("qd_cascade"));

	return reg;
}

}} // namespace sw::ucalc
