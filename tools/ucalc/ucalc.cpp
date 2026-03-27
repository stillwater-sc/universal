// ucalc.cpp: Universal Mixed-Precision REPL Calculator
//
// Interactive calculator for exploring and comparing arithmetic
// across Universal number types. Replaces the workflow of writing,
// compiling, and running C++ for each type comparison.
//
// Usage:
//   ucalc                             # interactive REPL
//   ucalc "type posit32; 1/3"         # evaluate expression
//   ucalc -t posit32 "1/3 + 1/3"     # set type via flag
//   ucalc -f script.ucalc             # batch mode
//   ucalc --json "show 3.14"          # JSON output
//   ucalc --csv "compare 3.14"        # CSV output
//   ucalc --quiet "1/3"               # value only
//   echo "type posit32; 1/3" | ucalc  # pipe mode
//
// Exit codes:
//   0  Success
//   1  Parse error
//   2  Arithmetic error
//   3  Unknown type
//   4  File not found
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
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
#define TAKUM_THROW_ARITHMETIC_EXCEPTION 0
#define DFIXPNT_THROW_ARITHMETIC_EXCEPTION 0
#define HFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define DFLOAT_THROW_ARITHMETIC_EXCEPTION 0
#define RATIONAL_THROW_ARITHMETIC_EXCEPTION 0
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
#include <universal/number/takum/takum.hpp>
#include <universal/number/dfixpnt/dfixpnt.hpp>
#include <universal/number/rational/rational.hpp>
#include <universal/number/hfloat/hfloat.hpp>
#include <universal/number/dfloat/dfloat.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>

// ucalc headers
#include "type_dispatch.hpp"
#include "expression.hpp"
#include "registry.hpp"
#include "output_format.hpp"

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

// Native type helpers, specializations, and registry builder
// are defined in registry.hpp (shared with regression.cpp)

// Exit codes
constexpr int EXIT_OK           = 0;
constexpr int EXIT_PARSE_ERROR  = 1;
constexpr int EXIT_ARITH_ERROR  = 2;
constexpr int EXIT_UNKNOWN_TYPE = 3;
constexpr int EXIT_FILE_NOT_FOUND = 4;

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
static void print_help(OutputFormat fmt) {
	if (fmt == OutputFormat::json) {
		std::cout << "{\"commands\":[\"type\",\"types\",\"show\",\"compare\","
		          << "\"bits\",\"range\",\"precision\",\"ulp\",\"sweep\","
		          << "\"trace\",\"cancel\",\"audit\",\"faithful\",\"color\",\"vars\",\"help\",\"quit\"]}\n";
		return;
	}
	std::cout << "ucalc -- Universal Mixed-Precision REPL Calculator\n\n";
	std::cout << "Commands:\n";
	std::cout << "  type <name>    Set active arithmetic type (e.g., type posit32)\n";
	std::cout << "  types          List all available types\n";
	std::cout << "  show <expr>    Evaluate and display value + binary + components\n";
	std::cout << "  compare <expr> Evaluate across all types in a table\n";
	std::cout << "  bits <expr>    Show raw bit pattern as hex and binary\n";
	std::cout << "  range          Show symmetry range [maxneg ... minneg] 0 [minpos ... maxpos]\n";
	std::cout << "  precision      Show precision, epsilon, and numeric properties\n";
	std::cout << "  ulp <value>    Show ULP at the given value\n";
	std::cout << "  sweep <expr> for <var> in [a, b, n]\n";
	std::cout << "                 Evaluate across a range, show error vs double\n";
	std::cout << "  trace <expr>   Show each operation with ULP error and rounding direction\n";
	std::cout << "  cancel <expr>  Detect catastrophic cancellation in subtractions\n";
	std::cout << "  audit <expr>   Show every rounding event with cumulative error drift\n";
	std::cout << "  faithful <expr> Check if result is faithfully rounded\n";
	std::cout << "  color [on|off] Toggle ANSI color-coded bit fields in show\n";
	std::cout << "  vars           List defined variables\n";
	std::cout << "  help           Show this help\n";
	std::cout << "  quit / exit    Exit the calculator\n";
	std::cout << "\n";
	std::cout << "Expressions:\n";
	std::cout << "  Arithmetic:    +  -  *  /  ^  (parentheses)\n";
	std::cout << "  Functions:     sqrt, abs, log, exp, sin, cos, tan, asin, acos, atan, pow\n";
	std::cout << "  Constants:     phi, e, pi, ln2, ln10, sqrt2, sqrt3, sqrt5 (quad-double precision)\n";
	std::cout << "  Variables:     x = 1/3  (then use x in expressions)\n";
	std::cout << "  Semicolons:    type posit32; 1/3 + 1/3 + 1/3\n";
	std::cout << "\n";
	std::cout << "CLI flags:\n";
	std::cout << "  --json         JSON output for all commands\n";
	std::cout << "  --csv          CSV output for tabular commands\n";
	std::cout << "  --quiet        Value only, no decoration\n";
	std::cout << "  -t <type>      Set active type (e.g., -t posit32)\n";
	std::cout << "  -f <file>      Execute a script file\n";
	std::cout << std::endl;
}

// REPL state
struct ReplState {
	TypeRegistry registry;
	ExpressionEvaluator* evaluator;
	std::string active_type;
	bool interactive;
	bool use_color;     // enable ANSI color_print output
	OutputFormat format;
	int last_error;     // last error exit code (for batch mode error reporting)
};

// Compute ULP at a given double value using the active type.
// Finds the smallest increment that changes the type's representation.
static double compute_ulp(const TypeOps& ops, double v) {
	if (v == 0.0) {
		Value tiny = ops.minpos();
		return tiny.num;
	}
	// Round v to the nearest representable value in the active type first,
	// then probe for the ULP at that representable value.
	double base = ops.from_double(v).num;
	double step = std::max(std::abs(base), std::numeric_limits<double>::min());
	double ulp_est = step;
	for (int i = 0; i < 200; ++i) {
		step *= 0.5;
		if (step == 0.0) break;
		Value test = ops.from_double(base + step);
		if (test.num == base) {
			ulp_est = step * 2.0;
			break;
		}
		ulp_est = step;
	}
	return ulp_est;
}

