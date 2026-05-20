#pragma once
// elreal_impl.hpp: Exact Lazy Real (McCleeary 2019) -- foundation skeleton (Phase A of epic #873)
//
// Copyright (C) 2017 Stillwater Supercomputing, Inc.
// SPDX-License-Identifier: MIT
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.
//
// =============================================================================
// elreal: Exact Lazy Real number system
// =============================================================================
//
// Reference:
//   McCleeary, R. (2019). "Lazy Exact Real Arithmetic Using Floating Point
//   Operations." Ph.D. dissertation, University of Iowa.
//   https://iro.uiowa.edu/esploro/outputs/doctoral/Lazy-exact-real-arithmetic-using-floating/9983776998202771
//
// This phase (Phase A, issue #874) establishes the type skeleton, storage
// representation, and refinement protocol. No arithmetic is implemented here;
// that lands in Phase C (#876). Comparison and sign determination land in
// Phase D (#877). The lazy machinery (closure-based generator) lights up in
// Phase C; this phase exposes the API surface and a single-component
// initialisation path so other code can compile against the type.
//
// -----------------------------------------------------------------------------
// Design decisions for Phase A (settled per the issue's design-questions list)
// -----------------------------------------------------------------------------
//
// 1. STORAGE REPRESENTATION
//
//    The value is stored as a memoized stream of double-precision components
//    plus a generator that produces successive components on demand:
//
//        mutable std::vector<double>               _components;
//        mutable std::function<double(std::size_t)> _generator;   // (Phase C)
//        mutable std::size_t                        _computed_depth = 0;
//
//    The members are `mutable` because refinement happens in const contexts
//    (notably during comparison and sign determination): the *logical* value
//    does not change, only the *representation precision* deepens.
//
//    Why this and not the alternatives:
//      - Closure-based generator is the natural fit for McCleeary's lazy
//        formulation. It re-uses Universal's existing EFT primitives
//        (`error_free_ops.hpp`): each successive component is the EFT
//        residual from the previous level.
//      - The existing design note `docs/multi-component/exact-lazy-arithmetic.md`
//        sketches this same approach.
//      - An explicit DAG of operation nodes (more powerful for sharing common
//        sub-expressions) is a future possibility, but a strictly larger
//        engineering scope. Phase A's API surface is independent of the
//        choice, so a migration is local to the implementation.
//
//    Trade-offs accepted:
//      - `std::function` type-erasure overhead (~one indirect call per
//        refinement step).
//      - Closures capture operands by value, which can be deep for nested
//        expressions; Phase C will introduce a `std::shared_ptr` for operand
//        sharing if measurements warrant it.
//      - The type is not constexpr-friendly. Phase A is not aiming for
//        compile-time elreal.
//
// 2. APPROXIMATION ELEMENT
//
//    Each element of `_components` is a single `double` interpreted as a
//    Shewchuk-style correction term:
//
//        value = _components[0] + _components[1] + _components[2] + ...
//
//    with the non-overlapping property `|_components[i+1]| < ulp(_components[i])`
//    preserved across operations (the EFTs in `error_free_ops.hpp` are designed
//    to produce this).
//
//    Why this and not (lo, hi) intervals or `floatcascade<2>` per element:
//      - Reuses EFT primitives directly; no conversion layer.
//      - Half the storage of an interval representation.
//      - Sign / comparison are still decidable: once
//        `|sum(first N components)| > 2 * ulp(_components[N-1])`, the sign
//        of the truncated sum equals the sign of the true value. The
//        comparison machinery in Phase D refines until this holds, or until
//        the per-call budget is exhausted.
//
// 3. REFINEMENT PROTOCOL
//
//    Two-tier API:
//      - `at(k)` is the primitive: ensure `_components[k]` is materialised
//        (call the generator if `k >= _computed_depth`) and return it.
//      - `refine_to(precision_bits)` is the user-facing wrapper:
//        call `at(0), at(1), ..., at(ceil(precision_bits / 53))` so the
//        prefix of the stream is materialised to at least `precision_bits`
//        bits of cumulative precision.
//
//    The primitive is `at`; everything else is convenience. This shape is
//    independent of the storage choice (the alternative DAG implementation
//    would expose the same `at(k)` entry point).
//
// -----------------------------------------------------------------------------
// What ships in Phase A vs later phases
// -----------------------------------------------------------------------------
//
// Phase A (this file, skeleton only):
//   - Construct from `double` (the trivial case: a single-component stream)
//   - Construct from `int`, `long`, `long long` (all exact at this stage if
//     within double's representable range)
//   - `at(0)` and `refine_to(precision_bits)` as no-ops past component 0
//   - `operator double()` returns `_components[0]`
//   - Triviality is *not* claimed: the type contains `std::vector` and (in
//     Phase C) `std::function`, neither of which is trivially constructible.
//     Universal's library-wide `ReportTrivialityOfType` is reported, not
//     asserted, for elastic types -- consistent with `ereal`.
//
// Deferred to later phases:
//   - Construction from `p/q` rationals and decimal strings (Phase B, #875)
//   - Cross-construction from `dd`, `qd`, `ereal<N>` (Phase B, #875)
//   - The closure-based generator and the four ring operations (Phase C, #876)
//   - Comparison, sign, and the refinement budget policy (Phase D, #877)
//   - Math functions (Phase E, #878)
//
// =============================================================================

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <vector>
#include <type_traits>

