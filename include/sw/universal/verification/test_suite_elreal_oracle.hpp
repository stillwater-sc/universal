#pragma once
// test_suite_elreal_oracle.hpp: elreal as a cross-validation oracle (Phase G of #873)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// =============================================================================
// elreal as a cross-validation oracle
// =============================================================================
//
// Issue #880 (Phase G of epic #873) wires `elreal` into Universal's
// validation pipeline as the higher-precision reference for cross-validating
// other multi-component types (`dd`, `qd`, `dd_cascade`, `td_cascade`,
// `qd_cascade`, `ereal<N>`).
//
// The pipeline pattern
// --------------------
//
//   1. Compute the value under test in the target type (e.g.
//      `dd_cascade y = exp(dd_cascade(x))`).
//   2. Compute the same expression via an INDEPENDENT path through `elreal`
//      (e.g. `elreal o = exp(elreal(x))`). The two paths use different
//      arithmetic backends -- Bailey/Hida hand-crafted cascade ops on one
//      side, McCleeary lazy refinement on the other -- so disagreement is
//      a strong bug signal.
//   3. Call `check_against_elreal_oracle(y, o, safety_margin)`, which
//      compares the two at the target type's advertised
//      `numeric_limits::digits10` precision.
//
// Current precision ceiling
// -------------------------
//
// elreal's lazy refinement currently caps at depth-1 for arithmetic ops
// and math functions (~53 bits for double-input cases, ~106 bits for
// rational-input cases). That cap is what makes the function bodies in
// Phase C-E small and easy to validate; lifting it to depth-2+ requires
// either (a) Newton refinement using lazy division (Phase F-or-later
// follow-up), or (b) a per-function higher-precision algorithm internally.
//
// Until that work lands, this helper compares both sides at *double*
// precision. The validation it provides today is therefore at the level
// of the existing `check_relative_error` helper, with one key difference:
// the reference is computed by an independent code path (elreal lazy)
// rather than directly via the std library. Cross-implementation
// agreement is a non-trivial bug signal even at matched precision.
//
// When deeper elreal refinement lands, this helper picks up the
// precision improvement automatically -- the API surface does not change,
// only the elreal oracle gets sharper.
//
// What the helper catches today
// -----------------------------
//
//   - Target-type implementations that produce a *wrong sign* or
//     *wrong magnitude* (catastrophic bugs); both sides disagree
//     at double precision.
//   - Target-type implementations that *match std lib* but disagree
//     with the lazy-real algebraic path; indicates a subtle
//     algorithmic issue.
//   - Regressions in target-type precision that drop below double's
//     ~16 decimal digits.
//
// What the helper does NOT yet catch
// ----------------------------------
//
//   - Sub-double-ULP precision drift in the target type (e.g.
//     dd_cascade losing the lower limb's precision). That requires
//     elreal to deliver >53 bits, which is the depth-2+ follow-up.
//
// See docs/algorithmic-details/multi-component-arithmetic.md section 8.7
// for the broader validation-strategy context.

#include <cmath>
#include <limits>

#include <universal/number/elreal/elreal.hpp>

namespace sw { namespace universal {

// Compare a Universal-type value against an elreal oracle. Returns true if
// the relative error |double(target) - double(oracle)| / |double(oracle)|
// is within the target type's advertised tolerance (derived from
// numeric_limits<TargetType>::digits10 minus a safety margin).
//
// safety_margin is the number of decimal digits of headroom; default 2
// (matches the convention used by `get_adaptive_threshold` in
// test_suite_mathlib_adaptive.hpp).
template<typename TargetType>
inline bool check_against_elreal_oracle(
	const TargetType& target,
	const elreal& oracle,
	int safety_margin = 2)
{
	using std::numeric_limits;

	// Tolerance derived from target's claimed decimal precision. For a
	// target type whose digits10 exceeds double's 15, we cap at double
	// precision today (see header docblock); when elreal gains depth-2+
	// refinement this cap goes away.
	constexpr int target_digits10 = numeric_limits<TargetType>::digits10;
	constexpr int double_digits10 = numeric_limits<double>::digits10;
	int effective_digits = target_digits10;
	if (effective_digits > double_digits10) effective_digits = double_digits10;
	int margin = safety_margin;
	if (margin >= effective_digits) margin = effective_digits - 1;
	if (margin < 0) margin = 0;
	double rel_tol = std::pow(10.0, -static_cast<double>(effective_digits - margin));

	double t = double(target);
	double o = double(oracle);

	// NaN handling: both NaN -> match; either NaN -> mismatch.
	if (std::isnan(t) && std::isnan(o)) return true;
	if (std::isnan(t) || std::isnan(o)) return false;

	// Infinity handling: both inf with same sign -> match.
	if (std::isinf(t) && std::isinf(o)) return (t > 0.0) == (o > 0.0);
	if (std::isinf(t) || std::isinf(o)) return false;

	// Near-zero oracle: switch to absolute tolerance to avoid divide-by-zero.
	if (o == 0.0) return std::abs(t) < rel_tol;

	return std::abs((t - o) / o) < rel_tol;
}

// Convenience wrapper that reports the failure to stderr and increments a
// failure counter. Matches the patterns used elsewhere in the verification
// suite (e.g. check_close in elastic/elreal/math/sqrt.cpp).
template<typename TargetType>
inline int report_against_elreal_oracle(
	const char* label,
	const TargetType& target,
	const elreal& oracle,
	int safety_margin = 2)
{
	if (check_against_elreal_oracle(target, oracle, safety_margin)) return 0;
	std::cerr << "FAIL: " << label << " target=" << double(target)
		<< " oracle=" << double(oracle)
		<< " (out of tolerance derived from digits10="
		<< std::numeric_limits<TargetType>::digits10 << ")\n";
	return 1;
}

}} // namespace sw::universal
