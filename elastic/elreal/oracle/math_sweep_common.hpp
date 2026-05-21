#pragma once
// math_sweep_common.hpp: shared inputs + reporting for elreal oracle sweeps
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase J of #903 (follow-up to #873). The Phase G oracle helper
// `check_against_elreal_oracle` lives in
// `include/sw/universal/verification/test_suite_elreal_oracle.hpp`. This
// header bundles the per-type sweep machinery for the multi-component
// types `dd`, `qd`, `dd_cascade`, `td_cascade`, `qd_cascade`, and
// `ereal<N>` so each per-type sweep cpp can stay short.
//
// Precision ceiling note
// ----------------------
// Today's elreal caps refinement at depth 1, which holds the helper's
// effective comparison at double precision (~ 15 decimal digits). The
// dd / qd / cascade types target 31 / 64 / 32 / 48 / 64 digits
// respectively, so the helper currently *cannot* detect drift below
// the 15th decimal. Phase L (#906) and Phase M (#907) of the follow-up
// epic lift this ceiling; the sweep API does not change when they
// land. Until then, a passing sweep means "no cross-implementation
// disagreement at double precision", not "all bits of the target
// type are correct".

#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <universal/number/elreal/elreal.hpp>
#include <universal/verification/test_suite_elreal_oracle.hpp>

namespace sw { namespace universal { namespace elreal_oracle_sweep {

// Representative inputs for unary functions over the real line. The
// span is chosen to exercise typical, small-magnitude, and modestly
// large arguments. None push into the std-library's accuracy-loss
// regions (Payne-Hanek for trig is a Phase N concern).
inline const std::vector<double>& general_inputs() {
	static const std::vector<double> inputs = {
		-2.5, -1.0, -0.5, -0.125, 0.0, 0.125, 0.5, 1.0, 1.5, 2.0, 3.0, 5.0
	};
	return inputs;
}

// Positive-only inputs for functions whose natural domain is x > 0
// (sqrt, log, log2, log10, log1p uses x > -1).
inline const std::vector<double>& positive_inputs() {
	static const std::vector<double> inputs = {
		0.125, 0.5, 1.0, 1.5, 2.0, 3.0, 5.0, 10.0, 100.0
	};
	return inputs;
}

// [-1, 1] domain for asin / acos / atanh.
inline const std::vector<double>& bounded_inputs() {
	static const std::vector<double> inputs = {
		-0.875, -0.5, -0.125, 0.0, 0.125, 0.5, 0.875
	};
	return inputs;
}

// (-1, inf) domain for log1p. Stays strictly above -1.
inline const std::vector<double>& log1p_inputs() {
	static const std::vector<double> inputs = {
		-0.875, -0.5, -0.125, 0.0, 0.125, 0.5, 1.0, 2.0, 5.0
	};
	return inputs;
}

// [1, inf) domain for acosh.
inline const std::vector<double>& acosh_inputs() {
	static const std::vector<double> inputs = {
		1.0, 1.125, 1.5, 2.0, 3.0, 5.0, 10.0
	};
	return inputs;
}

// Run a unary function on both sides of the oracle pipeline and report
// a single FAIL line per disagreement.
//
//   - target_fn:  takes a TargetType, returns a TargetType
//   - oracle_fn:  takes an elreal, returns an elreal
//   - label_fn:   produces the human-readable label for the input
template<typename TargetType, typename TargetFn, typename OracleFn>
int sweep_unary(
	const char* function_name,
	const std::vector<double>& inputs,
	TargetFn target_fn,
	OracleFn oracle_fn,
	int safety_margin = 2)
{
	int failures = 0;
	for (double x : inputs) {
		TargetType target = target_fn(TargetType(x));
		elreal     oracle = oracle_fn(elreal(x));

		std::string label = std::string(function_name) + "(" + std::to_string(x) + ")";
		failures += report_against_elreal_oracle(label.c_str(),
		                                        target, oracle,
		                                        safety_margin);
	}
	return failures;
}

// Binary function sweep. Inputs come in (x, y) pairs.
template<typename TargetType, typename TargetFn, typename OracleFn>
int sweep_binary(
	const char* function_name,
	const std::vector<std::pair<double, double>>& inputs,
	TargetFn target_fn,
	OracleFn oracle_fn,
	int safety_margin = 2)
{
	int failures = 0;
	for (auto& xy : inputs) {
		double x = xy.first, y = xy.second;
		TargetType target = target_fn(TargetType(x), TargetType(y));
		elreal     oracle = oracle_fn(elreal(x), elreal(y));

		std::string label = std::string(function_name) + "(" + std::to_string(x)
			+ ", " + std::to_string(y) + ")";
		failures += report_against_elreal_oracle(label.c_str(),
		                                        target, oracle,
		                                        safety_margin);
	}
	return failures;
}

// Integer-exponent pown sweep: TargetType pown(target, int n) compared
// against elreal pow(oracle, elreal(n)). elreal does not currently
// expose a free-function pown overload; pow with an integer-valued
// elreal exponent walks the same depth-1 derivative correction path
// and is the right oracle for testing pown.
template<typename TargetType, typename TargetFn>
int sweep_pown(
	const char* function_name,
	const std::vector<std::pair<double, int>>& inputs,
	TargetFn target_fn,
	int safety_margin = 2)
{
	using sw::universal::pow;
	int failures = 0;
	for (auto& xn : inputs) {
		double x = xn.first;
		int    n = xn.second;
		TargetType target = target_fn(TargetType(x), n);
		elreal     oracle = pow(elreal(x), elreal(static_cast<double>(n)));

		std::string label = std::string(function_name) + "(" + std::to_string(x)
			+ ", " + std::to_string(n) + ")";
		failures += report_against_elreal_oracle(label.c_str(),
		                                        target, oracle,
		                                        safety_margin);
	}
	return failures;
}

// Anchor checks: deterministic exact-result expectations that catch
// gross bugs that any random-input pass would miss (e.g. exp(0) == 1
// regardless of sign convention).
template<typename TargetType, typename TargetFn, typename OracleFn>
int anchor_check(
	const char* anchor_label,
	double x,
	double expected,
	TargetFn target_fn,
	OracleFn oracle_fn)
{
	TargetType target = target_fn(TargetType(x));
	elreal     oracle = oracle_fn(elreal(x));
	if (double(target) != expected || double(oracle) != expected) {
		std::cerr << "FAIL: " << anchor_label
			<< " target=" << double(target)
			<< " oracle=" << double(oracle)
			<< " expected=" << expected << "\n";
		return 1;
	}
	return 0;
}

} } } // namespace sw::universal::elreal_oracle_sweep
