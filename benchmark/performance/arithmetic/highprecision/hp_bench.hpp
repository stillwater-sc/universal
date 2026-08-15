#pragma once
//  hp_bench.hpp : measurement harness for the high-precision multi-component benchmarks
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// This harness supports the dd/dd_cascade and qd/qd_cascade performance comparison (issue #1315).
// Design constraints that shaped it:
//
// 1. the types under test differ in speed by three orders of magnitude (double vs qd division),
//    so a fixed operation count either takes forever on the slow types or is unmeasurable on the
//    fast ones. Every measurement therefore calibrates its own operation count to a target
//    wall-clock window and reports the normalized ns/op and ops/sec.
// 2. the whole point is to compare a cascade type against its classic counterpart, so the harness
//    remembers every measurement and prints the cascade/classic ratio matrix at the end.
// 3. multi-component arithmetic is exactly the kind of code an optimizer loves to delete: the
//    error-free transformations are pure arithmetic on values the compiler can often see through.
//    Every workload must hand its result to consume(), which stores through a volatile.
//
// NOTE: this header deliberately lives outside namespace sw::universal. The workloads call sqrt(),
// exp(), etc. unqualified so that ADL selects the number system's implementation, and universal's
// 'dd sqrt(double)' convenience overloads would make those calls ambiguous for the double baseline
// if the workloads were visible to namespace sw::universal.
#include <universal/utility/directives.hpp>
#include <universal/utility/architecture.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#if defined(__linux__)
#include <fstream>
#endif

namespace hpbench {

	////////////////////////////////////////////////////////////////////////////////////////////////
	/// optimizer barrier

	// the sink is volatile, so every store to it is a side effect the compiler must keep,
	// which in turn keeps the computation that produced the value
	inline volatile double g_sink = 0.0;

	// How a value reaches the sink. The primary template covers the single-component types, where
	// the value is its own leading component. hp_types.hpp specializes this for each
	// multi-component type: consuming only the leading component is not enough, because converting
	// a qd to a double hands the compiler limb 0 and it is then free to prove that the other three
	// limbs are dead and skip the work that produced them.
	template<typename Scalar>
	struct componentSink {
		static void store(const Scalar& v) { g_sink = double(v); }
	};

	template<typename Scalar>
	inline void consume(const Scalar& v) { componentSink<Scalar>::store(v); }

	////////////////////////////////////////////////////////////////////////////////////////////////
	/// deterministic sample data

	// a small LCG: reproducible across compilers and platforms, and cheap enough that generating
	// the operands never shows up in the measurement (the tables are generated once, outside the loop)
	class Lcg {
	public:
		Lcg(std::uint64_t seed = 0x2545F4914F6CDD1Dull) : state{ seed } {}
		std::uint64_t next() {
			state = state * 6364136223846793005ull + 1442695040888963407ull;
			return state >> 11;
		}
		// uniform in [0, 1)
		double uniform() { return double(next() % (1ull << 53)) / double(1ull << 53); }
		// uniform in [lo, hi)
		double uniform(double lo, double hi) { return lo + (hi - lo) * uniform(); }
	private:
		std::uint64_t state;
	};

