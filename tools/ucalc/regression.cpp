// regression.cpp: regression tests for ucalc REPL calculator
//
// Validates expression evaluation, type switching, commands, and
// display output by running the same code paths as the interactive
// REPL in pipe mode.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <random>

// Suppress exceptions -- regression catches errors via output
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#define BFLOAT16_THROW_ARITHMETIC_EXCEPTION 0
#define DD_THROW_ARITHMETIC_EXCEPTION 0
#define QD_THROW_ARITHMETIC_EXCEPTION 0
#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
#define TAKUM_THROW_ARITHMETIC_EXCEPTION 0
#define DFIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#define RATIONAL_THROW_ARITHMETIC_EXCEPTION 0
#define HFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define DFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define DD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0
#define TD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0
#define QD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0
#define AREAL_THROW_ARITHMETIC_EXCEPTION 0
#define POXEL_THROW_ARITHMETIC_EXCEPTION 0

#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/lns/lns.hpp>
#include <universal/number/dbns/dbns.hpp>
#include <universal/number/integer/integer.hpp>
#include <universal/number/takum/takum.hpp>
#include <universal/number/dfixpnt/dfixpnt.hpp>
#include <universal/number/rational/rational.hpp>
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/number/dfloat/dfloat.hpp>
// cascade types included for HighPrecisionConstants (qd constants)
// but not registered in the test registry
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
// qd_cascade must be included BEFORE type_dispatch.hpp for ADL to find
// its type_tag -- the header order here matches ucalc.cpp
#include <universal/number/qd_cascade/qd_cascade.hpp>

// ubit tile types (areal, poxel) and their tile intervals
#include <universal/number/areal/areal.hpp>
#include <universal/number/poxel/poxel.hpp>

#include "type_dispatch.hpp"
#include "expression.hpp"
#include "registry.hpp"
#include "output_format.hpp"
#include "uncertainty.hpp"

namespace {

using namespace sw::universal;
using namespace sw::ucalc;

int nrOfFailedTests = 0;

// Evaluate an expression in a given type and return the Value
Value eval_in(TypeRegistry& reg, const std::string& type, const std::string& expr) {
	const TypeOps& ops = reg.get(type);
	ExpressionEvaluator evaluator(ops);
	return evaluator.evaluate(expr);
}

// Check that an expression evaluates to the expected double value (within tolerance)
void check_value(TypeRegistry& reg, const std::string& type,
                 const std::string& expr, double expected, double tol,
                 const std::string& label) {
	try {
		Value result = eval_in(reg, type, expr);
		bool ok = false;
		if (std::isnan(expected)) {
			ok = std::isnan(result.num);
		} else if (std::isinf(expected)) {
			ok = std::isinf(result.num) &&
			     (std::signbit(result.num) == std::signbit(expected));
		} else {
			double err = std::abs(result.num - expected);
			ok = (err <= tol);
		}
		if (!ok) {
			std::cerr << "FAIL: " << label << ": " << type << "> " << expr
			          << " = " << result.native_rep << " (expected " << expected << ")\n";
			++nrOfFailedTests;
		}
	} catch (const std::exception& ex) {
		std::cerr << "FAIL: " << label << ": " << type << "> " << expr
		          << " threw: " << ex.what() << "\n";
		++nrOfFailedTests;
	}
}

// Check that native_rep contains an expected substring
void check_contains(TypeRegistry& reg, const std::string& type,
                    const std::string& expr, const std::string& substring,
                    const std::string& label) {
	try {
		Value result = eval_in(reg, type, expr);
		if (result.native_rep.find(substring) == std::string::npos) {
			std::cerr << "FAIL: " << label << ": " << type << "> " << expr
			          << " native_rep=" << result.native_rep
			          << " (expected to contain '" << substring << "')\n";
			++nrOfFailedTests;
		}
	} catch (const std::exception& ex) {
		std::cerr << "FAIL: " << label << ": " << type << "> " << expr
		          << " threw: " << ex.what() << "\n";
		++nrOfFailedTests;
	}
}

// Check that binary_rep is non-empty and starts with expected prefix
void check_binary(TypeRegistry& reg, const std::string& type,
                  const std::string& expr, const std::string& prefix,
                  const std::string& label) {
	try {
		Value result = eval_in(reg, type, expr);
		if (result.binary_rep.substr(0, prefix.size()) != prefix) {
			std::cerr << "FAIL: " << label << ": " << type << "> " << expr
			          << " binary=" << result.binary_rep
			          << " (expected prefix '" << prefix << "')\n";
			++nrOfFailedTests;
		}
	} catch (const std::exception& ex) {
		std::cerr << "FAIL: " << label << ": " << type << "> " << expr
		          << " threw: " << ex.what() << "\n";
		++nrOfFailedTests;
	}
}

// Check that an expression throws (parse error, undefined variable, etc.)
void check_throws(TypeRegistry& reg, const std::string& type,
                  const std::string& expr, const std::string& label) {
	try {
		eval_in(reg, type, expr);
		std::cerr << "FAIL: " << label << ": " << type << "> " << expr
		          << " did not throw\n";
		++nrOfFailedTests;
	} catch (...) {
		// expected
	}
}

// the exact decimal text of a finite double: M * 2^e as M * 5^-e / 10^-e for e < 0
std::string exact_decimal(double v) {
	if (v == 0.0) return "0";
	int e2 = 0;
	const double f = std::frexp(std::fabs(v), &e2);              // f in [0.5, 1)
	std::uint64_t m = static_cast<std::uint64_t>(std::ldexp(f, 53));
	int e = e2 - 53;
	while (m != 0 && (m & 1u) == 0) { m >>= 1; ++e; }
	std::string digits = std::to_string(m);                       // little arithmetic on decimal strings
	auto times = [&digits](unsigned k) {
		unsigned carry = 0;
		for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
			const unsigned d = static_cast<unsigned>(*it - '0') * k + carry;
			*it = static_cast<char>('0' + d % 10u);
			carry = d / 10u;
		}
		while (carry != 0) { digits.insert(digits.begin(), static_cast<char>('0' + carry % 10u)); carry /= 10u; }
	};
	std::string text;
	if (e >= 0) {
		for (int i = 0; i < e; ++i) times(2);
		text = digits;
	}
	else {
		for (int i = 0; i < -e; ++i) times(5);
		const std::size_t point = static_cast<std::size_t>(-e);
		if (digits.size() <= point) digits.insert(0, point - digits.size() + 1, '0');
		text = digits.substr(0, digits.size() - point) + "." + digits.substr(digits.size() - point);
	}
	return (v < 0 ? "-" : "") + text;
}

