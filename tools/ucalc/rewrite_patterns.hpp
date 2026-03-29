#pragma once
// rewrite_patterns.hpp: database of numerically unstable patterns and stable alternatives
//
// Each pattern describes:
//   - A numerically unstable expression form
//   - Its stable alternative
//   - The condition under which the rewrite helps
//   - A brief explanation of why the original is unstable
//
// These patterns are used by the `rewrites` command (listing) and will be
// used by the `suggest` command (Phase 4: AST matching) to detect and
// suggest reformulations.
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project.
#include <string>
#include <vector>

namespace sw { namespace ucalc {

struct RewritePattern {
	std::string id;              // short identifier, e.g. "sqrt_diff"
	std::string name;            // human-readable name
	std::string unstable;        // unstable expression form
	std::string stable;          // stable alternative
	std::string condition;       // when the rewrite helps
	std::string explanation;     // why the original is unstable
};

// The canonical rewrite pattern database
inline const std::vector<RewritePattern>& rewrite_database() {
	static const std::vector<RewritePattern> db = {
		{
			"sqrt_diff",
			"Square root difference",
			"sqrt(a) - sqrt(b)",
			"(a - b) / (sqrt(a) + sqrt(b))",
			"a, b exact inputs, a ~= b > 0",
			"When a ~= b, sqrt(a) ~= sqrt(b), and the subtraction cancels "
			"most significant digits. The conjugate form computes the same "
			"value via division, avoiding the cancellation."
		},
		{
			"quadratic_unstable",
			"Quadratic formula (unstable root)",
			"(-b + sqrt(b^2 - 4*a*c)) / (2*a)",
			"2*c / (-b - sqrt(b^2 - 4*a*c))",
			"when -b and sqrt(b^2-4ac) nearly cancel (b^2 >> 4ac)",
			"The standard quadratic formula subtracts two nearly equal values "
			"when the discriminant is close to b^2. The alternative formula "
			"uses division instead, preserving precision for the small root."
		},
		{
			"log1p",
			"Logarithm near 1",
			"log(1 + x)",
			"log1p(x)",
			"|x| << 1",
			"When x is small, 1+x rounds to 1 in finite precision, losing x "
			"entirely. log1p(x) computes log(1+x) directly without forming 1+x, "
			"preserving the contribution of x."
		},
		{
			"expm1",
			"Exponential minus 1",
			"exp(x) - 1",
			"expm1(x)",
			"|x| << 1",
			"When x is small, exp(x) ~= 1+x, so exp(x)-1 subtracts two nearly "
			"equal values. expm1(x) computes exp(x)-1 directly, avoiding the "
			"cancellation near zero."
		},
		{
			"half_angle",
			"One minus cosine (half-angle)",
			"1 - cos(x)",
			"2 * sin(x/2)^2",
			"|x| << 1",
			"When x is small, cos(x) ~= 1 - x^2/2, so 1-cos(x) cancels most "
			"digits. The half-angle identity 2*sin^2(x/2) computes the same "
			"value without subtraction of nearly equal quantities."
		},
		{
			"sin_diff",
			"Sine difference (product-to-sum)",
			"sin(a) - sin(b)",
			"2 * cos((a+b)/2) * sin((a-b)/2)",
			"a ~= b",
			"When a ~= b, sin(a) ~= sin(b) and the subtraction cancels digits. "
			"The product-to-sum identity factors the difference into a product "
			"that avoids the near-cancellation."
		},
		{
			"cos_ratio",
			"Cosine deviation ratio",
			"(1 - cos(x)) / x^2",
			"(sin(x/2) / (x/2))^2 / 2",
			"|x| << 1",
			"Combines the 1-cos(x) cancellation with division by x^2. "
			"The rewrite uses the sinc-like form sin(x/2)/(x/2) which is "
			"well-conditioned near zero (limit is 1/2)."
		}
	};
	return db;
}

}} // namespace sw::ucalc