	// operands in [0.5, 2.0): close to 1.0 so that long dependent chains neither overflow nor
	// collapse to zero, and so that log() and sqrt() stay in their domain
	inline std::vector<double> sampleData(std::size_t nrSamples, std::uint64_t seed = 0xC0FFEEull) {
		Lcg rng(seed);
		std::vector<double> data(nrSamples);
		for (std::size_t i = 0; i < nrSamples; ++i) data[i] = rng.uniform(0.5, 2.0);
		return data;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	/// measurement

	struct Measurement {
		std::string op;
		std::string type;
		std::size_t ops;
		double      elapsed;    // seconds, best of N repeats
		double      nsPerOp;
		double      opsPerSec;
	};

	// convert a rate to an engineering-notation string, i.e. 1234567 -> "  1 Mops/sec"
	inline std::string toRate(double value) {
		const char* scales[] = { " ", "K", "M", "G", "T", "P" };
		std::size_t scale = 0;
		while (value >= 1000.0 && scale < 5) { value /= 1000.0; ++scale; }
		std::stringstream ss;
		ss << std::setw(6) << std::fixed << std::setprecision(1) << value << ' ' << scales[scale] << "ops/sec";
		return ss.str();
	}

	class Suite {
	public:
		Suite(const std::string& title, double targetSeconds = 0.05, int repeats = 3)
			: _title{ title }, _target{ targetSeconds }, _repeats{ repeats } {}

		// run 'workload' with an operation count calibrated to the target window, report ns/op
		// workload is any callable with signature void(std::size_t nrOps) that performs
		// exactly nrOps elementary operations of the kind being measured
		// minOps is the granularity of the workload: a kernel that cannot subdivide below one
		// invocation (a 32x32 matmul is 32768 element ops) must not be calibrated below it, or
		// the reported operation count is smaller than the work actually performed
		template<typename Workload>
		void measure(const std::string& op, const std::string& type, Workload&& workload, std::size_t minOps = 256) {
			// calibrate: grow the operation count until the workload fills the target window
			std::size_t nrOps = minOps;
			double elapsed = time(workload, nrOps);
			while (elapsed < _target && nrOps < (1ull << 34)) {
				nrOps *= 2;
				elapsed = time(workload, nrOps);
			}
			// measure: best of _repeats, as the fastest run is the one least disturbed by the machine
			for (int i = 1; i < _repeats; ++i) elapsed = std::min(elapsed, time(workload, nrOps));

			Measurement m;
			m.op = op;
			m.type = type;
			m.ops = nrOps;
			m.elapsed = elapsed;
			m.nsPerOp = 1.0e9 * elapsed / double(nrOps);
			m.opsPerSec = double(nrOps) / elapsed;
			record(m);
			report(m);
		}

		void reportEnvironment() const {
			std::cout << _title << '\n';
			std::cout << std::string(_title.size(), '=') << '\n';
			reportPlatform();
			std::cout << "  measurement    : best of " << _repeats << ", operation count calibrated to a "
				<< std::fixed << std::setprecision(3) << _target << " sec window\n\n";
		}

		// ns/op matrix: one row per operation, one column per type, in registration order
		void reportLatency() const {
			std::cout << "\nlatency (nsec/op, lower is better)\n";
			header();
			for (const auto& op : _ops) {
				std::cout << std::left << std::setw(_opWidth) << op << std::right;
				for (const auto& type : _types) {
					std::cout << ' ' << std::setw(_colWidth) << cell(op, type, [](const Measurement& m) { return m.nsPerOp; });
				}
				std::cout << '\n';
			}
		}

		// throughput matrix: the same data as ops/sec, which is the unit the rest of the
		// performance suite reports in
		void reportThroughput() const {
			std::cout << "\nthroughput (Mops/sec, higher is better)\n";
			header();
			for (const auto& op : _ops) {
				std::cout << std::left << std::setw(_opWidth) << op << std::right;
				for (const auto& type : _types) {
					std::cout << ' ' << std::setw(_colWidth) << cell(op, type, [](const Measurement& m) { return m.opsPerSec / 1.0e6; });
				}
				std::cout << '\n';
			}
		}

		// the headline of this benchmark: what does the floatcascade<N> generalization cost
		// relative to the hand-specialized classic implementation
		void reportRatios(const std::vector<std::pair<std::string, std::string>>& pairs) const {
			std::cout << "\nnormalized cost (ratio of nsec/op, 1.00 is parity, >1 means the first is slower)\n";
			std::cout << std::left << std::setw(_opWidth) << "operation" << std::right;
			for (const auto& p : pairs) {
				std::cout << ' ' << std::setw(_ratioWidth) << (p.first + "/" + p.second);
			}
			std::cout << '\n';
			std::cout << std::string(_opWidth + pairs.size() * (_ratioWidth + 1), '-') << '\n';
			for (const auto& op : _ops) {
				std::cout << std::left << std::setw(_opWidth) << op << std::right;
				for (const auto& p : pairs) {
					const Measurement* a = find(op, p.first);
					const Measurement* b = find(op, p.second);
					std::stringstream ss;
					if (a == nullptr || b == nullptr || b->nsPerOp == 0.0) {
						ss << '-';
					}
					else {
						ss << std::fixed << std::setprecision(2) << (a->nsPerOp / b->nsPerOp);
					}
					std::cout << ' ' << std::setw(_ratioWidth) << ss.str();
				}
				std::cout << '\n';
			}
		}

		const std::vector<Measurement>& measurements() const { return _measurements; }

	private:
		std::string              _title;
		double                   _target;
		int                      _repeats;
		std::vector<Measurement> _measurements;
		std::vector<std::string> _ops;    // registration order, so the tables read like the source
		std::vector<std::string> _types;

		static constexpr int _opWidth = 22;
		static constexpr int _colWidth = 12;
		static constexpr int _ratioWidth = 22;

		template<typename Workload>
		static double time(Workload&& workload, std::size_t nrOps) {
			using namespace std::chrono;
			steady_clock::time_point begin = steady_clock::now();
			workload(nrOps);
			steady_clock::time_point end = steady_clock::now();
			return duration_cast<duration<double>>(end - begin).count();
		}

		void record(const Measurement& m) {
			_measurements.push_back(m);
			if (std::find(_ops.begin(), _ops.end(), m.op) == _ops.end()) _ops.push_back(m.op);
			if (std::find(_types.begin(), _types.end(), m.type) == _types.end()) _types.push_back(m.type);
		}

		static void report(const Measurement& m) {
			std::cout << "  " << std::left << std::setw(_opWidth) << m.op
				<< std::setw(12) << m.type << std::right
				<< std::setw(12) << m.ops << " ops in "
				<< std::setw(9) << std::fixed << std::setprecision(6) << m.elapsed << " sec -> "
				<< std::setw(10) << std::setprecision(2) << m.nsPerOp << " nsec/op   "
				<< toRate(m.opsPerSec) << '\n';
		}

		const Measurement* find(const std::string& op, const std::string& type) const {
			for (const auto& m : _measurements) {
				if (m.op == op && m.type == type) return &m;
			}
			return nullptr;
		}

		template<typename Extractor>
		std::string cell(const std::string& op, const std::string& type, Extractor extract) const {
			const Measurement* m = find(op, type);
			if (m == nullptr) return std::string("-");
			std::stringstream ss;
			ss << std::fixed << std::setprecision(2) << extract(*m);
			return ss.str();
		}

		void header() const {
			std::cout << std::left << std::setw(_opWidth) << "operation" << std::right;
			for (const auto& type : _types) std::cout << ' ' << std::setw(_colWidth) << type;
			std::cout << '\n';
			std::cout << std::string(_opWidth + _types.size() * (_colWidth + 1), '-') << '\n';
		}

		void reportPlatform() const {
			std::cout << "  processor      : " << cpuModel() << '\n';
			std::cout << "  architecture   : " << architecture() << '\n';
			std::cout << "  compiler       : " << compilerId() << '\n';
			std::cout << "  C++ standard   : " << __cplusplus << '\n';
			std::cout << "  ISA extensions : " << isaExtensions() << '\n';
			std::cout << "  assertions     : " <<
#if defined(NDEBUG)
				"off (NDEBUG defined, this is a release build)"
#else
				"ON (this is NOT a release build: the numbers below are not representative)"
#endif
				<< '\n';
		}

		static std::string cpuModel() {
#if defined(__linux__)
			std::ifstream cpuinfo("/proc/cpuinfo");
			std::string line;
			while (std::getline(cpuinfo, line)) {
				const std::string key("model name");
				if (line.compare(0, key.size(), key) == 0) {
					std::string::size_type colon = line.find(':');
					if (colon != std::string::npos) {
						std::string::size_type start = line.find_first_not_of(" \t", colon + 1);
						if (start != std::string::npos) return line.substr(start);
					}
				}
			}
#endif
			return std::string("unknown");
		}

		// a one-line compiler identification: the same source compiled by gcc and clang produces
		// materially different code for the error-free transformations, so the results are
		// meaningless without it
		static std::string compilerId() {
			std::stringstream ss;
#if defined(__clang__)
			ss << "clang " << __clang_major__ << '.' << __clang_minor__ << '.' << __clang_patchlevel__;
#elif defined(__INTEL_COMPILER)
			ss << "icc " << __INTEL_COMPILER;
#elif defined(_MSC_VER)
			ss << "msvc " << _MSC_FULL_VER;
#elif defined(__GNUC__)
			ss << "gcc " << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
#else
			ss << "unknown";
#endif
			return ss.str();
		}

		static std::string architecture() {
#if defined(UNIVERSAL_ARCH_X86_64)
			return std::string("x86-64");
#elif defined(UNIVERSAL_ARCH_ARM)
			return std::string("ARM");
#elif defined(UNIVERSAL_ARCH_POWER)
			return std::string("POWER");
#elif defined(UNIVERSAL_ARCH_RISCV)
			return std::string("RISC-V");
#else
			return std::string("unknown");
#endif
		}

		// the EFT sequences at the heart of both families are sensitive to these: an FMA lets the
		// compiler contract the two_prod error term into a single instruction, which changes both
		// the instruction count and, when the code is not fma-aware, the result
		static std::string isaExtensions() {
			// the list starts at the baseline and grows, so it is never empty and the floor is
			// stated rather than implied
			std::string extensions("baseline");
#if defined(__SSE3__)
			extensions += " SSE3";
#endif
#if defined(__AVX__)
			extensions += " AVX";
#endif
#if defined(__AVX2__)
			extensions += " AVX2";
#endif
#if defined(__AVX512F__)
			extensions += " AVX512F";
#endif
#if defined(__FMA__)
			extensions += " FMA";
#endif
#if defined(LIB_USE_AVX2)
			extensions += " (UNIVERSAL_USE_AVX2=ON)";
#endif
			return extensions;
		}
	};

	// benchmarks accept an optional target window in seconds so that a developer can trade
	// measurement time for stability without recompiling: ./benchmark_highprecision_scalar 0.25
	inline double targetWindow(int argc, char** argv, double defaultSeconds = 0.05) {
		if (argc > 1) {
			// atof reports failure as 0.0 and happily accepts "inf", which would let the
			// calibration loop run to its 2^34 operation ceiling on the slowest workloads
			double seconds = std::atof(argv[1]);
			if (std::isfinite(seconds) && seconds > 0.0) return seconds;
		}
		return defaultSeconds;
	}

	// the pairings that answer the question this benchmark exists to answer
	inline std::vector<std::pair<std::string, std::string>> cascadeVsClassic() {
		return {
			{ "dd_cascade", "dd" },
			{ "qd_cascade", "qd" },
			{ "td_cascade", "dd" },
			{ "dd",         "double" },
			{ "qd",         "double" }
		};
	}

} // namespace hpbench
