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
	std::cout << "  range          Show symmetry range [maxneg ... minneg] 0 [minpos ... maxpos]\n";
	std::cout << "  precision      Show precision, epsilon, and numeric properties\n";
	std::cout << "  ulp <value>    Show ULP at the given value\n";
	std::cout << "  sweep <expr> for <var> in [a, b, n]\n";
	std::cout << "                 Evaluate across a range, show error vs double\n";
	std::cout << "  faithful <expr> Check if result is faithfully rounded\n";
	std::cout << "  color [on|off] Toggle ANSI color-coded bit fields in show\n";
	std::cout << "  vars           List defined variables\n";
	std::cout << "  help           Show this help\n";
	std::cout << "  quit / exit    Exit the calculator\n";
	std::cout << "\n";
	std::cout << "Expressions:\n";
	std::cout << "  Arithmetic:    +  -  *  /  ^  (parentheses)\n";
	std::cout << "  Functions:     sqrt, abs, log, exp, sin, cos, pow\n";
	std::cout << "  Constants:     pi, e, phi, ln2, ln10, sqrt2 (quad-double precision)\n";
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
	bool use_color;     // enable ANSI color_print output
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

	// color on/off
	if (line == "color on") {
		state.use_color = true;
		if (state.interactive) std::cout << "Color output enabled.\n";
		return true;
	}
	if (line == "color off") {
		state.use_color = false;
		if (state.interactive) std::cout << "Color output disabled.\n";
		return true;
	}
	if (line == "color") {
		std::cout << "Color output: " << (state.use_color ? "on" : "off") << "\n";
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
			// Find the short alias for each variable's type
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
			std::cout << "  value:      " << result.native_rep << "\n";
			if (state.use_color && !result.color_rep.empty()) {
				std::cout << "  color:      " << result.color_rep << "\n";
			} else {
				std::cout << "  binary:     " << result.binary_rep << "\n";
			}
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
		// Group types by bit width: small (1-32), medium (33-80), large (>80)
		// Small/medium: single-line with native precision
		// Large: two-line format (value on first line, binary on second)
		struct CompareEntry {
			std::string alias;
			std::string value;
			std::string binary;
			std::string error;
			int nbits;
		};
		std::vector<CompareEntry> small, medium, large;
		for (const auto& alias : state.registry.aliases()) {
			const TypeOps& ops = state.registry.get(alias);
			ExpressionEvaluator eval(ops);
			for (const auto& kv : state.evaluator->variables()) {
				eval.set_variable(kv.first, kv.second);
			}
			CompareEntry entry;
			entry.alias = alias;
			entry.nbits = ops.nbits;
			try {
				Value result = eval.evaluate(expr);
				entry.value = result.native_rep;
				entry.binary = result.binary_rep;
			} catch (const std::exception& ex) {
				entry.error = ex.what();
			}
			// Group by bit width and rendered value width:
			//   small:  nbits <= 32 and value fits in 22 chars
			//   medium: nbits 33-80 or value fits in 25 chars
			//   large:  nbits > 80 or value exceeds 25 chars
			int vlen = static_cast<int>(entry.value.size());
			if (entry.nbits > 80 || vlen > 25)              large.push_back(std::move(entry));
			else if (entry.nbits > 32 || vlen > 22)         medium.push_back(std::move(entry));
			else                                             small.push_back(std::move(entry));
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
		return true;
	}

	// bits <expr>
	if (line.substr(0, 5) == "bits " || line.substr(0, 5) == "bits\t") {
		std::string expr = trim(line.substr(5));
		try {
			Value result = state.evaluator->evaluate(expr);
			std::cout << "  type:   " << result.type_name << "\n";
			std::cout << "  binary: " << result.binary_rep << "\n";
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
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
			std::cout << ops.type_tag << "\n";
			std::cout << "[ " << vMaxneg.native_rep << " ... " << vMinneg.native_rep
			          << "  0  "
			          << vMinpos.native_rep << " ... " << vMaxpos.native_rep << " ]\n";
			std::cout << "[ " << vMaxneg.binary_rep << " ... " << vMinneg.binary_rep
			          << "  0  "
			          << vMinpos.binary_rep << " ... " << vMaxpos.binary_rep << " ]\n";
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
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
			// Compute binary digits from epsilon
			double eps = vEps.num;
			int binary_digits = 0;
			if (eps > 0.0 && eps < 1.0) {
				binary_digits = static_cast<int>(-std::log2(eps));
			}
			double decimal_digits = binary_digits * 0.30103; // log10(2)
			std::cout << "  type:           " << ops.type_tag << "\n";
			std::cout << "  binary digits:  " << binary_digits << "\n";
			std::cout << "  decimal digits: " << std::setprecision(1) << std::fixed
			          << decimal_digits << std::defaultfloat << "\n";
			std::cout << "  epsilon:        " << vEps.native_rep << "\n";
			std::cout << "  minpos:         " << vMinpos.native_rep << "\n";
			std::cout << "  maxpos:         " << vMaxpos.native_rep << "\n";
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
			// Compute ULP by probing the type dispatch through double interchange.
			// The estimate is correct for the active type's granularity but is
			// stored as double, so precision is clamped to double's max_digits10.
			double v = val.num;
			double ulp_est = 0.0;
			if (v == 0.0) {
				// ULP at zero: smallest representable positive value of the active type
				Value tiny = ops.minpos();
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
			// Clamp display precision: ULP is computed via double arithmetic
			int prec = std::min(ops.max_digits10 > 0 ? ops.max_digits10 : 17,
			                    std::numeric_limits<double>::max_digits10);
			std::cout << "  value: " << val.native_rep << "\n";
			std::cout << "  ulp:   " << std::setprecision(prec) << ulp_est << "\n";
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
		if (range_str.empty()) {
			std::cerr << "Usage: sweep <expr> for <var> in [<a>, <b>, <n>]\n";
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
				// Cap value columns like compare: truncate to fit
				auto cap = [](const std::string& s, int w) -> std::string {
					return (static_cast<int>(s.size()) > w) ? s.substr(0, w - 1) + "~" : s;
				};
				std::cout << std::left << std::setw(20) << std::setprecision(8) << xval
				          << std::right << std::setw(25) << cap(result.native_rep, 25)
				          << std::setw(25) << cap(ref.native_rep, 25)
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
			// Convert reference to active type (nearest representable)
			Value rounded = ops.from_double(ref.num);
			// Find the OTHER adjacent representable value (the neighbor on
			// the opposite side of the true value from rounded).
			// If rounded == ref exactly, the result is faithful iff it equals rounded.
			double neighbor_val = rounded.num;
			if (rounded.num < ref.num) {
				// True value is above rounded -- find next representable above
				// Use a binary search: start from a step and halve until we
				// find the smallest increment that changes the representation.
				double step = std::max(std::abs(rounded.num), 1.0);
				for (int i = 0; i < 200; ++i) {
					Value test = ops.from_double(rounded.num + step);
					if (test.num > rounded.num) {
						neighbor_val = test.num;
						// Try smaller step to tighten
						double smaller = step * 0.5;
						for (int j = 0; j < 60; ++j) {
							Value t2 = ops.from_double(rounded.num + smaller);
							if (t2.num <= rounded.num) break;
							neighbor_val = t2.num;
							smaller *= 0.5;
						}
						break;
					}
					step *= 2.0;
				}
			} else if (rounded.num > ref.num) {
				// True value is below rounded -- find next representable below
				double step = std::max(std::abs(rounded.num), 1.0);
				for (int i = 0; i < 200; ++i) {
					Value test = ops.from_double(rounded.num - step);
					if (test.num < rounded.num) {
						neighbor_val = test.num;
						double smaller = step * 0.5;
						for (int j = 0; j < 60; ++j) {
							Value t2 = ops.from_double(rounded.num - smaller);
							if (t2.num >= rounded.num) break;
							neighbor_val = t2.num;
							smaller *= 0.5;
						}
						break;
					}
					step *= 2.0;
				}
			}
			// A faithfully rounded result is either the nearest representable
			// (rounded) or the adjacent representable on the other side (neighbor).
			bool is_faithful = (result.num == rounded.num) || (result.num == neighbor_val);
			std::cout << "  result:    " << result.native_rep << "\n";
			std::cout << "  reference: " << ref.native_rep << "\n";
			std::cout << "  rounded:   " << rounded.native_rep << "\n";
			Value neighbor = ops.from_double(neighbor_val);
			std::cout << "  neighbor:  " << neighbor.native_rep << "\n";
			std::cout << "  faithful:  " << (is_faithful ? "yes" : "no") << "\n";
		} catch (const std::exception& ex) {
			std::cerr << "Error: " << ex.what() << "\n";
		}
		return true;
	}

	// Otherwise, evaluate as expression
	try {
		Value result = state.evaluator->evaluate(line);
		std::cout << result.native_rep << "\n";
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
			"type", "types", "show", "compare", "bits", "range", "precision",
			"ulp", "sweep", "faithful", "color", "vars", "help", "quit", "exit", nullptr
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
			"sqrt", "abs", "log", "exp", "sin", "cos", "pow",
			"pi", "e", "phi", "ln2", "ln10", "sqrt2", nullptr
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

	bool use_color = interactive && ISATTY(FILENO(stdout));
	ReplState state{ registry, &evaluator, active_type, interactive, use_color };

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
