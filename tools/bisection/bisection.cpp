// bisection.cpp: standalone REPL for the bisection number system framework
//
// A focused interactive calculator for exploring number systems built from
// the bisection coding framework (Lindstrom, CoNGA'19). Each registered
// type is one (Generator, Refinement, nbits) instantiation. The REPL
// reuses the type-erased TypeOps machinery from tools/ucalc/ via header
// inclusion only -- no ucalc source files are touched or rebuilt.
//
// Issue #691, Epic #687.
//
// Usage:
//   bisection                              # interactive REPL
//   bisection "type bisection_posit16; pi" # one-shot
//   bisection -t bisection_natposit16 "1/3"
//   bisection -f script.txt                # batch mode
//
// Supported commands:
//   type <name>           select active type
//   types                 list registered types
//   show <expr>           evaluate expression in active type
//   compare <expr>        evaluate expression across all registered types
//   range                 show maxpos/maxneg/minpos/minneg of active type
//   precision             show machine epsilon of active type
//   bits                  show nbits of active type
//   numberline            list all representable values (small types only)
//   sweep <a> <b> <n>     sample n values across [a, b] in active type
//   heatmap <a> <b> <n>   density profile (decade-bucketed sample)
//   help                  show this list
//   quit | exit | q       leave the REPL
//
// Any other input is parsed as an arithmetic expression and evaluated
// in the active type.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>

// Suppress arithmetic exceptions: the REPL handles errors at the prompt level
#define POSIT_THROW_ARITHMETIC_EXCEPTION 0
#define BISECTION_THROW_ARITHMETIC_EXCEPTION 0

#include <universal/utility/directives.hpp>
#include <universal/native/ieee754.hpp>

// Number systems exposed by this REPL
#include <universal/number/posit/posit.hpp>
#include <universal/number/bisection/bisection.hpp>

// Required by tools/ucalc/type_dispatch.hpp's constant_via_qd<T> template:
// the function body unconditionally names qd/dd and the various cascade
// types, so they must be visible at template instantiation even when the
// constexpr branch we hit is the generic one.
#define DD_THROW_ARITHMETIC_EXCEPTION 0
#define QD_THROW_ARITHMETIC_EXCEPTION 0
#include <universal/number/dd/dd.hpp>
#include <universal/number/qd/qd.hpp>
#include <universal/number/dd_cascade/dd_cascade.hpp>
#include <universal/number/td_cascade/td_cascade.hpp>
#include <universal/number/qd_cascade/qd_cascade.hpp>

// Reusable building blocks from tools/ucalc (headers only -- no rebuild of ucalc)
#include "type_dispatch.hpp"
#include "expression.hpp"
#include "output_format.hpp"