// every lattice point and every open tile of a small tile type, read back from exact decimal
// text: the point itself, the midpoint of each open tile, and a double just past each point
template<typename Tile>
void check_literal_tiles(const std::string& name) {
	using T = tile_traits<Tile>;
	using I = tile_interval<Tile>;
	int failures = 0;
	auto expect = [&](const std::string& text, std::int64_t k) {
		const I box = tiles::literal_box<Tile>(text);
		if (box.isnan() || box.lo_key() != k || box.hi_key() != k) {
			if (failures++ < 5) std::cerr << "FAIL: " << name << " literal " << text << " -> " << tiles::box_text(box) << ", expected tile key " << k << "\n";
		}
	};
	for (std::int64_t k = -T::kmax + 1; k < T::kmax; ++k) {
		const double lo = I::from_keys(k, k).template lower<double>(), hi = I::from_keys(k, k).template upper<double>();
		if ((k & 1) == 0) {
			expect(exact_decimal(lo), k);
			const double past = std::nextafter(lo, std::numeric_limits<double>::infinity());
			expect(exact_decimal(past), k + 1);
		}
		else {
			expect(exact_decimal((lo + hi) / 2), k);
		}
	}
	nrOfFailedTests += failures;
}

// sampled doubles in wide types: the literal of a double's exact text must give the tile
// that the type's own conversion from double gives
template<typename Tile>
void check_literal_doubles(const std::string& name) {
	using I = tile_interval<Tile>;
	std::mt19937_64 rng(1654);
	std::uniform_real_distribution<double> frac(-1.0, 1.0);
	std::uniform_int_distribution<int> scale(-60, 60);
	int failures = 0;
	for (int i = 0; i < 2000; ++i) {
		const double d = std::ldexp(frac(rng), scale(rng));
		const I box = tiles::literal_box<Tile>(exact_decimal(d));
		const I ref(Tile{ d });
		if (box.isnan() || box.lo_key() != ref.lo_key() || box.hi_key() != ref.hi_key()) {
			if (failures++ < 5) std::cerr << "FAIL: " << name << " literal " << exact_decimal(d) << " -> " << tiles::box_text(box) << ", expected " << tiles::box_text(ref) << "\n";
		}
	}
	nrOfFailedTests += failures;
}

// the number of tiles and the sign verdict of an expression in a tile interval type
void check_box(TypeRegistry& reg, const std::string& type, const std::string& expr,
               std::uint64_t tiles, const std::string& sign, const std::string& label) {
	try {
		Value v = eval_in(reg, type, expr);
		if (v.tile_kind != 2 || v.tile_count != tiles || v.tile_sign != sign) {
			std::cerr << "FAIL: " << label << ": " << type << "> " << expr << " = " << v.native_rep
			          << " (" << v.tile_count << " tiles, sign " << v.tile_sign << "; expected "
			          << tiles << " tiles, sign " << sign << ")\n";
			++nrOfFailedTests;
		}
	} catch (const std::exception& ex) {
		std::cerr << "FAIL: " << label << ": " << type << "> " << expr << " threw: " << ex.what() << "\n";
		++nrOfFailedTests;
	}
}

} // anonymous namespace

