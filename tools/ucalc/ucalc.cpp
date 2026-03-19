// ucalc.cpp: Universal Mixed-Precision REPL Calculator
//
// Interactive calculator for exploring and comparing arithmetic
// across Universal number types. Replaces the workflow of writing,
// compiling, and running C++ for each type comparison.
//
// Usage:
//   ucalc                          # interactive REPL
//   echo "type posit32; 1/3" | ucalc  # pipe/script mode
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <cstdlib>

// Suppress exception macros -- ucalc catches errors at the REPL level
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#define CFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define FIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#define BFLOAT16_THROW_ARITHMETIC_EXCEPTION 0
#define DD_THROW_ARITHMETIC_EXCEPTION 0
#define QD_THROW_ARITHMETIC_EXCEPTION 0
#define LNS_THROW_ARITHMETIC_EXCEPTION 0
#define INTEGER_THROW_ARITHMETIC_EXCEPTION 0
// dbns and takum omitted -- math function stubs cause link errors
#define HFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define DFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define DD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0
#define TD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0
#define QD_CASCADE_THROW_ARITHMETIC_EXCEPTION 0

#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>

// Number system headers -- MVP types
#include <universal/number/posit/posit.hpp>
#include <universal/number/cfloat/cfloat.hpp>
#include <universal/number/fixpnt/fixpnt.hpp>
#include <universal/number/bfloat16/bfloat16.hpp>
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>

// Extended types
#include <universal/number/lns/lns.hpp>
#include <universal/number/integer/integer.hpp>
// dbns and takum omitted -- math function stubs cause link errors
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>

// ucalc headers
#include "type_dispatch.hpp"
#include "expression.hpp"

#ifdef _WIN32
#include <io.h>
#define ISATTY _isatty
#define FILENO _fileno
#else
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif

#ifdef UCALC_USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

