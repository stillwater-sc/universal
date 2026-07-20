// characterize.cpp: accuracy-vs-compute-time characterization for the adaptive-
//                   precision real oracles elreal (lazy ZBCL) and ereal (Priest/
//                   Shewchuk expansion).
//
// For each function it sweeps the precision knob -- elreal `depth`, ereal limb
// count `N` -- and reports, per (type, function, knob):
//   * wall-clock time per evaluation (median over repeats), and
//   * accuracy as decimal digits of agreement with an independent ~320-digit
//     reference, via the exact dyadic oracle (verification/elreal_reference_digits.hpp
//     for elreal, a limb-sum dyadic for ereal), plus correct bits and rel error.
// Output is a CSV to stdout followed by a per-function saturation/knee summary and
// an elreal-vs-ereal comparison (which type is cheaper for a target accuracy).
//
// Usage: characterize [maxDepth=4] [reps=5]
//   maxDepth  highest elreal depth to sweep (2..maxDepth); ereal sweeps a fixed
//             limb list {2,4,8,12,16}. Raise for a full characterization run.
//   reps      timing repeats (median).
//
// This is a MEASUREMENT tool only -- it does not optimize the implementations.
// Resolves issue #1040.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
#include <universal/utility/directives.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <universal/number/elreal/elreal.hpp>
#include <universal/number/ereal/ereal.hpp>
#include <universal/verification/elreal_reference_digits.hpp>  // dyadic, zbcl_to_dyadic, agreed_decimal_digits
#include <math/constants/reference_constants.hpp>              // s_pi, s_e, s_ln2, s_sqrt2, s_sin_half, ...

namespace {

	using namespace sw::universal;

	// ------------------------------------------------------------------ timing
	double median(std::vector<double>& v) {
		if (v.empty()) return 0.0;
		std::sort(v.begin(), v.end());
		const size_t n = v.size();
		return (n % 2) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
	}

	template<typename F>
	double time_ns(F&& f, int reps) {
		std::vector<double> t;
		t.reserve(static_cast<size_t>(reps));
		for (int i = 0; i < reps; ++i) {
			auto a = std::chrono::steady_clock::now();
			f();
			auto b = std::chrono::steady_clock::now();
			t.push_back(std::chrono::duration<double, std::nano>(b - a).count());
		}
		return median(t);
	}

	// ---------------------------------------------------------------- accuracy
	constexpr double kLog2of10 = 3.321928094887362;

	int correct_bits(int digits) { return static_cast<int>(std::lround(digits * kLog2of10)); }

	// rel error implied by d matching decimal digits (a reporting estimate)
	double rel_error(int digits) { return (digits <= 0) ? 1.0 : std::pow(10.0, -digits); }

	// build the exact dyadic value of an ereal from its (non-overlapping) limbs
	template<unsigned N>
	dyadic ereal_to_dyadic(const ereal<N>& x) {
		dyadic acc;  // 0
		for (double limb : x.limbs()) acc = acc + dyadic::from_double(limb);
		return acc;
	}

	// ------------------------------------------------------------------- cases
	enum class Op { Sqrt, Exp, Log, Sin, Cos, Tan, Sinh, Cosh, Tanh };

	struct Case {
		Op               op;
		const char*      name;
		double           arg;
		std::string_view ref;   // ~320-digit reference for f(arg)
	};

	const std::vector<Case>& cases() {
		static const std::vector<Case> c = {
			{ Op::Sqrt, "sqrt@2",  2.0, s_sqrt2   },
			{ Op::Exp,  "exp@1",   1.0, s_e       },
			{ Op::Log,  "log@2",   2.0, s_ln2     },
			{ Op::Sin,  "sin@0.5", 0.5, s_sin_half },
			{ Op::Cos,  "cos@0.5", 0.5, s_cos_half },
			{ Op::Tan,  "tan@0.5", 0.5, s_tan_half },
			{ Op::Sinh, "sinh@0.5",0.5, s_sinh_half },
			{ Op::Cosh, "cosh@0.5",0.5, s_cosh_half },
			{ Op::Tanh, "tanh@0.5",0.5, s_tanh_half },
		};
		return c;
	}