namespace sw { namespace bisection_repl {

using sw::ucalc::TypeOps;
using sw::ucalc::TypeRegistry;
using sw::ucalc::Value;
using sw::ucalc::ExpressionEvaluator;
using sw::ucalc::register_type;

// --- Registry construction ---------------------------------------------------

// Stamp a friendly variant tag onto an entry produced by the generic
// register_type<T> template. The template defaults type_tag to "bisection<N>"
// which loses the generator/refinement identity, so we override it here.
template <typename T>
inline TypeOps make_entry(const std::string& alias, const std::string& tag_suffix) {
	TypeOps ops = register_type<T>(alias);
	ops.type_tag = alias + " (" + tag_suffix + ")";
	return ops;
}

inline TypeRegistry build_bisection_registry() {
	using namespace sw::universal;
	TypeRegistry reg;

	// --- Bisection types from issue #691 ------------------------------------
	reg.add("bisection_unary8",
		make_entry<bisection_unary<8>>("bisection_unary8", "Unary, ArithmeticMean"));
	reg.add("bisection_unary16",
		make_entry<bisection_unary<16>>("bisection_unary16", "Unary, ArithmeticMean"));

	reg.add("bisection_fib8",
		make_entry<bisection_fibonacci<8>>("bisection_fib8", "Fibonacci, ArithmeticMean"));
	reg.add("bisection_fib16",
		make_entry<bisection_fibonacci<16>>("bisection_fib16", "Fibonacci, ArithmeticMean"));

	reg.add("bisection_posit8",
		make_entry<bisection_posit<8>>("bisection_posit8", "Posit(0) generator, HyperMean"));
	reg.add("bisection_posit16",
		make_entry<bisection_posit<16>>("bisection_posit16", "Posit(0) generator, HyperMean"));

	// High-precision AuxReal variants (issue #692): the interval bisection
	// arithmetic runs in dd (~31 digits) instead of double, giving tighter
	// round-trip encoding accuracy for wider nbits.
	reg.add("bisection_posit16_dd",
		make_entry<bisection_posit<16, 0, uint8_t, dd>>("bisection_posit16_dd", "Posit(0), HyperMean, dd AuxReal"));
	reg.add("bisection_posit32_dd",
		make_entry<bisection_posit<32, 0, uint8_t, dd>>("bisection_posit32_dd", "Posit(0), HyperMean, dd AuxReal"));
	reg.add("bisection_natposit16_dd",
		make_entry<bisection_natposit<16, 0, uint8_t, dd>>("bisection_natposit16_dd", "Posit(0), NaturalPosit, dd AuxReal"));

	reg.add("bisection_natposit8",
		make_entry<bisection_natposit<8>>("bisection_natposit8", "Posit(0), NaturalPosit refinement"));
	reg.add("bisection_natposit16",
		make_entry<bisection_natposit<16>>("bisection_natposit16", "Posit(0), NaturalPosit refinement"));

	reg.add("bisection_elias8",
		make_entry<bisection_elias_delta<8>>("bisection_elias8", "EliasDelta, HyperMean"));
	reg.add("bisection_elias16",
		make_entry<bisection_elias_delta<16>>("bisection_elias16", "EliasDelta, HyperMean"));

	reg.add("bisection_golden8",
		make_entry<bisection_golden<8>>("bisection_golden8", "GoldenRatio, ArithmeticMean"));
	reg.add("bisection_golden16",
		make_entry<bisection_golden<16>>("bisection_golden16", "GoldenRatio, ArithmeticMean"));

	// --- Reference posits for side-by-side comparison -----------------------
	reg.add("posit8",  register_type<posit<8,  2, uint8_t >>("posit8"));
	reg.add("posit16", register_type<posit<16, 2, uint16_t>>("posit16"));
	reg.add("posit32", register_type<posit<32, 2, uint32_t>>("posit32"));

	return reg;
}

// --- Helpers -----------------------------------------------------------------

inline std::string trim(const std::string& s) {
	size_t lo = 0, hi = s.size();
	while (lo < hi && std::isspace(static_cast<unsigned char>(s[lo]))) ++lo;
	while (hi > lo && std::isspace(static_cast<unsigned char>(s[hi - 1]))) --hi;
	return s.substr(lo, hi - lo);
}

inline std::vector<std::string> split_semis(const std::string& line) {
	std::vector<std::string> parts;
	std::string cur;
	for (char c : line) {
		if (c == ';') { parts.push_back(trim(cur)); cur.clear(); }
		else { cur += c; }
	}
	std::string last = trim(cur);
	if (!last.empty()) parts.push_back(last);
	return parts;
}

inline bool starts_with(const std::string& s, const std::string& prefix) {
	return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

// --- Command implementations -------------------------------------------------

void cmd_help() {
	std::cout <<
		"bisection REPL -- explore the bisection coding framework\n"
		"Commands:\n"
		"  type <name>            set the active type\n"
		"  types                  list registered types\n"
		"  show <expr>            evaluate <expr> and show full encoding\n"
		"  compare <expr>         evaluate <expr> across all registered types\n"
		"  range                  show range of the active type\n"
		"  precision              show machine epsilon of the active type\n"
		"  bits                   show nbits of the active type\n"
		"  numberline             enumerate all values (n <= 12 only)\n"
		"  sweep <a> <b> <n>      sample n values uniformly in [a,b]\n"
		"  heatmap <a> <b> <n>    decade-bucketed density of n samples\n"
		"  help                   show this help\n"
		"  quit | exit | q        leave the REPL\n"
		"\n"
		"Bare expressions are evaluated in the active type.\n";
}

void cmd_types(const TypeRegistry& reg) {
	std::cout << "Registered types (" << reg.aliases().size() << "):\n";
	for (const auto& alias : reg.aliases()) {
		const TypeOps* ops = reg.find(alias);
		std::cout << "  " << std::left << std::setw(22) << alias
		          << " " << (ops ? ops->type_tag : std::string("?"))
		          << "  [" << (ops ? ops->nbits : 0) << " bits]\n";
	}
}

void cmd_show(const TypeOps& ops, ExpressionEvaluator& ev, const std::string& expr) {
	Value v = ev.evaluate(expr);
	std::cout << ops.type_tag << "\n";
	std::cout << "  decimal : " << v.native_rep   << "\n";
	std::cout << "  binary  : " << v.binary_rep   << "\n";
	std::cout << "  components: " << v.components_rep << "\n";
}

void cmd_compare(const TypeRegistry& reg, const std::string& expr) {
	std::cout << "expression: " << expr << "\n";
	std::cout << std::left << std::setw(22) << "type"
	          << std::setw(28) << "value"
	          << "binary\n";
	for (const auto& alias : reg.aliases()) {
		const TypeOps& ops = reg.get(alias);
		try {
			ExpressionEvaluator local(ops);
			Value v = local.evaluate(expr);
			std::cout << std::left << std::setw(22) << alias
			          << std::setw(28) << v.native_rep
			          << v.binary_rep << "\n";
		}
		catch (const std::exception& e) {
			std::cout << std::left << std::setw(22) << alias
			          << "  <error: " << e.what() << ">\n";
		}
	}
}

void cmd_range(const TypeOps& ops) {
	std::cout << ops.type_tag << "\n";
	std::cout << "  maxpos = " << ops.maxpos().native_rep << "\n";
	std::cout << "  minpos = " << ops.minpos().native_rep << "\n";
	std::cout << "  maxneg = " << ops.maxneg().native_rep << "\n";
	std::cout << "  minneg = " << ops.minneg().native_rep << "\n";
	const double minpos_abs = std::abs(ops.minpos().num);
	const double maxpos_abs = std::abs(ops.maxpos().num);
	if (minpos_abs > 0.0 && maxpos_abs > 0.0) {
		const double dr = std::log10(maxpos_abs / minpos_abs);
		std::cout << "  dynamic range: ~" << std::fixed << std::setprecision(1)
		          << dr << " decades\n" << std::defaultfloat;
	} else {
		std::cout << "  dynamic range: undefined (minpos or maxpos is zero)\n";
	}
}

void cmd_precision(const TypeOps& ops) {
	Value eps = ops.epsilon();
	std::cout << ops.type_tag << "\n";
	std::cout << "  epsilon = " << eps.native_rep << "  (" << eps.binary_rep << ")\n";
	std::cout << "  max_digits10 = " << ops.max_digits10 << "\n";
}

void cmd_bits(const TypeOps& ops) {
	std::cout << ops.type_tag << ": " << ops.nbits << " bits\n";
}

void cmd_numberline(const TypeOps& ops) {
	if (ops.nbits > 12) {
		std::cout << "numberline: refusing to enumerate "
		          << (1ULL << ops.nbits) << " values; use sweep instead.\n";
		return;
	}
	if (!ops.next) {
		std::cout << "numberline: type does not expose ++/--\n";
		return;
	}
	// Walk from maxneg to maxpos by repeatedly applying next.
	Value v = ops.maxneg();
	const int limit = 1 << ops.nbits;
	std::cout << ops.type_tag << " (all " << limit << " encodings):\n";
	for (int i = 0; i < limit; ++i) {
		std::cout << std::setw(5) << i << "  "
		          << std::setw(22) << v.native_rep
		          << "  " << v.binary_rep << "\n";
		// Stop if next is a no-op (NaN region) or wraps
		Value n = ops.next(v);
		if (n.binary_rep == v.binary_rep) break;
		v = n;
	}
}

void cmd_sweep(const TypeOps& ops, double a, double b, int n) {
	if (n < 2) n = 2;
	std::cout << ops.type_tag << " sweep [" << a << ", " << b << "], "
	          << n << " samples:\n";
	for (int i = 0; i < n; ++i) {
		double t = a + (b - a) * (double(i) / double(n - 1));
		Value v = ops.from_double(t);
		std::cout << "  in=" << std::setw(14) << t
		          << "  enc=" << std::setw(20) << v.native_rep
		          << "  " << v.binary_rep << "\n";
	}
}

void cmd_heatmap(const TypeOps& ops, double a, double b, int n) {
	if (n < 2) n = 2;
	if (a <= 0.0) a = 1e-9;
	if (b <= a) b = a * 1e3;
	double la = std::log10(a);
	double lb = std::log10(b);
	const int decades = std::max(1, int(std::ceil(lb - la)));
	std::vector<int> buckets(decades, 0);
	for (int i = 0; i < n; ++i) {
		double t = std::pow(10.0, la + (lb - la) * (double(i) / double(n - 1)));
		Value v = ops.from_double(t);
		double dec = std::log10(std::abs(v.num) + 1e-300);
		int b_idx = std::clamp(int(dec - la), 0, decades - 1);
		buckets[b_idx]++;
	}
	std::cout << ops.type_tag << " heatmap [" << a << ", " << b << "], "
	          << n << " samples across " << decades << " decade(s):\n";
	int maxc = 1;
	for (int c : buckets) maxc = std::max(maxc, c);
	for (int d = 0; d < decades; ++d) {
		double lo = std::pow(10.0, la + d);
		int width = (buckets[d] * 40) / maxc;
		std::cout << "  1e" << std::setw(3) << int(la + d)
		          << " (" << std::setw(8) << lo << ")  "
		          << std::string(width, '#')
		          << " " << buckets[d] << "\n";
	}
}

// --- Dispatcher --------------------------------------------------------------

struct ReplState {
	TypeRegistry registry;
	const TypeOps* active = nullptr;
	std::string active_name;
};

void set_active(ReplState& s, const std::string& name) {
	const TypeOps* ops = s.registry.find(name);
	if (!ops) {
		std::cout << "unknown type: " << name << "\n";
		return;
	}
	s.active = ops;
	s.active_name = name;
	std::cout << "active type: " << ops->type_tag << "\n";
}

// Returns false to request REPL exit.
bool dispatch(ReplState& s, const std::string& raw) {
	std::string line = trim(raw);
	if (line.empty() || line[0] == '#') return true;

	if (line == "quit" || line == "exit" || line == "q") return false;
	if (line == "help" || line == "?") { cmd_help(); return true; }
	if (line == "types") { cmd_types(s.registry); return true; }

	if (starts_with(line, "type ")) {
		set_active(s, trim(line.substr(5)));
		return true;
	}
	auto require_active = [&]() {
		if (!s.active) {
			std::cout << "set a type first (try 'type bisection_posit16')\n";
			return false;
		}
		return true;
	};
	if (line == "range")     { if (require_active()) cmd_range(*s.active); return true; }
	if (line == "precision") { if (require_active()) cmd_precision(*s.active); return true; }
	if (line == "bits")      { if (require_active()) cmd_bits(*s.active); return true; }
	if (line == "numberline"){ if (require_active()) cmd_numberline(*s.active); return true; }

	if (starts_with(line, "show ")) {
		if (!s.active) { std::cout << "set a type first (try 'type bisection_posit16')\n"; return true; }
		try {
			ExpressionEvaluator ev(*s.active);
			cmd_show(*s.active, ev, line.substr(5));
		} catch (const std::exception& e) {
			std::cout << "error: " << e.what() << "\n";
		}
		return true;
	}
	if (starts_with(line, "compare ")) {
		try { cmd_compare(s.registry, line.substr(8)); }
		catch (const std::exception& e) { std::cout << "error: " << e.what() << "\n"; }
		return true;
	}
	if (starts_with(line, "sweep ")) {
		if (!s.active) { std::cout << "set a type first\n"; return true; }
		std::istringstream iss(line.substr(6));
		double a, b; int n;
		if (!(iss >> a >> b >> n)) { std::cout << "usage: sweep <a> <b> <n>\n"; return true; }
		cmd_sweep(*s.active, a, b, n);
		return true;
	}
	if (starts_with(line, "heatmap ")) {
		if (!s.active) { std::cout << "set a type first\n"; return true; }
		std::istringstream iss(line.substr(8));
		double a, b; int n;
		if (!(iss >> a >> b >> n)) { std::cout << "usage: heatmap <a> <b> <n>\n"; return true; }
		cmd_heatmap(*s.active, a, b, n);
		return true;
	}

	// Bare expression: evaluate in active type
	if (!s.active) {
		std::cout << "set a type first (try 'type bisection_posit16')\n";
		return true;
	}
	try {
		ExpressionEvaluator ev(*s.active);
		Value v = ev.evaluate(line);
		std::cout << v.native_rep << "  (" << v.binary_rep << ")\n";
	}
	catch (const std::exception& e) {
		std::cout << "error: " << e.what() << "\n";
	}
	return true;
}

void run_line(ReplState& s, const std::string& line, bool& keep_going) {
	for (const auto& part : split_semis(line)) {
		if (!dispatch(s, part)) { keep_going = false; return; }
	}
}

int run_repl(ReplState& s) {
	std::cout << "bisection REPL -- type 'help' for commands, 'quit' to exit\n";
	if (s.active) std::cout << "active type: " << s.active->type_tag << "\n";
	std::string line;
	bool keep_going = true;
	while (keep_going && std::cin) {
		std::cout << "bisect> " << std::flush;
		if (!std::getline(std::cin, line)) break;
		run_line(s, line, keep_going);
	}
	return 0;
}

int run_file(ReplState& s, const std::string& path) {
	std::ifstream in(path);
	if (!in) { std::cerr << "cannot open: " << path << "\n"; return 4; }
	std::string line;
	bool keep_going = true;
	while (keep_going && std::getline(in, line)) {
		run_line(s, line, keep_going);
	}
	return 0;
}

void print_usage() {
	std::cout <<
		"Usage: bisection [options] [expression]\n"
		"  -t, --type <name>   set active type before evaluating\n"
		"  -f, --file <path>   read commands from a script file\n"
		"  -h, --help          show this help\n"
		"      --types         list registered types and exit\n"
		"With no expression and no -f, runs an interactive REPL.\n";
}

}} // namespace sw::bisection_repl

int main(int argc, char** argv) {
	using namespace sw::bisection_repl;

	ReplState state;
	state.registry = build_bisection_registry();
	// Default active type: bisection_posit16 (a useful starting point).
	if (const auto* def = state.registry.find("bisection_posit16")) {
		state.active = def;
		state.active_name = "bisection_posit16";
	}

	std::string script_file;
	std::vector<std::string> positional;
	for (int i = 1; i < argc; ++i) {
		std::string a = argv[i];
		if (a == "-h" || a == "--help") { print_usage(); return 0; }
		if (a == "--types") { cmd_types(state.registry); return 0; }
		if ((a == "-t" || a == "--type") && i + 1 < argc) {
			set_active(state, argv[++i]);
			continue;
		}
		if ((a == "-f" || a == "--file") && i + 1 < argc) {
			script_file = argv[++i];
			continue;
		}
		positional.push_back(a);
	}

	if (!script_file.empty()) {
		return run_file(state, script_file);
	}

	if (!positional.empty()) {
		// One-shot: join all positional args, run as a single semicolon-separated batch
		std::string joined;
		for (size_t i = 0; i < positional.size(); ++i) {
			if (i) joined += " ";
			joined += positional[i];
		}
		bool keep_going = true;
		run_line(state, joined, keep_going);
		return 0;
	}

	return run_repl(state);
}