int main()
try {
	TypeRegistry reg = build_default_registry();

	// ================================================================
	// 1. Basic arithmetic
	// ================================================================
	check_value(reg, "double", "2 + 3", 5.0, 0.0, "add");
	check_value(reg, "double", "10 - 7", 3.0, 0.0, "sub");
	check_value(reg, "double", "6 * 7", 42.0, 0.0, "mul");
	check_value(reg, "double", "1 / 4", 0.25, 0.0, "div");
	check_value(reg, "double", "2 ^ 10", 1024.0, 0.0, "pow");
	check_value(reg, "double", "-3 + 5", 2.0, 0.0, "unary neg");
	check_value(reg, "double", "(2 + 3) * 4", 20.0, 0.0, "parens");

	// ================================================================
	// 2. Operator precedence
	// ================================================================
	check_value(reg, "double", "2 + 3 * 4", 14.0, 0.0, "precedence mul>add");
	check_value(reg, "double", "10 - 2 * 3", 4.0, 0.0, "precedence mul>sub");
	check_value(reg, "double", "2 * 3 ^ 2", 18.0, 0.0, "precedence pow>mul");
	check_value(reg, "double", "-2 ^ 2", -4.0, 0.0, "unary neg then pow"); // -(2^2), not (-2)^2

	// ================================================================
	// 3. Constants
	// ================================================================
	check_value(reg, "double", "pi", 3.14159265358979323846, 1e-15, "pi");
	check_value(reg, "double", "e", 2.71828182845904523536, 1e-15, "e");
	check_value(reg, "double", "phi", 1.61803398874989484820, 1e-15, "phi");
	check_value(reg, "double", "ln2", 0.69314718055994530942, 1e-15, "ln2");

	// ================================================================
	// 4. Built-in functions
	// ================================================================
	check_value(reg, "double", "sqrt(4)", 2.0, 0.0, "sqrt");
	check_value(reg, "double", "abs(-7)", 7.0, 0.0, "abs");
	check_value(reg, "double", "log(1)", 0.0, 1e-15, "log(1)");
	check_value(reg, "double", "exp(0)", 1.0, 0.0, "exp(0)");
	check_value(reg, "double", "sin(0)", 0.0, 0.0, "sin(0)");
	check_value(reg, "double", "cos(0)", 1.0, 0.0, "cos(0)");
	check_value(reg, "double", "pow(2, 10)", 1024.0, 0.0, "pow(2,10)");

	// ================================================================
	// 5. Variables
	// ================================================================
	{
		const TypeOps& ops = reg.get("double");
		ExpressionEvaluator eval(ops);
		Value v1 = eval.evaluate("x = 7");
		if (std::abs(v1.num - 7.0) > 0.0) {
			std::cerr << "FAIL: variable assignment\n";
			++nrOfFailedTests;
		}
		Value v2 = eval.evaluate("x * x");
		if (std::abs(v2.num - 49.0) > 0.0) {
			std::cerr << "FAIL: variable use\n";
			++nrOfFailedTests;
		}
	}

	// ================================================================
	// 6. Posit closure: 1/3 + 1/3 + 1/3 = 1
	// ================================================================
	check_value(reg, "posit32", "1/3 + 1/3 + 1/3", 1.0, 0.0, "posit closure");

	// ================================================================
	// 7. Type-specific precision (native_rep digit count)
	// ================================================================
	{
		// fp8e4m3 should render ~3 significant digits
		Value v = eval_in(reg, "fp8e4m3", "1/3");
		if (v.native_rep.size() > 10) {
			std::cerr << "FAIL: fp8e4m3 over-rendering: " << v.native_rep << "\n";
			++nrOfFailedTests;
		}
	}
	{
		// fp128 should render ~34 significant digits
		Value v = eval_in(reg, "fp128", "1/3");
		if (v.native_rep.size() < 30) {
			std::cerr << "FAIL: fp128 under-rendering: " << v.native_rep
			          << " (len=" << v.native_rep.size() << ")\n";
			++nrOfFailedTests;
		}
	}
	{
		// qd should render ~64 significant digits
		Value v = eval_in(reg, "qd", "1/3");
		if (v.native_rep.size() < 60) {
			std::cerr << "FAIL: qd under-rendering: " << v.native_rep
			          << " (len=" << v.native_rep.size() << ")\n";
			++nrOfFailedTests;
		}
	}

	// ================================================================
	// 8. Binary representation sanity
	// ================================================================
	check_binary(reg, "double", "1.0", "0b0.", "double binary prefix");
	check_binary(reg, "posit32", "1.0", "0b0.", "posit binary prefix");
	check_binary(reg, "fp32", "1.0", "0b0.", "fp32 binary prefix");
	check_binary(reg, "decimal32", "1.0", "0b0.", "decimal32 binary prefix");

	// ================================================================
	// 9. Components non-empty
	// ================================================================
	{
		Value v = eval_in(reg, "double", "1/3");
		if (v.components_rep.empty()) {
			std::cerr << "FAIL: double components empty\n";
			++nrOfFailedTests;
		}
		if (v.components_rep.find("sign:") == std::string::npos) {
			std::cerr << "FAIL: double components missing 'sign:': "
			          << v.components_rep << "\n";
			++nrOfFailedTests;
		}
	}
	{
		Value v = eval_in(reg, "posit32", "1/3");
		if (v.components_rep.find("sign:") == std::string::npos) {
			std::cerr << "FAIL: posit32 components missing 'sign:': "
			          << v.components_rep << "\n";
			++nrOfFailedTests;
		}
	}
	{
		Value v = eval_in(reg, "fp32", "1/3");
		if (v.components_rep.find("sign:") == std::string::npos) {
			std::cerr << "FAIL: fp32 components missing 'sign:': "
			          << v.components_rep << "\n";
			++nrOfFailedTests;
		}
	}

	// ================================================================
	// 10. Error handling
	// ================================================================
	check_throws(reg, "double", "1 +", "trailing operator");
	check_throws(reg, "double", "foo", "undefined variable");
	check_throws(reg, "double", "bar(1)", "undefined function");

	// ================================================================
	// 11. Special values
	// ================================================================
	check_value(reg, "double", "1/0", std::numeric_limits<double>::infinity(), 0.0, "+inf");
	check_value(reg, "double", "-1/0", -std::numeric_limits<double>::infinity(), 0.0, "-inf");
	check_value(reg, "double", "0 * (1/0)", std::numeric_limits<double>::quiet_NaN(), 0.0, "nan");

	// ================================================================
	// 12. Cross-type evaluation
	// ================================================================
	check_value(reg, "int32", "7 / 2", 3.0, 0.5, "int32 truncation");
	check_value(reg, "fixpnt16", "0.5 + 0.25", 0.75, 0.01, "fixpnt add");
	check_contains(reg, "lns16", "1.0", "1", "lns16 one");
	check_value(reg, "decimal32", "0.1 + 0.2", 0.3, 0.0001, "decimal add");

	// ================================================================
	// 13. JSON escape utility
	// ================================================================
	{
		// Test basic escaping
		auto test_escape = [&](const std::string& input, const std::string& expected,
		                       const std::string& label) {
			std::string result = json_escape(input);
			if (result != expected) {
				std::cerr << "FAIL: json_escape " << label
				          << ": got '" << result << "' expected '" << expected << "'\n";
				++nrOfFailedTests;
			}
		};
		test_escape("hello", "hello", "simple");
		test_escape("a\"b", "a\\\"b", "quote");
		test_escape("a\\b", "a\\\\b", "backslash");
		test_escape("a\nb", "a\\nb", "newline");
		test_escape("a\tb", "a\\tb", "tab");
	}

	// ================================================================
	// 14. CSV quote utility
	// ================================================================
	{
		auto test_csv = [&](const std::string& input, const std::string& expected,
		                    const std::string& label) {
			std::string result = csv_quote(input);
			if (result != expected) {
				std::cerr << "FAIL: csv_quote " << label
				          << ": got '" << result << "' expected '" << expected << "'\n";
				++nrOfFailedTests;
			}
		};
		test_csv("hello", "hello", "simple");
		test_csv("a,b", "\"a,b\"", "comma");
		test_csv("a\"b", "\"a\"\"b\"", "quote");
		test_csv("a\nb", "\"a\nb\"", "newline");
	}

	// ================================================================
	// 15. JSON number utility (inf/nan safety)
	// ================================================================
	{
		auto test_jn = [&](double val, const std::string& expected,
		                   const std::string& label) {
			std::string result = json_number(val);
			if (result != expected) {
				std::cerr << "FAIL: json_number " << label
				          << ": got '" << result << "' expected '" << expected << "'\n";
				++nrOfFailedTests;
			}
		};
		test_jn(std::numeric_limits<double>::infinity(), "\"inf\"", "+inf");
		test_jn(-std::numeric_limits<double>::infinity(), "\"-inf\"", "-inf");
		test_jn(std::numeric_limits<double>::quiet_NaN(), "\"nan\"", "nan");
		// Normal numbers should be numeric (not quoted)
		std::string result = json_number(3.14);
		if (result.find('"') != std::string::npos) {
			std::cerr << "FAIL: json_number 3.14 should not be quoted: " << result << "\n";
			++nrOfFailedTests;
		}
		if (result.find("3.14") == std::string::npos) {
			std::cerr << "FAIL: json_number 3.14 should contain '3.14': " << result << "\n";
			++nrOfFailedTests;
		}
	}

	// ================================================================
	// 16. Trace: step recording
	// ================================================================
	{
		const TypeOps& ops = reg.get("double");
		ExpressionEvaluator eval(ops);
		eval.enable_trace(true);
		eval.evaluate("2 + 3 * 4");
		const auto& steps = eval.trace_steps();
		// 2 + 3 * 4 = 2 + 12 = 14
		// Expect 2 steps: mul(3,4)->12, add(2,12)->14
		if (steps.size() != 2) {
			std::cerr << "FAIL: trace step count: got " << steps.size() << " expected 2\n";
			++nrOfFailedTests;
		} else {
			if (steps[0].operation != "mul") {
				std::cerr << "FAIL: trace step 1 op: got " << steps[0].operation << " expected mul\n";
				++nrOfFailedTests;
			}
			if (std::abs(steps[0].result - 12.0) > 1e-15) {
				std::cerr << "FAIL: trace step 1 result: got " << steps[0].result << " expected 12\n";
				++nrOfFailedTests;
			}
			if (steps[1].operation != "add") {
				std::cerr << "FAIL: trace step 2 op: got " << steps[1].operation << " expected add\n";
				++nrOfFailedTests;
			}
			if (std::abs(steps[1].result - 14.0) > 1e-15) {
				std::cerr << "FAIL: trace step 2 result: got " << steps[1].result << " expected 14\n";
				++nrOfFailedTests;
			}
		}
	}

	// ================================================================
	// 17. Trace: function calls recorded
	// ================================================================
	{
		const TypeOps& ops = reg.get("double");
		ExpressionEvaluator eval(ops);
		eval.enable_trace(true);
		eval.evaluate("sqrt(4) + 1");
		const auto& steps = eval.trace_steps();
		// sqrt(4) -> 2, then 2 + 1 -> 3
		if (steps.size() != 2) {
			std::cerr << "FAIL: trace fn step count: got " << steps.size() << " expected 2\n";
			++nrOfFailedTests;
		} else {
			if (steps[0].operation != "sqrt") {
				std::cerr << "FAIL: trace fn step 1: got " << steps[0].operation << " expected sqrt\n";
				++nrOfFailedTests;
			}
			if (steps[1].operation != "add") {
				std::cerr << "FAIL: trace fn step 2: got " << steps[1].operation << " expected add\n";
				++nrOfFailedTests;
			}
		}
	}

	// ================================================================
	// 18. Trace: tracing disabled by default
	// ================================================================
	{
		const TypeOps& ops = reg.get("double");
		ExpressionEvaluator eval(ops);
		eval.evaluate("2 + 3");
		if (!eval.trace_steps().empty()) {
			std::cerr << "FAIL: trace should be empty when not enabled\n";
			++nrOfFailedTests;
		}
	}

	// ================================================================
	// Takum, both variants
	// ================================================================
	// The family had no coverage here at all before takum_log was registered.
	{
		check_value(reg, "takum32",     "1 + 1",     2.0, 0.0,   "takum32 add");
		check_value(reg, "takum_log32", "1 + 1",     2.0, 1e-7,  "takum_log32 add");
		check_value(reg, "takum32",     "sqrt(4)",   2.0, 1e-7,  "takum32 sqrt");
		check_value(reg, "takum_log32", "sqrt(4)",   2.0, 1e-7,  "takum_log32 sqrt");
		check_value(reg, "takum_log32", "2 * 3",     6.0, 1e-7,  "takum_log32 mul");
		check_value(reg, "takum_log32", "1 / 4",     0.25, 1e-7, "takum_log32 div");
		check_value(reg, "takum_log32", "pi",        3.14159265358979324, 1e-7, "takum_log32 pi");
		check_value(reg, "takum_log32", "exp(0)",    1.0, 1e-7,  "takum_log32 exp(0)");
		check_value(reg, "takum_log32", "log(1)",    0.0, 1e-7,  "takum_log32 log(1)");
		check_value(reg, "takum_log16", "1 + 1",     2.0, 1e-3,  "takum_log16 add");
		check_value(reg, "takum_log64", "1 + 1",     2.0, 1e-12, "takum_log64 add");
		check_value(reg, "takum_log8",  "1 + 1",     2.0, 0.2,   "takum_log8 add");

		// e is exactly representable in the logarithmic takum and in no binary
		// floating-point format: the value base is sqrt(e), so e is sqrt(e)^2 and
		// its logarithmic value is the integer 2.
		check_value(reg, "takum_log32", "e", 2.71828182845904524, 1e-9, "takum_log32 e");

		// The two variants share a bit layout but not a value map, so the same
		// number must land on DIFFERENT bits.  That is the reason both are
		// registered, and a registration that silently aliased one to the other
		// would pass every check above.
		Value lin = eval_in(reg, "takum32",     "3.0");
		Value log = eval_in(reg, "takum_log32", "3.0");
		if (lin.binary_rep == log.binary_rep) {
			std::cerr << "FAIL: takum32 and takum_log32 produced identical encodings for 3.0: "
			          << lin.binary_rep << "\n";
			++nrOfFailedTests;
		}
		if (lin.type_name == log.type_name) {
			std::cerr << "FAIL: takum32 and takum_log32 report the same type tag: "
			          << lin.type_name << "\n";
			++nrOfFailedTests;
		}
	}

	// ================================================================
	// Tile types: areal and poxel, single tiles and tile intervals (#1654)
	// ================================================================
	{
		// literals are read from their decimal text, into the tile that contains them
		check_literal_tiles<areal<8, 2, uint8_t>>("areal8");
		check_literal_tiles<areal<16, 5, uint8_t>>("areal16");
		check_literal_tiles<poxel<8, 2, uint8_t>>("poxel8");
		check_literal_tiles<poxel<16, 2, uint8_t>>("poxel16");
		check_literal_doubles<poxel<32, 2, uint8_t>>("poxel32");
		check_literal_doubles<poxel<64, 2, uint8_t>>("poxel64");
		check_literal_doubles<areal<64, 11, uint8_t>>("areal64");

		// one tenth is one open tile even where the lattice is finer than double's
		check_box(reg, "poxel64i", "0.1", 1, "positive", "poxel64i 0.1");
		check_box(reg, "poxel64i", "-0.1", 1, "negative", "poxel64i -0.1");
		check_box(reg, "poxel64i", "pi", 1, "positive", "poxel64i pi");
		check_box(reg, "areal64i", "0.1", 1, "positive", "areal64i 0.1");
		check_contains(reg, "poxel64i", "0x10", "16", "poxel64i hex literal is exact");
		check_box(reg, "poxel64i", "0x10", 1, "positive", "poxel64i hex literal is one tile");
		check_contains(reg, "poxel32i", "1e400", "inf)", "poxel32i 1e400 is (maxpos, inf)");
		check_contains(reg, "poxel32i", "1e-400", "(0, ", "poxel32i 1e-400 is (0, minpos)");

		// interval syntax: [a, b], hull(a, b), x~
		check_contains(reg, "poxel32i", "[1, 2] * [-3, 4]", "[-6, 8]", "box product");
		check_contains(reg, "poxel32i", "[-2, 3]^2", "[0, 9]", "box square is non-negative");
		check_contains(reg, "poxel32i", "hull(1, 2)", "[1, 2]", "hull function");
		check_contains(reg, "poxel32i", "abs([-2, 1])", "[0, 2]", "box abs");
		check_box(reg, "poxel32i", "3~", 1, "positive", "3~ is one open tile");
		check_contains(reg, "poxel32i", "3~", "(3, ", "3~ lies above 3");
		check_contains(reg, "poxel32i", "1/[-1, 1]", "(-inf, inf)", "division by a box holding zero");
		check_contains(reg, "poxel32i", "exp(1)", "(-inf, inf)", "no enclosing exp: the entire line");
		check_box(reg, "poxel64i", "1/[-1, 1]", ~std::uint64_t(0), "undecidable", "entire 64-bit line counts 2^64 - 1 tiles");
		check_throws(reg, "posit32", "[1, 2]", "interval literal in a rounded type");
		check_throws(reg, "posit32", "3~", "tilde in a rounded type");
		check_throws(reg, "poxel32", "[1, 2]", "interval literal in a single-tile type");

		// values that cross types: a variable keeps its Value across a type switch
		{
			ExpressionEvaluator eval(reg.get("poxel16"));
			eval.evaluate("x = 0.1");                                   // the poxel16 tile (0.099976, 0.10004)
			eval.set_variable("h", Value(0.5));                         // a plain double, as sweep makes them
			eval.set_type(reg.get("poxel32i"));
			Value x = eval.evaluate("x + 0");             // an operation converts the variable
			const auto* box = std::any_cast<tile_interval<poxel<32, 2, uint8_t>>>(&x.native);
			// the same open interval, which poxel32's finer lattice holds exactly
			if (box == nullptr || box->lower<double>() != 0.0999755859375 || box->upper<double>() != 0.10003662109375
			    || !box->lower_open() || !box->upper_open() || !box->contains(0.1)) {
				std::cerr << "FAIL: a poxel16 tile read in poxel32i must be enclosed by its bounds: " << x.native_rep << "\n";
				++nrOfFailedTests;
			}
			Value h = eval.evaluate("h + 0");
			if (h.native_rep != "0.5" || h.tile_count != 1) {
				std::cerr << "FAIL: a plain double 0.5 in poxel32i should be the exact tile: " << h.native_rep << "\n";
				++nrOfFailedTests;
			}
			eval.set_type(reg.get("dd"));
			eval.evaluate("t = 1/3");
			eval.set_type(reg.get("poxel32i"));
			Value t = eval.evaluate("t + 0");
			const auto* tb = std::any_cast<tile_interval<poxel<32, 2, uint8_t>>>(&t.native);
			if (tb == nullptr || !tb->contains(1.0 / 3.0) || t.tile_count > 3) {
				std::cerr << "FAIL: a dd value in poxel32i should be bracketed: " << t.native_rep << "\n";
				++nrOfFailedTests;
			}
		}

		// a single tile from arithmetic on an open operand is a flag, not an enclosure: an
		// interval type must not promote it to a box that excludes the truth
		{
			ExpressionEvaluator eval(reg.get("poxel16"));
			Value r = eval.evaluate("r = (1/3) * 3");                   // (0.99951, 1): excludes 1
			Value l = eval.evaluate("0.1");
			Value s = eval.evaluate("sqrt(4)");
			Value q = eval.evaluate("3~ * 2");
			if (r.tile_encloses || !l.tile_encloses || !s.tile_encloses || q.tile_encloses) {
				std::cerr << "FAIL: single-tile enclosure flags: (1/3)*3 " << r.tile_encloses << ", 0.1 " << l.tile_encloses
				          << ", sqrt(4) " << s.tile_encloses << ", 3~ * 2 " << q.tile_encloses << " (expected 0 1 1 0)\n";
				++nrOfFailedTests;
			}
			// powers of an exact tile enclose (1/3 is the tile holding one third); of an open one, not
			Value p2 = eval.evaluate("pow(3, 2)");
			Value pn = eval.evaluate("pow(3, -1)");
			Value po = eval.evaluate("pow(0.1, 2)");
			if (!p2.tile_encloses || !pn.tile_encloses || po.tile_encloses) {
				std::cerr << "FAIL: single-tile pow enclosure: 3^2 " << p2.tile_encloses << ", 3^-1 " << pn.tile_encloses
				          << ", 0.1^2 " << po.tile_encloses << " (expected 1 1 0)\n";
				++nrOfFailedTests;
			}
			eval.set_type(reg.get("poxel32i"));
			Value rr = eval.evaluate("r + 0");
			const auto* box = std::any_cast<tile_interval<poxel<32, 2, uint8_t>>>(&rr.native);
			if (box == nullptr || !box->contains(1.0)) {
				std::cerr << "FAIL: a non-enclosing poxel16 tile read in poxel32i must still contain the truth: " << rr.native_rep << "\n";
				++nrOfFailedTests;
			}
		}

		// single tiles: sticky-flag arithmetic, the ubit says inexact
		check_contains(reg, "poxel16", "100^2", "(9984, 10048)", "poxel16 100^2 is not on the lattice");
		{
			Value v = eval_in(reg, "poxel32", "(1/3) * 3");
			if (v.tile_kind != 1 || !v.ubit) {
				std::cerr << "FAIL: poxel32 (1/3)*3 should be an open tile: " << v.native_rep << "\n";
				++nrOfFailedTests;
			}
			Value w = eval_in(reg, "poxel32", "sqrt(4)");
			if (w.tile_kind != 1 || w.ubit || w.native_rep != "2") {
				std::cerr << "FAIL: poxel32 sqrt(4) should be the exact tile 2: " << w.native_rep << "\n";
				++nrOfFailedTests;
			}
		}

		// the reference problem: 3x^2 + 100x + 2 = 0, as measured by
		// applications/precision/ubit/quadratic_roots.cpp (#1649)
		struct row { const char* type; std::uint64_t written, rearranged, r2; const char* written_sign; };
		const std::string r1w = "(-b + sqrt(b*b - 4*a*c)) / (2*a)";
		const std::string r1r = "(2*c) / (-b - sqrt(b*b - 4*a*c))";
		const std::string r2  = "(-b - sqrt(b*b - 4*a*c)) / (2*a)";
		const row exact[] = {
			{ "poxel16i", 16385, 7, 3, "undecidable" },
			{ "areal16i", 10581, 3, 3, "negative" },
			{ "areal32i", 1365, 3, 1, "negative" },
			{ "poxel32i", 1365, 3, 3, "negative" },
			{ "areal64i", 1365, 3, 1, "negative" },
			{ "poxel64i", 1367, 3, 3, "negative" },
		};
		const row ulp_wide[] = {
			{ "poxel16i", 17067, 9, 7, "undecidable" },
			{ "areal16i", 20821, 7, 5, "undecidable" },
			{ "areal32i", 5465, 7, 5, "negative" },
			{ "poxel32i", 9559, 9, 7, "negative" },
			{ "areal64i", 5465, 9, 7, "negative" },
			{ "poxel64i", 9559, 9, 7, "negative" },
		};
		auto run = [&](const row& r, const std::string& inputs, const std::string& label) {
			const TypeOps& ops = reg.get(r.type);
			ExpressionEvaluator eval(ops);
			std::stringstream statements(inputs);
			for (std::string statement; std::getline(statements, statement, ';');) eval.evaluate(statement);
			auto expect = [&](const std::string& expr, std::uint64_t tiles, const std::string& sign, const std::string& what) {
				Value v = eval.evaluate(expr);
				if (v.tile_count != tiles || v.tile_sign != sign) {
					std::cerr << "FAIL: quadratic " << label << " " << what << ": " << r.type << " = " << v.native_rep
					          << " (" << v.tile_count << " tiles, sign " << v.tile_sign << "; expected "
					          << tiles << " tiles, sign " << sign << ")\n";
					++nrOfFailedTests;
				}
			};
			expect(r1w, r.written, r.written_sign, "r1 as written");
			expect(r1r, r.rearranged, "negative", "r1 rearranged");
			expect(r2, r.r2, "negative", "r2");
		};
		for (const row& r : exact)    run(r, "a = 3; b = 100; c = 2", "exact");
		for (const row& r : ulp_wide) run(r, "a = 3~; b = 100~; c = 2~", "ULP-wide");
	}

	// ================================================================
	// Uncertainty box and decidability: ubox, decide (#1654 Phase 2)
	// ================================================================
	{
		// definitions are replayed in each type: b = 100~ is the open tile above 100 in the
		// type's own lattice, so the ULP-wide quadratic reproduces #1649 in every type
		ExpressionEvaluator session(reg.get("poxel32i"));
		session.evaluate("a = 3~");
		session.evaluate("b = 100~");
		session.evaluate("c = 2~");
		session.evaluate("a = 3~");                                       // a redefinition is history too
		if (session.definitions().size() != 4 || session.definitions().back().first != "a") {
			std::cerr << "FAIL: definitions should hold all four, in order, got " << session.definitions().size() << "\n";
			++nrOfFailedTests;
		}
		// the replay reproduces the session's values: b depends on the earlier a, and x on itself
		{
			ExpressionEvaluator history(reg.get("double"));
			for (const char* d : { "a = 1", "b = a + 1", "a = 5", "x = 1", "x = x + 1" }) history.evaluate(d);
			ExpressionEvaluator replayed = evaluator_for(reg.get("poxel32i"), history);
			const Value b = replayed.evaluate("b + 0"), a = replayed.evaluate("a + 0"), x = replayed.evaluate("x + 0");
			if (b.native_rep != "2" || a.native_rep != "5" || x.native_rep != "2") {
				std::cerr << "FAIL: replayed history gives a = " << a.native_rep << ", b = " << b.native_rep << ", x = " << x.native_rep
				          << " (the session holds 5, 2, 2)\n";
				++nrOfFailedTests;
			}
		}
		const std::string r1 = "(-b + sqrt(b*b - 4*a*c)) / (2*a)";
		const struct { const char* type; std::uint64_t tiles; const char* sign; } ulp_wide[] = {
			{ "areal16i", 20821, "undecidable" }, { "poxel16i", 17067, "undecidable" },
			{ "areal32i", 5465, "negative" },     { "poxel32i", 9559, "negative" },
			{ "areal64i", 5465, "negative" },     { "poxel64i", 9559, "negative" },
		};
		for (const auto& row : ulp_wide) {
			ExpressionEvaluator eval = evaluator_for(reg.get(row.type), session);
			const BoxReport b = box_report(row.type, eval.evaluate(r1));
			if (b.tiles != row.tiles || b.sign != row.sign) {
				std::cerr << "FAIL: replayed ULP-wide r1 in " << row.type << ": " << b.tiles << " tiles, " << b.sign
				          << " (expected " << row.tiles << ", " << row.sign << ")\n";
				++nrOfFailedTests;
			}
		}

		// decimals of accuracy of a box: -log10(width / 2|midpoint|)
		{
			ExpressionEvaluator eval(reg.get("poxel32i"));
			const BoxReport exact = box_report("poxel32i", eval.evaluate("3"));
			const BoxReport third = box_report("poxel32i", eval.evaluate("1/3"));
			const BoxReport wide  = box_report("poxel32i", eval.evaluate("[1, 3]"));
			const BoxReport zero  = box_report("poxel32i", eval.evaluate("[-1, 1]"));
			if (!exact.exact || !std::isinf(exact.decimals)) {
				std::cerr << "FAIL: an exact box should report exact, infinite decimals\n";
				++nrOfFailedTests;
			}
			if (third.decimals < 8.0 || third.decimals > 9.5) {
				std::cerr << "FAIL: poxel32i 1/3 decimals " << third.decimals << " (expected ~8.8: one 27-bit tile)\n";
				++nrOfFailedTests;
			}
			if (std::fabs(wide.rel_width - 1.0) > 1e-12 || std::fabs(wide.decimals - std::log10(2.0)) > 1e-12) {
				std::cerr << "FAIL: [1, 3] has relative width 1 and log10(2) decimals: " << wide.rel_width << ", " << wide.decimals << "\n";
				++nrOfFailedTests;
			}
			if (!std::isinf(zero.rel_width) || zero.decimals != 0.0) {
				std::cerr << "FAIL: a box straddling zero has no relative accuracy\n";
				++nrOfFailedTests;
			}
		}

		// predicates
		{
			Predicate p;
			std::string err;
			const struct { const char* text; bool ok; const char* op; const char* lhs; const char* rhs; } parses[] = {
				{ "sign x - 1", true, "sign", "x - 1", "" },
				{ "a <= b", true, "<=", "a", "b" },
				{ "f([1, 2]) != (x > y)", true, "!=", "f([1, 2])", "(x > y)" },   // only one at the top level
				{ "a < b < c", false, "", "", "" },
				{ "pow(a, 2) >= [0, 1]", true, ">=", "pow(a, 2)", "[0, 1]" },
				{ "a = b", false, "", "", "" },
				{ "a <", false, "", "", "" },
				{ "a + b", false, "", "", "" },
			};
			for (const auto& c : parses) {
				p = Predicate{};
				err.clear();
				const bool ok = parse_predicate(c.text, p, err);
				if (ok != c.ok || (ok && (p.op != c.op || p.lhs != c.lhs || p.rhs != c.rhs))) {
					std::cerr << "FAIL: parse_predicate(\"" << c.text << "\") ok=" << ok << " op=" << p.op << " lhs=" << p.lhs << " rhs=" << p.rhs << "\n";
					++nrOfFailedTests;
				}
			}

			// verdicts over sets, exhaustive over small boxes in poxel8i: brute force over
			// sample points of each set must agree with every decided verdict
			const TypeOps& ops = reg.get("poxel8i");
			const char* boxes[] = { "1", "2", "[1, 2]", "1~", "[0.5, 1]", "[1, 1~]", "2~", "[-1, 1]", "0" };
			const Relation rels[] = { Relation::lt, Relation::le, Relation::gt, Relation::ge, Relation::eq, Relation::ne };
			int bad = 0;
			for (const char* x : boxes) for (const char* y : boxes) {
				ExpressionEvaluator eval(ops);
				const Value a = eval.evaluate(x), b = eval.evaluate(y);
				const Ends ea = ends_of(a), eb = ends_of(b);
				// points: each closed end, and points just inside each end
				auto points = [](const Ends& e) {
					std::vector<long double> pts;
					const long double w = e.hi - e.lo;
					if (!e.lo_open) pts.push_back(e.lo);
					if (!e.hi_open) pts.push_back(e.hi);
					if (w > 0) { pts.push_back(e.lo + w / 1024); pts.push_back(e.hi - w / 1024); pts.push_back(e.lo + w / 2); }
					return pts;
				};
				for (Relation rel : rels) {
					const Verdict v = compare(rel, ea, eb);
					bool any_true = false, any_false = false;
					// equality between continuous sets: possible exactly when they share a point
					auto inside = [](long double q, const Ends& e) {
						return (e.lo_open ? q > e.lo : q >= e.lo) && (e.hi_open ? q < e.hi : q <= e.hi);
					};
					if (rel == Relation::eq || rel == Relation::ne) {
						bool meet = false;
						for (long double u : points(ea)) meet = meet || inside(u, eb);
						for (long double w : points(eb)) meet = meet || inside(w, ea);
						const bool one_point = ea.lo == ea.hi && eb.lo == eb.hi && ea.lo == eb.lo && meet;
						const bool can_equal = meet, can_differ = !one_point;
						any_true  = (rel == Relation::eq) ? can_equal : can_differ;
						any_false = (rel == Relation::eq) ? can_differ : can_equal;
					}
					else for (long double u : points(ea)) for (long double w : points(eb)) {
						bool t = false;
						switch (rel) {
						case Relation::lt: t = u < w; break;
						case Relation::le: t = u <= w; break;
						case Relation::gt: t = u > w; break;
						case Relation::ge: t = u >= w; break;
						case Relation::eq: t = u == w; break;
						default:           t = u != w; break;
						}
						(t ? any_true : any_false) = true;
					}
					const bool wrong = (v == Verdict::yes && any_false) || (v == Verdict::no && any_true)
					                || (v == Verdict::undecidable && !(any_true && any_false));
					if (wrong && bad++ < 5) std::cerr << "FAIL: verdict " << to_string(v) << " for " << x << " op#" << static_cast<int>(rel) << " " << y << "\n";
				}
			}
			nrOfFailedTests += bad;
		}

		// decide in each type, and the narrowest type that decides
		{
			Predicate p;
			std::string err;
			parse_predicate("sign " + r1, p, err);
			std::vector<Decision> ds;
			for (const std::string& t : default_box_types()) ds.push_back(decide_in(reg.get(t), t, session, p));
			const Decision* first = narrowest_decided(ds);
			if (first == nullptr || first->type != "areal32i" || first->answer != "negative") {
				std::cerr << "FAIL: the ULP-wide r1 sign should be decided first by areal32i: " << (first ? first->type : "none") << "\n";
				++nrOfFailedTests;
			}
			const Decision rounded = decide_in(reg.get("double"), "double", session, p);
			if (rounded.error.empty()) {
				std::cerr << "FAIL: decide in a rounded type should be refused\n";
				++nrOfFailedTests;
			}
		}

		// the trace records box widths: the dependency problem is the -b + sqrt(d) step
		{
			ExpressionEvaluator eval = evaluator_for(reg.get("poxel16i"), session);
			eval.enable_trace(true);
			eval.evaluate(r1);
			std::uint64_t widest = 0;
			std::string op;
			for (const TraceStep& t : eval.trace_steps()) {
				if (t.result_tiles > widest) { widest = t.result_tiles; op = t.operation; }
			}
			if (op != "add" || widest < 10000) {
				std::cerr << "FAIL: the widest trace step of r1 should be the cancelling add, got " << op << " (" << widest << " tiles)\n";
				++nrOfFailedTests;
			}
		}

		// the rounded types next to the boxes: #1649's errors, from exact inputs
		{
			ExpressionEvaluator exact_inputs(reg.get("double"));
			exact_inputs.evaluate("a = 3");
			exact_inputs.evaluate("b = 100");
			exact_inputs.evaluate("c = 2");
			std::string reference, note;
			const auto rr = rounded_reports(reg, exact_inputs, r1, reference, note);
			const struct { const char* type; double lo, hi; } expect[] = {
				{ "fp16", 0.03, 0.05 }, { "posit16", 1.0, 1.2 }, { "float", 5e-6, 6e-6 }, { "posit32", 2e-6, 2.5e-6 }, { "double", 1e-14, 2e-14 },
			};
			if (rr.size() != 5 || !note.empty()) {
				std::cerr << "FAIL: rounded reports: " << rr.size() << " rows, note '" << note << "'\n";
				++nrOfFailedTests;
			}
			for (std::size_t i = 0; i < rr.size() && i < 5; ++i) {
				if (rr[i].type != expect[i].type || rr[i].rel_error < expect[i].lo || rr[i].rel_error > expect[i].hi) {
					std::cerr << "FAIL: rounded " << rr[i].type << " relative error " << rr[i].rel_error << "\n";
					++nrOfFailedTests;
				}
			}
			// tile syntax in the inputs: no single true value
			std::string ref2, note2;
			if (!rounded_reports(reg, session, r1, ref2, note2).empty() || note2.empty()) {
				std::cerr << "FAIL: rounded reports should decline ULP-wide inputs\n";
				++nrOfFailedTests;
			}
		}
	}

	// ================================================================
	// The tightest box: the oracle (#1654 Phase 3)
	// ================================================================
	{
		const std::string r1w = "(-b + sqrt(b*b - 4*a*c)) / (2*a)";
		const std::string r1r = "(2*c) / (-b - sqrt(b*b - 4*a*c))";
		const std::string r2  = "(-b - sqrt(b*b - 4*a*c)) / (2*a)";
		// the computed box and the oracle's, for one type
		auto both = [&](const ExpressionEvaluator& session, const std::string& type, const std::string& expr) {
			const TypeOps& ops = reg.get(type);
			ExpressionEvaluator eval = evaluator_for(ops, session);
			const Value v = eval.evaluate(expr);
			return std::make_pair(v, ops.tightest(session, expr));
		};
		// soundness: the tightest box lies inside every valid enclosure, so inside the computed box
		auto inside = [&](const std::string& type, const Value& v, const TightestReport& t) {
			using sw::universal::tile_interval;
			std::int64_t lo = 0, hi = 0;
			bool found = false;
			auto keys = [&](auto tag) {
				using Tile = decltype(tag);
				if (const auto* p = std::any_cast<tile_interval<Tile>>(&v.native)) { lo = p->lo_key(); hi = p->hi_key(); found = true; }
			};
			keys(areal<16, 5, uint8_t>{}); keys(areal<32, 8, uint8_t>{}); keys(areal<64, 11, uint8_t>{});
			keys(poxel<16, 2, uint8_t>{}); keys(poxel<32, 2, uint8_t>{}); keys(poxel<64, 2, uint8_t>{});
			if (!found || t.lo_key < lo || t.hi_key > hi) {
				std::cerr << "FAIL: tightest box " << t.text << " is not inside the computed " << v.native_rep << " in " << type << "\n";
				++nrOfFailedTests;
			}
		};

		// exact inputs: one tile, located; the as-written formula is 1365 tiles at 32 bits
		ExpressionEvaluator exact(reg.get("poxel32i"));
		for (const char* d : { "a = 3", "b = 100", "c = 2" }) exact.evaluate(d);
		for (const std::string& type : default_box_types()) {
			for (const std::string& e : { r1w, r1r, r2 }) {
				const auto [v, t] = both(exact, type, e);
				if (!t.available || !t.proven || t.inner_tiles != 1 || t.outer_tiles != 1) {
					std::cerr << "FAIL: exact-input tightest box in " << type << " for " << e << ": " << t.inner_tiles << "-" << t.outer_tiles << " " << t.note << "\n";
					++nrOfFailedTests;
				}
				inside(type, v, t);
			}
		}

		// ULP-wide inputs: the tightest box depends on the function, not on how it is written,
		// so both forms of r1 must agree -- and match #1649's corner polynomials
		ExpressionEvaluator wide(reg.get("poxel32i"));
		for (const char* d : { "a = 3~", "b = 100~", "c = 2~" }) wide.evaluate(d);
		const struct { const char* type; std::uint64_t r1, r2; } tightest[] = {
			{ "areal16i", 7, 3 }, { "poxel16i", 5, 3 }, { "areal32i", 5, 5 },
			{ "poxel32i", 5, 3 }, { "areal64i", 5, 5 }, { "poxel64i", 5, 3 },
		};
		for (const auto& row : tightest) {
			const auto [vw, tw] = both(wide, row.type, r1w);
			const auto [vr, tr] = both(wide, row.type, r1r);
			const auto [v2, t2] = both(wide, row.type, r2);
			const bool ok = tw.proven && tr.proven && t2.proven && tw.outer_tiles == row.r1 && tr.outer_tiles == row.r1
			             && t2.outer_tiles == row.r2 && tw.lo_key == tr.lo_key && tw.hi_key == tr.hi_key;
			if (!ok) {
				std::cerr << "FAIL: ULP-wide tightest in " << row.type << ": r1 as written " << tw.inner_tiles << "-" << tw.outer_tiles
				          << ", rearranged " << tr.inner_tiles << "-" << tr.outer_tiles << ", r2 " << t2.inner_tiles << "-" << t2.outer_tiles
				          << " (expected " << row.r1 << ", " << row.r1 << ", " << row.r2 << ", all proven)\n";
				++nrOfFailedTests;
			}
			inside(row.type, vw, tw);
			inside(row.type, vr, tr);
			inside(row.type, v2, t2);
		}

		// not monotone: x*x over [-1, 3] has its minimum 0 at a point the bisection reaches
		// (the midpoint of [-1, 3] is 1, then 0): proven [0, 9], although x*x computes [-3, 9]
		ExpressionEvaluator sq(reg.get("poxel32i"));
		sq.evaluate("x = [-1, 3]");
		{
			const auto [v, t] = both(sq, "poxel16i", "x*x");
			const auto [p, tp] = both(sq, "poxel16i", "x^2");
			if (!t.proven || t.text != "[0, 9]" || p.tile_count != t.outer_tiles || v.tile_count <= t.outer_tiles) {
				std::cerr << "FAIL: x*x over [-1, 3]: tightest " << t.text << " (" << t.note << "), x^2 " << p.native_rep << ", x*x " << v.native_rep << "\n";
				++nrOfFailedTests;
			}
			inside("poxel16i", v, t);
			inside("poxel16i", p, tp);
		}
		// over [-1, 2] no dyadic midpoint is 0: bracketed to within the one tile at 0
		sq.evaluate("x = [-1, 2]");
		{
			const auto [v, t] = both(sq, "poxel16i", "x^2");
			if (!t.available || t.proven || t.outer_tiles - t.inner_tiles != 1) {
				std::cerr << "FAIL: x^2 over [-1, 2] should be bracketed to one tile: " << t.inner_tiles << "-" << t.outer_tiles << "\n";
				++nrOfFailedTests;
			}
			inside("poxel16i", v, t);
		}

		// open inputs exclude their ends: the identity on 3~ is the one open tile above 3, as the
		// tile type computes it -- not the three tiles of its closure [3, next(3)]
		{
			ExpressionEvaluator open_in(reg.get("poxel32i"));
			open_in.evaluate("a = 3~");
			open_in.evaluate("t = 0.1~");                              // 0.1 is no lattice point: the open tile that holds it
			open_in.evaluate("h = [1, 2~]");                           // a hull keeps the open end it takes from 2~
			for (const std::string& type : { std::string("poxel16i"), std::string("areal32i"), std::string("poxel64i") }) {
				for (const std::string& e : { std::string("a + 0"), std::string("2 * a"), std::string("t + 0"), std::string("-t") }) {
					const auto [v, t] = both(open_in, type, e);
					if (!t.proven || t.outer_tiles != v.tile_count || v.tile_count != 1) {
						std::cerr << "FAIL: open input " << e << " in " << type << ": tightest " << t.inner_tiles << "-" << t.outer_tiles
						          << ", computed " << v.tile_count << " (" << t.note << ")\n";
						++nrOfFailedTests;
					}
					inside(type, v, t);
				}
			}
			for (const std::string& type : { std::string("poxel16i"), std::string("areal32i") }) {
				const auto [v, t] = both(open_in, type, "h + 0");
				if (!t.proven || t.outer_tiles != v.tile_count) {
					std::cerr << "FAIL: [1, 2~] + 0 in " << type << ": tightest " << t.inner_tiles << "-" << t.outer_tiles << ", computed " << v.tile_count << "\n";
					++nrOfFailedTests;
				}
				inside(type, v, t);
			}
			// maxpos~ is (maxpos, inf): no bounded input; sqrt of a negative set has no real value
			ExpressionEvaluator edge(reg.get("poxel32i")), negative(reg.get("poxel32i"));
			edge.evaluate("m = 1e400");
			edge.evaluate("big = m~");
			negative.evaluate("neg = [-2, -1]");
			const TightestReport tb = reg.get("poxel32i").tightest(edge, "big + 0");
			const TightestReport tn = reg.get("poxel32i").tightest(negative, "sqrt(neg)");
			if (tb.available || tn.available || tn.note.find("not a real number") == std::string::npos || tn.subdivisions > 1) {
				std::cerr << "FAIL: maxpos~ should be unbounded (" << tb.note << "), sqrt of a negative set no real number (" << tn.note << ")\n";
				++nrOfFailedTests;
			}
		}

		// no exact enclosure for log; (1/3) * 3 is one tile, unresolved among the three around 1
		ExpressionEvaluator plain(reg.get("poxel32i"));
		{
			const TightestReport tl = reg.get("poxel32i").tightest(plain, "log(2)");
			const TightestReport tt = reg.get("poxel32i").tightest(plain, "(1/3) * 3");
			if (tl.available || tl.note.empty() || !tt.available || tt.proven || tt.outer_tiles != 1) {
				std::cerr << "FAIL: log(2) should have no tightest box, (1/3)*3 one unresolved tile\n";
				++nrOfFailedTests;
			}
		}
	}

	// ================================================================
	// Report
	// ================================================================
	if (nrOfFailedTests > 0) {
		std::cerr << "ucalc regression: FAIL (" << nrOfFailedTests << " failures)\n";
	} else {
		std::cout << "ucalc regression: PASS\n";
	}

	return nrOfFailedTests > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
catch (const std::exception& ex) {
	std::cerr << "ucalc regression: FATAL: " << ex.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "ucalc regression: FATAL: unknown exception" << std::endl;
	return EXIT_FAILURE;
}
