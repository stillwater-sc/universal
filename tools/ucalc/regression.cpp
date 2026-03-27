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

#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/lns/lns.hpp>
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

#include "type_dispatch.hpp"
#include "expression.hpp"
#include "registry.hpp"
#include "output_format.hpp"

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