// Process a single command or expression
// Returns false if the REPL should exit
static bool process_command(const std::string& input, ReplState& state) {
	std::string line = trim(input);
	if (line.empty()) return true;

	// Skip comments in batch mode
	if (line[0] == '#') return true;

	OutputFormat fmt = state.format;

	// Check for commands
	if (line == "quit" || line == "exit") {
		return false;
	}

	if (line == "help") {
		print_help(fmt);
		return true;
	}

	// color on/off
	if (line == "color on") {
		state.use_color = true;
		if (state.interactive && fmt == OutputFormat::plain)
			std::cout << "Color output enabled.\n";
		return true;
	}
	if (line == "color off") {
		state.use_color = false;
		if (state.interactive && fmt == OutputFormat::plain)
			std::cout << "Color output disabled.\n";
		return true;
	}
	if (line == "color") {
		if (fmt == OutputFormat::json) {
			std::cout << "{\"color\":" << (state.use_color ? "true" : "false") << "}\n";
		} else {
			std::cout << "Color output: " << (state.use_color ? "on" : "off") << "\n";
		}
		return true;
	}

	if (line == "types") {
		if (fmt == OutputFormat::json) {
			std::cout << "[";
			bool first = true;
			for (const auto& alias : state.registry.aliases()) {
				const TypeOps* ops = state.registry.find(alias);
				if (!first) std::cout << ",";
				first = false;
				std::cout << "{\"alias\":\"" << json_escape(alias) << "\""
				          << ",\"type_tag\":\"" << json_escape(ops->type_tag) << "\""
				          << ",\"nbits\":" << ops->nbits
				          << ",\"active\":" << (alias == state.active_type ? "true" : "false")
				          << "}";
			}
			std::cout << "]\n";
		} else if (fmt == OutputFormat::csv) {
			std::cout << "alias,type_tag,nbits,active\n";
			for (const auto& alias : state.registry.aliases()) {
				const TypeOps* ops = state.registry.find(alias);
				std::cout << csv_quote(alias) << ","
				          << csv_quote(ops->type_tag) << ","
				          << ops->nbits << ","
				          << (alias == state.active_type ? "true" : "false") << "\n";
			}
		} else {
			std::cout << "Available types:\n";
			for (const auto& alias : state.registry.aliases()) {
				const TypeOps* ops = state.registry.find(alias);
				std::string marker = (alias == state.active_type) ? " *" : "";
				std::cout << "  " << std::left << std::setw(12) << alias
				          << ops->type_tag << marker << "\n";
			}
			std::cout << std::endl;
		}
		return true;
	}

	if (line == "vars") {
		const auto& vars = state.evaluator->variables();
		if (fmt == OutputFormat::json) {
			std::cout << "{";
			bool first = true;
			for (const auto& kv : vars) {
				if (!first) std::cout << ",";
				first = false;
				std::cout << "\"" << json_escape(kv.first) << "\":"
				          << "{\"value\":\"" << json_escape(kv.second.native_rep) << "\""
				          << ",\"decimal\":" << json_number(kv.second.num)
				          << ",\"type\":\"" << json_escape(kv.second.type_name) << "\""
				          << "}";
			}
			std::cout << "}\n";
		} else if (fmt == OutputFormat::csv) {
			std::cout << "name,value,decimal,type\n";
			for (const auto& kv : vars) {
				std::cout << csv_quote(kv.first) << ","
				          << csv_quote(kv.second.native_rep) << ","
				          << std::setprecision(17) << kv.second.num << ","
				          << csv_quote(kv.second.type_name) << "\n";
			}
		} else {
			if (vars.empty()) {
				std::cout << "No variables defined.\n";
			} else {
				auto find_alias = [&](const std::string& type_name) -> std::string {
					for (const auto& alias : state.registry.aliases()) {
						const TypeOps* ops = state.registry.find(alias);
						if (ops && (ops->type_tag == type_name || ops->name == type_name))
							return alias;
					}
					return type_name;
				};
				for (const auto& kv : vars) {
					std::string alias = find_alias(kv.second.type_name);
					std::cout << "  " << std::left << std::setw(12) << alias
					          << std::setw(10) << kv.first
					          << " = " << kv.second.native_rep << "\n";
				}
			}
			std::cout << std::endl;
		}
		return true;
	}

	// type <name>
	if (line.substr(0, 5) == "type " || line.substr(0, 5) == "type\t") {
		std::string type_name = trim(line.substr(5));
		const TypeOps* ops = state.registry.find(type_name);
		if (!ops) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"unknown type\",\"type\":\""
				          << json_escape(type_name) << "\"}\n";
			} else {
				std::cerr << "Error: unknown type '" << type_name
				          << "'. Use 'types' to list available types.\n";
			}
			state.last_error = EXIT_UNKNOWN_TYPE;
			return true;
		}
		state.active_type = type_name;
		state.evaluator->set_type(*ops);
		if (fmt == OutputFormat::json) {
			std::cout << "{\"active_type\":\"" << json_escape(type_name) << "\""
			          << ",\"type_tag\":\"" << json_escape(ops->type_tag) << "\"}\n";
		} else if (state.interactive && fmt == OutputFormat::plain) {
			std::cout << "Active type: " << type_name << " (" << ops->type_tag << ")\n";
		}
		return true;
	}

	// show <expr>
	if (line.substr(0, 5) == "show " || line.substr(0, 5) == "show\t") {
		std::string expr = trim(line.substr(5));
		try {
			Value result = state.evaluator->evaluate(expr);
			if (fmt == OutputFormat::json) {
				std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"type\":\"" << json_escape(result.type_name) << "\""
				          << ",\"value\":\"" << json_escape(result.native_rep) << "\""
				          << ",\"decimal\":" << json_number(result.num)
				          << ",\"binary\":\"" << json_escape(result.binary_rep) << "\""
				          << ",\"components\":\"" << json_escape(result.components_rep) << "\""
				          << "}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "expression,type,value,decimal,binary,components\n";
				std::cout << csv_quote(expr) << ","
				          << csv_quote(result.type_name) << ","
				          << csv_quote(result.native_rep) << ","
				          << std::setprecision(17) << result.num << ","
				          << csv_quote(result.binary_rep) << ","
				          << csv_quote(result.components_rep) << "\n";
			} else if (fmt == OutputFormat::quiet) {
				std::cout << result.native_rep << "\n";
			} else {
				std::cout << "  value:      " << result.native_rep << "\n";
				if (state.use_color && !result.color_rep.empty()) {
					std::cout << "  color:      " << result.color_rep << "\n";
				} else {
					std::cout << "  binary:     " << result.binary_rep << "\n";
				}
				std::cout << "  components: " << result.components_rep << "\n";
				std::cout << "  type:       " << result.type_name << "\n";
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_PARSE_ERROR;
		}
		return true;
	}

	// compare <expr>
	if (line.substr(0, 8) == "compare " || line.substr(0, 8) == "compare\t") {
		std::string expr = trim(line.substr(8));
		struct CompareEntry {
			std::string alias;
			std::string value;
			std::string binary;
			double decimal;
			std::string error;
			int nbits;
		};
		std::vector<CompareEntry> all_entries;
		for (const auto& alias : state.registry.aliases()) {
			const TypeOps& ops = state.registry.get(alias);
			ExpressionEvaluator eval(ops);
			for (const auto& kv : state.evaluator->variables()) {
				eval.set_variable(kv.first, kv.second);
			}
			CompareEntry entry;
			entry.alias = alias;
			entry.nbits = ops.nbits;
			entry.decimal = 0.0;
			try {
				Value result = eval.evaluate(expr);
				entry.value = result.native_rep;
				entry.binary = result.binary_rep;
				entry.decimal = result.num;
			} catch (const std::exception& ex) {
				entry.error = ex.what();
			}
			all_entries.push_back(std::move(entry));
		}

		if (fmt == OutputFormat::json) {
			std::cout << "[";
			bool first = true;
			for (const auto& e : all_entries) {
				if (!first) std::cout << ",";
				first = false;
				if (!e.error.empty()) {
					std::cout << "{\"type\":\"" << json_escape(e.alias) << "\""
					          << ",\"error\":\"" << json_escape(e.error) << "\""
					          << ",\"nbits\":" << e.nbits << "}";
				} else {
					std::cout << "{\"type\":\"" << json_escape(e.alias) << "\""
					          << ",\"value\":\"" << json_escape(e.value) << "\""
					          << ",\"decimal\":" << json_number(e.decimal)
					          << ",\"binary\":\"" << json_escape(e.binary) << "\""
					          << ",\"nbits\":" << e.nbits << "}";
				}
			}
			std::cout << "]\n";
		} else if (fmt == OutputFormat::csv) {
			std::cout << "type,value,decimal,binary,nbits,error\n";
			for (const auto& e : all_entries) {
				std::cout << csv_quote(e.alias) << ","
				          << csv_quote(e.value) << ","
				          << std::setprecision(17) << e.decimal << ","
				          << csv_quote(e.binary) << ","
				          << e.nbits << ","
				          << csv_quote(e.error) << "\n";
			}
		} else {
			// Group by bit width for plain output
			std::vector<CompareEntry> small, medium, large;
			for (auto& e : all_entries) {
				int vlen = static_cast<int>(e.value.size());
				if (e.nbits > 80 || vlen > 25)              large.push_back(std::move(e));
				else if (e.nbits > 32 || vlen > 22)         medium.push_back(std::move(e));
				else                                         small.push_back(std::move(e));
			}
			auto print_single_line = [](const std::vector<CompareEntry>& entries, int vw) {
				for (const auto& e : entries) {
					if (!e.error.empty()) {
						std::cout << std::left << std::setw(12) << e.alias
						          << "  Error: " << e.error << "\n";
					} else {
						std::cout << std::left << std::setw(12) << e.alias
						          << std::right << std::setw(vw) << e.value
						          << "  " << e.binary << "\n";
					}
				}
			};
			auto print_two_line = [](const std::vector<CompareEntry>& entries) {
				for (const auto& e : entries) {
					if (!e.error.empty()) {
						std::cout << std::left << std::setw(12) << e.alias
						          << "  Error: " << e.error << "\n";
					} else {
						std::cout << std::left << std::setw(12) << e.alias
						          << e.value << "\n"
						          << std::string(12, ' ') << e.binary << "\n";
					}
				}
			};
			if (!small.empty()) {
				std::cout << std::left << std::setw(12) << "Type"
				          << std::right << std::setw(22) << "Value"
				          << "  Binary\n";
				std::cout << std::string(70, '-') << "\n";
				print_single_line(small, 22);
			}
			if (!medium.empty()) {
				std::cout << "\n";
				std::cout << std::left << std::setw(12) << "Type"
				          << std::right << std::setw(25) << "Value"
				          << "  Binary\n";
				std::cout << std::string(80, '-') << "\n";
				print_single_line(medium, 25);
			}
			if (!large.empty()) {
				std::cout << "\n";
				std::cout << std::left << std::setw(12) << "Type"
				          << "Value / Binary\n";
				std::cout << std::string(80, '-') << "\n";
				print_two_line(large);
			}
			std::cout << std::endl;
		}
		return true;
	}

	// bits <expr>
	if (line.substr(0, 5) == "bits " || line.substr(0, 5) == "bits\t") {
		std::string expr = trim(line.substr(5));
		try {
			Value result = state.evaluator->evaluate(expr);
			if (fmt == OutputFormat::json) {
				std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"type\":\"" << json_escape(result.type_name) << "\""
				          << ",\"binary\":\"" << json_escape(result.binary_rep) << "\""
				          << ",\"value\":\"" << json_escape(result.native_rep) << "\""
				          << "}\n";
			} else if (fmt == OutputFormat::quiet) {
				std::cout << result.binary_rep << "\n";
			} else {
				std::cout << "  type:   " << result.type_name << "\n";
				std::cout << "  binary: " << result.binary_rep << "\n";
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_PARSE_ERROR;
		}
		return true;
	}

	// range -- show symmetry range [maxneg ... minneg] 0 [minpos ... maxpos]
	if (line == "range") {
		const TypeOps& ops = state.registry.get(state.active_type);
		try {
			Value vMaxneg = ops.maxneg();
			Value vMinneg = ops.minneg();
			Value vMinpos = ops.minpos();
			Value vMaxpos = ops.maxpos();
			if (fmt == OutputFormat::json) {
				std::cout << "{\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"maxneg\":\"" << json_escape(vMaxneg.native_rep) << "\""
				          << ",\"minneg\":\"" << json_escape(vMinneg.native_rep) << "\""
				          << ",\"minpos\":\"" << json_escape(vMinpos.native_rep) << "\""
				          << ",\"maxpos\":\"" << json_escape(vMaxpos.native_rep) << "\""
				          << ",\"maxneg_decimal\":" << json_number(vMaxneg.num)
				          << ",\"minneg_decimal\":" << json_number(vMinneg.num)
				          << ",\"minpos_decimal\":" << json_number(vMinpos.num)
				          << ",\"maxpos_decimal\":" << json_number(vMaxpos.num)
				          << "}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "property,value,decimal,binary\n";
				std::cout << "maxneg," << csv_quote(vMaxneg.native_rep) << "," << std::setprecision(17) << vMaxneg.num << "," << csv_quote(vMaxneg.binary_rep) << "\n";
				std::cout << "minneg," << csv_quote(vMinneg.native_rep) << "," << std::setprecision(17) << vMinneg.num << "," << csv_quote(vMinneg.binary_rep) << "\n";
				std::cout << "minpos," << csv_quote(vMinpos.native_rep) << "," << std::setprecision(17) << vMinpos.num << "," << csv_quote(vMinpos.binary_rep) << "\n";
				std::cout << "maxpos," << csv_quote(vMaxpos.native_rep) << "," << std::setprecision(17) << vMaxpos.num << "," << csv_quote(vMaxpos.binary_rep) << "\n";
			} else {
				std::cout << ops.type_tag << "\n";
				std::cout << "[ " << vMaxneg.native_rep << " ... " << vMinneg.native_rep
				          << "  0  "
				          << vMinpos.native_rep << " ... " << vMaxpos.native_rep << " ]\n";
				std::cout << "[ " << vMaxneg.binary_rep << " ... " << vMinneg.binary_rep
				          << "  0  "
				          << vMinpos.binary_rep << " ... " << vMaxpos.binary_rep << " ]\n";
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_ARITH_ERROR;
		}
		return true;
	}

	// precision -- show effective precision and numeric properties
	if (line == "precision") {
		const TypeOps& ops = state.registry.get(state.active_type);
		try {
			Value vMaxpos = ops.maxpos();
			Value vMinpos = ops.minpos();
			Value vEps    = ops.epsilon();
			double eps = vEps.num;
			int binary_digits = 0;
			if (eps > 0.0 && eps < 1.0) {
				binary_digits = static_cast<int>(-std::log2(eps));
			}
			double decimal_digits = binary_digits * 0.30103; // log10(2)
			if (fmt == OutputFormat::json) {
				std::cout << "{\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"binary_digits\":" << binary_digits
				          << ",\"decimal_digits\":" << std::setprecision(1) << std::fixed << decimal_digits << std::defaultfloat
				          << ",\"epsilon\":\"" << json_escape(vEps.native_rep) << "\""
				          << ",\"epsilon_decimal\":" << json_number(vEps.num)
				          << ",\"minpos\":\"" << json_escape(vMinpos.native_rep) << "\""
				          << ",\"maxpos\":\"" << json_escape(vMaxpos.native_rep) << "\""
				          << "}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "property,value\n";
				std::cout << "type," << csv_quote(ops.type_tag) << "\n";
				std::cout << "binary_digits," << binary_digits << "\n";
				std::cout << "decimal_digits," << std::setprecision(1) << std::fixed << decimal_digits << std::defaultfloat << "\n";
				std::cout << "epsilon," << csv_quote(vEps.native_rep) << "\n";
				std::cout << "minpos," << csv_quote(vMinpos.native_rep) << "\n";
				std::cout << "maxpos," << csv_quote(vMaxpos.native_rep) << "\n";
			} else {
				std::cout << "  type:           " << ops.type_tag << "\n";
				std::cout << "  binary digits:  " << binary_digits << "\n";
				std::cout << "  decimal digits: " << std::setprecision(1) << std::fixed
				          << decimal_digits << std::defaultfloat << "\n";
				std::cout << "  epsilon:        " << vEps.native_rep << "\n";
				std::cout << "  minpos:         " << vMinpos.native_rep << "\n";
				std::cout << "  maxpos:         " << vMaxpos.native_rep << "\n";
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_ARITH_ERROR;
		}
		return true;
	}

	// ulp <value>
	if (line.substr(0, 4) == "ulp " || line.substr(0, 4) == "ulp\t") {
		std::string expr = trim(line.substr(4));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);
			Value val = state.evaluator->evaluate(expr);
			double ulp_est = compute_ulp(ops, val.num);
			int prec = std::min(ops.max_digits10 > 0 ? ops.max_digits10 : 17,
			                    std::numeric_limits<double>::max_digits10);
			if (fmt == OutputFormat::json) {
				std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"value\":\"" << json_escape(val.native_rep) << "\""
				          << ",\"ulp\":" << std::setprecision(prec) << ulp_est
				          << "}\n";
			} else if (fmt == OutputFormat::quiet) {
				std::cout << std::setprecision(prec) << ulp_est << "\n";
			} else {
				std::cout << "  value: " << val.native_rep << "\n";
				std::cout << "  ulp:   " << std::setprecision(prec) << ulp_est << "\n";
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_PARSE_ERROR;
		}
		return true;
	}

	// sweep <expr> for x in [a, b, n]
	if (line.substr(0, 6) == "sweep ") {
		auto for_pos = line.find(" for ");
		if (for_pos == std::string::npos) {
			std::cerr << "Usage: sweep <expr> for <var> in [<a>, <b>, <n>]\n";
			state.last_error = EXIT_PARSE_ERROR;
			return true;
		}
		std::string expr = trim(line.substr(6, for_pos - 6));
		std::string rest = line.substr(for_pos + 5);
		auto in_pos = rest.find(" in ");
		if (in_pos == std::string::npos) {
			std::cerr << "Usage: sweep <expr> for <var> in [<a>, <b>, <n>]\n";
			state.last_error = EXIT_PARSE_ERROR;
			return true;
		}
		std::string var = trim(rest.substr(0, in_pos));
		std::string range_str = trim(rest.substr(in_pos + 4));
		if (range_str.empty()) {
			std::cerr << "Usage: sweep <expr> for <var> in [<a>, <b>, <n>]\n";
			state.last_error = EXIT_PARSE_ERROR;
			return true;
		}
		if (range_str.front() == '[') range_str = range_str.substr(1);
		if (!range_str.empty() && range_str.back() == ']') range_str.pop_back();
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
			double step_size = (b - a) / (n - 1);
			const TypeOps& ops = state.registry.get(state.active_type);

			if (fmt == OutputFormat::json) {
				std::cout << "[";
			} else if (fmt == OutputFormat::csv) {
				std::cout << var << ",result,double_ref,ulp_error\n";
			} else if (fmt == OutputFormat::plain) {
				std::cout << std::left << std::setw(20) << var
				          << std::right << std::setw(25) << "result"
				          << std::setw(25) << "double ref"
				          << std::setw(15) << "ULP error" << "\n";
				std::cout << std::string(85, '-') << "\n";
			}

			for (int i = 0; i < n; ++i) {
				double xval = a + i * step_size;
				ExpressionEvaluator eval(ops);
				eval.set_variable(var, Value(xval));
				for (const auto& kv : state.evaluator->variables()) {
					if (kv.first != var) eval.set_variable(kv.first, kv.second);
				}
				Value result = eval.evaluate(expr);
				ExpressionEvaluator ref_eval(state.registry.get("double"));
				ref_eval.set_variable(var, Value(xval));
				for (const auto& kv : state.evaluator->variables()) {
					if (kv.first != var) ref_eval.set_variable(kv.first, kv.second);
				}
				Value ref = ref_eval.evaluate(expr);
				double err = 0.0;
				if (ref.num != 0.0) {
					err = std::abs(result.num - ref.num);
					double ulp_est = compute_ulp(ops, ref.num);
					if (ulp_est > 0.0) err /= ulp_est;
				}

				if (fmt == OutputFormat::json) {
					if (i > 0) std::cout << ",";
					std::cout << "{\"" << json_escape(var) << "\":" << json_number(xval)
					          << ",\"result\":\"" << json_escape(result.native_rep) << "\""
					          << ",\"result_decimal\":" << json_number(result.num)
					          << ",\"double_ref\":" << json_number(ref.num)
					          << ",\"ulp_error\":" << json_number(err)
					          << "}";
				} else if (fmt == OutputFormat::csv) {
					std::cout << std::setprecision(17) << xval << ","
					          << csv_quote(result.native_rep) << ","
					          << std::setprecision(17) << ref.num << ","
					          << std::setprecision(6) << err << "\n";
				} else if (fmt == OutputFormat::quiet) {
					std::cout << std::setprecision(8) << xval << " "
					          << result.native_rep << " "
					          << std::setprecision(2) << std::fixed << err
					          << std::defaultfloat << "\n";
				} else {
					auto cap = [](const std::string& s, int w) -> std::string {
						return (static_cast<int>(s.size()) > w) ? s.substr(0, w - 1) + "~" : s;
					};
					std::cout << std::left << std::setw(20) << std::setprecision(8) << xval
					          << std::right << std::setw(25) << cap(result.native_rep, 25)
					          << std::setw(25) << cap(ref.native_rep, 25)
					          << std::setw(15) << std::setprecision(2) << std::fixed << err
					          << std::defaultfloat << "\n";
				}
			}
			if (fmt == OutputFormat::json) {
				std::cout << "]\n";
			} else if (fmt == OutputFormat::plain) {
				std::cout << std::endl;
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_PARSE_ERROR;
		}
		return true;
	}

	// trace <expr> -- show each operation with ULP error and rounding direction
	if (line.substr(0, 6) == "trace " || line.substr(0, 6) == "trace\t") {
		std::string expr = trim(line.substr(6));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);

			// Evaluate in the active type with tracing enabled
			ExpressionEvaluator eval(ops);
			for (const auto& kv : state.evaluator->variables()) {
				eval.set_variable(kv.first, kv.second);
			}
			eval.enable_trace(true);
			Value result = eval.evaluate(expr);
			const auto& steps = eval.trace_steps();

			// Compute reference for each step using qd
			const TypeOps* ref_ops = state.registry.find("qd");
			if (!ref_ops) ref_ops = &state.registry.get("double");

			// Build reference results: replay each operation in qd
			struct TraceAnnotation {
				double exact;           // qd reference result (as double)
				std::string exact_rep;  // qd reference native_rep
				double ulp_error;       // |result - exact| / ulp
				std::string rounding;   // "exact", "up", "down"
				bool cancellation;      // catastrophic cancellation detected
				int cancelled_digits;   // significant digits lost
			};
			std::vector<TraceAnnotation> annotations;

			for (const auto& s : steps) {
				TraceAnnotation ann;
				ann.cancellation = false;
				ann.cancelled_digits = 0;
				ann.ulp_error = 0.0;
				ann.rounding = "exact";

				// Compute exact result in qd
				double exact_d = 0.0;
				std::string exact_rep;
				try {
					if (s.operation == "add") {
						Value a = ref_ops->from_double(s.operand_a);
						Value b = ref_ops->from_double(s.operand_b);
						Value r = ref_ops->add(a, b);
						exact_d = r.num; exact_rep = r.native_rep;
					} else if (s.operation == "sub") {
						Value a = ref_ops->from_double(s.operand_a);
						Value b = ref_ops->from_double(s.operand_b);
						Value r = ref_ops->sub(a, b);
						exact_d = r.num; exact_rep = r.native_rep;
					} else if (s.operation == "mul") {
						Value a = ref_ops->from_double(s.operand_a);
						Value b = ref_ops->from_double(s.operand_b);
						Value r = ref_ops->mul(a, b);
						exact_d = r.num; exact_rep = r.native_rep;
					} else if (s.operation == "div") {
						Value a = ref_ops->from_double(s.operand_a);
						Value b = ref_ops->from_double(s.operand_b);
						Value r = ref_ops->div(a, b);
						exact_d = r.num; exact_rep = r.native_rep;
					} else if (s.operation == "negate") {
						Value a = ref_ops->from_double(s.operand_a);
						Value r = ref_ops->negate(a);
						exact_d = r.num; exact_rep = r.native_rep;
					} else if (s.operation == "pow") {
						Value a = ref_ops->from_double(s.operand_a);
						Value b = ref_ops->from_double(s.operand_b);
						Value r = ref_ops->fn_pow(a, b);
						exact_d = r.num; exact_rep = r.native_rep;
					} else {
						// Unary math functions
						Value a = ref_ops->from_double(s.operand_a);
						Value r;
						if (s.operation == "sqrt") r = ref_ops->fn_sqrt(a);
						else if (s.operation == "abs") r = ref_ops->fn_abs(a);
						else if (s.operation == "log") r = ref_ops->fn_log(a);
						else if (s.operation == "exp") r = ref_ops->fn_exp(a);
						else if (s.operation == "sin") r = ref_ops->fn_sin(a);
						else if (s.operation == "cos") r = ref_ops->fn_cos(a);
						else if (s.operation == "tan") r = ref_ops->fn_tan(a);
						else if (s.operation == "asin") r = ref_ops->fn_asin(a);
						else if (s.operation == "acos") r = ref_ops->fn_acos(a);
						else if (s.operation == "atan") r = ref_ops->fn_atan(a);
						else r = ref_ops->from_double(s.result);
						exact_d = r.num; exact_rep = r.native_rep;
					}
				} catch (...) {
					exact_d = s.result; exact_rep = s.result_rep;
				}
				ann.exact = exact_d;
				ann.exact_rep = exact_rep;

				// Compute ULP error
				if (exact_d != 0.0 && std::isfinite(exact_d) && std::isfinite(s.result)) {
					double abs_err = std::abs(s.result - exact_d);
					double ulp = compute_ulp(ops, exact_d);
					if (ulp > 0.0) ann.ulp_error = abs_err / ulp;
				}

				// Determine rounding direction
				if (s.result == exact_d) {
					ann.rounding = "exact";
				} else if (s.result > exact_d) {
					ann.rounding = "up";
				} else {
					ann.rounding = "down";
				}

				// Detect catastrophic cancellation for subtraction
				if (s.operation == "sub" && s.operand_a != 0.0 && s.operand_b != 0.0) {
					double mag = std::max(std::abs(s.operand_a), std::abs(s.operand_b));
					double res_mag = std::abs(exact_d);
					if (mag > 0.0 && res_mag > 0.0) {
						double ratio = res_mag / mag;
						if (ratio < 1e-3) {
							ann.cancellation = true;
							ann.cancelled_digits = static_cast<int>(-std::log10(ratio));
						}
					}
				}

				annotations.push_back(std::move(ann));
			}

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"result\":\"" << json_escape(result.native_rep) << "\""
				          << ",\"result_decimal\":" << json_number(result.num)
				          << ",\"steps\":[";
				for (size_t i = 0; i < steps.size(); ++i) {
					const auto& s = steps[i];
					const auto& a = annotations[i];
					if (i > 0) std::cout << ",";
					std::cout << "{\"step\":" << s.step_number
					          << ",\"operation\":\"" << json_escape(s.operation) << "\""
					          << ",\"description\":\"" << json_escape(s.description) << "\""
					          << ",\"result\":\"" << json_escape(s.result_rep) << "\""
					          << ",\"result_decimal\":" << json_number(s.result)
					          << ",\"reference\":\"" << json_escape(a.exact_rep) << "\""
					          << ",\"reference_decimal\":" << json_number(a.exact)
					          << ",\"ulp_error\":" << json_number(a.ulp_error)
					          << ",\"rounding\":\"" << a.rounding << "\"";
					if (a.cancellation) {
						std::cout << ",\"cancellation\":true"
						          << ",\"cancelled_digits\":" << a.cancelled_digits;
					}
					std::cout << "}";
				}
				std::cout << "]}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "step,operation,description,result,reference,ulp_error,rounding,cancellation\n";
				for (size_t i = 0; i < steps.size(); ++i) {
					const auto& s = steps[i];
					const auto& a = annotations[i];
					std::cout << s.step_number << ","
					          << csv_quote(s.operation) << ","
					          << csv_quote(s.description) << ","
					          << csv_quote(s.result_rep) << ","
					          << csv_quote(a.exact_rep) << ","
					          << std::setprecision(4) << a.ulp_error << ","
					          << a.rounding << ","
					          << (a.cancellation ? "yes" : "no") << "\n";
				}
			} else if (fmt == OutputFormat::quiet) {
				for (size_t i = 0; i < steps.size(); ++i) {
					const auto& s = steps[i];
					const auto& a = annotations[i];
					std::cout << s.step_number << " " << s.operation << " "
					          << s.result_rep << " " << std::setprecision(2) << std::fixed
					          << a.ulp_error << std::defaultfloat << " " << a.rounding;
					if (a.cancellation) std::cout << " CANCEL";
					std::cout << "\n";
				}
				std::cout << "= " << result.native_rep << "\n";
			} else {
				// Plain text output
				// Show result and reference on separate lines so the rounding
				// direction is self-evident from comparing the two decimal values.
				for (size_t i = 0; i < steps.size(); ++i) {
					const auto& s = steps[i];
					const auto& a = annotations[i];
					std::cout << "  step " << s.step_number << ": " << s.description << "\n";
					if (a.rounding == "exact") {
						std::cout << "          = " << s.result_rep << "  (exact)\n";
					} else {
						std::cout << "          result:    " << std::setprecision(17) << s.result << "\n";
						std::cout << "          reference: " << std::setprecision(17) << a.exact << "\n";
						std::cout << "          ";
						if (a.rounding == "up") std::cout << "ROUNDED UP";
						else                    std::cout << "ROUNDED DOWN";
						std::cout << "  " << std::setprecision(2) << std::fixed
						          << a.ulp_error << std::defaultfloat << " ULP\n";
					}
					if (a.cancellation) {
						std::cout << "          WARNING: catastrophic cancellation (~"
						          << a.cancelled_digits << " digits lost)\n";
					}
				}
				std::cout << "  result: " << result.native_rep << "\n";
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_PARSE_ERROR;
		}
		return true;
	}

	// cancel <expr> -- detect catastrophic cancellation in subtractions
	if (line.substr(0, 7) == "cancel " || line.substr(0, 7) == "cancel\t") {
		std::string expr = trim(line.substr(7));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);

			// Evaluate with tracing to capture all subtraction steps
			ExpressionEvaluator eval(ops);
			for (const auto& kv : state.evaluator->variables()) {
				eval.set_variable(kv.first, kv.second);
			}
			eval.enable_trace(true);
			Value result = eval.evaluate(expr);
			const auto& steps = eval.trace_steps();

			// Compute type's effective significant decimal digits.
			// For types with epsilon >= 1 (integers, exact types), subtraction
			// is exact so cancellation analysis doesn't apply -- set type_digits
			// to 0 so all subtractions classify as "none".
			Value vEps = ops.epsilon();
			double eps = vEps.num;
			double type_digits = 0.0;
			if (eps > 0.0 && eps < 1.0) {
				type_digits = -std::log2(eps) * 0.30103; // log10(2)
			}

			// Analyze each subtraction
			struct CancelInfo {
				int step_number;
				std::string description;
				double operand_a;
				double operand_b;
				std::string operand_a_rep;   // lossless native representation
				std::string operand_b_rep;   // lossless native representation
				std::string result_rep;
				double result_val;
				double exact_result;        // qd reference
				std::string exact_rep;
				double shared_digits;       // leading decimal digits shared
				double result_digits;       // significant digits remaining
				std::string severity;       // "none", "mild", "severe", "catastrophic"
				std::string suggestion;     // reformulation hint (empty if none)
			};
			std::vector<CancelInfo> cancellations;

			const TypeOps* ref_ops = state.registry.find("qd");
			if (!ref_ops) ref_ops = &state.registry.get("double");

			for (const auto& s : steps) {
				if (s.operation != "sub") continue;

				CancelInfo ci;
				ci.step_number = s.step_number;
				ci.description = s.description;
				ci.operand_a = s.operand_a;
				ci.operand_b = s.operand_b;
				ci.operand_a_rep = s.operand_a_rep;
				ci.operand_b_rep = s.operand_b_rep;
				ci.result_rep = s.result_rep;
				ci.result_val = s.result;

				// Compute exact result in qd using lossless native representations
				// when available, falling back to double conversion
				try {
					Value a, b;
					if (!s.operand_a_rep.empty()) {
						// Parse from the lossless native representation
						ExpressionEvaluator ref_eval(*ref_ops);
						a = ref_eval.evaluate(s.operand_a_rep);
					} else {
						a = ref_ops->from_double(s.operand_a);
					}
					if (!s.operand_b_rep.empty()) {
						ExpressionEvaluator ref_eval(*ref_ops);
						b = ref_eval.evaluate(s.operand_b_rep);
					} else {
						b = ref_ops->from_double(s.operand_b);
					}
					Value r = ref_ops->sub(a, b);
					ci.exact_result = r.num;
					ci.exact_rep = r.native_rep;
				} catch (...) {
					ci.exact_result = s.result;
					ci.exact_rep = s.result_rep;
				}

				// Compute shared leading digits and remaining digits
				// Clamp to [0, type_digits] to handle anti-cancellation (e.g. 1 - (-1))
				double mag = std::max(std::abs(s.operand_a), std::abs(s.operand_b));
				double res_mag = std::abs(ci.exact_result);
				if (mag > 0.0 && res_mag > 0.0) {
					double ratio = std::min(res_mag / mag, 1.0); // clamp ratio <= 1
					ci.shared_digits = std::max(0.0, -std::log10(ratio));
					ci.shared_digits = std::min(ci.shared_digits, type_digits);
					ci.result_digits = std::max(0.0, type_digits - ci.shared_digits);
				} else if (mag > 0.0 && res_mag == 0.0) {
					ci.shared_digits = type_digits;
					ci.result_digits = 0.0;
				} else {
					ci.shared_digits = 0.0;
					ci.result_digits = type_digits;
				}

				// Classify severity
				if (ci.shared_digits < 1.0)       ci.severity = "none";
				else if (ci.shared_digits < 3.0)  ci.severity = "mild";
				else if (ci.shared_digits < 6.0)  ci.severity = "severe";
				else                               ci.severity = "catastrophic";

				// No auto-suggestions: pattern-matching on syntax cannot reliably
				// determine whether a reformulation would help. The cancellation
				// diagnostic (shared/result digits, severity) is the actionable
				// output -- the user or agent must analyze whether the cancelling
				// operands are exact inputs or computed intermediates to decide
				// the appropriate fix.

				cancellations.push_back(std::move(ci));
			}

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"type_digits\":" << std::setprecision(1) << std::fixed << type_digits << std::defaultfloat
				          << ",\"result\":\"" << json_escape(result.native_rep) << "\""
				          << ",\"result_decimal\":" << json_number(result.num)
				          << ",\"subtractions\":[";
				for (size_t i = 0; i < cancellations.size(); ++i) {
					const auto& ci = cancellations[i];
					if (i > 0) std::cout << ",";
					std::cout << "{\"step\":" << ci.step_number
					          << ",\"description\":\"" << json_escape(ci.description) << "\""
					          << ",\"operand_a\":\"" << json_escape(ci.operand_a_rep) << "\""
					          << ",\"operand_a_decimal\":" << json_number(ci.operand_a)
					          << ",\"operand_b\":\"" << json_escape(ci.operand_b_rep) << "\""
					          << ",\"operand_b_decimal\":" << json_number(ci.operand_b)
					          << ",\"result\":\"" << json_escape(ci.result_rep) << "\""
					          << ",\"reference\":\"" << json_escape(ci.exact_rep) << "\""
					          << ",\"shared_digits\":" << std::setprecision(1) << std::fixed << ci.shared_digits << std::defaultfloat
					          << ",\"result_digits\":" << std::setprecision(1) << std::fixed << ci.result_digits << std::defaultfloat
					          << ",\"severity\":\"" << ci.severity << "\"";
					if (!ci.suggestion.empty()) {
						std::cout << ",\"suggestion\":\"" << json_escape(ci.suggestion) << "\"";
					}
					std::cout << "}";
				}
				std::cout << "]}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "step,description,operand_a,operand_b,result,reference,shared_digits,result_digits,severity,suggestion\n";
				for (const auto& ci : cancellations) {
					std::cout << ci.step_number << ","
					          << csv_quote(ci.description) << ","
					          << csv_quote(ci.operand_a_rep) << ","
					          << csv_quote(ci.operand_b_rep) << ","
					          << csv_quote(ci.result_rep) << ","
					          << csv_quote(ci.exact_rep) << ","
					          << std::setprecision(1) << std::fixed << ci.shared_digits << std::defaultfloat << ","
					          << std::setprecision(1) << std::fixed << ci.result_digits << std::defaultfloat << ","
					          << ci.severity << ","
					          << csv_quote(ci.suggestion) << "\n";
				}
			} else if (fmt == OutputFormat::quiet) {
				for (const auto& ci : cancellations) {
					std::cout << ci.severity << " " << std::setprecision(1) << std::fixed
					          << ci.shared_digits << std::defaultfloat << "\n";
				}
			} else {
				// Plain output
				if (cancellations.empty()) {
					std::cout << "  No subtractions found in expression.\n";
				}
				for (const auto& ci : cancellations) {
					bool warn = (ci.severity != "none");
					if (warn) {
						std::string sev_upper = ci.severity;
						for (auto& c : sev_upper) c = static_cast<char>(std::toupper(c));
						std::cout << "  WARNING: " << sev_upper << " cancellation (step " << ci.step_number << ")\n";
					} else {
						std::cout << "  step " << ci.step_number << ": no significant cancellation\n";
					}
					std::cout << "  operand 1:       " << ci.operand_a_rep << "\n";
					std::cout << "  operand 2:       " << ci.operand_b_rep << "\n";
					std::cout << "  result:          " << ci.result_rep << "\n";
					std::cout << "  reference:       " << ci.exact_rep << "\n";
					std::cout << "  shared digits:   " << std::setprecision(1) << std::fixed
					          << ci.shared_digits << " of " << type_digits
					          << std::defaultfloat << "\n";
					std::cout << "  result digits:   ~" << std::setprecision(1) << std::fixed
					          << ci.result_digits << std::defaultfloat << "\n";
					if (!ci.suggestion.empty()) {
						std::cout << "  suggestion:      " << ci.suggestion << "\n";
					}
					std::cout << "\n";
				}
				std::cout << "  final result:    " << result.native_rep << "\n";
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_PARSE_ERROR;
		}
		return true;
	}

	// audit <expr> -- show every rounding event with cumulative error drift
	if (line.substr(0, 6) == "audit " || line.substr(0, 6) == "audit\t") {
		std::string expr = trim(line.substr(6));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);

			// Evaluate with tracing
			ExpressionEvaluator eval(ops);
			for (const auto& kv : state.evaluator->variables()) {
				eval.set_variable(kv.first, kv.second);
			}
			eval.enable_trace(true);
			Value result = eval.evaluate(expr);
			const auto& steps = eval.trace_steps();

			const TypeOps* ref_ops = state.registry.find("qd");
			if (!ref_ops) ref_ops = &state.registry.get("double");

			// Analyze each step for rounding
			struct AuditEntry {
				int step_number;
				std::string operation;
				std::string description;
				std::string result_rep;
				double result_decimal;
				std::string exact_rep;
				double exact_decimal;
				double ulp_error;          // unsigned
				double signed_ulp_error;   // positive = rounded up, negative = rounded down
				std::string rounding;      // "exact", "up", "down", "ties-to-even"
				double cumulative_ulp;     // running signed sum
			};
			std::vector<AuditEntry> entries;
			double cumulative = 0.0;
			int rounding_events = 0;
			double max_ulp = 0.0;

			for (const auto& s : steps) {
				// Compute exact result in qd (same replay logic as trace)
				double exact_d = 0.0;
				std::string exact_rep;
				try {
					Value r;
					if (s.operation == "add" || s.operation == "sub" ||
					    s.operation == "mul" || s.operation == "div" || s.operation == "pow") {
						Value a = ref_ops->from_double(s.operand_a);
						Value b = ref_ops->from_double(s.operand_b);
						if (s.operation == "add") r = ref_ops->add(a, b);
						else if (s.operation == "sub") r = ref_ops->sub(a, b);
						else if (s.operation == "mul") r = ref_ops->mul(a, b);
						else if (s.operation == "div") r = ref_ops->div(a, b);
						else r = ref_ops->fn_pow(a, b);
					} else if (s.operation == "negate") {
						r = ref_ops->negate(ref_ops->from_double(s.operand_a));
					} else {
						Value a = ref_ops->from_double(s.operand_a);
						if (s.operation == "sqrt") r = ref_ops->fn_sqrt(a);
						else if (s.operation == "abs") r = ref_ops->fn_abs(a);
						else if (s.operation == "log") r = ref_ops->fn_log(a);
						else if (s.operation == "exp") r = ref_ops->fn_exp(a);
						else if (s.operation == "sin") r = ref_ops->fn_sin(a);
						else if (s.operation == "cos") r = ref_ops->fn_cos(a);
						else if (s.operation == "tan") r = ref_ops->fn_tan(a);
						else if (s.operation == "asin") r = ref_ops->fn_asin(a);
						else if (s.operation == "acos") r = ref_ops->fn_acos(a);
						else if (s.operation == "atan") r = ref_ops->fn_atan(a);
						else r = ref_ops->from_double(s.result);
					}
					exact_d = r.num; exact_rep = r.native_rep;
				} catch (...) {
					exact_d = s.result; exact_rep = s.result_rep;
				}

				AuditEntry ae;
				ae.step_number = s.step_number;
				ae.operation = s.operation;
				ae.description = s.description;
				ae.result_rep = s.result_rep;
				ae.result_decimal = s.result;
				ae.exact_rep = exact_rep;
				ae.exact_decimal = exact_d;
				ae.ulp_error = 0.0;
				ae.signed_ulp_error = 0.0;
				ae.rounding = "exact";

				if (exact_d != 0.0 && std::isfinite(exact_d) && std::isfinite(s.result)) {
					double diff = s.result - exact_d;
					double ulp = compute_ulp(ops, exact_d);
					if (ulp > 0.0) {
						ae.ulp_error = std::abs(diff) / ulp;
						ae.signed_ulp_error = diff / ulp;
					}
				}

				if (s.result == exact_d) {
					ae.rounding = "exact";
				} else {
					// Detect ties-to-even: ULP error is exactly 0.5
					bool is_tie = (std::abs(ae.ulp_error - 0.5) < 1e-6);
					if (is_tie) {
						ae.rounding = "ties-to-even";
					} else if (s.result > exact_d) {
						ae.rounding = "up";
					} else {
						ae.rounding = "down";
					}
					++rounding_events;
					if (ae.ulp_error > max_ulp) max_ulp = ae.ulp_error;
				}

				cumulative += ae.signed_ulp_error;
				ae.cumulative_ulp = cumulative;
				entries.push_back(std::move(ae));
			}

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"result\":\"" << json_escape(result.native_rep) << "\""
				          << ",\"result_decimal\":" << json_number(result.num)
				          << ",\"rounding_events\":" << rounding_events
				          << ",\"max_ulp_error\":" << json_number(max_ulp)
				          << ",\"cumulative_drift\":" << json_number(cumulative)
				          << ",\"steps\":[";
				for (size_t i = 0; i < entries.size(); ++i) {
					const auto& ae = entries[i];
					if (i > 0) std::cout << ",";
					std::cout << "{\"step\":" << ae.step_number
					          << ",\"operation\":\"" << json_escape(ae.operation) << "\""
					          << ",\"description\":\"" << json_escape(ae.description) << "\""
					          << ",\"result\":\"" << json_escape(ae.result_rep) << "\""
					          << ",\"reference\":\"" << json_escape(ae.exact_rep) << "\""
					          << ",\"ulp_error\":" << json_number(ae.ulp_error)
					          << ",\"signed_ulp\":" << json_number(ae.signed_ulp_error)
					          << ",\"rounding\":\"" << ae.rounding << "\""
					          << ",\"cumulative_ulp\":" << json_number(ae.cumulative_ulp)
					          << "}";
				}
				std::cout << "]}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "step,operation,description,result,reference,ulp_error,signed_ulp,rounding,cumulative_ulp\n";
				for (const auto& ae : entries) {
					std::cout << ae.step_number << ","
					          << csv_quote(ae.operation) << ","
					          << csv_quote(ae.description) << ","
					          << csv_quote(ae.result_rep) << ","
					          << csv_quote(ae.exact_rep) << ","
					          << std::setprecision(4) << ae.ulp_error << ","
					          << std::setprecision(4) << std::showpos << ae.signed_ulp_error << std::noshowpos << ","
					          << ae.rounding << ","
					          << std::setprecision(4) << std::showpos << ae.cumulative_ulp << std::noshowpos << "\n";
				}
			} else if (fmt == OutputFormat::quiet) {
				for (const auto& ae : entries) {
					if (ae.rounding == "exact") continue;
					std::cout << ae.step_number << " " << ae.rounding << " "
					          << std::setprecision(2) << std::fixed << std::showpos
					          << ae.signed_ulp_error << std::noshowpos << std::defaultfloat
					          << " cum=" << std::setprecision(2) << std::fixed
					          << ae.cumulative_ulp << std::defaultfloat << "\n";
				}
			} else {
				// Plain text
				for (const auto& ae : entries) {
					std::cout << "  step " << ae.step_number << ": " << ae.description << "\n";
					if (ae.rounding == "exact") {
						std::cout << "          = " << ae.result_rep << "  (exact)\n";
					} else {
						std::cout << "          result:    " << std::setprecision(17) << ae.result_decimal << "\n";
						std::cout << "          reference: " << std::setprecision(17) << ae.exact_decimal << "\n";
						std::string dir;
						if (ae.rounding == "ties-to-even") dir = "TIES-TO-EVEN";
						else if (ae.rounding == "up") dir = "ROUNDED UP";
						else dir = "ROUNDED DOWN";
						std::cout << "          " << dir
						          << "  ulp: " << std::showpos << std::setprecision(2)
						          << std::fixed << ae.signed_ulp_error << std::noshowpos
						          << std::defaultfloat
						          << "  cumulative: " << std::showpos << std::setprecision(2)
						          << std::fixed << ae.cumulative_ulp << std::noshowpos
						          << std::defaultfloat << "\n";
					}
				}
				std::cout << "  --------\n";
				std::cout << "  result:           " << result.native_rep << "\n";
				std::cout << "  rounding events:  " << rounding_events << " of " << entries.size() << " operations\n";
				std::cout << "  max |ulp| error:  " << std::setprecision(2) << std::fixed << max_ulp << std::defaultfloat << "\n";
				std::cout << "  cumulative drift: " << std::showpos << std::setprecision(2) << std::fixed
				          << cumulative << std::noshowpos << std::defaultfloat << " ULPs\n";
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_PARSE_ERROR;
		}
		return true;
	}

	// faithful <expr> -- check if result is faithfully rounded
	if (line.substr(0, 9) == "faithful ") {
		std::string expr = trim(line.substr(9));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);
			Value result = state.evaluator->evaluate(expr);
			const TypeOps* ref_ops = state.registry.find("qd");
			if (!ref_ops) ref_ops = &state.registry.get("double");
			ExpressionEvaluator ref_eval(*ref_ops);
			for (const auto& kv : state.evaluator->variables()) {
				ref_eval.set_variable(kv.first, kv.second);
			}
			Value ref = ref_eval.evaluate(expr);
			Value rounded = ops.from_double(ref.num);
			double neighbor_val = rounded.num;
			if (rounded.num < ref.num) {
				double step_sz = std::max(std::abs(rounded.num), 1.0);
				for (int i = 0; i < 200; ++i) {
					Value test = ops.from_double(rounded.num + step_sz);
					if (test.num > rounded.num) {
						neighbor_val = test.num;
						double smaller = step_sz * 0.5;
						for (int j = 0; j < 60; ++j) {
							Value t2 = ops.from_double(rounded.num + smaller);
							if (t2.num <= rounded.num) break;
							neighbor_val = t2.num;
							smaller *= 0.5;
						}
						break;
					}
					step_sz *= 2.0;
				}
			} else if (rounded.num > ref.num) {
				double step_sz = std::max(std::abs(rounded.num), 1.0);
				for (int i = 0; i < 200; ++i) {
					Value test = ops.from_double(rounded.num - step_sz);
					if (test.num < rounded.num) {
						neighbor_val = test.num;
						double smaller = step_sz * 0.5;
						for (int j = 0; j < 60; ++j) {
							Value t2 = ops.from_double(rounded.num - smaller);
							if (t2.num >= rounded.num) break;
							neighbor_val = t2.num;
							smaller *= 0.5;
						}
						break;
					}
					step_sz *= 2.0;
				}
			}
			bool is_faithful = (result.num == rounded.num) || (result.num == neighbor_val);
			if (fmt == OutputFormat::json) {
				Value neighbor = ops.from_double(neighbor_val);
				std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"result\":\"" << json_escape(result.native_rep) << "\""
				          << ",\"reference\":\"" << json_escape(ref.native_rep) << "\""
				          << ",\"rounded\":\"" << json_escape(rounded.native_rep) << "\""
				          << ",\"neighbor\":\"" << json_escape(neighbor.native_rep) << "\""
				          << ",\"faithful\":" << (is_faithful ? "true" : "false")
				          << "}\n";
			} else if (fmt == OutputFormat::quiet) {
				std::cout << (is_faithful ? "yes" : "no") << "\n";
			} else {
				std::cout << "  result:    " << result.native_rep << "\n";
				std::cout << "  reference: " << ref.native_rep << "\n";
				std::cout << "  rounded:   " << rounded.native_rep << "\n";
				Value neighbor = ops.from_double(neighbor_val);
				std::cout << "  neighbor:  " << neighbor.native_rep << "\n";
				std::cout << "  faithful:  " << (is_faithful ? "yes" : "no") << "\n";
			}
		} catch (const std::exception& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_PARSE_ERROR;
		}
		return true;
	}

	// Otherwise, evaluate as expression
	try {
		Value result = state.evaluator->evaluate(line);
		if (fmt == OutputFormat::json) {
			std::cout << "{\"expression\":\"" << json_escape(line) << "\""
			          << ",\"type\":\"" << json_escape(result.type_name) << "\""
			          << ",\"value\":\"" << json_escape(result.native_rep) << "\""
			          << ",\"decimal\":" << json_number(result.num)
			          << ",\"binary\":\"" << json_escape(result.binary_rep) << "\""
			          << "}\n";
		} else {
			std::cout << result.native_rep << "\n";
		}
	} catch (const std::exception& ex) {
		if (fmt == OutputFormat::json) {
			std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
		} else {
			std::cerr << "Error: " << ex.what() << "\n";
		}
		state.last_error = EXIT_PARSE_ERROR;
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
			"type", "types", "show", "compare", "bits", "range", "precision",
			"ulp", "sweep", "trace", "cancel", "audit", "faithful", "color", "vars", "help", "quit", "exit", nullptr
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
			"sqrt", "abs", "log", "exp", "sin", "cos", "tan",
			"asin", "acos", "atan", "pow",
			"pi", "e", "phi", "ln2", "ln10", "sqrt2", "sqrt3", "sqrt5", nullptr
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

// Print usage for --help flag
static void print_usage() {
	std::cout << "Usage: ucalc [OPTIONS] [EXPRESSION]\n\n";
	std::cout << "Options:\n";
	std::cout << "  --json        JSON output for all commands\n";
	std::cout << "  --csv         CSV output for tabular commands\n";
	std::cout << "  --quiet       Value only, no decoration\n";
	std::cout << "  -t <type>     Set active type (e.g., -t posit32)\n";
	std::cout << "  -f <file>     Execute a script file (batch mode)\n";
	std::cout << "  --help        Show this usage message\n";
	std::cout << "  --version     Show version\n";
	std::cout << "\nExamples:\n";
	std::cout << "  ucalc \"type posit32; 1/3 + 1/3 + 1/3\"\n";
	std::cout << "  ucalc -t posit32 \"1/3 + 1/3 + 1/3\"\n";
	std::cout << "  ucalc --json \"show 3.14\"\n";
	std::cout << "  ucalc --quiet -t fp16 \"sin(0.1)\"\n";
	std::cout << "  ucalc -f my_script.ucalc\n";
	std::cout << "  echo \"compare pi\" | ucalc --csv\n";
}

}} // namespace sw::ucalc