	template<typename Real>
	Real apply(Op op, const Real& x) {
		switch (op) {
		case Op::Sqrt: return sqrt(x);
		case Op::Exp:  return exp(x);
		case Op::Log:  return log(x);
		case Op::Sin:  return sin(x);
		case Op::Cos:  return cos(x);
		case Op::Tan:  return tan(x);
		case Op::Sinh: return sinh(x);
		case Op::Cosh: return cosh(x);
		case Op::Tanh: return tanh(x);
		}
		return x;
	}

	// ------------------------------------------------------------------- rows
	struct Row {
		std::string type;    // "elreal" | "ereal"
		std::string func;
		long        knob;    // elreal depth or ereal N
		double      time_ns;
		int         digits;  // -1 if not measured
	};

	std::vector<Row> g_rows;

	void emit(const Row& r) {
		std::cout << r.type << ",double," << r.func << ',' << r.knob << ','
		          << std::llround(r.time_ns) << ',' << r.digits << ','
		          << (r.digits < 0 ? 0 : correct_bits(r.digits)) << ',';
		if (r.digits < 0) std::cout << "n/a";
		else              std::cout << std::scientific << std::setprecision(1) << rel_error(r.digits) << std::defaultfloat;
		std::cout << '\n';
		g_rows.push_back(r);
	}

	// ----------------------------------------------------------------- sweeps
	void run_elreal(int maxDepth, int reps) {
		for (const auto& c : cases()) {
			for (int d = 2; d <= maxDepth; ++d) {
				elreal<double> x(c.arg);
				x.precision(static_cast<std::size_t>(d));
				volatile double sink = 0.0;
				double t = time_ns([&] {
					elreal<double> r = apply(c.op, x);
					sink = r.approx<double>(static_cast<std::size_t>(d));  // force materialization to depth d
				}, reps);
				(void)sink;
				elreal<double> r = apply(c.op, x);
				(void)r.approx<double>(static_cast<std::size_t>(d));       // force before reading the stream
				int digits = agreed_decimal_digits(zbcl_to_dyadic(r.stream()), c.ref);
				emit({ "elreal", c.name, d, t, digits });
			}
		}
	}

	template<unsigned N>
	void run_ereal_N(int reps) {
		for (const auto& c : cases()) {
			ereal<N> x(c.arg), r;
			double t = time_ns([&] { r = apply(c.op, x); }, reps);
			int digits = agreed_decimal_digits(ereal_to_dyadic(r), c.ref);
			emit({ "ereal", c.name, static_cast<long>(N), t, digits });
		}
	}

	template<unsigned... Ns>
	void run_ereal(int reps) {
		(run_ereal_N<Ns>(reps), ...);   // fixed compile-time limb list
	}

	// arithmetic: accuracy is exact for a single op, so report time only (digits=-1).
	// The interesting axis is the renorm cost growth (elreal ~depth^2, ereal ~N^2).
	void run_arithmetic(int maxDepth, int reps) {
		const double A = 1.4142135623730951, B = 2.7182818284590452;
		for (int d = 2; d <= maxDepth; ++d) {
			elreal<double> a(A), b(B);
			a.precision(static_cast<std::size_t>(d));
			b.precision(static_cast<std::size_t>(d));
			volatile double sink = 0.0;
			auto timeOp = [&](auto fn) {
				return time_ns([&] { elreal<double> r = fn(); sink = r.approx<double>(static_cast<std::size_t>(d)); }, reps);
			};
			emit({ "elreal", "add", d, timeOp([&] { return a + b; }), -1 });
			emit({ "elreal", "mul", d, timeOp([&] { return a * b; }), -1 });
			emit({ "elreal", "div", d, timeOp([&] { return a / b; }), -1 });
			(void)sink;
		}
	}

	template<unsigned N>
	void run_arithmetic_ereal_N(int reps) {
		const double A = 1.4142135623730951, B = 2.7182818284590452;
		ereal<N> a(A), b(B), r;
		emit({ "ereal", "add", static_cast<long>(N), time_ns([&] { r = a + b; }, reps), -1 });
		emit({ "ereal", "mul", static_cast<long>(N), time_ns([&] { r = a * b; }, reps), -1 });
		emit({ "ereal", "div", static_cast<long>(N), time_ns([&] { r = a / b; }, reps), -1 });
	}

	template<unsigned... Ns>
	void run_arithmetic_ereal(int reps) {
		(run_arithmetic_ereal_N<Ns>(reps), ...);
	}