namespace sw { namespace ucalc {

// Specialization for native float: use std:: math functions
template<>
TypeOps register_type<float>(const std::string& name) {
	TypeOps ops;
	ops.name = name;
	ops.type_tag = "float (IEEE-754 binary32)";

	ops.from_double = [](double v) -> Value {
		float x = static_cast<float>(v);
		std::ostringstream bin_ss;
		bin_ss << sw::universal::to_binary(x);
		std::ostringstream comp_ss;
		comp_ss << "float: " << std::setprecision(9) << x;
		return Value(double(x), bin_ss.str(), comp_ss.str(), "float");
	};
	ops.add = [](const Value& a, const Value& b) -> Value {
		float r = float(a.num) + float(b.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.sub = [](const Value& a, const Value& b) -> Value {
		float r = float(a.num) - float(b.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.mul = [](const Value& a, const Value& b) -> Value {
		float r = float(a.num) * float(b.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.div = [](const Value& a, const Value& b) -> Value {
		float r = float(a.num) / float(b.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.negate = [](const Value& a) -> Value {
		float r = -float(a.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.fn_sqrt = [](const Value& a) -> Value {
		float r = std::sqrt(float(a.num));
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.fn_abs = [](const Value& a) -> Value {
		float r = std::abs(float(a.num));
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.fn_log = [](const Value& a) -> Value {
		float r = std::log(float(a.num));
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.fn_exp = [](const Value& a) -> Value {
		float r = std::exp(float(a.num));
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.fn_sin = [](const Value& a) -> Value {
		float r = std::sin(float(a.num));
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.fn_cos = [](const Value& a) -> Value {
		float r = std::cos(float(a.num));
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	ops.fn_pow = [](const Value& a, const Value& b) -> Value {
		float r = std::pow(float(a.num), float(b.num));
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(double(r), bs.str(), "", "float");
	};
	return ops;
}

// Specialization for native double: use std:: math functions
template<>
TypeOps register_type<double>(const std::string& name) {
	TypeOps ops;
	ops.name = name;
	ops.type_tag = "double (IEEE-754 binary64)";

	ops.from_double = [](double v) -> Value {
		std::ostringstream bin_ss;
		bin_ss << sw::universal::to_binary(v);
		std::ostringstream comp_ss;
		comp_ss << "double: " << std::setprecision(17) << v;
		return Value(v, bin_ss.str(), comp_ss.str(), "double");
	};
	ops.add = [](const Value& a, const Value& b) -> Value {
		double r = a.num + b.num;
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.sub = [](const Value& a, const Value& b) -> Value {
		double r = a.num - b.num;
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.mul = [](const Value& a, const Value& b) -> Value {
		double r = a.num * b.num;
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.div = [](const Value& a, const Value& b) -> Value {
		double r = a.num / b.num;
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.negate = [](const Value& a) -> Value {
		double r = -a.num;
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.fn_sqrt = [](const Value& a) -> Value {
		double r = std::sqrt(a.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.fn_abs = [](const Value& a) -> Value {
		double r = std::abs(a.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.fn_log = [](const Value& a) -> Value {
		double r = std::log(a.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.fn_exp = [](const Value& a) -> Value {
		double r = std::exp(a.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.fn_sin = [](const Value& a) -> Value {
		double r = std::sin(a.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.fn_cos = [](const Value& a) -> Value {
		double r = std::cos(a.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	ops.fn_pow = [](const Value& a, const Value& b) -> Value {
		double r = std::pow(a.num, b.num);
		std::ostringstream bs; bs << sw::universal::to_binary(r);
		return Value(r, bs.str(), "", "double");
	};
	return ops;
}

// Build the default type registry with ~15 MVP types
TypeRegistry build_default_registry() {
	using namespace sw::universal;

	TypeRegistry reg;

	// Native IEEE types
	reg.add("float",    register_type<float>("float"));
	reg.add("double",   register_type<double>("double"));

	// Posit types
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

	// Double-double and quad-double
	reg.add("dd",       register_type<dd>("dd"));
	reg.add("qd",       register_type<qd>("qd"));

	// Logarithmic number system types
	reg.add("lns8",     register_type<lns<8, 2, uint8_t>>("lns8"));
	reg.add("lns16",    register_type<lns<16, 8, uint16_t>>("lns16"));
	reg.add("lns32",    register_type<lns<32, 16, uint32_t>>("lns32"));

	// Integer types
	reg.add("int8",     register_type<integer<8, uint8_t>>("int8"));
	reg.add("int16",    register_type<integer<16, uint16_t>>("int16"));
	reg.add("int32",    register_type<integer<32, uint32_t>>("int32"));
	reg.add("int64",    register_type<integer<64, uint64_t>>("int64"));

	// Note: dbns and takum omitted -- their math functions are declared
	// but not yet implemented (missing sqrt.hpp etc.), causing link errors.

	// Hexadecimal floating-point
	reg.add("hfloat32", register_type<hfloat<6, 7>>("hfloat32"));
	reg.add("hfloat64", register_type<hfloat<14, 7>>("hfloat64"));

	// Decimal floating-point
	reg.add("decimal32",  register_type<dfloat<7, 6>>("decimal32"));
	reg.add("decimal64",  register_type<dfloat<16, 8>>("decimal64"));

	// Cascaded double-double
	reg.add("dd_cascade", register_type<dd_cascade>("dd_cascade"));
	reg.add("td_cascade", register_type<td_cascade>("td_cascade"));
	reg.add("qd_cascade", register_type<qd_cascade>("qd_cascade"));

	return reg;
}

// Trim whitespace from both ends
static std::string trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos) return "";
	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

// Split a line by semicolons into multiple commands
static std::vector<std::string> split_commands(const std::string& line) {
	std::vector<std::string> cmds;
	std::istringstream ss(line);
	std::string segment;
	while (std::getline(ss, segment, ';')) {
		std::string trimmed = trim(segment);
		if (!trimmed.empty()) {
			cmds.push_back(trimmed);
		}
	}
	return cmds;
}

// Print help text
static void print_help() {
	std::cout << "ucalc -- Universal Mixed-Precision REPL Calculator\n\n";
	std::cout << "Commands:\n";
	std::cout << "  type <name>    Set active arithmetic type (e.g., type posit32)\n";
	std::cout << "  types          List all available types\n";
	std::cout << "  show <expr>    Evaluate and display value + binary + components\n";
	std::cout << "  compare <expr> Evaluate across all types in a table\n";
	std::cout << "  bits <expr>    Show raw bit pattern as hex and binary\n";
	std::cout << "  range          Show dynamic range of current type\n";
	std::cout << "  precision      Show effective precision of current type\n";
	std::cout << "  ulp <value>    Show ULP at the given value\n";
	std::cout << "  sweep <expr> for <var> in [a, b, n]\n";
	std::cout << "                 Evaluate across a range, show error vs double\n";
	std::cout << "  faithful <expr> Check if result is faithfully rounded\n";
	std::cout << "  vars           List defined variables\n";
	std::cout << "  help           Show this help\n";
	std::cout << "  quit / exit    Exit the calculator\n";
	std::cout << "\n";
	std::cout << "Expressions:\n";
	std::cout << "  Arithmetic:    +  -  *  /  ^  (parentheses)\n";
	std::cout << "  Functions:     sqrt, abs, log, exp, sin, cos, pow\n";
	std::cout << "  Constants:     pi, e\n";
	std::cout << "  Variables:     x = 1/3  (then use x in expressions)\n";
	std::cout << "  Semicolons:    type posit32; 1/3 + 1/3 + 1/3\n";
	std::cout << std::endl;
}

// REPL state
struct ReplState {
	TypeRegistry registry;
	ExpressionEvaluator* evaluator;
	std::string active_type;
	bool interactive;
};

// Process a single command or expression
// Returns false if the REPL should exit
static bool process_command(const std::string& input, ReplState& state) {
	std::string line = trim(input);
	if (line.empty()) return true;

	// Check for commands
	if (line == "quit" || line == "exit") {
		return false;
	}

	if (line == "help") {
		print_help();
		return true;
	}

	if (line == "types") {
		std::cout << "Available types:\n";
		for (const auto& alias : state.registry.aliases()) {
			const TypeOps* ops = state.registry.find(alias);
			std::string marker = (alias == state.active_type) ? " *" : "";
			std::cout << "  " << std::left << std::setw(12) << alias
			          << ops->type_tag << marker << "\n";
		}
		std::cout << std::endl;
		return true;
	}

	if (line == "vars") {
		const auto& vars = state.evaluator->variables();
		if (vars.empty()) {
			std::cout << "No variables defined.\n";
		} else {
			for (const auto& kv : vars) {
				std::cout << "  " << std::left << std::setw(12) << kv.first
				          << " = " << std::setprecision(17) << kv.second.num << "\n";
			}
		}
		std::cout << std::endl;
		return true;
	}

	// type <name>
	if (line.substr(0, 5) == "type " || line.substr(0, 5) == "type\t") {
		std::string type_name = trim(line.substr(5));
		const TypeOps* ops = state.registry.find(type_name);
		if (!ops) {
			std::cerr << "Error: unknown type '" << type_name << "'. Use 'types' to list available types.\n";
			return true;
		}
		state.active_type = type_name;
		state.evaluator->set_type(*ops);
		if (state.interactive) {
			std::cout << "Active type: " << type_name << " (" << ops->type_tag << ")\n";
		}
		return true;
	}

	// show <expr>
	if (line.substr(0, 5) == "show " || line.substr(0, 5) == "show\t") {
		std::string expr = trim(line.substr(5));
		try {
			Value result = state.evaluator->evaluate(expr);
			std::cout << "  value:      " << std::setprecision(17) << result.num << "\n";
			std::cout << "  binary:     " << result.binary_rep << "\n";
			std::cout << "  components: " << result.components_rep << "\n";
			std::cout << "  type:       " << result.type_name << "\n";
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
		}
		return true;
	}

	// compare <expr>
	if (line.substr(0, 8) == "compare " || line.substr(0, 8) == "compare\t") {
		std::string expr = trim(line.substr(8));
		// Evaluate the expression in each registered type
		std::cout << std::left << std::setw(12) << "Type"
		          << std::right << std::setw(25) << "Value"
		          << "  Binary\n";
		std::cout << std::string(80, '-') << "\n";
		for (const auto& alias : state.registry.aliases()) {
			const TypeOps& ops = state.registry.get(alias);
			ExpressionEvaluator eval(ops);
			// Copy variables
			for (const auto& kv : state.evaluator->variables()) {
				eval.set_variable(kv.first, kv.second);
			}
			try {
				Value result = eval.evaluate(expr);
				std::cout << std::left << std::setw(12) << alias
				          << std::right << std::setw(25) << std::setprecision(17) << result.num
				          << "  " << result.binary_rep << "\n";
			} catch (const std::exception& ex) {
				std::cout << std::left << std::setw(12) << alias
				          << "  Error: " << ex.what() << "\n";
			}
		}
		std::cout << std::endl;
		return true;
	}

	// bits <expr>
	if (line.substr(0, 5) == "bits " || line.substr(0, 5) == "bits\t") {
		std::string expr = trim(line.substr(5));
		try {
			Value result = state.evaluator->evaluate(expr);
			std::cout << "  binary: " << result.binary_rep << "\n";
			// Show hex representation of the double value
			union { double d; uint64_t u; } pun;
			pun.d = result.num;
			std::cout << "  hex:    0x" << std::hex << std::setfill('0')
			          << std::setw(16) << pun.u << std::dec << std::setfill(' ') << "\n";
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
		}
		return true;
	}

	// range -- show dynamic range of current type
	if (line == "range") {
		const TypeOps& ops = state.registry.get(state.active_type);
		try {
			Value maxpos = ops.from_double(std::numeric_limits<double>::max());
			Value minpos = ops.from_double(std::numeric_limits<double>::min());
			Value eps = ops.from_double(1.0);
			// Compute epsilon: smallest value such that 1 + eps != 1
			// Use binary search approach via the type's arithmetic
			Value one = ops.from_double(1.0);
			Value half = ops.from_double(0.5);
			Value candidate = ops.from_double(1.0);
			for (int i = 0; i < 200; ++i) {
				Value next = ops.mul(candidate, half);
				Value test = ops.add(one, next);
				if (test.num <= 1.0) break;
				candidate = next;
			}
			std::cout << "  type:    " << ops.type_tag << "\n";
			std::cout << "  maxpos:  " << std::setprecision(17) << maxpos.num << "\n";
			std::cout << "  minpos:  " << std::setprecision(17) << minpos.num << "\n";
			std::cout << "  epsilon: " << std::setprecision(17) << candidate.num << "\n";
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
		}
		return true;
	}

	// precision -- show effective precision of current type
	if (line == "precision") {
		const TypeOps& ops = state.registry.get(state.active_type);
		try {
			// Compute epsilon as above
			Value one = ops.from_double(1.0);
			Value half = ops.from_double(0.5);
			Value candidate = ops.from_double(1.0);
			int binary_digits = 0;
			for (int i = 0; i < 200; ++i) {
				Value next = ops.mul(candidate, half);
				Value test = ops.add(one, next);
				if (test.num <= 1.0) break;
				candidate = next;
				++binary_digits;
			}
			double decimal_digits = binary_digits * 0.30103; // log10(2)
			std::cout << "  type:           " << ops.type_tag << "\n";
			std::cout << "  binary digits:  " << binary_digits << "\n";
			std::cout << "  decimal digits: " << std::setprecision(1) << std::fixed
			          << decimal_digits << std::defaultfloat << "\n";
			std::cout << "  epsilon:        " << std::setprecision(17) << candidate.num << "\n";
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
		}
		return true;
	}

	// ulp <value>
	if (line.substr(0, 4) == "ulp " || line.substr(0, 4) == "ulp\t") {
		std::string expr = trim(line.substr(4));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);
			Value val = state.evaluator->evaluate(expr);
			// Compute ULP by finding the difference to the next representable value
			// Strategy: add progressively smaller values until the result changes
			double v = val.num;
			double ulp_est = 0.0;
			if (v == 0.0) {
				// ULP at zero: smallest representable positive
				Value tiny = ops.from_double(std::numeric_limits<double>::min());
				ulp_est = tiny.num;
			} else {
				// Binary search for ULP
				double step = std::abs(v);
				for (int i = 0; i < 200; ++i) {
					step *= 0.5;
					Value test = ops.from_double(v + step);
					if (test.num == v) {
						ulp_est = step * 2.0;
						break;
					}
					ulp_est = step;
				}
			}
			std::cout << "  value: " << std::setprecision(17) << v << "\n";
			std::cout << "  ulp:   " << std::setprecision(17) << ulp_est << "\n";
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
		}
		return true;
	}

	// sweep <expr> for x in [a, b, n]
	if (line.substr(0, 6) == "sweep ") {
		// Parse: sweep <expr> for <var> in [<a>, <b>, <n>]
		auto for_pos = line.find(" for ");
		if (for_pos == std::string::npos) {
			std::cerr << "Usage: sweep <expr> for <var> in [<a>, <b>, <n>]\n";
			return true;
		}
		std::string expr = trim(line.substr(6, for_pos - 6));
		std::string rest = line.substr(for_pos + 5);
		auto in_pos = rest.find(" in ");
		if (in_pos == std::string::npos) {
			std::cerr << "Usage: sweep <expr> for <var> in [<a>, <b>, <n>]\n";
			return true;
		}
		std::string var = trim(rest.substr(0, in_pos));
		std::string range_str = trim(rest.substr(in_pos + 4));
		// Parse [a, b, n]
		if (range_str.front() == '[') range_str = range_str.substr(1);
		if (range_str.back() == ']') range_str.pop_back();
		std::istringstream rss(range_str);
		std::string sa, sb, sn;
		std::getline(rss, sa, ',');
		std::getline(rss, sb, ',');
		std::getline(rss, sn, ',');
		try {
			double a = std::stod(trim(sa));
			double b = std::stod(trim(sb));
			int n = std::stoi(trim(sn));
			if (n < 2) n = 2;
			double step = (b - a) / (n - 1);
			const TypeOps& ops = state.registry.get(state.active_type);
			std::cout << std::left << std::setw(20) << var
			          << std::right << std::setw(25) << "result"
			          << std::setw(25) << "double ref"
			          << std::setw(15) << "ULP error" << "\n";
			std::cout << std::string(85, '-') << "\n";
			for (int i = 0; i < n; ++i) {
				double xval = a + i * step;
				// Evaluate in the active type
				ExpressionEvaluator eval(ops);
				eval.set_variable(var, Value(xval));
				// Copy other variables
				for (const auto& kv : state.evaluator->variables()) {
					if (kv.first != var) eval.set_variable(kv.first, kv.second);
				}
				Value result = eval.evaluate(expr);
				// Evaluate reference in double
				ExpressionEvaluator ref_eval(state.registry.get("double"));
				ref_eval.set_variable(var, Value(xval));
				for (const auto& kv : state.evaluator->variables()) {
					if (kv.first != var) ref_eval.set_variable(kv.first, kv.second);
				}
				Value ref = ref_eval.evaluate(expr);
				// Compute ULP error
				double err = 0.0;
				if (ref.num != 0.0) {
					// Simple relative error in ULPs approximation
					err = std::abs(result.num - ref.num);
					// Find ULP at this value
					double v = ref.num;
					double ulp_est = std::abs(v);
					for (int j = 0; j < 200; ++j) {
						ulp_est *= 0.5;
						Value test = ops.from_double(v + ulp_est);
						if (test.num == ops.from_double(v).num) {
							ulp_est *= 2.0;
							break;
						}
					}
					if (ulp_est > 0.0) err /= ulp_est;
				}
				std::cout << std::left << std::setw(20) << std::setprecision(8) << xval
				          << std::right << std::setw(25) << std::setprecision(15) << result.num
				          << std::setw(25) << std::setprecision(15) << ref.num
				          << std::setw(15) << std::setprecision(2) << std::fixed << err
				          << std::defaultfloat << "\n";
			}
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
		}
		std::cout << std::endl;
		return true;
	}

	// faithful <expr> -- check if result is faithfully rounded
	if (line.substr(0, 9) == "faithful ") {
		std::string expr = trim(line.substr(9));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);
			Value result = state.evaluator->evaluate(expr);
			// Compute reference in double (or qd if available)
			const TypeOps* ref_ops = state.registry.find("qd");
			if (!ref_ops) ref_ops = &state.registry.get("double");
			ExpressionEvaluator ref_eval(*ref_ops);
			for (const auto& kv : state.evaluator->variables()) {
				ref_eval.set_variable(kv.first, kv.second);
			}
			Value ref = ref_eval.evaluate(expr);
			// Convert reference to active type
			Value rounded = ops.from_double(ref.num);
			// Check if the result matches either rounding direction
			double diff = std::abs(result.num - ref.num);
			double rdiff = std::abs(rounded.num - ref.num);
			bool is_faithful = (diff <= rdiff * 1.0001) || (result.num == rounded.num);
			std::cout << "  result:    " << std::setprecision(17) << result.num << "\n";
			std::cout << "  reference: " << std::setprecision(17) << ref.num << "\n";
			std::cout << "  rounded:   " << std::setprecision(17) << rounded.num << "\n";
			std::cout << "  faithful:  " << (is_faithful ? "yes" : "no") << "\n";
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
		}
		return true;
	}

	// Otherwise, evaluate as expression
	try {
		Value result = state.evaluator->evaluate(line);
		std::cout << std::setprecision(17) << result.num << "\n";
	} catch (const std::exception& ex) {
		std::cerr << "Error: " << ex.what() << "\n";
	}

	return true;
}

// Readline integration
#ifdef UCALC_USE_READLINE

// Global pointer for completion (readline callbacks need global state)
static ReplState* g_completion_state = nullptr;

static char* ucalc_generator(const char* text, int state_idx) {
	static std::vector<std::string> matches;
	static size_t match_index;

	if (state_idx == 0) {
		matches.clear();
		match_index = 0;
		std::string prefix(text);

		// Complete commands
		static const char* commands[] = {
			"type", "types", "show", "compare", "vars", "help", "quit", "exit", nullptr
		};
		for (int i = 0; commands[i]; ++i) {
			if (std::string(commands[i]).substr(0, prefix.size()) == prefix) {
				matches.push_back(commands[i]);
			}
		}

		// Complete type names
		if (g_completion_state) {
			for (const auto& alias : g_completion_state->registry.aliases()) {
				if (alias.substr(0, prefix.size()) == prefix) {
					matches.push_back(alias);
				}
			}
			// Complete variable names
			for (const auto& kv : g_completion_state->evaluator->variables()) {
				if (kv.first.substr(0, prefix.size()) == prefix) {
					matches.push_back(kv.first);
				}
			}
		}

		// Complete function names
		static const char* functions[] = {
			"sqrt", "abs", "log", "exp", "sin", "cos", "pow", "pi", nullptr
		};
		for (int i = 0; functions[i]; ++i) {
			if (std::string(functions[i]).substr(0, prefix.size()) == prefix) {
				matches.push_back(functions[i]);
			}
		}
	}

	if (match_index < matches.size()) {
		// readline expects malloc'd strings
		return strdup(matches[match_index++].c_str());
	}
	return nullptr;
}

static char** ucalc_completion(const char* text, int /*start*/, int /*end*/) {
	rl_attempted_completion_over = 1;  // don't fall back to filename completion
	return rl_completion_matches(text, ucalc_generator);
}

static std::string get_history_path() {
	const char* home = std::getenv("HOME");
	if (!home) home = ".";
	return std::string(home) + "/.ucalc_history";
}

#endif // UCALC_USE_READLINE

}} // namespace sw::ucalc

int main(int argc, char** argv)
try {
	using namespace sw::ucalc;

	// Build the type registry
	TypeRegistry registry = build_default_registry();

	// Default to double
	std::string active_type = "double";
	const TypeOps& default_ops = registry.get(active_type);
	ExpressionEvaluator evaluator(default_ops);

	bool interactive = ISATTY(FILENO(stdin));

	ReplState state{ registry, &evaluator, active_type, interactive };

	// Handle command-line expression: ucalc "1/3 + 1/3 + 1/3"
	if (argc > 1) {
		std::string expr;
		for (int i = 1; i < argc; ++i) {
			if (i > 1) expr += " ";
			expr += argv[i];
		}
		auto cmds = split_commands(expr);
		for (const auto& cmd : cmds) {
			process_command(cmd, state);
		}
		return EXIT_SUCCESS;
	}

	// REPL loop
	if (interactive) {
		std::cout << "ucalc -- Universal Mixed-Precision REPL Calculator\n";
		std::cout << "Type 'help' for commands, 'quit' to exit.\n";
		std::cout << "Active type: " << state.active_type << "\n\n";
	}

#ifdef UCALC_USE_READLINE
	if (interactive) {
		// Set up readline
		g_completion_state = &state;
		rl_attempted_completion_function = ucalc_completion;
		std::string history_path = get_history_path();
		read_history(history_path.c_str());

		while (true) {
			std::string prompt = state.active_type + "> ";
			char* raw_line = readline(prompt.c_str());
			if (!raw_line) break;  // EOF / Ctrl-D

			std::string line(raw_line);
			free(raw_line);

			std::string trimmed = trim(line);
			if (trimmed.empty()) continue;

			add_history(trimmed.c_str());

			auto cmds = split_commands(line);
			bool should_continue = true;
			for (const auto& cmd : cmds) {
				if (!process_command(cmd, state)) {
					should_continue = false;
					break;
				}
			}
			if (!should_continue) break;
		}

		write_history(history_path.c_str());
		g_completion_state = nullptr;
		return EXIT_SUCCESS;
	}
#endif // UCALC_USE_READLINE

	// Fallback: basic getline loop (non-interactive or no readline)
	std::string line;
	while (true) {
		if (interactive) {
			std::cout << state.active_type << "> ";
			std::cout.flush();
		}
		if (!std::getline(std::cin, line)) break;

		auto cmds = split_commands(line);
		bool should_continue = true;
		for (const auto& cmd : cmds) {
			if (!process_command(cmd, state)) {
				should_continue = false;
				break;
			}
		}
		if (!should_continue) break;
	}

	return EXIT_SUCCESS;
}
catch (const std::exception& ex) {
	std::cerr << "Fatal error: " << ex.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
