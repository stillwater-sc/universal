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

// Pull in the build_default_registry and native type helpers
// by including ucalc.cpp's namespace content directly.
// We forward-declare what we need and reuse the same registry builder.
namespace sw { namespace ucalc {
	// Defined in ucalc.cpp -- we duplicate the essentials here
	// to keep regression.cpp self-contained
	extern TypeRegistry build_default_registry();
}}

// Since build_default_registry is defined in ucalc.cpp with static helpers,
// we include the relevant portions. The simplest approach: build a minimal
// registry for testing using register_type<> directly.
namespace {

using namespace sw::universal;
using namespace sw::ucalc;

TypeRegistry build_test_registry() {
	TypeRegistry reg;
	// Note: native float/double require explicit specializations (in ucalc.cpp)
	// because the generic register_type<T> relies on ADL which doesn't work for
	// fundamental types. Use fp32/fp64 (cfloat equivalents) for testing instead.
	reg.add("fp32",     register_type<fp32>("fp32"));
	reg.add("fp64",     register_type<fp64>("fp64"));
	reg.add("posit8",   register_type<posit<8, 2, uint8_t>>("posit8"));
	reg.add("posit32",  register_type<posit<32, 2, uint32_t>>("posit32"));
	reg.add("fp16",     register_type<fp16>("fp16"));
	reg.add("fp128",    register_type<fp128>("fp128"));
	reg.add("fp8e4m3",  register_type<fp8e4m3>("fp8e4m3"));
	reg.add("dd",       register_type<dd>("dd"));
	reg.add("qd",       register_type<qd>("qd"));
	reg.add("fixpnt16", register_type<fixpnt<16, 8, Modulo, uint16_t>>("fixpnt16"));
	reg.add("lns16",    register_type<lns<16, 8, uint16_t>>("lns16"));
	reg.add("int32",    register_type<integer<32, uint32_t>>("int32"));
	reg.add("decimal32", register_type<dfloat<7, 6>>("decimal32"));
	return reg;
}

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
		double err = std::abs(result.num - expected);
		if (err > tol) {
			std::cerr << "FAIL: " << label << ": " << type << "> " << expr
			          << " = " << result.native_rep << " (expected " << expected
			          << ", err=" << err << ")\n";
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
	TypeRegistry reg = build_test_registry();

	// ================================================================
	// 1. Basic arithmetic
	// ================================================================
	check_value(reg, "fp64", "2 + 3", 5.0, 0.0, "add");
	check_value(reg, "fp64", "10 - 7", 3.0, 0.0, "sub");
	check_value(reg, "fp64", "6 * 7", 42.0, 0.0, "mul");
	check_value(reg, "fp64", "1 / 4", 0.25, 0.0, "div");
	check_value(reg, "fp64", "2 ^ 10", 1024.0, 0.0, "pow");
	check_value(reg, "fp64", "-3 + 5", 2.0, 0.0, "unary neg");
	check_value(reg, "fp64", "(2 + 3) * 4", 20.0, 0.0, "parens");

	// ================================================================
	// 2. Operator precedence
	// ================================================================
	check_value(reg, "fp64", "2 + 3 * 4", 14.0, 0.0, "precedence mul>add");
	check_value(reg, "fp64", "10 - 2 * 3", 4.0, 0.0, "precedence mul>sub");
	check_value(reg, "fp64", "2 * 3 ^ 2", 18.0, 0.0, "precedence pow>mul");
	check_value(reg, "fp64", "-2 ^ 2", -4.0, 0.0, "unary neg then pow"); // -(2^2), not (-2)^2

	// ================================================================
	// 3. Constants
	// ================================================================
	check_value(reg, "fp64", "pi", 3.14159265358979323846, 1e-15, "pi");
	check_value(reg, "fp64", "e", 2.71828182845904523536, 1e-15, "e");
	check_value(reg, "fp64", "phi", 1.61803398874989484820, 1e-15, "phi");
	check_value(reg, "fp64", "ln2", 0.69314718055994530942, 1e-15, "ln2");

	// ================================================================
	// 4. Built-in functions
	// ================================================================
	check_value(reg, "fp64", "sqrt(4)", 2.0, 0.0, "sqrt");
	check_value(reg, "fp64", "abs(-7)", 7.0, 0.0, "abs");
	check_value(reg, "fp64", "log(1)", 0.0, 1e-15, "log(1)");
	check_value(reg, "fp64", "exp(0)", 1.0, 0.0, "exp(0)");
	check_value(reg, "fp64", "sin(0)", 0.0, 0.0, "sin(0)");
	check_value(reg, "fp64", "cos(0)", 1.0, 0.0, "cos(0)");
	check_value(reg, "fp64", "pow(2, 10)", 1024.0, 0.0, "pow(2,10)");

	// ================================================================
	// 5. Variables
	// ================================================================
	{
		const TypeOps& ops = reg.get("fp64");
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
	check_binary(reg, "fp64", "1.0", "0b0.", "fp64 binary prefix");
	check_binary(reg, "posit32", "1.0", "0b0.", "posit binary prefix");
	check_binary(reg, "fp32", "1.0", "0b0.", "fp32 binary prefix");
	check_binary(reg, "decimal32", "1.0", "0b0.", "decimal32 binary prefix");

	// ================================================================
	// 9. Components non-empty
	// ================================================================
	{
		Value v = eval_in(reg, "fp64", "1/3");
		if (v.components_rep.empty()) {
			std::cerr << "FAIL: fp64 components empty\n";
			++nrOfFailedTests;
		}
		if (v.components_rep.find("sign:") == std::string::npos) {
			std::cerr << "FAIL: fp64 components missing 'sign:': "
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
	check_throws(reg, "fp64", "1 +", "trailing operator");
	check_throws(reg, "fp64", "foo", "undefined variable");
	check_throws(reg, "fp64", "bar(1)", "undefined function");

	// ================================================================
	// 11. Special values
	// ================================================================
	check_value(reg, "fp64", "1/0", std::numeric_limits<double>::infinity(), 0.0, "div by zero");
	check_value(reg, "fp64", "0 * (1/0)", std::numeric_limits<double>::quiet_NaN(), 0.0, "0*inf=nan");

	// NaN != NaN, so check with isnan
	{
		Value v = eval_in(reg, "fp64", "0 * (1/0)");
		if (!std::isnan(v.num)) {
			std::cerr << "FAIL: 0*inf should be NaN, got " << v.num << "\n";
			++nrOfFailedTests;
		}
	}

	// ================================================================
	// 12. Cross-type evaluation
	// ================================================================
	check_value(reg, "int32", "7 / 2", 3.0, 0.5, "int32 truncation");
	check_value(reg, "fixpnt16", "0.5 + 0.25", 0.75, 0.01, "fixpnt add");
	check_contains(reg, "lns16", "1.0", "1", "lns16 one");
	check_value(reg, "decimal32", "0.1 + 0.2", 0.3, 0.0001, "decimal add");

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