	// ---------------------------------------------------------------- summary
	// per (type, function): saturation knob (first reaching >=95% of max digits)
	// and the accuracy/time knee (knob maximizing digits per log10(time)).
	void summary() {
		std::vector<std::string> funcs;
		for (const auto& c : cases()) funcs.push_back(c.name);

		std::cout << "\n== per-function summary (accuracy saturation + accuracy/time knee) ==\n";
		for (const char* type : { "elreal", "ereal" }) {
			for (const auto& f : funcs) {
				std::vector<const Row*> rs;
				for (const auto& r : g_rows)
					if (r.type == type && r.func == f && r.digits >= 0) rs.push_back(&r);
				if (rs.empty()) continue;
				std::sort(rs.begin(), rs.end(), [](const Row* a, const Row* b) { return a->knob < b->knob; });
				int maxDig = 0;
				for (auto* r : rs) maxDig = std::max(maxDig, r->digits);
				long satKnob = rs.back()->knob;
				for (auto* r : rs) if (r->digits >= (maxDig * 95) / 100) { satKnob = r->knob; break; }
				const Row* knee = rs.front();
				double bestScore = -1.0;
				for (auto* r : rs) {
					double score = r->digits / std::max(1.0, std::log10(std::max(1.0, r->time_ns)));
					if (score > bestScore) { bestScore = score; knee = r; }
				}
				std::cout << "  " << std::left << std::setw(7) << type << ' ' << std::setw(9) << f
				          << " saturates ~" << (std::string(type) == "elreal" ? "depth " : "N=") << satKnob
				          << " (" << maxDig << " digits); knee at "
				          << (std::string(type) == "elreal" ? "depth " : "N=") << knee->knob
				          << " (" << knee->digits << " digits, " << std::llround(knee->time_ns) << " ns)\n";
			}
		}

		// elreal-vs-ereal: cheapest config reaching a target accuracy, per function
		const int target = 30;  // digits
		std::cout << "\n== elreal vs ereal: cheapest config reaching >=" << target << " digits ==\n";
		for (const auto& f : funcs) {
			auto cheapest = [&](const char* type) -> const Row* {
				const Row* best = nullptr;
				for (const auto& r : g_rows)
					if (r.type == type && r.func == f && r.digits >= target)
						if (!best || r.time_ns < best->time_ns) best = &r;
				return best;
			};
			const Row* e = cheapest("elreal");
			const Row* r = cheapest("ereal");
			std::cout << "  " << std::left << std::setw(9) << f << ' ';
			if (e) std::cout << "elreal depth " << e->knob << " @ " << std::llround(e->time_ns) << " ns";
			else   std::cout << "elreal (not reached)";
			std::cout << "  vs  ";
			if (r) std::cout << "ereal N=" << r->knob << " @ " << std::llround(r->time_ns) << " ns";
			else   std::cout << "ereal (not reached)";
			if (e && r) std::cout << "  -> " << (e->time_ns < r->time_ns ? "elreal" : "ereal") << " cheaper";
			std::cout << '\n';
		}
	}

}  // anonymous namespace

int main(int argc, char** argv) try {
	// Defaults are deliberately small so a first look is quick; the elreal
	// transcendentals cost 100s of ms at depth >= 4, so raise maxDepth for a full
	// characterization run (e.g. `characterize 6 9`).
	int maxDepth = (argc > 1) ? std::atoi(argv[1]) : 3;
	int reps     = (argc > 2) ? std::atoi(argv[2]) : 3;
	if (maxDepth < 2) maxDepth = 2;
	if (reps < 1) reps = 1;

	std::cout << "# adaptive-precision accuracy-vs-compute-time characterization (issue #1040)\n";
	std::cout << "# elreal depth sweep 2.." << maxDepth << ", ereal limb list {2,4,8,12,16}, reps=" << reps << "\n";
	std::cout << "type,FpType,function,depth,time_ns,correct_digits,correct_bits,rel_error\n";

	run_elreal(maxDepth, reps);
	run_ereal<2, 4, 8, 12, 16>(reps);
	run_arithmetic(maxDepth, reps);
	run_arithmetic_ereal<2, 4, 8, 12, 16>(reps);

	summary();
	return EXIT_SUCCESS;
}
catch (const std::exception& e) {
	std::cerr << "Caught exception: " << e.what() << std::endl;
	return EXIT_FAILURE;
}
catch (...) {
	std::cerr << "Caught unknown exception" << std::endl;
	return EXIT_FAILURE;
}
