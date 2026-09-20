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
// Usage: characterize [maxDepth=3] [reps=3]
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
#include <utility>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>
#include <universal/number/bfloat16/bfloat16.hpp>
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

	// build the exact dyadic value of an ereal from its (non-overlapping) limbs.
	// from_fp, not from_double: a long double limb would be rounded to 53 bits by the
	// latter, which is precisely the precision the wide limb exists to carry (#1568).
	template<unsigned N, typename FpType>
	dyadic ereal_to_dyadic(const ereal<N, FpType>& x) {
		dyadic acc;  // 0
		for (FpType limb : x.limbs()) acc = acc + dyadic::from_fp(limb);
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
		std::string fptype;  // elreal host block type; always "double" for ereal
		std::string func;
		long        knob;    // elreal depth or ereal N
		double      time_ns;
		int         digits;  // -1 if not measured
	};

	std::vector<Row> g_rows;

	void emit(const Row& r) {
		std::cout << r.type << ',' << r.fptype << ',' << r.func << ',' << r.knob << ','
		          << std::llround(r.time_ns) << ',' << r.digits << ','
		          << (r.digits < 0 ? 0 : correct_bits(r.digits)) << ',';
		if (r.digits < 0) std::cout << "n/a";
		else              std::cout << std::scientific << std::setprecision(1) << rel_error(r.digits) << std::defaultfloat;
		std::cout << '\n';
		g_rows.push_back(r);
	}

	// ----------------------------------------------------------------- sweeps
	// There is deliberately no exception guard around the sweep body.
	//
	// An earlier revision caught std::exception per cell and reported it as "this
	// host ran out of range", which is wrong in both directions. The math paths
	// reach infsum, which throws a bare std::runtime_error whose own message says
	// "non-convergence bug"; elreal's exception family (elreal_sum_budget_exceeded,
	// elreal_divide_by_zero) carries no host-range type at all; and std::bad_alloc
	// is a std::exception too. Catching them all would turn a genuine defect, or an
	// allocation failure, into a silently truncated sweep that reads as a normal
	// narrow-host limit -- the exact failure mode this tool exists to detect.
	//
	// Nor is the guard needed for the hosts swept here: double, float and bfloat16
	// all carry an 8-bit or wider exponent, and a probe of all nine functions on
	// all three hosts throws nothing. The 5-bit fp16 host is the one that would
	// exhaust its range, and it is not supported yet (it needs the online-division
	// floor-lift). When it lands, the right answer is an explicit range-failure
	// exception type to catch here, not a blanket handler.
	//
	// Accuracy saturation -- what the issue actually asked to be gated -- is not an
	// exception at all: it shows up as a digit count that stops rising, which the
	// CSV records and summary() reports.
	template<typename FpType>
	void run_elreal(const char* host, int maxDepth, int reps) {
		for (const auto& c : cases()) {
			for (int d = 2; d <= maxDepth; ++d) {
				{
					elreal<FpType> x(c.arg);
					x.precision(static_cast<std::size_t>(d));
					volatile double sink = 0.0;
					double t = time_ns([&] {
						elreal<FpType> r = apply(c.op, x);
						sink = r.template approx<double>(static_cast<std::size_t>(d));  // force materialization to depth d
					}, reps);
					(void)sink;
					elreal<FpType> r = apply(c.op, x);
					(void)r.template approx<double>(static_cast<std::size_t>(d));       // force before reading the stream
					int digits = agreed_decimal_digits(zbcl_to_dyadic(r.stream()), c.ref);
					emit({ "elreal", host, c.name, d, t, digits });
				}
			}
		}
	}

	// ereal arithmetic operators are inline and visible in this TU, so a plain
	// fixed-input result can be folded/hoisted out of the timing loop -- read a
	// limb through a volatile sink each rep to keep the work observable.
	template<unsigned N, typename FpType>
	double sink_limb(const ereal<N, FpType>& r) {
		return r.limbs().empty() ? 0.0 : static_cast<double>(r.limbs()[0]);
	}

	template<unsigned N, typename FpType>
	void run_ereal_N(const char* fptype, int reps) {
		// A limb list is shared across limb types, but max_safe_limbs is not: 5 for float,
		// 19 for double, 255 for x87, 144 for binary128. Skip the entries this limb type
		// cannot hold rather than failing to compile (#1568).
		if constexpr (N <= ereal<1, FpType>::max_safe_limbs) {
			for (const auto& c : cases()) {
				ereal<N, FpType> x(c.arg), r;
				volatile double sink = 0.0;
				double t = time_ns([&] { r = apply(c.op, x); sink = sink_limb(r); }, reps);
				(void)sink;
				int digits = agreed_decimal_digits(ereal_to_dyadic(r), c.ref);
				emit({ "ereal", fptype, c.name, static_cast<long>(N), t, digits });
			}
		}
		else {
			(void)fptype; (void)reps;
		}
	}

	template<typename FpType, unsigned... Ns>
	void run_ereal(const char* fptype, int reps) {
		(run_ereal_N<Ns, FpType>(fptype, reps), ...);   // fixed compile-time limb list
	}

	// arithmetic: accuracy is exact for a single op, so report time only (digits=-1).
	// The interesting axis is the renorm cost growth (elreal ~depth^2, ereal ~N^2).
	template<typename FpType>
	void run_arithmetic(const char* host, int maxDepth, int reps) {
		const double A = 1.4142135623730951, B = 2.7182818284590452;
		for (int d = 2; d <= maxDepth; ++d) {
			{
				elreal<FpType> a(A), b(B);
				a.precision(static_cast<std::size_t>(d));
				b.precision(static_cast<std::size_t>(d));
				volatile double sink = 0.0;
				auto timeOp = [&](auto fn) {
					return time_ns([&] { elreal<FpType> r = fn(); sink = r.template approx<double>(static_cast<std::size_t>(d)); }, reps);
				};
				emit({ "elreal", host, "add", d, timeOp([&] { return a + b; }), -1 });
				emit({ "elreal", host, "mul", d, timeOp([&] { return a * b; }), -1 });
				emit({ "elreal", host, "div", d, timeOp([&] { return a / b; }), -1 });
				(void)sink;
			}
		}
	}

	template<unsigned N, typename FpType>
	void run_arithmetic_ereal_N(const char* fptype, int reps) {
		if constexpr (N <= ereal<1, FpType>::max_safe_limbs) {
			const double A = 1.4142135623730951, B = 2.7182818284590452;
			ereal<N, FpType> a(A), b(B), r;
			volatile double sink = 0.0;
			auto timeOp = [&](auto fn) { return time_ns([&] { r = fn(); sink = sink_limb(r); }, reps); };
			emit({ "ereal", fptype, "add", static_cast<long>(N), timeOp([&] { return a + b; }), -1 });
			emit({ "ereal", fptype, "mul", static_cast<long>(N), timeOp([&] { return a * b; }), -1 });
			emit({ "ereal", fptype, "div", static_cast<long>(N), timeOp([&] { return a / b; }), -1 });
			(void)sink;
		}
		else {
			(void)fptype; (void)reps;
		}
	}

	template<typename FpType, unsigned... Ns>
	void run_arithmetic_ereal(const char* fptype, int reps) {
		(run_arithmetic_ereal_N<Ns, FpType>(fptype, reps), ...);
	}

	// The long double rows go through a template so that LD is a template parameter.
	// `if constexpr` only leaves its discarded branch uninstantiated inside a template:
	// in a plain function the branch is still instantiated, and naming
	// ereal<1, long double>::max_safe_limbs there completes the class and trips its
	// static_assert. That is exactly what happens on ppc64le, where the default
	// long double is IBM double-double and not a valid limb -- the ppc64le cross build
	// caught it (#1568).
	template<typename LD>
	void report_long_double_limbs() {
		if constexpr (is_expansion_limb_v<LD>) {
			std::cout << " long double=" << ereal<1, LD>::max_safe_limbs;
		}
	}

	template<typename LD>
	void run_long_double_limbs(int reps) {
		if constexpr (is_expansion_limb_v<LD>) {
			run_ereal<LD, 2, 4, 8, 12, 16>("long double", reps);
			run_arithmetic_ereal<LD, 2, 4, 8, 12, 16>("long double", reps);
		}
		else {
			(void)reps;
		}
	}

	// ---------------------------------------------------------------- summary
	// per (type, function): saturation knob (first reaching >=95% of max digits)
	// and the accuracy/time knee (knob maximizing digits per log10(time)).
	void summary() {
		std::vector<std::string> funcs;
		for (const auto& c : cases()) funcs.push_back(c.name);

		// the distinct (type, host) series present, in first-seen order. Grouping by
		// type alone would pool every host's rows together and report a saturation
		// point that belongs to none of them.
		std::vector<std::pair<std::string, std::string>> series;
		for (const auto& r : g_rows) {
			std::pair<std::string, std::string> key{ r.type, r.fptype };
			if (std::find(series.begin(), series.end(), key) == series.end()) series.push_back(key);
		}

		std::cout << "\n== per-function summary (accuracy saturation + accuracy/time knee) ==\n";
		for (const auto& [type, host] : series) {
			for (const auto& f : funcs) {
				std::vector<const Row*> rs;
				for (const auto& r : g_rows)
					if (r.type == type && r.fptype == host && r.func == f && r.digits >= 0) rs.push_back(&r);
				if (rs.empty()) continue;
				std::sort(rs.begin(), rs.end(), [](const Row* a, const Row* b) { return a->knob < b->knob; });
				int maxDig = 0;
				for (auto* r : rs) maxDig = std::max(maxDig, r->digits);

				// Saturation means additional knob buys (almost) nothing, and we can
				// only claim it if the plateau STARTED BEFORE the sweep ended. Taking
				// "first knob within 95% of the best" on its own is not enough: for a
				// series that is still climbing, the best IS the last row, so the test
				// fires at the sweep limit and reports the operator's choice of maxDepth
				// back to them as a property of the type. elreal does exactly this --
				// since v4.9.0 its accuracy is linear in the knob and unbounded, so it
				// has no saturation point at any depth (#1177).
				long satKnob = rs.back()->knob;
				bool saturated = false;
				for (auto* r : rs) {
					if (r->digits >= (maxDig * 95) / 100) {
						satKnob = r->knob;
						saturated = (r != rs.back()) && (rs.size() > 1);
						break;
					}
				}
				const Row* knee = rs.front();
				double bestScore = -1.0;
				for (auto* r : rs) {
					double score = r->digits / std::max(1.0, std::log10(std::max(1.0, r->time_ns)));
					if (score > bestScore) { bestScore = score; knee = r; }
				}
				// The knee has the same failure mode: digits/log10(time) rises
				// monotonically for a series that never plateaus, so the "best" is
				// again just the last row. Only call it a knee when a plateau was seen.
				const bool kneeIsSweepLimit = (knee == rs.back());
				const char* unit = (type == "elreal" ? "depth " : "N=");
				const std::string label = type + "<" + host + ">";
				std::cout << "  " << std::left << std::setw(17) << label << ' ' << std::setw(9) << f;
				if (saturated) {
					std::cout << " saturates ~" << unit << satKnob << " (" << maxDig << " digits)";
				}
				else {
					std::cout << " NO saturation through " << unit << rs.back()->knob
					          << " (" << maxDig << " digits, still climbing)";
				}
				if (saturated || !kneeIsSweepLimit) {
					std::cout << "; knee at " << unit << knee->knob
					          << " (" << knee->digits << " digits, " << std::llround(knee->time_ns) << " ns)";
				}
				else {
					std::cout << "; no knee -- accuracy/time still improving at " << unit << knee->knob
					          << " (" << knee->digits << " digits, " << std::llround(knee->time_ns) << " ns)";
				}
				std::cout << '\n';
			}
		}

		// elreal-vs-ereal: cheapest config reaching a target accuracy, per function
		const int target = 30;  // digits
		std::cout << "\n== elreal vs ereal: cheapest config reaching >=" << target << " digits ==\n";
		for (const auto& f : funcs) {
			auto cheapest = [&](const std::string& type, const std::string& host) -> const Row* {
				const Row* best = nullptr;
				for (const auto& r : g_rows)
					if (r.type == type && r.fptype == host && r.func == f && r.digits >= target)
						if (!best || r.time_ns < best->time_ns) best = &r;
				return best;
			};
			// the best elreal host for this function, whichever it turns out to be
			const Row* e = nullptr;
			std::string eHost;
			for (const auto& [type, host] : series) {
				if (type != "elreal") continue;
				const Row* cand = cheapest(type, host);
				if (cand && (!e || cand->time_ns < e->time_ns)) { e = cand; eHost = host; }
			}
			const Row* r = cheapest("ereal", "double");
			std::cout << "  " << std::left << std::setw(9) << f << ' ';
			if (e) std::cout << "elreal<" << eHost << "> depth " << e->knob << " @ " << std::llround(e->time_ns) << " ns";
			else   std::cout << "elreal (not reached on any host)";
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
	std::cout << "# elreal depth sweep 2.." << maxDepth << " over hosts {double, float, bfloat16},"
	          << " ereal limb list {2,4,8,12,16} over limb types {float, double";
	if constexpr (is_expansion_limb_v<long double>) std::cout << ", long double";
	std::cout << "}, reps=" << reps << "\n";
	std::cout << "# long double here: digits=" << std::numeric_limits<long double>::digits
	          << (is_expansion_limb_v<long double> ? "" : " -- not a valid limb on this host") << "\n";
	// a limb list entry past a limb type's max_safe_limbs is skipped, not an error
	std::cout << "# max_safe_limbs: float=" << ereal<1, float>::max_safe_limbs
	          << " double=" << ereal<1, double>::max_safe_limbs;
	report_long_double_limbs<long double>();
	std::cout << "\n";
	std::cout << "type,FpType,function,depth,time_ns,correct_digits,correct_bits,rel_error\n";

	// Both types are templated on the arithmetic they are built from, and the
	// accuracy/time curve moves with it: a narrower one carries fewer significand bits
	// per block or limb, so it needs more of them for the same accuracy.
	run_elreal<double>("double", maxDepth, reps);
	run_elreal<float>("float", maxDepth, reps);
	run_elreal<bfloat16>("bfloat16", maxDepth, reps);
	run_ereal<float,  2, 4, 8, 12, 16>("float", reps);
	run_ereal<double, 2, 4, 8, 12, 16>("double", reps);
	run_arithmetic<double>("double", maxDepth, reps);
	run_arithmetic<float>("float", maxDepth, reps);
	run_arithmetic<bfloat16>("bfloat16", maxDepth, reps);
	run_arithmetic_ereal<float,  2, 4, 8, 12, 16>("float", reps);
	run_arithmetic_ereal<double, 2, 4, 8, 12, 16>("double", reps);
	// long double is x87 extended on x86-64 and MinGW, binary128 on aarch64 and
	// riscv64, and the default IBM double-double on ppc64le -- which is itself a
	// two-component expansion and is not a valid limb, so the trait rejects it and
	// these rows are simply absent there.
	//
	// EXPECT binary128 TO BE SLOW. On aarch64 and riscv64 it has no hardware and every
	// operation is a libgcc soft-float call, so it is slower than double by a large
	// factor -- typically 10x or more -- for about twice the significand. x87 is
	// hardware, but its 80-bit loads and stores are not free either. The wider limb is
	// for reach, not for speed: it is what takes ereal past double's ~303-digit
	// ceiling (#1355), and this sweep is here to keep that trade honest rather than
	// to advertise the wide limb as faster.
	run_long_double_limbs<long double>(reps);

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
