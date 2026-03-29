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
#include <random>
#include <map>

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

// Block format types for quantize/block commands
#include <universal/number/mxfloat/mxfloat.hpp>
#include <universal/number/nvblock/nvblock.hpp>

// ucalc headers
#include "type_dispatch.hpp"
#include "expression.hpp"
#include "registry.hpp"
#include "output_format.hpp"
#include "steps_ieee.hpp"
#include "data_loader.hpp"

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
		          << "\"testvec\",\"oracle\",\"steps\",\"trace\",\"cancel\",\"audit\",\"diverge\",\"quantize\",\"block\","
		          << "\"dot\",\"clip\",\"increment\",\"decrement\",\"stochastic\",\"histogram\",\"heatmap\",\"numberline\",\"faithful\",\"color\",\"vars\",\"help\",\"quit\"]}\n";
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
	std::cout << "  testvec <type> <func> [a, b, n]  Generate golden test vectors\n";
	std::cout << "  oracle <type> <expr>  Canonical result with rounding verification\n";
	std::cout << "  steps <expr>   Step-by-step arithmetic (align, add, normalize, round)\n";
	std::cout << "  trace <expr>   Show each operation with ULP error and rounding direction\n";
	std::cout << "  cancel <expr>  Detect catastrophic cancellation in subtractions\n";
	std::cout << "  audit <expr>   Show every rounding event with cumulative error drift\n";
	std::cout << "  diverge <expr> <t1> <t2> <tol> for <var> in [a, b]\n";
	std::cout << "                 Find first input where two types disagree beyond tolerance\n";
	std::cout << "  quantize <fmt> [data] | -f <file>\n";
	std::cout << "                 Quantize a vector/file, report RMSE/QSNR/errors\n";
	std::cout << "  block <fmt> [data] | -f <file> [tensor_scale=N]\n";
	std::cout << "                 Show MX/NV block decomposition (scale + elements)\n";
	std::cout << "  dot [v1] [v2] [accum=<type>]  Mixed-precision dot product\n";
	std::cout << "  clip <type> [data] | -f <file>  Overflow/underflow map\n";
	std::cout << "  increment <expr>  Show value and next representable value\n";
	std::cout << "  decrement <expr>  Show value and previous representable value\n";
	std::cout << "  stochastic <expr> N  Simulate stochastic rounding N times\n";
	std::cout << "  histogram [lo, hi, bins]  Representable value distribution\n";
	std::cout << "  heatmap          Precision (sig bits) vs magnitude bar chart\n";
	std::cout << "  numberline [lo, hi]  ASCII visualization of representable value density\n";
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
				// Show full decimal when native_rep loses distinguishing digits
				{
					std::ostringstream dss;
					dss << std::setprecision(17) << result.num;
					std::string decimal = dss.str();
					if (decimal != result.native_rep) {
						std::cout << "  decimal:    " << decimal << "\n";
					}
				}
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

	// testvec <type> <function> [a, b, n] -- generate golden test vectors
	if (line.substr(0, 8) == "testvec " || line.substr(0, 8) == "testvec\t") {
		std::string args = trim(line.substr(8));
		try {
			// Parse: testvec <type> <function> [a, b, n]
			// Tokenize by spaces
			std::vector<std::string> tokens;
			{
				std::istringstream tss(args);
				std::string tok;
				while (tss >> tok) tokens.push_back(tok);
			}
			if (tokens.size() < 3)
				throw std::runtime_error("usage: testvec <type> <function> [a, b, n]");

			std::string type_name = tokens[0];
			std::string func_name = tokens[1];

			// Remaining tokens form the range [a, b, n]
			std::string range_str;
			for (size_t i = 2; i < tokens.size(); ++i) {
				if (i > 2) range_str += " ";
				range_str += tokens[i];
			}
			// Strip brackets and parse
			for (auto& c : range_str) { if (c == '[' || c == ']' || c == ',') c = ' '; }
			std::istringstream rss(range_str);
			double lo, hi;
			int n;
			if (!(rss >> lo >> hi >> n) || n < 1)
				throw std::runtime_error("range must be [a, b, n] with n >= 1");

			const TypeOps* target = state.registry.find(type_name);
			if (!target) {
				if (fmt == OutputFormat::json) {
					std::cout << "{\"error\":\"unknown type\",\"type\":\""
					          << json_escape(type_name) << "\"}\n";
				} else {
					std::cerr << "Error: unknown type '" << type_name << "'\n";
				}
				state.last_error = EXIT_UNKNOWN_TYPE;
				return true;
			}

			// Generate test vectors
			struct TestEntry {
				double input;
				std::string result_rep;    // native_rep
				double result_decimal;
				std::string result_binary; // to_binary
				std::string result_native; // to_native
			};
			std::vector<TestEntry> entries;

			double step = (n > 1) ? (hi - lo) / (n - 1) : 0.0;
			std::string expr = func_name + "(x)";
			for (int i = 0; i < n; ++i) {
				double x = lo + i * step;
				ExpressionEvaluator eval(*target);
				eval.set_variable("x", Value(x));
				Value result = eval.evaluate(expr);
				entries.push_back({ x, result.native_rep, result.num,
				                    result.binary_rep, result.native_enc });
			}

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"type\":\"" << json_escape(type_name) << "\""
				          << ",\"type_tag\":\"" << json_escape(target->type_tag) << "\""
				          << ",\"function\":\"" << json_escape(func_name) << "\""
				          << ",\"range\":[" << json_number(lo) << "," << json_number(hi) << "," << n << "]"
				          << ",\"vectors\":[";
				for (size_t i = 0; i < entries.size(); ++i) {
					const auto& e = entries[i];
					if (i > 0) std::cout << ",";
					std::cout << "{\"input\":" << json_number(e.input)
					          << ",\"value\":\"" << json_escape(e.result_rep) << "\""
					          << ",\"decimal\":" << json_number(e.result_decimal)
					          << ",\"binary\":\"" << json_escape(e.result_binary) << "\""
					          << "}";
				}
				std::cout << "]}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "input,value,decimal,binary\n";
				for (const auto& e : entries) {
					std::cout << std::setprecision(17) << e.input << ","
					          << csv_quote(e.result_rep) << ","
					          << std::setprecision(17) << e.result_decimal << ","
					          << csv_quote(e.result_binary) << "\n";
				}
			} else if (fmt == OutputFormat::quiet) {
				for (const auto& e : entries) {
					std::cout << std::setprecision(17) << e.input << " "
					          << e.result_rep << "\n";
				}
			} else {
				// Plain text: C++ initializer list format
				std::cout << "// Golden reference vectors for " << func_name
				          << "(x) in " << target->type_tag << "\n";
				std::cout << "// Generated by ucalc testvec\n";
				std::cout << "struct TestVector { double input; double expected; };\n";
				std::cout << "constexpr TestVector " << func_name << "_"
				          << type_name << "[] = {\n";
				for (size_t i = 0; i < entries.size(); ++i) {
					const auto& e = entries[i];
					std::cout << "    { " << std::setprecision(17) << e.input
					          << ", " << std::setprecision(17) << e.result_decimal
					          << " }";
					if (i + 1 < entries.size()) std::cout << ",";
					std::cout << "  // " << e.result_rep << "\n";
				}
				std::cout << "};\n";
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

	// oracle <type> <expr> -- canonical correctly-rounded result for a type
	if (line.substr(0, 7) == "oracle " || line.substr(0, 7) == "oracle\t") {
		std::string args = trim(line.substr(7));
		try {
			// Parse: oracle <type> <expr>
			auto space_pos = args.find_first_of(" \t");
			if (space_pos == std::string::npos)
				throw std::runtime_error("usage: oracle <type> <expr>");
			std::string type_name = trim(args.substr(0, space_pos));
			std::string expr = trim(args.substr(space_pos + 1));

			const TypeOps* target = state.registry.find(type_name);
			if (!target) {
				if (fmt == OutputFormat::json) {
					std::cout << "{\"error\":\"unknown type\",\"type\":\""
					          << json_escape(type_name) << "\"}\n";
				} else {
					std::cerr << "Error: unknown type '" << type_name << "'\n";
				}
				state.last_error = EXIT_UNKNOWN_TYPE;
				return true;
			}

			// Evaluate expression in the target type
			ExpressionEvaluator eval(*target);
			for (const auto& kv : state.evaluator->variables()) {
				eval.set_variable(kv.first, kv.second);
			}
			Value result = eval.evaluate(expr);

			// Compute reference in qd for faithfulness check
			const TypeOps* ref_ops = state.registry.find("qd");
			if (!ref_ops) ref_ops = &state.registry.get("double");
			ExpressionEvaluator ref_eval(*ref_ops);
			for (const auto& kv : state.evaluator->variables()) {
				ref_eval.set_variable(kv.first, kv.second);
			}
			Value ref = ref_eval.evaluate(expr);

			// Determine rounding status. Round the qd reference to the
			// target type -- if that matches the result, it's correctly rounded.
			// Use native_rep comparison (not double) so wide types are handled.
			Value rounded = target->from_double(ref.num);
			// Exact: the target result, when promoted back to qd, equals the
			// qd reference. Compare via native_rep of the rounded-back value.
			Value result_in_ref = ref_ops->from_double(result.num);
			bool is_exact = (result_in_ref.native_rep == ref.native_rep);
			bool is_nearest = (result.native_rep == rounded.native_rep);
			bool is_faithful = is_nearest;
			std::string rounding_status;

			if (is_exact) {
				rounding_status = "exact";
			} else if (is_nearest) {
				// Result matches nearest representable -- check if there's
				// also an adjacent value by using next/prev when available.
				rounding_status = "correctly rounded (nearest)";
				is_faithful = true;
			} else {
				// Result doesn't match nearest. Check if it's the adjacent
				// representable (faithfully rounded) using next/prev.
				if (target->next && target->prev) {
					Value neighbor_above = target->next(rounded);
					Value neighbor_below = target->prev(rounded);
					is_faithful = (result.native_rep == neighbor_above.native_rep) ||
					              (result.native_rep == neighbor_below.native_rep);
				} else {
					// No next/prev available -- fall back to double comparison
					is_faithful = false;
				}
				if (is_faithful) {
					rounding_status = "faithfully rounded (adjacent representable)";
				} else {
					rounding_status = "not faithfully rounded";
				}
			}

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"type\":\"" << json_escape(type_name) << "\""
				          << ",\"type_tag\":\"" << json_escape(target->type_tag) << "\""
				          << ",\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"value\":\"" << json_escape(result.native_rep) << "\""
				          << ",\"decimal\":" << json_number(result.num)
				          << ",\"binary\":\"" << json_escape(result.binary_rep) << "\""
				          << ",\"native_encoding\":\"" << json_escape(result.native_enc) << "\""
				          << ",\"components\":\"" << json_escape(result.components_rep) << "\""
				          << ",\"reference\":\"" << json_escape(ref.native_rep) << "\""
				          << ",\"exact\":" << (is_exact ? "true" : "false")
				          << ",\"faithful\":" << (is_faithful ? "true" : "false")
				          << ",\"rounding\":\"" << rounding_status << "\""
				          << "}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "field,value\n"
				          << "type," << csv_quote(target->type_tag) << "\n"
				          << "expression," << csv_quote(expr) << "\n"
				          << "value," << csv_quote(result.native_rep) << "\n"
				          << "native_encoding," << csv_quote(result.native_enc) << "\n"
				          << "binary," << csv_quote(result.binary_rep) << "\n"
				          << "components," << csv_quote(result.components_rep) << "\n"
				          << "reference," << csv_quote(ref.native_rep) << "\n"
				          << "exact," << (is_exact ? "true" : "false") << "\n"
				          << "faithful," << (is_faithful ? "true" : "false") << "\n"
				          << "rounding," << csv_quote(rounding_status) << "\n";
			} else if (fmt == OutputFormat::quiet) {
				std::cout << result.native_rep << "\n";
			} else {
				// Plain text
				std::cout << "  type:       " << target->type_tag << "\n";
				std::cout << "  expression: " << expr << "\n";
				std::cout << "  value:      " << result.native_rep << "\n";
				if (!result.native_enc.empty() && result.native_enc != result.binary_rep) {
					std::cout << "  encoding:   " << result.native_enc << "\n";
				}
				std::cout << "  binary:     " << result.binary_rep << "\n";
				std::cout << "  components: " << result.components_rep << "\n";
				std::cout << "  reference:  " << ref.native_rep << "\n";
				std::cout << "  rounding:   " << rounding_status << "\n";
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

	// steps <expr> -- step-by-step arithmetic visualization
	if (line.substr(0, 6) == "steps " || line.substr(0, 6) == "steps\t") {
		std::string expr = trim(line.substr(6));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);

			// Evaluate with tracing to capture operations
			ExpressionEvaluator eval(ops);
			for (const auto& kv : state.evaluator->variables()) {
				eval.set_variable(kv.first, kv.second);
			}
			eval.enable_trace(true);
			Value result = eval.evaluate(expr);
			const auto& traced = eval.trace_steps();

			if (traced.empty()) {
				if (fmt == OutputFormat::json) {
					std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
					          << ",\"steps\":[]"
					          << ",\"result\":\"" << json_escape(result.native_rep) << "\"}\n";
				} else {
					std::cout << "  No arithmetic operations to decompose.\n";
					std::cout << "  result: " << result.native_rep << "\n";
				}
				return true;
			}

			// Determine precision bits from epsilon
			double eps = ops.epsilon().num;
			int precision_bits = (eps > 0.0 && eps < 1.0)
			    ? static_cast<int>(-std::log2(eps)) + 1 : 24;

			// CSV header (before the loop)
			if (fmt == OutputFormat::csv) {
				std::cout << "operation,step,label,detail\n";
			}

			// Decompose each traced operation
			for (const auto& t : traced) {
				// Only decompose binary arithmetic ops
				bool is_arith = (t.operation == "add" || t.operation == "sub" ||
				                 t.operation == "mul" || t.operation == "div");
				std::vector<StepDescription> explanation;

				if (is_arith && ops.explain) {
					// Use type-specific explain if available
					Value va; va.num = t.operand_a;
					Value vb; vb.num = t.operand_b;
					explanation = ops.explain(va, vb, t.operation);
				} else if (is_arith) {
					// Fall back to IEEE binary decomposition
					Value va; va.num = t.operand_a;
					Value vb; vb.num = t.operand_b;
					explanation = explain_ieee(va, vb, t.operation, precision_bits);
				}

				if (fmt == OutputFormat::json) {
					std::cout << "{\"operation\":\"" << json_escape(t.description) << "\""
					          << ",\"result\":\"" << json_escape(t.result_rep) << "\""
					          << ",\"steps\":[";
					for (size_t i = 0; i < explanation.size(); ++i) {
						const auto& s = explanation[i];
						if (i > 0) std::cout << ",";
						std::cout << "{\"step\":" << s.step_number
						          << ",\"label\":\"" << json_escape(s.label) << "\""
						          << ",\"detail\":\"" << json_escape(s.detail) << "\""
						          << "}";
					}
					std::cout << "]}\n";
				} else if (fmt == OutputFormat::csv) {
					for (const auto& s : explanation) {
						std::cout << csv_quote(t.description) << ","
						          << s.step_number << ","
						          << csv_quote(s.label) << ","
						          << csv_quote(s.detail) << "\n";
					}
				} else {
					// Plain text
					std::cout << "  " << t.description << " = " << t.result_rep << "\n";
					if (explanation.empty()) {
						std::cout << "    (step-by-step not available for "
						          << t.operation << ")\n";
					}
					for (const auto& s : explanation) {
						std::cout << "    " << s.step_number << ". " << s.label << "\n";
						// Indent detail lines
						std::istringstream dss(s.detail);
						std::string dline;
						while (std::getline(dss, dline)) {
							std::cout << "       " << dline << "\n";
						}
					}
					std::cout << "\n";
				}
			}

			if (fmt == OutputFormat::plain) {
				std::cout << "  final result: " << result.native_rep << "\n";
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

			// Helper: load a reference operand. For types wider than double
			// (dd, qd, cascades), parse the lossless native_rep string to
			// avoid double truncation. For types <= double, from_double is
			// exact and avoids display-string round-trip artifacts.
			auto load_ref = [&](double fallback, const std::string& rep) -> Value {
				if (!rep.empty() && ops.nbits > 64) {
					ExpressionEvaluator ref_eval(*ref_ops);
					return ref_eval.evaluate(rep);
				}
				return ref_ops->from_double(fallback);
			};

			// Helper: replay a single traced operation in the reference type
			auto replay_in_ref = [&](const TraceStep& s, double& out_d, std::string& out_rep) {
				Value r;
				Value a = load_ref(s.operand_a, s.operand_a_rep);
				if (s.operation == "add" || s.operation == "sub" ||
				    s.operation == "mul" || s.operation == "div" || s.operation == "pow") {
					Value b = load_ref(s.operand_b, s.operand_b_rep);
					if (s.operation == "add")      r = ref_ops->add(a, b);
					else if (s.operation == "sub") r = ref_ops->sub(a, b);
					else if (s.operation == "mul") r = ref_ops->mul(a, b);
					else if (s.operation == "div") r = ref_ops->div(a, b);
					else                           r = ref_ops->fn_pow(a, b);
				} else if (s.operation == "negate") {
					r = ref_ops->negate(a);
				} else {
					if (s.operation == "sqrt")      r = ref_ops->fn_sqrt(a);
					else if (s.operation == "abs")  r = ref_ops->fn_abs(a);
					else if (s.operation == "log")  r = ref_ops->fn_log(a);
					else if (s.operation == "exp")  r = ref_ops->fn_exp(a);
					else if (s.operation == "sin")  r = ref_ops->fn_sin(a);
					else if (s.operation == "cos")  r = ref_ops->fn_cos(a);
					else if (s.operation == "tan")  r = ref_ops->fn_tan(a);
					else if (s.operation == "asin") r = ref_ops->fn_asin(a);
					else if (s.operation == "acos") r = ref_ops->fn_acos(a);
					else if (s.operation == "atan") r = ref_ops->fn_atan(a);
					else r = ref_ops->from_double(s.result);
				}
				out_d = r.num; out_rep = r.native_rep;
			};

			for (const auto& s : steps) {
				TraceAnnotation ann;
				ann.cancellation = false;
				ann.cancelled_digits = 0;
				ann.ulp_error = 0.0;
				ann.rounding = "exact";

				// Replay operation in reference type using lossless operands
				double exact_d = 0.0;
				std::string exact_rep;
				try {
					replay_in_ref(s, exact_d, exact_rep);
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
				          << ",\"reference_type\":\"" << json_escape(ref_ops->type_tag) << "\""
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
					// Format result as "decimal (native)" when they differ,
					// so the user can see both the distinguishing value and
					// the type's own representation.
					auto format_result = [](double num, const std::string& rep) -> std::string {
						std::ostringstream dss;
						dss << std::setprecision(17) << num;
						std::string decimal = dss.str();
						if (decimal == rep) return rep;
						return decimal + "  (" + rep + ")";
					};
					if (a.rounding == "exact") {
						std::cout << "          = " << format_result(s.result, s.result_rep) << "  (exact)\n";
					} else {
						std::cout << "          result:    " << format_result(s.result, s.result_rep) << "\n";
						std::cout << "          reference: " << a.exact_rep << "\n";
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
				std::cout << "  reference precision: " << ref_ops->type_tag << "\n";
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

				// Compute reference result in qd. For types wider than double,
				// parse the lossless native_rep to avoid double truncation;
				// for types <= double, from_double is exact.
				try {
					auto cancel_load_ref = [&](double fallback, const std::string& rep) -> Value {
						if (!rep.empty() && ops.nbits > 64) {
							ExpressionEvaluator ref_eval(*ref_ops);
							return ref_eval.evaluate(rep);
						}
						return ref_ops->from_double(fallback);
					};
					Value a = cancel_load_ref(s.operand_a, s.operand_a_rep);
					Value b = cancel_load_ref(s.operand_b, s.operand_b_rep);
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

			// Helper: load a reference operand. Same policy as trace:
			// for types wider than double, parse lossless native_rep;
			// for types <= double, from_double is exact and avoids
			// display-string round-trip artifacts.
			auto load_ref = [&](double fallback, const std::string& rep) -> Value {
				if (!rep.empty() && ops.nbits > 64) {
					ExpressionEvaluator ref_eval(*ref_ops);
					return ref_eval.evaluate(rep);
				}
				return ref_ops->from_double(fallback);
			};

			for (const auto& s : steps) {
				// Replay operation in reference type using lossless operands
				double exact_d = 0.0;
				std::string exact_rep;
				try {
					Value r;
					Value a = load_ref(s.operand_a, s.operand_a_rep);
					if (s.operation == "add" || s.operation == "sub" ||
					    s.operation == "mul" || s.operation == "div" || s.operation == "pow") {
						Value b = load_ref(s.operand_b, s.operand_b_rep);
						if (s.operation == "add")      r = ref_ops->add(a, b);
						else if (s.operation == "sub") r = ref_ops->sub(a, b);
						else if (s.operation == "mul") r = ref_ops->mul(a, b);
						else if (s.operation == "div") r = ref_ops->div(a, b);
						else                           r = ref_ops->fn_pow(a, b);
					} else if (s.operation == "negate") {
						r = ref_ops->negate(a);
					} else {
						if (s.operation == "sqrt")      r = ref_ops->fn_sqrt(a);
						else if (s.operation == "abs")  r = ref_ops->fn_abs(a);
						else if (s.operation == "log")  r = ref_ops->fn_log(a);
						else if (s.operation == "exp")  r = ref_ops->fn_exp(a);
						else if (s.operation == "sin")  r = ref_ops->fn_sin(a);
						else if (s.operation == "cos")  r = ref_ops->fn_cos(a);
						else if (s.operation == "tan")  r = ref_ops->fn_tan(a);
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
				          << ",\"reference_type\":\"" << json_escape(ref_ops->type_tag) << "\""
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
				auto format_result = [](double num, const std::string& rep) -> std::string {
					std::ostringstream dss;
					dss << std::setprecision(17) << num;
					std::string decimal = dss.str();
					if (decimal == rep) return rep;
					return decimal + "  (" + rep + ")";
				};
				for (const auto& ae : entries) {
					std::cout << "  step " << ae.step_number << ": " << ae.description << "\n";
					if (ae.rounding == "exact") {
						std::cout << "          = " << format_result(ae.result_decimal, ae.result_rep) << "  (exact)\n";
					} else {
						std::cout << "          result:    " << format_result(ae.result_decimal, ae.result_rep) << "\n";
						std::cout << "          reference: " << ae.exact_rep << "\n";
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
				std::cout << "  reference:        " << ref_ops->type_tag << "\n";
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

	// diverge <expr> <type1> <type2> <tol> for <var> in [a, b]
	// Find first input where two types disagree by more than tolerance
	if (line.substr(0, 8) == "diverge " || line.substr(0, 8) == "diverge\t") {
		try {
			// Parse: diverge <expr> <type1> <type2> <tol> for <var> in [a, b]
			auto for_pos = line.find(" for ");
			if (for_pos == std::string::npos)
				throw std::runtime_error("usage: diverge <expr> <type1> <type2> <tol> for <var> in [a, b]");
			std::string before_for = trim(line.substr(8, for_pos - 8));
			std::string after_for = line.substr(for_pos + 5);
			auto in_pos = after_for.find(" in ");
			if (in_pos == std::string::npos)
				throw std::runtime_error("usage: diverge <expr> <type1> <type2> <tol> for <var> in [a, b]");
			std::string var = trim(after_for.substr(0, in_pos));
			std::string range_str = trim(after_for.substr(in_pos + 4));

			// Parse the range [a, b]
			if (range_str.empty())
				throw std::runtime_error("empty range specification");
			if (range_str.front() == '[') range_str = range_str.substr(1);
			if (!range_str.empty() && range_str.back() == ']') range_str.pop_back();
			std::istringstream rss(range_str);
			std::string sa, sb;
			std::getline(rss, sa, ',');
			std::getline(rss, sb, ',');
			if (trim(sa).empty() || trim(sb).empty())
				throw std::runtime_error("range must have two values: [a, b]");
			double lo = std::stod(trim(sa));
			double hi = std::stod(trim(sb));

			// Parse before_for: <expr> <type1> <type2> <tol>
			std::vector<std::string> tokens;
			{
				std::istringstream tss(before_for);
				std::string tok;
				while (tss >> tok) tokens.push_back(tok);
			}
			if (tokens.size() < 4)
				throw std::runtime_error("usage: diverge <expr> <type1> <type2> <tol> for <var> in [a, b]");
			std::string tol_str = tokens.back(); tokens.pop_back();
			std::string type2_name = tokens.back(); tokens.pop_back();
			std::string type1_name = tokens.back(); tokens.pop_back();
			std::string expr;
			for (size_t i = 0; i < tokens.size(); ++i) {
				if (i > 0) expr += " ";
				expr += tokens[i];
			}

			// Parse tolerance: plain number (absolute) or "Nulp" (ULP-relative)
			bool tol_is_ulp = false;
			double tol_val = 0.0;
			if (tol_str.size() > 3 && tol_str.substr(tol_str.size() - 3) == "ulp") {
				tol_is_ulp = true;
				tol_val = std::stod(tol_str.substr(0, tol_str.size() - 3));
			} else {
				tol_val = std::stod(tol_str);
			}
			if (tol_val < 0.0)
				throw std::runtime_error("tolerance must be non-negative");

			const TypeOps* ops1 = state.registry.find(type1_name);
			const TypeOps* ops2 = state.registry.find(type2_name);
			if (!ops1) {
				if (fmt == OutputFormat::json) {
					std::cout << "{\"error\":\"unknown type\",\"type\":\""
					          << json_escape(type1_name) << "\"}\n";
				} else {
					std::cerr << "Error: unknown type '" << type1_name << "'\n";
				}
				state.last_error = EXIT_UNKNOWN_TYPE;
				return true;
			}
			if (!ops2) {
				if (fmt == OutputFormat::json) {
					std::cout << "{\"error\":\"unknown type\",\"type\":\""
					          << json_escape(type2_name) << "\"}\n";
				} else {
					std::cerr << "Error: unknown type '" << type2_name << "'\n";
				}
				state.last_error = EXIT_UNKNOWN_TYPE;
				return true;
			}

			// Evaluate expression at a given x in both types
			auto eval_at = [&](double xval) -> std::pair<Value, Value> {
				ExpressionEvaluator e1(*ops1);
				e1.set_variable(var, Value(xval));
				for (const auto& kv : state.evaluator->variables()) {
					if (kv.first != var) e1.set_variable(kv.first, kv.second);
				}
				ExpressionEvaluator e2(*ops2);
				e2.set_variable(var, Value(xval));
				for (const auto& kv : state.evaluator->variables()) {
					if (kv.first != var) e2.set_variable(kv.first, kv.second);
				}
				return { e1.evaluate(expr), e2.evaluate(expr) };
			};

			// Compute both absolute and ULP difference
			struct DiffResult {
				double abs_diff;
				double ulp_diff;
			};
			auto compute_diff = [&](const Value& v1, const Value& v2) -> DiffResult {
				// Handle non-finite values
				if (!std::isfinite(v1.num) || !std::isfinite(v2.num)) {
					if (v1.num == v2.num) return { 0.0, 0.0 }; // both same inf
					return { std::numeric_limits<double>::infinity(),
					         std::numeric_limits<double>::infinity() };
				}
				double ad = std::abs(v1.num - v2.num);
				double ud = 0.0;
				double ulp = compute_ulp(*ops1, v1.num);
				if (ulp > 0.0) ud = ad / ulp;
				return { ad, ud };
			};

			// Scan then binary-search for the first divergence
			int n_scan = 1000;
			double scan_step = (hi - lo) / n_scan;
			double found_x = std::numeric_limits<double>::quiet_NaN();
			DiffResult found_dr{ 0.0, 0.0 };
			Value found_v1, found_v2;

			for (int i = 0; i <= n_scan; ++i) {
				double x = lo + i * scan_step;
				auto [v1, v2] = eval_at(x);
				DiffResult dr = compute_diff(v1, v2);
				double check = tol_is_ulp ? dr.ulp_diff : dr.abs_diff;
				if (check > tol_val) {
					// Binary search to narrow down
					double x_lo = (i > 0) ? (lo + (i - 1) * scan_step) : lo;
					double x_hi = x;
					for (int j = 0; j < 64; ++j) {
						double x_mid = (x_lo + x_hi) * 0.5;
						if (x_mid == x_lo || x_mid == x_hi) break;
						auto [m1, m2] = eval_at(x_mid);
						DiffResult dm = compute_diff(m1, m2);
						double mc = tol_is_ulp ? dm.ulp_diff : dm.abs_diff;
						if (mc > tol_val) {
							x_hi = x_mid;
							found_v1 = m1; found_v2 = m2; found_dr = dm;
						} else {
							x_lo = x_mid;
						}
					}
					found_x = x_hi;
					if (found_dr.abs_diff == 0.0 && found_dr.ulp_diff == 0.0) {
						found_v1 = v1; found_v2 = v2; found_dr = dr;
					}
					break;
				}
			}

			// Output -- always report both abs_diff and ulp_diff
			if (fmt == OutputFormat::json) {
				std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"type1\":\"" << json_escape(type1_name) << "\""
				          << ",\"type2\":\"" << json_escape(type2_name) << "\""
				          << ",\"tolerance\":" << json_number(tol_val)
				          << ",\"tolerance_unit\":\"" << (tol_is_ulp ? "ulp" : "absolute") << "\"";
				if (std::isnan(found_x)) {
					std::cout << ",\"diverged\":false"
					          << ",\"range\":[" << json_number(lo) << "," << json_number(hi) << "]}\n";
				} else {
					std::cout << ",\"diverged\":true"
					          << ",\"x\":" << json_number(found_x)
					          << ",\"value1\":\"" << json_escape(found_v1.native_rep) << "\""
					          << ",\"value1_decimal\":" << json_number(found_v1.num)
					          << ",\"value2\":\"" << json_escape(found_v2.native_rep) << "\""
					          << ",\"value2_decimal\":" << json_number(found_v2.num)
					          << ",\"abs_diff\":" << json_number(found_dr.abs_diff)
					          << ",\"ulp_diff\":" << json_number(found_dr.ulp_diff)
					          << "}\n";
				}
			} else if (fmt == OutputFormat::csv) {
				std::cout << "diverged,x,value1,value2,abs_diff,ulp_diff\n";
				if (std::isnan(found_x)) {
					std::cout << "false,,,,,\n";
				} else {
					std::cout << "true,"
					          << std::setprecision(17) << found_x << ","
					          << csv_quote(found_v1.native_rep) << ","
					          << csv_quote(found_v2.native_rep) << ","
					          << std::setprecision(17) << found_dr.abs_diff << ","
					          << std::setprecision(6) << found_dr.ulp_diff << "\n";
				}
			} else if (fmt == OutputFormat::quiet) {
				if (std::isnan(found_x)) {
					std::cout << "no divergence\n";
				} else {
					std::cout << std::setprecision(17) << found_x << " "
					          << std::setprecision(6) << found_dr.abs_diff << " "
					          << std::setprecision(4) << found_dr.ulp_diff << "ulp\n";
				}
			} else {
				// Plain text
				if (std::isnan(found_x)) {
					std::cout << "  No divergence > " << tol_val << (tol_is_ulp ? " ULP" : "")
					          << " found in [" << lo << ", " << hi << "]\n";
				} else {
					std::cout << "  first divergence at " << var << " = "
					          << std::setprecision(17) << found_x << "\n";
					std::cout << "  " << std::left << std::setw(14) << type1_name
					          << found_v1.native_rep << "\n";
					std::cout << "  " << std::left << std::setw(14) << type2_name
					          << found_v2.native_rep << "\n";
					std::cout << "  abs diff:     " << std::setprecision(8) << found_dr.abs_diff << "\n";
					std::cout << "  ulp diff:     " << std::setprecision(4) << found_dr.ulp_diff << " ULPs\n";
				}
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

	// quantize <format> <data> -- quantize a vector/file and report error metrics
	// Syntax: quantize <format> [a, b, c, ...]
	//         quantize <format> -f <file.csv>
	if (line.substr(0, 9) == "quantize " || line.substr(0, 9) == "quantize\t") {
		std::string args = trim(line.substr(9));
		try {
			// Parse format name and data source
			std::string format_name;
			std::vector<double> data;
			auto space_pos = args.find_first_of(" \t");
			if (space_pos == std::string::npos) {
				throw std::runtime_error("usage: quantize <format> [data...] or quantize <format> -f <file>");
			}
			format_name = trim(args.substr(0, space_pos));
			std::string rest = trim(args.substr(space_pos + 1));

			// Look up the target format type
			const TypeOps* target = state.registry.find(format_name);
			if (!target) {
				if (fmt == OutputFormat::json) {
					std::cout << "{\"error\":\"unknown format\",\"format\":\""
					          << json_escape(format_name) << "\"}\n";
				} else {
					std::cerr << "Error: unknown format '" << format_name
					          << "'. Use 'types' to list available formats.\n";
				}
				state.last_error = EXIT_UNKNOWN_TYPE;
				return true;
			}

			// Parse data source: -f <file> or [inline vector]
			if (rest.size() >= 3 && rest.substr(0, 3) == "-f ") {
				std::string filepath = trim(rest.substr(3));
				data = load_csv(filepath);
			} else if (rest.front() == '[') {
				data = parse_vector_literal(rest);
			} else {
				throw std::runtime_error("expected [a, b, c, ...] or -f <file>, got: " + rest);
			}

			size_t n = data.size();

			// Quantize each element and compute metrics
			double sum_sq_signal = 0.0;   // sum(x^2)
			double sum_sq_error = 0.0;    // sum((x - Q(x))^2)
			double max_abs_err = 0.0;
			int clipped = 0;              // saturated to maxpos/maxneg
			int flushed = 0;              // underflowed to zero
			double sum_error = 0.0;       // sum(x - Q(x)) for bias

			double maxpos_val = target->maxpos().num;
			double minpos_val = target->minpos().num;

			// For per-element output (small datasets only)
			bool show_elements = (n <= 64);
			struct ElemInfo {
				double original;
				double quantized;
				std::string quantized_rep;
				double error;
				std::string status;  // "", "clipped", "flushed"
			};
			std::vector<ElemInfo> elements;

			for (size_t i = 0; i < n; ++i) {
				double x = data[i];
				Value qv = target->from_double(x);
				double qx = qv.num;

				double err = x - qx;
				sum_sq_signal += x * x;
				sum_sq_error += err * err;
				sum_error += err;
				double abs_err = std::abs(err);
				if (abs_err > max_abs_err) max_abs_err = abs_err;

				std::string status;
				if (x != 0.0 && qx == 0.0 && std::abs(x) < minpos_val * 2.0) {
					++flushed;
					status = "flushed";
				} else if (std::abs(qx) >= maxpos_val && std::abs(x) > std::abs(qx)) {
					++clipped;
					status = "clipped";
				}

				if (show_elements) {
					elements.push_back({x, qx, qv.native_rep, err, status});
				}
			}

			double rmse = (n > 0) ? std::sqrt(sum_sq_error / n) : 0.0;
			double qsnr_db = (sum_sq_error > 0.0)
			    ? 10.0 * std::log10(sum_sq_signal / sum_sq_error)
			    : std::numeric_limits<double>::infinity();
			double mean_error = (n > 0) ? sum_error / n : 0.0;

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"format\":\"" << json_escape(format_name) << "\""
				          << ",\"format_tag\":\"" << json_escape(target->type_tag) << "\""
				          << ",\"elements\":" << n
				          << ",\"rmse\":" << json_number(rmse)
				          << ",\"qsnr_db\":" << json_number(qsnr_db)
				          << ",\"max_abs_error\":" << json_number(max_abs_err)
				          << ",\"mean_error\":" << json_number(mean_error)
				          << ",\"clipped\":" << clipped
				          << ",\"flushed\":" << flushed;
				if (show_elements) {
					std::cout << ",\"data\":[";
					for (size_t i = 0; i < elements.size(); ++i) {
						const auto& e = elements[i];
						if (i > 0) std::cout << ",";
						std::cout << "{\"original\":" << json_number(e.original)
						          << ",\"quantized\":\"" << json_escape(e.quantized_rep) << "\""
						          << ",\"quantized_decimal\":" << json_number(e.quantized)
						          << ",\"error\":" << json_number(e.error);
						if (!e.status.empty()) {
							std::cout << ",\"status\":\"" << e.status << "\"";
						}
						std::cout << "}";
					}
					std::cout << "]";
				}
				std::cout << "}\n";
			} else if (fmt == OutputFormat::csv) {
				if (show_elements) {
					std::cout << "original,quantized,quantized_decimal,error,status\n";
					for (const auto& e : elements) {
						std::cout << std::setprecision(17) << e.original << ","
						          << csv_quote(e.quantized_rep) << ","
						          << std::setprecision(17) << e.quantized << ","
						          << std::setprecision(17) << e.error << ","
						          << e.status << "\n";
					}
					std::cout << "\n";
				}
				std::cout << "metric,value\n"
				          << "elements," << n << "\n"
				          << "rmse," << std::setprecision(17) << rmse << "\n"
				          << "qsnr_db," << std::setprecision(4) << qsnr_db << "\n"
				          << "max_abs_error," << std::setprecision(17) << max_abs_err << "\n"
				          << "mean_error," << std::setprecision(17) << mean_error << "\n"
				          << "clipped," << clipped << "\n"
				          << "flushed," << flushed << "\n";
			} else if (fmt == OutputFormat::quiet) {
				// One-line summary: RMSE QSNR(dB) elements
				std::cout << std::setprecision(6) << rmse << " "
				          << std::setprecision(1) << std::fixed << qsnr_db << std::defaultfloat << "dB "
				          << n << "\n";
			} else {
				// Plain text
				std::cout << "  format:         " << format_name << " (" << target->type_tag << ")\n";
				std::cout << "  elements:       " << n << "\n";
				if (show_elements) {
					std::cout << "\n";
					std::cout << "  " << std::left << std::setw(6) << "idx"
					          << std::right << std::setw(16) << "original"
					          << std::setw(16) << "quantized"
					          << std::setw(14) << "error"
					          << "  status\n";
					std::cout << "  " << std::string(58, '-') << "\n";
					for (size_t i = 0; i < elements.size(); ++i) {
						const auto& e = elements[i];
						std::cout << "  " << std::left << std::setw(6) << i
						          << std::right << std::setw(16) << std::setprecision(8) << e.original
						          << std::setw(16) << e.quantized_rep
						          << std::setw(14) << std::setprecision(6) << e.error;
						if (!e.status.empty()) std::cout << "  " << e.status;
						std::cout << "\n";
					}
					std::cout << "\n";
				}
				std::cout << "  RMSE:           " << std::setprecision(8) << rmse << "\n";
				std::cout << "  QSNR:           " << std::setprecision(1) << std::fixed << qsnr_db << std::defaultfloat << " dB\n";
				std::cout << "  max |error|:    " << std::setprecision(8) << max_abs_err << "\n";
				std::cout << "  mean error:     " << std::showpos << std::setprecision(8) << mean_error << std::noshowpos << "\n";
				std::cout << "  clipped:        " << clipped << "\n";
				std::cout << "  flushed:        " << flushed << "\n";
			}
		} catch (const file_not_found& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_FILE_NOT_FOUND;
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

	// block <format> <data> -- show MX/NV block decomposition
	if (line.substr(0, 6) == "block " || line.substr(0, 6) == "block\t") {
		std::string args = trim(line.substr(6));
		try {
			// Parse format name and data source
			auto space_pos = args.find_first_of(" \t");
			if (space_pos == std::string::npos)
				throw std::runtime_error("usage: block <format> [data...] or block <format> -f <file>");
			std::string format_name = trim(args.substr(0, space_pos));
			std::string rest = trim(args.substr(space_pos + 1));

			// Parse optional tensor_scale=N for nvblock formats
			float tensor_scale = 1.0f;
			{
				auto ts_pos = rest.find("tensor_scale=");
				if (ts_pos != std::string::npos) {
					auto ts_start = ts_pos + 13; // len("tensor_scale=")
					auto ts_end = rest.find_first_of(" \t,]", ts_start);
					if (ts_end == std::string::npos) ts_end = rest.size();
					tensor_scale = std::stof(rest.substr(ts_start, ts_end - ts_start));
					// Remove tensor_scale=N from rest
					rest = trim(rest.substr(0, ts_pos) + rest.substr(ts_end));
				}
			}

			// Parse data source
			std::vector<double> data;
			if (rest.size() >= 3 && rest.substr(0, 3) == "-f ") {
				data = load_csv(trim(rest.substr(3)));
			} else if (!rest.empty() && rest.front() == '[') {
				data = parse_vector_literal(rest);
			} else {
				throw std::runtime_error("expected [a, b, c, ...] or -f <file>");
			}

			// Convert to float for block APIs
			std::vector<float> fdata(data.begin(), data.end());

			// Block decomposition helper for mxblock types
			using namespace sw::universal;
			struct BlockEntry {
				size_t index;
				double original;
				std::string element_rep;
				double decoded;
				double error;
			};
			struct BlockInfo {
				size_t block_idx;
				size_t start;
				size_t count;
				std::string scale_rep;
				double scale_val;
				std::vector<BlockEntry> entries;
			};
			std::vector<BlockInfo> blocks;
			std::string format_desc;

			// Dispatch to the right block format
			auto run_mxblock = [&](auto block_proto, const std::string& desc) {
				using Block = decltype(block_proto);
				constexpr size_t BS = Block::blockSize;
				format_desc = desc;
				size_t n = fdata.size();
				for (size_t bi = 0; bi < n; bi += BS) {
					size_t count = std::min(BS, n - bi);
					Block blk;
					blk.quantize(fdata.data() + bi, count);
					BlockInfo info;
					info.block_idx = bi / BS;
					info.start = bi;
					info.count = count;
					{
						std::ostringstream ss;
						ss << blk.scale();
						info.scale_rep = ss.str();
					}
					info.scale_val = blk.scale().to_float();
					for (size_t j = 0; j < count; ++j) {
						BlockEntry e;
						e.index = bi + j;
						e.original = data[bi + j];
						float dec = blk[j];
						e.decoded = dec;
						{
							std::ostringstream ss;
							ss << blk.element(j);
							e.element_rep = ss.str();
						}
						e.error = e.original - e.decoded;
						info.entries.push_back(std::move(e));
					}
					blocks.push_back(std::move(info));
				}
			};

			auto run_nvblock = [&](auto block_proto, const std::string& desc) {
				using Block = decltype(block_proto);
				constexpr size_t BS = Block::blockSize;
				format_desc = desc;
				size_t n = fdata.size();
				for (size_t bi = 0; bi < n; bi += BS) {
					size_t count = std::min(BS, n - bi);
					Block blk;
					blk.quantize(fdata.data() + bi, tensor_scale, count);
					BlockInfo info;
					info.block_idx = bi / BS;
					info.start = bi;
					info.count = count;
					{
						std::ostringstream ss;
						ss << blk.block_scale();
						info.scale_rep = ss.str();
					}
					info.scale_val = static_cast<double>(blk.block_scale().to_float());
					for (size_t j = 0; j < count; ++j) {
						BlockEntry e;
						e.index = bi + j;
						e.original = data[bi + j];
						// nvblock operator[] returns block_scale * element (without tensor_scale)
						float dec = tensor_scale * blk[j];
						e.decoded = dec;
						{
							std::ostringstream ss;
							ss << blk.element(j);
							e.element_rep = ss.str();
						}
						e.error = e.original - e.decoded;
						info.entries.push_back(std::move(e));
					}
					blocks.push_back(std::move(info));
				}
			};

			if (format_name == "mxfp4")          run_mxblock(mxfp4{}, "MX FP4 (e2m1, block=32, e8m0 scale)");
			else if (format_name == "mxfp6")     run_mxblock(mxfp6{}, "MX FP6 (e3m2, block=32, e8m0 scale)");
			else if (format_name == "mxfp6e2m3") run_mxblock(mxfp6e2m3{}, "MX FP6 (e2m3, block=32, e8m0 scale)");
			else if (format_name == "mxfp8")     run_mxblock(mxfp8{}, "MX FP8 (e4m3, block=32, e8m0 scale)");
			else if (format_name == "mxfp8e5m2") run_mxblock(mxfp8e5m2{}, "MX FP8 (e5m2, block=32, e8m0 scale)");
			else if (format_name == "mxint8")    run_mxblock(mxint8{}, "MX INT8 (int8, block=32, e8m0 scale)");
			else if (format_name == "nvfp4")     run_nvblock(nvfp4{}, "NVFP4 (e2m1, block=16, e4m3 scale)");
			else throw std::runtime_error("unknown block format '" + format_name
			     + "'. Available: mxfp4, mxfp6, mxfp6e2m3, mxfp8, mxfp8e5m2, mxint8, nvfp4");

			// Compute aggregate metrics
			double sum_sq_signal = 0.0, sum_sq_error = 0.0;
			for (const auto& bi : blocks) {
				for (const auto& e : bi.entries) {
					sum_sq_signal += e.original * e.original;
					sum_sq_error += e.error * e.error;
				}
			}
			double rmse = (data.size() > 0) ? std::sqrt(sum_sq_error / data.size()) : 0.0;
			double qsnr = (sum_sq_error > 0.0) ? 10.0 * std::log10(sum_sq_signal / sum_sq_error)
			                                    : std::numeric_limits<double>::infinity();

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"format\":\"" << json_escape(format_name) << "\""
				          << ",\"description\":\"" << json_escape(format_desc) << "\""
				          << ",\"elements\":" << data.size();
				// tensor_scale only applies to nvblock formats
				if (format_name == "nvfp4") {
					std::cout << ",\"tensor_scale\":" << json_number(static_cast<double>(tensor_scale));
				}
				std::cout << ",\"rmse\":" << json_number(rmse)
				          << ",\"qsnr_db\":" << json_number(qsnr)
				          << ",\"blocks\":[";
				for (size_t i = 0; i < blocks.size(); ++i) {
					const auto& bi = blocks[i];
					if (i > 0) std::cout << ",";
					std::cout << "{\"block\":" << bi.block_idx
					          << ",\"scale\":\"" << json_escape(bi.scale_rep) << "\""
					          << ",\"scale_value\":" << json_number(bi.scale_val)
					          << ",\"elements\":[";
					for (size_t j = 0; j < bi.entries.size(); ++j) {
						const auto& e = bi.entries[j];
						if (j > 0) std::cout << ",";
						std::cout << "{\"idx\":" << e.index
						          << ",\"original\":" << json_number(e.original)
						          << ",\"element\":\"" << json_escape(e.element_rep) << "\""
						          << ",\"decoded\":" << json_number(e.decoded)
						          << ",\"error\":" << json_number(e.error) << "}";
					}
					std::cout << "]}";
				}
				std::cout << "]}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "block,index,original,element,scale,decoded,error\n";
				for (const auto& bi : blocks) {
					for (const auto& e : bi.entries) {
						std::cout << bi.block_idx << ","
						          << e.index << ","
						          << std::setprecision(8) << e.original << ","
						          << csv_quote(e.element_rep) << ","
						          << csv_quote(bi.scale_rep) << ","
						          << std::setprecision(8) << e.decoded << ","
						          << std::setprecision(8) << e.error << "\n";
					}
				}
			} else if (fmt == OutputFormat::quiet) {
				std::cout << std::setprecision(6) << rmse << " "
				          << std::setprecision(1) << std::fixed << qsnr << std::defaultfloat << "dB "
				          << data.size() << " " << blocks.size() << "blk\n";
			} else {
				// Plain text
				std::cout << "  format:    " << format_desc << "\n";
				std::cout << "  elements:  " << data.size() << "\n";
				if (tensor_scale != 1.0f) {
					std::cout << "  tensor_scale: " << tensor_scale << "\n";
				}
				std::cout << "\n";
				for (const auto& bi : blocks) {
					std::cout << "  block " << bi.block_idx
					          << "  scale: " << bi.scale_rep
					          << " (" << bi.scale_val << ")\n";
					std::cout << "  " << std::left << std::setw(6) << "idx"
					          << std::right << std::setw(14) << "original"
					          << "  " << std::left << std::setw(14) << "element"
					          << std::right << std::setw(14) << "decoded"
					          << std::setw(14) << "error" << "\n";
					std::cout << "  " << std::string(62, '-') << "\n";
					for (const auto& e : bi.entries) {
						std::cout << "  " << std::left << std::setw(6) << e.index
						          << std::right << std::setw(14) << std::setprecision(6) << e.original
						          << "  " << std::left << std::setw(14) << e.element_rep
						          << std::right << std::setw(14) << std::setprecision(6) << e.decoded
						          << std::setw(14) << std::setprecision(6) << e.error << "\n";
					}
					std::cout << "\n";
				}
				std::cout << "  RMSE:  " << std::setprecision(8) << rmse << "\n";
				std::cout << "  QSNR:  " << std::setprecision(1) << std::fixed << qsnr << std::defaultfloat << " dB\n";
			}
		} catch (const file_not_found& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_FILE_NOT_FOUND;
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

	// dot <v1> <v2> [accum=<type>] -- mixed-precision dot product
	if (line.substr(0, 4) == "dot " || line.substr(0, 4) == "dot\t") {
		std::string args = trim(line.substr(4));
		try {
			const TypeOps& elem_ops = state.registry.get(state.active_type);

			// Parse optional accum=<type> from the end
			std::string accum_name = state.active_type;
			const TypeOps* accum_ops = &elem_ops;
			{
				auto accum_pos = args.rfind("accum=");
				if (accum_pos != std::string::npos) {
					accum_name = trim(args.substr(accum_pos + 6));
					// Remove any trailing ] or whitespace from accum_name
					auto sp = accum_name.find_first_of(" \t]");
					if (sp != std::string::npos) accum_name = accum_name.substr(0, sp);
					accum_ops = state.registry.find(accum_name);
					if (!accum_ops)
						throw std::runtime_error("unknown accumulation type: " + accum_name);
					args = trim(args.substr(0, accum_pos));
				}
			}

			// Parse two vectors: [a, b, c] [d, e, f]
			// Find the boundary between the two vector literals
			auto first_close = args.find(']');
			if (first_close == std::string::npos)
				throw std::runtime_error("usage: dot [v1] [v2] [accum=<type>]");
			std::string v1_str = trim(args.substr(0, first_close + 1));
			std::string v2_str = trim(args.substr(first_close + 1));

			std::vector<double> v1 = parse_vector_literal(v1_str);
			std::vector<double> v2 = parse_vector_literal(v2_str);
			if (v1.size() != v2.size())
				throw std::runtime_error("vectors must have equal length (got "
				    + std::to_string(v1.size()) + " and " + std::to_string(v2.size()) + ")");
			size_t n = v1.size();

			// Compute dot product: elements in active type, accumulation in accum type
			// result = sum_i( accum_type(elem_type(v1[i]) * elem_type(v2[i])) )
			Value accum = accum_ops->from_double(0.0);
			for (size_t i = 0; i < n; ++i) {
				Value a = elem_ops.from_double(v1[i]);
				Value b = elem_ops.from_double(v2[i]);
				Value prod = elem_ops.mul(a, b);
				// Accumulate in the accumulation type
				Value prod_in_accum = accum_ops->from_double(prod.num);
				accum = accum_ops->add(accum, prod_in_accum);
			}

			// Compute reference in qd
			const TypeOps* ref_ops = state.registry.find("qd");
			if (!ref_ops) ref_ops = &state.registry.get("double");
			Value ref_accum = ref_ops->from_double(0.0);
			for (size_t i = 0; i < n; ++i) {
				Value a = ref_ops->from_double(v1[i]);
				Value b = ref_ops->from_double(v2[i]);
				Value prod = ref_ops->mul(a, b);
				ref_accum = ref_ops->add(ref_accum, prod);
			}

			// Compute error
			double abs_err = std::abs(accum.num - ref_accum.num);
			double rel_err = (ref_accum.num != 0.0) ? abs_err / std::abs(ref_accum.num) : 0.0;

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"element_type\":\"" << json_escape(state.active_type) << "\""
				          << ",\"accum_type\":\"" << json_escape(accum_name) << "\""
				          << ",\"length\":" << n
				          << ",\"result\":\"" << json_escape(accum.native_rep) << "\""
				          << ",\"result_decimal\":" << json_number(accum.num)
				          << ",\"reference\":\"" << json_escape(ref_accum.native_rep) << "\""
				          << ",\"reference_decimal\":" << json_number(ref_accum.num)
				          << ",\"abs_error\":" << json_number(abs_err)
				          << ",\"rel_error\":" << json_number(rel_err)
				          << "}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "element_type,accum_type,length,result,reference,abs_error,rel_error\n";
				std::cout << csv_quote(state.active_type) << ","
				          << csv_quote(accum_name) << ","
				          << n << ","
				          << csv_quote(accum.native_rep) << ","
				          << csv_quote(ref_accum.native_rep) << ","
				          << std::setprecision(17) << abs_err << ","
				          << std::setprecision(17) << rel_err << "\n";
			} else if (fmt == OutputFormat::quiet) {
				std::cout << accum.native_rep << "\n";
			} else {
				// Plain text
				std::cout << "  element type: " << elem_ops.type_tag << "\n";
				std::cout << "  accum type:   " << accum_ops->type_tag << "\n";
				std::cout << "  length:       " << n << "\n";
				std::cout << "  result:       " << accum.native_rep << "\n";
				std::cout << "  reference:    " << ref_accum.native_rep << "\n";
				if (abs_err == 0.0) {
					std::cout << "  error:        exact\n";
				} else {
					std::cout << "  abs error:    " << std::setprecision(8) << abs_err << "\n";
					std::cout << "  rel error:    " << std::setprecision(4) << rel_err << "\n";
				}
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

	// increment/decrement <expr> -- show value and its successor/predecessor
	if (line.substr(0, 10) == "increment " || line.substr(0, 10) == "increment\t" ||
	    line.substr(0, 10) == "decrement " || line.substr(0, 10) == "decrement\t") {
		bool is_increment = (line[0] == 'i');
		std::string expr = trim(line.substr(10));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);
			Value val = state.evaluator->evaluate(expr);
			auto& op_fn = is_increment ? ops.next : ops.prev;
			if (!op_fn)
				throw std::runtime_error("increment/decrement not supported for " + state.active_type);
			Value adj = op_fn(val);

			if (fmt == OutputFormat::json) {
				std::cout << "{\"operation\":\"" << (is_increment ? "increment" : "decrement") << "\""
				          << ",\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"value\":\"" << json_escape(val.native_rep) << "\""
				          << ",\"value_decimal\":" << json_number(val.num)
				          << ",\"value_binary\":\"" << json_escape(val.binary_rep) << "\""
				          << ",\"result\":\"" << json_escape(adj.native_rep) << "\""
				          << ",\"result_decimal\":" << json_number(adj.num)
				          << ",\"result_binary\":\"" << json_escape(adj.binary_rep) << "\""
				          << "}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "label,value,binary\n";
				std::cout << "original," << csv_quote(val.native_rep) << "," << csv_quote(val.binary_rep) << "\n";
				std::cout << (is_increment ? "increment" : "decrement") << ","
				          << csv_quote(adj.native_rep) << "," << csv_quote(adj.binary_rep) << "\n";
			} else if (fmt == OutputFormat::quiet) {
				std::cout << adj.native_rep << "\n";
			} else {
				// Plain text: native encoding and value on one line,
				// stacked so the fixed-width encoding aligns vertically.
				// Uses to_native() which shows binary for binary types,
				// decimal digits for decimal types, hex for hex types.
				std::cout << "  " << val.native_enc << "  " << val.native_rep << "\n";
				std::cout << "  " << adj.native_enc << "  " << adj.native_rep << "\n";
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

	// clip <type> <data> -- overflow/underflow map for a distribution
	if (line.substr(0, 5) == "clip " || line.substr(0, 5) == "clip\t") {
		std::string args = trim(line.substr(5));
		try {
			// Parse: clip <type> [data] or clip <type> -f <file>
			auto space_pos = args.find_first_of(" \t");
			if (space_pos == std::string::npos)
				throw std::runtime_error("usage: clip <type> [data...] or clip <type> -f <file>");
			std::string type_name = trim(args.substr(0, space_pos));
			std::string rest = trim(args.substr(space_pos + 1));

			const TypeOps* target = state.registry.find(type_name);
			if (!target) {
				if (fmt == OutputFormat::json) {
					std::cout << "{\"error\":\"unknown type\",\"type\":\""
					          << json_escape(type_name) << "\"}\n";
				} else {
					std::cerr << "Error: unknown type '" << type_name << "'\n";
				}
				state.last_error = EXIT_UNKNOWN_TYPE;
				return true;
			}

			// Parse data source
			std::vector<double> data;
			if (rest.size() >= 3 && rest.substr(0, 3) == "-f ") {
				data = load_csv(trim(rest.substr(3)));
			} else if (!rest.empty() && rest.front() == '[') {
				data = parse_vector_literal(rest);
			} else {
				throw std::runtime_error("expected [a, b, c, ...] or -f <file>");
			}

			size_t n = data.size();
			double maxpos_val = target->maxpos().num;
			double minpos_val = target->minpos().num;
			double maxneg_val = target->maxneg().num;

			// Categorize each value
			int representable = 0;
			int clipped_pos = 0;   // |value| > maxpos, saturated
			int clipped_neg = 0;
			int flushed = 0;       // 0 < |value| < minpos, became zero
			int exact_zero = 0;    // input was exactly zero

			// Data magnitude range
			double data_min = std::numeric_limits<double>::max();
			double data_max = 0.0;
			for (double x : data) {
				double ax = std::abs(x);
				if (ax > 0.0 && ax < data_min) data_min = ax;
				if (ax > data_max) data_max = ax;
			}
			if (data_min == std::numeric_limits<double>::max()) data_min = 0.0;

			for (size_t i = 0; i < n; ++i) {
				double x = data[i];
				if (x == 0.0) { ++exact_zero; ++representable; continue; }

				Value qv = target->from_double(x);
				double qx = qv.num;
				double ax = std::abs(x);

				if (qx == 0.0 && ax > 0.0) {
					++flushed;
				} else if (x > maxpos_val) {
					++clipped_pos;
				} else if (x < maxneg_val) {
					++clipped_neg;
				} else {
					++representable;
				}
			}

			int total_clipped = clipped_pos + clipped_neg;

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"type\":\"" << json_escape(type_name) << "\""
				          << ",\"type_tag\":\"" << json_escape(target->type_tag) << "\""
				          << ",\"elements\":" << n
				          << ",\"representable\":" << representable
				          << ",\"clipped\":" << total_clipped
				          << ",\"clipped_pos\":" << clipped_pos
				          << ",\"clipped_neg\":" << clipped_neg
				          << ",\"flushed\":" << flushed
				          << ",\"exact_zero\":" << exact_zero
				          << ",\"maxpos\":" << json_number(maxpos_val)
				          << ",\"minpos\":" << json_number(minpos_val)
				          << ",\"data_max\":" << json_number(data_max)
				          << ",\"data_min\":" << json_number(data_min)
				          << "}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "metric,value\n"
				          << "elements," << n << "\n"
				          << "representable," << representable << "\n"
				          << "clipped," << total_clipped << "\n"
				          << "clipped_pos," << clipped_pos << "\n"
				          << "clipped_neg," << clipped_neg << "\n"
				          << "flushed," << flushed << "\n"
				          << "exact_zero," << exact_zero << "\n";
			} else if (fmt == OutputFormat::quiet) {
				// One-liner: representable% clipped flushed
				double rep_pct = (n > 0) ? 100.0 * representable / n : 0.0;
				std::cout << std::setprecision(1) << std::fixed << rep_pct << std::defaultfloat
				          << "% " << total_clipped << "clip " << flushed << "flush\n";
			} else {
				// Plain text
				auto pct = [n](int count) -> std::string {
					if (n == 0) return "0.0%";
					std::ostringstream ss;
					ss << std::setprecision(1) << std::fixed << (100.0 * count / n) << "%";
					return ss.str();
				};
				std::cout << "  type:           " << target->type_tag << "\n";
				std::cout << "  total values:   " << n << "\n";
				std::cout << "  representable:  " << representable << " (" << pct(representable) << ")\n";
				std::cout << "  clipped:        " << total_clipped << " (" << pct(total_clipped) << ")";
				if (total_clipped > 0) {
					std::cout << "  [+" << clipped_pos << " / -" << clipped_neg << "]";
				}
				std::cout << "\n";
				std::cout << "  flushed:        " << flushed << " (" << pct(flushed) << ")\n";
				std::cout << "  exact zeros:    " << exact_zero << "\n";
				std::cout << "\n";
				std::cout << "  type range:     [" << maxneg_val << ", " << maxpos_val << "]\n";
				std::cout << "  type minpos:    " << minpos_val << "\n";
				std::cout << "  data |range|:   [" << data_min << ", " << data_max << "]\n";
			}
		} catch (const file_not_found& ex) {
			if (fmt == OutputFormat::json) {
				std::cout << "{\"error\":\"" << json_escape(ex.what()) << "\"}\n";
			} else {
				std::cerr << "Error: " << ex.what() << "\n";
			}
			state.last_error = EXIT_FILE_NOT_FOUND;
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

	// stochastic <expr> N -- simulate stochastic rounding N times
	if (line.substr(0, 11) == "stochastic " || line.substr(0, 11) == "stochastic\t") {
		std::string args = trim(line.substr(11));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);

			// Parse: stochastic <expr> N
			// N is the last token
			auto last_space = args.find_last_of(" \t");
			if (last_space == std::string::npos)
				throw std::runtime_error("usage: stochastic <expr> N");
			std::string expr = trim(args.substr(0, last_space));
			std::string trials_tok = trim(args.substr(last_space + 1));
			size_t consumed = 0;
			int trials = std::stoi(trials_tok, &consumed);
			if (consumed != trials_tok.size())
				throw std::runtime_error("usage: stochastic <expr> N (N must be an integer)");
			if (trials < 1 || trials > 10000000)
				throw std::runtime_error("N must be 1-10000000");

			// Evaluate in qd for the high-precision reference value.
			// Note: exact_val is truncated to double for neighbor finding
			// and probability computation (double interchange limitation).
			// This is adequate for types <= double precision.
			const TypeOps* ref_ops = state.registry.find("qd");
			if (!ref_ops) ref_ops = &state.registry.get("double");
			ExpressionEvaluator eval(*ref_ops);
			for (const auto& kv : state.evaluator->variables()) {
				eval.set_variable(kv.first, kv.second);
			}
			Value ref_result = eval.evaluate(expr);
			double exact_val = ref_result.num;

			// Round the exact value to the active type (nearest)
			Value v_round = ops.from_double(exact_val);
			double rounded = v_round.num;

			double val_lo, val_hi;
			if (rounded <= exact_val) {
				val_lo = rounded;
				// Find next above
				if (ops.next) {
					val_hi = ops.next(v_round).num;
				} else {
					double step = compute_ulp(ops, rounded);
					val_hi = ops.from_double(rounded + step).num;
				}
			} else {
				val_hi = rounded;
				// Find next below
				if (ops.prev) {
					val_lo = ops.prev(v_round).num;
				} else {
					double step = compute_ulp(ops, rounded);
					val_lo = ops.from_double(rounded - step).num;
				}
			}

			// If exact, all trials give the same result
			bool is_exact = (val_lo == val_hi) || (exact_val == val_lo) || (exact_val == val_hi);
			double prob_hi = 0.0;
			if (!is_exact && val_hi > val_lo) {
				prob_hi = (exact_val - val_lo) / (val_hi - val_lo);
			} else if (exact_val == val_hi) {
				prob_hi = 1.0;
			}

			// Run stochastic rounding trials
			std::mt19937 rng(42); // fixed seed for reproducibility
			std::uniform_real_distribution<double> dist(0.0, 1.0);
			std::map<std::string, int> result_counts; // native_rep -> count
			double mean = 0.0; // online incremental mean (avoids sum overflow)

			for (int t = 0; t < trials; ++t) {
				double chosen;
				if (is_exact) {
					chosen = exact_val;
				} else {
					chosen = (dist(rng) < prob_hi) ? val_hi : val_lo;
				}
				Value cv = ops.from_double(chosen);
				result_counts[cv.native_rep]++;
				mean += (cv.num - mean) / static_cast<double>(t + 1);
			}
			// Bias relative to the qd reference (already computed above)
			double bias = mean - exact_val;

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"expression\":\"" << json_escape(expr) << "\""
				          << ",\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"trials\":" << trials
				          << ",\"unique_results\":" << result_counts.size()
				          << ",\"mean\":" << json_number(mean)
				          << ",\"exact\":" << json_number(exact_val)
				          << ",\"bias\":" << json_number(bias)
				          << ",\"results\":[";
				bool first = true;
				for (const auto& [rep, count] : result_counts) {
					if (!first) std::cout << ",";
					first = false;
					std::cout << "{\"value\":\"" << json_escape(rep) << "\""
					          << ",\"count\":" << count
					          << ",\"pct\":" << std::setprecision(4) << (100.0 * count / trials)
					          << "}";
				}
				std::cout << "]}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "value,count,pct\n";
				for (const auto& [rep, count] : result_counts) {
					std::cout << csv_quote(rep) << ","
					          << count << ","
					          << std::setprecision(4) << (100.0 * count / trials) << "\n";
				}
			} else if (fmt == OutputFormat::quiet) {
				std::cout << std::setprecision(8) << mean << " bias=" << std::showpos
				          << std::setprecision(6) << bias << std::noshowpos << "\n";
			} else {
				// Plain text
				std::cout << "  unique results: " << result_counts.size() << "\n";
				for (const auto& [rep, count] : result_counts) {
					double pct = 100.0 * count / trials;
					std::cout << "  " << std::left << std::setw(20) << rep
					          << std::right << std::setw(8) << count
					          << " (" << std::setprecision(1) << std::fixed << pct << std::defaultfloat << "%)\n";
				}
				std::cout << "  mean:  " << std::setprecision(10) << mean << "\n";
				std::cout << "  exact: " << ref_result.native_rep << "\n";
				std::cout << "  bias:  " << std::showpos << std::setprecision(8) << bias << std::noshowpos << "\n";
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

	// histogram [lo, hi, bins] -- representable value distribution
	if (line.substr(0, 10) == "histogram " || line.substr(0, 10) == "histogram\t") {
		std::string args = trim(line.substr(10));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);

			// Parse [lo, hi, bins]
			for (auto& c : args) { if (c == '[' || c == ']' || c == ',') c = ' '; }
			std::istringstream rss(args);
			double lo, hi;
			int bins = 20;
			if (!(rss >> lo >> hi))
				throw std::runtime_error("usage: histogram [lo, hi] or [lo, hi, bins]");
			rss >> bins; // optional, defaults to 20
			if (lo >= hi) throw std::runtime_error("lo must be less than hi");
			if (bins < 1 || bins > 1000) throw std::runtime_error("bins must be 1-1000");

			// Estimate count first (same as numberline)
			constexpr size_t max_values = 100000;
			double estimated_count = 0.0;
			{
				constexpr int n_samples = 5;
				double sum_density = 0.0;
				for (int i = 0; i < n_samples; ++i) {
					double x = lo + (hi - lo) * (0.1 + 0.8 * i / (n_samples - 1));
					double ulp_val = compute_ulp(ops, x);
					if (ulp_val > 0.0) sum_density += 1.0 / ulp_val;
				}
				estimated_count = (sum_density / n_samples) * (hi - lo);
			}
			bool skip = (estimated_count > max_values * 10);

			// Enumerate values if feasible
			std::vector<int> bin_counts(bins, 0);
			size_t total = 0;
			double bin_width = (hi - lo) / bins;

			if (!skip) {
				auto next_representable = [&](double v) -> double {
					double s = compute_ulp(ops, v);
					if (s == 0.0) s = std::numeric_limits<double>::min();
					for (int i = 0; i < 200; ++i) {
						double c = ops.from_double(v + s).num;
						if (c > v) return c;
						s *= 2.0;
						if (!std::isfinite(s)) break;
					}
					return std::numeric_limits<double>::infinity();
				};

				double v = ops.from_double(lo).num;
				if (v < lo) v = next_representable(v);
				while (v <= hi && total < max_values) {
					int bin = static_cast<int>((v - lo) / bin_width);
					if (bin < 0) bin = 0;
					if (bin >= bins) bin = bins - 1;
					++bin_counts[bin];
					++total;
					double next = next_representable(v);
					if (next <= v || !std::isfinite(next)) break;
					v = next;
				}
			}

			// Output
			if (fmt == OutputFormat::json) {
				std::cout << "{\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"range\":[" << json_number(lo) << "," << json_number(hi) << "]"
				          << ",\"bins\":" << bins
				          << ",\"total\":" << total;
				if (skip) {
					std::cout << ",\"estimated\":" << json_number(estimated_count)
					          << ",\"enumerated\":false";
				}
				std::cout << ",\"histogram\":[";
				for (int i = 0; i < bins; ++i) {
					if (i > 0) std::cout << ",";
					double blo = lo + i * bin_width;
					double bhi = blo + bin_width;
					std::cout << "{\"lo\":" << json_number(blo)
					          << ",\"hi\":" << json_number(bhi)
					          << ",\"count\":" << bin_counts[i] << "}";
				}
				std::cout << "]}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "bin_lo,bin_hi,count\n";
				for (int i = 0; i < bins; ++i) {
					double blo = lo + i * bin_width;
					double bhi = blo + bin_width;
					std::cout << std::setprecision(8) << blo << ","
					          << std::setprecision(8) << bhi << ","
					          << bin_counts[i] << "\n";
				}
			} else if (fmt == OutputFormat::quiet) {
				for (int i = 0; i < bins; ++i) {
					std::cout << bin_counts[i] << "\n";
				}
			} else {
				// Plain text with ASCII bar chart
				std::cout << "  " << ops.type_tag << " in ["
				          << lo << ", " << hi << "]\n";
				if (skip) {
					std::cout << "  (too many values to enumerate; showing empty histogram)\n\n";
				} else {
					std::cout << "  representable values: " << total << "\n\n";
				}
				int max_count = *std::max_element(bin_counts.begin(), bin_counts.end());
				constexpr int bar_max = 40;
				for (int i = 0; i < bins; ++i) {
					double blo = lo + i * bin_width;
					double bhi = blo + bin_width;
					int bar_len = (max_count > 0)
					    ? static_cast<int>(static_cast<double>(bin_counts[i]) / max_count * bar_max)
					    : 0;
					std::cout << "  [" << std::setw(10) << std::setprecision(4) << blo
					          << "," << std::setw(10) << std::setprecision(4) << bhi << ") "
					          << std::string(bar_len, '#')
					          << " " << bin_counts[i] << "\n";
				}
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

	// heatmap -- precision (significant bits) as a function of magnitude
	if (line == "heatmap" || line.substr(0, 8) == "heatmap " || line.substr(0, 8) == "heatmap\t") {
		try {
			const TypeOps& ops = state.registry.get(state.active_type);

			// Determine the type's dynamic range from minpos/maxpos
			double minpos = ops.minpos().num;
			double maxpos = ops.maxpos().num;
			if (minpos <= 0.0) minpos = 1e-38;
			if (maxpos <= 0.0 || !std::isfinite(maxpos)) maxpos = 1e38;

			// Sample magnitudes: powers of 10 from minpos to maxpos
			int lo_exp = static_cast<int>(std::floor(std::log10(minpos)));
			int hi_exp = static_cast<int>(std::floor(std::log10(maxpos)));
			// Limit span to 80 decades; center if wider
			if (hi_exp - lo_exp > 80) {
				int center = (lo_exp + hi_exp) / 2;
				lo_exp = center - 40;
				hi_exp = center + 40;
			}

			struct HeatmapEntry {
				double magnitude;
				int exponent;       // power of 10
				double sig_bits;    // effective significant bits at this magnitude
			};
			std::vector<HeatmapEntry> entries;
			double max_bits = 0.0;

			for (int e = lo_exp; e <= hi_exp; ++e) {
				double mag = std::pow(10.0, e);
				double rounded = ops.from_double(mag).num;
				if (rounded == 0.0 || !std::isfinite(rounded)) continue;
				double ulp = compute_ulp(ops, mag);
				if (ulp <= 0.0) continue;
				double bits = -std::log2(ulp / std::abs(rounded));
				if (bits <= 0.0) bits = 0.0;  // avoid -0.0 display
				if (bits > max_bits) max_bits = bits;
				entries.push_back({ mag, e, bits });
			}

			// Classify the precision profile
			std::string profile = "unclassified";
			if (entries.size() >= 3) {
				double first = entries.front().sig_bits;
				double last = entries.back().sig_bits;
				double mid_bits = 0.0;
				for (const auto& h : entries) mid_bits = std::max(mid_bits, h.sig_bits);
				if (mid_bits > first * 1.5 && mid_bits > last * 1.5) {
					profile = "tapered";
				} else if (std::abs(first - last) < 1.0 && std::abs(first - mid_bits) < 2.0) {
					profile = "uniform";
				} else if (first > last * 1.5) {
					profile = "decreasing";
				} else if (last > first * 1.5) {
					profile = "increasing";
				}
			}

			if (fmt == OutputFormat::json) {
				std::cout << "{\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"profile\":\"" << profile << "\""
				          << ",\"entries\":[";
				for (size_t i = 0; i < entries.size(); ++i) {
					const auto& h = entries[i];
					if (i > 0) std::cout << ",";
					std::cout << "{\"magnitude\":\"1e" << (h.exponent >= 0 ? "+" : "") << h.exponent << "\""
					          << ",\"magnitude_decimal\":" << json_number(h.magnitude)
					          << ",\"sig_bits\":" << std::setprecision(1) << std::fixed << h.sig_bits << std::defaultfloat
					          << "}";
				}
				std::cout << "]}\n";
			} else if (fmt == OutputFormat::csv) {
				std::cout << "magnitude,sig_bits,profile\n";
				for (const auto& h : entries) {
					std::cout << "1e" << (h.exponent >= 0 ? "+" : "") << h.exponent
					          << "," << std::setprecision(1) << std::fixed << h.sig_bits << std::defaultfloat
					          << "," << profile << "\n";
				}
			} else if (fmt == OutputFormat::quiet) {
				std::cout << profile << "\n";
				for (const auto& h : entries) {
					std::cout << h.exponent << " "
					          << std::setprecision(1) << std::fixed << h.sig_bits << std::defaultfloat << "\n";
				}
			} else {
				// Plain text with ASCII bar chart
				// Collapse runs of identical precision into "..." ranges
				std::cout << "  " << ops.type_tag << "\n\n";
				constexpr int bar_max = 40;
				std::cout << "  " << std::left << std::setw(12) << "magnitude"
				          << std::right << std::setw(10) << "sig_bits"
				          << "  bar\n";
				std::cout << "  " << std::string(62, '-') << "\n";
				auto fmt_mag = [](int e) -> std::string {
					return "1e" + std::string(e >= 0 ? "+" : "") + std::to_string(e);
				};
				auto print_row = [&](const std::string& mag_str, double bits) {
					int bar_len = (max_bits > 0.0)
					    ? static_cast<int>(bits / max_bits * bar_max) : 0;
					std::cout << "  " << std::left << std::setw(12) << mag_str
					          << std::right << std::setw(10) << std::setprecision(1) << std::fixed << bits << std::defaultfloat
					          << "  " << std::string(bar_len, '#') << "\n";
				};
				size_t i = 0;
				while (i < entries.size()) {
					size_t run_start = i;
					double bits = entries[i].sig_bits;
					while (i < entries.size() && std::abs(entries[i].sig_bits - bits) < 0.5) ++i;
					size_t run_len = i - run_start;
					if (run_len <= 3) {
						for (size_t j = run_start; j < i; ++j)
							print_row(fmt_mag(entries[j].exponent), entries[j].sig_bits);
					} else {
						print_row(fmt_mag(entries[run_start].exponent), entries[run_start].sig_bits);
						std::cout << "  " << std::left << std::setw(12) << "  ..."
						          << std::right << std::setw(10) << ""
						          << "  (" << (run_len - 2) << " more at same precision)\n";
						print_row(fmt_mag(entries[i - 1].exponent), entries[i - 1].sig_bits);
					}
				}

				std::cout << "\n";
				if (profile == "tapered")
					std::cout << "  tapered precision: peaks near 1, falls off at extremes\n";
				else if (profile == "uniform")
					std::cout << "  uniform precision across dynamic range\n";
				else if (profile == "decreasing")
					std::cout << "  precision decreases with magnitude\n";
				else if (profile == "increasing")
					std::cout << "  precision increases with magnitude\n";
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

	// numberline [lo, hi] -- ASCII visualization of representable values
	if (line == "numberline" || line.substr(0, 11) == "numberline " || line.substr(0, 11) == "numberline\t") {
		std::string args = (line.size() <= 10) ? "" : trim(line.substr(11));
		try {
			const TypeOps& ops = state.registry.get(state.active_type);

			// Parse range [lo, hi]
			if (args.empty())
				throw std::runtime_error("usage: numberline [lo, hi]");
			std::string range_str = args;
			if (range_str.front() == '[') range_str = range_str.substr(1);
			if (!range_str.empty() && range_str.back() == ']') range_str.pop_back();
			// Accept both comma and space as separator: [lo, hi] or [lo hi]
			for (auto& c : range_str) { if (c == ',') c = ' '; }
			std::istringstream rss(range_str);
			double lo, hi;
			if (!(rss >> lo >> hi))
				throw std::runtime_error("usage: numberline [lo, hi] or [lo hi]");
			if (lo >= hi)
				throw std::runtime_error("lo must be less than hi");

			// For types wider than double (dd, qd, cascades), the double
			// lattice is coarser than the type's lattice, so we undercount.
			bool wide_type = (ops.nbits > 64);

			// Estimate representable value count from ULP samples before
			// committing to full enumeration. Sample ULP at a few points
			// across the range, compute average density, extrapolate.
			constexpr size_t max_values = 100000;
			double estimated_count = 0.0;
			{
				constexpr int n_samples = 5;
				double sum_density = 0.0;
				for (int i = 0; i < n_samples; ++i) {
					double x = lo + (hi - lo) * (0.1 + 0.8 * i / (n_samples - 1));
					double ulp = compute_ulp(ops, x);
					if (ulp > 0.0) sum_density += 1.0 / ulp;
				}
				double avg_density = sum_density / n_samples;
				estimated_count = avg_density * (hi - lo);
			}

			bool skip_enumeration = (estimated_count > max_values * 10);

			// Find the next representable value strictly greater than v.
			// Starts from ULP and doubles the probe step if needed.
			auto next_representable = [&](double v) -> double {
				double step = compute_ulp(ops, v);
				if (step == 0.0) step = std::numeric_limits<double>::min();
				for (int i = 0; i < 200; ++i) {
					double candidate = ops.from_double(v + step).num;
					if (candidate > v) return candidate;
					step *= 2.0;
					if (!std::isfinite(step)) break;
				}
				return std::numeric_limits<double>::infinity(); // no successor
			};

			// Enumerate representable values in [lo, hi] unless skipped.
			std::vector<double> values;
			if (!skip_enumeration) {
				double v = ops.from_double(lo).num;
				if (v < lo) v = next_representable(v);
				while (v <= hi && values.size() < max_values) {
					values.push_back(v);
					double next = next_representable(v);
					if (next <= v || !std::isfinite(next)) break;
					v = next;
				}
			}

			size_t count = values.size();
			bool truncated = (count >= max_values);

			// Format the estimated count for display
			auto format_estimate = [](double est) -> std::string {
				if (est >= 1e12) {
					std::ostringstream ss;
					ss << std::setprecision(2) << std::fixed << (est / 1e12) << " trillion";
					return ss.str();
				} else if (est >= 1e9) {
					std::ostringstream ss;
					ss << std::setprecision(2) << std::fixed << (est / 1e9) << " billion";
					return ss.str();
				} else if (est >= 1e6) {
					std::ostringstream ss;
					ss << std::setprecision(2) << std::fixed << (est / 1e6) << " million";
					return ss.str();
				} else {
					std::ostringstream ss;
					ss << std::setprecision(0) << std::fixed << est;
					return ss.str();
				}
			};

			if (fmt == OutputFormat::json) {
				std::cout << "{\"type\":\"" << json_escape(ops.type_tag) << "\""
				          << ",\"range\":[" << json_number(lo) << "," << json_number(hi) << "]";
				if (skip_enumeration) {
					std::cout << ",\"estimated_count\":" << json_number(estimated_count)
					          << ",\"enumerated\":false";
				} else {
					std::cout << ",\"count\":" << count
					          << ",\"truncated\":" << (truncated ? "true" : "false");
				}
				std::cout << ",\"approximate\":" << ((wide_type || skip_enumeration) ? "true" : "false");
				if (!skip_enumeration) {
					std::cout << ",\"values\":[";
					for (size_t i = 0; i < count; ++i) {
						if (i > 0) std::cout << ",";
						std::cout << json_number(values[i]);
					}
					std::cout << "]";
				}
				std::cout << "}\n";
			} else if (fmt == OutputFormat::csv) {
				if (skip_enumeration) {
					std::cout << "estimated_count\n"
					          << std::setprecision(0) << std::fixed << estimated_count << "\n";
				} else {
					std::cout << "index,value\n";
					for (size_t i = 0; i < count; ++i) {
						std::cout << i << "," << std::setprecision(17) << values[i] << "\n";
					}
				}
			} else if (fmt == OutputFormat::quiet) {
				if (skip_enumeration) {
					std::cout << "~" << format_estimate(estimated_count) << "\n";
				} else {
					std::cout << count << (truncated ? "+" : "") << "\n";
				}
			} else {
				// ASCII number line visualization
				constexpr int width = 72;
				std::cout << "  " << ops.type_tag << " in ["
				          << lo << ", " << hi << "]\n";
				if (skip_enumeration) {
					std::cout << "  estimated values: ~" << format_estimate(estimated_count) << "\n";
					std::cout << "  (too many to enumerate; narrow the range for visualization)\n";
					if (wide_type) {
						std::cout << "  NOTE: type is wider than double; estimate may be inaccurate\n";
					}
				} else {
					std::cout << "  representable values: " << count
					          << (truncated ? " (truncated at 100000)" : "") << "\n";
					if (wide_type) {
						std::cout << "  NOTE: type is wider than double; count may underestimate\n";
					}
				}
				std::cout << "\n";

				if (skip_enumeration) {
					// No visualization -- already printed estimate above
				} else if (count == 0) {
					std::cout << "  (no representable values in range)\n";
				} else if (count <= 200) {
					// Render the number line
					std::string line_marks(width, ' ');
					std::string line_ticks(width, ' ');

					// Place each value on the line
					for (double val : values) {
						int pos = static_cast<int>((val - lo) / (hi - lo) * (width - 1));
						if (pos < 0) pos = 0;
						if (pos >= width) pos = width - 1;
						line_marks[pos] = '|';
					}

					// Place a few reference labels
					auto place_label = [&](double val, const std::string& label) {
						int pos = static_cast<int>((val - lo) / (hi - lo) * (width - 1));
						if (pos < 0 || pos >= width) return;
						int lstart = pos - static_cast<int>(label.size()) / 2;
						if (lstart < 0) lstart = 0;
						if (lstart + static_cast<int>(label.size()) > width)
							lstart = width - static_cast<int>(label.size());
						for (size_t c = 0; c < label.size(); ++c) {
							line_ticks[lstart + c] = label[c];
						}
					};

					// Labels at endpoints and midpoint
					{
						std::ostringstream ss;
						ss << std::setprecision(4) << lo;
						place_label(lo, ss.str());
					}
					{
						std::ostringstream ss;
						ss << std::setprecision(4) << hi;
						place_label(hi, ss.str());
					}
					{
						double mid = (lo + hi) * 0.5;
						std::ostringstream ss;
						ss << std::setprecision(4) << mid;
						place_label(mid, ss.str());
					}

					std::cout << "  " << line_ticks << "\n";
					std::cout << "  " << line_marks << "\n";
				} else {
					// Too many values for ASCII art -- show density histogram
					constexpr int bins = 20;
					std::vector<int> histogram(bins, 0);
					double bin_width = (hi - lo) / bins;
					for (double val : values) {
						int bin = static_cast<int>((val - lo) / bin_width);
						if (bin < 0) bin = 0;
						if (bin >= bins) bin = bins - 1;
						++histogram[bin];
					}
					int max_count = *std::max_element(histogram.begin(), histogram.end());
					constexpr int bar_width = 40;
					for (int i = 0; i < bins; ++i) {
						double bin_lo_v = lo + i * bin_width;
						double bin_hi_v = bin_lo_v + bin_width;
						int bar_len = (max_count > 0)
						    ? static_cast<int>(static_cast<double>(histogram[i]) / max_count * bar_width)
						    : 0;
						std::cout << "  [" << std::setw(10) << std::setprecision(4) << bin_lo_v
						          << "," << std::setw(10) << std::setprecision(4) << bin_hi_v << ") "
						          << std::string(bar_len, '#')
						          << " " << histogram[i] << "\n";
					}
				}

				// Density summary: divide range into thirds, compare
				// center vs edges to detect clustering patterns.
				if (count > 1) {
					double third = (hi - lo) / 3.0;
					double lo_edge = lo + third;
					double hi_edge = hi - third;
					size_t lo_count = 0, center_count = 0, hi_count = 0;
					for (double val : values) {
						if (val < lo_edge) ++lo_count;
						else if (val > hi_edge) ++hi_count;
						else ++center_count;
					}
					if (lo_count > (center_count + hi_count) * 2) {
						std::cout << "  dense near " << lo << "  ------>  sparse near " << hi << "\n";
					} else if (hi_count > (center_count + lo_count) * 2) {
						std::cout << "  sparse near " << lo << "  ------>  dense near " << hi << "\n";
					} else if (center_count > (lo_count + hi_count) * 2) {
						std::cout << "  dense near center, sparse at edges\n";
					} else {
						std::cout << "  roughly uniform density\n";
					}
				}
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
			"ulp", "sweep", "testvec", "oracle", "steps", "trace", "cancel", "audit", "diverge", "quantize", "block",
			"dot", "clip", "increment", "decrement", "stochastic", "histogram", "heatmap", "numberline", "faithful", "color", "vars", "help", "quit", "exit", nullptr
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
			// Skip comment lines before semicolon splitting
			std::string trimmed = trim(line);
			if (!trimmed.empty() && trimmed[0] == '#') continue;
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
