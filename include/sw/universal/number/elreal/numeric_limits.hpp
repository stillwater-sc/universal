#pragma once
// numeric_limits.hpp: numeric_limits specialization for the elreal type (Phase A stub)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// Phase A ships a minimal specialization: the constants that downstream code
// (e.g. generic algorithms templated on a real type) typically asks about,
// stubbed to the underlying `double` ranges. Phase E will refine these as the
// elreal-native value constants (pi, e, ln2, ...) come online.
#include <limits>
#include <universal/number/elreal/elreal_fwd.hpp>
// elreal_impl.hpp is included directly so SpecificValue is in scope for the
// infinity / NaN / max / min constructors below. Without this, this header
// is only correct when included via the elreal.hpp umbrella in a specific
// order; including it directly here removes that fragile coupling.
#include <universal/number/elreal/elreal_impl.hpp>

namespace std {

template<>
class numeric_limits< sw::universal::elreal > {
public:
	using ElrealType = sw::universal::elreal;
	static constexpr bool is_specialized = true;

	static ElrealType (min)() {
		return ElrealType(std::numeric_limits<double>::min());
	}

	static ElrealType (max)() {
		return ElrealType(std::numeric_limits<double>::max());
	}

	static ElrealType lowest() {
		// Phase A has no unary minus on elreal (arithmetic lands in Phase C).
		// Construct directly from the lowest double instead.
		return ElrealType(std::numeric_limits<double>::lowest());
	}

	// elreal precision is unbounded by construction; the value reported here
	// is the *initial* precision of a value constructed from a single double.
	// Phase D will introduce a per-call refinement budget that callers can
	// query for the actual upper bound of a given expression.
	static constexpr int digits     = std::numeric_limits<double>::digits;
	static constexpr int digits10   = std::numeric_limits<double>::digits10;
	static constexpr int max_digits10 = std::numeric_limits<double>::max_digits10;

	static constexpr bool is_signed  = true;
	static constexpr bool is_integer = false;
	static constexpr bool is_exact   = false;
	static constexpr int  radix      = 2;

	static ElrealType epsilon()      { return ElrealType(std::numeric_limits<double>::epsilon()); }
	static ElrealType round_error()  { return ElrealType(0.5); }

	static constexpr int  min_exponent   = std::numeric_limits<double>::min_exponent;
	static constexpr int  min_exponent10 = std::numeric_limits<double>::min_exponent10;
	static constexpr int  max_exponent   = std::numeric_limits<double>::max_exponent;
	static constexpr int  max_exponent10 = std::numeric_limits<double>::max_exponent10;

	static constexpr bool has_infinity      = true;
	static constexpr bool has_quiet_NaN     = true;
	static constexpr bool has_signaling_NaN = true;
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss   = false;

	static ElrealType infinity()      { return ElrealType(sw::universal::SpecificValue::infpos); }
	static ElrealType quiet_NaN()     { return ElrealType(sw::universal::SpecificValue::qnan); }
	static ElrealType signaling_NaN() { return ElrealType(sw::universal::SpecificValue::snan); }
	// Per the C++ standard: when has_denorm == denorm_absent, denorm_min()
	// must equal min(). elreal does not expose denormals at the number-system
	// level (the lazy stream has no subnormal regime; the underlying double's
	// denormals are an implementation detail of a single component, not of the
	// value as a whole), so the contract pair is (denorm_absent, min()).
	static ElrealType denorm_min()    { return (min)(); }

	static constexpr bool is_iec559    = false;
	static constexpr bool is_bounded   = false;  // unbounded by design
	static constexpr bool is_modulo    = false;
	static constexpr bool traps        = false;
	static constexpr bool tinyness_before  = false;
	static constexpr float_round_style round_style = round_to_nearest;
};

} // namespace std