int main(int argc, char** argv)
try {
	using namespace sw::ucalc;

	// Parse CLI options
	OutputFormat format = OutputFormat::plain;
	std::string cli_type;
	std::string script_file;
	std::vector<std::string> positional_args;

	for (int i = 1; i < argc; ++i) {
		std::string arg(argv[i]);
		if (arg == "--json") {
			format = OutputFormat::json;
		} else if (arg == "--csv") {
			format = OutputFormat::csv;
		} else if (arg == "--quiet") {
			format = OutputFormat::quiet;
		} else if (arg == "-t" && i + 1 < argc) {
			cli_type = argv[++i];
		} else if (arg == "-f" && i + 1 < argc) {
			script_file = argv[++i];
		} else if (arg == "--help") {
			print_usage();
			return EXIT_OK;
		} else if (arg == "--version") {
			std::cout << "ucalc 1.0 (Universal Numbers Library)\n";
			return EXIT_OK;
		} else if (arg[0] == '-' && arg.size() > 1 && arg[1] != '-') {
			// Unknown short flag -- could be negative number, try as positional
			positional_args.push_back(arg);
		} else {
			positional_args.push_back(arg);
		}
	}

	// Build the type registry
	TypeRegistry registry = build_default_registry();

	// Default to double, or use -t flag
	std::string active_type = "double";
	if (!cli_type.empty()) {
		const TypeOps* ops = registry.find(cli_type);
		if (!ops) {
			std::cerr << "Error: unknown type '" << cli_type
			          << "'. Use 'ucalc types' to list available types.\n";
			return EXIT_UNKNOWN_TYPE;
		}
		active_type = cli_type;
	}

	const TypeOps& default_ops = registry.get(active_type);
	ExpressionEvaluator evaluator(default_ops);

	bool interactive = ISATTY(FILENO(stdin)) && positional_args.empty() && script_file.empty();
	bool use_color = interactive && ISATTY(FILENO(stdout)) && format == OutputFormat::plain;
	ReplState state{ registry, &evaluator, active_type, interactive, use_color, format, EXIT_OK };

	// Batch file mode: -f script.ucalc
	if (!script_file.empty()) {
		std::ifstream fin(script_file);
		if (!fin.is_open()) {
			std::cerr << "Error: cannot open script file '" << script_file << "'\n";
			return EXIT_FILE_NOT_FOUND;
		}
		std::string line;
		while (std::getline(fin, line)) {
			auto cmds = split_commands(line);
			for (const auto& cmd : cmds) {
				if (!process_command(cmd, state)) {
					return state.last_error;
				}
			}
		}
		return state.last_error;
	}

	// Handle command-line expression: ucalc "1/3 + 1/3 + 1/3"
	if (!positional_args.empty()) {
		std::string expr;
		for (size_t i = 0; i < positional_args.size(); ++i) {
			if (i > 0) expr += " ";
			expr += positional_args[i];
		}
		auto cmds = split_commands(expr);
		for (const auto& cmd : cmds) {
			process_command(cmd, state);
		}
		return state.last_error;
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
		return EXIT_OK;
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

	return EXIT_OK;
}
catch (const std::exception& ex) {
	std::cerr << "Fatal error: " << ex.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