#include <universal/number/elreal/exceptions.hpp>
#include <universal/number/elreal/elreal_fwd.hpp>
#include <universal/number/shared/specific_value_encoding.hpp>

namespace sw { namespace universal {

class elreal {
public:
	// IEEE-754 double precision constants for constructing special values.
	// These mirror ereal's analogous constants and exist so numeric_limits
	// can be specialized without including the entire arithmetic implementation.
	static constexpr int EXP_BIAS         = 1023;
	static constexpr int MAX_EXP          = 1024;
	static constexpr int MIN_EXP_NORMAL   = -1022;

	// ---------------------------------------------------------------------
	// constructors
	// ---------------------------------------------------------------------
	elreal() : _components{}, _computed_depth{0} {
		// canonical zero -- empty stream interpreted as 0
	}

	// Construction from native floating-point types. The stream starts with
	// the input value as its sole component; subsequent refinements (in
	// Phase C onward) extend the stream.
	explicit elreal(double v) : _components{v}, _computed_depth{1} {}
	explicit elreal(float v) : _components{static_cast<double>(v)}, _computed_depth{1} {}

	// Construction from integer types. Integers up to 2^53 are exact in
	// `double`; larger integers are correctly representable only when the
	// rational constructor lands in Phase B. For now, store the rounded
	// double; Phase B will extend the stream with the residual when needed.
	explicit elreal(signed char v)        : elreal(static_cast<double>(v)) {}
	explicit elreal(short v)              : elreal(static_cast<double>(v)) {}
	explicit elreal(int v)                : elreal(static_cast<double>(v)) {}
	explicit elreal(long v)               : elreal(static_cast<double>(v)) {}
	explicit elreal(long long v)          : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned char v)      : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned short v)     : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned int v)       : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned long v)      : elreal(static_cast<double>(v)) {}
	explicit elreal(unsigned long long v) : elreal(static_cast<double>(v)) {}

	// Specific-value construction (canonical zero / NaN / inf encodings).
	// Phase A treats NaN / inf as single-component leading values; finer
	// special-value semantics arrive with arithmetic in Phase C.
	explicit elreal(SpecificValue code) : _components{}, _computed_depth{0} {
		switch (code) {
		case SpecificValue::infpos:
			_components.push_back(std::numeric_limits<double>::infinity());
			_computed_depth = 1;
			break;
		case SpecificValue::infneg:
			_components.push_back(-std::numeric_limits<double>::infinity());
			_computed_depth = 1;
			break;
		case SpecificValue::qnan:
			_components.push_back(std::numeric_limits<double>::quiet_NaN());
			_computed_depth = 1;
			break;
		case SpecificValue::snan:
			_components.push_back(std::numeric_limits<double>::signaling_NaN());
			_computed_depth = 1;
			break;
		case SpecificValue::maxpos:
			_components.push_back(std::numeric_limits<double>::max());
			_computed_depth = 1;
			break;
		case SpecificValue::maxneg:
			_components.push_back(std::numeric_limits<double>::lowest());
			_computed_depth = 1;
			break;
		case SpecificValue::minpos:
			_components.push_back(std::numeric_limits<double>::min());
			_computed_depth = 1;
			break;
		case SpecificValue::minneg:
			_components.push_back(-std::numeric_limits<double>::min());
			_computed_depth = 1;
			break;
		case SpecificValue::zero:
		default:
			// empty stream == 0
			break;
		}
	}

	// rule-of-five: defaulted, defensible for a vector-holding type
	elreal(const elreal&)            = default;
	elreal(elreal&&) noexcept        = default;
	elreal& operator=(const elreal&) = default;
	elreal& operator=(elreal&&) noexcept = default;
	~elreal()                        = default;

	// ---------------------------------------------------------------------
	// refinement protocol
	// ---------------------------------------------------------------------
	//
	// at(k):           the primitive. Returns the k-th component of the
	//                  lazy stream, generating it if necessary. In Phase A
	//                  the generator is absent, so k >= _computed_depth
	//                  returns 0.0 (the implicit trailing extension).
	//
	// refine_to(p):    the user-facing wrapper. Ensures the stream has at
	//                  least ceil(p / 53) components materialised. No-op
	//                  in Phase A.

	double at(std::size_t k) const noexcept {
		if (k < _computed_depth) return _components[k];
		// Phase A: no generator; the implicit extension is 0.0.
		return 0.0;
	}

	void refine_to(std::size_t precision_bits) const noexcept {
		// Phase A: no-op. Phase C will iteratively call at(k) to materialise
		// the prefix.
		(void)precision_bits;
	}

	std::size_t computed_depth() const noexcept { return _computed_depth; }

	// ---------------------------------------------------------------------
	// selectors
	// ---------------------------------------------------------------------

	// Sum of the currently-computed components -- the best estimate of the
	// true value at the current refinement depth.
	explicit operator double() const noexcept {
		double s = 0.0;
		for (std::size_t i = 0; i < _computed_depth; ++i) s += _components[i];
		return s;
	}

	// Component access for tests and inspection. Returns a copy; the
	// underlying vector is not mutable through this accessor.
	const std::vector<double>& components() const noexcept { return _components; }

	bool iszero() const noexcept { return _computed_depth == 0 || double(*this) == 0.0; }

	bool isnan() const noexcept {
		for (std::size_t i = 0; i < _computed_depth; ++i) {
			if (std::isnan(_components[i])) return true;
		}
		return false;
	}

	bool isinf() const noexcept {
		for (std::size_t i = 0; i < _computed_depth; ++i) {
			if (std::isinf(_components[i])) return true;
		}
		return false;
	}

private:
	// The lazy stream of components. Mutable because refinement is invoked
	// in const contexts (comparison, decode-to-double).
	mutable std::vector<double> _components;

	// The high-water mark of materialised components. Phase A always has
	// _computed_depth == _components.size(); Phase C may pre-allocate
	// _components and grow _computed_depth as the generator fires.
	mutable std::size_t _computed_depth;

	// Generator slot for Phase C. Empty in Phase A; the at(k) entry point
	// returns the implicit-zero extension when k >= _computed_depth.
	//
	// (Declared here as a comment to document the planned storage layout;
	// the actual member lands in Phase C alongside the closure-instantiation
	// patterns.)
	//
	// mutable std::function<double(std::size_t)> _generator;
};

}} // namespace sw::universal
